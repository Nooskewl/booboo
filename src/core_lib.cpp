#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>

#include "booboo/booboo.h"
#include "booboo/internal.h"

#include <shim4/shim4.h>

namespace booboo {

bool breaker_reset(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	reset_game_name = as_string_inline(prg, v[0]);

	return false;
}

bool breaker_exit(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	return_code = as_number_inline(prg, v[0]);
	reset_game_name = "";
	quit = true;
	return false;
}

bool breaker_return(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = prg->s->result;
	Variable &v2 = as_variable_inline(prg, v[0]);

	if (v2.type == Variable::EXPRESSION) {
		v1.type = Variable::NUMBER;
		v1.n = evaluate_expression(prg, v2.e);
	}
	else if (v2.type == Variable::FISH) {
		v1 = go_fish(prg, v2.f);
	}
	else {
		v1 = v2;
	}

	return false;
}

bool corefunc_number(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)
	return true;
}

bool corefunc_string(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)
	return true;
}

bool corefunc_vector(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)
	
	for (size_t i = 0; i < v.size(); i++) {
		prg->variables[v[i].i].v.clear();
	}

	return true;
}

bool corefunc_map(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)
	
	for (size_t i = 0; i < v.size(); i++) {
		prg->variables[v[i].i].m.clear();
	}
	return true;
}

bool corefunc_pointer(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)
	return true;
}

static std::string typeof_var(Variable &v1)
{
	std::string res;
	if (v1.type == Variable::NUMBER) {
		res = "number";
	}
	else if (v1.type == Variable::STRING) {
		res = "string";
	}
	else if (v1.type == Variable::VECTOR) {
		res = "vector";
	}
	else if (v1.type == Variable::MAP) {
		res = "map";
	}
	else if (v1.type == Variable::POINTER) {
		res = "pointer";
	}
	else if (v1.type == Variable::FUNCTION) {
		res = "function";
	}
	else if (v1.type == Variable::LABEL) {
		res = "label";
	}
	else {
		res = "unknown";
	}

	return res;
}

