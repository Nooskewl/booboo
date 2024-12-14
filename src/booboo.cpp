#include <climits>

#include <fstream>
#include <sstream>

#include <sys/stat.h>

#include <shim5/shim5.h>

#include <libutil/libutil.h>
using namespace noo;

#include <twinkle.h>

#include "booboo/booboo.h"
#include "booboo/internal.h"
#include "booboo/game_lib.h"

static std::map<std::string, int> library_map;
static std::map<char, booboo::token_func> token_map;
static std::map<std::string, int> expression_map;
static std::vector<booboo::expression_func> expression_handlers;

// First off maybe 10 utility functions taken from Nooskewl Shim

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

std::string reset_game_name;
std::string main_program_name;
int return_code;
bool quit;
std::string (*load_text)(std::string filename);
std::vector<booboo::library_func> library;

// And this all makes BooBoo work

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
	return get_file_name(prg) + ":" + itos(get_line_num(prg));
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

static std::string tokenfunc_add(Program *prg)
{
	prg->s->p++;
	return "+";
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

static std::string tokenfunc_multiply(Program *prg)
{
	prg->s->p++;
	return "*";
}

static std::string tokenfunc_divide(Program *prg)
{
	prg->s->p++;
	return "/";
}

static std::string tokenfunc_modulus(Program *prg)
{
	prg->s->p++;
	return "%";
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

static std::string tokenfunc_or(Program *prg)
{
	prg->s->p++;
	return "|";
}

static std::string tokenfunc_xor(Program *prg)
{
	prg->s->p++;
	return "^";
}

static std::string tokenfunc_and(Program *prg)
{
	prg->s->p++;
	return "&";
}

static std::string tokenfunc_leftshift(Program *prg)
{
	prg->s->p++;
	if (prg->s->p < prg->s->code.length()-1) {
		if (prg->s->code[prg->s->p] != '<') {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid token at " + get_error_info(prg));
		}
	}
	prg->s->p++;
	return "<<";
}

static std::string tokenfunc_rightshift(Program *prg)
{
	prg->s->p++;
	if (prg->s->p < prg->s->code.length()-1) {
		if (prg->s->code[prg->s->p] != '>') {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid token at " + get_error_info(prg));
		}
	}
	prg->s->p++;
	return ">>";
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

static std::string tokenfunc_deref(Program *prg)
{
	prg->s->p++;
	return "`";
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
	else {
		ret_type = Token::STRING;
	}

	std::map<char, token_func>::iterator it = token_map.find(prg->s->code[prg->s->p]);
	if (it != token_map.end()) {
		return (*it).second(prg);
	}

	throw Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg));

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
		if (tok == ";") {
			while (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] != '\n') {
				prg->s->p++;
			}
			prg->s->line++;
			if (prg->s->p < prg->s->code.length()) {
				prg->s->p++;
			}
		}
		else if (tok == "include") {
			int start_line = prg->s->line;

			std::string name = token(prg, tt);

			if (name == "") {
				throw Error(std::string(__FUNCTION__) + ": " + "Expected include parameters at " + get_error_info(prg));
			}

			if (name[0] != '"') {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid include name at " + get_error_info(prg));
			}

			name = util::remove_quotes(util::unescape_string(name));

			while (isspace(prg->s->code[prev])) {
				prev++;
			}

			code += prg->s->code.substr(start, prev-start);

			std::string new_code;
			std::string fn;
			fn = name;
			new_code = booboo::load_text("scripts/" + name);

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
	prg->backup.clear();
	std::map<std::string, int>::iterator it;
	for (it = prg->locals[func].begin(); it != prg->locals[func].end(); it++) {
		std::pair<std::string, int> pair = *it;
		if (prg->variables_map.find(pair.first) != prg->variables_map.end()) {
			prg->backup[pair.first] = prg->variables_map[pair.first];
		}
		if (restore_locals || IS_LABEL(prg->variables[pair.second])) {
			prg->variables_map[pair.first] = pair.second;
		}
	}
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
	for (it = prg->backup.begin(); it != prg->backup.end(); it++) {
		std::pair<std::string, int> pair = *it;
		prg->variables_map[pair.first] = pair.second;
	}
}

static Variable::Fish parse_fish(Program *prg, Program *func, std::string expr, int &var_i, int &expression_i, int &fish_i, Pass pass);

static Variable::Expression parse_expression(Program *prg, Program *func, std::string expr, int &var_i, int &expression_i, int &fish_i, Pass pass)
{
	int p = 0;

	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	if (p >= (int)expr.length()-1) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid expression at " + get_error_info(prg));
	}
	p++; // skip (
	std::string name;
	while (p < (int)expr.length() && !isspace(expr[p]) && expr[p] != '(' && expr[p] != '[') {
		char buf[2];
		buf[0] = expr[p];
		buf[1] = 0;
		name += buf;
		p++;
	}

	Variable::Expression e;

	if (expression_map.find(name) == expression_map.end()) {
		e.i = -1;
		e.name = name;
	}
	else {
		e.i = expression_map[name];
	}

	bool done = false;

	bool deref = false;

	while (!done) {
		while (isspace(expr[p]) && p < (int)expr.length()) {
			p++;
		}
		if (p >= (int)expr.length()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid expression at " + get_error_info(prg));
		}
		char c = expr[p];
		Token tok;
		if (c == '(') {
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
			tok.i = var_i++;
			tok.token = new_expr;
			tok.dereference = deref;
			deref = false;

			Variable v1;
			v1.name = "__expr" + itos(expression_i++);
			v1.type = Variable::EXPRESSION;

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].e = parse_expression(prg, func, new_expr, var_i, expression_i, fish_i, pass);
		}
		else if (c == ')') {
			done = true;
			deref = false;
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
			tok.i = var_i++;
			tok.token = new_expr;
			tok.dereference = deref;
			deref = false;

			Variable v1;
			v1.name = "__fish" + itos(fish_i++);
			v1.type = Variable::FISH;

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].f = parse_fish(prg, func, new_expr, var_i, expression_i, fish_i, pass);
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
			tok.token = str;
			tok.n = atof(str.c_str());
			tok.dereference = false;
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
			tok.token = str;
			tok.s = str;
			tok.dereference = false;
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
			tok.token = sym;
			tok.dereference = deref;
			deref = false;

			if (pass == PASS2) {
				if (prg->variables_map.find(sym) == prg->variables_map.end()) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg));
				}
				tok.i = prg->variables_map[sym];
			}
		}
		else if (c == '`') {
			deref = true;
			p++;
			continue;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg));
		}

		e.v.push_back(tok);
	}

	return e;
}

static Variable::Fish parse_fish(Program *prg, Program *func, std::string expr, int &var_i, int &expression_i, int &fish_i, Pass pass)
{
	int p = 0;
	Variable::Fish e;

	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	if (p >= (int)expr.length()-1) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid fish at " + get_error_info(prg));
	}
	p++; // skip [
	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
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
		v.name = "__fish" + itos(fish_i++);
		v.type = Variable::FISH;

		if (pass == PASS1) {
			prg->variables.push_back(v);
		}
		else if (pass == PASS2) {
			prg->variables_map[v.name] = var_i;
		}
		e.c_i = var_i;
		var_i++;

		std::string s = expr.substr(start, p-start);
		prg->variables[e.c_i].f = parse_fish(prg, func, s, var_i, expression_i, fish_i, pass);
	}
	else {
		std::string name;
		while (!isspace(expr[p]) && p < (int)expr.length()) {
			char buf[2];
			buf[0] = expr[p];
			buf[1] = 0;
			name += buf;
			p++;
		}
		if (pass == PASS2) {
			if (prg->variables_map.find(name) == prg->variables_map.end()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Unknown variable at " + get_error_info(prg));
			}
			e.c_i = prg->variables_map[name];
		}
	}

	bool done = false;

	bool deref = false;

	while (!done) {
		while (isspace(expr[p]) && p < (int)expr.length()) {
			p++;
		}
		if (p >= (int)expr.length()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid fish at " + get_error_info(prg));
		}
		char c = expr[p];
		Token tok;
		if (c == '(') {
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
			tok.i = var_i++;
			tok.token = new_expr;
			tok.dereference = deref;
			deref = false;

			Variable v1;
			v1.name = "__expr" + itos(expression_i++);
			v1.type = Variable::EXPRESSION;

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].e = parse_expression(prg, func, new_expr, var_i, expression_i, fish_i, pass);
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
			tok.i = var_i++;
			tok.token = new_expr;
			tok.dereference = deref;
			deref = false;

			Variable v1;
			v1.name = "__fish" + itos(fish_i++);
			v1.type = Variable::FISH;

			if (pass == PASS1) {
				prg->variables.push_back(v1);
			}
			else if (pass == PASS2) {
				prg->variables_map[v1.name] = tok.i;
			}

			prg->variables[tok.i].f = parse_fish(prg, func, new_expr, var_i, expression_i, fish_i, pass);
		}
		else if (c == ']') {
			done = true;
			deref = false;
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
			tok.token = str;
			tok.n = atof(str.c_str());
			tok.dereference = false;
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
			tok.token = str;
			tok.s = str;
			tok.dereference = false;
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
			tok.token = sym;
			tok.dereference = deref;
			deref = false;

			if (pass == PASS2) {
				if (prg->variables_map.find(sym) == prg->variables_map.end()) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(prg));
				}
				tok.i = prg->variables_map[sym];
			}
		}
		else if (c == '`') {
			deref = true;
			p++;
			continue;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg));
		}

		e.v.push_back(tok);
	}

	return e;
}

static void insert_constant(Program *prg, std::string name, double value, Pass pass, int &var_i)
{
	int var_index = var_i;
	var_i++;
	if (pass == PASS2) {
		prg->variables_map[name] = var_index;
	}
	Variable v;
	v.name = name;
	v.type = Variable::NUMBER;
	v.n = value;
	if (pass == PASS1) {
		prg->variables.push_back(v);
	}
}

