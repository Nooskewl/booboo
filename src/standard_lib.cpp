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

#include "booboo/booboo.h"
using namespace booboo;

template <typename T> T sign(T v) { return (T(0) < v) - (v < T(0)); }

struct File_Info {
	int file_id;
	std::map<int, std::fstream *> files;
};

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

bool corefunc_getenv(Program *prg, std::vector<Token> &v)
{
	Variable &v1 = as_variable(prg, v[0]);
	std::string get = as_string(prg, v[1]);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.s = getenv(get.c_str());

	return true;
}

bool corefunc_print(Program *prg, std::vector<Token> &v)
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
			Variable &v1 = as_variable(prg, v[_tok]);
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
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e).n);
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

	Variable &var = as_variable(prg, v[0]);

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

	std::string path = as_string(prg, v[0]);

	util::mkdir(path);

	return true;
}

bool corefunc_get_system_language(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	v1.s = util::get_system_language();

	return true;
}

bool corefunc_get_full_path(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
	std::string path = as_string(prg, v[1]);

	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

#ifdef _WIN32
	char buf[MAX_PATH];
	GetFullPathName(path.c_str(), MAX_PATH, buf, NULL);
	v1.s = buf;
#else
	char buf[PATH_MAX];
	v1.s = realpath(path.c_str(), buf);
	if (v1.s != "/") {
		struct stat s;
		if (stat(v1.s.c_str(), &s) == 0) {
			if (S_ISDIR(s.st_mode)) {
				v1.s += "/";
			}
		}
	}
#endif

	return true;
}

