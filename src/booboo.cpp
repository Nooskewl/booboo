#include <climits>

#include <fstream>
#include <sstream>
#include <iostream>

#include <sys/stat.h>

#include <shim5/shim5.h>
#include <shim5/internal/gfx.h>

using namespace noo;

#include <twinkle.h>

#include "booboo/booboo.h"
#include "booboo/internal.h"
#include "booboo/game_lib.h"

#include <stack>

static std::map<std::string, int> library_map;
static std::map<char, booboo::token_func> token_map;
static std::map<std::string, int> expression_map;
static std::vector<booboo::expression_func> expression_handlers;
static std::stack< std::vector<booboo::Token> > var_args;
static std::stack<int> num_var_args_args;
static bool break_on_interpret = false;
static std::vector<std::string> backtrace;
static std::vector<booboo::Watchpoint> watchpoints;

static void skip_whitespace(booboo::Program *prg)
{
	while (prg->s->p < prg->s->code.length() && isspace(prg->s->code[prg->s->p])) {
		if (prg->s->code[prg->s->p] == '\n') {
			prg->s->line++;
		}
		prg->s->p++;
	}
}

namespace booboo {

Program *prg;
Program *prg_func;
std::map<std::string, void *> black_box;
std::string reset_game_name;
std::string main_program_name;
int return_code;
bool quit;
bool callbacks_enabled;
std::string (*load_text)(std::string filename);
std::vector<booboo::library_func> library;
std::vector<Timer_Callback> timer_callbacks;
std::vector<std::string> function_breakpoints;
std::vector<std::string> file_breakpoints;
std::map<std::string, std::string> src_code;

std::vector<std::string> cli_args;

// And this all makes BooBoo work

bool Variable::operator==(const Variable &var) const
{
	if (type != var.type) {
		return false;
	}

	switch (type) {
		case NUMBER:
		case FUNCTION:
		case LABEL:
			return n == var.n;
		case STRING:
			return s == var.s;
		case VECTOR:
			return v == var.v;
		case MAP:
			return m == var.m;
		case POINTER:
			return p == var.p;
		default:
			return true;
	}

	return false;
}

void Variable::set(const Variable &var)
{
	if (constant) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}

	type = var.type;

	switch (type) {
		case NUMBER:
		case FUNCTION:
		case LABEL:
			n = var.n;
			break;
		case STRING:
			s = var.s;
			break;
		case VECTOR:
			v = var.v;
			break;
		case MAP:
			m = var.m;
			break;
		case EXPRESSION:
			e = var.e;
			break;
		case FISH:
			f = var.f;
			break;
		case POINTER:
			p = var.p;
			break;
		case USER:
			n = var.n;
			s = var.s;
			v = var.v;
			m = var.m;
			e = var.e;
			f = var.f;
			p = var.p;
			break;
		default:
			break;
	}

	changed();
}

Variable& Variable::operator=(const Variable &var)
{
	if (constant) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}

	type = var.type;
	name = var.name;
	constant = var.constant;

	switch (type) {
		case NUMBER:
		case FUNCTION:
		case LABEL:
			n = var.n;
			break;
		case STRING:
			s = var.s;
			break;
		case VECTOR:
			v = var.v;
			break;
		case MAP:
			m = var.m;
			break;
		case EXPRESSION:
			e = var.e;
			break;
		case FISH:
			f = var.f;
			break;
		case POINTER:
			p = var.p;
			break;
		case USER:
			n = var.n;
			s = var.s;
			v = var.v;
			m = var.m;
			e = var.e;
			f = var.f;
			p = var.p;
			break;
		default:
			break;
	}

	changed();

	return *this;
}

Variable::Variable(const Variable &var) :
	name(var.name),
	constant(var.constant),
	type(var.type)
{
	switch (type) {
		case NUMBER:
		case FUNCTION:
		case LABEL:
			n = var.n;
			break;
		case STRING:
			s = var.s;
			break;
		case VECTOR:
			v = var.v;
			break;
		case MAP:
			m = var.m;
			break;
		case EXPRESSION:
			e = var.e;
			break;
		case FISH:
			f = var.f;
			break;
		case POINTER:
			p = var.p;
			break;
		case USER:
			n = var.n;
			s = var.s;
			v = var.v;
			m = var.m;
			e = var.e;
			f = var.f;
			p = var.p;
			break;
		default:
			break;
	}

	changed();
}

Variable::Variable() :
	constant(false),
	type(UNTYPED)
{
}

Variable::~Variable()
{
}

void Variable::clear()
{
	v.clear();
	m.clear();
	n = 0;
	s = "";
	p = nullptr;
}

void Variable::set_type(Variable_Type type, bool clear_values)
{
	if (constant) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}
	this->type = type;
	if (clear_values) {
		clear();
	}
	changed();
}

Variable::Variable_Type Variable::get_type()
{
	return type;
}

void Variable::set_n(double n)
{
	if (constant) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}
	this->n = n;
	changed();
}

double Variable::get_n()
{
	return n;
}

void Variable::set_s(std::string s)
{
	if (constant) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}
	this->s = s;
	changed();
}

std::string Variable::get_s()
{
	return s;
}

void Variable::set_p(Variable *p)
{
	if (constant) {
		printf("HUH 1\n"); exit(0);
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}
	this->p = p;
	changed();
}

Variable *Variable::get_p()
{
	return p;
}

void Variable::changed()
{
	for (size_t i = 0; i < watchpoints.size(); i++) {
		if (watchpoints[i].p == this) {
			evaluate_expression(prg, watchpoints[i].e);
			if (prg->result->get_n() != 0) {
				debug("Watchpoint " + util::itos(i) + " (" + watchpoints[i].var + ":" +  watchpoints[i].expr + ") hit...");
			}
		}
	}
}

static std::string escape_string(std::string s)
{
	std::string ret;
	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] == '\n') {
			ret += "\\n";
		}
		else if (s[i] == '\\') {
			ret += "\\\\";
		}
		else {
			char c[2];
			c[0] = s[i];
			c[1] = 0;
			ret += c;
		}
	}
	return ret;
}

std::string get_file_name(Program *prg)
{
	if (prg->complete_pass != PASS2) {
		if (prg->real_file_names.size() > prg->s->line) {
			return prg->real_file_names[prg->s->line];
		}
		else {
			return "UNKNOWN";
		}
	}
	else {
		if (prg->s->line_numbers.size() <= prg->s->pc) {
			return "UNKNOWN";
		}

		int l = prg->s->line_numbers[prg->s->pc];

		if (prg->real_file_names.size() <= (unsigned int)l) {
			return "UNKNOWN";
		}

		return prg->real_file_names[l];
	}
}

int get_line_num(Program *prg)
{
	if (prg->complete_pass != PASS2) {
		if (prg->real_line_numbers.size() > prg->s->line) {
			return prg->real_line_numbers[prg->s->line];
		}
		else {
			return -1;
		}
	}
	else {
		int l;

		l = prg->s->pc;

		if ((unsigned int)l < prg->s->line_numbers.size()) {
			l = prg->s->line_numbers[l];
		}
		else {
			l = 0;
		}

		if ((unsigned int)l < prg->real_line_numbers.size()) {
			return prg->real_line_numbers[l];
		}
		else {
			return 0;
		}
	}
}

std::string get_error_info(Program *prg)
{
	return get_file_name(prg) + ":" + util::itos(get_line_num(prg));
}

static std::string tokenfunc_label(Program *prg)
{
	prg->s->p++;
	return ":";
}

static std::string tokenfunc_string(Program *prg)
{
	char s[2];
	s[1] = 0;

	int prev = -1;
	int prev_prev = -1;

	std::string tok = "\"";
	prg->s->p++;

	if (prg->s->p < prg->s->code.length()) {
		while (prg->s->p < prg->s->code.length() && (prg->s->code[prg->s->p] != '"' || (prev == '\\' && prev_prev != '\\')) && prg->s->code[prg->s->p] != '\n') {
			s[0] = prg->s->code[prg->s->p];
			tok += s;
			prev_prev = prev;
			prev = prg->s->code[prg->s->p];
			prg->s->p++;
		}

		tok += "\"";

		prg->s->p++;
	}

	return tok;
}

static std::string tokenfunc_openbrace(Program *prg)
{
	prg->s->p++;
	return "{";
}

static std::string tokenfunc_closebrace(Program *prg)
{
	prg->s->p++;
	return "}";
}

static std::string tokenfunc_comment(Program *prg)
{
	prg->s->p++;
	return ";";
}

static std::string tokenfunc_subtract(Program *prg)
{
	char s[2];
	s[1] = 0;

	prg->s->p++;
	if (prg->s->p < prg->s->code.length() && isdigit(prg->s->code[prg->s->p])) {
		std::string tok = "-";
		while (prg->s->p < prg->s->code.length() && (isdigit(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '.')) {
			s[0] = prg->s->code[prg->s->p];
			tok += s;
			prg->s->p++;
		}
		return tok;
	}
	else {
		return "-";
	}
}

static std::string tokenfunc_equals(Program *prg)
{
	prg->s->p++;
	return "=";
}

static std::string tokenfunc_compare(Program *prg)
{
	prg->s->p++;
	return "?";
}

static std::string tokenfunc_divide(Program *prg)
{
	prg->s->p++;
	if (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] == '*') {
		prg->s->p++;
		return "/*";
	}
	return "/";
}

static std::string tokenfunc_expression(Program *prg)
{
	std::string e;
	int open = 0;

	while (prg->s->p < prg->s->code.length()) {
		char buf[2];
		buf[0] = prg->s->code[prg->s->p];
		buf[1] = 0;
		e += buf;
		prg->s->p++;
		if (buf[0] == '(') {
			open++;
		}
		else if (buf[0] == ')') {
			open--;
			if (open == 0) {
				break;
			}
		}
		else if (buf[0] == '\n') {
			prg->s->line++;
		}
	}

	return e;
}

static std::string tokenfunc_fish(Program *prg)
{
	std::string e;
	int open = 0;

	while (prg->s->p < prg->s->code.length()) {
		char buf[2];
		buf[0] = prg->s->code[prg->s->p];
		buf[1] = 0;
		e += buf;
		prg->s->p++;
		if (buf[0] == '[') {
			open++;
		}
		else if (buf[0] == ']') {
			open--;
			if (open == 0) {
				break;
			}
		}
		else if (buf[0] == '\n') {
			prg->s->line++;
		}
	}

	return e;
}

static std::string tokenfunc_hex(Program *prg)
{
	prg->s->p++;

	std::string num;

	while (prg->s->p < prg->s->code.length() && ((prg->s->code[prg->s->p] >= 'A' && prg->s->code[prg->s->p] <= 'F') || (prg->s->code[prg->s->p] >= 'a' && prg->s->code[prg->s->p] <= 'f') || (prg->s->code[prg->s->p] >= '0' && prg->s->code[prg->s->p] <= '9'))) {
		char buf[2];
		buf[1] = 0;
		buf[0] = prg->s->code[prg->s->p];
		num += buf;
		prg->s->p++;
	}

	int n;
	std::stringstream ss;
	ss << std::hex << num;
	ss >> n;

	char buf[1000];
	snprintf(buf, 1000, "%d", n);

	return buf;
}

static std::string tokenfunc_ref(Program *prg)
{
	prg->s->p++;
	return "~";
}

static std::string tokenfunc_mlcomment(Program *prg)
{
	prg->s->p++;
	if (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] == '/') {
		prg->s->p++;
		return "*/";
	}
	return "*";
}

static std::string tokenfunc_deref(Program *prg)
{
	prg->s->p++;
	return "`";
}

static std::string tokenfunc_char(Program *prg)
{
	prg->s->p++;

	int n = prg->s->code[prg->s->p];

	prg->s->p++;
	prg->s->p++;

	char buf[1000];
	snprintf(buf, 1000, "%d", n);

	return buf;
}

static std::string token(Program *prg, Token::Token_Type &ret_type)
{
	skip_whitespace(prg);

	if (prg->s->p >= prg->s->code.length()) {
		return "";
	}

	std::string tok;

	if (isalpha(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '_') {
		int start = prg->s->p;
		while (prg->s->p < prg->s->code.length() && (isdigit(prg->s->code[prg->s->p]) || isalpha(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '_')) {
			prg->s->p++;
		}
		tok = prg->s->code.substr(start, prg->s->p-start);
		ret_type = Token::SYMBOL;
		return tok;
	}
	else if (isdigit(prg->s->code[prg->s->p]) || (prg->s->p < prg->s->code.length()-1 && prg->s->code[prg->s->p] == '-' && isdigit(prg->s->code[prg->s->p+1]))) {
		int start = prg->s->p;
		while (prg->s->p < prg->s->code.length() && (isdigit(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '.' || prg->s->code[prg->s->p] == '-')) {
			prg->s->p++;
		}
		tok = prg->s->code.substr(start, prg->s->p-start);
		ret_type = Token::NUMBER;
		return tok;
	}

	char c = prg->s->code[prg->s->p];

	if (c == '(' || c == '[') {
		ret_type = Token::SYMBOL;
	}
	else if (c == '#' || c == '\'') {
		ret_type = Token::NUMBER;
	}
	else {
		ret_type = Token::STRING;
	}

	std::map<char, token_func>::iterator it = token_map.find(prg->s->code[prg->s->p]);
	if (it != token_map.end()) {
		std::string tok = (*it).second(prg);
		if (tok == "/*") {
			while ((tok = token(prg, ret_type)) != "*/") {
			}
			return token(prg, ret_type);
		}
		else if (tok == ";") {
			while (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] != '\n') {
				prg->s->p++;
			}
			prg->s->line++;
			if (prg->s->p < prg->s->code.length()) {
				prg->s->p++;
			}
			return token(prg, ret_type);
		}
		return tok;
	}

	my_throw(Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg)));

	return "";
}

bool process_includes(Program *prg)
{
	bool ret = false;

	int total_added = 0;

	std::string code;

	std::string tok;

	prg->s->p = 0;
	prg->s->line = 0;
	prg->s->line_numbers.clear();

	int prev = prg->s->p;
	int start = 0;

	Token::Token_Type tt;

	while ((tok = token(prg, tt)) != "") {
		if (tok == "include") {
			int start_line = prg->s->line;

			std::string name = token(prg, tt);

			if (name == "") {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Expected include parameters at " + get_error_info(prg)));
			}

			if (name[0] != '"') {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid include name at " + get_error_info(prg)));
			}

			name = util::remove_quotes(util::unescape_string(name));

			while (isspace(prg->s->code[prev])) {
				prev++;
			}

			code += prg->s->code.substr(start, prev-start);

			std::string new_code;
			std::string fn;
			fn = name;

			if (shim::cpa) {
				new_code = booboo::load_text("scripts/" + name);
			}
			else {
				new_code = booboo::load_text(name);
			}

			int nlines = 1;
			int i = 0;
			while (new_code[i] != 0) {
				if (new_code[i] == '\n') {
					nlines++;
				}
				i++;
			}

			code += new_code;
		
			prg->real_line_numbers[start_line+total_added] = 1;	
			prg->real_file_names[start_line+total_added] = fn;
			for (int i = 1; i < nlines; i++) {
				prg->real_line_numbers.insert(prg->real_line_numbers.begin()+start_line+i+total_added, i+1);
				prg->real_file_names.insert(prg->real_file_names.begin()+start_line+i+total_added, fn);
			}

			start = prg->s->p;

			total_added += nlines;

			ret = true;

			break;
		}

		prev = prg->s->p;
	}

	code += prg->s->code.substr(start, prg->s->code.length()-start);

	prg->s->code = code;
	prg->s->p = 0;
	prg->s->line = 0;

	return ret;
}

static void backup(Program *prg, int func, bool restore_locals = true)
{
	std::map<std::string, int> backup;

	std::map<std::string, int>::iterator it;
	for (it = prg->locals[func].begin(); it != prg->locals[func].end(); it++) {
		std::pair<std::string, int> pair = *it;
		if (prg->variables_map.find(pair.first) != prg->variables_map.end()) {
			backup[pair.first] = prg->variables_map[pair.first];
		}
		if (prg->variables[pair.second].get_type() == Variable::LABEL) {
			prg->variables_map[pair.first] = pair.second;
		}
	}

	prg->backup.push_back(backup);
}

static void restore(Program *prg, int func)
{
	std::map<std::string, int>::iterator it;
	for (it = prg->locals[func].begin(); it != prg->locals[func].end(); it++) {
		std::pair<std::string, int> pair = *it;
		std::map<std::string, int>::iterator it2;
		it2 = prg->variables_map.find(pair.first);
		if (it2 != prg->variables_map.end()) {
			prg->variables_map.erase(it2);
		}
	}

	std::map<std::string, int> backup = prg->backup.back();
	prg->backup.pop_back();

	for (it = backup.begin(); it != backup.end(); it++) {
		std::pair<std::string, int> pair = *it;
		prg->variables_map[pair.first] = pair.second;
	}
}