static void compile(Program *prg, Pass pass)
{
	int p_bak = prg->s->p;
	int line_bak = prg->s->line;

	std::string tok;
	Token::Token_Type tt;

	int var_i = 0;
	int func_i = 0;
	int expression_i = 0;
	int fish_i = 0;

	// Constants
	insert_constant(prg, "TRUE", 1, pass, var_i);
	insert_constant(prg, "FALSE", 0, pass, var_i);
	insert_constant(prg, "PI", M_PI, pass, var_i);
	insert_constant(prg, "E", M_E, pass, var_i);
	insert_constant(prg, "BLACK", 0, pass, var_i);
	insert_constant(prg, "BLUE", 1, pass, var_i);
	insert_constant(prg, "GREEN", 2, pass, var_i);
	insert_constant(prg, "CYAN", 3, pass, var_i);
	insert_constant(prg, "RED", 4, pass, var_i);
	insert_constant(prg, "PURPLE", 5, pass, var_i);
	insert_constant(prg, "YELLOW", 6, pass, var_i);
	insert_constant(prg, "WHITE", 7, pass, var_i);
#ifndef CLI
	insert_constant(prg, "KEY_UNKNOWN", 0, pass, var_i);
	insert_constant(prg, "KEY_RETURN", 13, pass, var_i);
	insert_constant(prg, "KEY_ESCAPE", 27, pass, var_i);
	insert_constant(prg, "KEY_BACKSPACE", 8, pass, var_i);
	insert_constant(prg, "KEY_TAB", 9, pass, var_i);
	insert_constant(prg, "KEY_SPACE", 32, pass, var_i);
	insert_constant(prg, "KEY_EXCLAIM", 33, pass, var_i);
	insert_constant(prg, "KEY_QUOTEDBL", 34, pass, var_i);
	insert_constant(prg, "KEY_HASH", 35, pass, var_i);
	insert_constant(prg, "KEY_PERCENT", 37, pass, var_i);
	insert_constant(prg, "KEY_DOLLAR", 36, pass, var_i);
	insert_constant(prg, "KEY_AMPERSAND", 38, pass, var_i);
	insert_constant(prg, "KEY_QUOTE", 39, pass, var_i);
	insert_constant(prg, "KEY_LEFTPAREN", 40, pass, var_i);
	insert_constant(prg, "KEY_RIGHTPAREN", 41, pass, var_i);
	insert_constant(prg, "KEY_ASTERISK", 42, pass, var_i);
	insert_constant(prg, "KEY_PLUS", 43, pass, var_i);
	insert_constant(prg, "KEY_COMMA", 44, pass, var_i);
	insert_constant(prg, "KEY_MINUS", 45, pass, var_i);
	insert_constant(prg, "KEY_PERIOD", 46, pass, var_i);
	insert_constant(prg, "KEY_SLASH", 47, pass, var_i);
	insert_constant(prg, "KEY_0", 48, pass, var_i);
	insert_constant(prg, "KEY_1", 49, pass, var_i);
	insert_constant(prg, "KEY_2", 50, pass, var_i);
	insert_constant(prg, "KEY_3", 51, pass, var_i);
	insert_constant(prg, "KEY_4", 52, pass, var_i);
	insert_constant(prg, "KEY_5", 53, pass, var_i);
	insert_constant(prg, "KEY_6", 54, pass, var_i);
	insert_constant(prg, "KEY_7", 55, pass, var_i);
	insert_constant(prg, "KEY_8", 56, pass, var_i);
	insert_constant(prg, "KEY_9", 57, pass, var_i);
	insert_constant(prg, "KEY_COLON", 58, pass, var_i);
	insert_constant(prg, "KEY_SEMICOLON", 59, pass, var_i);
	insert_constant(prg, "KEY_LESS", 60, pass, var_i);
	insert_constant(prg, "KEY_EQUALS", 61, pass, var_i);
	insert_constant(prg, "KEY_GREATER", 62, pass, var_i);
	insert_constant(prg, "KEY_QUESTION", 63, pass, var_i);
	insert_constant(prg, "KEY_AT", 64, pass, var_i);
	insert_constant(prg, "KEY_LEFTBRACKET", 91, pass, var_i);
	insert_constant(prg, "KEY_BACKSLASH", 92, pass, var_i);
	insert_constant(prg, "KEY_RIGHTBRACKET", 93, pass, var_i);
	insert_constant(prg, "KEY_CARET", 94, pass, var_i);
	insert_constant(prg, "KEY_UNDERSCORE", 95, pass, var_i);
	insert_constant(prg, "KEY_BACKQUOTE", 96, pass, var_i);
	insert_constant(prg, "KEY_a", 97, pass, var_i);
	insert_constant(prg, "KEY_b", 98, pass, var_i);
	insert_constant(prg, "KEY_c", 99, pass, var_i);
	insert_constant(prg, "KEY_d", 100, pass, var_i);
	insert_constant(prg, "KEY_e", 101, pass, var_i);
	insert_constant(prg, "KEY_f", 102, pass, var_i);
	insert_constant(prg, "KEY_g", 103, pass, var_i);
	insert_constant(prg, "KEY_h", 104, pass, var_i);
	insert_constant(prg, "KEY_i", 105, pass, var_i);
	insert_constant(prg, "KEY_j", 106, pass, var_i);
	insert_constant(prg, "KEY_k", 107, pass, var_i);
	insert_constant(prg, "KEY_l", 108, pass, var_i);
	insert_constant(prg, "KEY_m", 109, pass, var_i);
	insert_constant(prg, "KEY_n", 110, pass, var_i);
	insert_constant(prg, "KEY_o", 111, pass, var_i);
	insert_constant(prg, "KEY_p", 112, pass, var_i);
	insert_constant(prg, "KEY_q", 113, pass, var_i);
	insert_constant(prg, "KEY_r", 114, pass, var_i);
	insert_constant(prg, "KEY_s", 115, pass, var_i);
	insert_constant(prg, "KEY_t", 116, pass, var_i);
	insert_constant(prg, "KEY_u", 117, pass, var_i);
	insert_constant(prg, "KEY_v", 118, pass, var_i);
	insert_constant(prg, "KEY_w", 119, pass, var_i);
	insert_constant(prg, "KEY_x", 120, pass, var_i);
	insert_constant(prg, "KEY_y", 121, pass, var_i);
	insert_constant(prg, "KEY_z", 122, pass, var_i);
	insert_constant(prg, "KEY_CAPSLOCK", 1073741881, pass, var_i);
	insert_constant(prg, "KEY_F1", 1073741882, pass, var_i);
	insert_constant(prg, "KEY_F2", 1073741883, pass, var_i);
	insert_constant(prg, "KEY_F3", 1073741884, pass, var_i);
	insert_constant(prg, "KEY_F4", 1073741885, pass, var_i);
	insert_constant(prg, "KEY_F5", 1073741886, pass, var_i);
	insert_constant(prg, "KEY_F6", 1073741887, pass, var_i);
	insert_constant(prg, "KEY_F7", 1073741888, pass, var_i);
	insert_constant(prg, "KEY_F8", 1073741889, pass, var_i);
	insert_constant(prg, "KEY_F9", 1073741890, pass, var_i);
	insert_constant(prg, "KEY_F10", 1073741891, pass, var_i);
	insert_constant(prg, "KEY_F11", 1073741892, pass, var_i);
	insert_constant(prg, "KEY_F12", 1073741893, pass, var_i);
	insert_constant(prg, "KEY_PRINTSCREEN", 1073741894, pass, var_i);
	insert_constant(prg, "KEY_SCROLLLOCK", 1073741895, pass, var_i);
	insert_constant(prg, "KEY_PAUSE", 1073741896, pass, var_i);
	insert_constant(prg, "KEY_INSERT", 1073741897, pass, var_i);
	insert_constant(prg, "KEY_HOME", 1073741898, pass, var_i);
	insert_constant(prg, "KEY_PAGEUP", 1073741899, pass, var_i);
	insert_constant(prg, "KEY_DELETE", 127, pass, var_i);
	insert_constant(prg, "KEY_END", 1073741901, pass, var_i);
	insert_constant(prg, "KEY_PAGEDOWN", 1073741902, pass, var_i);
	insert_constant(prg, "KEY_RIGHT", 1073741903, pass, var_i);
	insert_constant(prg, "KEY_LEFT", 1073741904, pass, var_i);
	insert_constant(prg, "KEY_DOWN", 1073741905, pass, var_i);
	insert_constant(prg, "KEY_UP", 1073741906, pass, var_i);
	insert_constant(prg, "KEY_NUMLOCKCLEAR", 1073741907, pass, var_i);
	insert_constant(prg, "KEY_KP_DIVIDE", 1073741908, pass, var_i);
	insert_constant(prg, "KEY_KP_MULTIPLY", 1073741909, pass, var_i);
	insert_constant(prg, "KEY_KP_MINUS", 1073741910, pass, var_i);
	insert_constant(prg, "KEY_KP_PLUS", 1073741911, pass, var_i);
	insert_constant(prg, "KEY_KP_ENTER", 1073741912, pass, var_i);
	insert_constant(prg, "KEY_KP_1", 1073741913, pass, var_i);
	insert_constant(prg, "KEY_KP_2", 1073741914, pass, var_i);
	insert_constant(prg, "KEY_KP_3", 1073741915, pass, var_i);
	insert_constant(prg, "KEY_KP_4", 1073741916, pass, var_i);
	insert_constant(prg, "KEY_KP_5", 1073741917, pass, var_i);
	insert_constant(prg, "KEY_KP_6", 1073741918, pass, var_i);
	insert_constant(prg, "KEY_KP_7", 1073741919, pass, var_i);
	insert_constant(prg, "KEY_KP_8", 1073741920, pass, var_i);
	insert_constant(prg, "KEY_KP_9", 1073741921, pass, var_i);
	insert_constant(prg, "KEY_KP_0", 1073741922, pass, var_i);
	insert_constant(prg, "KEY_KP_PERIOD", 1073741923, pass, var_i);
	insert_constant(prg, "KEY_APPLICATION", 1073741925, pass, var_i);
	insert_constant(prg, "KEY_POWER", 1073741926, pass, var_i);
	insert_constant(prg, "KEY_KP_EQUALS", 1073741927, pass, var_i);
	insert_constant(prg, "KEY_F13", 1073741928, pass, var_i);
	insert_constant(prg, "KEY_F14", 1073741929, pass, var_i);
	insert_constant(prg, "KEY_F15", 1073741930, pass, var_i);
	insert_constant(prg, "KEY_F16", 1073741931, pass, var_i);
	insert_constant(prg, "KEY_F17", 1073741932, pass, var_i);
	insert_constant(prg, "KEY_F18", 1073741933, pass, var_i);
	insert_constant(prg, "KEY_F19", 1073741934, pass, var_i);
	insert_constant(prg, "KEY_F20", 1073741935, pass, var_i);
	insert_constant(prg, "KEY_F21", 1073741936, pass, var_i);
	insert_constant(prg, "KEY_F22", 1073741937, pass, var_i);
	insert_constant(prg, "KEY_F23", 1073741938, pass, var_i);
	insert_constant(prg, "KEY_F24", 1073741939, pass, var_i);
	insert_constant(prg, "KEY_EXECUTE", 1073741940, pass, var_i);
	insert_constant(prg, "KEY_HELP", 1073741941, pass, var_i);
	insert_constant(prg, "KEY_MENU", 1073741942, pass, var_i);
	insert_constant(prg, "KEY_SELECT", 1073741943, pass, var_i);
	insert_constant(prg, "KEY_STOP", 1073741944, pass, var_i);
	insert_constant(prg, "KEY_AGAIN", 1073741945, pass, var_i);
	insert_constant(prg, "KEY_UNDO", 1073741946, pass, var_i);
	insert_constant(prg, "KEY_CUT", 1073741947, pass, var_i);
	insert_constant(prg, "KEY_COPY", 1073741948, pass, var_i);
	insert_constant(prg, "KEY_PASTE", 1073741949, pass, var_i);
	insert_constant(prg, "KEY_FIND", 1073741950, pass, var_i);
	insert_constant(prg, "KEY_MUTE", 1073741951, pass, var_i);
	insert_constant(prg, "KEY_VOLUMEUP", 1073741952, pass, var_i);
	insert_constant(prg, "KEY_VOLUMEDOWN", 1073741953, pass, var_i);
	insert_constant(prg, "KEY_KP_COMMA", 1073741957, pass, var_i);
	insert_constant(prg, "KEY_KP_EQUALSAS400", 1073741958, pass, var_i);
	insert_constant(prg, "KEY_ALTERASE", 1073741977, pass, var_i);
	insert_constant(prg, "KEY_SYSREQ", 1073741978, pass, var_i);
	insert_constant(prg, "KEY_CANCEL", 1073741979, pass, var_i);
	insert_constant(prg, "KEY_CLEAR", 1073741980, pass, var_i);
	insert_constant(prg, "KEY_PRIOR", 1073741981, pass, var_i);
	insert_constant(prg, "KEY_RETURN2", 1073741982, pass, var_i);
	insert_constant(prg, "KEY_SEPARATOR", 1073741983, pass, var_i);
	insert_constant(prg, "KEY_OUT", 1073741984, pass, var_i);
	insert_constant(prg, "KEY_OPER", 1073741985, pass, var_i);
	insert_constant(prg, "KEY_CLEARAGAIN", 1073741986, pass, var_i);
	insert_constant(prg, "KEY_CRSEL", 1073741987, pass, var_i);
	insert_constant(prg, "KEY_EXSEL", 1073741988, pass, var_i);
	insert_constant(prg, "KEY_KP_00", 1073742000, pass, var_i);
	insert_constant(prg, "KEY_KP_000", 1073742001, pass, var_i);
	insert_constant(prg, "KEY_THOUSANDSSEPARATOR", 1073742002, pass, var_i);
	insert_constant(prg, "KEY_DECIMALSEPARATOR", 1073742003, pass, var_i);
	insert_constant(prg, "KEY_CURRENCYUNIT", 1073742004, pass, var_i);
	insert_constant(prg, "KEY_CURRENCYSUBUNIT", 1073742005, pass, var_i);
	insert_constant(prg, "KEY_KP_LEFTPAREN", 1073742006, pass, var_i);
	insert_constant(prg, "KEY_KP_RIGHTPAREN", 1073742007, pass, var_i);
	insert_constant(prg, "KEY_KP_LEFTBRACE", 1073742008, pass, var_i);
	insert_constant(prg, "KEY_KP_RIGHTBRACE", 1073742009, pass, var_i);
	insert_constant(prg, "KEY_KP_TAB", 1073742010, pass, var_i);
	insert_constant(prg, "KEY_KP_BACKSPACE", 1073742011, pass, var_i);
	insert_constant(prg, "KEY_KP_A", 1073742012, pass, var_i);
	insert_constant(prg, "KEY_KP_B", 1073742013, pass, var_i);
	insert_constant(prg, "KEY_KP_C", 1073742014, pass, var_i);
	insert_constant(prg, "KEY_KP_D", 1073742015, pass, var_i);
	insert_constant(prg, "KEY_KP_E", 1073742016, pass, var_i);
	insert_constant(prg, "KEY_KP_F", 1073742017, pass, var_i);
	insert_constant(prg, "KEY_KP_XOR", 1073742018, pass, var_i);
	insert_constant(prg, "KEY_KP_POWER", 1073742019, pass, var_i);
	insert_constant(prg, "KEY_KP_PERCENT", 1073742020, pass, var_i);
	insert_constant(prg, "KEY_KP_LESS", 1073742021, pass, var_i);
	insert_constant(prg, "KEY_KP_GREATER", 1073742022, pass, var_i);
	insert_constant(prg, "KEY_KP_AMPERSAND", 1073742023, pass, var_i);
	insert_constant(prg, "KEY_KP_DBLAMPERSAND", 1073742024, pass, var_i);
	insert_constant(prg, "KEY_KP_VERTICALBAR", 1073742025, pass, var_i);
	insert_constant(prg, "KEY_KP_DBLVERTICALBAR", 1073742026, pass, var_i);
	insert_constant(prg, "KEY_KP_COLON", 1073742027, pass, var_i);
	insert_constant(prg, "KEY_KP_HASH", 1073742028, pass, var_i);
	insert_constant(prg, "KEY_KP_SPACE", 1073742029, pass, var_i);
	insert_constant(prg, "KEY_KP_AT", 1073742030, pass, var_i);
	insert_constant(prg, "KEY_KP_EXCLAM", 1073742031, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMSTORE", 1073742032, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMRECALL", 1073742033, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMCLEAR", 1073742034, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMADD", 1073742035, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMSUBTRACT", 1073742036, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMMULTIPLY", 1073742037, pass, var_i);
	insert_constant(prg, "KEY_KP_MEMDIVIDE", 1073742038, pass, var_i);
	insert_constant(prg, "KEY_KP_PLUSMINUS", 1073742039, pass, var_i);
	insert_constant(prg, "KEY_KP_CLEAR", 1073742040, pass, var_i);
	insert_constant(prg, "KEY_KP_CLEARENTRY", 1073742041, pass, var_i);
	insert_constant(prg, "KEY_KP_BINARY", 1073742042, pass, var_i);
	insert_constant(prg, "KEY_KP_OCTAL", 1073742043, pass, var_i);
	insert_constant(prg, "KEY_KP_DECIMAL", 1073742044, pass, var_i);
	insert_constant(prg, "KEY_KP_HEXADECIMAL", 1073742045, pass, var_i);
	insert_constant(prg, "KEY_LCTRL", 1073742048, pass, var_i);
	insert_constant(prg, "KEY_LSHIFT", 1073742049, pass, var_i);
	insert_constant(prg, "KEY_LALT", 1073742050, pass, var_i);
	insert_constant(prg, "KEY_LGUI", 1073742051, pass, var_i);
	insert_constant(prg, "KEY_RCTRL", 1073742052, pass, var_i);
	insert_constant(prg, "KEY_RSHIFT", 1073742053, pass, var_i);
	insert_constant(prg, "KEY_RALT", 1073742054, pass, var_i);
	insert_constant(prg, "KEY_RGUI", 1073742055, pass, var_i);
	insert_constant(prg, "KEY_MODE", 1073742081, pass, var_i);
	insert_constant(prg, "KEY_AUDIONEXT", 1073742082, pass, var_i);
	insert_constant(prg, "KEY_AUDIOPREV", 1073742083, pass, var_i);
	insert_constant(prg, "KEY_AUDIOSTOP", 1073742084, pass, var_i);
	insert_constant(prg, "KEY_AUDIOPLAY", 1073742085, pass, var_i);
	insert_constant(prg, "KEY_AUDIOMUTE", 1073742086, pass, var_i);
	insert_constant(prg, "KEY_MEDIASELECT", 1073742087, pass, var_i);
	insert_constant(prg, "KEY_WWW", 1073742088, pass, var_i);
	insert_constant(prg, "KEY_MAIL", 1073742089, pass, var_i);
	insert_constant(prg, "KEY_CALCULATOR", 1073742090, pass, var_i);
	insert_constant(prg, "KEY_COMPUTER", 1073742091, pass, var_i);
	insert_constant(prg, "KEY_AC_SEARCH", 1073742092, pass, var_i);
	insert_constant(prg, "KEY_AC_HOME", 1073742093, pass, var_i);
	insert_constant(prg, "KEY_AC_BACK", 1073742094, pass, var_i);
	insert_constant(prg, "KEY_AC_FORWARD", 1073742095, pass, var_i);
	insert_constant(prg, "KEY_AC_STOP", 1073742096, pass, var_i);
	insert_constant(prg, "KEY_AC_REFRESH", 1073742097, pass, var_i);
	insert_constant(prg, "KEY_AC_BOOKMARKS", 1073742098, pass, var_i);
	insert_constant(prg, "KEY_BRIGHTNESSDOWN", 1073742099, pass, var_i);
	insert_constant(prg, "KEY_BRIGHTNESSUP", 1073742100, pass, var_i);
	insert_constant(prg, "KEY_DISPLAYSWITCH", 1073742101, pass, var_i);
	insert_constant(prg, "KEY_KBDILLUMTOGGLE", 1073742102, pass, var_i);
	insert_constant(prg, "KEY_KBDILLUMDOWN", 1073742103, pass, var_i);
	insert_constant(prg, "KEY_KBDILLUMUP", 1073742104, pass, var_i);
	insert_constant(prg, "KEY_EJECT", 1073742105, pass, var_i);
	insert_constant(prg, "KEY_SLEEP", 1073742106, pass, var_i);
	insert_constant(prg, "EVENT_KEY_DOWN", TGUI_KEY_DOWN, pass, var_i);
	insert_constant(prg, "EVENT_KEY_UP", TGUI_KEY_UP, pass, var_i);
	insert_constant(prg, "EVENT_JOY_DOWN", TGUI_JOY_DOWN, pass, var_i);
	insert_constant(prg, "EVENT_JOY_UP", TGUI_JOY_UP, pass, var_i);
	insert_constant(prg, "EVENT_JOY_AXIS", TGUI_JOY_AXIS, pass, var_i);
	insert_constant(prg, "EVENT_MOUSE_DOWN", TGUI_MOUSE_DOWN, pass, var_i);
	insert_constant(prg, "EVENT_MOUSE_UP", TGUI_MOUSE_UP, pass, var_i);
	insert_constant(prg, "EVENT_MOUSE_AXIS", TGUI_MOUSE_AXIS, pass, var_i);
	insert_constant(prg, "EVENT_MOUSE_WHEEL", TGUI_MOUSE_WHEEL, pass, var_i);
	insert_constant(prg, "JOY_A", TGUI_B_A, pass, var_i);
	insert_constant(prg, "JOY_B", TGUI_B_B, pass, var_i);
	insert_constant(prg, "JOY_X", TGUI_B_X, pass, var_i);
	insert_constant(prg, "JOY_Y", TGUI_B_Y, pass, var_i);
	insert_constant(prg, "JOY_BACK", TGUI_B_BACK, pass, var_i);
	insert_constant(prg, "JOY_GUIDE", TGUI_B_GUIDE, pass, var_i);
	insert_constant(prg, "JOY_START", TGUI_B_START, pass, var_i);
	insert_constant(prg, "JOY_LS", TGUI_B_LS, pass, var_i);
	insert_constant(prg, "JOY_RS", TGUI_B_RS, pass, var_i);
	insert_constant(prg, "JOY_LB", TGUI_B_LB, pass, var_i);
	insert_constant(prg, "JOY_RB", TGUI_B_RB, pass, var_i);
	insert_constant(prg, "JOY_U", TGUI_B_U, pass, var_i);
	insert_constant(prg, "JOY_D", TGUI_B_D, pass, var_i);
	insert_constant(prg, "JOY_L", TGUI_B_L, pass, var_i);
	insert_constant(prg, "JOY_R", TGUI_B_R, pass, var_i);
	insert_constant(prg, "TRANSITION_NONE", TRANSITION_NONE, pass, var_i);
	insert_constant(prg, "TRANSITION_ENLARGE", TRANSITION_ENLARGE, pass, var_i);
	insert_constant(prg, "TRANSITION_SHRINK", TRANSITION_SHRINK, pass, var_i);
	insert_constant(prg, "TRANSITION_SLIDE", TRANSITION_SLIDE, pass, var_i);
	insert_constant(prg, "TRANSITION_SLIDE_VERTICAL", TRANSITION_SLIDE_VERTICAL, pass, var_i);
	insert_constant(prg, "TRANSITION_FADE", TRANSITION_FADE, pass, var_i);
	insert_constant(prg, "BAR_TOP", gfx::BAR_TOP, pass, var_i);
	insert_constant(prg, "BAR_BOTTOM", gfx::BAR_BOTTOM, pass, var_i);
	insert_constant(prg, "BAR_LEFT", gfx::BAR_LEFT, pass, var_i);
	insert_constant(prg, "BAR_RIGHT", gfx::BAR_RIGHT, pass, var_i);
#endif

	bool _is_deref = false;

	while ((tok = token(prg, tt)) != "") {
top:
		if (tok == ";") {
			while (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] != '\n') {
				prg->s->p++;
			}
			prg->s->line++;
			if (prg->s->p < prg->s->code.length()) {
				prg->s->p++;
			}
		}
		else if (tok == "`") {
			_is_deref = true;
		}
		else if (tok == "function") {
			std::string func_name = token(prg, tt);

			int func_index = func_i;
			if (pass == PASS2) {
				backup(prg, func_index, false);
			}

			Variable v;
			v.name = func_name;
			v.type = Variable::FUNCTION;
			v.n = func_i++;
			std::map<std::string, int> tmp;
			prg->locals.push_back(tmp);
			prg->variables_map[func_name] = var_i++;
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
			bool is_deref = false;
			while ((tok = token(prg, tt)) != "") {
func_top:
				func.s->line = prg->s->line;
				if (tok == ";") {
					while (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] != '\n') {
						prg->s->p++;
					}
					prg->s->line++;
					func.s->line = prg->s->line;
					if (prg->s->p < prg->s->code.length()) {
						prg->s->p++;
					}
				}
				else if (tok == "`") {
					is_deref = true;
				}
				else if (tok == "{") {
					is_param = false;
					func.s->start_line = prg->s->line;
				}
				else if (tok == "}") {
					finished = true;
					break;
				}
				else if (tok == ":") {
					std::string tok2 = token(prg, tt);

					Variable v;
					v.name = tok2;
					v.type = Variable::LABEL;
					v.n = func.s->program.size();

					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					Token t;
					t.type = Token::SYMBOL;
					t.i = var_i;
					t.s = tok2;
					t.token = tok2;
					t.dereference = false;
					is_deref = false;
					func.s->program[func.s->program.size()-1].data.push_back(t);

					if (pass == PASS1) {
						if (prg->locals[func_index].find(tok2) != prg->locals[func_index].end()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Duplicate label " + tok2 + " at " + get_error_info(&func));
						}
						prg->locals[func_index][tok2] = var_i;
					}
					prg->variables.push_back(v);
					if (pass == PASS2) {
						prg->variables_map[tok2] = prg->locals[func_index][tok2];
					}
					var_i++;
				}
				else if (tok == "number" || tok == "string" || tok == "vector" || tok == "map" || tok == "pointer") {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					int count = 0;
					while (true) {
						std::string tok2 = token(prg, tt);
						if (tok2 == "" || (count != 0 && (tok2 == ";" || tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
							tok = tok2;
							goto func_top;
						}
						if (tok2[0] != '_' && !isalpha(tok2[0])) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok2 + " at " + get_error_info(&func));
						}
						count++;
						if (pass == PASS1) {
							prg->locals[func_index][tok2] = var_i++;
						}
						else {
							prg->variables_map[tok2] = prg->locals[func_index][tok2];
							var_i++;
						}
						Variable v;
						v.name = tok2;
						if (tok == "number") {
							v.type = Variable::NUMBER;
						}
						else if (tok == "string") {
							v.type = Variable::STRING;
						}
						else if (tok == "vector") {
							v.type = Variable::VECTOR;
						}
						else if (tok == "map") {
							v.type = Variable::MAP;
						}
						else { // pointer
							v.type = Variable::POINTER;
						}
						std::map<std::string, int>::iterator it;
						it = prg->variables_map.find(tok2);
						if (it != prg->variables_map.end()) {
							Variable &var = prg->variables[(*it).second];
							if (var.type != v.type) {
								throw Error("Type for " + tok2 + " changed at " + get_error_info(&func));
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
						t.token = tok2;
						t.dereference = false;
						is_deref = false;
						func.s->program[func.s->program.size()-1].data.push_back(t);
					}
				}
				else if (tok[0] == '(') {
					Variable v;
					v.name = "__expr" + itos(expression_i++);
					v.type = Variable::EXPRESSION;

					int var_index = var_i;
					var_i++;

					if (pass == PASS1) {
						prg->locals[func_index][v.name] = var_index;
						prg->variables.push_back(v);
					}
					else {
						prg->variables_map[v.name] = prg->locals[func_index][v.name];
					}

					prg->variables[var_index].e = parse_expression(prg, &func, tok, var_i, expression_i, fish_i, pass);

					Token t;
					t.token = tok;
					t.type = Token::SYMBOL;
					t.s = v.name;
					if (pass == PASS2) {
						t.i = prg->variables_map[v.name];
					}
					t.dereference = is_deref;
					is_deref = false;

					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
				else if (tok[0] == '[') {
					Variable v;
					v.name = "__fish" + itos(fish_i++);
					v.type = Variable::FISH;

					int var_index = var_i;
					var_i++;

					if (pass == PASS1) {
						prg->locals[func_index][v.name] = var_index;
						prg->variables.push_back(v);
					}
					else {
						prg->variables_map[v.name] = prg->locals[func_index][v.name];
					}

					prg->variables[var_index].f = parse_fish(prg, &func, tok, var_i, expression_i, fish_i, pass);

					Token t;
					t.token = tok;
					t.type = Token::SYMBOL;
					t.s = v.name;
					if (pass == PASS2) {
						t.i = prg->variables_map[v.name];
					}
					t.dereference = is_deref;
					is_deref = false;

					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
				else if (library_map.find(tok) != library_map.end()) {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					func.s->line_numbers.push_back(prg->s->line);
					is_deref = false;
				}
				else if (is_param) {
					if (tok == "~") {
						param_is_ref = true;
					}
					else {
						int param_i = var_i++;
						if (pass == PASS1) {
							prg->locals[func_index][tok] = param_i;
						}
						else {
							prg->variables_map[tok] = prg->locals[func_index][tok];
						}
						Variable v;
						v.name = tok;
						if (pass == PASS1) {
							prg->variables.push_back(v);
						}

						func.params.push_back(param_i);
						func.ref.push_back(param_is_ref);
						param_is_ref = false;
						is_deref = false;
					}
				}
				else {
					if (func.s->program.size() == 0) {
						throw Error("Expected keyword at " + get_error_info(&func));
					}
					Token t;
					t.token = tok;
					t.type = tt;
					switch (tt) {
						case Token::STRING:
							t.s = util::remove_quotes(util::unescape_string(tok));
							break;
						case Token::SYMBOL:
							t.s = util::remove_quotes(util::unescape_string(tok));
							if (pass == PASS2 && prg->variables_map.find(t.s) == prg->variables_map.end()) {
								throw Error(std::string(__FUNCTION__) + ": " + "Invalid symbol name " + tok + " at " + get_error_info(&func));
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
					is_deref = false;
					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
			}

			if (is_param == true) {
				throw Error(std::string(__FUNCTION__) + ": " + "Missing { at " + get_error_info(prg));
			}

			if (finished == false) {
				throw Error(std::string(__FUNCTION__) + ": " + "Missing } at " + get_error_info(prg));
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
			v.type = Variable::LABEL;
			v.n = prg->s->program.size();

			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);

			Token t;
			t.type = Token::SYMBOL;
			t.i = var_i;
			t.s = tok2;
			t.token = tok2;
			t.dereference = false;
			_is_deref = false;
			prg->s->program[prg->s->program.size()-1].data.push_back(t);

			if (pass == PASS1 && prg->variables_map.find(tok2) != prg->variables_map.end()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Duplicate label " + tok2 + " at " + get_error_info(prg));
			}

			prg->s->line_numbers.push_back(prg->s->line);
			prg->variables.push_back(v);
			prg->variables_map[tok2] = var_i;
			var_i++;
		}
		else if (tok == "number" || tok == "string" || tok == "vector" || tok == "map" || tok == "pointer") {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			prg->s->line_numbers.push_back(prg->s->line);
			int count = 0;
			while (true) {
				std::string tok2 = token(prg, tt);
				if (tok2 == "" || (count != 0 && (tok2 == ";" || tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
					tok = tok2;
					goto top;
				}
				if (tok2[0] != '_' && !isalpha(tok2[0])) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok2 + " at " + get_error_info(prg));
				}
				count++;
				int var_index = var_i;
				var_i++;
				if (pass == PASS2) {
					prg->variables_map[tok2] = var_index;
				}
				Variable v;
				v.name = tok2;
				if (tok == "number") {
					v.type = Variable::NUMBER;
				}
				else if (tok == "string") {
					v.type = Variable::STRING;
				}
				else if (tok == "vector") {
					v.type = Variable::VECTOR;
				}
				else if (tok == "map") {
					v.type = Variable::MAP;
				}
				else { // pointer
					v.type = Variable::POINTER;
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
				t.token = tok2;
				t.dereference = false;
				_is_deref = false;
				prg->s->program[prg->s->program.size()-1].data.push_back(t);
			}
		}
		else if (tok[0] == '(') {
			Variable v;
			v.name = "__expr" + itos(expression_i++);
			v.type = Variable::EXPRESSION;

			int var_index = var_i;
			var_i++;

			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
			else if (pass == PASS2) {
				prg->variables_map[v.name] = var_index;
			}
			prg->variables[var_index].e = parse_expression(prg, prg, tok, var_i, expression_i, fish_i, pass);

			Token t;
			t.token = tok;
			t.type = Token::SYMBOL;
			t.s = v.name;
			if (pass == PASS2) {
				t.i = prg->variables_map[v.name];
			}
			t.dereference = _is_deref;
			_is_deref = false;

			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
		else if (tok[0] == '[') {
			Variable v;
			v.name = "__fish" + itos(fish_i++);
			v.type = Variable::FISH;

			int var_index = var_i;
			var_i++;

			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
			else if (pass == PASS2) {
				prg->variables_map[v.name] = var_index;
			}
			prg->variables[var_index].f = parse_fish(prg, prg, tok, var_i, expression_i, fish_i, pass);

			Token t;
			t.token = tok;
			t.type = Token::SYMBOL;
			t.s = v.name;
			if (pass == PASS2) {
				t.i = prg->variables_map[v.name];
			}
			t.dereference = _is_deref;
			_is_deref = false;

			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
		else if (library_map.find(tok) != library_map.end()) {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			prg->s->line_numbers.push_back(prg->s->line);
			_is_deref = false;
		}
		else if (prg->s->program.size() == 0) {
			throw Error("Expected keyword at " + get_error_info(prg));
		}
		else {
			Token t;
			t.token = tok;
			t.type = tt;
			switch (tt) {
				case Token::STRING:
					t.s = util::remove_quotes(util::unescape_string(tok));
					break;
				case Token::SYMBOL:
					t.s = util::remove_quotes(util::unescape_string(tok));
					if (pass == PASS2 && prg->variables_map.find(t.s) == prg->variables_map.end()) {
						throw Error(std::string(__FUNCTION__) + ": " + "Invalid symbol name " + tok + " at " + get_error_info(prg));
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
			_is_deref = false;
			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
	}

	prg->s->p = p_bak;
	prg->s->line = line_bak;

	prg->complete_pass = pass;
}

void call_function(Program *prg, int function, const std::vector<Token> &params, Variable &result, int ignore_params)
{
	Program &func = prg->functions[function];

	for (size_t j = 0; j < func.params.size(); j++) {
		const Token &param = params[j+ignore_params];
		
		Variable &var = prg->variables[func.params[j]];

		if (param.type == Token::NUMBER) {
			var.type = Variable::NUMBER;
			var.n = param.n;
			//var.name = param.token;
		}
		else if (param.type == Token::STRING) {
			var.type = Variable::STRING;
			var.s = param.s;
			//var.name = param.token;
		}
		else if (prg->variables[param.i].type == Variable::EXPRESSION) {
			var = evaluate_expression(prg, prg->variables[param.i].e);
		}
		else if (prg->variables[param.i].type == Variable::FISH) {
			var = go_fish(prg, prg->variables[param.i].f);
		}
		else {
			var = prg->variables[param.i];
		}
	}

	Function_Swap *bak2 = prg->s;
	prg->s = func.s;

	// To handle recursive calls:
	int pc_bak = prg->s->pc;

	prg->s->pc = 0;

	while (interpret(prg)) {
	}

	prg->s->pc = pc_bak;

	std::string bak = result.name;
	result = prg->s->result;
	result.name = bak;

	// keep values for references
	for (size_t j = 0; j < func.params.size(); j++) {
		const Token &param = params[j+ignore_params];
		
		Variable &var = prg->variables[func.params[j]];

		if (func.ref[j] && param.type != Token::NUMBER && param.type != Token::STRING && prg->variables[param.i].type != Variable::EXPRESSION) {
			if (prg->variables[param.i].type == Variable::FISH) {
				go_fish(prg, prg->variables[param.i].f) = var;
			}
			else {
				prg->variables[param.i] = var;
			}
		}
	}

	prg->s = bak2;
}

void call_function(Program *prg, std::string function_name, const std::vector<Token> &params, Variable &result, int ignore_params)
{
	std::map<std::string, int>::iterator it = prg->function_name_map.find(function_name);
	if (it == prg->function_name_map.end()) {
		return;
	}
	call_function(prg, (*it).second, params, result, ignore_params);
}

void call_void_function(Program *prg, int function, const std::vector<Token> &params, int ignore_params)
{
	static Variable tmp;
	call_function(prg, function, params, tmp, ignore_params);
}

void call_void_function(Program *prg, std::string function_name, const std::vector<Token> &params, int ignore_params)
{
	static Variable tmp;
	call_function(prg, function_name, params, tmp, ignore_params);
}

bool interpret(Program *prg)
{
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

	return ret;
}

void destroy_program(Program *prg)
{
	for (size_t i = 0; i < prg->functions.size(); i++) {
		delete prg->functions[i].s;
	}

	prg->variables.clear();
	prg->functions.clear();

	delete prg->s;
	prg->black_box.clear();
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

bool breaker_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	reset_game_name = as_string(prg, v[0]);

	return false;
}

bool breaker_exit(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	return_code = as_number(prg, v[0]);
	reset_game_name = "";
	quit = true;
	return false;
}

bool breaker_return(Program *prg, const std::vector<Token> &v)
{
	Variable &v1 = prg->s->result;

	if (v.size() > 0) {
		if (v[0].type == Token::NUMBER) {
			v1.type = Variable::NUMBER;
			v1.n = v[0].n;
		}
		else if (v[0].type == Token::STRING) {
			v1.type = Variable::STRING;
			v1.s = v[0].s;
		}
		else {
			Variable v2 = as_variable_resolve(prg, v[0]);

			v1 = v2;
		}
	}

	return false;
}

bool corefunc_number(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)
	return true;
}

bool corefunc_string(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)
	return true;
}

bool corefunc_vector(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)
	
	for (size_t i = 0; i < v.size(); i++) {
		prg->variables[v[i].i].v.clear();
	}

	return true;
}

bool corefunc_map(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)
	
	for (size_t i = 0; i < v.size(); i++) {
		prg->variables[v[i].i].m.clear();
	}
	return true;
}

bool corefunc_pointer(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)
	return true;
}

bool corefunc_set(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable *v1;

	if (v[0].dereference) {
		v1 = as_variable(prg, v[0]).p;
	}
	else {
		v1 = &as_variable(prg, v[0]);
	}

	if (v[1].type == Token::NUMBER) {
		v1->n = as_number(prg, v[1]);
	}
	else if (v[1].type == Token::STRING) {
		v1->s = as_string(prg, v[1]);
	}
	else {
		Variable v2 = as_variable_resolve(prg, v[1]);

		if (IS_NUMBER(v2)) {
			if (v1->type == Variable::NUMBER) {
				v1->n = v2.n;
			}
			else { // string
				char buf[1000];
				snprintf(buf, 1000, "%d", v2.n);
				v1->s = buf;
			}
		}
		else if (IS_STRING(v2)) {
			if (v1->type == Variable::NUMBER) {
				v1->n = atof(v2.s.c_str());
			}
			else { // string
				v1->s = v2.s;
			}
		}
		else if (IS_VECTOR(v2)) {
			if (IS_VECTOR(v2)) {
				v1->v = v2.v;
			}
			else {
				throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
			}
		}
		else if (IS_MAP(v2)) {
			if (IS_MAP(v2)) {
				v1->m = v2.m;
			}
			else {
				throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
			}
		}
		else if (IS_POINTER(v2)) {
			v1->p = v2.p;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
		}
	}

	return true;
}

bool corefunc_add(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (IS_NUMBER(v1)) {
			v1.n += as_number(prg, v[i]);
		}
		else if (IS_STRING(v1)) {
			v1.s += as_string(prg, v[i]);
		}
		else if (IS_VECTOR(v1)) {
			Variable &v2 = as_variable(prg, v[i]);
			if (IS_VECTOR(v2) == false) {
				throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
			}
			else {
				v1.v.insert(v1.v.end(), v2.v.begin(), v2.v.end());
			}
		}
		else if (IS_MAP(v1)) {
			Variable &v2 = as_variable(prg, v[i]);
			if (IS_MAP(v2) == false) {
				throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
			}
			else {
				v1.m.insert(v2.m.begin(), v2.m.end());
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
		}
	}

	return true;
}

bool corefunc_subtract(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);

	CHECK_NUMBER(v1)

	for (size_t i = 1; i < v.size(); i++) {
		v1.n -= as_number(prg, v[i]);
	}

	return true;
}

bool corefunc_multiply(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);

	CHECK_NUMBER(v1)	

	for (size_t i = 1; i < v.size(); i++) {
		v1.n *= as_number(prg, v[i]);
	}

	return true;
}

bool corefunc_divide(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);

	CHECK_NUMBER(v1)	

	for (size_t i = 1; i < v.size(); i++) {
		v1.n /= as_number(prg, v[i]);
	}

	return true;
}

bool corefunc_label(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	return true;
}

bool corefunc_goto(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	prg->s->pc = as_label(prg, v[0]);

	return true;
}

bool corefunc_compare(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	bool is_num = false;
	double n;

	if (v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_NUMBER(var)) {
			is_num = true;
			n = var.n;
		}
	}
	else if (v[0].type == Token::NUMBER) {
		is_num = true;
		n = v[0].n;
	}

	if (is_num) {
		double n2 = as_number(prg, v[1]);
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
			Variable var = as_variable_resolve(prg, v[0]);
			if (IS_STRING(var)) {
				a_string = true;
				s1 = var.s;
			}
		}
		
		if (v[1].type == Token::STRING) {
			b_string = true;
			s2 = v[1].s;
		}
		else if (v[1].type == Token::SYMBOL) {
			Variable var = as_variable_resolve(prg, v[1]);
			if (IS_STRING(var)) {
				b_string = true;
				s2 = var.s;
			}
		}
		
		if (a_string && b_string) {
			prg->compare_flag = strcmp(s1.c_str(), s2.c_str());
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool corefunc_je(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag == 0) {
		prg->s->pc = as_label(prg, v[0]);
	}

	return true;
}

bool corefunc_jne(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag != 0) {
		prg->s->pc = as_label(prg, v[0]);
	}

	return true;
}

bool corefunc_jl(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag < 0) {
		prg->s->pc = as_label(prg, v[0]);
	}

	return true;
}

bool corefunc_jle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag <= 0) {
		prg->s->pc = as_label(prg, v[0]);
	}

	return true;
}

