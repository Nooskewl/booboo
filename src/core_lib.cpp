#include <cmath>
#include <cstring>
#include <iostream>

#include "booboo/booboo.h"
#include "booboo/internal.h"

namespace booboo {

bool breaker_reset(Program *prg, std::vector<Token> &v)
{
	reset_game_name = as_string_inline(prg, v[0]);

	return false;
}

bool breaker_exit(Program *prg, std::vector<Token> &v)
{
	return_code = as_number_inline(prg, v[0]);
	reset_game_name = "";
	quit = true;
	return false;
}

bool breaker_return(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = prg->s->result;

	if (v[0].type == Token::SYMBOL) {
		v1 = prg->variables[v[0].i];
	}
	else if (v[0].type == Token::NUMBER) {
		v1.type = Variable::NUMBER;
		v1.n = v[0].n;
	}
	else if (v[0].type == Token::STRING) {
		v1.type = Variable::STRING;
		v1.s = v[0].s;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return false;
}

bool corefunc_number(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
	return true;
}

bool corefunc_string(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
	return true;
}

bool corefunc_vector(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
	prg->variables[v[0].i].v.clear();
	return true;
}

bool corefunc_map(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
	prg->variables[v[0].i].m.clear();
	return true;
}

bool corefunc_pointer(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
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
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n += as_number_inline(prg, v[1]);
	}
	else if (v1.type == Variable::STRING) {
		v1.s += as_string_inline(prg, v[1]);
	}
	else if (v1.type == Variable::VECTOR) {
		Variable &v2 = as_variable_inline(prg, v[1]);
		if (v2.type != Variable::VECTOR) {
			throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
		}
		else {
			v1.v.insert(v1.v.end(), v2.v.begin(), v2.v.end());
		}
	}
	else if (v1.type == Variable::MAP) {
		Variable &v2 = as_variable_inline(prg, v[1]);
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

	return true;
}

bool corefunc_subtract(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)
	
	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n -= as_number_inline(prg, v[1]);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool corefunc_multiply(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n *= as_number_inline(prg, v[1]);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool corefunc_divide(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n /= as_number_inline(prg, v[1]);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

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
	int function = as_function_inline(prg, v[0]);

	call_void_function(prg, function, v, 1);

	return true;
}

bool corefunc_call_result(Program *prg, std::vector<Token> &v)
{
	Variable &result = as_variable_inline(prg, v[0]);
	int function = as_function_inline(prg, v[1]);

	call_function(prg, function, v, result, 2);

	return true;
}

bool corefunc_int(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)
	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	v1.n = (int)v1.n;

	return true;
}

bool corefunc_neg(Program *prg, std::vector<Token> &v)
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

bool corefunc_intmod(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	int d = as_number(prg, v[1]);
	
	if (v1.type == Variable::NUMBER) {
		v1.n = int(v1.n) % int(d);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool corefunc_fmod(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);
	double d = as_number(prg, v[1]);

	if (v1.type == Variable::NUMBER) {
		v1.n = fmod(v1.n, d);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

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
	if (v.size() < 2) {
		throw Error(std::string(__FUNCTION__) + ": " + "Too few arguments at " + get_error_info(prg));
	}

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

bool corefunc_print(Program *prg, std::vector<Token> &v)
{
	std::string fmt = as_string(prg, v[0]);
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
		while (c < (int)fmt.length()) {
			if (fmt[c] == '%' && prev != '%') {
				break;
			}
			prev = fmt[c];
			c++;
		}

		result += fmt.substr(start, c-start);

		std::string param = v[_tok++].token;

		std::string val;

		if (param[0] == '-' || isdigit(param[0])) {
			val = param;
		}
		else if (param[0] == '"') {
			val = param;
		}
		else {
			Variable &v1 = as_variable_inline(prg, v[_tok-1]);
			if (v1.type == Variable::NUMBER) {
				char buf[1000];
				snprintf(buf, 1000, "%g", v1.n);
				val = buf;
			}
			else if (v1.type == Variable::STRING) {
				val = v1.s;
			}
			else if (v1.type == Variable::VECTOR) {
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
		}

		result += val;

		c++;
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

bool stringfunc_format(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string fmt = as_string(prg, v[1]);
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
		while (c < (int)fmt.length()) {
			if (fmt[c] == '%' && prev != '%') {
				break;
			}
			prev = fmt[c];
			c++;
		}

		result += fmt.substr(start, c-start);

		std::string param = v[_tok++].token;

		std::string val;

		if (param[0] == '-' || isdigit(param[0])) {
			val = param;
		}
		else if (param[0] == '"') {
			val = param;
		}
		else {
			Variable &v1 = as_variable_inline(prg, v[_tok-1]);
			if (v1.type == Variable::NUMBER) {
				char buf[1000];
				snprintf(buf, 1000, "%g", v1.n);
				val = buf;
			}
			else if (v1.type == Variable::STRING) {
				val = v1.s;
			}
			else if (v1.type == Variable::VECTOR) {
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
		}

		result += val;

		c++;
	}

	if (c < (int)fmt.length()) {
		result += fmt.substr(c);
	}

	v1.type = Variable::STRING;
	v1.s = result;

	return true;
}

bool stringfunc_char_at(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string s = as_string(prg, v[1]);
	int index = as_number(prg, v[2]);

	int value = s[index];

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
	Variable &v1 = as_variable_inline(prg, v[0]);
	std::string s = as_string(prg, v[1]);

	if (v1.type == Variable::NUMBER) {
		v1.n = s.length();
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_from_number(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = as_variable_inline(prg, v[0]);
	int n = as_number(prg, v[1]);

	static char buf[2] = { 0, 0 };
	
	buf[0] = n;

	if (v1.type == Variable::STRING) {
		v1.s = buf;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sin(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = sin(as_number(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_cos(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = cos(as_number(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_atan2(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = atan2(as_number(prg, v[1]), as_number(prg, v[2]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_abs(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = fabs(as_number(prg, v[1]));
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
		v1.n = pow(as_number(prg, v[1]), as_number(prg, v[2]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sqrt(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable_inline(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = sqrt(as_number(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
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

	if (v.size() < 3) {
		throw Error(std::string(__FUNCTION__) + ": " + "Too few arguments at " + get_error_info(prg));
	}

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
	double index = as_number(prg, v[1]);

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

	if (v.size() < 3) {
		throw Error(std::string(__FUNCTION__) + ": " + "Too few arguments at " + get_error_info(prg));
	}

	for (size_t i = 2; i < v.size(); i++) {
		int index = as_number(prg, v[i]);
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
	double index = as_number(prg, v[1]);

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
	COUNT_ARGS(3)

	Variable &id = as_variable_inline(prg, v[0]);
	std::string key = as_string(prg, v[1]);

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

	id.m[key] = var;

	return true;
}

static bool mapfunc_get(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &id = as_variable_inline(prg, v[0]);
	std::string key = as_string(prg, v[2]);

	std::map<std::string, Variable>::iterator it = id.m.find(key);

	if (it == id.m.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid map key at " + get_error_info(prg));
	}

	if (prg->variables[v[1].i].type == Variable::POINTER) {
		Variable &v1 = prg->variables[v[1].i];
		v1.p = id.m[key].p;
	}
	else {
		Variable &v1 = as_variable_inline(prg, v[1]);
		std::string bak = v1.name;
		v1 = id.m[key];
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
	std::string key = as_string(prg, v[1]);

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

void start_lib_core()
{
	add_syntax("reset", breaker_reset);
	add_syntax("exit", breaker_exit);
	add_syntax("return", breaker_return);

	add_syntax("number", corefunc_number);
	add_syntax("string", corefunc_string);
	add_syntax("vector", corefunc_vector);
	add_syntax("map", corefunc_map);
	add_syntax("pointer", corefunc_vector);
	
	add_syntax("address", corefunc_address);
	add_syntax("=", corefunc_set);
	add_syntax("+", corefunc_add);
	add_syntax("-", corefunc_subtract);
	add_syntax("*", corefunc_multiply);
	add_syntax("/", corefunc_divide);
	
	add_syntax("goto", corefunc_goto);
	add_syntax("?", corefunc_compare);
	add_syntax("je", corefunc_je);
	add_syntax("jne", corefunc_jne);
	add_syntax("jl", corefunc_jl);
	add_syntax("jle", corefunc_jle);
	add_syntax("jg", corefunc_jg);
	add_syntax("jge", corefunc_jge);
	add_syntax("call", corefunc_call);
	add_syntax("call_result", corefunc_call_result);
	
	add_syntax("int", corefunc_int);
	add_syntax("neg", corefunc_neg);
	add_syntax("%", corefunc_intmod);
	add_syntax("fmod", corefunc_fmod);

	add_syntax("typeof", corefunc_typeof);
	add_syntax("print", corefunc_print);
	add_syntax("input", corefunc_input);

	add_syntax("string_format", stringfunc_format);
	add_syntax("string_char_at", stringfunc_char_at);
	add_syntax("string_length", stringfunc_length);
	add_syntax("string_from_number", stringfunc_from_number);

	add_syntax("sin", mathfunc_sin);
	add_syntax("cos", mathfunc_cos);
	add_syntax("atan2", mathfunc_atan2);
	add_syntax("abs", mathfunc_abs);
	add_syntax("pow", mathfunc_pow);
	add_syntax("sqrt", mathfunc_sqrt);

	add_syntax("vector_add", vectorfunc_add);
	add_syntax("vector_size", vectorfunc_size);
	add_syntax("vector_set", vectorfunc_set);
	add_syntax("vector_insert", vectorfunc_insert);
	add_syntax("vector_get", vectorfunc_get);
	add_syntax("vector_erase", vectorfunc_erase);
	add_syntax("vector_clear", vectorfunc_clear);

	add_syntax("map_set", mapfunc_set);
	add_syntax("map_get", mapfunc_get);
	add_syntax("map_clear", mapfunc_clear);
	add_syntax("map_erase", mapfunc_erase);
	add_syntax("map_keys", mapfunc_keys);
}

} // end namespace booboo