Variable::Expression parse_expression(Program *prg, Program *func, std::string expr, Pass pass)
{
	int p = 0;

	Variable::Expression e;
	
	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	if (p >= (int)expr.length()-1) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid expression at " + get_error_info(prg)));
	}
	p++; // skip (
	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	std::string name;
	char buf[2];
	buf[0] = expr[p];
	buf[1] = 0;
	name += buf;
	p++;
	e.dereference = 0;
	int first = name[0];
	int open_count = name[0] == '[' || name[0] == '(';
	while (p < (int)expr.length()) {
		if ((expr[p] == ' ' || expr[p] == ')') && open_count == 0) {
			break;
		}
		else if (first == '[' && expr[p] == '[') {
			open_count++;
		}
		else if (first == '[' && expr[p] == ']') {
			open_count--;
		}
		if (first == '(' && expr[p] == '(') {
			open_count++;
		}
		else if (first == '(' && expr[p] == ')') {
			open_count--;
		}
		char buf[2];
		buf[0] = expr[p];
		buf[1] = 0;
		name += buf;
		p++;
	}

	e.name = name;

	if (name.length() > 0 && name[0] == '(') {
		Variable v1;
		v1.name = "__e" + util::itos(prg->expression_i++);
		v1.set_type(Variable::EXPRESSION);

		int i = prg->var_i++;

		if (pass == PASS1) {
			prg->variables.push_back(v1);
		}
		else if (pass == PASS2) {
			prg->variables_map[v1.name] = i;
		}

		prg->variables[i].e = parse_expression(prg, func, e.name, pass);

		e.name = " ex ";
		e.i = i;
	}
	else if (name.length() > 0 && name[0] == '[') {
		Variable v1;
		v1.name = "__f" + util::itos(prg->fish_i++);
		v1.set_type(Variable::FISH);

		int i = prg->var_i++;

		if (pass == PASS1) {
			prg->variables.push_back(v1);
		}
		else if (pass == PASS2) {
			prg->variables_map[v1.name] = i;
		}

		prg->variables[i].f = parse_fish(prg, func, e.name, pass);

		e.name = " fi ";
		e.i = i;
	}
	else if (expression_map.find(name) == expression_map.end()) {
		e.i = -1;
		e.name = name;
	}
	else {
		e.i = expression_map[name];
	}

	bool done = false;

	int deref = 0;

	while (!done) {
		while (isspace(expr[p]) && p < (int)expr.length()) {
			p++;
		}
		if (p >= (int)expr.length()) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid expression at " + get_error_info(prg)));
		}
		char c = expr[p];
		Token tok;
		if (c == ';') {
			p++;
			while (expr[p] != '\n' && p < (int)expr.length()) {
				p++;
			}
			if (p < (int)expr.length()) {
				p++;
			}
		}
		else if (c == '/' && p+1 < (int)expr.length() && expr[p+1] == '*') {
			p++;
			while (p < (int)expr.length() && (expr[p] != '/' || (p > 0 && expr[p-1] != '*'))) {
				p++;
			}
			if (p < (int)expr.length()) {
				p++;
			}
		}
		else if (c == '(') {
			tok.type = Token::SYMBOL;
			int open = 0;
			std::string new_expr;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				new_expr += buf;
				p++;
				if (buf[0] == '(') {
					open++;
				}
				else if (buf[0] == ')') {
					open--;
				}
				if (open == 0) {
					break;
				}
			}
			tok.i = prg->var_i++;
			tok.dereference = deref;
			deref = 0;

			Variable v1;
			v1.name = "__e" + util::itos(prg->expression_i++);
			v1.set_type(Variable::EXPRESSION);

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}
			
			prg->variables[tok.i].e = parse_expression(prg, func, new_expr, pass);
		}
		else if (c == ')') {
			done = true;
			deref = 0;
			break;
		}
		else if (c == '[') {
			tok.type = Token::SYMBOL;
			int open = 0;
			std::string new_expr;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				new_expr += buf;
				p++;
				if (buf[0] == '[') {
					open++;
				}
				else if (buf[0] == ']') {
					open--;
				}
				if (open == 0) {
					break;
				}
			}
			tok.i = prg->var_i++;
			tok.dereference = deref;
			deref = 0;

			Variable v1;
			v1.name = "__f" + util::itos(prg->fish_i++);
			v1.set_type(Variable::FISH);

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].f = parse_fish(prg, func, new_expr, pass);
		}
		else if (isdigit(c) || c == '-' || c == '.') {
			tok.type = Token::NUMBER;
			std::string str;
			while (p < (int)expr.length() && (isdigit(expr[p]) || expr[p] == '.' || expr[p] == '-')) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				p++;
			}
			tok.n = atof(str.c_str());
			tok.dereference = 0;
		}
		else if (c == '#') {
			tok.type = Token::NUMBER;
			std::string str = "";
			p++;
			int prev = -1;
			int prev_prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (!((buf[0] >= 'A' && buf[0] <= 'F') || (buf[0] >= 'a' && buf[0] <= 'f') || (buf[0] >= '0' && buf[0] <= '9'))) {
					break;
				}
				prev_prev = prev;
				prev = buf[0];
				p++;
			}
			str = util::remove_quotes(util::unescape_string(str));
			int n;
			std::stringstream ss;
			ss << std::hex << str;
			ss >> n;
			char buf[1000];
			snprintf(buf, 1000, "%d", n);
			tok.n = n;
			tok.dereference = 0;
		}
		else if (c == '\'') {
			tok.type = Token::NUMBER;
			std::string str = "";
			p++;
			int prev = -1;
			int prev_prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (buf[0] == '\'' && (prev != '\\' || prev_prev == '\\')) {
					p++;
					break;
				}
				prev_prev = prev;
				prev = buf[0];
				p++;
			}
			str = util::remove_quotes(util::unescape_string(str));
			tok.n = str[0];
			tok.dereference = 0;
		}
		else if (c == '"') {
			tok.type = Token::STRING;
			std::string str = "";
			p++;
			int prev = -1;
			int prev_prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (buf[0] == '"' && (prev != '\\' || prev_prev == '\\')) {
					p++;
					break;
				}
				prev_prev = prev;
				prev = buf[0];
				p++;
			}
			str = util::remove_quotes(util::unescape_string(str));
			tok.s = str;
			tok.dereference = 0;
		}
		else if (isalpha(c) || c == '_') {
			tok.type = Token::SYMBOL;
			std::string sym;
			while (p < (int)expr.length()) {
				if (!(isalpha(expr[p]) || expr[p] == '_' || isdigit(expr[p]))) {
					break;
				}
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				sym += buf;
				p++;
			}
			tok.dereference = deref;
			deref = 0;

			if (prg->complete_pass == PASS2) {
				size_t f = 0;
				for (f = 0; f < prg->function_names.size(); f++) {
					if (prg->function_names[f] == prg_func->s->name) {
						break;
					}
				}
				if (f < prg->function_names.size()) {
					if (prg->locals[f].find(sym) != prg->locals[f].end()) {
						tok.i = prg->locals[f][sym];
					}
					else {
						my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg)));
					}
				}
				else {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg)));
				}
			}
			else if (pass == PASS2) {
				if (prg->variables_map.find(sym) == prg->variables_map.end()) {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg)));
				}
				tok.i = prg->variables_map[sym];
			}
		}
		else if (c == '`') {
			deref++;
			p++;
			continue;
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg)));
		}

		e.v.push_back(tok);
	}

	return e;
}

Variable::Fish parse_fish(Program *prg, Program *func, std::string expr, Pass pass)
{
	int p = 0;
	Variable::Fish e;

	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	if (p >= (int)expr.length()-1) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid fish at " + get_error_info(prg)));
	}
	p++; // skip [
	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	if (expr[p] == '`') {
		while (expr[p] == '`') {
			e.dereference++;
			p++;
			while (isspace(expr[p]) && p < (int)expr.length()) {
				p++;
			}
		}
	}
	else {
		e.dereference = 0;
	}
	if (expr[p] == '[') {
		int start = p;
		int open = 1;
		p++;
		while (p < (int)expr.length()) {
			if (expr[p] == '[') {
				open++;
			}
			else if (expr[p] == ']') {
				open--;
				if (open == 0) {
					p++;
					break;
				}
			}
			p++;
		}

		Variable v;
		v.name = "__f" + util::itos(prg->fish_i++);
		v.set_type(Variable::FISH);

		if (pass == PASS1) {
			prg->variables.push_back(v);
		}
		else if (pass == PASS2) {
			prg->variables_map[v.name] = prg->var_i;
		}
		e.c_i = prg->var_i;
		prg->var_i++;

		std::string s = expr.substr(start, p-start);
		prg->variables[e.c_i].f = parse_fish(prg, func, s, pass);
	}
	else {
		std::string name;
		if (expr[p] == '(') {
			int start = p;
			int open = 1;
			p++;
			while (p < (int)expr.length()) {
				if (expr[p] == '(') {
					open++;
				}
				else if (expr[p] == ')') {
					open--;
					if (open == 0) {
						p++;
						break;
					}
				}
				p++;
			}

			Variable v;
			v.name = "__e" + util::itos(prg->expression_i++);
			v.set_type(Variable::EXPRESSION);

			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
			else if (pass == PASS2) {
				prg->variables_map[v.name] = prg->var_i;
			}
			e.c_i = prg->var_i;
			prg->var_i++;

			std::string s = expr.substr(start, p-start);
			prg->variables[e.c_i].e = parse_expression(prg, func, s, pass);
		}
		else {
			while (!isspace(expr[p]) && p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				name += buf;
				p++;
			}
			if (pass == PASS2) {
				if (prg->variables_map.find(name) == prg->variables_map.end()) {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Unknown variable at " + get_error_info(prg)));
				}
				e.c_i = prg->variables_map[name];
			}
		}
	}

	bool done = false;

	int deref = 0;

	while (!done) {
		while (isspace(expr[p]) && p < (int)expr.length()) {
			p++;
		}
		if (p >= (int)expr.length()) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid fish at " + get_error_info(prg)));
		}
		char c = expr[p];
		Token tok;
		if (c == ';') {
			p++;
			while (expr[p] != '\n' && p < (int)expr.length()) {
				p++;
			}
			if (p < (int)expr.length()) {
				p++;
			}
		}
		else if (c == '/' && p+1 < (int)expr.length() && expr[p+1] == '*') {
			p++;
			while (p < (int)expr.length() && (expr[p] != '/' || (p > 0 && expr[p-1] != '*'))) {
				p++;
			}
			if (p < (int)expr.length()) {
				p++;
			}
		}
		else if (c == '(') {
			tok.type = Token::SYMBOL;
			int open = 0;
			std::string new_expr;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				new_expr += buf;
				p++;
				if (buf[0] == '(') {
					open++;
				}
				else if (buf[0] == ')') {
					open--;
				}
				if (open == 0) {
					break;
				}
			}
			tok.i = prg->var_i++;
			tok.dereference = deref;
			deref = 0;

			Variable v1;
			v1.name = "__e" + util::itos(prg->expression_i++);
			v1.set_type(Variable::EXPRESSION);

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].e = parse_expression(prg, func, new_expr, pass);
		}
		else if (c == '[') {
			tok.type = Token::SYMBOL;
			int open = 0;
			std::string new_expr;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				new_expr += buf;
				p++;
				if (buf[0] == '[') {
					open++;
				}
				else if (buf[0] == ']') {
					open--;
				}
				if (open == 0) {
					break;
				}
			}
			tok.i = prg->var_i++;
			tok.dereference = deref;
			deref = 0;

			Variable v1;
			v1.name = "__f" + util::itos(prg->fish_i++);
			v1.set_type(Variable::FISH);

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].f = parse_fish(prg, func, new_expr, pass);
		}
		else if (c == ']') {
			done = true;
			deref = 0;
			break;
		}
		else if (isdigit(c) || c == '-' || c == '.') {
			tok.type = Token::NUMBER;
			std::string str;
			while (p < (int)expr.length() && (isdigit(expr[p]) || expr[p] == '.' || expr[p] == '-')) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				p++;
			}
			tok.n = atof(str.c_str());
			tok.dereference = 0;
		}
		else if (c == '#') {
			tok.type = Token::NUMBER;
			std::string str = "";
			p++;
			int prev = -1;
			int prev_prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (!((buf[0] >= 'A' && buf[0] <= 'F') || (buf[0] >= 'a' && buf[0] <= 'f') || (buf[0] >= '0' && buf[0] <= '9'))) {
					break;
				}
				prev_prev = prev;
				prev = buf[0];
				p++;
			}
			str = util::remove_quotes(util::unescape_string(str));
			int n;
			std::stringstream ss;
			ss << std::hex << str;
			ss >> n;
			char buf[1000];
			snprintf(buf, 1000, "%d", n);
			tok.n = n;
			tok.dereference = 0;
		}
		else if (c == '\'') {
			tok.type = Token::NUMBER;
			std::string str = "";
			p++;
			int prev = -1;
			int prev_prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (buf[0] == '\'' && (prev != '\\' || prev_prev == '\\')) {
					p++;
					break;
				}
				prev_prev = prev;
				prev = buf[0];
				p++;
			}
			str = util::remove_quotes(util::unescape_string(str));
			tok.n = str[0];
			tok.dereference = 0;
		}
		else if (c == '"') {
			tok.type = Token::STRING;
			std::string str = "";
			p++;
			int prev = -1;
			int prev_prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (buf[0] == '"' && (prev != '\\' || prev_prev == '\\')) {
					p++;
					break;
				}
				prev_prev = prev;
				prev = buf[0];
				p++;
			}
			str = util::remove_quotes(util::unescape_string(str));
			tok.s = str;
			tok.dereference = 0;
		}
		else if (isalpha(c) || c == '_') {
			tok.type = Token::SYMBOL;
			std::string sym;
			while (p < (int)expr.length()) {
				if (!(isalpha(expr[p]) || expr[p] == '_' || isdigit(expr[p]))) {
					break;
				}
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				sym += buf;
				p++;
			}
			tok.dereference = deref;
			deref = 0;

			if (prg->complete_pass == PASS2) {
				size_t f = 0;
				for (f = 0; f < prg->function_names.size(); f++) {
					if (prg->function_names[f] == prg_func->s->name) {
						break;
					}
				}
				if (f < prg->function_names.size()) {
					if (prg->locals[f].find(sym) != prg->locals[f].end()) {
						tok.i = prg->locals[f][sym];
					}
					else {
						my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg)));
					}
				}
				else {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg)));
				}
			}
			else if (pass == PASS2) {
				if (prg->variables_map.find(sym) == prg->variables_map.end()) {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg)));
				}
				tok.i = prg->variables_map[sym];
			}
		}
		else if (c == '`') {
			deref++;
			p++;
			continue;
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg)));
		}

		e.v.push_back(tok);
	}

	return e;
}

static void insert_constant(Program *prg, std::string name, double value, Pass pass)
{
	int var_index = prg->var_i;
	prg->var_i++;
	if (pass == PASS2) {
		prg->variables_map[name] = var_index;
	}
	Variable v;
	v.name = name;
	v.set_type(Variable::NUMBER);
	v.set_n(value);
	v.constant = true;
	char buf[1000];
	snprintf(buf, 1000, "%f", value);
	if (pass == PASS1) {
		prg->variables.push_back(v);
	}
}

static void insert_pointer(Program *prg, std::string name, Variable *value, Pass pass)
{
	int var_index = prg->var_i;
	prg->var_i++;
	if (pass == PASS2) {
		prg->variables_map[name] = var_index;
	}
	Variable v;
	v.name = name;
	v.set_type(Variable::POINTER);
	v.set_p(value);
	v.constant = true;
	char buf[1000];
	snprintf(buf, 1000, "%p", value);
	if (pass == PASS1) {
		prg->variables.push_back(v);
	}
}

static void insert_var(Program *prg, std::string name, Pass pass)
{
	int var_index = prg->var_i;
	prg->var_i++;
	if (pass == PASS2) {
		prg->variables_map[name] = var_index;
	}
	Variable v;
	v.name = name;
	if (pass == PASS1) {
		prg->variables.push_back(v);
	}
}