bool corefunc_jg(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag > 0) {
		prg->s->pc = as_label(prg, v[0]);
	}

	return true;
}

bool corefunc_jge(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag >= 0) {
		prg->s->pc = as_label(prg, v[0]);
	}

	return true;
}

bool corefunc_call(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int function = as_function(prg, v[0]);

	call_void_function(prg, function, v, 1);

	return true;
}

bool corefunc_call_result(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &result = prg->variables[v[0].i];
	int function = as_function(prg, v[1]);

	call_function(prg, function, v, result, 2);

	return true;
}

static std::string typeof_var(Variable &v1)
{
	std::string res;
	if (IS_NUMBER(v1)) {
		res = "number";
	}
	else if (IS_STRING(v1)) {
		res = "string";
	}
	else if (IS_VECTOR(v1)) {
		res = "vector";
	}
	else if (IS_MAP(v1)) {
		res = "map";
	}
	else if (IS_FUNCTION(v1)) {
		res = "function";
	}
	else if (IS_LABEL(v1)) {
		res = "label";
	}
	else if (IS_POINTER(v1)) {
		res = "pointer";
	}
	else {
		res = "unknown";
	}

	return res;
}

bool corefunc_typeof(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	std::string res;

	if (v[1].type != Token::SYMBOL) {
		if (v[1].type == Token::NUMBER) {
			res = "number";
		}
		else if (v[1].type == Token::STRING) {
			res = "string";
		}
		else {
			res = "unknown";
		}
	}
	else {
		Variable &v1 = prg->variables[v[1].i];

		if (IS_VECTOR(v1)) {
			if (v.size() > 2) {
				std::vector<Variable> *p;
				p = &v1.v;
				int index = 0;
				for (size_t i = 2; i < v.size(); i++) {
					index = as_number(prg, v[i]);
					if (i < v.size()-1) {
						p = &(*p)[index].v;
					}
				}
				Variable &v2 = (*p)[index];
				res = typeof_var(v2);
			}
			else {
				res = "vector";
			}
		}
		else if (IS_MAP(v1)) {
			if (v.size() > 2) {
				std::map<std::string, Variable> *p;
				p = &v1.m;
				std::string key = "";
				for (size_t i = 2; i < v.size(); i++) {
					key = as_string(prg, v[i]);
					if (i < v.size()-1) {
						p = &(*p)[key].m;
					}
				}
				Variable &v2 = (*p)[key];
				res = typeof_var(v2);
			}
			else {
				res = "map";
			}
		}
		else {
			res = typeof_var(v1);
		}
	}

	Variable &v2 = as_variable(prg, v[0]);

	CHECK_STRING(v2)

	v2.s = res;

	return true;
}

