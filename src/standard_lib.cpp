#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <regex>

#ifdef _WIN32
#include <shlwapi.h>
#else
#include <limits.h>
#endif

#include <sys/stat.h>

#include <libutil/libutil.h>
using namespace noo;

#include <twinkle.h>

#include "booboo/booboo.h"
#include "booboo/internal.h"
using namespace booboo;

template <typename T> T sign(T v) { return (T(0) < v) - (v < T(0)); }

struct File_Info {
	int file_id;
	std::map<int, std::fstream *> files;
};

File_Info *file_info(Program *prg)
{
	File_Info *info = (File_Info *)get_black_box(prg, "com.nooskewl.booboo.files");
	if (info == nullptr) {
		info = new File_Info;
		info->file_id = 0;
		set_black_box(prg, "com.nooskewl.booboo.files", info);
	}
	return info;
}

static Variable exprfunc_getenv(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string get = as_string(prg, v[0]);

	char *ptr = getenv(get.c_str());

	Variable var;
	var.type = Variable::STRING;
	var.s = ptr == nullptr ? "" : ptr;

	return var;
}

static bool corefunc_print(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

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
			Variable *v1;
			if (v[_tok].dereference) {
				v1 = as_variable(prg, v[_tok]).p;
			}
			else {
				v1 = &as_variable(prg, v[_tok]);
			}
			if (IS_NUMBER(*v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1->n);
				val = buf;
			}
			else if (IS_STRING(*v1)) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1->s.c_str());
				val = buf;
			}
			else if (IS_EXPRESSION(*v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1->e).n);
				val = buf;
			}
			else if (IS_FISH(*v1)) {
				Variable &var = go_fish(prg, v1->f);
				if (IS_NUMBER(var)) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					val = buf;
				}
				else {
					if (IS_STRING(var)) {
						val = var.s;
					}
					else if (IS_VECTOR(var)) {
						val = "-vector-";
					}
					else if (IS_MAP(var)) {
						val = "-map-";
					}
					else if (IS_FUNCTION(var)) {
						val = "-function-";
					}
					else if (IS_LABEL(var)) {
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
				if (IS_VECTOR(*v1)) {
					val = "-vector-";
				}
				else if (IS_MAP(*v1)) {
					val = "-map-";
				}
				else if (IS_FUNCTION(*v1)) {
					val = "-function-";
				}
				else if (IS_LABEL(*v1)) {
					val = "-label-";
				}
				else if (IS_POINTER(*v1)) {
					val = "-pointer-";
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

static Variable exprfunc_input(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable var;
	var.type = Variable::STRING;

	if (std::cin.eof()) {
		var.s = "";
	}
	else {
		std::cin >> var.s;
	}

	return var;
}

static bool corefunc_mkdir(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string path = as_string(prg, v[0]);

	util::mkdir(path);

	return true;
}

static Variable exprfunc_get_system_language(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable v1;
	v1.type = Variable::STRING;
	v1.s = util::get_system_language();

	return v1;
}

static Variable exprfunc_get_full_path(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v[0]);

	Variable v1;
	v1.type = Variable::STRING;

#ifdef _WIN32
	char buf[MAX_PATH];
	GetFullPathName(str.c_str(), MAX_PATH, buf, NULL);
	v1.s = buf;
#else
	char buf[PATH_MAX];
	v1.s = realpath(str.c_str(), buf);
	if (v1.s != "/") {
		struct stat s;
		if (stat(v1.s.c_str(), &s) == 0) {
			if (S_ISDIR(s.st_mode)) {
				v1.s += "/";
			}
		}
	}
#endif

	return v1;
}

static Variable exprfunc_list_drives(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	Variable vec;
	vec.type = Variable::VECTOR;

#ifdef _WIN32
	DWORD d = GetLogicalDrives();
	for (int i = 0; i < 32; i++) {
		bool set = d & (1 << i);
		if (set) {
			Variable s;
			s.type = Variable::STRING;
			s.name = "-booboo-";
			char buf[10];
			snprintf(buf, 10, "%c", 'A' + i);
			s.s = buf;
			vec.v.push_back(s);
		}
	}
#endif

	return vec;
}

static Variable exprfunc_list_directory(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable vec;
	vec.type = Variable::VECTOR;

	std::string glob = as_string(prg, v[0]);

	std::string path_part;
	int p = glob.length() - 1;
	if (p >= 0) {
		while (p >= 0 && glob[p] != '/' && glob[p] != '\\') {
			p--;
		}
		if (p > 0) {
			path_part = glob.substr(0, p+1);
		}
	}

	util::List_Directory l(glob);

	std::string fn;

	while ((fn = l.next()) != "") {
		if (fn == "." || fn == "..") {
			continue;
		}
#ifdef _WIN32
		if (PathIsDirectory((path_part + fn).c_str())) {
			fn += "/";
		}
#else
		struct stat s;
		if (stat(fn.c_str(), &s) == 0) {
			if (S_ISDIR(s.st_mode)) {
				fn += "/";
			}
		}
#endif
		Variable v;
		v.type = Variable::STRING;
		v.name = "-booboo-";
		v.s = fn;
		vec.v.push_back(v);
	}

	return vec;
}

static Variable exprfunc_string_format(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;

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
			Variable &v1 = as_variable(prg, v[_tok]);
			if (IS_NUMBER(v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.n);
				val = buf;
			}
			else if (IS_STRING(v1)) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.s.c_str());
				val = buf;
			}
			else if (IS_EXPRESSION(v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e).n);
				val = buf;
			}
			else if (IS_FISH(v1)) {
				Variable &var = go_fish(prg, v1.f);
				if (IS_NUMBER(var)) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					val = buf;
				}
				else {
					if (IS_STRING(var)) {
						val = var.s;
					}
					else if (IS_VECTOR(var)) {
						val = "-vector-";
					}
					else if (IS_MAP(var)) {
						val = "-map-";
					}
					else if (IS_FUNCTION(var)) {
						val = "-function-";
					}
					else if (IS_LABEL(var)) {
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
				if (IS_VECTOR(v1)) {
					val = "-vector-";
				}
				else if (IS_MAP(v1)) {
					val = "-map-";
				}
				else if (IS_FUNCTION(v1)) {
					val = "-function-";
				}
				else if (IS_LABEL(v1)) {
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

	return v1;
}

static Variable exprfunc_string_char_at(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;
	
	std::string s = as_string(prg, v[0]);
	int index = as_number(prg, v[1]);

	uint32_t value = util::utf8_char(s, index);

	v1.n = value;

	return v1;
}

static Variable exprfunc_string_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	std::string s = as_string(prg, v[0]);

	v1.n = util::utf8_len(s);

	return v1;
}

static Variable exprfunc_string_from_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
       	v1.type = Variable::STRING;

	uint32_t n = as_number(prg, v[0]);

	v1.s = util::utf8_char_to_string(n);

	return v1; 
}