static void compile(Program *prg, Pass pass)
{
	int p_bak = prg->s->p;
	int line_bak = prg->s->line;

	prg->var_i = 0;
	prg->func_i = 0;
	prg->expression_i = 0;
	prg->fish_i = 0;

	std::string tok;
	Token::Token_Type tt;

	// Constants
	insert_var(prg, "VOID", pass);
	insert_constant(prg, "TRUE", 1, pass);
	insert_constant(prg, "FALSE", 0, pass);
	insert_constant(prg, "PI", M_PI, pass);
	insert_constant(prg, "E", M_E, pass);
	insert_pointer(prg, "NULL", nullptr, pass);
	insert_constant(prg, "BLACK", 0, pass);
	insert_constant(prg, "BLUE", 1, pass);
	insert_constant(prg, "GREEN", 2, pass);
	insert_constant(prg, "CYAN", 3, pass);
	insert_constant(prg, "RED", 4, pass);
	insert_constant(prg, "PURPLE", 5, pass);
	insert_constant(prg, "YELLOW", 6, pass);
	insert_constant(prg, "WHITE", 7, pass);
	insert_constant(prg, "KEY_UNKNOWN", TGUIK_UNKNOWN, pass);
	insert_constant(prg, "KEY_RETURN", TGUIK_RETURN, pass);
	insert_constant(prg, "KEY_ESCAPE", TGUIK_ESCAPE, pass);
	insert_constant(prg, "KEY_BACKSPACE", TGUIK_BACKSPACE, pass);
	insert_constant(prg, "KEY_TAB", TGUIK_TAB, pass);
	insert_constant(prg, "KEY_SPACE", TGUIK_SPACE, pass);
	insert_constant(prg, "KEY_EXCLAIM", TGUIK_EXCLAIM, pass);
	insert_constant(prg, "KEY_DBLAPOSTROPHE", TGUIK_DBLAPOSTROPHE, pass);
	insert_constant(prg, "KEY_HASH", TGUIK_HASH, pass);
	insert_constant(prg, "KEY_DOLLAR", TGUIK_DOLLAR, pass);
	insert_constant(prg, "KEY_PERCENT", TGUIK_PERCENT, pass);
	insert_constant(prg, "KEY_AMPERSAND", TGUIK_AMPERSAND, pass);
	insert_constant(prg, "KEY_APOSTROPHE", TGUIK_APOSTROPHE, pass);
	insert_constant(prg, "KEY_LEFTPAREN", TGUIK_LEFTPAREN, pass);
	insert_constant(prg, "KEY_RIGHTPAREN", TGUIK_RIGHTPAREN, pass);
	insert_constant(prg, "KEY_ASTERISK", TGUIK_ASTERISK, pass);
	insert_constant(prg, "KEY_PLUS", TGUIK_PLUS, pass);
	insert_constant(prg, "KEY_COMMA", TGUIK_COMMA, pass);
	insert_constant(prg, "KEY_MINUS", TGUIK_MINUS, pass);
	insert_constant(prg, "KEY_PERIOD", TGUIK_PERIOD, pass);
	insert_constant(prg, "KEY_SLASH", TGUIK_SLASH, pass);
	insert_constant(prg, "KEY_0", TGUIK_0, pass);
	insert_constant(prg, "KEY_1", TGUIK_1, pass);
	insert_constant(prg, "KEY_2", TGUIK_2, pass);
	insert_constant(prg, "KEY_3", TGUIK_3, pass);
	insert_constant(prg, "KEY_4", TGUIK_4, pass);
	insert_constant(prg, "KEY_5", TGUIK_5, pass);
	insert_constant(prg, "KEY_6", TGUIK_6, pass);
	insert_constant(prg, "KEY_7", TGUIK_7, pass);
	insert_constant(prg, "KEY_8", TGUIK_8, pass);
	insert_constant(prg, "KEY_9", TGUIK_9, pass);
	insert_constant(prg, "KEY_COLON", TGUIK_COLON, pass);
	insert_constant(prg, "KEY_SEMICOLON", TGUIK_SEMICOLON, pass);
	insert_constant(prg, "KEY_LESS", TGUIK_LESS, pass);
	insert_constant(prg, "KEY_EQUALS", TGUIK_EQUALS, pass);
	insert_constant(prg, "KEY_GREATER", TGUIK_GREATER, pass);
	insert_constant(prg, "KEY_QUESTION", TGUIK_QUESTION, pass);
	insert_constant(prg, "KEY_AT", TGUIK_AT, pass);
	insert_constant(prg, "KEY_LEFTBRACKET", TGUIK_LEFTBRACKET, pass);
	insert_constant(prg, "KEY_BACKSLASH", TGUIK_BACKSLASH, pass);
	insert_constant(prg, "KEY_RIGHTBRACKET", TGUIK_RIGHTBRACKET, pass);
	insert_constant(prg, "KEY_CARET", TGUIK_CARET, pass);
	insert_constant(prg, "KEY_UNDERSCORE", TGUIK_UNDERSCORE, pass);
	insert_constant(prg, "KEY_GRAVE", TGUIK_GRAVE, pass);
	insert_constant(prg, "KEY_A", TGUIK_A, pass);
	insert_constant(prg, "KEY_B", TGUIK_B, pass);
	insert_constant(prg, "KEY_C", TGUIK_C, pass);
	insert_constant(prg, "KEY_D", TGUIK_D, pass);
	insert_constant(prg, "KEY_E", TGUIK_E, pass);
	insert_constant(prg, "KEY_F", TGUIK_F, pass);
	insert_constant(prg, "KEY_G", TGUIK_G, pass);
	insert_constant(prg, "KEY_H", TGUIK_H, pass);
	insert_constant(prg, "KEY_I", TGUIK_I, pass);
	insert_constant(prg, "KEY_J", TGUIK_J, pass);
	insert_constant(prg, "KEY_K", TGUIK_K, pass);
	insert_constant(prg, "KEY_L", TGUIK_L, pass);
	insert_constant(prg, "KEY_M", TGUIK_M, pass);
	insert_constant(prg, "KEY_N", TGUIK_N, pass);
	insert_constant(prg, "KEY_O", TGUIK_O, pass);
	insert_constant(prg, "KEY_P", TGUIK_P, pass);
	insert_constant(prg, "KEY_Q", TGUIK_Q, pass);
	insert_constant(prg, "KEY_R", TGUIK_R, pass);
	insert_constant(prg, "KEY_S", TGUIK_S, pass);
	insert_constant(prg, "KEY_T", TGUIK_T, pass);
	insert_constant(prg, "KEY_U", TGUIK_U, pass);
	insert_constant(prg, "KEY_V", TGUIK_V, pass);
	insert_constant(prg, "KEY_W", TGUIK_W, pass);
	insert_constant(prg, "KEY_X", TGUIK_X, pass);
	insert_constant(prg, "KEY_Y", TGUIK_Y, pass);
	insert_constant(prg, "KEY_Z", TGUIK_Z, pass);
	insert_constant(prg, "KEY_LEFTBRACE", TGUIK_LEFTBRACE, pass);
	insert_constant(prg, "KEY_PIPE", TGUIK_PIPE, pass);
	insert_constant(prg, "KEY_RIGHTBRACE", TGUIK_RIGHTBRACE, pass);
	insert_constant(prg, "KEY_TILDE", TGUIK_TILDE, pass);
	insert_constant(prg, "KEY_DELETE", TGUIK_DELETE, pass);
	insert_constant(prg, "KEY_PLUSMINUS", TGUIK_PLUSMINUS, pass);
	insert_constant(prg, "KEY_CAPSLOCK", TGUIK_CAPSLOCK, pass);
	insert_constant(prg, "KEY_F1", TGUIK_F1, pass);
	insert_constant(prg, "KEY_F2", TGUIK_F2, pass);
	insert_constant(prg, "KEY_F3", TGUIK_F3, pass);
	insert_constant(prg, "KEY_F4", TGUIK_F4, pass);
	insert_constant(prg, "KEY_F5", TGUIK_F5, pass);
	insert_constant(prg, "KEY_F6", TGUIK_F6, pass);
	insert_constant(prg, "KEY_F7", TGUIK_F7, pass);
	insert_constant(prg, "KEY_F8", TGUIK_F8, pass);
	insert_constant(prg, "KEY_F9", TGUIK_F9, pass);
	insert_constant(prg, "KEY_F10", TGUIK_F10, pass);
	insert_constant(prg, "KEY_F11", TGUIK_F11, pass);
	insert_constant(prg, "KEY_F12", TGUIK_F12, pass);
	insert_constant(prg, "KEY_PRINTSCREEN", TGUIK_PRINTSCREEN, pass);
	insert_constant(prg, "KEY_SCROLLLOCK", TGUIK_SCROLLLOCK, pass);
	insert_constant(prg, "KEY_PAUSE", TGUIK_PAUSE, pass);
	insert_constant(prg, "KEY_INSERT", TGUIK_INSERT, pass);
	insert_constant(prg, "KEY_HOME", TGUIK_HOME, pass);
	insert_constant(prg, "KEY_PAGEUP", TGUIK_PAGEUP, pass);
	insert_constant(prg, "KEY_END", TGUIK_END, pass);
	insert_constant(prg, "KEY_PAGEDOWN", TGUIK_PAGEDOWN, pass);
	insert_constant(prg, "KEY_RIGHT", TGUIK_RIGHT, pass);
	insert_constant(prg, "KEY_LEFT", TGUIK_LEFT, pass);
	insert_constant(prg, "KEY_DOWN", TGUIK_DOWN, pass);
	insert_constant(prg, "KEY_UP", TGUIK_UP, pass);
	insert_constant(prg, "KEY_NUMLOCKCLEAR", TGUIK_NUMLOCKCLEAR, pass);
	insert_constant(prg, "KEY_KP_DIVIDE", TGUIK_KP_DIVIDE, pass);
	insert_constant(prg, "KEY_KP_MULTIPLY", TGUIK_KP_MULTIPLY, pass);
	insert_constant(prg, "KEY_KP_MINUS", TGUIK_KP_MINUS, pass);
	insert_constant(prg, "KEY_KP_PLUS", TGUIK_KP_PLUS, pass);
	insert_constant(prg, "KEY_KP_ENTER", TGUIK_KP_ENTER, pass);
	insert_constant(prg, "KEY_KP_1", TGUIK_KP_1, pass);
	insert_constant(prg, "KEY_KP_2", TGUIK_KP_2, pass);
	insert_constant(prg, "KEY_KP_3", TGUIK_KP_3, pass);
	insert_constant(prg, "KEY_KP_4", TGUIK_KP_4, pass);
	insert_constant(prg, "KEY_KP_5", TGUIK_KP_5, pass);
	insert_constant(prg, "KEY_KP_6", TGUIK_KP_6, pass);
	insert_constant(prg, "KEY_KP_7", TGUIK_KP_7, pass);
	insert_constant(prg, "KEY_KP_8", TGUIK_KP_8, pass);
	insert_constant(prg, "KEY_KP_9", TGUIK_KP_9, pass);
	insert_constant(prg, "KEY_KP_0", TGUIK_KP_0, pass);
	insert_constant(prg, "KEY_KP_PERIOD", TGUIK_KP_PERIOD, pass);
	insert_constant(prg, "KEY_APPLICATION", TGUIK_APPLICATION, pass);
	insert_constant(prg, "KEY_POWER", TGUIK_POWER, pass);
	insert_constant(prg, "KEY_KP_EQUALS", TGUIK_KP_EQUALS, pass);
	insert_constant(prg, "KEY_F13", TGUIK_F13, pass);
	insert_constant(prg, "KEY_F14", TGUIK_F14, pass);
	insert_constant(prg, "KEY_F15", TGUIK_F15, pass);
	insert_constant(prg, "KEY_F16", TGUIK_F16, pass);
	insert_constant(prg, "KEY_F17", TGUIK_F17, pass);
	insert_constant(prg, "KEY_F18", TGUIK_F18, pass);
	insert_constant(prg, "KEY_F19", TGUIK_F19, pass);
	insert_constant(prg, "KEY_F20", TGUIK_F20, pass);
	insert_constant(prg, "KEY_F21", TGUIK_F21, pass);
	insert_constant(prg, "KEY_F22", TGUIK_F22, pass);
	insert_constant(prg, "KEY_F23", TGUIK_F23, pass);
	insert_constant(prg, "KEY_F24", TGUIK_F24, pass);
	insert_constant(prg, "KEY_EXECUTE", TGUIK_EXECUTE, pass);
	insert_constant(prg, "KEY_HELP", TGUIK_HELP, pass);
	insert_constant(prg, "KEY_MENU", TGUIK_MENU, pass);
	insert_constant(prg, "KEY_SELECT", TGUIK_SELECT, pass);
	insert_constant(prg, "KEY_STOP", TGUIK_STOP, pass);
	insert_constant(prg, "KEY_AGAIN", TGUIK_AGAIN, pass);
	insert_constant(prg, "KEY_UNDO", TGUIK_UNDO, pass);
	insert_constant(prg, "KEY_CUT", TGUIK_CUT, pass);
	insert_constant(prg, "KEY_COPY", TGUIK_COPY, pass);
	insert_constant(prg, "KEY_PASTE", TGUIK_PASTE, pass);
	insert_constant(prg, "KEY_FIND", TGUIK_FIND, pass);
	insert_constant(prg, "KEY_MUTE", TGUIK_MUTE, pass);
	insert_constant(prg, "KEY_VOLUMEUP", TGUIK_VOLUMEUP, pass);
	insert_constant(prg, "KEY_VOLUMEDOWN", TGUIK_VOLUMEDOWN, pass);
	insert_constant(prg, "KEY_KP_COMMA", TGUIK_KP_COMMA, pass);
	insert_constant(prg, "KEY_KP_EQUALSAS400", TGUIK_KP_EQUALSAS400, pass);
	insert_constant(prg, "KEY_ALTERASE", TGUIK_ALTERASE, pass);
	insert_constant(prg, "KEY_SYSREQ", TGUIK_SYSREQ, pass);
	insert_constant(prg, "KEY_CANCEL", TGUIK_CANCEL, pass);
	insert_constant(prg, "KEY_CLEAR", TGUIK_CLEAR, pass);
	insert_constant(prg, "KEY_PRIOR", TGUIK_PRIOR, pass);
	insert_constant(prg, "KEY_RETURN2", TGUIK_RETURN2, pass);
	insert_constant(prg, "KEY_SEPARATOR", TGUIK_SEPARATOR, pass);
	insert_constant(prg, "KEY_OUT", TGUIK_OUT, pass);
	insert_constant(prg, "KEY_OPER", TGUIK_OPER, pass);
	insert_constant(prg, "KEY_CLEARAGAIN", TGUIK_CLEARAGAIN, pass);
	insert_constant(prg, "KEY_CRSEL", TGUIK_CRSEL, pass);
	insert_constant(prg, "KEY_EXSEL", TGUIK_EXSEL, pass);
	insert_constant(prg, "KEY_KP_00", TGUIK_KP_00, pass);
	insert_constant(prg, "KEY_KP_000", TGUIK_KP_000, pass);
	insert_constant(prg, "KEY_THOUSANDSSEPARATOR", TGUIK_THOUSANDSSEPARATOR, pass);
	insert_constant(prg, "KEY_DECIMALSEPARATOR", TGUIK_DECIMALSEPARATOR, pass);
	insert_constant(prg, "KEY_CURRENCYUNIT", TGUIK_CURRENCYUNIT, pass);
	insert_constant(prg, "KEY_CURRENCYSUBUNIT", TGUIK_CURRENCYSUBUNIT, pass);
	insert_constant(prg, "KEY_KP_LEFTPAREN", TGUIK_KP_LEFTPAREN, pass);
	insert_constant(prg, "KEY_KP_RIGHTPAREN", TGUIK_KP_RIGHTPAREN, pass);
	insert_constant(prg, "KEY_KP_LEFTBRACE", TGUIK_KP_LEFTBRACE, pass);
	insert_constant(prg, "KEY_KP_RIGHTBRACE", TGUIK_KP_RIGHTBRACE, pass);
	insert_constant(prg, "KEY_KP_TAB", TGUIK_KP_TAB, pass);
	insert_constant(prg, "KEY_KP_BACKSPACE", TGUIK_KP_BACKSPACE, pass);
	insert_constant(prg, "KEY_KP_A", TGUIK_KP_A, pass);
	insert_constant(prg, "KEY_KP_B", TGUIK_KP_B, pass);
	insert_constant(prg, "KEY_KP_C", TGUIK_KP_C, pass);
	insert_constant(prg, "KEY_KP_D", TGUIK_KP_D, pass);
	insert_constant(prg, "KEY_KP_E", TGUIK_KP_E, pass);
	insert_constant(prg, "KEY_KP_F", TGUIK_KP_F, pass);
	insert_constant(prg, "KEY_KP_XOR", TGUIK_KP_XOR, pass);
	insert_constant(prg, "KEY_KP_POWER", TGUIK_KP_POWER, pass);
	insert_constant(prg, "KEY_KP_PERCENT", TGUIK_KP_PERCENT, pass);
	insert_constant(prg, "KEY_KP_LESS", TGUIK_KP_LESS, pass);
	insert_constant(prg, "KEY_KP_GREATER", TGUIK_KP_GREATER, pass);
	insert_constant(prg, "KEY_KP_AMPERSAND", TGUIK_KP_AMPERSAND, pass);
	insert_constant(prg, "KEY_KP_DBLAMPERSAND", TGUIK_KP_DBLAMPERSAND, pass);
	insert_constant(prg, "KEY_KP_VERTICALBAR", TGUIK_KP_VERTICALBAR, pass);
	insert_constant(prg, "KEY_KP_DBLVERTICALBAR", TGUIK_KP_DBLVERTICALBAR, pass);
	insert_constant(prg, "KEY_KP_COLON", TGUIK_KP_COLON, pass);
	insert_constant(prg, "KEY_KP_HASH", TGUIK_KP_HASH, pass);
	insert_constant(prg, "KEY_KP_SPACE", TGUIK_KP_SPACE, pass);
	insert_constant(prg, "KEY_KP_AT", TGUIK_KP_AT, pass);
	insert_constant(prg, "KEY_KP_EXCLAM", TGUIK_KP_EXCLAM, pass);
	insert_constant(prg, "KEY_KP_MEMSTORE", TGUIK_KP_MEMSTORE, pass);
	insert_constant(prg, "KEY_KP_MEMRECALL", TGUIK_KP_MEMRECALL, pass);
	insert_constant(prg, "KEY_KP_MEMCLEAR", TGUIK_KP_MEMCLEAR, pass);
	insert_constant(prg, "KEY_KP_MEMADD", TGUIK_KP_MEMADD, pass);
	insert_constant(prg, "KEY_KP_MEMSUBTRACT", TGUIK_KP_MEMSUBTRACT, pass);
	insert_constant(prg, "KEY_KP_MEMMULTIPLY", TGUIK_KP_MEMMULTIPLY, pass);
	insert_constant(prg, "KEY_KP_MEMDIVIDE", TGUIK_KP_MEMDIVIDE, pass);
	insert_constant(prg, "KEY_KP_PLUSMINUS", TGUIK_KP_PLUSMINUS, pass);
	insert_constant(prg, "KEY_KP_CLEAR", TGUIK_KP_CLEAR, pass);
	insert_constant(prg, "KEY_KP_CLEARENTRY", TGUIK_KP_CLEARENTRY, pass);
	insert_constant(prg, "KEY_KP_BINARY", TGUIK_KP_BINARY, pass);
	insert_constant(prg, "KEY_KP_OCTAL", TGUIK_KP_OCTAL, pass);
	insert_constant(prg, "KEY_KP_DECIMAL", TGUIK_KP_DECIMAL, pass);
	insert_constant(prg, "KEY_KP_HEXADECIMAL", TGUIK_KP_HEXADECIMAL, pass);
	insert_constant(prg, "KEY_LCTRL", TGUIK_LCTRL, pass);
	insert_constant(prg, "KEY_LSHIFT", TGUIK_LSHIFT, pass);
	insert_constant(prg, "KEY_LALT", TGUIK_LALT, pass);
	insert_constant(prg, "KEY_LGUI", TGUIK_LGUI, pass);
	insert_constant(prg, "KEY_RCTRL", TGUIK_RCTRL, pass);
	insert_constant(prg, "KEY_RSHIFT", TGUIK_RSHIFT, pass);
	insert_constant(prg, "KEY_RALT", TGUIK_RALT, pass);
	insert_constant(prg, "KEY_RGUI", TGUIK_RGUI, pass);
	insert_constant(prg, "KEY_MODE", TGUIK_MODE, pass);
	insert_constant(prg, "KEY_SLEEP", TGUIK_SLEEP, pass);
	insert_constant(prg, "KEY_WAKE", TGUIK_WAKE, pass);
	insert_constant(prg, "KEY_CHANNEL_INCREMENT", TGUIK_CHANNEL_INCREMENT, pass);
	insert_constant(prg, "KEY_CHANNEL_DECREMENT", TGUIK_CHANNEL_DECREMENT, pass);
	insert_constant(prg, "KEY_MEDIA_PLAY", TGUIK_MEDIA_PLAY, pass);
	insert_constant(prg, "KEY_MEDIA_PAUSE", TGUIK_MEDIA_PAUSE, pass);
	insert_constant(prg, "KEY_MEDIA_RECORD", TGUIK_MEDIA_RECORD, pass);
	insert_constant(prg, "KEY_MEDIA_FAST_FORWARD", TGUIK_MEDIA_FAST_FORWARD, pass);
	insert_constant(prg, "KEY_MEDIA_REWIND", TGUIK_MEDIA_REWIND, pass);
	insert_constant(prg, "KEY_MEDIA_NEXT_TRACK", TGUIK_MEDIA_NEXT_TRACK, pass);
	insert_constant(prg, "KEY_MEDIA_PREVIOUS_TRACK", TGUIK_MEDIA_PREVIOUS_TRACK, pass);
	insert_constant(prg, "KEY_MEDIA_STOP", TGUIK_MEDIA_STOP, pass);
	insert_constant(prg, "KEY_MEDIA_EJECT", TGUIK_MEDIA_EJECT, pass);
	insert_constant(prg, "KEY_MEDIA_PLAY_PAUSE", TGUIK_MEDIA_PLAY_PAUSE, pass);
	insert_constant(prg, "KEY_MEDIA_SELECT", TGUIK_MEDIA_SELECT, pass);
	insert_constant(prg, "KEY_AC_NEW", TGUIK_AC_NEW, pass);
	insert_constant(prg, "KEY_AC_OPEN", TGUIK_AC_OPEN, pass);
	insert_constant(prg, "KEY_AC_CLOSE", TGUIK_AC_CLOSE, pass);
	insert_constant(prg, "KEY_AC_EXIT", TGUIK_AC_EXIT, pass);
	insert_constant(prg, "KEY_AC_SAVE", TGUIK_AC_SAVE, pass);
	insert_constant(prg, "KEY_AC_PRINT", TGUIK_AC_PRINT, pass);
	insert_constant(prg, "KEY_AC_PROPERTIES", TGUIK_AC_PROPERTIES, pass);
	insert_constant(prg, "KEY_AC_SEARCH", TGUIK_AC_SEARCH, pass);
	insert_constant(prg, "KEY_AC_HOME", TGUIK_AC_HOME, pass);
	insert_constant(prg, "KEY_AC_BACK", TGUIK_AC_BACK, pass);
	insert_constant(prg, "KEY_AC_FORWARD", TGUIK_AC_FORWARD, pass);
	insert_constant(prg, "KEY_AC_STOP", TGUIK_AC_STOP, pass);
	insert_constant(prg, "KEY_AC_REFRESH", TGUIK_AC_REFRESH, pass);
	insert_constant(prg, "KEY_AC_BOOKMARKS", TGUIK_AC_BOOKMARKS, pass);
	insert_constant(prg, "KEY_SOFTLEFT", TGUIK_SOFTLEFT, pass);
	insert_constant(prg, "KEY_SOFTRIGHT", TGUIK_SOFTRIGHT, pass);
	insert_constant(prg, "KEY_CALL", TGUIK_CALL, pass);
	insert_constant(prg, "KEY_ENDCALL", TGUIK_ENDCALL, pass);
	insert_constant(prg, "KEY_LEFT_TAB", TGUIK_LEFT_TAB, pass);
	insert_constant(prg, "KEY_LEVEL5_SHIFT", TGUIK_LEVEL5_SHIFT, pass);
	insert_constant(prg, "KEY_MULTI_KEY_COMPOSE", TGUIK_MULTI_KEY_COMPOSE, pass);
	insert_constant(prg, "KEY_LMETA", TGUIK_LMETA, pass);
	insert_constant(prg, "KEY_RMETA", TGUIK_RMETA, pass);
	insert_constant(prg, "KEY_LHYPER", TGUIK_LHYPER, pass);
	insert_constant(prg, "KEY_RHYPER", TGUIK_RHYPER, pass);
	insert_constant(prg, "EVENT_KEY_DOWN", TGUI_KEY_DOWN, pass);
	insert_constant(prg, "EVENT_KEY_UP", TGUI_KEY_UP, pass);
	insert_constant(prg, "EVENT_JOY_DOWN", TGUI_JOY_DOWN, pass);
	insert_constant(prg, "EVENT_JOY_UP", TGUI_JOY_UP, pass);
	insert_constant(prg, "EVENT_JOY_AXIS", TGUI_JOY_AXIS, pass);
	insert_constant(prg, "EVENT_MOUSE_DOWN", TGUI_MOUSE_DOWN, pass);
	insert_constant(prg, "EVENT_MOUSE_UP", TGUI_MOUSE_UP, pass);
	insert_constant(prg, "EVENT_MOUSE_AXIS", TGUI_MOUSE_AXIS, pass);
	insert_constant(prg, "EVENT_MOUSE_WHEEL", TGUI_MOUSE_WHEEL, pass);
	insert_constant(prg, "EVENT_TEXT", TGUI_TEXT, pass);
	insert_constant(prg, "JOY_A", TGUI_B_A, pass);
	insert_constant(prg, "JOY_B", TGUI_B_B, pass);
	insert_constant(prg, "JOY_X", TGUI_B_X, pass);
	insert_constant(prg, "JOY_Y", TGUI_B_Y, pass);
	insert_constant(prg, "JOY_BACK", TGUI_B_BACK, pass);
	insert_constant(prg, "JOY_GUIDE", TGUI_B_GUIDE, pass);
	insert_constant(prg, "JOY_START", TGUI_B_START, pass);
	insert_constant(prg, "JOY_LS", TGUI_B_LS, pass);
	insert_constant(prg, "JOY_RS", TGUI_B_RS, pass);
	insert_constant(prg, "JOY_LB", TGUI_B_LB, pass);
	insert_constant(prg, "JOY_RB", TGUI_B_RB, pass);
	insert_constant(prg, "JOY_U", TGUI_B_U, pass);
	insert_constant(prg, "JOY_D", TGUI_B_D, pass);
	insert_constant(prg, "JOY_L", TGUI_B_L, pass);
	insert_constant(prg, "JOY_R", TGUI_B_R, pass);
	insert_constant(prg, "JOY_LEFTX", SDL_GAMEPAD_AXIS_LEFTX, pass);
	insert_constant(prg, "JOY_LEFTY", SDL_GAMEPAD_AXIS_LEFTY, pass);
	insert_constant(prg, "JOY_RIGHTX", SDL_GAMEPAD_AXIS_RIGHTX, pass);
	insert_constant(prg, "JOY_RIGHTY", SDL_GAMEPAD_AXIS_RIGHTY, pass);
	insert_constant(prg, "JOY_TRIGGERLEFT", SDL_GAMEPAD_AXIS_LEFT_TRIGGER, pass);
	insert_constant(prg, "JOY_TRIGGERRIGHT", SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, pass);
	insert_constant(prg, "TRANSITION_NONE", gui::GUI::TRANSITION_NONE, pass);
	insert_constant(prg, "TRANSITION_GROW", gui::GUI::TRANSITION_GROW, pass);
	insert_constant(prg, "TRANSITION_SHRINK", gui::GUI::TRANSITION_SHRINK, pass);
	insert_constant(prg, "TRANSITION_SLIDE", gui::GUI::TRANSITION_SLIDE, pass);
	insert_constant(prg, "TRANSITION_SLIDE_VERTICAL", gui::GUI::TRANSITION_SLIDE_VERTICAL, pass);
	insert_constant(prg, "TRANSITION_SLIDE_REVERSE", gui::GUI::TRANSITION_SLIDE_REVERSE, pass);
	insert_constant(prg, "TRANSITION_SLIDE_VERTICAL_REVERSE", gui::GUI::TRANSITION_SLIDE_VERTICAL_REVERSE, pass);
	insert_constant(prg, "GUI_CENTRE", GUI_CENTRE, pass);
	insert_constant(prg, "GUI_LEFT", GUI_LEFT, pass);
	insert_constant(prg, "GUI_RIGHT", GUI_RIGHT, pass);
	insert_constant(prg, "GUI_TOP", GUI_LEFT, pass);
	insert_constant(prg, "GUI_BOTTOM", GUI_BOTTOM, pass);
	insert_constant(prg, "LETTERBOX_TOP", gfx::LETTERBOX_TOP, pass);
	insert_constant(prg, "LETTERBOX_BOTTOM", gfx::LETTERBOX_BOTTOM, pass);
	insert_constant(prg, "LETTERBOX_LEFT", gfx::LETTERBOX_LEFT, pass);
	insert_constant(prg, "LETTERBOX_RIGHT", gfx::LETTERBOX_RIGHT, pass);
	insert_constant(prg, "BLEND_ZERO", gfx::BLEND_ZERO, pass);
	insert_constant(prg, "BLEND_ONE", gfx::BLEND_ONE, pass);
	insert_constant(prg, "BLEND_SRCCOLOR", gfx::BLEND_SRCCOLOR, pass);
	insert_constant(prg, "BLEND_INVSRCCOLOR", gfx::BLEND_INVSRCCOLOR, pass);
	insert_constant(prg, "BLEND_SRCALPHA", gfx::BLEND_SRCALPHA, pass);
	insert_constant(prg, "BLEND_INVSRCALPHA", gfx::BLEND_INVSRCALPHA, pass);
	insert_constant(prg, "COMPARE_NEVER", gfx::COMPARE_NEVER, pass);
	insert_constant(prg, "COMPARE_LESS", gfx::COMPARE_LESS, pass);
	insert_constant(prg, "COMPARE_EQUAL", gfx::COMPARE_EQUAL, pass);
	insert_constant(prg, "COMPARE_LESSEQUAL", gfx::COMPARE_LESSEQUAL, pass);
	insert_constant(prg, "COMPARE_GREATER", gfx::COMPARE_GREATER, pass);
	insert_constant(prg, "COMPARE_NOTEQUAL", gfx::COMPARE_NOTEQUAL, pass);
	insert_constant(prg, "COMPARE_GREATEREQUAL", gfx::COMPARE_GREATEREQUAL, pass);
	insert_constant(prg, "COMPARE_ALWAYS", gfx::COMPARE_ALWAYS, pass);
	insert_constant(prg, "STENCILOP_KEEP", gfx::STENCILOP_KEEP, pass);
	insert_constant(prg, "STENCILOP_ZERO", gfx::STENCILOP_ZERO, pass);
	insert_constant(prg, "STENCILOP_REPLACE", gfx::STENCILOP_REPLACE, pass);
	insert_constant(prg, "STENCILOP_INCRSAT", gfx::STENCILOP_INCRSAT, pass);
	insert_constant(prg, "STENCILOP_DECRSAT", gfx::STENCILOP_DECRSAT, pass);
	insert_constant(prg, "STENCILOP_INVERT", gfx::STENCILOP_INVERT, pass);
	insert_constant(prg, "STENCILOP_INCR", gfx::STENCILOP_INCR, pass);
	insert_constant(prg, "STENCILOP_DECR", gfx::STENCILOP_DECR, pass);
	insert_constant(prg, "NO_FACE", gfx::NO_FACE, pass);
	insert_constant(prg, "FRONT_FACE", gfx::FRONT_FACE, pass);
	insert_constant(prg, "BACK_FACE", gfx::BACK_FACE, pass);
	insert_constant(prg, "FACE_CW", gfx::FACE_CW, pass);
	insert_constant(prg, "FACE_CCW", gfx::FACE_CCW, pass);
	insert_constant(prg, "SEEK_SET", SDL_IO_SEEK_SET, pass);
	insert_constant(prg, "SEEK_CUR", SDL_IO_SEEK_CUR, pass);
	insert_constant(prg, "SEEK_END", SDL_IO_SEEK_END, pass);
	insert_constant(prg, "STDIN", 0, pass);
	insert_constant(prg, "STDOUT", 1, pass);
	insert_constant(prg, "STDERR", 2, pass);
	insert_constant(prg, "F12_START", F12_START, pass);
	insert_constant(prg, "F12_END", F12_END, pass);

	for (int i = 0; i < 100; i++) {
		std::string name = "__tmp" + util::itos(i);
		int var_index = prg->var_i;
		prg->var_i++;
		if (pass == PASS2) {
			prg->variables_map[name] = var_index;
		}
		Variable v;
		v.name = name;
		v.set_type(Variable::UNTYPED);
		if (pass == PASS1) {
			prg->variables.push_back(v);
		}
	}

	if (prg->num_consts == -1) {
		prg->num_consts = prg->variables.size();
	}

	int _is_deref = 0;

	while ((tok = token(prg, tt)) != "") {
top:
		if (tok == "`") {
			_is_deref++;
		}
		else if (tok == "function") {
			std::string func_name = token(prg, tt);

			int func_index = prg->func_i;
			if (pass == PASS2) {
				backup(prg, func_index, false);
			}

			std::string nm = func_name;

			if (pass == PASS1) {
				prg->function_names.push_back(nm);
			}

			Variable v;
			v.name = func_name;
			v.set_type(Variable::FUNCTION);
			v.set_n(prg->func_i++);
			v.constant = true;
			std::map<std::string, int> tmp;
			prg->locals.push_back(tmp);
			prg->variables_map[func_name] = prg->var_i++;
			if (pass == PASS1) {
				prg->variables.push_back(v);
			}

			Program func;
			func.s = new Function_Swap;
			func.s->name = func_name;
			func.s->pc = 0;
			func.s->line = prg->s->line;
			func.real_line_numbers = prg->real_line_numbers;
			func.real_file_names = prg->real_file_names;
			func.complete_pass = prg->complete_pass;
			bool is_param = true;
			bool finished = false;
			bool param_is_ref = false;
			bool param_is_const = false;
			int is_deref = 0;
			while ((tok = token(prg, tt)) != "") {
func_top:
				func.s->line = prg->s->line;
				if (tok == "{") {
					is_param = false;
					func.s->start_line = prg->s->line;
				}
				else if (tok == "}") {
					finished = true;
					break;
				}
				else if (is_param) {
					if (tok == "~") {
						param_is_ref = true;
					}
					else if (tok == "const") {
						param_is_const = true;
					}
					else {
						int param_i = prg->var_i++;
						if (pass == PASS1) {
							prg->locals[func_index][tok] = param_i;
						}
						else {
							prg->variables_map[tok] = prg->locals[func_index][tok];
						}
						Variable v;
						v.name = tok;
						v.constant = param_is_const;
						if (pass == PASS1) {
							prg->variables.push_back(v);
						}

						func.params.push_back(param_i);
						func.param_names.push_back(tok);
						func.ref.push_back(param_is_ref);
						param_is_ref = false;
						param_is_const = false;
						is_deref = 0;
					}
				}
				else if (tok == "`") {
					is_deref++;
				}
				else if (tok == ":") {
					std::string tok2 = token(prg, tt);

					Variable v;
					v.name = tok2;
					v.set_type(Variable::LABEL);
					v.set_n(func.s->program.size());
					v.constant = true;

					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					Token t;
					t.type = Token::SYMBOL;
					t.i = prg->var_i;
					t.s = tok2;
					t.dereference = 0;
					is_deref = 0;
					func.s->program[func.s->program.size()-1].data.push_back(t);

					if (pass == PASS1) {
						if (prg->locals[func_index].find(tok2) != prg->locals[func_index].end()) {
							my_throw(Error(std::string(__FUNCTION__) + ": " + "Duplicate label " + tok2 + " at " + get_error_info(&func)));
						}
						prg->locals[func_index][tok2] = prg->var_i;
					}
					prg->variables.push_back(v);
					if (pass == PASS2) {
						prg->variables_map[tok2] = prg->locals[func_index][tok2];
					}
					prg->var_i++;
				}
				else if (tok == "const") {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					int count = 0;
					while (true) {
						std::string tok2 = token(prg, tt);
						if (tok2 == "" || (count != 0 && (tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
							tok = tok2;
							goto func_top;
						}
						if (tok2[0] != '_' && !isalpha(tok2[0])) {
							my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok2 + " at " + get_error_info(&func)));
						}
						count++;
						if (pass == PASS1) {
							prg->locals[func_index][tok2] = prg->var_i++;
						}
						else {
							prg->variables_map[tok2] = prg->locals[func_index][tok2];
							prg->var_i++;
						}
						Variable v;
						v.name = tok2;
						v.set_type(Variable::UNTYPED);
						v.constant = true;
						std::map<std::string, int>::iterator it;
						it = prg->variables_map.find(tok2);
						if (it != prg->variables_map.end()) {
							Variable &var = prg->variables[(*it).second];
							if (var.get_type() != v.get_type()) {
								my_throw(Error("Type for " + tok2 + " changed at " + get_error_info(&func)));
							}
						}
						if (pass == PASS1) {
							prg->variables.push_back(v);
						}
						Token t;
						t.type = Token::SYMBOL;
						if (pass == PASS2) {
							t.i = prg->variables_map[tok2];
						}
						t.s = tok2;
						t.dereference = 0;
						is_deref = 0;
						func.s->program[func.s->program.size()-1].data.push_back(t);
						std::string valtok = token(prg, tt);
						if (valtok[0] == '(') {
							Variable v;
							v.name = "__e" + util::itos(prg->expression_i++);
							v.set_type(Variable::EXPRESSION);

							int var_index = prg->var_i;
							prg->var_i++;

							if (pass == PASS1) {
								prg->locals[func_index][v.name] = var_index;
								prg->variables.push_back(v);
							}
							else {
								prg->variables_map[v.name] = prg->locals[func_index][v.name];
							}

							prg->variables[var_index].e = parse_expression(prg, &func, valtok, pass);

							Token t;
							t.type = Token::SYMBOL;
							t.s = v.name;
							if (pass == PASS2) {
								t.i = prg->variables_map[v.name];
							}
							t.dereference = is_deref;
							is_deref = 0;

							func.s->program[func.s->program.size()-1].data.push_back(t);
						}
						else if (valtok[0] == '[') {
							Variable v;
							v.name = "__f" + util::itos(prg->fish_i++);
							v.set_type(Variable::FISH);

							int var_index = prg->var_i;
							prg->var_i++;

							if (pass == PASS1) {
								prg->locals[func_index][v.name] = var_index;
								prg->variables.push_back(v);
							}
							else {
								prg->variables_map[v.name] = prg->locals[func_index][v.name];
							}

							prg->variables[var_index].f = parse_fish(prg, &func, valtok, pass);

							Token t;
							t.type = Token::SYMBOL;
							t.s = v.name;
							if (pass == PASS2) {
								t.i = prg->variables_map[v.name];
							}
							t.dereference = is_deref;
							is_deref = 0;

							func.s->program[func.s->program.size()-1].data.push_back(t);
						}
						else {
							Token t2;
							t2.type = tt;
							if (pass == PASS2 && tt == Token::SYMBOL) {
								t2.i = prg->variables_map[valtok];
							}
							else if (tt == Token::STRING) {
								t2.s = valtok;
							}
							else if (tt == Token::NUMBER) {
								t2.n = atof(valtok.c_str());
							}
							t2.s = valtok;
							t2.n = atof(valtok.c_str());
							t2.dereference = 0;
							is_deref = 0;
							func.s->program[func.s->program.size()-1].data.push_back(t2);
						}
					}
				}
				else if (tok == "var") {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					int count = 0;
					while (true) {
						std::string tok2 = token(prg, tt);
						if (tok2 == "" || (count != 0 && (tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
							tok = tok2;
							goto func_top;
						}
						if (tok2[0] != '_' && !isalpha(tok2[0])) {
							my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok2 + " at " + get_error_info(&func)));
						}
						count++;
						if (pass == PASS1) {
							prg->locals[func_index][tok2] = prg->var_i++;
						}
						else {
							prg->variables_map[tok2] = prg->locals[func_index][tok2];
							prg->var_i++;
						}
						Variable v;
						v.name = tok2;
						v.set_type(Variable::UNTYPED);
						std::map<std::string, int>::iterator it;
						it = prg->variables_map.find(tok2);
						if (it != prg->variables_map.end()) {
							Variable &var = prg->variables[(*it).second];
							if (var.get_type() != v.get_type()) {
								my_throw(Error("Type for " + tok2 + " changed at " + get_error_info(&func)));
							}
						}
						if (pass == PASS1) {
							prg->variables.push_back(v);
						}
						Token t;
						t.type = Token::SYMBOL;
						if (pass == PASS2) {
							t.i = prg->variables_map[tok2];
						}
						t.s = tok2;
						t.dereference = 0;
						is_deref = 0;
						func.s->program[func.s->program.size()-1].data.push_back(t);
					}
				}
				else if (tok[0] == '(') {
					Variable v;
					v.name = "__e" + util::itos(prg->expression_i++);
					v.set_type(Variable::EXPRESSION);

					int var_index = prg->var_i;
					prg->var_i++;

					if (pass == PASS1) {
						prg->locals[func_index][v.name] = var_index;
						prg->variables.push_back(v);
					}
					else {
						prg->variables_map[v.name] = prg->locals[func_index][v.name];
					}

					prg->variables[var_index].e = parse_expression(prg, &func, tok, pass);

					Token t;
					t.type = Token::SYMBOL;
					t.s = v.name;
					if (pass == PASS2) {
						t.i = prg->variables_map[v.name];
					}
					t.dereference = is_deref;
					is_deref = 0;

					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
				else if (tok[0] == '[') {
					Variable v;
					v.name = "__f" + util::itos(prg->fish_i++);
					v.set_type(Variable::FISH);

					int var_index = prg->var_i;
					prg->var_i++;

					if (pass == PASS1) {
						prg->locals[func_index][v.name] = var_index;
						prg->variables.push_back(v);
					}
					else {
						prg->variables_map[v.name] = prg->locals[func_index][v.name];
					}

					prg->variables[var_index].f = parse_fish(prg, &func, tok, pass);

					Token t;
					t.type = Token::SYMBOL;
					t.s = v.name;
					if (pass == PASS2) {
						t.i = prg->variables_map[v.name];
					}
					t.dereference = is_deref;
					is_deref = 0;

					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
				else if (library_map.find(tok) != library_map.end()) {
					std::string b = get_error_info(prg);
					prg->lines_with_instructions[b] = true;
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					is_deref = 0;
				}
				else {
					if (func.s->program.size() == 0) {
						my_throw(Error("Expected keyword at " + get_error_info(&func)));
					}
					Token t;
					t.type = tt;
					switch (tt) {
						case Token::STRING:
							t.s = util::remove_quotes(util::unescape_string(tok));
							break;
						case Token::SYMBOL:
							t.s = util::remove_quotes(util::unescape_string(tok));
							if (pass == PASS2 && prg->variables_map.find(t.s) == prg->variables_map.end()) {
								my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid symbol name " + tok + " at " + get_error_info(&func)));
							}
							if (pass == PASS2) {
								t.i = prg->variables_map[t.s];
							}
							break;
						case Token::NUMBER:
							t.n = atof(tok.c_str());
							break;
					}
					t.dereference = is_deref;
					is_deref = 0;
					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
			}

			if (is_param == true) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Missing { at " + get_error_info(prg)));
			}

			if (finished == false) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Missing } at " + get_error_info(prg)));
			}

			prg->function_name_map[func.s->name] = prg->functions.size();
			func.complete_pass = pass;

			prg->functions.push_back(func);

			std::map<std::string, int>::iterator it;

			if (pass == PASS2) {
				restore(prg, func_index);
			}
			else if (pass == PASS1) {
			}
		}
		else if (tok == ":") {
			std::string tok2 = token(prg, tt);

			Variable v;
			v.name = tok2;
			v.set_type(Variable::LABEL);
			v.set_n(prg->s->program.size());
			v.constant = true;

			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);

			Token t;
			t.type = Token::SYMBOL;
			t.i = prg->var_i;
			t.s = tok2;
			t.dereference = 0;
			_is_deref = 0;
			prg->s->program[prg->s->program.size()-1].data.push_back(t);

			if (pass == PASS1 && prg->variables_map.find(tok2) != prg->variables_map.end()) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Duplicate label " + tok2 + " at " + get_error_info(prg)));
			}

			prg->s->line_numbers.push_back(prg->s->line);
			prg->variables.push_back(v);
			prg->variables_map[tok2] = prg->var_i;
			prg->var_i++;
		}
		else if (tok == "const") {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			prg->s->line_numbers.push_back(prg->s->line);
			int count = 0;
			while (true) {
				std::string tok2 = token(prg, tt);
				if (tok2 == "" || (count != 0 && (tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
					tok = tok2;
					goto top;
				}
				if (tok2[0] != '_' && !isalpha(tok2[0])) {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok2 + " at " + get_error_info(prg)));
				}
				count++;
				int var_index = prg->var_i;
				prg->var_i++;
				if (pass == PASS2) {
					prg->variables_map[tok2] = var_index;
				}
				Variable v;
				v.name = tok2;
				v.set_type(Variable::UNTYPED);
				v.constant = true;
				if (pass == PASS1) {
					prg->variables.push_back(v);
				}
				Token t;
				t.type = Token::SYMBOL;
				if (pass == PASS2) {
					t.i = prg->variables_map[tok2];
				}
				t.s = tok2;
				t.dereference = 0;
				_is_deref = 0;
				prg->s->program[prg->s->program.size()-1].data.push_back(t);
				std::string valtok = token(prg, tt);
				if (valtok[0] == '(') {
					Variable v;
					v.name = "__e" + util::itos(prg->expression_i++);
					v.set_type(Variable::EXPRESSION);

					int var_index = prg->var_i;
					prg->var_i++;

					if (pass == PASS1) {
						prg->variables.push_back(v);
					}
					else if (pass == PASS2) {
						prg->variables_map[v.name] = var_index;
					}
					prg->variables[var_index].e = parse_expression(prg, prg, valtok, pass);

					Token t;
					t.type = Token::SYMBOL;
					t.s = v.name;
					if (pass == PASS2) {
						t.i = prg->variables_map[v.name];
					}
					t.dereference = _is_deref;
					_is_deref = 0;

					prg->s->program[prg->s->program.size()-1].data.push_back(t);
				}
				else if (valtok[0] == '[') {
					Variable v;
					v.name = "__f" + util::itos(prg->fish_i++);
					v.set_type(Variable::FISH);

					int var_index = prg->var_i;
					prg->var_i++;

					if (pass == PASS1) {
						prg->variables.push_back(v);
					}
					else if (pass == PASS2) {
						prg->variables_map[v.name] = var_index;
					}
					prg->variables[var_index].f = parse_fish(prg, prg, valtok, pass);

					Token t;
					t.type = Token::SYMBOL;
					t.s = v.name;
					if (pass == PASS2) {
						t.i = prg->variables_map[v.name];
					}
					t.dereference = _is_deref;
					_is_deref = 0;

					prg->s->program[prg->s->program.size()-1].data.push_back(t);
				}
				else {
					Token t2;
					t2.type = tt;
					if (pass == PASS2 && tt == Token::SYMBOL) {
						t2.i = prg->variables_map[valtok];
					}
					else if (tt == Token::STRING) {
						t2.s = valtok;
					}
					else if (tt == Token::NUMBER) {
						t2.n = atof(valtok.c_str());
					}
					t2.dereference = 0;
					prg->s->program[prg->s->program.size()-1].data.push_back(t2);
				}
			}
		}
		else if (tok == "var") {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			prg->s->line_numbers.push_back(prg->s->line);
			int count = 0;
			while (true) {
				std::string tok2 = token(prg, tt);
				if (tok2 == "" || (count != 0 && (tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
					tok = tok2;
					goto top;
				}
				if (tok2[0] != '_' && !isalpha(tok2[0])) {
					my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok2 + " at " + get_error_info(prg)));
				}
				count++;
				int var_index = prg->var_i;
				prg->var_i++;
				if (pass == PASS2) {
					prg->variables_map[tok2] = var_index;
				}
				Variable v;
				v.name = tok2;
				v.set_type(Variable::UNTYPED);
				if (pass == PASS1) {
					prg->variables.push_back(v);
				}
				Token t;
				t.type = Token::SYMBOL;
				if (pass == PASS2) {
					t.i = prg->variables_map[tok2];
				}
				t.s = tok2;
				t.dereference = 0;
				_is_deref = 0;
				prg->s->program[prg->s->program.size()-1].data.push_back(t);
			}
		}
		else if (tok[0] == '(') {
			Variable v;
			v.name = "__e" + util::itos(prg->expression_i++);
			v.set_type(Variable::EXPRESSION);

			int var_index = prg->var_i;
			prg->var_i++;

			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
			else if (pass == PASS2) {
				prg->variables_map[v.name] = var_index;
			}
			prg->variables[var_index].e = parse_expression(prg, prg, tok, pass);

			Token t;
			t.type = Token::SYMBOL;
			t.s = v.name;
			if (pass == PASS2) {
				t.i = prg->variables_map[v.name];
			}
			t.dereference = _is_deref;
			_is_deref = 0;

			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
		else if (tok[0] == '[') {
			Variable v;
			v.name = "__f" + util::itos(prg->fish_i++);
			v.set_type(Variable::FISH);

			int var_index = prg->var_i;
			prg->var_i++;

			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
			else if (pass == PASS2) {
				prg->variables_map[v.name] = var_index;
			}
			prg->variables[var_index].f = parse_fish(prg, prg, tok, pass);

			Token t;
			t.type = Token::SYMBOL;
			t.s = v.name;
			if (pass == PASS2) {
				t.i = prg->variables_map[v.name];
			}
			t.dereference = _is_deref;
			_is_deref = 0;

			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
		else if (library_map.find(tok) != library_map.end()) {
			std::string b = get_error_info(prg);
			prg->lines_with_instructions[b] = true;
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			prg->s->line_numbers.push_back(prg->s->line);
			_is_deref = 0;
		}
		else if (prg->s->program.size() == 0) {
			my_throw(Error("Expected keyword at " + get_error_info(prg)));
		}
		else {
			Token t;
			t.type = tt;
			switch (tt) {
				case Token::STRING:
					t.s = util::remove_quotes(util::unescape_string(tok));
					break;
				case Token::SYMBOL:
					t.s = util::remove_quotes(util::unescape_string(tok));
					if (pass == PASS2 && prg->variables_map.find(t.s) == prg->variables_map.end()) {
						my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid symbol name " + tok + " at " + get_error_info(prg)));
					}
					if (pass == PASS2) {
						t.i = prg->variables_map[t.s];
					}
					break;
				case Token::NUMBER:
					t.n = atof(tok.c_str());
					break;
			}
			t.dereference = _is_deref;
			_is_deref = 0;
			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
	}

	prg->s->p = p_bak;
	prg->s->line = line_bak;

	prg->complete_pass = pass;
}

void call_function(Program *prg, int function, const std::vector<Token> &params, int ignore_params)
{
	bool bt = true;
	if (shim::debug == false || get_file_name(prg) == "UNKNOWN") {
		bt = false;
	}
	if (bt) {
		backtrace.push_back(get_file_name(prg) + ":" + prg->s->name + ":" + util::itos(get_line_num(prg)));
	}

	// locals backup supports recursion, and only gets done if this is recursion
	bool backup_locals = false;
	std::vector<Variable> locals_backup;
	std::vector<int> locals_backup_i;

	std::map<std::string, int>::iterator it;
	if (prg_func == &prg->functions[function]) {
		backup_locals = true;
	}

	if (backup_locals) {
		std::map<std::string, int>::iterator it;
		for (it = prg->locals[function].begin(); it != prg->locals[function].end(); it++) {
			std::pair<std::string, int> pair = *it;
			if (prg->variables[pair.second].constant == false) {
				locals_backup.push_back(prg->variables[pair.second]);
				locals_backup_i.push_back(pair.second);
			}
		}
	}

	Program &func = prg->functions[function];
	Program *prg_bak = prg_func;
	prg_func = &func;

	var_args.push(params);
	num_var_args_args.push(func.params.size()+ignore_params);

	for (size_t j = 0; j < func.params.size(); j++) {
		const Token &param = params[j+ignore_params];

		Variable &var = prg->variables[func.params[j]];

		bool constant = var.constant;
		var.constant = false;

		if (param.type == Token::NUMBER) {
			var.set_type(Variable::NUMBER);
			var.set_n(param.n);
		}
		else if (param.type == Token::STRING) {
			var.set_type(Variable::STRING);
			var.set_s(param.s);
		}
		else if (prg->variables[param.i].get_type() == Variable::EXPRESSION) {
			Variable *tmp = prg->result;
			prg->result = &var;
			evaluate_expression(prg, prg->variables[param.i].e);
			prg->result = tmp;
		}
		else if (prg->variables[param.i].get_type() == Variable::FISH) {
			var.set(go_fish(prg, prg->variables[param.i].f));
		}
		else {
			var.set(prg->variables[param.i]);
		}

		var.constant = constant;
	}

	std::string bak = prg->result->name;

	Function_Swap *bak2 = prg->s;
	prg->s = func.s;

	int pc_bak = prg->s->pc; // To handle recursive calls
	int c_bak = prg->compare_flag;

	prg->s->pc = 0;

	if (shim::debug) {
		for (size_t i = 0; i < function_breakpoints.size(); i++) {
			if (function_breakpoints[i] == prg->s->name) {
				debug("Breakpoint (" + prg->s->name + ") hit...");
				break;
			}
		}
	}

	while (interpret(prg)) {
	}

	if (bt) {
		backtrace.pop_back();
	}

	prg->s->pc = pc_bak;
	prg->compare_flag = c_bak;

	prg->result->name = bak;
	
	var_args.pop();
	num_var_args_args.pop();

	// keep values for references
	for (size_t j = 0; j < func.params.size(); j++) {
		const Token &param = params[j+ignore_params];
		
		Variable &var = prg->variables[func.params[j]];

		if (func.ref[j] && param.type != Token::NUMBER && param.type != Token::STRING && prg->variables[param.i].get_type() != Variable::EXPRESSION) {
			if (prg->variables[param.i].get_type() == Variable::FISH) {
				Variable &v2 = go_fish(prg, prg->variables[param.i].f);
			       	v2.set(var);
			}
			else {
				Variable &v2 = prg->variables[param.i];
				v2.set(var);
			}
		}
	}

	if (backup_locals) {
		int i = 0;
		for (size_t i = 0; i < locals_backup_i.size(); i++) {
			prg->variables[locals_backup_i[i]].set(locals_backup[i]);
		}
	}

	prg->s = bak2;

	prg_func = prg_bak;
}

void call_function(Program *prg, std::string function_name, const std::vector<Token> &params, int ignore_params)
{
	std::map<std::string, int>::iterator it = prg->function_name_map.find(function_name);
	if (it == prg->function_name_map.end()) {
		return;
	}
	call_function(prg, (*it).second, params, ignore_params);
}

bool interpret(Program *prg, bool trigger_breakpoints)
{
	if (prg->s->pc >= prg->s->program.size()) {
		return false;
	}

	if (shim::debug && trigger_breakpoints) {
		std::string inf = get_error_info(prg);
		for (size_t i = 0; i < file_breakpoints.size(); i++) {
			if (file_breakpoints[i] == inf) {
				debug("Breakpoint " + inf + " hit...");
				break;
			}
		}
	}

	if (trigger_breakpoints && break_on_interpret) {
		debug("Stepping into " + prg->s->name + "...");
	}

	if (prg->s->pc >= prg->s->program.size()) {
		return false;
	}

	Statement &s = prg->s->program[prg->s->pc];

	unsigned int pc_bak = prg->s->pc;

	library_func func = library[s.method];
	bool ret = func(prg, s.data);

	if (pc_bak == prg->s->pc) {
		prg->s->pc++;
	}

	if (prg->s->pc >= prg->s->program.size()) { // this is actually needed in 3 places
		return false;
	}

	return ret;
}

void destroy_program(Program *prg)
{
	for (size_t i = 0; i < prg->functions.size(); i++) {
		delete prg->functions[i].s;
	}

	prg->variables.clear();
	prg->functions.clear();

	delete prg->main_s;
	delete prg->result;
	black_box.clear();
	delete prg;
}

void add_instruction(std::string name, library_func processing)
{
	library_map[name] = library.size();
	library.push_back(processing);
}

void add_token_handler(char token, token_func func)
{
	token_map[token] = func;
}

void add_expression_handler(std::string name, expression_func func)
{
	expression_map[name] = expression_handlers.size();
	expression_handlers.push_back(func);
}

static bool breaker_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	reset_game_name = as_string(prg, v, 0);

	return false;
}

static bool breaker_exit(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	return_code = as_number(prg, v, 0);
	reset_game_name = "";
	quit = true;
	return false;
}

static bool breaker_return(Program *prg, const std::vector<Token> &v)
{
	if (v.size() > 0) {
		if (v[0].type == Token::NUMBER) {
			prg->result->set_type(Variable::NUMBER);
			prg->result->set_n(v[0].n);
		}
		else if (v[0].type == Token::STRING) {
			prg->result->set_type(Variable::STRING);
			prg->result->set_s(v[0].s);
		}
		else {
			prg->result->set(as_variable_resolve(prg, v, 0));
		}
	}

	return false;
}

static bool do_set(Program *prg, const std::vector<Token> &v, bool const_ok)
{
	COUNT_ARGS(2)

	Variable *v1 = as_variable_pointer(prg, v, 0);

	if (const_ok == false && v1->constant == true) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Attempt to set constant at " + get_error_info(prg)));
	}

	if (v[1].type == Token::NUMBER) {
		v1->set_type(Variable::NUMBER);
		v1->set_n(as_number(prg, v, 1));
	}
	else if (v[1].type == Token::STRING) {
		v1->set_type(Variable::STRING);
		v1->set_s(as_string(prg, v, 1));
	}
	else {
		Variable *v2 = as_variable_pointer(prg, v, 1);

		std::string name = v1->name;
		bool constant = v1->constant;
		v1->set(*v2);
		v1->name = name;
		v1->constant = constant;
	}

	return true;
}