bool corefunc_for(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(5)

	Variable &count = as_variable(prg, v[0]);

	CHECK_NUMBER(count)

	count.n = as_number(prg, v[1]);
	Variable &expr = as_variable(prg, v[2]);
	int increment = as_number(prg, v[3]);
	unsigned int end_label = as_label(prg, v[4]);

	CHECK_EXPRESSION(expr)

	Variable er = evaluate_expression(prg, expr.e);

	if (er.n == 0) {
		prg->s->pc = end_label;
		return true;
	}

	prg->s->pc++;

	unsigned int start = prg->s->pc;

	while (true) {
		if (interpret(prg) == false) {
			return false;
		}
		if (prg->s->pc < start || prg->s->pc > end_label) {
			break;
		}
		if (prg->s->pc == end_label) {
			count.n += increment;
			er = evaluate_expression(prg, expr.e);
			if (er.n == 0) {
				prg->s->pc++;
				break;
			}
			prg->s->pc = start;
		}
	}

	return true;
}

bool corefunc_if(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int end = v.size();
	end -= (end % 2);
	int prev = -1;

	for (int i = 0; i < end; i += 2) {
		bool b = as_number(prg, v[i]);
		if (b) {
			if (prev == -1) {
				prg->s->pc++;
			}
			else {
				prg->s->pc = prev;
			}
			unsigned int start = prg->s->pc;
			unsigned int end_block = as_label(prg, v[i+1]);
			while (prg->s->pc != end_block) {
				if (interpret(prg) == false) {
					return false;
				}
				if (prg->s->pc < start || prg->s->pc > end_block) {
					return true;
				}
			}
			prg->s->pc = as_label(prg, v[v.size()-1]);
			return true;
		}
		prev = as_label(prg, v[i+1]);
	}

	if (v.size() <= 2 || v.size() % 2 == 1) {
		prg->s->pc = prev;
		unsigned int start = prg->s->pc;
		unsigned int end_block = as_label(prg, v[v.size()-1]);
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
		unsigned int end_block = as_label(prg, v[v.size()-1]);
		prg->s->pc = end_block;
	}

	return true;
}

