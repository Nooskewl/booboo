#include <sys/stat.h>

#include <climits>
#include <fstream>

#include "booboo/booboo.h"
#include "booboo/internal.h"

static std::map<std::string, int> library_map;
static std::vector<booboo::library_func> library;
static std::map<char, booboo::token_func> token_map;
static std::map<std::string, int> expression_map;
static std::vector<booboo::expression_func> expression_handlers;

// First off maybe 10 utility functions taken from Nooskewl Shim

// FIXME: make a utility file because this is shared with core_launcher
static std::string load_text_from_filesystem(std::string filename)
{
	int _sz;
	struct stat st;
	int r = stat(filename.c_str(), &st);
	if (r == 0) {
		_sz = st.st_size;
	}
	else {
		throw booboo::Error("booboo::Error getting file size: " + filename);
	}

	FILE *file = fopen(filename.c_str(), "rb");

	if (file == nullptr) {
		throw booboo::Error("File not found: " + filename);
	}

	char *buf = new char[_sz+1];

	if (fread(buf, _sz, 1, file) != 1) {
		throw booboo::Error("File load error: " + filename);
	}

	fclose(file);

	buf[_sz] = 0;

	std::string text = buf;

	delete[] buf;

	return text;
}

static std::string &ltrim(std::string &s)
{
	int i = 0;
	while (i < (int)s.length() && isspace(s[i])) {
		i++;
	}
	if (i >= (int)s.length()) {
		s = "";
	}
	else {
		s = s.substr(i);
	}
	return s;
}

static std::string &rtrim(std::string &s)
{
	int i = (int)s.length() - 1;
	while (i >= 0 && isspace(s[i])) {
		i--;
	}
	if (i < 0) {
		s = "";
	}
	else {
		s = s.substr(0, i+1);
	}
	return s;
}

static std::string &trim(std::string &s)
{
	return ltrim(rtrim(s));
}

static std::string unescape_string(std::string s)
{
	std::string ret;
	int p = 0;
	char buf[2];
	buf[1] = 0;

	if (s.length() == 0) {
		return "";
	}

	while (p < (int)s.length()) {
		if (s[p] == '\\') {
			if (p+1 < (int)s.length()) {
				if (s[p+1] == '\\' || s[p+1] == '"') {
					p++;
					buf[0] = s[p];
					ret += buf;
					p++;
				}
				else if (s[p+1] == 'n') {
					p++;
					buf[0] = '\n';
					ret += buf;
					p++;
				}
				else if (s[p+1] == 't') {
					p++;
					buf[0] = '\t';
					ret += buf;
					p++;
				}
				else {
					buf[0] = '\\';
					ret += buf;
					p++;
				}
			}
			else {
				buf[0] = '\\';
				ret += buf;
				p++;
			}
		}
		else {
			buf[0] = s[p];
			ret += buf;
			p++;
		}
	}

	return ret;
}

static void skip_whitespace(booboo::Program *prg)
{
	while (prg->s->p < prg->s->code.length() && isspace(prg->s->code[prg->s->p])) {
		if (prg->s->code[prg->s->p] == '\n') {
			prg->s->line++;
		}
		prg->s->p++;
	}
}

static std::string remove_quotes(std::string s)
{
	int start = 0;
	int count = s.length();

	if (s[0] == '"') {
		start++;
		count--;
	}

	if (s[s.length()-1] == '"') {
		count--;
	}

	return s.substr(start, count);
}