static bool corefunc_set(Program *prg, const std::vector<Token> &v)
{
	return do_set(prg, v, false);
}

static void exprfunc_set(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable *dst = as_variable_pointer(prg, v, 0);
	dst->set(*as_variable_pointer(prg, v, 1));
	prg->result->set(*dst);
}

static bool corefunc_break(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)
	prg->break_flag = true;
	return true;
}

static bool corefunc_continue(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)
	prg->continue_flag = true;
	return true;
}

static bool corefunc_var(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)
	// Needed, redeclaring should start fresh
	for (size_t i = 0; i < v.size(); i++) {
		prg->variables[v[i].i].v.clear();
		prg->variables[v[i].i].m.clear();
	}
	return true;
}

static bool corefunc_const(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)
	if (v.size() % 2 != 0) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Incorrect number of arguments to const at " + get_error_info(prg)));
	}
	for (size_t i = 0; i < v.size(); i += 2) {
		std::vector<Token> toks;
		toks.push_back(v[i]);
		toks.push_back(v[i+1]);
		prg->variables[v[i].i].constant = false;
		do_set(prg, toks, true);
		prg->variables[v[i].i].constant = true;
	}
	return true;
}

static bool corefunc_label(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	return true;
}

static bool corefunc_goto(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	prg->s->pc = as_label(prg, v, 0);

	return true;
}