static Variable exprfunc_string_substr(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);

	int start = as_number(prg, v[1]);
	int count = -1;

	if (v.size() >= 3) {
		count = as_number(prg, v[2]);
	}

	v1.s = util::utf8_substr(str, start, count);

	return v1;
}

static Variable exprfunc_string_uppercase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);

	v1.s = util::uppercase(str);

	return v1;
}

static Variable exprfunc_string_lowercase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);

	v1.s = util::lowercase(str);

	return v1;
}

static Variable exprfunc_string_trim(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);

	v1.s = util::trim(str);

	return v1;
}

static Variable exprfunc_string_replace(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);
	std::string regex = as_string(prg, v[1]);
	std::string fmt = as_string(prg, v[2]);

	v1.s = std::regex_replace(str, std::regex(regex), fmt.c_str());

	return v1;
}

static Variable exprfunc_string_match(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
	v1.type = Variable::VECTOR;

	std::string str = as_string(prg, v[0]);
	std::string regex = as_string(prg, v[1]);

	std::smatch match;

	if (std::regex_match(str, match, std::regex(regex))) {
		for (size_t i = 1; i < match.size(); i++) {
			std::ssub_match sub = match[i];
			Variable v;
			v.type = Variable::STRING;
			v.name = "-booboo-";
			v.s = sub.str();
			v1.v.push_back(v);
		}
	}

	return v1;
}