namespace booboo {

std::string reset_game_name;
std::string main_program_name;
int return_code;
bool quit;

// And this all makes BooBoo work

File_Info *file_info(Program *prg)
{
	File_Info *info = (File_Info *)get_black_box(prg, "com.b1stable.booboo.files");
	if (info == nullptr) {
		info = new File_Info;
		info->file_id = 0;
		set_black_box(prg, "com.b1stable.booboo.files", info);
	}
	return info;
}

std::string get_file_name(Program *prg)
{
	if (prg->line_numbers.size() <= prg->s->pc) {
		return "UNKNOWN";
	}

	int l = prg->line_numbers[prg->s->pc];

	if (prg->real_file_names.size() <= (unsigned int)l) {
		return "UNKNOWN";
	}

	return prg->real_file_names[prg->line_numbers[prg->s->pc]];
}

int get_line_num(Program *prg)
{
	int l;

	l = prg->s->pc;

	if (prg->line_numbers.size() <= (unsigned int)l) {
		if (prg->line_numbers.size() > 0) {
			return prg->line_numbers[prg->line_numbers.size()-1]+(prg->complete_pass == PASS2 ? prg->s->start_line : 0);
		}
		else {
			return 1;
		}
	}

	l = prg->line_numbers[l];

	if (prg->real_line_numbers.size() <= (unsigned int)l) {
		if (prg->real_line_numbers.size() > 0) {
			return prg->real_line_numbers[prg->real_line_numbers.size()-1]+(prg->complete_pass == PASS2 ? prg->s->start_line : 0);
		}
		else {
			return 1;
		}
	}

	return prg->real_line_numbers[prg->line_numbers[prg->s->pc]]+(prg->complete_pass == PASS2 ? prg->s->start_line : 0);
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

	prg->line_numbers.push_back(prg->s->line); // can help give better line number
	throw Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(prg) + " (pc=" + itos(prg->s->p) + ", tok=\"" + tok + "\")");

	return "";
}

bool process_includes(Program *prg)
{
	bool ret = false;

	int total_added = 0;

	std::string code;

	std::string tok;

	prg->s->p = 0;
	prg->s->line = 1;
	prg->line_numbers.clear();

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
			std::string name = token(prg, tt);

			int start_line = prg->s->line;

			if (name == "") {
				throw Error(std::string(__FUNCTION__) + ": " + "Expected include parameters at " + get_error_info(prg));
			}

			if (name[0] != '"') {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid include name at " + get_error_info(prg));
			}

			name = remove_quotes(unescape_string(name));

			code += prg->s->code.substr(start, prev-start);

			std::string new_code;
			std::string fn;
			fn = name;
			new_code = load_text_from_filesystem(name);

			new_code = trim(new_code);

			int nlines = 1;
			int i = 0;
			while (new_code[i] != 0) {
				if (new_code[i] == '\n') {
					nlines++;
				}
				i++;
			}

			code += "\n" + new_code;
			
			prg->real_line_numbers[start_line-1+total_added] = 1;
			prg->real_file_names[start_line-1+total_added] = fn;
			for (int i = 1; i < nlines; i++) {
				prg->real_line_numbers.insert(prg->real_line_numbers.begin()+start_line+(i-1+total_added), i+1);
				prg->real_file_names.insert(prg->real_file_names.begin()+start_line+(i-1+total_added), fn);
			}

			start = prg->s->p;

			total_added += nlines;

			ret = true;
		}

		prev = prg->s->p;
	}

	code += prg->s->code.substr(start, prg->s->code.length()-start);

	prg->s->code = code;
	prg->s->p = 0;
	prg->s->line = 1;

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
		if (restore_locals || prg->variables[pair.second].type == Variable::LABEL) {
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
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid expression at " + get_error_info(func));
	}
	p++; // skip (
	std::string name;
	while (!isspace(expr[p]) && p < (int)expr.length()) {
		char buf[2];
		buf[0] = expr[p];
		buf[1] = 0;
		name += buf;
		p++;
	}
	if (expression_map.find(name) == expression_map.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Unknown expression function at " + get_error_info(func));
	}

	Variable::Expression e;
	e.i = expression_map[name];

	bool done = false;

	while (!done) {
		while (isspace(expr[p]) && p < (int)expr.length()) {
			p++;
		}
		if (p >= (int)expr.length()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid expression at " + get_error_info(func));
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

			//func->s->program[func->s->program.size()-1].data.push_back(tok);
		}
		else if (c == ')') {
			done = true;
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

			//func->s->program[func->s->program.size()-1].data.push_back(tok);
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
		}
		else if (c == '"') {
			tok.type = Token::STRING;
			std::string str = "\"";
			p++;
			int prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (buf[0] == '"' && prev != '\\') {
					p++;
					break;
				}
				prev = buf[0];
				p++;
			}
			str = remove_quotes(unescape_string(str));
			tok.token = str;
			tok.s = str;
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

			if (pass == PASS2) {
				if (prg->variables_map.find(sym) == prg->variables_map.end()) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(func));
				}
				tok.i = prg->variables_map[sym];
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(func));
		}

		e.v.push_back(tok);
	}

	return e;
}