static bool corefunc_compare(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	bool is_num = false;
	double n;

	if (v[0].type == Token::SYMBOL) {
		Variable *var = as_variable_pointer(prg, v, 0);
		if (var->get_type() == Variable::NUMBER) {
			is_num = true;
			n = var->get_n();
		}
	}
	else if (v[0].type == Token::NUMBER) {
		is_num = true;
		n = v[0].n;
	}

	if (is_num) {
		double n2 = as_number(prg, v, 1);
		if (n < n2) {
			prg->compare_flag = -1;
		}
		else if (n == n2) {
			prg->compare_flag = 0;
		}
		else {
			prg->compare_flag = 1;
		}
	}
	else {
		bool a_string = false;
		bool b_string = false;
		std::string s1;
		std::string s2;

		if (v[0].type == Token::STRING) {
			a_string = true;
			s1 = v[0].s;
		}
		else if (v[0].type == Token::SYMBOL) {
			Variable *var = as_variable_pointer(prg, v, 0);
			if (var->get_type() == Variable::STRING) {
				a_string = true;
				s1 = var->get_s();
			}
		}
		
		if (v[1].type == Token::STRING) {
			b_string = true;
			s2 = v[1].s;
		}
		else if (v[1].type == Token::SYMBOL) {
			Variable *var = as_variable_pointer(prg, v, 1);
			if (var->get_type() == Variable::STRING) {
				b_string = true;
				s2 = var->get_s();
			}
		}
		
		if (a_string && b_string) {
			prg->compare_flag = strcmp(s1.c_str(), s2.c_str());
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
		}
	}

	return true;
}

static bool corefunc_je(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag == 0) {
		prg->s->pc = as_label(prg, v, 0);
	}

	return true;
}

static bool corefunc_jne(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag != 0) {
		prg->s->pc = as_label(prg, v, 0);
	}

	return true;
}

static bool corefunc_jl(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag < 0) {
		prg->s->pc = as_label(prg, v, 0);
	}

	return true;
}

static bool corefunc_jle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag <= 0) {
		prg->s->pc = as_label(prg, v, 0);
	}

	return true;
}

static bool corefunc_jg(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag > 0) {
		prg->s->pc = as_label(prg, v, 0);
	}

	return true;
}

static bool corefunc_jge(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag >= 0) {
		prg->s->pc = as_label(prg, v, 0);
	}

	return true;
}

static bool corefunc_call(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int function = as_function(prg, v, 0);

	call_function(prg, function, v, 1);

	return true;
}

static bool corefunc_call_result(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &result = prg->variables[v[0].i];
	int function = as_function(prg, v, 1);

	call_function(prg, function, v, 2);

	result.set(*prg->result);

	return true;
}

std::string typeof_var(Variable *v1)
{
	std::string res;
	if (v1->get_type() == Variable::NUMBER) {
		res = "number";
	}
	else if (v1->get_type() == Variable::STRING) {
		res = "string";
	}
	else if (v1->get_type() == Variable::VECTOR) {
		res = "vector";
	}
	else if (v1->get_type() == Variable::MAP) {
		res = "map";
	}
	else if (v1->get_type() == Variable::FUNCTION) {
		res = "function";
	}
	else if (v1->get_type() == Variable::LABEL) {
		res = "label";
	}
	else if (v1->get_type() == Variable::POINTER) {
		res = "pointer";
	}
	else if (v1->get_type() == Variable::EXPRESSION) {
		res = "expression";
	}
	else if (v1->get_type() == Variable::FISH) {
		res = "fish";
	}
	else if (v1->get_type() == Variable::USER) {
		res = "user";
	}
	else {
		res = "unknown";
	}

	return res;
}

static void exprfunc_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string s = as_string(prg, v, 0);

	prg->result->set_type(Variable::STRING);
	prg->result->set_s(s);
}

static void exprfunc_vector(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable *var = as_variable_pointer(prg, v, 0);

	prg->result->set_type(Variable::VECTOR);
	prg->result->v = var->v;
}

static void exprfunc_map(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable *var = as_variable_pointer(prg, v, 0);

	prg->result->set_type(Variable::MAP);
	prg->result->m = var->m;
}

static void exprfunc_function(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int n = as_function(prg, v, 0);

	prg->result->set_type(Variable::FUNCTION);
	prg->result->set_n(n);
}

static void exprfunc_label(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)
	
	int n = as_label(prg, v, 0);

	prg->result->set_type(Variable::LABEL);
	prg->result->set_n(n);
}

static void exprfunc_pointer(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable *var = as_variable_pointer(prg, v, 0);

	prg->result->set_type(Variable::POINTER);
	prg->result->set_p(var);
}

static void exprfunc_typeof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)
	
	if (v[0].type != Token::SYMBOL) {
		prg->result->set_type(Variable::STRING);
		if (v[0].type == Token::NUMBER) {
			prg->result->set_s("number");
		}
		else if (v[0].type == Token::STRING) {
			prg->result->set_s("string");
		}
		else {
			prg->result->set_s("unknown");
		}
	}
	else {
		Variable *v1 = as_variable_pointer(prg, v, 0);

		prg->result->set_type(Variable::STRING);
		prg->result->set_s(typeof_var(v1));
	}
}

static bool corefunc_for(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	Variable &count = as_variable(prg, v, 0);
	count.set_type(Variable::NUMBER);

	count.set_n(as_number(prg, v, 1));
	Variable &expr = prg->variables[v[2].i];
	double increment = as_number(prg, v, 3);
	unsigned int end_label = as_label(prg, v, 4);

	CHECK_EXPRESSION(expr)

	evaluate_expression(prg, expr.e);

	if (prg->result->get_n() == 0) {
		prg->s->pc = end_label;
		return true;
	}

	prg->s->pc++;

	prg->break_flag = false;
	prg->continue_flag = false;

	unsigned int start = prg->s->pc;

	while (true) {
		if (interpret(prg) == false) {
			return false;
		}
		if (prg->break_flag) {
			prg->break_flag = false;
			prg->s->pc = end_label;
			return true;
		}
		else if (prg->s->pc < start || prg->s->pc > end_label) {
			break;
		}
		if (prg->continue_flag) {
			prg->continue_flag = false;
			prg->s->pc = end_label;
		}
		if (prg->s->pc == end_label) {
			count.set_n(count.get_n()+increment);
			evaluate_expression(prg, expr.e);
			if (prg->result->get_n() == 0) {
				prg->s->pc++;
				break;
			}
			prg->s->pc = start;
		}
	}

	return true;
}

static bool corefunc_while(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &expr = prg->variables[v[0].i];
	unsigned int end_label = as_label(prg, v, 1);

	CHECK_EXPRESSION(expr)

	evaluate_expression(prg, expr.e);

	if (prg->result->get_n() == 0) {
		prg->s->pc = end_label;
		return true;
	}

	prg->s->pc++;

	prg->break_flag = false;
	prg->continue_flag = false;

	unsigned int start = prg->s->pc;

	while (true) {
		if (interpret(prg) == false) {
			return false;
		}
		if (prg->break_flag) {
			prg->break_flag = false;
			prg->s->pc = end_label;
			return true;
		}
		else if (prg->s->pc < start || prg->s->pc > end_label) {
			break;
		}
		if (prg->continue_flag) {
			prg->continue_flag = false;
			prg->s->pc = end_label;
		}
		if (prg->s->pc == end_label) {
			evaluate_expression(prg, expr.e);
			if (prg->result->get_n() == 0) {
				prg->s->pc++;
				break;
			}
			prg->s->pc = start;
		}
	}

	return true;
}

static bool corefunc_do_while(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &expr = prg->variables[v[0].i];
	unsigned int end_label = as_label(prg, v, 1);

	CHECK_EXPRESSION(expr)

	prg->s->pc++;

	prg->break_flag = false;
	prg->continue_flag = false;

	unsigned int start = prg->s->pc;

	while (true) {
		if (interpret(prg) == false) {
			return false;
		}
		if (prg->break_flag) {
			prg->break_flag = false;
			prg->s->pc = end_label;
			return true;
		}
		else if (prg->s->pc < start || prg->s->pc > end_label) {
			break;
		}
		if (prg->continue_flag) {
			prg->continue_flag = false;
			prg->s->pc = end_label;
		}
		if (prg->s->pc == end_label) {
			evaluate_expression(prg, expr.e);
			if (prg->result->get_n() == 0) {
				prg->s->pc++;
				break;
			}
			prg->s->pc = start;
		}
	}

	return true;
}