bool corefunc_address(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &var = as_variable(prg, v[0]);

	if (IS_POINTER(var) == false) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected pointer at " + get_error_info(prg));
	}

	var.p = &as_variable(prg, v[1]);

	return true;
}

Variable exprfunc_add(Program *prg, const std::vector<Token> &v)
{
	Variable ret;
	ret.name = "-booboo-";

	if (v[0].type == Token::NUMBER) {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			n += as_number(prg, v[i]);
		}

		ret.type = Variable::NUMBER;
		ret.n = n;
		return ret;
	}
	else if (v[0].type == Token::STRING) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			s += as_string(prg, v[i]);
		}

		ret.type = Variable::STRING;
		ret.s = s;
		return ret;
	}
	else {
		Variable &var = get_variable(prg, v[0].i);

		if (IS_EXPRESSION(var)) {
			var = evaluate_expression(prg, var.e);
		}
		else if (IS_FISH(var)) {
			var = go_fish(prg, var.f);
		}

		if (var.type  == Variable::NUMBER) {
			double n = var.n;

			for (size_t i = 1; i < v.size(); i++) {
				n += as_number(prg, v[i]);
			}

			ret.type = Variable::NUMBER;
			ret.n = n;
			return ret;
		}
		else if (IS_STRING(var)) {
			std::string s = var.s;

			for (size_t i = 1; i < v.size(); i++) {
				s += as_string(prg, v[i]);
			}

			ret.type = Variable::STRING;
			ret.s = s;
			return ret;
		}
		else if (IS_VECTOR(var)) {
			std::vector<Variable> vec = var.v;

			for (size_t i = 1; i < v.size(); i++) {
				Variable v2 = as_variable_resolve(prg, v[i]);
				if (IS_VECTOR(v2) == false) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
				}
				vec.insert(vec.end(), v2.v.begin(), v2.v.end());
			}

			ret.type = Variable::VECTOR;
			ret.v = vec;
			return ret;
		}
		else if (IS_MAP(var)) {
			std::map<std::string, Variable> m = var.m;

			for (size_t i = 1; i < v.size(); i++) {
				Variable v2 = as_variable_resolve(prg, v[i]);
				if (IS_MAP(v2) == false) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
				}
				m.insert(v2.m.begin(), v2.m.end());
			}

			ret.type = Variable::MAP;
			ret.m = m;
			return ret;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return ret;
}