static Variable::Fish parse_fish(Program *prg, Program *func, std::string expr, int &var_i, int &expression_i, int &fish_i, Pass pass)
{
	int p = 0;

	while (isspace(expr[p]) && p < (int)expr.length()) {
		p++;
	}
	if (p >= (int)expr.length()-1) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid fish at " + get_error_info(func));
	}
	p++; // skip [
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
			throw Error(std::string(__FUNCTION__) + ": " + "Unknown variable at " + get_error_info(func));
		}
	}

	Variable::Fish e;
	e.c_i = prg->variables_map[name];

	bool done = false;

	while (!done) {
		while (isspace(expr[p]) && p < (int)expr.length()) {
			p++;
		}
		if (p >= (int)expr.length()) {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid fish at " + get_error_info(func));
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

			//func->s->program[func->s->program.size()-1].data.push_back(tok);
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

			//func->s->program[func->s->program.size()-1].data.push_back(tok);
		}
		else if (c == ']') {
			done = true;
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
		}
		else if (c == '"') {
			tok.type = Token::STRING;
			std::string str = "\"";
			p++;
			int prev = -1;
			while (p < (int)expr.length()) {
				char buf[2];
				buf[0] = expr[p];
				buf[1] = 0;
				str += buf;
				if (buf[0] == '"' && prev != '\\') {
					p++;
					break;
				}
				prev = buf[0];
				p++;
			}
			str = remove_quotes(unescape_string(str));
			tok.token = str;
			tok.s = str;
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

			if (pass == PASS2) {
				if (prg->variables_map.find(sym) == prg->variables_map.end()) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + sym + " at " + get_error_info(func));
				}
				tok.i = prg->variables_map[sym];
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Parse error at " + get_error_info(func));
		}

		e.v.push_back(tok);
	}

	return e;
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
			func.real_line_numbers = prg->real_line_numbers;
			func.real_file_names = prg->real_file_names;
			bool is_param = true;
			bool finished = false;
			while ((tok = token(prg, tt)) != "") {
func_top:
				if (tok == ";") {
					while (prg->s->p < prg->s->code.length() && prg->s->code[prg->s->p] != '\n') {
						prg->s->p++;
					}
					prg->s->line++;
					if (prg->s->p < prg->s->code.length()) {
						prg->s->p++;
					}
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
					if (pass == PASS1) {
						prg->locals[func_index][tok2] = var_i++;
						prg->variables.push_back(v);
					}
					else {
						var_i++;
					}
				}
				else if (tok == "number" || tok == "string" || tok == "vector" || tok == "map" || tok == "pointer") {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					if (pass == PASS2) {
						//if (func.line_numbers.size() != 0) {
							func.s->pc++;
						//}
						func.line_numbers.push_back(prg->s->line);
					}
					int count = 0;
					while (true) {
						std::string tok2 = token(prg, tt);
						if (tok2 == "" || (count != 0 && (tok2 == ";" || tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
							tok = tok2;
							goto func_top;
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
						else {
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

					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
				else if (library_map.find(tok) != library_map.end()) {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					if (pass == PASS2) {
						//if (func.line_numbers.size() != 0) {
							func.s->pc++;
						//}
						func.line_numbers.push_back(prg->s->line);
					}
				}
				else if (is_param) {
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
				}
				else {
					if (func.s->program.size() == 0) {
						func.line_numbers.push_back(prg->s->line); // can help give better line number
						throw Error("Expected keyword at " + get_error_info(&func));
					}
					Token t;
					t.token = tok;
					t.type = tt;
					switch (tt) {
						case Token::STRING:
							t.s = remove_quotes(unescape_string(tok));
							break;
						case Token::SYMBOL:
							t.s = remove_quotes(unescape_string(tok));
							if (pass == PASS2 && prg->variables_map.find(t.s) == prg->variables_map.end()) {
								func.line_numbers.push_back(prg->s->line); // can help give better line number
								throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok + " at " + get_error_info(&func));
							}
							if (pass == PASS2) {
								t.i = prg->variables_map[t.s];
							}
							break;
						case Token::NUMBER:
							t.n = atof(tok.c_str());
							break;
					}
					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
			}

			if (is_param == true) {
				func.line_numbers.push_back(prg->s->line); // can help give better line number
				throw Error(std::string(__FUNCTION__) + ": " + "Missing { at " + get_error_info(&func));
			}

			if (finished == false) {
				func.line_numbers.push_back(prg->s->line); // can help give better line number
				throw Error(std::string(__FUNCTION__) + ": " + "Missing } at " + get_error_info(&func));
			}

			prg->function_name_map[func.s->name] = prg->functions.size();
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
			if (prg->variables_map.find(tok2) != prg->variables_map.end()) {
				if (pass == PASS1) {
					printf("duplicate label %s\n", tok2.c_str());
				}
			}

			prg->variables_map[tok2] = var_i++;
			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
		}
		else if (tok == "number" || tok == "string" || tok == "vector" || tok == "map" || tok == "pointer") {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			if (pass == PASS2) {
				//if (prg->line_numbers.size() != 0) {
					prg->s->pc++;
				//}
				prg->line_numbers.push_back(prg->s->line);
			}
			int count = 0;
			while (true) {
				std::string tok2 = token(prg, tt);
				if (tok2 == "" || (count != 0 && (tok2 == ";" || tok2 == "{" || tok2 == "}" || tok2 == ":" || tok2 == "function" || tok2[0] == '(' || tok2[0] == '[' || library_map.find(tok2) != library_map.end()))) {
					tok = tok2;
					goto top;
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
				else {
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

			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
		else if (library_map.find(tok) != library_map.end()) {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			if (pass == PASS2) {
				//if (prg->line_numbers.size() != 0) {
					prg->s->pc++;
				//}
				prg->line_numbers.push_back(prg->s->line);
			}
		}
		else if (prg->s->program.size() == 0) {
			prg->line_numbers.push_back(prg->s->line); // can help give better line number
			throw Error("Expected keyword at " + get_error_info(prg));
		}
		else {
			Token t;
			t.token = tok;
			t.type = tt;
			switch (tt) {
				case Token::STRING:
					t.s = remove_quotes(unescape_string(tok));
					break;
				case Token::SYMBOL:
					t.s = remove_quotes(unescape_string(tok));
					if (pass == PASS2 && prg->variables_map.find(t.s) == prg->variables_map.end()) {
						throw Error(std::string(__FUNCTION__) + ": " + "Invalid variable name " + tok + " at " + get_error_info(prg));
					}
					if (pass == PASS2) {
						t.i = prg->variables_map[t.s];
					}
					break;
				case Token::NUMBER:
					t.n = atof(tok.c_str());
					break;
			}
			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
	}

	prg->s->p = p_bak;
	prg->s->line = line_bak;

	prg->complete_pass = pass;
}

void call_function(Program *prg, int function, std::vector<Token> &params, Variable &result, int ignore_params)
{
	Program &func = prg->functions[function];

	for (size_t j = 0; j < func.params.size(); j++) {
		Token &param = params[j+ignore_params];
		
		Variable &var = prg->variables[func.params[j]];

		if (param.type == Token::NUMBER) {
			var.type = Variable::NUMBER;
			var.n = param.n;
			var.name = param.token;
		}
		else if (param.type == Token::STRING) {
			var.type = Variable::STRING;
			var.s = param.s;
			var.name = param.token;
		}
		else {
			var = prg->variables[param.i];
		}
	}

	Function_Swap *bak2 = prg->s;
	prg->s = func.s;
	//prg->s->p = 0;
	//prg->s->line = 1;
	prg->s->pc = 0;

	while (interpret(prg, INT_MAX)) {
	}

	std::string bak = result.name;
	result = prg->s->result;
	result.name = bak;

	prg->s = bak2;
}

void call_function(Program *prg, std::string function_name, std::vector<Token> &params, Variable &result, int ignore_params)
{
	std::map<std::string, int>::iterator it = prg->function_name_map.find(function_name);
	if (it == prg->function_name_map.end()) {
		return;
	}
	call_function(prg, (*it).second, params, result, ignore_params);
}

void call_void_function(Program *prg, int function, std::vector<Token> &params, int ignore_params)
{
	static Variable tmp;
	call_function(prg, function, params, tmp, ignore_params);
}

void call_void_function(Program *prg, std::string function_name, std::vector<Token> &params, int ignore_params)
{
	static Variable tmp;
	call_function(prg, function_name, params, tmp, ignore_params);
}

bool interpret(Program *prg, int instructions)
{
	for (int i = 0; i < instructions; i++) {
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

		if (ret == false) {
			return false;
		}
	}

	return true;
}

void destroy_program(Program *prg)
{
	prg->variables.clear();
	prg->functions.clear();

	File_Info *file_i = file_info(prg);
	for (size_t i = 0; i < file_i->files.size(); i++) {
		file_i->files[i]->close();
		delete file_i->files[i];
	}
	delete file_i;

	set_black_box(prg, "com.b1stable.booboo.files", nullptr);
	for (size_t i = 0; i < prg->functions.size(); i++) {
		delete prg->functions[i].s;
	}
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
}

void start()
{
	init_token_map();
	
	start_lib_core();
	
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
		}
		prg->real_file_names.push_back(main_program_name);
		i++;
	}

	prg->s->code = code;
	prg->s->name = "main";
	prg->s->line = 1;
	prg->line_numbers.clear();
	prg->s->start_line = 0;
	prg->s->p = 0;
	prg->s->pc = 0;
	prg->complete_pass = PASS0;
	
	while(process_includes(prg));

	compile(prg, PASS1);

	prg->s->p = 0;
	prg->s->line = 1;
	prg->s->start_line = 0;
	prg->s->program.clear();
	prg->functions.clear();

	compile(prg, PASS2);

	prg->s->p = 0;
	prg->s->line = 1;
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
	return t;
}

Token token_string(std::string token, std::string s)
{
	Token t;
	t.type = Token::STRING;
	t.token = token;
	t.s = s;
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

Variable *get_pointer(Variable &v)
{
	return v.p;
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

Variable &as_variable(Program *prg, Token &t)
{
	return as_variable_inline(prg, t);
}

double as_number(Program *prg, Token &t)
{
	return as_number_inline(prg, t);
}

std::string as_string(Program *prg, Token &t)
{
	return as_string_inline(prg, t);
}

int as_label(Program *prg, Token &t)
{
	return as_label_inline(prg, t);
}

int as_function(Program *prg, Token &t)
{
	return as_function_inline(prg, t);
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

double evaluate_expression(Program *prg, Variable::Expression &e)
{
	return expression_handlers[e.i](prg, e.v);
}

Variable &go_fish(Program *prg, Variable::Fish &f)
{
	Variable *v = &prg->variables[f.c_i];
	int type = v->type;

	int index = 0;
	std::string key;

	for (size_t i = 0; i < f.v.size(); i++) {
		if (type == Variable::VECTOR) {
			index = as_number_inline(prg, f.v[i]);
		}
		else {
			key = as_string_inline(prg, f.v[i]);
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