static bool corefunc_if(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int end = v.size();
	end -= (end % 2);
	int prev = -1;

	for (int i = 0; i < end; i += 2) {
		bool b = as_number(prg, v, i);
		if (b) {
			if (prev == -1) {
				prg->s->pc++;
			}
			else {
				prg->s->pc = prev;
			}
			unsigned int start = prg->s->pc;
			unsigned int end_block = as_label(prg, v, i+1);
			while (prg->s->pc != end_block) {
				if (interpret(prg) == false) {
					return false;
				}
				if (prg->s->pc < start || prg->s->pc > end_block) {
					return true;
				}
			}
			prg->s->pc = as_label(prg, v, v.size()-1);
			return true;
		}
		prev = as_label(prg, v, i+1);
	}

	if (v.size() <= 2 || v.size() % 2 == 1) {
		prg->s->pc = prev;
		unsigned int start = prg->s->pc;
		unsigned int end_block = as_label(prg, v, v.size()-1);
		while (prg->s->pc != end_block) {
			if (interpret(prg) == false) {
				return false;
			}
			if (prg->s->pc < start || prg->s->pc > end_block) {
				return true;
			}
		}
		prg->s->pc++;
	}
	else {
		unsigned int end_block = as_label(prg, v, v.size()-1);
		prg->s->pc = end_block;
	}

	return true;
}

static void exprfunc_time(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)
	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n((double)time(NULL));
}

static bool corefunc_srand(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	uint32_t seed = (uint32_t)as_number(prg, v, 0);

	util::srand(seed);

	return true;
}

static void exprfunc_rand(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int min_incl = as_number(prg, v, 0);
	int max_incl = as_number(prg, v, 1);

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(util::rand(min_incl, max_incl));
}

static bool corefunc_explode(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable *vec = as_variable_pointer(prg, v, 0);

	CHECK_VECTOR(*vec)

	for (size_t i = 1; i < v.size() && (i-1) < vec->v.size(); i++) {
		Variable &v1 = as_variable(prg, v, i);
		v1.set(vec->v[i-1]);
	}

	return true;
}

static void exprfunc_core_get_savedgames_path(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result->set_type(Variable::STRING);
	prg->result->set_s(util::get_savegames_dir());
}

static void exprfunc_add(Program *prg, const std::vector<Token> &v)
{
	if (v[0].type == Token::SYMBOL) {
		Variable *var = &prg->variables[v[0].i];

		if (var->get_type() == Variable::EXPRESSION) {
			static Variable var2;
			Variable *tmp = prg->result;
			prg->result = &var2;
			evaluate_expression(prg, var->e);
			prg->result = tmp;
			var = &var2;
		}
		else if (var->get_type() == Variable::FISH) {
			var = &go_fish(prg, var->f);
		}

		if (var->get_type() == Variable::NUMBER) {
			double n = var->get_n();

			for (size_t i = 1; i < v.size(); i++) {
				n += as_number(prg, v, i);
			}

			prg->result->set_type(Variable::NUMBER);
			prg->result->set_n(n);
			return;
		}
		else if (var->get_type() == Variable::STRING) {
			std::string s = var->get_s();

			for (size_t i = 1; i < v.size(); i++) {
				s += as_string(prg, v, i);
			}

			prg->result->set_type(Variable::STRING);
			prg->result->set_s(s);
			return;
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
		}
	}
	else if (v[0].type == Token::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			n += as_number(prg, v, i);
		}

		prg->result->set_type(Variable::NUMBER);
		prg->result->set_n(n);
		return;
	}
	else if (v[0].type == Token::STRING) {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			s += as_string(prg, v, i);
		}

		prg->result->set_type(Variable::STRING);
		prg->result->set_s(s);
		return;
	}
}

static void exprfunc_subtract(Program *prg, const std::vector<Token> &v)
{
	double n = as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n -= as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_multiply(Program *prg, const std::vector<Token> &v)
{
	double n = as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n *= as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_divide(Program *prg, const std::vector<Token> &v)
{
	double n = as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n /= as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_modulus(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int n = as_number(prg, v, 0);
	int n2 = as_number(prg, v, 1);

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n % n2);
}

static void exprfunc_and(Program *prg, const std::vector<Token> &v)
{
	bool b = (bool)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		b = b && (bool)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_or(Program *prg, const std::vector<Token> &v)
{
	bool b = (bool)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		b = b || (bool)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_not(Program *prg, const std::vector<Token> &v)
{
	bool b = !(bool)as_number(prg, v, 0);

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_greater(Program *prg, const std::vector<Token> &v)
{
	Variable *p = nullptr;
	if (!(v[0].type == Token::NUMBER || v[0].type == Token::STRING)) {
		p = as_variable_pointer(prg, v, 0);
	}

	bool b = true;

	if (v[0].type == Token::NUMBER || p->get_type() == Variable::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n > as_number(prg, v, i);
		}
	}
	else {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s > as_string(prg, v, i);
		}
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_less(Program *prg, const std::vector<Token> &v)
{
	Variable *p = nullptr;
	if (!(v[0].type == Token::NUMBER || v[0].type == Token::STRING)) {
		p = as_variable_pointer(prg, v, 0);
	}

	bool b = true;

	if (v[0].type == Token::NUMBER || p->get_type() == Variable::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n < as_number(prg, v, i);
		}
	}
	else {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s < as_string(prg, v, i);
		}
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_greaterequal(Program *prg, const std::vector<Token> &v)
{
	Variable *p = nullptr;
	if (!(v[0].type == Token::NUMBER || v[0].type == Token::STRING)) {
		p = as_variable_pointer(prg, v, 0);
	}

	bool b = true;

	if (v[0].type == Token::NUMBER || p->get_type() == Variable::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n >= as_number(prg, v, i);
		}
	}
	else {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s >= as_string(prg, v, i);
		}
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_lessequal(Program *prg, const std::vector<Token> &v)
{
	Variable *p = nullptr;
	if (!(v[0].type == Token::NUMBER || v[0].type == Token::STRING)) {
		p = as_variable_pointer(prg, v, 0);
	}

	bool b = true;

	if (v[0].type == Token::NUMBER || p->get_type() == Variable::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n <= as_number(prg, v, i);
		}
	}
	else {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s <= as_string(prg, v, i);
		}
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_equal(Program *prg, const std::vector<Token> &v)
{
	Variable *p = nullptr;
	if (!(v[0].type == Token::NUMBER || v[0].type == Token::STRING)) {
		p = as_variable_pointer(prg, v, 0);
	}

	bool b = true;

	if (v[0].type == Token::NUMBER || p->get_type() == Variable::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n == as_number(prg, v, i);
		}
	}
	else if (v[0].type == Token::STRING || p->get_type() == Variable::STRING) {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s == as_string(prg, v, i);
		}
	}
	else if  (p->get_type() == Variable::USER) {
		for (size_t i = 1; i < v.size(); i++) {
			Variable *p2 = as_variable_pointer(prg, v, i);
			b = b && (p->get_type() == p2->get_type() && p->get_n() == p2->get_n() && p->get_s() == p2->get_s() && p->get_p() == p2->get_p() && p->v == p2->v && p->m == p2->m);
		}
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_notequal(Program *prg, const std::vector<Token> &v)
{
	Variable *p = nullptr;
	if (!(v[0].type == Token::NUMBER || v[0].type == Token::STRING)) {
		p = as_variable_pointer(prg, v, 0);
	}

	bool b = true;

	if (v[0].type == Token::NUMBER || p->get_type() == Variable::NUMBER) {
		double n = as_number(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n != as_number(prg, v, i);
		}
	}
	else if (v[0].type == Token::STRING || p->get_type() == Variable::STRING) {
		std::string s = as_string(prg, v, 0);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s != as_string(prg, v, i);
		}
	}
	else if  (p->get_type() == Variable::USER) {
		for (size_t i = 1; i < v.size(); i++) {
			Variable *p2 = as_variable_pointer(prg, v, i);
			b = b && (p->get_type() != p2->get_type() || p->get_n() != p2->get_n() || p->get_s() != p2->get_s() || p->get_p() != p2->get_p() || p->v != p2->v || p->m != p2->m);
		}
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(b);
}

static void exprfunc_bitor(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n |= (int)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_xor(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n ^= (int)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_bitand(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n &= (int)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_leftshift(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n <<= (int)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static void exprfunc_rightshift(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n >>= (int)as_number(prg, v, i);
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(n);
}

static Variable matmul(Program *prg, Variable ret, Variable vec2)
{
	if (ret.get_type() == Variable::NUMBER && vec2.get_type() == Variable::NUMBER) {
		Variable var;
		var.set_type(Variable::NUMBER);
		var.set_n(ret.get_n() * vec2.get_n());
		return var;
	}
	if (ret.get_type() == Variable::NUMBER) {
		Variable tmp = ret;
		ret = vec2;
		vec2 = tmp;
	}
	if (vec2.get_type() == Variable::NUMBER) {
		// it's a vector
		if (ret.v[0].get_type() == Variable::NUMBER) {
			for (size_t i = 0; i < ret.v.size(); i++) {
				ret.v[i].set_n(ret.v[i].get_n()*vec2.get_n());
			}
		}
		// it's a matrix
		else {
			for (size_t i = 0; i < ret.v.size(); i++) {
				for (size_t j = 0; j < ret.v[0].v.size(); j++) {
					ret.v[i].v[j].set_n(ret.v[i].v[j].get_n()*vec2.get_n());
				}
			}
		}
	}
	else {
		CHECK_VECTOR(vec2);

		bool is_mat1, is_mat2;

		Variable tmp;

		if (ret.v[0].get_type() == Variable::VECTOR) {
			is_mat1 = true;
		}
		else {
			is_mat1 = false;
			tmp = ret;
			ret.v.clear();
			for (size_t i = 0; i < tmp.v.size(); i++) {
				Variable var;
				var.set_type(Variable::VECTOR);
				var.v.push_back(tmp.v[i]);
				ret.v.push_back(var);
			}
		}

		if (vec2.v[0].get_type() == Variable::VECTOR) {
			is_mat2 = true;
		}
		else {
			is_mat2 = false;
			tmp = vec2;
			vec2.v.clear();
			vec2.v.push_back(tmp);
		}

		if (is_mat1 == false && is_mat2 == false) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Vector-vector multiplication is undefined at " + get_error_info(prg)));
		}

		if (ret.v.size() != vec2.v[0].v.size()) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Matrices cannot be multiplied at " + get_error_info(prg)));
		}

		unsigned int w = vec2.v.size();
		unsigned int h = ret.v[0].v.size();

		Variable tmp2 = ret;

		while (ret.v.size() > w) {
			ret.v.pop_back();
		}
		for (size_t i = 0; i < ret.v.size(); i++) {
			while (ret.v[i].v.size() > h) {
				ret.v[i].v.pop_back();
			}
		}

		if (ret.v.size() != w || ret.v[0].v.size() != h) {
			for (unsigned int c = 0; c < w; c++) {
				Variable var;
				var.set_type(Variable::VECTOR);
				for (unsigned int r = 0; r < h; r++) {
					Variable var2;
					var2.set_type(Variable::NUMBER);
					var2.set_n(0);
					var.v.push_back(var2);
				}
				ret.v.push_back(var);
			}
		}

		tmp = ret;
		ret = tmp2;

		int rr = 0;
		for (size_t r = 0; r < h; r++) {
			int cc = 0;
			for (size_t c = 0; c < w; c++) {
				double sum = 0;
				for (size_t r2 = 0; r2 < vec2.v[c].v.size(); r2++) {
					sum += ret.v[r2].v[r].get_n() * vec2.v[c].v[r2].get_n();
				}
				tmp.v[cc].v[rr].set_n(sum);
				cc++;
			}
			rr++;
		}

		ret = tmp;

		if (is_mat1 == false) {
			tmp = ret;
			ret.v.clear();
			for (size_t i = 0; i < tmp.v.size(); i++) {
				ret.v.push_back(tmp.v[i].v[0]);
			}
		}
		else if (is_mat2 == false) {
			tmp = ret;
			ret = tmp.v[0];
		}
	}

	return ret;
}

static void exprfunc_mul(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable var = as_variable_resolve(prg, v, 0);
	
	for (size_t n = 1; n < v.size(); n++) {	
		Variable var2 = as_variable_resolve(prg, v, n);

		var = matmul(prg, var, var2);
	}

	prg->result->set(var);
}

glm::mat4 to_glm_mat4(Variable &v)
{
	glm::mat4 m = glm::mat4(1.0f);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m[i][j] = (float)v.v[i].v[j].get_n();
		}
	}
	return m;
}

Variable from_glm_mat4(glm::mat4 m)
{
	Variable var;
	var.set_type(Variable::VECTOR);
	for (int i = 0; i < 4; i++) {
		Variable v2;
		v2.set_type(Variable::VECTOR);
		for (int j = 0; j < 4; j++) {
			Variable v3;
			v3.set_type(Variable::NUMBER);
			v3.set_n(m[i][j]);
			v2.v.push_back(v3);
		}
		var.v.push_back(v2);
	}
	return var;
}

static Variable identity(int sz)
{
	return from_glm_mat4(glm::mat4(1.0f));
}

static void exprfunc_frustum(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(6)

	float left = as_number(prg, v, 0);
	float right = as_number(prg, v, 1);
	float bottom = as_number(prg, v, 2);
	float top = as_number(prg, v, 3);
	float nearval = as_number(prg, v, 4);
	float farval = as_number(prg, v, 5);

	glm::mat4 m = glm::frustum(left, right, bottom, top, nearval, farval);

	prg->result->set(from_glm_mat4(m));
}

static void exprfunc_perspective(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	float fovy = as_number(prg, v, 0);
	float aspect = as_number(prg, v, 1);
	float nearval = as_number(prg, v, 2);
	float farval = as_number(prg, v, 3);

	glm::mat4 m = glm::perspective(fovy, aspect, nearval, farval);

	prg->result->set(from_glm_mat4(m));
}

static void exprfunc_ortho(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	float left = as_number(prg, v, 0);
	float right = as_number(prg, v, 1);
	float bottom = as_number(prg, v, 2);
	float top = as_number(prg, v, 3);

	glm::mat4 m = glm::ortho(left, right, bottom, top);

	prg->result->set(from_glm_mat4(m));
}

static void exprfunc_identity(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)
	
	int sz = as_number(prg, v, 0);

	prg->result->set(identity(sz));
}

static void exprfunc_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double sx = as_number(prg, v, 0);
	double sy = as_number(prg, v, 1);
	double sz = as_number(prg, v, 2);

	prg->result->set(identity(4));

	prg->result->v[0].v[0].set_n(sx);
	prg->result->v[1].v[1].set_n(sy);
	prg->result->v[2].v[2].set_n(sz);
}

static void exprfunc_rotate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)
	
	double angle = as_number(prg, v, 0);
	double x = as_number(prg, v, 1);
	double y = as_number(prg, v, 2);
	double z = as_number(prg, v, 3);

	double s = sin(angle);
	double c = cos(angle);
	double invc = 1 - c;

	prg->result->set(identity(4));

	prg->result->v[0].v[0].set_n((invc * x * x) + c);
	prg->result->v[0].v[1].set_n((invc * x * y) + (z * s));
	prg->result->v[0].v[2].set_n((invc * x * z) - (y * s));
	prg->result->v[1].v[0].set_n((invc * x * y) - (z * s));
	prg->result->v[1].v[1].set_n((invc * y * y) + c);
	prg->result->v[1].v[2].set_n((invc * z * y) + (x * s));
	prg->result->v[2].v[0].set_n((invc * x * z) + (y * s));
	prg->result->v[2].v[1].set_n((invc * y * z) - (x * s));
	prg->result->v[2].v[2].set_n((invc * z * z) + c);
}

static void exprfunc_translate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double tx = as_number(prg, v, 0);
	double ty = as_number(prg, v, 1);
	double tz = as_number(prg, v, 2);

	prg->result->set(identity(4));

	prg->result->v[3].v[0].set_n(tx);
	prg->result->v[3].v[1].set_n(ty);
	prg->result->v[3].v[2].set_n(tz);
}

static double veclen(Variable vec)
{
	if (vec.v.size() == 2) {
		return sqrt(pow(vec.v[0].get_n(), 2) + pow(vec.v[1].get_n(), 2));
	}
	return sqrt(pow(vec.v[0].get_n(), 2) + pow(vec.v[1].get_n(), 2) + pow(vec.v[2].get_n(), 2));
}

static void exprfunc_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable vec = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(vec)
	
	if (vec.v.size() != 2 && vec.v.size() != 3) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Vector size not supported in length at " + get_error_info(prg)));
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(veclen(vec));
}

static double vecdot(Variable vec1, Variable vec2)
{
	return vec1.v[0].get_n() * vec2.v[0].get_n() + vec1.v[1].get_n() * vec2.v[1].get_n() + vec1.v[2].get_n() * vec2.v[2].get_n();
}

static void exprfunc_dot(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable vec1 = as_variable_resolve(prg, v, 0);
	Variable vec2 = as_variable_resolve(prg, v, 1);

	CHECK_VECTOR(vec1)
	CHECK_VECTOR(vec2)

	if (vec1.v.size() < 3 || vec2.v.size() < 3) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Vector with < 3 components not supported at " + get_error_info(prg)));
	}

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(vecdot(vec1, vec2));
}

static Variable veccross(Variable vec, Variable vec2)
{
	Variable tmp = vec;
	vec.v[0].set_n(tmp.v[0].get_n() * vec2.v[2].get_n() - tmp.v[2].get_n() * vec2.v[1].get_n());
	vec.v[1].set_n(tmp.v[2].get_n() * vec2.v[0].get_n() - tmp.v[0].get_n() * vec2.v[2].get_n());
	vec.v[2].set_n(tmp.v[0].get_n() * vec2.v[1].get_n() - tmp.v[1].get_n() * vec2.v[0].get_n());
	return vec;
}

static void exprfunc_angle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable vec1 = as_variable_resolve(prg, v, 0);
	Variable vec2 = as_variable_resolve(prg, v, 1);

	CHECK_VECTOR(vec1)
	CHECK_VECTOR(vec2)

	if (vec1.v.size() == 2) {
		double a1 = atan2(vec1.v[1].get_n(), vec1.v[0].get_n());
		double a2 = atan2(vec2.v[1].get_n(), vec2.v[0].get_n());
		prg->result->set_type(Variable::NUMBER);
		prg->result->set_n(a2 - a1);
		if (prg->result->get_n() < 0) {
			prg->result->set_n(prg->result->get_n()+(M_PI * 2));
		}
		return;
	}
	
	if (vec1.v.size() < 3 || vec2.v.size() < 3) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Vector with < 3 components not supported at " + get_error_info(prg)));
	}

	Variable len;
	len.set_type(Variable::NUMBER);
	len.set_n(veclen(vec2));
	
	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(acosf(vecdot(vec1, vec2) / veclen(matmul(prg, vec1, len))));
}

static void exprfunc_cross(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable var = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(var)

	for (size_t i = 1; i < v.size(); i++) {
		Variable vec2 = as_variable_resolve(prg, v, i);
		CHECK_VECTOR(vec2)
		if (var.v.size() < 3 || vec2.v.size() < 3) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Vector with < 3 components not supported at " + get_error_info(prg)));
		}
		var = veccross(var, vec2);
	}

	prg->result->set(var);
}

static void exprfunc_normalize(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable vec = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(vec)

	Variable len;
	len.set_type(Variable::NUMBER);
	len.set_n(1.0 / veclen(vec));

	prg->result->set(matmul(prg, vec, len));
}

static void exprfunc_vadd(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable vec = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(vec)

	for (size_t i = 1; i < v.size(); i++) {
		Variable vec2 = as_variable_resolve(prg, v, i);
		CHECK_VECTOR(vec2)
		if (vec.v.size() != vec2.v.size()) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Can't add different sized vectors at " + get_error_info(prg)));
		}
		if (vec.v[0].get_type() == Variable::NUMBER && vec2.v[0].get_type() == Variable::NUMBER) {
			for (size_t j = 0; j < vec.v.size(); j++) {
				vec.v[j].set_n(vec.v[j].get_n()+vec2.v[j].get_n());
			}
		}
		else if (vec.v[0].get_type() == Variable::VECTOR && vec2.v[0].get_type() == Variable::VECTOR) {
			if (vec.v[0].v.size() != vec2.v[0].v.size()) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Can't add different sized matrices at " + get_error_info(prg)));
			}
			for (size_t j = 0; j < vec.v.size(); j++) {
				for (size_t i = 0; i < vec.v[j].v.size(); i++) {
					vec.v[j].v[i].set_n(vec.v[j].v[i].get_n()+vec2.v[j].v[i].get_n());
				}
			}
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Add not supported on types at " + get_error_info(prg)));
		}
	}

	prg->result->set(vec);
}