Variable exprfunc_subtract(Program *prg, const std::vector<Token> &v)
{
	double n = as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n -= as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_multiply(Program *prg, const std::vector<Token> &v)
{
	double n = as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n *= as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_divide(Program *prg, const std::vector<Token> &v)
{
	double n = as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n /= as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_modulus(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = (int)as_number(prg, v[0]) % (int)as_number(prg, v[1]);
	return var;
}

Variable exprfunc_and(Program *prg, const std::vector<Token> &v)
{
	bool b = (bool)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		b = b && (bool)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_or(Program *prg, const std::vector<Token> &v)
{
	bool b = (bool)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		b = b || (bool)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_greater(Program *prg, const std::vector<Token> &v)
{
	bool b = true;

	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_STRING(var)) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s > as_string(prg, v[i]);
		}
	}
	else {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n > as_number(prg, v[i]);
		}
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_less(Program *prg, const std::vector<Token> &v)
{
	bool b = true;
	
	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_STRING(var)) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s < as_string(prg, v[i]);
		}
	}
	else {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n < as_number(prg, v[i]);
		}
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_greaterequal(Program *prg, const std::vector<Token> &v)
{
	bool b = true;
	
	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_STRING(var)) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s >= as_string(prg, v[i]);
		}
	}
	else {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n >= as_number(prg, v[i]);
		}
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_lessequal(Program *prg, const std::vector<Token> &v)
{
	bool b = true;
	
	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_STRING(var)) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s <= as_string(prg, v[i]);
		}
	}
	else {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n <= as_number(prg, v[i]);
		}
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_equal(Program *prg, const std::vector<Token> &v)
{
	bool b = true;

	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_STRING(var)) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s == as_string(prg, v[i]);
		}
	}
	else {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n == as_number(prg, v[i]);
		}
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_notequal(Program *prg, const std::vector<Token> &v)
{
	bool b = true;

	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable var = as_variable_resolve(prg, v[0]);
		if (IS_STRING(var)) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s != as_string(prg, v[i]);
		}
	}
	else {
		double n = as_number(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n != as_number(prg, v[i]);
		}
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = b;
	return var;
}