static Variable exprfunc_string_matches(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
       	v1.type = Variable::NUMBER;
	std::string str = as_string(prg, v[0]);
	std::string regex = as_string(prg, v[1]);

	v1.n = std::regex_search(str, std::regex(regex));

	return v1;
}

static Variable exprfunc_math_sin(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = sin(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_cos(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = cos(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_tan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = tan(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_asin(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = asin(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_acos(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = acos(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_atan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = atan(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_atan2(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = atan2(as_number(prg, v[0]), as_number(prg, v[1]));

	return v1;
}

static Variable exprfunc_math_abs(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;
		
	v1.n = fabs(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_pow(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = pow(as_number(prg, v[0]), as_number(prg, v[1]));

	return v1;
}

static Variable exprfunc_math_sqrt(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = sqrt(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_floor(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = floor(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_ceil(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = ceil(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_neg(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = -as_number(prg, v[0]);

	return v1;
}

static Variable exprfunc_math_intmod(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
       	v1.type = Variable::NUMBER;

	int n1 = (int)as_number(prg, v[0]);
	int n2 = (int)as_number(prg, v[1]);

	v1.n = n1 % n2;

	return v1;
}

static Variable exprfunc_math_fmod(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
       	v1.type = Variable::NUMBER;

	double n1 = as_number(prg, v[0]);
	double n2 = as_number(prg, v[1]);

	v1.n = fmod(n1, n2);

	return v1;
}

static Variable exprfunc_math_sign(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;
		
	v1.n = sign(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_exp(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;
		
	v1.n = exp(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_hypot(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;
		
	v1.n = hypot(as_number(prg, v[0]), as_number(prg, v[1]));

	return v1;
}

static Variable exprfunc_math_log(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;
		
	v1.n = log(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_log10(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;
		
	v1.n = log10(as_number(prg, v[0]));

	return v1;
}

static Variable exprfunc_math_min(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		v1.n = MIN(v1.n, as_number(prg, v[i]));
	}

	return v1;
}

static Variable exprfunc_math_max(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = as_number(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		v1.n = MAX(v1.n, as_number(prg, v[i]));
	}

	return v1;
}

static bool vectorfunc_init(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable &vec = as_variable(prg, v[0]);
	vec.type = Variable::VECTOR;
	vec.v.clear();

	for (size_t i = 1; i < v.size(); i++) {
		if (v[i].type == Token::NUMBER) {
			Variable var;
			var.type = Variable::NUMBER;
			var.n = as_number(prg, v[i]);
			vec.v.push_back(var);
		}
		else if (v[i].type == Token::STRING) {
			Variable var;
			var.type = Variable::STRING;
			var.s = as_string(prg, v[i]);
			vec.v.push_back(var);
		}
		else {
			Variable var = as_variable_resolve(prg, v[i]);
			var.constant = false;
			vec.v.push_back(var);
		}
	}

	return true;
}

static bool vectorfunc_add(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	id.type = Variable::VECTOR;

	for (size_t i  = 1; i < v.size(); i++) {
		Variable var = as_variable_resolve(prg, v[i]);

		var.constant = false;

		id.v.push_back(var);
	}

	return true;
}

static Variable exprfunc_vector_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable id = as_variable_resolve(prg, v[0]);

	CHECK_VECTOR(id)
	
	Variable var;
	var.type = Variable::NUMBER;
	var.n = id.v.size();

	return var;
}

static bool vectorfunc_insert(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &id = as_variable(prg, v[0]);
	double index = as_number(prg, v[1]);

	id.type = Variable::VECTOR;

	if (index < 0 || index > id.v.size()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
	}

	Variable var;

	if (v[2].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-booboo-";
		var.n = v[2].n;
	}
	else if (v[2].type == Token::SYMBOL) {
		var = as_variable(prg, v[2]);
		var.constant = false;
	}
	else {
		var.type = Variable::STRING;
		var.name = "-booboo-";
		var.s = v[2].s;
	}

	id.v.insert(id.v.begin()+index, var);

	return true;
}

static bool vectorfunc_erase(Program *prg, const std::vector<Token> &v)
{

	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	double index = as_number(prg, v[1]);

	CHECK_VECTOR(id)

	if (index < 0 || index >= id.v.size()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
	}

	id.v.erase(id.v.begin() + int(index));

	return true;
}

static bool vectorfunc_clear(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &id = as_variable(prg, v[0]);

	id.type = Variable::VECTOR;

	id.v.clear();

	return true;
}

static bool vectorfunc_reserve(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	int n = as_number(prg, v[1]);

	id.type = Variable::VECTOR;

	id.v.reserve(n);

	return true;
}

static bool mapfunc_clear(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &id = as_variable(prg, v[0]);

	id.type = Variable::MAP;

	id.m.clear();

	return true;
}

static bool mapfunc_erase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	std::string key = as_string(prg, v[1]);

	CHECK_MAP(id)

	std::map<std::string, Variable>::iterator it = id.m.find(key);

	if (it == id.m.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid map key at " + get_error_info(prg));
	}

	id.m.erase(it);

	return true;
}

static Variable exprfunc_map_keys(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable m = as_variable_resolve(prg, v[0]);

	CHECK_MAP(m)

	std::map<std::string, Variable>::iterator it;

	Variable v1;
	v1.type = Variable::VECTOR;

	for (it = m.m.begin(); it != m.m.end(); it++) {
		std::pair<std::string, Variable> p = *it;
		Variable var;
		var.type = Variable::STRING;
		var.name = "-booboo-";
		var.s = p.first;
		v1.v.push_back(var);
	}

	return v1;
}

static Variable exprfunc_file_open(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	std::string filename = as_string(prg, v[0]);
	std::string mode = as_string(prg, v[1]);
	
	File_Info *info = file_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = info->file_id;

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
		return v1;
	}

	info->files[info->file_id++] = f;

	return v1;
}

static bool filefunc_close(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	
	File_Info *info = file_info(prg);

	info->files[id]->close();

	return true;
}

static Variable exprfunc_file_read(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable var;
	var.type = Variable::STRING;

	File_Info *info = file_info(prg);

	(*info->files[id]) >> var.s;

	return var;
}

static Variable exprfunc_file_read_line(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable var;
	var.type = Variable::STRING;

	File_Info *info = file_info(prg);

	if ((*info->files[id]).eof()) {
		var.s = "";
	}
	else {
		std::getline(*info->files[id], var.s);
	}

	return var;
}

static bool filefunc_write(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string val = as_string(prg, v[1]);

	File_Info *info = file_info(prg);

	(*info->files[id]) << val;

	return true;
}

static bool filefunc_print(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number(prg, v[0]);
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
			Variable &v1 = as_variable(prg, v[_tok]);
			if (IS_NUMBER(v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.n);
				val = buf;
			}
			else if (IS_STRING(v1)) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1.s.c_str());
				val = buf;
			}
			else if (IS_EXPRESSION(v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e).n);
				val = buf;
			}
			else if (IS_FISH(v1)) {
				Variable &var = go_fish(prg, v1.f);
				if (IS_NUMBER(var)) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					val = buf;
				}
				else {
					if (IS_STRING(var)) {
						val = var.s;
					}
					else if (IS_VECTOR(var)) {
						val = "-vector-";
					}
					else if (IS_MAP(var)) {
						val = "-map-";
					}
					else if (IS_FUNCTION(var)) {
						val = "-function-";
					}
					else if (IS_LABEL(var)) {
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
				if (IS_VECTOR(v1)) {
					val = "-vector-";
				}
				else if (IS_MAP(v1)) {
					val = "-map-";
				}
				else if (IS_FUNCTION(v1)) {
					val = "-function-";
				}
				else if (IS_LABEL(v1)) {
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

static bool twinklefunc_colour(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(4)

	int fore = as_number(prg, v[0]);
	int fore_b = as_number(prg, v[1]);
	int back = as_number(prg, v[2]);
	int back_b = as_number(prg, v[3]);

	twinkle::set((twinkle::TWINKLE_COLOR)fore, fore_b, (twinkle::TWINKLE_COLOR)back, back_b);

	return true;
}

static bool twinklefunc_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	twinkle::reset();

	return true;
}

static Variable exprfunc_twinkle_getch(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)
	
	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = twinkle::getch();

	return v1;
}

static bool twinklefunc_clear(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	twinkle::clear();

	return true;
}

void start_lib_standard()
{
	add_expression_handler("getenv", exprfunc_getenv);
	add_expression_handler("list_directory", exprfunc_list_directory);
	add_instruction("print", corefunc_print);
	add_expression_handler("input", exprfunc_input);
	add_instruction("mkdir", corefunc_mkdir);
	add_expression_handler("get_system_language", exprfunc_get_system_language);
	add_expression_handler("get_full_path", exprfunc_get_full_path);
	add_expression_handler("list_drives", exprfunc_list_drives);

	add_expression_handler("string_format", exprfunc_string_format);
	add_expression_handler("string_char_at", exprfunc_string_char_at);
	add_expression_handler("string_length", exprfunc_string_length);
	add_expression_handler("string_from_number", exprfunc_string_from_number);
	add_expression_handler("string_substr", exprfunc_string_substr);
	add_expression_handler("string_uppercase", exprfunc_string_uppercase);
	add_expression_handler("string_lowercase", exprfunc_string_lowercase);
	add_expression_handler("string_trim", exprfunc_string_trim);
	add_expression_handler("string_replace", exprfunc_string_replace);
	add_expression_handler("string_match", exprfunc_string_match);
	add_expression_handler("string_matches", exprfunc_string_matches);

	add_expression_handler("sin", exprfunc_math_sin);
	add_expression_handler("cos", exprfunc_math_cos);
	add_expression_handler("tan", exprfunc_math_tan);
	add_expression_handler("asin", exprfunc_math_asin);
	add_expression_handler("acos", exprfunc_math_acos);
	add_expression_handler("atan", exprfunc_math_atan);
	add_expression_handler("atan2", exprfunc_math_atan2);
	add_expression_handler("abs", exprfunc_math_abs);
	add_expression_handler("pow", exprfunc_math_pow);
	add_expression_handler("sqrt", exprfunc_math_sqrt);
	add_expression_handler("floor", exprfunc_math_floor);
	add_expression_handler("ceil", exprfunc_math_ceil);
	add_expression_handler("neg", exprfunc_math_neg);
	add_expression_handler("%", exprfunc_math_intmod);
	add_expression_handler("fmod", exprfunc_math_fmod);
	add_expression_handler("sign", exprfunc_math_sign);
	add_expression_handler("exp", exprfunc_math_exp);
	add_expression_handler("hypot", exprfunc_math_hypot);
	add_expression_handler("log", exprfunc_math_log);
	add_expression_handler("log10", exprfunc_math_log10);
	add_expression_handler("min", exprfunc_math_min);
	add_expression_handler("max", exprfunc_math_max);

	add_instruction("vector_init", vectorfunc_init);
	add_instruction("vector_add", vectorfunc_add);
	add_expression_handler("vector_size", exprfunc_vector_size);
	add_instruction("vector_insert", vectorfunc_insert);
	add_instruction("vector_erase", vectorfunc_erase);
	add_instruction("vector_clear", vectorfunc_clear);
	add_instruction("vector_reserve", vectorfunc_reserve);

	add_instruction("map_clear", mapfunc_clear);
	add_instruction("map_erase", mapfunc_erase);
	add_expression_handler("map_keys", exprfunc_map_keys);

	add_expression_handler("file_open", exprfunc_file_open);
	add_instruction("file_close", filefunc_close);
	add_expression_handler("file_read", exprfunc_file_read);
	add_expression_handler("file_read_line", exprfunc_file_read_line);
	add_instruction("file_write", filefunc_write);
	add_instruction("file_print", filefunc_print);
	
	add_instruction("text_colour", twinklefunc_colour);
	add_instruction("text_reset", twinklefunc_reset);
	add_expression_handler("getch", exprfunc_twinkle_getch);
	add_instruction("text_clear", twinklefunc_clear);
}

void end_lib_standard()
{
}

void standard_lib_destroy_program(Program *prg)
{
	File_Info *file_i = file_info(prg);
	for (size_t i = 0; i < file_i->files.size(); i++) {
		file_i->files[i]->close();
		delete file_i->files[i];
	}
	delete file_i;

	set_black_box(prg, "com.nooskewl.booboo.files", nullptr);
}