static void exprfunc_vsub(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable vec = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(vec)

	for (size_t i = 1; i < v.size(); i++) {
		Variable vec2 = as_variable_resolve(prg, v, i);
		CHECK_VECTOR(vec2)
		if (vec.v.size() != vec2.v.size()) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Can't subtract different sized vectors at " + get_error_info(prg)));
		}
		if (vec.v[0].get_type() == Variable::NUMBER && vec2.v[0].get_type() == Variable::NUMBER) {
			for (size_t j = 0; j < vec.v.size(); j++) {
				vec.v[j].set_n(vec.v[j].get_n()-vec2.v[j].get_n());
			}
		}
		else if (vec.v[0].get_type() == Variable::VECTOR && vec2.v[0].get_type() == Variable::VECTOR) {
			if (vec.v[0].v.size() != vec2.v[0].v.size()) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Can't subtract different sized matrices at " + get_error_info(prg)));
			}
			for (size_t j = 0; j < vec.v.size(); j++) {
				for (size_t i = 0; i < vec.v[j].v.size(); i++) {
					vec.v[j].v[i].set_n(vec.v[j].v[i].get_n()-vec2.v[j].v[i].get_n());
				}
			}
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Subtract not supported on types at " + get_error_info(prg)));
		}
	}

	prg->result->set(vec);
}

static void exprfunc_inverse(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable mat = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(mat)

	glm::mat4 m = to_glm_mat4(mat);
	m = glm::inverse(m);
	prg->result->set(from_glm_mat4(m));
}

static void exprfunc_transpose(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable mat = as_variable_resolve(prg, v, 0);

	CHECK_VECTOR(mat)

	glm::mat4 m = to_glm_mat4(mat);
	m = glm::transpose(m);
	prg->result->set(from_glm_mat4(m));
}

static void exprfunc_address(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	prg->result->set_type(Variable::POINTER);
	prg->result->set_p(&as_variable(prg, v, 0));
}

static void exprfunc_toptr(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string s = as_string(prg, v, 0);

	Variable *p = nullptr;

	if (prg != prg_func) {
		size_t f = 0;
		for (f = 0; f < prg->function_names.size(); f++) {
			if (prg->function_names[f] == prg_func->s->name) {
				break;
			}
		}
		if (f < prg->function_names.size()) {
			if (prg->locals[f].find(s) != prg->locals[f].end()) {
				p = &prg->variables[prg->locals[f][s]];
			}
		}
	}

	if (p == nullptr) {
		if (prg->variables_map.find(s) != prg->variables_map.end()) {
			p = &prg->variables[prg->variables_map[s]];
		}
	}

	prg->result->set_type(Variable::POINTER);
	prg->result->set_p(p);
}

static void exprfunc_get_args(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result->set_type(Variable::VECTOR);

	for (int i = 0; i < shim::argc; i++) {
		Variable var;
		var.set_type(Variable::STRING);
		var.set_s(shim::argv[i]);
		prg->result->v.push_back(var);
	}
}

static void exprfunc_num_var_args(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	const std::vector<Token> &params = var_args.top();
	int num_hard_params = num_var_args_args.top();

	prg->result->set_type(Variable::NUMBER);
	prg->result->set_n(params.size()-num_hard_params);
}

static void exprfunc_get_var_arg(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int i = as_number(prg, v, 0);

	const std::vector<Token> &params = var_args.top();
	int num_hard_params = num_var_args_args.top();

	if (i+num_hard_params >= params.size() || i+num_hard_params < 0) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Parameter out of range at " + get_error_info(prg)));
	}

	if (params[i+num_hard_params].type == Token::NUMBER) {
		prg->result->set_type(Variable::NUMBER);
		prg->result->set_n(params[i+num_hard_params].n);
	}
	else if (params[i+num_hard_params].type == Token::STRING) {
		prg->result->set_type(Variable::STRING);
		prg->result->set_s(params[i+num_hard_params].s);
	}
	else {
		prg->result->set(*as_variable_pointer(prg, params, i+num_hard_params));
	}
}

static void init_token_map()
{
	add_token_handler(':', tokenfunc_label);
	add_token_handler('"', tokenfunc_string);
	add_token_handler('{', tokenfunc_openbrace);
	add_token_handler('}', tokenfunc_closebrace);
	add_token_handler(';', tokenfunc_comment);
	add_token_handler('-', tokenfunc_subtract);
	add_token_handler('=', tokenfunc_equals);
	add_token_handler('?', tokenfunc_compare);
	add_token_handler('/', tokenfunc_divide);
	add_token_handler('(', tokenfunc_expression);
	add_token_handler('[', tokenfunc_fish);
	add_token_handler('#', tokenfunc_hex);
	add_token_handler('~', tokenfunc_ref);
	add_token_handler('*', tokenfunc_mlcomment);
	add_token_handler('\'', tokenfunc_char);
	add_token_handler('`', tokenfunc_deref);
}

void start()
{
	util::srand((Uint32)time(NULL));

	twinkle::start();

	init_token_map();

	add_expression_handler("+", exprfunc_add);
	add_expression_handler("-", exprfunc_subtract);
	add_expression_handler("*", exprfunc_multiply);
	add_expression_handler("/", exprfunc_divide);
	add_expression_handler("%", exprfunc_modulus);
	add_expression_handler("&&", exprfunc_and);
	add_expression_handler("||", exprfunc_or);
	add_expression_handler("!", exprfunc_not);
	add_expression_handler(">", exprfunc_greater);
	add_expression_handler("<", exprfunc_less);
	add_expression_handler(">=", exprfunc_greaterequal);
	add_expression_handler("<=", exprfunc_lessequal);
	add_expression_handler("==", exprfunc_equal);
	add_expression_handler("!=", exprfunc_notequal);
	add_expression_handler("|", exprfunc_bitor);
	add_expression_handler("^", exprfunc_xor);
	add_expression_handler("&", exprfunc_bitand);
	add_expression_handler("<<", exprfunc_leftshift);
	add_expression_handler(">>", exprfunc_rightshift);
	add_expression_handler("mul", exprfunc_mul);
	add_expression_handler("frustum", exprfunc_frustum);
	add_expression_handler("perspective", exprfunc_perspective);
	add_expression_handler("ortho", exprfunc_ortho);
	add_expression_handler("identity", exprfunc_identity);
	add_expression_handler("scale", exprfunc_scale);
	add_expression_handler("rotate", exprfunc_rotate);
	add_expression_handler("translate", exprfunc_translate);
	add_expression_handler("length", exprfunc_length);
	add_expression_handler("dot", exprfunc_dot);
	add_expression_handler("angle", exprfunc_angle);
	add_expression_handler("cross", exprfunc_cross);
	add_expression_handler("normalize", exprfunc_normalize);
	add_expression_handler("add", exprfunc_vadd);
	add_expression_handler("sub", exprfunc_vsub);
	add_expression_handler("inverse", exprfunc_inverse);
	add_expression_handler("transpose", exprfunc_transpose);
	add_expression_handler("@", exprfunc_address);
	add_expression_handler("toptr", exprfunc_toptr);

	add_instruction("reset", breaker_reset);
	add_instruction("exit", breaker_exit);
	add_instruction("return", breaker_return);

	add_instruction("break", corefunc_break);
	add_instruction("continue", corefunc_continue);

	add_instruction("var", corefunc_var);
	add_instruction("const", corefunc_const);
	
	add_instruction("=", corefunc_set);
	add_expression_handler("=", exprfunc_set);
	
	add_instruction(":", corefunc_label);
	add_instruction("goto", corefunc_goto);
	add_instruction("?", corefunc_compare);
	add_instruction("je", corefunc_je);
	add_instruction("jne", corefunc_jne);
	add_instruction("jl", corefunc_jl);
	add_instruction("jle", corefunc_jle);
	add_instruction("jg", corefunc_jg);
	add_instruction("jge", corefunc_jge);
	add_instruction("call", corefunc_call);
	add_instruction("call_result", corefunc_call_result);
	add_expression_handler("typeof", exprfunc_typeof);
	add_expression_handler("number", exprfunc_number);
	add_expression_handler("string", exprfunc_string);
	add_expression_handler("vector", exprfunc_vector);
	add_expression_handler("map", exprfunc_map);
	add_expression_handler("function", exprfunc_function);
	add_expression_handler("label", exprfunc_label);
	add_expression_handler("pointer", exprfunc_pointer);

	add_instruction("for", corefunc_for);
	add_instruction("while", corefunc_while);
	add_instruction("do_while", corefunc_do_while);
	add_instruction("if", corefunc_if);
	
	add_expression_handler("time", exprfunc_time);
	add_instruction("srand", corefunc_srand);
	add_expression_handler("rand", exprfunc_rand);
	add_instruction("explode", corefunc_explode);
	add_expression_handler("get_savedgames_path", exprfunc_core_get_savedgames_path);
	add_expression_handler("get_args", exprfunc_get_args);
	
	add_expression_handler("num_var_args", exprfunc_num_var_args);
	add_expression_handler("get_var_arg", exprfunc_get_var_arg);

	std::vector<Token> tmp;
	var_args.push(tmp);
	num_var_args_args.push(0);

	return_code = 0;
}

void end()
{
	library_map.clear();

	while (var_args.size() > 0) {
		var_args.pop();
	}
	while (num_var_args_args.size() > 0) {
		num_var_args_args.pop();
	}
}

static bool is_special(std::string s)
{
	return s == "draw_letterbox" || s == "gui_event" || s == "gui_draw" || s == "end" || s == "f12" || s == "draw" || s == "event" || s == "run";
}

static void obfuscate_expression(Program *prg, Variable::Expression e);
static void obfuscate_fish(Program *prg, Variable::Fish f);
static void obfuscate_token(Program *prg, Token t);

static void print_derefs(Program *prg, int n)
{
	for (size_t i = 0; i < n; i++) {
		printf("`");
	}
}

static void obfuscate_expression(Program *prg, Variable::Expression e)
{
	printf("(");
	print_derefs(prg, e.dereference);
	if (e.i == -1) {
		printf("%s ", e.name.c_str());
	}
	else if (e.name == " ex ") {
		obfuscate_expression(prg, prg->variables[e.i].e);
	}
	else if (e.name == " fi ") {
		obfuscate_fish(prg, prg->variables[e.i].f);
	}
	else {
		std::map<std::string, int>::iterator it;
		for (it = expression_map.begin(); it != expression_map.end(); it++) {
			if (it->second == e.i) {
				printf("%s ", it->first.c_str());
				break;
			}
		}
	}
	for (size_t i = 0; i < e.v.size(); i++) {
		obfuscate_token(prg, e.v[i]);
	}
	printf(") ");
}

static void obfuscate_fish(Program *prg, Variable::Fish f)
{
	printf("[");
	print_derefs(prg, f.dereference);
	Variable *v = &prg->variables[f.c_i];
	if (v->get_type() == Variable::EXPRESSION) {
		obfuscate_expression(prg, v->e);
	}
	else if (v->get_type() == Variable::FISH) {
		obfuscate_fish(prg, v->f);
	}
	else {
		printf("%s ", v->name.c_str());
	}
	for (size_t i = 0; i < f.v.size(); i++) {
		obfuscate_token(prg, f.v[i]);
	}
	printf("] ");
}

static void obfuscate_token(Program *prg, Token t)
{
	if (t.type == Token::NUMBER) {
		printf("%g ", t.n);
	}
	else if (t.type == Token::STRING) {
		printf("\"%s\" ", escape_string(t.s).c_str());
	}
	else {
		Variable *v = &prg->variables[t.i];
		print_derefs(prg, t.dereference);
		if (v->get_type() == Variable::EXPRESSION) {
			obfuscate_expression(prg, v->e);
		}
		else if (v->get_type() == Variable::FISH) {
			obfuscate_fish(prg, v->f);
		}
		else {
			printf("%s ", v->name.c_str());
		}
	}
}

Program *create_program(std::string code)
{
	Program *prg = new Program;
	prg_func = prg;

	prg->break_flag = false;
	prg->continue_flag = false;

	prg->num_consts = -1;

	prg->main_s = new Function_Swap;
	prg->s = prg->main_s;

	prg->real_line_numbers.push_back(1);
	prg->real_file_names.push_back(main_program_name);
	int i = 0;
	int ln = 2;
	while (code[i] != 0) {
		if (code[i] == '\n') {
			prg->real_line_numbers.push_back(ln++);
			prg->real_file_names.push_back(main_program_name);
		}
		i++;
	}

	prg->s->code = code;
	prg->s->name = "__main";
	prg->s->line = 0;
	prg->s->line_numbers.clear();
	prg->s->start_line = 0;
	prg->s->p = 0;
	prg->s->pc = 0;
	prg->complete_pass = PASS0;
	
	while(process_includes(prg));

	// This prints the program with all includes inserted, prefixed by line number and filename
#if 0
	printf("---\n");
	for (size_t i = 0; i < prg->real_line_numbers.size(); i++) {
		int off = 0;
		std::string line;
		for (int j = 0; j < i+1; j++) {
			line = "";
			while (off < prg->s->code.length() && prg->s->code[off] != '\n') {
				char buf[2];
				buf[0] = prg->s->code[off];
				buf[1] = 0;
				line += buf;
				off++;
			}
			off++;
		}
		printf("%d:%s:%s\n", prg->real_line_numbers[i], prg->real_file_names[i].c_str(), line.c_str());
	}
	printf("---\n");
#endif

	compile(prg, PASS1);

	prg->s->p = 0;
	prg->s->line = 0;
	prg->s->start_line = 0;
	prg->s->program.clear();
	for (size_t i = 0; i < prg->functions.size(); i++) {
		delete prg->functions[i].s;
	}
	prg->functions.clear();
	prg->s->line_numbers.clear();

	compile(prg, PASS2);

	prg->s->p = 0;
	prg->s->line = 0;
	prg->s->start_line = 0;
	prg->s->pc = 0;

	prg->num_vars = prg->variables.size();

	prg->result = new Variable();
	prg->result->name = "result";
	
	if (util::bool_arg(false, shim::argc, shim::argv, "obfuscate")) {
		int count = 0;
		for (size_t i = prg->num_consts; i < prg->variables.size(); i++) {
			std::string old = prg->variables[i].name;
			prg->variables[i].name = std::string("__") + util::itos(count++);
			if (prg->variables[i].get_type() == Variable::FUNCTION) {
				for (size_t j = prg->num_consts; j < prg->variables.size(); j++) {
					if (prg->variables[j].get_type() == Variable::EXPRESSION && prg->variables[j].e.i == -1 && prg->variables[j].e.name == old) {
						prg->variables[j].e.name = prg->variables[i].name;
					}
				}
			}
			if (prg->variables[i].get_type() == Variable::FUNCTION) {
				if (is_special(prg->function_names[prg->variables[i].get_n()]) == false) {
					prg->function_names[prg->variables[i].get_n()] = prg->variables[i].name;
				}
			}
		}
		for (size_t i = 0; i < prg->s->program.size(); i++) {
			Statement &s = prg->s->program[i];
			std::map<std::string, int>::iterator it;
			std::string name;
			for (it = library_map.begin(); it != library_map.end(); it++) {
				if (it->second == s.method) {
					name = it->first;
					break;
				}
			}
			printf("%s%s", name.c_str(), name == ":" ? "" : " ");
			for (size_t j = 0; j < s.data.size(); j++) {
				obfuscate_token(prg, s.data[j]);
			}
		}
		printf("\n");
		for (size_t j = 0; j < prg->functions.size(); j++) {
			Program &p = prg->functions[j];
			if (is_special(p.s->name)) {
				printf("function %s ", p.s->name.c_str());
			}
			else {
				printf("function %s ", prg->function_names[j].c_str());
			}
			for (size_t k = 0; k < p.param_names.size(); k++) {
				printf("%s%s ", p.ref[k] ? "~" : "", prg->variables[p.params[k]].name.c_str());
			}
			printf("{ ");
			for (size_t i = 0; i < p.s->program.size(); i++) {
				Statement &s = p.s->program[i];
				std::map<std::string, int>::iterator it;
				std::string name;
				for (it = library_map.begin(); it != library_map.end(); it++) {
					if (it->second == s.method) {
						name = it->first;
						break;
					}
				}
				printf("%s ", name.c_str());
				for (size_t j = 0; j < s.data.size(); j++) {
					obfuscate_token(prg, s.data[j]);
				}
			}
			printf("}\n");
		}
		exit(0);
	}

	return prg;
}

// These help when developing BooBoo apps in C++ or adding API

Token token_number(std::string token, double n)
{
	Token t;
	t.type = Token::NUMBER;
	t.n = n;
	t.dereference = 0;
	return t;
}

Token token_string(std::string token, std::string s)
{
	Token t;
	t.type = Token::STRING;
	t.s = s;
	t.dereference = 0;
	return t;
}

double get_number(Variable &v)
{
	return v.get_n();
}

std::string get_string(Variable &v)
{
	return v.get_s();
}

std::vector<Variable> get_vector(Variable &v)
{
	return v.v;
}

void *get_black_box(std::string id)
{
	if (black_box.find(id) == black_box.end()) {
		return nullptr;
	}
	return black_box[id];
}

void set_black_box(std::string id, void *data)
{
	black_box[id] = data;
}

Variable &get_variable(Program *prg, int index)
{
	return prg->variables[index];
}

Variable &as_variable(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type != Token::SYMBOL) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	if (prg->variables[v[index].i].get_type() == Variable::FISH) {
		return go_fish(prg, prg->variables[v[index].i].f);
	}
	return prg->variables[v[index].i];
}

Variable *as_variable_pointer(Program *prg, const std::vector<Token> &v, int index)
{
	static Variable vars[100];

	if (index < 0 || index >= 100) {
		return nullptr;
	}

	Variable &v2 = as_variable(prg, v, index);

	Variable *var;
	var = &v2;

	if (v2.get_type() == Variable::POINTER) {
		var = dereference(prg, v, index);
	}
	
	if (var->get_type() == Variable::EXPRESSION) {
		Variable *tmp = prg->result;
		prg->result = &vars[index];
		evaluate_expression(prg, var->e);
		prg->result = tmp;
		var = &vars[index];
	}

	return var;
}

Variable as_variable_resolve(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type == Token::NUMBER) {
		Variable var;
		var.set_type(Variable::NUMBER);
		var.set_n(v[index].n);
		return var;
	}
	else if (v[index].type == Token::STRING) {
		Variable var;
		var.set_type(Variable::STRING);
		var.set_s(v[index].s);
		return var;
	}
	else if (prg->variables[v[index].i].get_type() == Variable::FISH) {
		return go_fish(prg, prg->variables[v[index].i].f);
	}
	else if (prg->variables[v[index].i].get_type() == Variable::EXPRESSION) {
		evaluate_expression(prg, prg->variables[v[index].i].e);
		return *prg->result;
	}
	else if (v[index].dereference > 0) {
		return *dereference(prg, v, index);
	}
	else {
		return prg->variables[v[index].i];
	}
}

double as_number(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type == Token::NUMBER) {
		return v[index].n;
	}
	else if (v[index].type == Token::SYMBOL) {
		Variable *var = as_variable_pointer(prg, v, index);
		if (var->get_type() == Variable::NUMBER) {
			return var->get_n();
		}
		else if (var->get_type() == Variable::FISH) {
			Variable &v2 = go_fish(prg, var->f);
			if (v2.get_type() == Variable::NUMBER) {
				return v2.get_n();
			}
			else if (v2.get_type() == Variable::STRING) {
				return atof(v2.get_s().c_str());
			}
			else {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg)));
			}
		}
		else if (var->get_type() == Variable::EXPRESSION) {
			evaluate_expression(prg, var->e);
			if (prg->result->get_type() == Variable::NUMBER) {
				return prg->result->get_n();
			}
			else if (prg->result->get_type() == Variable::STRING) {
				return atof(prg->result->get_s().c_str());
			}
			else {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
			}
		}
		else if (var->get_type() == Variable::STRING) {
			return atof(var->get_s().c_str());
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
		}
	}
	else if (v[index].type == Token::STRING) {
		return atof(v[index].s.c_str());
	}
	else {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	return 0.0f;
}