Variable exprfunc_bitor(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n |= (int)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_xor(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n ^= (int)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_bitand(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n &= (int)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_leftshift(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n <<= (int)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

Variable exprfunc_rightshift(Program *prg, const std::vector<Token> &v)
{
	int n = (int)as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n >>= (int)as_number(prg, v[i]);
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.name = "-booboo-";
	var.n = n;
	return var;
}

static Variable matmul(Program *prg, Variable ret, Variable vec2)
{
	if (IS_NUMBER(ret) && IS_NUMBER(vec2)) {
		Variable var;
		var.type = Variable::NUMBER;
		var.n = ret.n * vec2.n;
		return var;
	}
	if (IS_NUMBER(ret)) {
		Variable tmp = ret;
		ret = vec2;
		vec2 = tmp;
	}
	if (IS_NUMBER(vec2)) {
		// it's a vector
		if (IS_NUMBER(ret.v[0])) {
			for (size_t i = 0; i < ret.v.size(); i++) {
				ret.v[i].n *= vec2.n;
			}
		}
		// it's a matrix
		else {
			for (size_t i = 0; i < ret.v.size(); i++) {
				for (size_t j = 0; j < ret.v[0].v.size(); j++) {
					ret.v[i].v[j].n *= vec2.n;
				}
			}
		}
	}
	else {
		CHECK_VECTOR(vec2);

		bool is_mat1, is_mat2;

		Variable tmp;

		if (IS_VECTOR(ret.v[0])) {
			is_mat1 = true;
		}
		else {
			is_mat1 = false;
			tmp = ret;
			ret.v.clear();
			for (size_t i = 0; i < tmp.v.size(); i++) {
				Variable var;
				var.type = Variable::VECTOR;
				var.v.push_back(tmp.v[i]);
				ret.v.push_back(var);
			}
		}

		if (IS_VECTOR(vec2.v[0])) {
			is_mat2 = true;
		}
		else {
			is_mat2 = false;
			tmp = vec2;
			vec2.v.clear();
			vec2.v.push_back(tmp);
		}

		if (is_mat1 == false && is_mat2 == false) {
			throw Error(std::string(__FUNCTION__) + ": " + "Vector-vector multiplication is undefined at " + get_error_info(prg));
		}

		if (ret.v.size() != vec2.v[0].v.size()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Matrices cannot be multiplied at " + get_error_info(prg));
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
				var.type = Variable::VECTOR;
				for (unsigned int r = 0; r < h; r++) {
					Variable var2;
					var2.type = Variable::NUMBER;
					var2.n = 0;
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
					sum += ret.v[r2].v[r].n * vec2.v[c].v[r2].n;
				}
				tmp.v[cc].v[rr].n = sum;
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

Variable exprfunc_mul(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable var = as_variable_resolve(prg, v[0]);
	
	Variable ret;
	ret = var;

	for (size_t n = 1; n < v.size(); n++) {	
		Variable var2 = as_variable_resolve(prg, v[n]);

		ret = matmul(prg, ret, var2);
	}

	return ret;
}

static glm::mat4 to_glm_mat4(Variable &v)
{
	glm::mat4 m;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m[i][j] = v.v[i].v[j].n;
		}
	}
	return m;
}

static Variable from_glm_mat4(glm::mat4 m)
{
	Variable var;
	var.type = Variable::VECTOR;
	for (int i = 0; i < 4; i++) {
		glm::vec4 v = m[i];
		Variable v2;
		v2.type = Variable::VECTOR;
		for (int j = 0; j < 4; j++) {
			Variable v3;
			v3.type = Variable::NUMBER;
			v3.n = v[j];
			v2.v.push_back(v3);
		}
		var.v.push_back(v2);
	}
	return var;
}

static Variable identity(int sz)
{
	Variable var;
	var.type = Variable::VECTOR;

	for (int c = 0; c < sz; c++) {
		Variable col;
		col.type = Variable::VECTOR;
		for (int r = 0; r < sz; r++) {
			Variable num;
			num.type = Variable::NUMBER;
			if (c == r) {
				num.n = 1;
			}
			else {
				num.n = 0;
			}
			col.v.push_back(num);
		}
		var.v.push_back(col);
	}

	return var;
}

Variable exprfunc_identity(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)
	
	int sz = as_number(prg, v[0]);

	return identity(sz);
}

Variable exprfunc_scale(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	double sx = as_number(prg, v[0]);
	double sy = as_number(prg, v[1]);
	double sz = as_number(prg, v[2]);

	Variable mat = identity(4);

	mat.v[0].v[0].n = sx;
	mat.v[1].v[1].n = sy;
	mat.v[2].v[2].n = sz;

	return mat;
}

Variable exprfunc_rotate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)
	
	double angle = as_number(prg, v[0]);
	double x = as_number(prg, v[1]);
	double y = as_number(prg, v[2]);
	double z = as_number(prg, v[3]);

	double s = sin(angle);
	double c = cos(angle);
	double invc = 1 - c;

	Variable mat = identity(4);

	mat.v[0].v[0].n = (invc * x * x) + c;
	mat.v[0].v[1].n = (invc * x * y) + (z * s);
	mat.v[0].v[2].n = (invc * x * z) - (y * s);
	mat.v[1].v[0].n = (invc * x * y) - (z * s);
	mat.v[1].v[1].n = (invc * y * y) + c;
	mat.v[1].v[2].n = (invc * z * y) + (x * s);
	mat.v[2].v[0].n = (invc * x * z) + (y * s);
	mat.v[2].v[1].n = (invc * y * z) - (x * s);
	mat.v[2].v[2].n = (invc * z * z) + c;

	return mat;
}

Variable exprfunc_translate(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable mat = identity(4);

	double tx = as_number(prg, v[0]);
	double ty = as_number(prg, v[1]);
	double tz = as_number(prg, v[2]);

	mat.v[3].v[0].n = tx;
	mat.v[3].v[1].n = ty;
	mat.v[3].v[2].n = tz;

	return mat;
}

static double veclen(const Variable &vec)
{
	if (vec.v.size() == 2) {
		return sqrt(pow(vec.v[0].n, 2) + pow(vec.v[1].n, 2));
	}
	return sqrt(pow(vec.v[0].n, 2) + pow(vec.v[1].n, 2) + pow(vec.v[2].n, 2));
}

Variable exprfunc_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable vec = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(vec)
	
	if (vec.v.size() != 2 && vec.v.size() != 3) {
		throw Error(std::string(__FUNCTION__) + ": " + "Vector size not supported in length at " + get_error_info(prg));
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.n = veclen(vec);

	return var;
}

static double vecdot(const Variable &vec1, const Variable &vec2)
{
	return vec1.v[0].n * vec2.v[0].n + vec1.v[1].n * vec2.v[1].n + vec1.v[2].n * vec2.v[2].n;
}

Variable exprfunc_dot(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	const Variable vec1 = as_variable_resolve(prg, v[0]);
	const Variable vec2 = as_variable_resolve(prg, v[1]);

	CHECK_VECTOR(vec1)
	CHECK_VECTOR(vec2)

	if (vec1.v.size() < 3 || vec2.v.size() < 3) {
		throw Error(std::string(__FUNCTION__) + ": " + "Vector with < 3 components not supported at " + get_error_info(prg));
	}

	Variable var;
	var.type = Variable::NUMBER;
	var.n = vecdot(vec1, vec2);

	return var;
}

static Variable veccross(Variable vec, Variable vec2)
{
	Variable tmp = vec;
	vec.v[0].n = tmp.v[0].n * vec2.v[2].n - tmp.v[2].n * vec2.v[1].n;
	vec.v[1].n = tmp.v[2].n * vec2.v[0].n - tmp.v[0].n * vec2.v[2].n;
	vec.v[2].n = tmp.v[0].n * vec2.v[1].n - tmp.v[1].n * vec2.v[0].n;
	return vec;
}

Variable exprfunc_angle(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable vec1 = as_variable_resolve(prg, v[0]);
	Variable vec2 = as_variable_resolve(prg, v[1]);

	CHECK_VECTOR(vec1)
	CHECK_VECTOR(vec2)

	if (vec1.v.size() == 2) {
		double a1 = atan2(vec1.v[1].n, vec1.v[0].n);
		double a2 = atan2(vec2.v[1].n, vec2.v[0].n);
		Variable var;
		var.type = Variable::NUMBER;
		var.n = a2 - a1;
		if (var.n < 0) {
			var.n += M_PI * 2;
		}
		return var;
	}
	
	if (vec1.v.size() < 3 || vec2.v.size() < 3) {
		throw Error(std::string(__FUNCTION__) + ": " + "Vector with < 3 components not supported at " + get_error_info(prg));
	}

	Variable len;
	len.type = Variable::NUMBER;
	len.n = veclen(vec2);
	
	Variable var;
	var.type = Variable::NUMBER;
	var.n = acosf(vecdot(vec1, vec2) / veclen(matmul(prg, vec1, len)));

	return var;
}

Variable exprfunc_cross(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable vec = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(vec)

	for (size_t i = 1; i < v.size(); i++) {
		Variable vec2 = as_variable_resolve(prg, v[i]);
		CHECK_VECTOR(vec2)
		if (vec.v.size() < 3 || vec2.v.size() < 3) {
			throw Error(std::string(__FUNCTION__) + ": " + "Vector with < 3 components not supported at " + get_error_info(prg));
		}
		vec = veccross(vec, vec2);
	}

	return vec;
}

Variable exprfunc_normalize(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable vec = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(vec)

	Variable len;
	len.type = Variable::NUMBER;
	len.n = 1.0 / veclen(vec);

	return matmul(prg, vec, len);
}

Variable exprfunc_vadd(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable vec = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(vec)

	for (size_t i = 1; i < v.size(); i++) {
		Variable vec2 = as_variable_resolve(prg, v[i]);
		CHECK_VECTOR(vec2)
		if (vec.v.size() != vec2.v.size()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Can't add different sized vectors at " + get_error_info(prg));
		}
		if (IS_NUMBER(vec.v[0]) && IS_NUMBER(vec2.v[0])) {
			for (size_t j = 0; j < vec.v.size(); j++) {
				vec.v[j].n += vec2.v[j].n;
			}
		}
		else if (IS_VECTOR(vec.v[0]) && IS_VECTOR(vec2.v[0])) {
			if (vec.v[0].v.size() != vec2.v[0].v.size()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Can't add different sized matrices at " + get_error_info(prg));
			}
			for (size_t j = 0; j < vec.v.size(); j++) {
				for (size_t i = 0; i < vec.v[j].v.size(); i++) {
					vec.v[j].v[i].n += vec2.v[j].v[i].n;
				}
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Add not supported on types at " + get_error_info(prg));
		}
	}

	return vec;
}

Variable exprfunc_vsub(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable vec = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(vec)

	for (size_t i = 1; i < v.size(); i++) {
		Variable vec2 = as_variable_resolve(prg, v[i]);
		CHECK_VECTOR(vec2)
		if (vec.v.size() != vec2.v.size()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Can't subtract different sized vectors at " + get_error_info(prg));
		}
		if (IS_NUMBER(vec.v[0]) && IS_NUMBER(vec2.v[0])) {
			for (size_t j = 0; j < vec.v.size(); j++) {
				vec.v[j].n -= vec2.v[j].n;
			}
		}
		else if (IS_VECTOR(vec.v[0]) && IS_VECTOR(vec2.v[0])) {
			if (vec.v[0].v.size() != vec2.v[0].v.size()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Can't subtract different sized matrices at " + get_error_info(prg));
			}
			for (size_t j = 0; j < vec.v.size(); j++) {
				for (size_t i = 0; i < vec.v[j].v.size(); i++) {
					vec.v[j].v[i].n -= vec2.v[j].v[i].n;
				}
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Subtract not supported on types at " + get_error_info(prg));
		}
	}

	return vec;
}

Variable exprfunc_inverse(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable mat = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(mat)

	glm::mat4 m = to_glm_mat4(mat);
	m = glm::inverse(m);
	return from_glm_mat4(m);
}

Variable exprfunc_transpose(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable mat = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(mat)

	glm::mat4 m = to_glm_mat4(mat);
	m = glm::transpose(m);
	return from_glm_mat4(m);
}

static void init_token_map()
{
	add_token_handler(':', tokenfunc_label);
	add_token_handler('"', tokenfunc_string);
	add_token_handler('{', tokenfunc_openbrace);
	add_token_handler('}', tokenfunc_closebrace);
	add_token_handler(';', tokenfunc_comment);
	add_token_handler('+', tokenfunc_add);
	add_token_handler('-', tokenfunc_subtract);
	add_token_handler('=', tokenfunc_equals);
	add_token_handler('?', tokenfunc_compare);
	add_token_handler('*', tokenfunc_multiply);
	add_token_handler('/', tokenfunc_divide);
	add_token_handler('%', tokenfunc_modulus);
	add_token_handler('(', tokenfunc_expression);
	add_token_handler('[', tokenfunc_fish);
	add_token_handler('|', tokenfunc_or);
	add_token_handler('^', tokenfunc_xor);
	add_token_handler('&', tokenfunc_and);
	add_token_handler('<', tokenfunc_leftshift);
	add_token_handler('>', tokenfunc_rightshift);
	add_token_handler('#', tokenfunc_hex);
	add_token_handler('~', tokenfunc_ref);
	add_token_handler('`', tokenfunc_deref);
}

void start()
{
	twinkle::start();

	init_token_map();
	
	add_expression_handler("+", exprfunc_add);
	add_expression_handler("-", exprfunc_subtract);
	add_expression_handler("*", exprfunc_multiply);
	add_expression_handler("/", exprfunc_divide);
	add_expression_handler("%", exprfunc_modulus);
	add_expression_handler("&&", exprfunc_and);
	add_expression_handler("||", exprfunc_or);
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

	add_instruction("reset", breaker_reset);
	add_instruction("exit", breaker_exit);
	add_instruction("return", breaker_return);

	add_instruction("number", corefunc_number);
	add_instruction("string", corefunc_string);
	add_instruction("vector", corefunc_vector);
	add_instruction("map", corefunc_map);
	add_instruction("pointer", corefunc_pointer);
	
	add_instruction("=", corefunc_set);
	add_instruction("+", corefunc_add);
	add_instruction("-", corefunc_subtract);
	add_instruction("*", corefunc_multiply);
	add_instruction("/", corefunc_divide);
	
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
	
	add_instruction("typeof", corefunc_typeof);
	
	add_instruction("for", corefunc_for);
	add_instruction("if", corefunc_if);
	
	add_instruction("address", corefunc_address);
	
	return_code = 0;
}

void end()
{
	library_map.clear();
}

Program *create_program(std::string code)
{
	Program *prg = new Program;

	prg->tmps = 0;

	prg->s = new Function_Swap;

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
	prg->s->name = "main";
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
	prg->functions.clear();
	prg->s->line_numbers.clear();

	compile(prg, PASS2);

	prg->s->p = 0;
	prg->s->line = 0;
	prg->s->start_line = 0;
	prg->s->pc = 0;

	prg->s->result.name = "result";

	return prg;
}

// These help when developing BooBoo apps in C++ or adding API

Token token_number(std::string token, double n)
{
	Token t;
	t.type = Token::NUMBER;
	t.token = token;
	t.n = n;
	t.dereference = false;
	return t;
}

Token token_string(std::string token, std::string s)
{
	Token t;
	t.type = Token::STRING;
	t.token = token;
	t.s = s;
	t.dereference = false;
	return t;
}

double get_number(Variable &v)
{
	return v.n;
}

std::string get_string(Variable &v)
{
	return v.s;
}

std::vector<Variable> get_vector(Variable &v)
{
	return v.v;
}

void *get_black_box(Program *prg, std::string id)
{
	if (prg->black_box.find(id) == prg->black_box.end()) {
		return nullptr;
	}
	return prg->black_box[id];
}

void set_black_box(Program *prg, std::string id, void *data)
{
	prg->black_box[id] = data;
}

Variable &get_variable(Program *prg, int index)
{
	return prg->variables[index];
}

Variable &as_variable(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (prg->variables[t.i].type == Variable::FISH) {
		return go_fish(prg, prg->variables[t.i].f);
	}
	return prg->variables[t.i];
}

Variable as_variable_resolve(Program *prg, const Token &t)
{
	if (t.type == Token::NUMBER) {
		Variable var;
		var.type = Variable::NUMBER;
		var.n = t.n;
		return var;
	}
	if (t.type == Token::STRING) {
		Variable var;
		var.type = Variable::STRING;
		var.s = t.s;
		return var;
	}
	if (prg->variables[t.i].type == Variable::FISH) {
		return go_fish(prg, prg->variables[t.i].f);
	}
	if (prg->variables[t.i].type == Variable::EXPRESSION) {
		return evaluate_expression(prg, prg->variables[t.i].e);
	}
	return prg->variables[t.i];
}

double as_number(Program *prg, const Token &t)
{
	if (t.type == Token::NUMBER) {
		return t.n;
	}
	else if (t.type == Token::SYMBOL) {
		Variable *v;
		if (t.dereference) {
			if (prg->variables[t.i].type == Variable::EXPRESSION) {
				Variable var = evaluate_expression(prg, prg->variables[t.i].e);
				v = var.p;
			}
			else if (prg->variables[t.i].type == Variable::FISH) {
				Variable &var = go_fish(prg, prg->variables[t.i].f);
				v = var.p;
			}
			else {
				v = prg->variables[t.i].p;
			}
		}
		else {
			v = &prg->variables[t.i];
		}
		if (v->type == Variable::NUMBER) {
			return v->n;
		}
		else if (v->type == Variable::EXPRESSION) {
			Variable var = evaluate_expression(prg, v->e);
			if (var.type == Variable::NUMBER) {
				return var.n;
			}
			else if (var.type == Variable::STRING) {
				return atof(var.s.c_str());
			}
			else {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
			}
		}
		else if (v->type == Variable::STRING) {
			return atof(v->s.c_str());
		}
		else if (v->type == Variable::FISH) {
			Variable &var = go_fish(prg, v->f);
			if (var.type == Variable::NUMBER) {
				return var.n;
			}
			else if (var.type == Variable::STRING) {
				return atof(var.s.c_str());
			}
			else {
				throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}
	else if (t.type == Token::STRING) {
		return atof(t.s.c_str());
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
}

std::string as_string(Program *prg, const Token &t)
{
	if (t.type == Token::NUMBER) {
		char buf[1000];
		snprintf(buf, 1000, "%g", t.n);
		return buf;
	}
	else if (t.type == Token::SYMBOL) {
		Variable *v;
		if (t.dereference) {
			if (prg->variables[t.i].type == Variable::EXPRESSION) {
				Variable var = evaluate_expression(prg, prg->variables[t.i].e);
				v = var.p;
			}
			else if (prg->variables[t.i].type == Variable::FISH) {
				Variable &var = go_fish(prg, prg->variables[t.i].f);
				v = var.p;
			}
			else {
				v = prg->variables[t.i].p;
			}
		}
		else {
			v = &prg->variables[t.i];
		}
		if (v->type == Variable::STRING) {
			return v->s;
		}
		else if (v->type == Variable::NUMBER) {
			char buf[1000];
			snprintf(buf, 1000, "%g", v->n);
			return buf;
		}
		else if (v->type == Variable::EXPRESSION) {
			Variable var = evaluate_expression(prg, v->e);
			if (var.type != Variable::STRING) {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
			}
			return var.s;
		}
		else if (v->type == Variable::FISH) {
			Variable &var = go_fish(prg, v->f);
			if (var.type != Variable::STRING) {
				throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
			}
			return var.s;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}
	else if (t.type == Token::STRING) {
		return t.s;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
}

int as_label(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	Variable *v;
	if (t.dereference) {
		if (prg->variables[t.i].type == Variable::EXPRESSION) {
			Variable var = evaluate_expression(prg, prg->variables[t.i].e);
			v = var.p;
		}
		else if (prg->variables[t.i].type == Variable::FISH) {
			Variable &var = go_fish(prg, prg->variables[t.i].f);
			v = var.p;
		}
		else {
			v = prg->variables[t.i].p;
		}
	}
	else {
		v = &prg->variables[t.i];
	}
	if (v->type == Variable::FISH) {
		Variable &var = go_fish(prg, v->f);
		if (var.type != Variable::LABEL) {
			throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
		}
		return var.n;
	}
	else if (v->type != Variable::LABEL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	return v->n;
}

int as_function(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	Variable *v;
	if (t.dereference) {
		if (prg->variables[t.i].type == Variable::EXPRESSION) {
			Variable var = evaluate_expression(prg, prg->variables[t.i].e);
			v = var.p;
		}
		else if (prg->variables[t.i].type == Variable::FISH) {
			Variable &var = go_fish(prg, prg->variables[t.i].f);
			v = var.p;
		}
		else {
			v = prg->variables[t.i].p;
		}
	}
	else {
		v = &prg->variables[t.i];
	}
	if (v->type == Variable::FISH) {
		Variable &var = go_fish(prg, v->f);
		if (var.type != Variable::FUNCTION) {
			throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
		}
		return var.n;
	}
	else if (v->type != Variable::FUNCTION) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	return v->n;
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

std::string itos(int i)
{
	char buf[20];
	snprintf(buf, 20, "%d", i);
	return std::string(buf);
}

Variable evaluate_expression(Program *prg, const Variable::Expression &e)
{
	if (e.i == -1) {
		std::map<std::string, int>::iterator it = prg->variables_map.find(e.name);
		if (it == prg->variables_map.end()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Unknown expression function '" + e.name + "' at " + get_error_info(prg));

		}

		Variable &func = prg->variables[it->second];

		Variable result;

		call_function(prg, func.n, e.v, result);

		return result;
	}

	return expression_handlers[e.i](prg, e.v);
}

Variable &go_fish(Program *prg, const Variable::Fish &f)
{
	Variable *v = &prg->variables[f.c_i];
	int type = v->type;

	while (type == Variable::FISH) {
		v = &go_fish(prg, v->f);
		type = v->type;
	}

	int index = 0;
	std::string key;

	for (size_t i = 0; i < f.v.size(); i++) {
		if (type == Variable::VECTOR) {
			index = as_number(prg, f.v[i]);
		}
		else {
			key = as_string(prg, f.v[i]);
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

	if (type == Variable::VECTOR) {
		return v->v[index];
	}
	else {
		return v->m[key];
	}
}

} // end namespace booboo