bool corefunc_typeof(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

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

		if (v1.type == Variable::VECTOR) {
			if (v.size() > 2) {
				std::vector<Variable> *p;
				p = &v1.v;
				int index = 0;
				for (size_t i = 2; i < v.size(); i++) {
					index = as_number_inline(prg, v[i]);
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
		else if (v1.type == Variable::MAP) {
			if (v.size() > 2) {
				std::map<std::string, Variable> *p;
				p = &v1.m;
				std::string key = "";
				for (size_t i = 2; i < v.size(); i++) {
					key = as_string_inline(prg, v[i]);
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

	Variable &v2 = as_variable_inline(prg, v[0]);

	if (v2.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v2.s = res;

	return true;
}

bool corefunc_address(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	if (v[0].type != Token::SYMBOL || v[1].type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	Variable &v1 = prg->variables[v[0].i];
	Variable &v2 = prg->variables[v[1].i];

	v1.p = &v2;

	return true;
}

bool corefunc_for(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(5)

	Variable &count = as_variable_inline(prg, v[0]);
	if (count.type == Variable::POINTER) {
		count = (*count.p);
	}
	if (count.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	count.n = as_number_inline(prg, v[1]);
	Variable &expr = as_variable_inline(prg, v[2]);
	int increment = as_number_inline(prg, v[3]);
	unsigned int end_label = as_label_inline(prg, v[4]);

	if (expr.type != Variable::EXPRESSION) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	if (evaluate_expression(prg, expr.e) == 0) {
		prg->s->pc = end_label;
		return true;
	}

	prg->s->pc++;

	unsigned int start = prg->s->pc;

	while (true) {
		if (interpret(prg, 1) == false) {
			return false;
		}
		if (prg->s->pc < start || prg->s->pc > end_label) {
			break;
		}
		if (prg->s->pc == end_label) {
			count.n += increment;
			if (evaluate_expression(prg, expr.e) == 0) {
				prg->s->pc++;
				break;
			}
			prg->s->pc = start;
		}
	}

	return true;
}

bool corefunc_if(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	int end = v.size();
	end -= (end % 2);
	int prev = -1;

	for (int i = 0; i < end; i += 2) {
		bool b = as_number_inline(prg, v[i]);
		if (b) {
			if (prev == -1) {
				prg->s->pc++;
			}
			else {
				prg->s->pc = prev;
			}
			unsigned int start = prg->s->pc;
			unsigned int end_block = as_label_inline(prg, v[i+1]);
			while (prg->s->pc != end_block) {
				interpret(prg, 1);
				if (prg->s->pc < start || prg->s->pc > end_block) {
					return true;
				}
			}
			prg->s->pc = as_label_inline(prg, v[v.size()-1]);
			return true;
		}
		prev = as_label_inline(prg, v[i+1]);
	}

	if (v.size() <= 2 || v.size() % 2 == 1) {
		prg->s->pc = prev;
		unsigned int start = prg->s->pc;
		unsigned int end_block = as_label_inline(prg, v[v.size()-1]);
		while (prg->s->pc != end_block) {
			interpret(prg, 1);
			if (prg->s->pc < start || prg->s->pc > end_block) {
				return true;
			}
		}
		prg->s->pc++;
	}
	else {
		unsigned int end_block = as_label_inline(prg, v[v.size()-1]);
		prg->s->pc = end_block;
	}

	return true;
}

bool corefunc_getenv(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string get = as_string_inline(prg, v[1]);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.s = getenv(get.c_str());

	return true;
}

bool corefunc_set(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = as_number_inline(prg, v[1]);
	}
	else if (v1.type == Variable::STRING) {
		v1.s = as_string_inline(prg, v[1]);
	}
	else if (v1.type == Variable::VECTOR) {
		Variable &v2 = as_variable_inline(prg, v[1]);
		if (v2.type == Variable::VECTOR) {
			v1.v = v2.v;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
		}
	}
	else if (v1.type == Variable::MAP) {
		Variable &v2 = as_variable_inline(prg, v[1]);
		if (v2.type == Variable::MAP) {
			v1.m = v2.m;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
		}
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool corefunc_add(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n += as_number_inline(prg, v[i]);
		}
		else if (v1.type == Variable::STRING) {
			v1.s += as_string_inline(prg, v[i]);
		}
		else if (v1.type == Variable::VECTOR) {
			Variable &v2 = as_variable_inline(prg, v[i]);
			if (v2.type != Variable::VECTOR) {
				throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
			}
			else {
				v1.v.insert(v1.v.end(), v2.v.begin(), v2.v.end());
			}
		}
		else if (v1.type == Variable::MAP) {
			Variable &v2 = as_variable_inline(prg, v[i]);
			if (v2.type != Variable::MAP) {
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

bool corefunc_subtract(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n -= as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool corefunc_multiply(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n *= as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool corefunc_divide(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n /= as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool corefunc_label(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	return true;
}

bool corefunc_goto(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	prg->s->pc = as_label_inline(prg, v[0]);

	return true;
}

bool corefunc_compare(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	bool is_num = false;
	double n;

	if (v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::NUMBER) {
			is_num = true;
			n = var.n;
		}
	}
	else if (v[0].type == Token::NUMBER) {
		is_num = true;
		n = v[0].n;
	}

	if (is_num) {
		double n2 = as_number_inline(prg, v[1]);
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
			Variable &var = as_variable_inline(prg, v[0]);
			if (var.type == Variable::STRING) {
				a_string = true;
				s1 = var.s;
			}
		}
		
		if (v[1].type == Token::STRING) {
			b_string = true;
			s2 = v[1].s;
		}
		else if (v[1].type == Token::SYMBOL) {
			Variable &var = as_variable_inline(prg, v[1]);
			if (var.type == Variable::STRING) {
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

bool corefunc_je(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag == 0) {
		prg->s->pc = as_label_inline(prg, v[0]);
	}

	return true;
}

bool corefunc_jne(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag != 0) {
		prg->s->pc = as_label_inline(prg, v[0]);
	}

	return true;
}

bool corefunc_jl(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag < 0) {
		prg->s->pc = as_label_inline(prg, v[0]);
	}

	return true;
}

bool corefunc_jle(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag <= 0) {
		prg->s->pc = as_label_inline(prg, v[0]);
	}

	return true;
}

bool corefunc_jg(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag > 0) {
		prg->s->pc = as_label_inline(prg, v[0]);
	}

	return true;
}

bool corefunc_jge(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	if (prg->compare_flag >= 0) {
		prg->s->pc = as_label_inline(prg, v[0]);
	}

	return true;
}

bool corefunc_call(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)

	int function = as_function_inline(prg, v[0]);

	call_void_function(prg, function, v, 1);

	return true;
}

bool corefunc_call_result(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &result = prg->variables[v[0].i];
	int function = as_function_inline(prg, v[1]);

	call_function(prg, function, v, result, 2);

	return true;
}

bool corefunc_print(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string fmt = as_string_inline(prg, v[0]);
	int _tok = 1;
	
	int prev = 0;
	int arg_count = 0;

	for (size_t i = 0; i < fmt.length(); i++) {
		if (fmt[i] == '%' && prev != '%') {
			arg_count++;
		}
		prev = fmt[i];
	}

	std::string result;
	int c = 0;
	prev = 0;

	for (int arg = 0; arg < arg_count; arg++) {
		int start = c;
		std::string format;
		int fmt_len = 1;
		while (c < (int)fmt.length()) {
			if (fmt[c] == '%' && prev != '%') {
				if (c < (int)fmt.length()-1) {
					if (fmt[c+1] == '(') {
						int l = 2;
						int st = c+l;
						if (c+l >= (int)fmt.length()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid format specifier at " + get_error_info(prg));
						}
						while (fmt[c+l] != ')' && c+l < (int)fmt.length()) {
							l++;
						}
						if (c+l >= (int)fmt.length()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid format specifier at " + get_error_info(prg));
						}
						format = fmt.substr(st, l-2);
						fmt_len = l + 1;
					}
				}
				break;
			}
			prev = fmt[c];
			c++;
		}

		result += fmt.substr(start, c-start);

		c += fmt_len;
		prev = fmt[c];

		std::string val;

		if (v[_tok].type == Token::NUMBER) {
			format = (format == "") ? "g" : format;
			char buf[1000];
			snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].n);
			val = buf;
		}
		else if (v[_tok].type == Token::STRING) {
			format = (format == "") ? "s" : format;
			char buf[1000];
			snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].s.c_str());
			val = buf;
		}
		else {
			Variable &v1 = as_variable_inline(prg, v[_tok]);
			if (v1.type == Variable::NUMBER) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.n);
				val = buf;
			}
			else if (v1.type == Variable::STRING) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.s.c_str());
				val = buf;
			}
			else if (v1.type == Variable::EXPRESSION) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e));
				val = buf;
			}
			else if (v1.type == Variable::FISH) {
				Variable &var = go_fish(prg, v1.f);
				if (var.type == Variable::NUMBER) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					val = buf;
				}
				else {
					if (var.type == Variable::STRING) {
						val = var.s;
					}
					else if (var.type == Variable::VECTOR) {
						val = "-vector-";
					}
					else if (var.type == Variable::MAP) {
						val = "-map-";
					}
					else if (var.type == Variable::POINTER) {
						val = "-pointer-";
					}
					else if (var.type == Variable::FUNCTION) {
						val = "-function-";
					}
					else if (var.type == Variable::LABEL) {
						val = "-label-";
					}
					else {
						val = "-unknown-";
					}
					format = (format == "") ? "s" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), val.c_str());
					val = buf;
				}
			}
			else {
				if (v1.type == Variable::VECTOR) {
					val = "-vector-";
				}
				else if (v1.type == Variable::MAP) {
					val = "-map-";
				}
				else if (v1.type == Variable::POINTER) {
					val = "-pointer-";
				}
				else if (v1.type == Variable::FUNCTION) {
					val = "-function-";
				}
				else if (v1.type == Variable::LABEL) {
					val = "-label-";
				}
				else {
					val = "-unknown-";
				}
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), val.c_str());
				val = buf;
			}
		}

		_tok++;

		result += val;
	}

	if (c < (int)fmt.length()) {
		result += fmt.substr(c);
	}

	printf("%s", result.c_str());

	return true;
}

