//#define DEBUG_LINE_NUMBERS

#include <sys/stat.h>

#include "booboo/booboo.h"
#include "booboo/internal.h"

namespace booboo {

std::map<std::string, int> library_map;
std::vector<library_func> library;
std::map<char, token_func> token_map;

std::string reset_game_name;
std::string main_program_name;
int return_code;
bool quit;

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
		throw Error("Error getting file size: " + filename);
	}

	FILE *file = fopen(filename.c_str(), "rb");

	if (file == nullptr) {
		throw Error("File not found: " + filename);
	}

	char *buf = new char[_sz+1];

	if (fread(buf, _sz, 1, file) != 1) {
		throw Error("File load error: " + filename);
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

static void skip_whitespace(Program *prg)
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

// And this all makes BooBoo work

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

static std::string tokenfunc_label(booboo::Program *prg)
{
	prg->s->p++;
	return ":";
}

static std::string tokenfunc_string(booboo::Program *prg)
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

static std::string tokenfunc_openbrace(booboo::Program *prg)
{
	prg->s->p++;
	return "{";
}

static std::string tokenfunc_closebrace(booboo::Program *prg)
{
	prg->s->p++;
	return "}";
}

static std::string tokenfunc_comment(booboo::Program *prg)
{
	prg->s->p++;
	return ";";
}

static std::string tokenfunc_add(booboo::Program *prg)
{
	prg->s->p++;
	return "+";
}

static std::string tokenfunc_subtract(booboo::Program *prg)
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

static std::string tokenfunc_equals(booboo::Program *prg)
{
	prg->s->p++;
	return "=";
}

static std::string tokenfunc_compare(booboo::Program *prg)
{
	prg->s->p++;
	return "?";
}

static std::string tokenfunc_multiply(booboo::Program *prg)
{
	prg->s->p++;
	return "*";
}

static std::string tokenfunc_divide(booboo::Program *prg)
{
	prg->s->p++;
	return "/";
}

static std::string tokenfunc_modulus(booboo::Program *prg)
{
	prg->s->p++;
	return "%";
}

static std::string token(Program *prg, Token::Token_Type &ret_type)
{
	skip_whitespace(prg);

	if (prg->s->p >= prg->s->code.length()) {
		return "";
	}

	std::string tok;
	char s[2];
	s[1] = 0;

	if ((prg->s->p < prg->s->code.length()-1 && prg->s->code[prg->s->p] == '-' && isdigit(prg->s->code[prg->s->p+1])) || isdigit(prg->s->code[prg->s->p])) {
		while (prg->s->p < prg->s->code.length() && (isdigit(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '.' || prg->s->code[prg->s->p] == '-')) {
			s[0] = prg->s->code[prg->s->p];
			tok += s;
			prg->s->p++;
		}
		ret_type = Token::NUMBER;
		return tok;
	}
	else if (isalpha(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '_') {
		while (prg->s->p < prg->s->code.length() && (isdigit(prg->s->code[prg->s->p]) || isalpha(prg->s->code[prg->s->p]) || prg->s->code[prg->s->p] == '_')) {
			s[0] = prg->s->code[prg->s->p];
			tok += s;
			prg->s->p++;
		}
		ret_type = Token::SYMBOL;
		return tok;
	}

	ret_type = Token::STRING;

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

			//for (int i = start_line+nlines; i < prg->real_line_numbers.size(); i++) {
				//prg->real_line_numbers[i] += nlines-1;
			//}

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

static void compile(Program *prg, Pass pass)
{
	int p_bak = prg->s->p;
	int line_bak = prg->s->line;

	std::string tok;
	Token::Token_Type tt;

	int var_i = 0;
	int func_i = 0;

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
		else if (tok == "function") {
			std::string func_name = token(prg, tt);

			Variable v;
			v.name = func_name;
			v.type = Variable::FUNCTION;
			v.n = func_i++;
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
			std::map<std::string, int> backup;
			//std::vector<std::string> new_vars;
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
					if (prg->variables_map.find(tok2) != prg->variables_map.end()) {
						if (pass == PASS1) {
							printf("duplicate label %s\n", tok2.c_str());
						}
						backup[tok2] = prg->variables_map[tok2];
					}
					/*
					else {
						new_vars.push_back(tok2);
					}
					*/

					prg->variables_map[tok2] = var_i++;
					if (pass == PASS1) {
						prg->variables.push_back(v);
					}
				}
				else if (tok == "number" || tok == "string" || tok == "vector") {
					std::string tok2 = token(prg, tt);
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					if (pass == PASS2) {
						if (func.line_numbers.size() != 0) {
							func.s->pc++;
						}
						func.line_numbers.push_back(prg->s->line);
					}
					//if (std::find(new_vars.begin(), new_vars.end(), tok2) == new_vars.end()) {
						if (prg->variables_map.find(tok2) != prg->variables_map.end()) {
							backup[tok2] = prg->variables_map[tok2];
						}
						/*
						else {
							new_vars.push_back(tok2);
						}
						*/
						prg->variables_map[tok2] = var_i++;
						Variable v;
						v.name = tok2;
						if (tok == "number") {
							v.type = Variable::NUMBER;
						}
						else if (tok == "string") {
							v.type = Variable::STRING;
						}
						else {
							v.type = Variable::VECTOR;
						}
						if (pass == PASS1) {
							prg->variables.push_back(v);
						}
					//}
					Token t;
					t.type = Token::SYMBOL;
					t.i = prg->variables_map[tok2];
					t.s = tok2;
					t.token = tok2;
					func.s->program[func.s->program.size()-1].data.push_back(t);
				}
				else if (library_map.find(tok) != library_map.end()) {
					Statement s;
					s.method = library_map[tok];
					func.s->program.push_back(s);
					if (pass == PASS2) {
						if (func.line_numbers.size() != 0) {
							func.s->pc++;
						}
						func.line_numbers.push_back(prg->s->line);
					}
				}
				else if (is_param) {
					if (prg->variables_map.find(tok) != prg->variables_map.end()) {
						backup[tok] = prg->variables_map[tok];
					}
					/*
					else {
						new_vars.push_back(tok);
					}
					*/
					int param_i = var_i++;
					prg->variables_map[tok] = param_i;
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
			for (it = backup.begin(); it != backup.end(); it++) {
				prg->variables_map[(*it).first] = (*it).second;
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
		else if (tok == "number" || tok == "string" || tok == "vector") {
			std::string tok2 = token(prg, tt);
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			if (pass == PASS2) {
				if (prg->line_numbers.size() != 0) {
					prg->s->pc++;
				}
				prg->line_numbers.push_back(prg->s->line);
			}
			prg->variables_map[tok2] = var_i++;
			Variable v;
			v.name = tok2;
			if (tok == "number") {
				v.type = Variable::NUMBER;
			}
			else if (tok == "string") {
				v.type = Variable::STRING;
			}
			else {
				v.type = Variable::VECTOR;
			}
			if (pass == PASS1) {
				prg->variables.push_back(v);
			}
			Token t;
			t.type = Token::SYMBOL;
			t.i = prg->variables_map[tok2];
			t.s = tok2;
			t.token = tok2;
			prg->s->program[prg->s->program.size()-1].data.push_back(t);
		}
		else if (library_map.find(tok) != library_map.end()) {
			Statement s;
			s.method = library_map[tok];
			prg->s->program.push_back(s);
			if (pass == PASS2) {
				if (prg->line_numbers.size() != 0) {
					prg->s->pc++;
				}
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
	prg->s->p = 0;
	prg->s->line = 1;
	prg->s->pc = 0;

	while (interpret(prg)) {
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

bool interpret(Program *prg)
{
	bool ret = true;

	if (prg->s->pc >= prg->s->program.size()) {
		return false;
	}

	Statement &s = prg->s->program[prg->s->pc];

	unsigned int pc_bak = prg->s->pc;

	library_func func = library[s.method];
	ret = func(prg, s.data);

	if (pc_bak == prg->s->pc) {
		prg->s->pc++;
	}

	return ret;
}

void destroy_program(Program *prg)
{
	prg->variables.clear();
	prg->functions.clear();

	for (size_t i = 0; i < prg->functions.size(); i++) {
		delete prg->functions[i].s;
	}
	delete prg->s;
	prg->black_box.clear();
	delete prg;
}

void add_syntax(std::string name, library_func processing)
{
	library_map[name] = library.size();
	library.push_back(processing);
}

void add_token(char token, token_func func)
{
	token_map[token] = func;
}

static void init_token_map()
{
	add_token(':', tokenfunc_label);
	add_token('"', tokenfunc_string);
	add_token('{', tokenfunc_openbrace);
	add_token('}', tokenfunc_closebrace);
	add_token(';', tokenfunc_comment);
	
	add_token('+', tokenfunc_add);
	add_token('-', tokenfunc_subtract);
	add_token('=', tokenfunc_equals);
	add_token('?', tokenfunc_compare);
	add_token('*', tokenfunc_multiply);
	add_token('/', tokenfunc_divide);
	add_token('%', tokenfunc_modulus);
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

#ifdef DEBUG_LINE_NUMBERS
FILE *foo = fopen("debug_line_numbers.txt", "w");
fprintf(foo, "%s", prg->s->code.c_str());
fprintf(foo, "---\n");
for (size_t i = 0; i < prg->real_line_numbers.size(); i++) {
	fprintf(foo, "%d\n", prg->real_line_numbers[i]);
}
fclose(foo);
#endif
		
	compile(prg, PASS1);

	prg->s->p = 0;
	prg->s->line = 1;
	prg->s->start_line = 0;
	prg->s->program.clear();
	prg->functions.clear();

	compile(prg, PASS2);

	for (int i = 0; i < int(prg->variables.size())-1; i++) {
		if (prg->variables[i].type != Variable::LABEL) {
			continue;
		}
		for (size_t j = i+1; j < prg->variables.size(); j++) {
			if (prg->variables[j].type != Variable::LABEL) {
				continue;
			}
			if (prg->variables[i].name == prg->variables[j].name) {
				throw Error(std::string(__FUNCTION__) + ": " + "Duplicate labels \"" + prg->variables[i].name + "\"");
			}
		}
	}
	
#ifdef DEBUG_LINE_NUMBERS
foo = fopen("debug_line_numbers.txt", "a");
fprintf(foo, "---\n");
for (size_t i = 0; i < prg->line_numbers.size(); i++) {
	if (i >= prg->s->program.size()) {
		fprintf(foo, "-- line_numbers bigger than program --\n");
		break;
	}
	Statement &s = prg->s->program[i];
	std::map<std::string, int>::iterator it;
	std::string op = "UNKNOWN";
	for (it = library_map.begin(); it != library_map.end(); it++) {
		if ((*it).second == s.method) {
			op = (*it).first;
			break;
		}
	}
	fprintf(foo, "%d (%s)\n", prg->line_numbers[i], op.c_str());
}
for (size_t i = 0; i < prg->functions.size(); i++) {
	fprintf(foo, "--- %s ---\n", prg->functions[i].name.c_str());
	for (size_t j = 0; j < prg->functions[i].line_numbers.size(); j++) {
		if (j >= prg->functions[i].program.size()) {
			fprintf(foo, "-- line_numbers bigger than program --\n");
			break;
		}
		Statement &s = prg->functions[i].program[j];
		std::map<std::string, int>::iterator it;
		std::string op = "UNKNOWN";
		for (it = library_map.begin(); it != library_map.end(); it++) {
			if ((*it).second == s.method) {
				op = (*it).first;
				break;
			}
		}
		fprintf(foo, "%d (%s)\n", prg->functions[i].line_numbers[j], op.c_str());
	}
}
fclose(foo);
#endif

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

} // end namespace booboo