bool corefunc_list_drives(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &vec = as_variable(prg, v[0]);
	
	if (vec.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	vec.v.clear();

#ifdef _WIN32
	DWORD d = GetLogicalDrives();
	for (int i = 0; i < 32; i++) {
		bool set = d & (1 << i);
		if (set) {
			Variable s;
			s.type = Variable::STRING;
			s.name = "-constant-";
			char buf[10];
			snprintf(buf, 10, "%c", 'A' + i);
			s.s = buf;
			vec.v.push_back(s);
		}
	}
#endif

	return true;
}

bool corefunc_list_directory(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &vec = as_variable(prg, v[0]);

	if (vec.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	std::string glob = as_string(prg, v[1]);

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

	vec.v.clear();

	while ((fn = l.next()) != "") {
		if (fn == "." || fn == "..") {
			continue;
		}
#ifdef _WIN32
		//char buf[1000];
		//GetFullPathName(fn.c_str(), 1000, buf, NULL);
		//if (PathIsDirectory(buf)) {
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
		v.name = "-constant-";
		v.s = fn;
		vec.v.push_back(v);
	}

	return true;
}

bool stringfunc_format(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &v1 = as_variable(prg, v[0]);
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
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e).n);
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

bool stringfunc_char_at(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable(prg, v[0]);
	std::string s = as_string(prg, v[1]);
	int index = as_number(prg, v[2]);

	uint32_t value = util::utf8_char(s, index);

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

	Variable &v1 = as_variable(prg, v[0]);
	std::string s = as_string(prg, v[1]);

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

	Variable &v1 = as_variable(prg, v[0]);
	uint32_t n = as_number(prg, v[1]);

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

	Variable &v1 = as_variable(prg, v[0]);
	int start = as_number(prg, v[1]);
	int count = -1;
	
	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	if (v.size() >= 3) {
		count = as_number(prg, v[2]);
	}

	v1.s = util::utf8_substr(v1.s, start, count);

	return true;
}

bool stringfunc_uppercase(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

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

	Variable &v1 = as_variable(prg, v[0]);

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

	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type == Variable::STRING) {
		v1.s = util::trim(v1.s);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

bool stringfunc_replace(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(4)

	Variable &v1 = as_variable(prg, v[0]);
	
	if (v1.type != Variable::STRING) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	std::string str = as_string(prg, v[1]);
	std::string regex = as_string(prg, v[2]);
	std::string fmt = as_string(prg, v[3]);

	v1.s = std::regex_replace(str.c_str(), std::regex(regex), fmt.c_str());

	return true;
}

bool stringfunc_match(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable(prg, v[0]);
	
	if (v1.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	std::string str = as_string(prg, v[1]);
	std::string regex = as_string(prg, v[2]);

	v1.v.clear();

	std::smatch match;

	if (std::regex_match(str, match, std::regex(regex))) {
		for (size_t i = 1; i < match.size(); i++) {
			std::ssub_match sub = match[i];
			Variable v;
			v.type = Variable::STRING;
			v.name = "-constant-";
			v.s = sub.str();
			v1.v.push_back(v);
		}
	}

	return true;
}

bool stringfunc_matches(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &v1 = as_variable(prg, v[0]);
	std::string str = as_string(prg, v[1]);
	std::string regex = as_string(prg, v[2]);

	if (v1.type == Variable::NUMBER) {
		v1.n = std::regex_search(str, std::regex(regex));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sin(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

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

	Variable &v1 = as_variable(prg, v[0]);

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

	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = atan2(v1.n, as_number(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_abs(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

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

	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = pow(v1.n, as_number(prg, v[1]));
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	return true;
}

bool mathfunc_sqrt(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

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
	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type != Variable::NUMBER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Operation undefined for operands at " + get_error_info(prg));
	}

	v1.n = (int)v1.n;

	return true;
}

bool mathfunc_neg(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &v1 = as_variable(prg, v[0]);

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

	Variable &v1 = as_variable(prg, v[0]);
	int d = as_number(prg, v[1]);
	
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

	Variable &v1 = as_variable(prg, v[0]);
	double d = as_number(prg, v[1]);

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

	Variable &v1 = as_variable(prg, v[0]);

	if (v1.type == Variable::NUMBER) {
		v1.n = sign(v1.n);
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}

	return true;
}

static bool vectorfunc_add(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);

	Variable var;

	if (v[1].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[1].n;
	}
	else if (v[1].type == Token::SYMBOL) {
		Variable &tmp = get_variable(prg, v[1].i);
		if (tmp.type == Variable::POINTER) {
			var = tmp;
		}
		else {
			var = as_variable(prg, v[1]);
			if (var.type == Variable::FISH) {
				var = go_fish(prg, var.f);
			}
			else if (var.type == Variable::EXPRESSION) {
				var = evaluate_expression(prg, var.e);
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

	Variable &id = as_variable(prg, v[0]);
	Variable &v1 = as_variable(prg, v[1]);

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
	Variable &id = as_variable(prg, v[0]);
	int val_index = v.size() - 1;
	std::vector<int> indices;

	MIN_ARGS(3)

	for (int i = 1; i < val_index; i++) {
		indices.push_back(as_number(prg, v[i]));
	}

	Variable var;

	if (v[val_index].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[val_index].n;
	}
	else if (v[val_index].type == Token::SYMBOL) {
		Variable &tmp = get_variable(prg, v[val_index].i);
		if (tmp.type == Variable::POINTER) {
			var = tmp;
		}
		else {
			var = as_variable(prg, v[val_index]);
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

	Variable &id = as_variable(prg, v[0]);
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
		Variable &tmp = get_variable(prg, v[2].i);
		if (tmp.type == Variable::POINTER) {
			var = tmp;
		}
		else {
			var = as_variable(prg, v[2]);
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
	Variable &id = as_variable(prg, v[0]);
	std::vector<int> indices;

	MIN_ARGS(3)

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
			Variable &tmp = get_variable(prg, v[1].i);
			if (tmp.type == Variable::POINTER) {
				Variable &v1 = tmp;
				v1.p = (*p)[index].p;
			}
			else {
				Variable &v1 = as_variable(prg, v[1]);
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

	Variable &id = as_variable(prg, v[0]);
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

	Variable &id = as_variable(prg, v[0]);

	id.v.clear();

	return true;
}

static bool mapfunc_set(Program *prg, std::vector<Token> &v)
{
	MIN_ARGS(3)

	Variable &id = as_variable(prg, v[0]);

	Variable var;
	int val_index = v.size()-1;

	if (v[val_index].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.name = "-constant-";
		var.n = v[val_index].n;
	}
	else if (v[val_index].type == Token::SYMBOL) {
		Variable &tmp = get_variable(prg, v[val_index].i);
		if (tmp.type == Variable::POINTER) {
			var = tmp;
		}
		else {
			var = as_variable(prg, v[val_index]);
			if (var.type == Variable::FISH) {
				var = go_fish(prg, var.f);
			}
			else if (var.type == Variable::EXPRESSION) {
				var = evaluate_expression(prg, var.e);
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
		key = as_string(prg, v[i]);
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

	Variable &id = as_variable(prg, v[0]);

	std::map<std::string, Variable> *p = &id.m;
	std::string key;

	for (size_t i = 2; i < v.size(); i++) {
		key = as_string(prg, v[i]);
		if (i < v.size()-1) {
			p = &(*p)[key].m;
		}
	}

	Variable &tmp = get_variable(prg, v[1].i);
	if (tmp.type == Variable::POINTER) {
		Variable &v1 = tmp;
		v1.p = (*p)[key].p;
	}
	else {
		Variable &v1 = as_variable(prg, v[1]);
		std::string bak = v1.name;
		v1 = (*p)[key];
		v1.name = bak;
	}

	return true;
}

static bool mapfunc_clear(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable &id = as_variable(prg, v[0]);

	id.m.clear();

	return true;
}

static bool mapfunc_erase(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
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

	Variable &m = as_variable(prg, v[0]);
	Variable &vec_var = as_variable(prg, v[1]);

	if (m.type != Variable::MAP) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected a map at " + get_error_info(prg));
	}

	if (vec_var.type != Variable::VECTOR) {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected a vector at " + get_error_info(prg));
	}

	vec_var.v.clear();

	std::map<std::string, Variable>::iterator it;

	for (it = m.m.begin(); it != m.m.end(); it++) {
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

	Variable &v1 = as_variable(prg, v[0]);
	std::string filename = as_string(prg, v[1]);
	std::string mode = as_string(prg, v[2]);
	
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

	int id = as_number(prg, v[0]);
	
	File_Info *info = file_info(prg);

	info->files[id]->close();

	return true;
}

static bool filefunc_read(Program *prg, std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	Variable &var = as_variable(prg, v[1]);

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

	int id = as_number(prg, v[0]);
	Variable &var = as_variable(prg, v[1]);

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

	int id = as_number(prg, v[0]);
	std::string val = as_string(prg, v[1]);

	File_Info *info = file_info(prg);

	(*info->files[id]) << val;

	return true;
}

bool filefunc_print(Program *prg, std::vector<Token> &v)
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
				snprintf(buf, 1000, ("%" + format).c_str(), evaluate_expression(prg, v1.e).n);
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

	Variable &v1 = as_variable(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n | (int)as_number(prg, v[i]);
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

	Variable &v1 = as_variable(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n ^ (int)as_number(prg, v[i]);
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

	Variable &v1 = as_variable(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n & (int)as_number(prg, v[i]);
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

	Variable &v1 = as_variable(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n << (int)as_number(prg, v[i]);
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

	Variable &v1 = as_variable(prg, v[0]);

	for (size_t i = 1; i < v.size(); i++) {
		if (v1.type == Variable::NUMBER) {
			v1.n = (int)v1.n >> (int)as_number(prg, v[i]);
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}

	return true;
}

void start_lib_standard()
{
	add_instruction("getenv", corefunc_getenv);
	add_instruction("list_directory", corefunc_list_directory);
	add_instruction("print", corefunc_print);
	add_instruction("input", corefunc_input);
	add_instruction("mkdir", corefunc_mkdir);
	add_instruction("get_system_language", corefunc_get_system_language);
	add_instruction("get_full_path", corefunc_get_full_path);
	add_instruction("list_drives", corefunc_list_drives);

	add_instruction("string_format", stringfunc_format);
	add_instruction("string_char_at", stringfunc_char_at);
	add_instruction("string_length", stringfunc_length);
	add_instruction("string_from_number", stringfunc_from_number);
	add_instruction("string_substr", stringfunc_substr);
	add_instruction("string_uppercase", stringfunc_uppercase);
	add_instruction("string_lowercase", stringfunc_lowercase);
	add_instruction("string_trim", stringfunc_trim);
	add_instruction("string_replace", stringfunc_replace);
	add_instruction("string_match", stringfunc_match);
	add_instruction("string_matches", stringfunc_matches);

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

	set_black_box(prg, "com.b1stable.booboo.files", nullptr);
}