bool corefunc_input(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &var = as_variable_inline(prg, v[0]);

	if (var.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	if (std::cin.eof()) {
		var.s = "";
	}
	else {
		std::cin >> var.s;
	}

	return true;
}

bool corefunc_mkdir(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string path = as_string_inline(prg, v[0]);

	util::mkdir(path);

	return true;
}

bool corefunc_get_system_language(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.s = util::get_system_language();

	return true;
}

bool corefunc_list_directory(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	std::vector<Variable> &vec = as_vector_inline(prg, v[0]);
	std::string glob = as_string_inline(prg, v[1]);

	util::List_Directory l(glob);

	std::string fn;

	while ((fn = l.next()) != "") {
		struct stat s;
		if (stat(fn.c_str(), &s) == 0) {
			if ((s.st_mode & S_IFMT) == S_IFDIR) {
				fn += "/";
			}
		}
		Variable v;
		v.type = Variable::STRING;
		v.name = "-constant-";
		v.s = fn;
		vec.push_back(v);
	}

	return true;
}

bool stringfunc_format(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string fmt = as_string_inline(prg, v[1]);
	int _tok = 2;
	
	int prev = 0;
	int arg_count = 0;

	for (size_t i = 0; i < fmt.length(); i++) {
		if (fmt[i] == '%' && prev != '%') {
			arg_count++;
		}
		prev = fmt[i];
	}

	std::string result;
	int c = 0;
	prev = 0;

	for (int arg = 0; arg < arg_count; arg++) {
		int start = c;
		std::string format;
		int fmt_len = 1;
		while (c < (int)fmt.length()) {
			if (fmt[c] == '%' && prev != '%') {
				if (c < (int)fmt.length()-1) {
					if (fmt[c+1] == '(') {
						int l = 2;
						int st = c+l;
						if (c+l >= (int)fmt.length()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid format specifier at " + get_error_info(prg));
						}
						while (fmt[c+l] != ')' && c+l < (int)fmt.length()) {
							l++;
						}
						if (c+l >= (int)fmt.length()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid format specifier at " + get_error_info(prg));
						}
						format = fmt.substr(st, l-2);
						fmt_len = l + 1;
					}
				}
				break;
			}
			prev = fmt[c];
			c++;
		}

		result += fmt.substr(start, c-start);

		c += fmt_len;
		prev = fmt[c];

		std::string val;

		if (v[_tok].type == Token::NUMBER) {
			format = (format == "") ? "g" : format;
			char buf[1000];
			snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].n);
			val = buf;
		}
		else if (v[_tok].type == Token::STRING) {
			format = (format == "") ? "s" : format;
			char buf[1000];
			snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].s.c_str());
			val = buf;
		}
		else {
			Variable &v1 = as_variable_inline(prg, v[_tok]);
			if (v1.type == Variable::NUMBER) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.n);
				val = buf;
			}
			else if (v1.type == Variable::STRING) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.s.c_str());
				val = buf;
			}
			else if (v1.type == Variable::EXPRESSION) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e));
				val = buf;
			}
			else if (v1.type == Variable::FISH) {
				Variable &var = go_fish(prg, v1.f);
				if (var.type == Variable::NUMBER) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					val = buf;
				}
				else {
					if (var.type == Variable::STRING) {
						val = var.s;
					}
					else if (var.type == Variable::VECTOR) {
						val = "-vector-";
					}
					else if (var.type == Variable::MAP) {
						val = "-map-";
					}
					else if (var.type == Variable::POINTER) {
						val = "-pointer-";
					}
					else if (var.type == Variable::FUNCTION) {
						val = "-function-";
					}
					else if (var.type == Variable::LABEL) {
						val = "-label-";
					}
					else {
						val = "-unknown-";
					}
					format = (format == "") ? "s" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), val.c_str());
					val = buf;
				}
			}
			else {
				if (v1.type == Variable::VECTOR) {
					val = "-vector-";
				}
				else if (v1.type == Variable::MAP) {
					val = "-map-";
				}
				else if (v1.type == Variable::POINTER) {
					val = "-pointer-";
				}
				else if (v1.type == Variable::FUNCTION) {
					val = "-function-";
				}
				else if (v1.type == Variable::LABEL) {
					val = "-label-";
				}
				else {
					val = "-unknown-";
				}
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), val.c_str());
				val = buf;
			}
		}

		_tok++;

		result += val;
	}

	if (c < (int)fmt.length()) {
		result += fmt.substr(c);
	}

	v1.type = Variable::STRING;
	v1.s = result;

	return true;
}