std::string as_string(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type == Token::STRING) {
		return v[index].s;
	}
	else if (v[index].type == Token::NUMBER) {
		char buf[1000];
		snprintf(buf, 1000, "%f", v[index].n);
		return buf;
	}
	else if (v[index].type == Token::SYMBOL) {
		Variable *var = as_variable_pointer(prg, v, index);
		if (var->get_type() == Variable::STRING) {
			return var->get_s();
		}
		else if (var->get_type() == Variable::NUMBER) {
			char buf[1000];
			snprintf(buf, 1000, "%f", var->get_n());
			return buf;
		}
		else if (var->get_type() == Variable::EXPRESSION) {
			evaluate_expression(prg, var->e);
			if (prg->result->get_type() != Variable::STRING) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
			}
			return prg->result->get_s();
		}
		else if (var->get_type() == Variable::FISH) {
			Variable &v2 = go_fish(prg, var->f);
			if (v2.get_type() != Variable::STRING) {
				my_throw(Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg)));
			}
			return v2.get_s();
		}
		else {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
		}
	}
	else {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	return "";
}

int as_label(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type != Token::SYMBOL) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	Variable *var = as_variable_pointer(prg, v, index);
	if (var->get_type() == Variable::FISH) {
		Variable &v2 = go_fish(prg, var->f);
		if (v2.get_type() != Variable::LABEL) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg)));
		}
		return v2.get_n();
	}
	else if (var->get_type() != Variable::LABEL) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	return var->get_n();
}

int as_function(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type != Token::SYMBOL) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	Variable *var = as_variable_pointer(prg, v, index);
	if (var->get_type() == Variable::FISH) {
		Variable &v2 = go_fish(prg, var->f);
		if (v2.get_type() != Variable::FUNCTION) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type (" + util::itos(v2.get_type()) + ") at " + get_error_info(prg)));
		}
		return v2.get_n();
	}
	else if (var->get_type() != Variable::FUNCTION) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	return var->get_n();
}

Variable as_pointer(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type != Token::SYMBOL) {
		my_throw(Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg)));
	}
	if (prg->variables[v[index].i].get_type() == Variable::FISH) {
		return go_fish(prg, prg->variables[v[index].i].f);
	}
	else if (prg->variables[v[index].i].get_type() == Variable::EXPRESSION) {
		evaluate_expression(prg, prg->variables[v[index].i].e);
		return *prg->result;
	}
	else if (v[index].dereference) {
		return *(prg->variables[v[index].i].get_p());
	}
	else {
		return prg->variables[v[index].i];
	}
}

// Error class

Error::Error()
{
}

Error::Error(std::string error_message) : error_message(error_message)
{
}

Error::~Error()
{
}

// Internal stuff

void evaluate_expression(Program *prg, const Variable::Expression &e)
{
	if (e.i == -1) {
		int i;
		for (i = 0; i < (int)prg->function_names.size(); i++) {
			if (prg->function_names[i] == e.name) {
				break;
			}
		}

		if (i >= (int)prg->function_names.size()) {
			my_throw(Error(std::string(__FUNCTION__) + ": " + "Unknown expression function '" + e.name + "' at " + get_error_info(prg)));
		}

		call_function(prg, i, e.v);

		return;
	}
	else if (e.name == " ex ") {
		evaluate_expression(prg, prg->variables[e.i].e);

		call_function(prg, e.dereference ? prg->result->get_p()->get_n() : prg->result->get_n(), e.v);

		return;
	}
	else if (e.name == " fi ") {
		Variable &var = go_fish(prg, prg->variables[e.i].f);

		call_function(prg, var.get_n(), e.v);

		return;
	}

	expression_handlers[e.i](prg, e.v);
}

Variable &go_fish(Program *prg, const Variable::Fish &f)
{
	Variable *v = &prg->variables[f.c_i];
	if (v->get_type() == Variable::POINTER) {
		v = v->get_p();
	}
	int type = v->get_type();
	bool constant = v->constant;
	static Variable tmp;

	while (type == Variable::FISH || type == Variable::EXPRESSION) {
		if (type == Variable::FISH) {
			v = &go_fish(prg, v->f);
			type = v->get_type();
			constant = v->constant;
		}
		else {
			static Variable _v;
			Variable *tmp = prg->result;
			prg->result = &_v;
			evaluate_expression(prg, v->e);
			prg->result = tmp;
			v = &_v;
			type = v->get_type();
			constant = v->constant;
		}
	}

	int index = 0;
	std::string key;

	for (size_t i = 0; i < f.v.size(); i++) {
		Variable *var;
		if (f.v[i].type == Token::NUMBER) {
			type = Variable::VECTOR;
			index = as_number(prg, f.v, i);
		}
		else if (f.v[i].type == Token::STRING) {
			type = Variable::MAP;
			key = as_string(prg, f.v, i);
		}
		else if (f.v[i].type == Token::SYMBOL) {
			if (f.v[i].dereference) {
				var = prg->variables[f.v[i].i].get_p();
			}
			else {
				var = &prg->variables[f.v[i].i];
			}
			if (var->get_type() == Variable::EXPRESSION) {
				evaluate_expression(prg, var->e);
				if (prg->result->get_type() == Variable::NUMBER) {
					index = prg->result->get_n();
					type = Variable::VECTOR;
				}
				else {
					key = prg->result->get_s();
					type = Variable::MAP;
				}
			}
			else if (var->get_type() == Variable::FISH) {
				Variable &v2 = go_fish(prg, var->f);
				if (v2.get_type() == Variable::NUMBER) {
					index = v2.get_n();
					type = Variable::VECTOR;
				}
				else {
					key = v2.get_s();
					type = Variable::MAP;
				}
			}
			else if (var->get_type() == Variable::NUMBER) {
				index = as_number(prg, f.v, i);
				type = Variable::VECTOR;
			}
			else {
				key = as_string(prg, f.v, i);
				type = Variable::MAP;
			}
		}
		if (i < f.v.size()-1) {
			if (type == Variable::VECTOR) {
				v = &v->v[index];
			}
			else {
				v = &v->m[key];
			}
		}
	}

	if (v->get_type() != (Variable::Variable_Type)type) {
		v->set_type((Variable::Variable_Type)type, false);
	}
	//v->set_type((Variable::Variable_Type)type);
	if (type == Variable::VECTOR) {
		v->v[index].constant = constant;
		return v->v[index];
	}
	else {
		v->m[key].constant = constant;
		return v->m[key];
	}
}

Variable *dereference(Program *prg, const std::vector<Token> &v, int index)
{
	Variable *var = &prg->variables[v[index].i];
	if (var->get_type() == Variable::EXPRESSION) {
		evaluate_expression(prg, prg->variables[v[index].i].e);
		var = prg->result;
	}
	else if (var->get_type() == Variable::FISH) {
		var = &go_fish(prg, prg->variables[v[index].i].f);
	}
	for (int i = 0; i < v[index].dereference; i++) {
		var = var->get_p();
	}
	return var;
}

void my_throw(Error e)
{
	int result;
	if (gfx::internal::gfx_context.inited == true) {
		if (prg && prg->complete_pass == booboo::PASS2) {
			result = gui::popup("ERROR!", e.error_message, "Abort", "Debug", "Continue");
			if (result == 0) {
				throw e;
			}
			else if (result == 1) {
				shim::debug = true;
				AllocConsole();
				FILE* fp;
				freopen_s(&fp, "CONIN$", "r", stdin);
				freopen_s(&fp, "CONOUT$", "w", stdout);
				freopen_s(&fp, "CONOUT$", "w", stderr);
				booboo::debug("Debugging program. Type 'help' for help...");
			}
			else {
			}
		}
		else {
			result = gui::popup("ERROR!", e.error_message, "Abort");
			throw e;
		}
	}
	else if (shim::debug && prg && prg->complete_pass == booboo::PASS2) {
		booboo::debug("An error occurred: " + e.error_message + "...");
	}
	else {
		throw e;
	}
}

static booboo::Variable *get_var(std::string id, bool allow_expressions = true)
{
	id = util::trim(id);
	if (isalpha(id[0]) || id[0] == '_') {
		if (prg != prg_func) {
			size_t f = 0;
			for (f = 0; f < prg->function_names.size(); f++) {
				if (prg->function_names[f] == prg_func->s->name) {
					break;
				}
			}
			if (f < prg->function_names.size()) {
				if (prg->locals[f].find(id) != prg->locals[f].end()) {
					return &prg->variables[prg->locals[f][id]];
				}
			}
		}
		if (booboo::prg->variables_map.find(id) == booboo::prg->variables_map.end()) {
			printf("Unknown variable '%s'...\n", id.c_str());
			return nullptr;
		}
		else {
			return &booboo::prg->variables[booboo::prg->variables_map[id]];
		}
	}
	else if (id[0] == '[') {
		booboo::Variable::Fish f = booboo::parse_fish(booboo::prg, booboo::prg_func, id, booboo::PASS1);
		f = booboo::parse_fish(booboo::prg, booboo::prg_func, id, booboo::PASS2);
		std::vector<std::string> file_bak = file_breakpoints;
		std::vector<std::string> func_bak = function_breakpoints;
		std::vector<Watchpoint> watch_bak = watchpoints;
		file_breakpoints.clear();
		function_breakpoints.clear();
		watchpoints.clear();
		booboo::Variable *vptr = &booboo::go_fish(booboo::prg, f);
		file_breakpoints = file_bak;
		function_breakpoints = func_bak;
		watchpoints = watch_bak;
		return vptr;
	}
	else if (allow_expressions && id[0] == '(') {
		booboo::Variable::Expression f = booboo::parse_expression(booboo::prg, booboo::prg_func, id, booboo::PASS1);
		f = booboo::parse_expression(booboo::prg, booboo::prg_func, id, booboo::PASS2);
		std::vector<std::string> file_bak = file_breakpoints;
		std::vector<std::string> func_bak = function_breakpoints;
		std::vector<Watchpoint> watch_bak = watchpoints;
		file_breakpoints.clear();
		function_breakpoints.clear();
		watchpoints.clear();
		booboo::evaluate_expression(booboo::prg, f);
		static booboo::Variable var;
		var.set(*prg->result);
		file_breakpoints = file_bak;
		function_breakpoints = func_bak;
		watchpoints = watch_bak;
		return &var;
	}
	else {
		printf("Don't know how to handle that. Try a variable or fish...\n");
		return nullptr;
	}
}

static void print_lines(std::string fn, int curr, int side)
{
	printf("-- %s:%s:%d\n", fn.c_str(), prg->s->name.c_str(), get_line_num(prg));

	if (src_code.find(fn) == src_code.end()) {
		printf("No source found...\n");
		return;
	}

	std::string &s = src_code[fn];

	int start = curr - side;
	int end = curr + side;
	if (start <= 0) {
		end += (-start)+1;
		start = 1;
	}
	int count = 0;
	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] == '\n') {
			count++;
		}
	}
	if (end > count) {
		start -= end - count;
		if (start < 1) {
			start = 1;
		}
		end = count;
	}

	size_t pos = 0;

	for (int i = 1; i < start; i++) {
		while (pos < s.length() && s[pos] != '\n') {
			pos++;
		}
		pos++;
	}

	for (int i = 0; i <= end-start; i++) {
		std::string l;
		while (pos < s.length() && s[pos] != '\n') {
			char buf[2];
			buf[0] = s[pos];
			buf[1] = 0;
			l += buf;
			pos++;
		}
		pos++;
		std::string fmt = std::string(i+start == curr ? "*" : " ") + std::string("%") + util::itos(log10(end)+1) + "d %s\n";
		printf(fmt.c_str(), i+start, l.c_str());
	}
}

void debug(std::string text)
{
	static int count = 0;
	count++;

	bool printed_lines = false;

	printf("%s\n", text.c_str());
	while (true) {
		std::string fn = get_file_name(prg);

		if (src_code.find(fn) == src_code.end()) {
			try {
				std::string text;
				if (gfx::internal::gfx_context.inited == true) {
					text = booboo::load_text("scripts/" + fn);
				}
				else {
					text = booboo::load_text(fn);
				}
				src_code[fn] = text;
			}
			catch (util::Error &e) {
			}
		}

		if (printed_lines == false) {
			int l = get_line_num(prg);
			print_lines(fn, l, 2);
		}
		else {
			printed_lines = false;
		}

		printf("> ");
		fflush(stdout);
		std::string line;
		std::getline(std::cin, line);
		line = util::trim(line);
		if (line == "help") {
			printf("run                  start or continue program execution\n");
			printf("step                 run one instruction\n");
			printf("stepin               run one instruction and step into functions/loops\n");
			printf("skip                 skip this statement\n");
			printf("break <bp>           set a breakpoint. use break delete <bp> to delete\n");
			printf("watch <var> <expr>   set a watchpoint. use watch delete <num> to delete\n");
			printf("list                 list watchpoints and breakpoints\n");
			printf("bt [all]             print a backtrace\n");
			printf("print                print more code context\n");
			printf("print <v>            print the value of a variable or fish\n");
			printf("set <d> <s>          set the value of d to s\n");
			printf("quit                 exit the program\n");
			printed_lines = true;
		}
		else if (line == "quit") {
			exit(0);
		}
		else if (line == "run") {
			break;
		}
		else if (line.substr(0, 5) == "print") {
			printed_lines = true;
			line = line.substr(5);
			line = util::trim(line);
			if (line == "") {
				int l = get_line_num(prg);
				print_lines(fn, l, 11);
			}
			else {
				booboo::Variable *var = get_var(line);
				if (var) {
					printf("Type: %s\n", typeof_var(var).c_str());
					switch (var->get_type()) {
						case booboo::Variable::NUMBER:
							printf("Value: %g\n", var->get_n());
							break;
						case booboo::Variable::STRING:
							printf("Value: %s\n", var->get_s().c_str());
							break;
						case booboo::Variable::POINTER:
							printf("Value: %p\n", var->get_p());
							break;
						default:
							break;
					}
				}
			}
		}
		else if (line.substr(0, 3) == "set") {
			printed_lines = true;
			line = line.substr(3);
			line = util::trim(line);
			std::string dest, src;
			int i = 0;
			if (line[0] == '[') {
				int open = 0;
				while (i < (int)line.length()) {
					if (line[i] == '[') {
						open++;
					}
					else if (line[i] == ']') {
						open--;
					}
					i++;
					if (open == 0) {
						break;
					}
				}
				dest = line.substr(0, i);
				dest = util::trim(dest);
			}
			else {
				while (i < (int)line.length()) {
					char buf[2];
					buf[0] = line[i];
					buf[1] = 0;
					dest += buf;
					if (isspace(line[i])) {
						break;
					}
					i++;
				}
				dest = util::trim(dest);
			}
			src = line.substr(i);
			src = util::trim(src);
			if (!(isalpha(dest[0]) || dest[0] == '_' || dest[0] == '[') || !(src[0] == '-' || isdigit(src[0]) || isalpha(src[0]) || src[0] == '_' || src[0] == '[' || src[0] == '"' || src[0] == '(')) {
				printf("Can't do that...\n");
			}
			else {
				booboo::Variable *var = get_var(dest, false);
				if (isdigit(src[0]) || src[0] == '-') {
					var->set_type(booboo::Variable::NUMBER);
					var->set_n(atof(src.c_str()));
				}
				else if (src[0] == '"') {
					var->set_type(booboo::Variable::STRING);
					var->set_s(util::remove_quotes(src));
				}
				else {
					booboo::Variable *var2 = get_var(src);
					var->set(*var2);
				}
			}
		}
		else if (line.substr(0, 4) == "list") {
			printed_lines = true;
			printf("File breakpoints:\n");
			for (size_t i = 0; i < file_breakpoints.size(); i++) {
				printf("%s\n", file_breakpoints[i].c_str());
			}
			printf("--\n");
			printf("Function breakpoints:\n");
			for (size_t i = 0; i < function_breakpoints.size(); i++) {
				printf("%s\n", function_breakpoints[i].c_str());
			}
			printf("--\n");
			printf("Watchpoints:\n");
			for (size_t i = 0; i < watchpoints.size(); i++) {
				Watchpoint &w = watchpoints[i];
				int digits = log10(watchpoints.size())+1;
				std::string fmt = std::string("%") + util::itos(digits) + "d %s\n";
				printf(fmt.c_str(), i, (w.var + " " + w.expr).c_str());
			}
		}
		else if (line.substr(0, 5) == "break") {
			printed_lines = true;
			line = line.substr(5);
			line = util::trim(line);
			if (line.substr(0, 6) == "delete") {
				line = line.substr(6);
				line = util::trim(line);
				bool del = false;
				if (line.find(':') == std::string::npos) {
					for (std::vector<std::string>::iterator it = booboo::function_breakpoints.begin(); it != booboo::function_breakpoints.end(); it++) {
						if (*it == line) {
							it = booboo::function_breakpoints.erase(it);
							del = true;
							break;
						}
					}
				}
				else {
					for (std::vector<std::string>::iterator it = booboo::file_breakpoints.begin(); it != booboo::file_breakpoints.end(); it++) {
						if (*it == line) {
							it = booboo::file_breakpoints.erase(it);
							del = true;
							break;
						}
					}
				}
				if (del) {
					printf("Breakpoint deleted!\n");
				}
				else {
					printf("Nothing deleted...\n");
				}
			}
			else {
				if (line.find(':') == std::string::npos) {
					if (booboo::prg->variables_map.find(line) == booboo::prg->variables_map.end()) {
						printf("No such function %s...\n", line.c_str());
					}
					else {
						booboo::function_breakpoints.push_back(line);
						printf("Breakpoint added!\n");
					}
				}
				else {
					if (prg->lines_with_instructions.find(line) == prg->lines_with_instructions.end()) {
						printf("No instruction on %s. No breakpoint set!\n", line.c_str());
					}
					else {
						file_breakpoints.push_back(line);
						printf("Breakpoint added!\n");
					}
				}
			}
		}
		else if (line == "step") {
			if (booboo::interpret(prg, false) == false) {
				return;
			}
			if (prg->s->pc >= prg->s->program.size()) {
				return;
			}
		}
		else if (line == "stepin") {
			break_on_interpret = true;
			if (booboo::interpret(prg, false) == false) {
				break_on_interpret = false;
				return;
			}
			if (prg->s->pc >= prg->s->program.size()) {
				break_on_interpret = false;
				return;
			}
			break_on_interpret = false;
		}
		else if (line.substr(0, 2) == "bt") {
			line = line.substr(2);
			line = util::trim(line);
			printed_lines = true;
			int start = (int)backtrace.size() - 23;
			start = MAX(0, start);
			if (line == "all") {
				start = 0;
			}
			printf("Backtrace:\n");
			if (start > 0) {
				printf("... %d more\n", start);
			}
			int digits = log10(backtrace.size()) + 1;
			for (size_t i = start; i < backtrace.size(); i++) {
				std::string fmt = std::string("%") + util::itos(digits) + "d %s\n";
				printf(fmt.c_str(), i-start, backtrace[i].c_str());
			}
		}
		else if (line == "skip") {
			prg->s->pc++;
		}
		else if (line.substr(0, 5) == "watch") {
			printed_lines = true;
			line = line.substr(5);
			line = util::trim(line);
			if (line.substr(0, 6) == "delete") {
				line = line.substr(6);
				line = util::trim(line);
				int index = atoi(line.c_str());
				if (index >= 0 && index < watchpoints.size()) {
					watchpoints.erase(watchpoints.begin()+index);
					printf("Watch deleted!\n");
				}
				else {
					printf("No such watchpoint...\n");
				}
			}
			else {
				std::string dest, src;
				int i = 0;
				if (line[0] == '[') {
					int open = 0;
					while (i < (int)line.length()) {
						if (line[i] == '[') {
							open++;
						}
						else if (line[i] == ']') {
							open--;
						}
						i++;
						if (open == 0) {
							break;
						}
					}
					dest = line.substr(0, i);
					dest = util::trim(dest);
				}
				else {
					while (i < (int)line.length()) {
						char buf[2];
						buf[0] = line[i];
						buf[1] = 0;
						dest += buf;
						if (isspace(line[i])) {
							break;
						}
						i++;
					}
					dest = util::trim(dest);
				}
				src = line.substr(i);
				src = util::trim(src);
				if (src[0] != '(') {
					printf("Invalid watchpoint...\n");
				}
				else {
					booboo::Variable::Expression f = booboo::parse_expression(booboo::prg, booboo::prg_func, src, booboo::PASS1);
					f = booboo::parse_expression(booboo::prg, booboo::prg_func, src, booboo::PASS2);
					Watchpoint w;
					w.p = get_var(dest, false);
					w.e = f;
					w.var = dest;
					w.expr = src;
					watchpoints.push_back(w);
					printf("Watchpoint added!\n");
				}
			}
		}
		else {
			printf("Unknown command...\n");
		}
	}

	count--;
	if (count == 0) {
		while (prg->variables.size() > prg->num_vars) {
			prg->variables.pop_back();
		}
	}
}

} // end namespace booboo