bool stringfunc_scan(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(4)

	Variable &v1 = as_variable_inline(prg, v[0]);
	if (v1.type == Variable::POINTER) {
		v1 = *v1.p;
	}
	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected number at " + get_error_info(prg));
	}
	std::string fmt = as_string_inline(prg, v[1]);
	std::string scan_str = as_string_inline(prg, v[2]);
	int _tok = 3;
	int p = 0;
	int scanned = 0;
	int gotten = 0;

	while (p < (int)fmt.length()) {
		int start = p;
		// scan up till %
		while (p < (int)fmt.length() && fmt[p] != '%') {
			p++;
		}
		if (p < (int)fmt.length()-1 && fmt[p+1] == '%') {
			p += 2;
			continue;
		}
		int scan_till = p >= (int)fmt.length()-1 ? -1 : fmt[p+1];
		if (p-start > 0) {
			std::string const_str = fmt.substr(start, p-start);
			bool all_whitespace = true;
			for (size_t i = 0; i < const_str.length(); i++) {
				if (!isspace(const_str[i])) {
					all_whitespace = false;
					break;
				}
			}
			if (all_whitespace) {
				while (scanned < (int)scan_str.length() && isspace(scan_str[scanned])) {
					scanned++;
				}
			}
			else {
				if (const_str != scan_str.substr(scanned, const_str.length())) {
					break;
				}
				scanned += const_str.length();
			}
		}

		Variable &var = as_variable_inline(prg, v[_tok]);
		if (var.type == Variable::POINTER) {
			var = *var.p;
		}

		int scan_start = scanned;

		while (true) {
			if (scanned >= (int)scan_str.length()) {
				break;
			}
			if (isspace(scan_till)) {
				if (isspace(scan_str[scanned])) {
					break;
				}
			}
			else {
				if (scan_str[scanned] == scan_till) {
					break;
				}
			}
			scanned++;
		}

		std::string got = scan_str.substr(scan_start, scanned-scan_start);

		if (var.type == Variable::NUMBER) {
			var.n = atof(got.c_str());
		}
		else if (var.type == Variable::STRING) {
			var.s = got;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Expected string or number at " + get_error_info(prg));
		}

		gotten++;
		_tok++;
		p++;

		if (_tok >= (int)v.size()) {
			break;
		}
	}

	v1.n = gotten;

	return true;
}

bool stringfunc_char_at(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string s = as_string_inline(prg, v[1]);
	int index = as_number_inline(prg, v[2]);

	Uint32 value = util::utf8_char(s, index);

	if (v1.type == Variable::NUMBER) {
		v1.n = value;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_length(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string s = as_string_inline(prg, v[1]);

	if (v1.type == Variable::NUMBER) {
		v1.n = util::utf8_len(s);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_from_number(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	Uint32 n = as_number_inline(prg, v[1]);

	if (v1.type == Variable::STRING) {
		v1.s = util::utf8_char_to_string(n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_substr(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	int start = as_number_inline(prg, v[1]);
	int end = -1;
	
	if (v.size() >= 3) {
		end = as_number_inline(prg, v[2]);
	}

	if (v1.type == Variable::STRING) {
		v1.s = util::utf8_substr(v1.s, start, end);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_uppercase(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::STRING) {
		v1.s = util::uppercase(v1.s);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_lowercase(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::STRING) {
		v1.s = util::lowercase(v1.s);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_trim(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::STRING) {
		v1.s = util::trim(v1.s);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sin(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = sin(v1.n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_cos(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = cos(v1.n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_atan2(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = atan2(v1.n, as_number_inline(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_abs(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = fabs(v1.n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_pow(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = pow(v1.n, as_number_inline(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sqrt(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = sqrt(v1.n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_floor(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	v1.n = (int)v1.n;

	return true;
}

bool mathfunc_neg(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = -v1.n;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_intmod(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	int d = as_number_inline(prg, v[1]);
	
	if (v1.type == Variable::NUMBER) {
		v1.n = int(v1.n) % int(d);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_fmod(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	double d = as_number_inline(prg, v[1]);

	if (v1.type == Variable::NUMBER) {
		v1.n = fmod(v1.n, d);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sign(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = util::sign(v1.n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool vectorfunc_add(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable_inline(prg, v[0]);

	Variable var;

	if (v[1].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[1].n;
	}
	else if (v[1].type == Token::SYMBOL) {
		if (prg->variables[v[1].i].type == Variable::POINTER) {
			var = prg->variables[v[1].i];
		}
		else {
			var = as_variable_inline(prg, v[1]);
			if (var.type == Variable::FISH) {
				var = go_fish(prg, var.f);
			}
			else if (var.type == Variable::EXPRESSION) {
				var.n = evaluate_expression(prg, var.e);
				var.type = Variable::NUMBER;
				var.name = "-calculated-";
			}
		}
	}
	else {
		var.type = Variable::STRING;
		var.name = "-constant-";
		var.s = v[1].s;
	}

	id.v.push_back(var);

	return true;
}

static bool vectorfunc_size(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable_inline(prg, v[0]);
	Variable &v1 = as_variable_inline(prg, v[1]);

	if (v1.type == Variable::NUMBER) {
		v1.n = id.v.size();
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool vectorfunc_set(Program *prg, std::vector<Token> &v)
{
	Variable &id = as_variable_inline(prg, v[0]);
	int val_index = v.size() - 1;
	std::vector<int> indices;

	MIN_ARGS(3)

	for (int i = 1; i < val_index; i++) {
		indices.push_back(as_number_inline(prg, v[i]));
	}

	Variable var;

	if (v[val_index].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[val_index].n;
	}
	else if (v[val_index].type == Token::SYMBOL) {
		if (prg->variables[v[val_index].i].type == Variable::POINTER) {
			var = prg->variables[v[val_index].i];
		}
		else {
			var = as_variable_inline(prg, v[val_index]);
		}
	}
	else {
		var.type = Variable::STRING;
		var.name = "-constant-";
		var.s = v[2].s;
	}

	std::vector<Variable> *p = nullptr;

	for (size_t i = 0; i < indices.size()-1; i++) {
		int index = indices[i];
		if (p == nullptr) {
			if (index < 0 || index >= (int)id.v.size()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
			}
			p = &id.v[index].v;
		}
		else {
			if (index < 0 || index >= (int)(*p).size()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
			}
			p = &(*p)[index].v;
		}
	}
			
	if (p == nullptr) {
		p = &id.v;
	}

	(*p)[indices[indices.size()-1]] = var;

	return true;
}

static bool vectorfunc_insert(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &id = as_variable_inline(prg, v[0]);
	double index = as_number_inline(prg, v[1]);

	if (index < 0 || index > id.v.size()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
	}

	Variable var;

	if (v[2].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[2].n;
	}
	else if (v[2].type == Token::SYMBOL) {
		if (prg->variables[v[2].i].type == Variable::POINTER) {
			var = prg->variables[v[2].i];
		}
		else {
			var = as_variable_inline(prg, v[2]);
		}
	}
	else {
		var.type = Variable::STRING;
		var.name = "-constant-";
		var.s = v[2].s;
	}

	id.v.insert(id.v.begin()+index, var);

	return true;
}

static bool vectorfunc_get(Program *prg, std::vector<Token> &v)
{
	Variable &id = as_variable_inline(prg, v[0]);
	std::vector<int> indices;

	MIN_ARGS(3)

	for (size_t i = 2; i < v.size(); i++) {
		int index = as_number_inline(prg, v[i]);
		indices.push_back(index);
	}

	std::vector<Variable> *p = nullptr;

	for (size_t i = 0; i < indices.size(); i++) {
		int index = indices[i];
		if (i == indices.size()-1) {
			if (p == nullptr) {
				p = &id.v;
			}
			if (index < 0 || index >= (int)(*p).size()) {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
			}
			if (prg->variables[v[1].i].type == Variable::POINTER) {
				Variable &v1 = prg->variables[v[1].i];
				v1.p = (*p)[index].p;
			}
			else {
				Variable &v1 = as_variable_inline(prg, v[1]);
				std::string bak = v1.name;
				v1 = (*p)[index];
				v1.name = bak;
			}
		}
		else {
			if (p == nullptr) {
				if (index < 0 || index >= (int)id.v.size()) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
				}
				p = &id.v[index].v;
			}
			else {
				if (index < 0 || index >= (int)(*p).size()) {
					throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
				}
				p = &(*p)[index].v;
			}
		}
	}

	return true;
}

static bool vectorfunc_erase(Program *prg, std::vector<Token> &v)
{

	COUNT_ARGS(2)

	Variable &id = as_variable_inline(prg, v[0]);
	double index = as_number_inline(prg, v[1]);

	if (index < 0 || index >= id.v.size()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
	}

	id.v.erase(id.v.begin() + int(index));

	return true;
}

static bool vectorfunc_clear(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &id = as_variable_inline(prg, v[0]);

	id.v.clear();

	return true;
}

static bool mapfunc_set(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(3)

	Variable &id = as_variable_inline(prg, v[0]);

	Variable var;
	int val_index = v.size()-1;

	if (v[val_index].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[val_index].n;
	}
	else if (v[val_index].type == Token::SYMBOL) {
		if (prg->variables[v[val_index].i].type == Variable::POINTER) {
			var = prg->variables[v[val_index].i];
		}
		else {
			var = as_variable_inline(prg, v[val_index]);
			if (var.type == Variable::FISH) {
				var = go_fish(prg, var.f);
			}
			else if (var.type == Variable::EXPRESSION) {
				var.n = evaluate_expression(prg, var.e);
				var.type = Variable::NUMBER;
				var.name = "-calculated-";
			}
		}
	}
	else {
		var.type = Variable::STRING;
		var.name = "-constant-";
		var.s = v[val_index].s;
	}

	std::map<std::string, Variable> *p = &id.m;
	std::string key;

	for (size_t i = 1; i < (size_t)val_index; i++) {
		key = as_string_inline(prg, v[i]);
		if ((int)i < val_index-1) {
			p = &(*p)[key].m;
		}
	}

	(*p)[key] = var;

	return true;
}

static bool mapfunc_get(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(3)

	Variable &id = as_variable_inline(prg, v[0]);

	std::map<std::string, Variable> *p = &id.m;
	std::string key;

	for (size_t i = 2; i < v.size(); i++) {
		key = as_string_inline(prg, v[i]);
		if (i < v.size()-1) {
			p = &(*p)[key].m;
		}
	}

	if (prg->variables[v[1].i].type == Variable::POINTER) {
		Variable &v1 = prg->variables[v[1].i];
		v1.p = (*p)[key].p;
	}
	else {
		Variable &v1 = as_variable_inline(prg, v[1]);
		std::string bak = v1.name;
		v1 = (*p)[key];
		v1.name = bak;
	}

	return true;
}

static bool mapfunc_clear(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &id = as_variable_inline(prg, v[0]);

	id.m.clear();

	return true;
}

static bool mapfunc_erase(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable_inline(prg, v[0]);
	std::string key = as_string_inline(prg, v[1]);

	std::map<std::string, Variable>::iterator it = id.m.find(key);

	if (it == id.m.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid map key at " + get_error_info(prg));
	}

	id.m.erase(it);

	return true;
}

static bool mapfunc_keys(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	std::map<std::string, Variable> &m = as_map_inline(prg, v[0]);
	Variable &vec_var = as_variable_inline(prg, v[1]);

	if (vec_var.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected a vector at " + get_error_info(prg));
	}

	vec_var.v.clear();

	std::map<std::string, Variable>::iterator it;

	for (it = m.begin(); it != m.end(); it++) {
		std::pair<std::string, Variable> p = *it;
		Variable var;
		var.type = Variable::STRING;
		var.name = "-constant-";
		var.s = p.first;
		vec_var.v.push_back(var);
	}

	return true;
}

static bool filefunc_open(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string filename = as_string_inline(prg, v[1]);
	std::string mode = as_string_inline(prg, v[2]);
	
	File_Info *info = file_info(prg);

	if (v1.type == Variable::NUMBER) {
		v1.n = info->file_id;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	std::fstream *f = new std::fstream;

	if (mode == "r") {
		f->open(filename, std::fstream::in);
	}
	else if (mode == "w") {
		f->open(filename, std::fstream::out);
	}
	else if (mode == "a") {
		f->open(filename, std::fstream::out | std::fstream::app);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid file open mode at " + get_error_info(prg));
	}

	if (f->fail()) {
		v1.n = -1;
		return true;
	}

	info->files[info->file_id++] = f;

	return true;
}

static bool filefunc_close(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number_inline(prg, v[0]);
	
	File_Info *info = file_info(prg);

	info->files[id]->close();

	return true;
}

static bool filefunc_read(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &var = as_variable_inline(prg, v[1]);

	if (var.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected string at " + get_error_info(prg));
	}

	File_Info *info = file_info(prg);

	(*info->files[id]) >> var.s;

	return true;
}

static bool filefunc_read_line(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	Variable &var = as_variable_inline(prg, v[1]);

	if (var.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected string at " + get_error_info(prg));
	}

	File_Info *info = file_info(prg);

	if ((*info->files[id]).eof()) {
		var.s = "";
	}
	else {
		std::getline(*info->files[id], var.s);
	}

	return true;
}

static bool filefunc_write(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::string val = as_string_inline(prg, v[1]);

	File_Info *info = file_info(prg);

	(*info->files[id]) << val;

	return true;
}

bool filefunc_print(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number_inline(prg, v[0]);
	std::string fmt = as_string_inline(prg, v[1]);
	int _tok = 2;
	
	int prev = 0;
	int arg_count = 0;

	for (size_t i = 0; i < fmt.length(); i++) {
		if (fmt[i] == '%' && prev != '%') {
			arg_count++;
		}
		prev = fmt[i];
	}

	std::string result;
	int c = 0;
	prev = 0;

	for (int arg = 0; arg < arg_count; arg++) {
		int start = c;
		std::string format;
		int fmt_len = 1;
		while (c < (int)fmt.length()) {
			if (fmt[c] == '%' && prev != '%') {
				if (c < (int)fmt.length()-1) {
					if (fmt[c+1] == '(') {
						int l = 2;
						int st = c+l;
						if (c+l >= (int)fmt.length()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid format specifier at " + get_error_info(prg));
						}
						while (fmt[c+l] != ')' && c+l < (int)fmt.length()) {
							l++;
						}
						if (c+l >= (int)fmt.length()) {
							throw Error(std::string(__FUNCTION__) + ": " + "Invalid format specifier at " + get_error_info(prg));
						}
						format = fmt.substr(st, l-2);
						fmt_len = l + 1;
					}
				}
				break;
			}
			prev = fmt[c];
			c++;
		}

		result += fmt.substr(start, c-start);

		c += fmt_len;
		prev = fmt[c];

		std::string val;

		if (v[_tok].type == Token::NUMBER) {
			format = (format == "") ? "g" : format;
			char buf[1000];
			snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].n);
			val = buf;
		}
		else if (v[_tok].type == Token::STRING) {
			format = (format == "") ? "s" : format;
			char buf[1000];
			snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].s.c_str());
			val = buf;
		}
		else {
			Variable &v1 = as_variable_inline(prg, v[_tok]);
			if (v1.type == Variable::NUMBER) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.n);
				val = buf;
			}
			else if (v1.type == Variable::STRING) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.s.c_str());
				val = buf;
			}
			else if (v1.type == Variable::EXPRESSION) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e));
				val = buf;
			}
			else if (v1.type == Variable::FISH) {
				Variable &var = go_fish(prg, v1.f);
				if (var.type == Variable::NUMBER) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					val = buf;
				}
				else {
					if (var.type == Variable::STRING) {
						val = var.s;
					}
					else if (var.type == Variable::VECTOR) {
						val = "-vector-";
					}
					else if (var.type == Variable::MAP) {
						val = "-map-";
					}
					else if (var.type == Variable::POINTER) {
						val = "-pointer-";
					}
					else if (var.type == Variable::FUNCTION) {
						val = "-function-";
					}
					else if (var.type == Variable::LABEL) {
						val = "-label-";
					}
					else {
						val = "-unknown-";
					}
					format = (format == "") ? "s" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), val.c_str());
					val = buf;
				}
			}
			else {
				if (v1.type == Variable::VECTOR) {
					val = "-vector-";
				}
				else if (v1.type == Variable::MAP) {
					val = "-map-";
				}
				else if (v1.type == Variable::POINTER) {
					val = "-pointer-";
				}
				else if (v1.type == Variable::FUNCTION) {
					val = "-function-";
				}
				else if (v1.type == Variable::LABEL) {
					val = "-label-";
				}
				else {
					val = "-unknown-";
				}
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), val.c_str());
				val = buf;
			}
		}

		_tok++;

		result += val;
	}

	if (c < (int)fmt.length()) {
		result += fmt.substr(c);
	}
	
	File_Info *info = file_info(prg);

	(*info->files[id]) << result;

	return true;
}

bool bitfunc_or(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n | (int)as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool bitfunc_xor(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n ^ (int)as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool bitfunc_and(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n & (int)as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool bitfunc_leftshift(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n << (int)as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

bool bitfunc_rightshift(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n >> (int)as_number_inline(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

double exprfunc_add(Program *prg, std::vector<Token> &v)
{
	double n = as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n += as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_subtract(Program *prg, std::vector<Token> &v)
{
	double n = as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n -= as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_multiply(Program *prg, std::vector<Token> &v)
{
	double n = as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n *= as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_divide(Program *prg, std::vector<Token> &v)
{
	double n = as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n /= as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_modulus(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	return (int)as_number_inline(prg, v[0]) % (int)as_number_inline(prg, v[1]);
}

double exprfunc_and(Program *prg, std::vector<Token> &v)
{
	bool b = (bool)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		b = b && (bool)as_number_inline(prg, v[i]);
	}

	return b;
}

double exprfunc_or(Program *prg, std::vector<Token> &v)
{
	bool b = (bool)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		b = b || (bool)as_number_inline(prg, v[i]);
	}

	return b;
}

double exprfunc_greater(Program *prg, std::vector<Token> &v)
{
	bool b = true;

	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::STRING) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s > as_string_inline(prg, v[i]);
		}
	}
	else {
		double n = as_number_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n > as_number_inline(prg, v[i]);
		}
	}

	return b;
}

double exprfunc_less(Program *prg, std::vector<Token> &v)
{
	bool b = true;
	
	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::STRING) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s < as_string_inline(prg, v[i]);
		}
	}
	else {
		double n = as_number_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n < as_number_inline(prg, v[i]);
		}
	}

	return b;
}

double exprfunc_greaterequal(Program *prg, std::vector<Token> &v)
{
	bool b = true;
	
	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::STRING) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s >= as_string_inline(prg, v[i]);
		}
	}
	else {
		double n = as_number_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n >= as_number_inline(prg, v[i]);
		}
	}

	return b;
}

double exprfunc_lessequal(Program *prg, std::vector<Token> &v)
{
	bool b = true;
	
	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::STRING) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s <= as_string_inline(prg, v[i]);
		}
	}
	else {
		double n = as_number_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n <= as_number_inline(prg, v[i]);
		}
	}

	return b;
}

double exprfunc_equal(Program *prg, std::vector<Token> &v)
{
	bool b = true;

	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::STRING) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s == as_string_inline(prg, v[i]);
		}
	}
	else {
		double n = as_number_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n == as_number_inline(prg, v[i]);
		}
	}

	return b;
}

double exprfunc_notequal(Program *prg, std::vector<Token> &v)
{
	bool b = true;

	bool string = v[0].type == Token::STRING ? true : false;

	if (string == false && v[0].type == Token::SYMBOL) {
		Variable &var = as_variable_inline(prg, v[0]);
		if (var.type == Variable::STRING) {
			string = true;
		}
	}

	if (string) {
		std::string s = as_string_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && s != as_string_inline(prg, v[i]);
		}
	}
	else {
		double n = as_number_inline(prg, v[0]);

		for (size_t i = 1; i < v.size(); i++) {
			b = b && n != as_number_inline(prg, v[i]);
		}
	}

	return b;
}

double exprfunc_bitor(Program *prg, std::vector<Token> &v)
{
	int n = (int)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n |= (int)as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_xor(Program *prg, std::vector<Token> &v)
{
	int n = (int)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n ^= (int)as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_bitand(Program *prg, std::vector<Token> &v)
{
	int n = (int)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n &= (int)as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_leftshift(Program *prg, std::vector<Token> &v)
{
	int n = (int)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n <<= (int)as_number_inline(prg, v[i]);
	}

	return n;
}

double exprfunc_rightshift(Program *prg, std::vector<Token> &v)
{
	int n = (int)as_number_inline(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		n >>= (int)as_number_inline(prg, v[i]);
	}

	return n;
}

void start_lib_core()
{
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

	add_instruction("reset", breaker_reset);
	add_instruction("exit", breaker_exit);
	add_instruction("return", breaker_return);

	add_instruction("number", corefunc_number);
	add_instruction("string", corefunc_string);
	add_instruction("vector", corefunc_vector);
	add_instruction("map", corefunc_map);
	add_instruction("pointer", corefunc_pointer);
	
	add_instruction("typeof", corefunc_typeof);
	add_instruction("address", corefunc_address);
	add_instruction("for", corefunc_for);
	add_instruction("if", corefunc_if);
	add_instruction("getenv", corefunc_getenv);
	add_instruction("list_directory", corefunc_list_directory);

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
	
	add_instruction("print", corefunc_print);
	add_instruction("input", corefunc_input);
	add_instruction("mkdir", corefunc_mkdir);
	add_instruction("get_system_language", corefunc_get_system_language);

	add_instruction("string_format", stringfunc_format);
	add_instruction("string_scan", stringfunc_scan);
	add_instruction("string_char_at", stringfunc_char_at);
	add_instruction("string_length", stringfunc_length);
	add_instruction("string_from_number", stringfunc_from_number);
	add_instruction("string_substr", stringfunc_substr);
	add_instruction("string_uppercase", stringfunc_uppercase);
	add_instruction("string_lowercase", stringfunc_lowercase);
	add_instruction("string_trim", stringfunc_trim);

	add_instruction("sin", mathfunc_sin);
	add_instruction("cos", mathfunc_cos);
	add_instruction("atan2", mathfunc_atan2);
	add_instruction("abs", mathfunc_abs);
	add_instruction("pow", mathfunc_pow);
	add_instruction("sqrt", mathfunc_sqrt);
	add_instruction("floor", mathfunc_floor);
	add_instruction("neg", mathfunc_neg);
	add_instruction("%", mathfunc_intmod);
	add_instruction("fmod", mathfunc_fmod);
	add_instruction("sign", mathfunc_sign);

	add_instruction("vector_add", vectorfunc_add);
	add_instruction("vector_size", vectorfunc_size);
	add_instruction("vector_set", vectorfunc_set);
	add_instruction("vector_insert", vectorfunc_insert);
	add_instruction("vector_get", vectorfunc_get);
	add_instruction("vector_erase", vectorfunc_erase);
	add_instruction("vector_clear", vectorfunc_clear);

	add_instruction("map_set", mapfunc_set);
	add_instruction("map_get", mapfunc_get);
	add_instruction("map_clear", mapfunc_clear);
	add_instruction("map_erase", mapfunc_erase);
	add_instruction("map_keys", mapfunc_keys);

	add_instruction("file_open", filefunc_open);
	add_instruction("file_close", filefunc_close);
	add_instruction("file_read", filefunc_read);
	add_instruction("file_read_line", filefunc_read_line);
	add_instruction("file_write", filefunc_write);
	add_instruction("file_print", filefunc_print);
	
	add_instruction("|", bitfunc_or);
	add_instruction("^", bitfunc_xor);
	add_instruction("&", bitfunc_and);
	add_instruction("<<", bitfunc_leftshift);
	add_instruction(">>", bitfunc_rightshift);
}

} // end namespace booboo
