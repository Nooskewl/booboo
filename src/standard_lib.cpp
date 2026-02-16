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

#include <shim5/shim5.h>
using namespace noo;

#include <twinkle.h>

#include "booboo/booboo.h"
#include "booboo/standard_lib.h"
#include "booboo/internal.h"
using namespace booboo;

#define INFO_EXISTS(m, i) if (m.find(i) == m.end()) { \
	throw Error(std::string(__FUNCTION__) + ": " + "Invalid handle at " + get_error_info(prg)); \
}

template <typename T> T sign(T v) { return (T(0) < v) - (v < T(0)); }

struct File_Info {
	int file_id;
	std::map<int, SDL_IOStream *> files;
};

struct Config_Value
{
	Variable::Variable_Type type;

	double n;
	std::string s;
};

struct CFG_Info {
	unsigned int cfg_id;
	std::map<int, std::map<std::string, Config_Value> > cfgs;
};

struct JSON_Info {
	unsigned int json_id;
	std::map<int, util::JSON *> jsons;
};

struct CPA {
	util::CPA *cpa;
};

struct CPA_Info
{
	int cpa_id;
	std::map<int, CPA *> cpas;
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

static CFG_Info *cfg_info(Program *prg)
{
	CFG_Info *info = (CFG_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.cfg");
	if (info == nullptr) {
		info = new CFG_Info;
		info->cfg_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.cfg", info);
	}
	return info;
}

static JSON_Info *json_info(Program *prg)
{
	JSON_Info *info = (JSON_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.json");
	if (info == nullptr) {
		info = new JSON_Info;
		info->json_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.json", info);
	}
	return info;
}

static CPA_Info *cpa_info(Program *prg)
{
	CPA_Info *info = (CPA_Info *)booboo::get_black_box(prg, "com.nooskewl.booboo.cpa");
	if (info == nullptr) {
		info = new CPA_Info;
		info->cpa_id = 0;
		booboo::set_black_box(prg, "com.nooskewl.booboo.cpa", info);
	}
	return info;
}

static std::string cfg_path(std::string cfg_name)
{
	std::string path = util::get_savegames_dir() + "/" + cfg_name + ".txt";
	return path;
}

static std::map<std::string, Config_Value> load_cfg(Program *prg, std::string cfg_name)
{
	std::map<std::string, Config_Value> v;

	std::string text;

	try {
		text = util::load_text_from_filesystem(cfg_path(cfg_name));
	}
	catch (util::Error &e) {
		return v;
	}

	util::Tokenizer t(text, '\n');

	std::string line;

	while ((line = t.next()) != "") {
		util::Tokenizer t2(line, '=');
		std::string name = t2.next();
		std::string value = t2.remaining();
		util::trim(value);

		if (name == "") {
			continue;
		}

		Config_Value val;
		
		if (value.length() > 0 && value[0] == '"') {
			val.type = Variable::STRING;
			val.s = util::remove_quotes(value);
		}
		else {
			val.type = Variable::NUMBER;
			val.n = atof(value.c_str());
		}

		v[name] = val;
	}

	return v;
}

static bool save_cfg(Program *prg, int id, std::string cfg_name)
{
	FILE *f = fopen(cfg_path(cfg_name).c_str(), "w");
	if (f == nullptr) {
		return false;
	}

	std::map<std::string, Config_Value>::iterator it;

	CFG_Info *info = cfg_info(prg);

	for (it = info->cfgs[id].begin(); it != info->cfgs[id].end(); it++) {
		std::string name = (*it).first;
		Config_Value &v = (*it).second;
		if (IS_NUMBER(v)) {
			fprintf(f, "%s=%g\n", name.c_str(), v.n);
		}
		else {
			fprintf(f, "%s=\"%s\"\n", name.c_str(), v.s.c_str());
		}
	}

	fclose(f);

	return true;
}

static std::string sformat(Program *prg, const std::vector<Token> &v, int skip)
{
	std::string fmt = as_string(prg, v[skip]);
	int _tok = skip+1;
	
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
		prev = fmt[c > 0 ? c-1 : c];

		std::string val;

		if (v[_tok].type == Token::NUMBER) {
			format = (format == "") ? "g" : format;
			char buf[1000];
			if (format.find('c') != std::string::npos || format.find('d') != std::string::npos || format.find('x') != std::string::npos) {
				snprintf(buf, 1000, ("%" + format).c_str(), (int)v[_tok].n);
			}
			else {
				snprintf(buf, 1000, ("%" + format).c_str(), v[_tok].n);
			}
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
			if (v[_tok].dereference > 0) {
				v1 = dereference(prg, v[_tok]);
			}
			else {
				v1 = &as_variable(prg, v[_tok]);
			}
			if (IS_NUMBER(*v1)) {
				format = (format == "") ? "g" : format;
				char buf[1000];
				if (format.find('c') != std::string::npos || format.find('d') != std::string::npos || format.find('x') != std::string::npos) {
					snprintf(buf, 1000, ("%" + format).c_str(), (int)v1->n);
				}
				else {
					snprintf(buf, 1000, ("%" + format).c_str(), v1->n);
				}
				val = buf;
			}
			else if (IS_STRING(*v1)) {
				format = (format == "") ? "s" : format;
				char buf[1000];
				snprintf(buf, 1000, ("%" + format).c_str(), v1->s.c_str());
				val = buf;
			}
			else if (IS_EXPRESSION(*v1)) {
				Variable var = evaluate_expression(prg, v1->e);
				if (IS_NUMBER(var)) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					if (format.find('c') != std::string::npos || format.find('d') != std::string::npos || format.find('x') != std::string::npos) {
						snprintf(buf, 1000, ("%" + format).c_str(), (int)var.n);
					}
					else {
						snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					}
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
			else if (IS_FISH(*v1)) {
				Variable &var = go_fish(prg, v1->f);
				if (IS_NUMBER(var)) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					if (format.find('c') != std::string::npos || format.find('d') != std::string::npos || format.find('x') != std::string::npos) {
						snprintf(buf, 1000, ("%" + format).c_str(), (int)var.n);
					}
					else {
						snprintf(buf, 1000, ("%" + format).c_str(), var.n);
					}
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

	return result;
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

	std::string result = sformat(prg, v, 0);

	printf(result.c_str());

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
			path_part = glob.substr(0, p);
		}
	}

	util::List_Directory l(glob);

	std::string fn;

	Variable filenames;
	filenames.type = Variable::VECTOR;
	Variable is_dir;
	is_dir.type = Variable::VECTOR;

	while ((fn = l.next()) != "") {
		if (fn == "." || fn == "..") {
			continue;
		}
		bool _is_dir = false;
#ifdef _WIN32
		if (PathIsDirectory((path_part + "/" + fn).c_str())) {
			_is_dir = true;
		}
#else
		struct stat s;
		if (stat(fn.c_str(), &s) == 0) {
			if (S_ISDIR(s.st_mode)) {
				_is_dir = true;
			}
		}
#endif
		Variable v;
		v.type = Variable::STRING;
		v.s = fn;
		filenames.v.push_back(v);
		v.type = Variable::NUMBER;
		v.n = _is_dir;
		is_dir.v.push_back(v);
	}

	vec.v.push_back(filenames);
	vec.v.push_back(is_dir);

	return vec;
}

static void quicksort(Program *prg, int func, Variable *vec, int start, int pivot)
{
	if (start >= pivot) {
		return;
	}
	int save = pivot;
	for (int i = start; i < pivot; i++) {
		int i1 = prg->variables_map["__tmp0"];
		int i2 = prg->variables_map["__tmp1"];
		Variable &v1 = get_variable(prg, i1);
		Variable &v2 = get_variable(prg, i2);
		v1 = vec->v[i];
		v2 = vec->v[pivot];
		std::vector<Token> params;
		Token t;
		t.type = Token::SYMBOL;
		t.i = i1;
		params.push_back(t);
		t.i = i2;
		params.push_back(t);
		Variable result;
		call_function(prg, func, params, result, 0);
		if (result.n == false) {
			Variable v = vec->v[i];
			vec->v.erase(vec->v.begin()+i);
			vec->v.insert(vec->v.begin()+pivot, v);
			pivot--;
			i--;
		}
	}
	quicksort(prg, func, vec, start, pivot-1);
	quicksort(prg, func, vec, pivot+1, save);
}

static bool corefunc_sort(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable *vec;
       	if (v[0].type == Token::SYMBOL) {
		if (prg->variables[v[0].i].type == Variable::EXPRESSION) {
			static Variable var = evaluate_expression(prg, prg->variables[v[0].i].e);
			vec = &var;
		}
		else {
			vec = &as_variable(prg, v[0]);
		}
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected symbol at " + get_error_info(prg));
	}
	int func = as_function(prg, v[1]);

	if (vec->v.size() <= 1) {
		return true;
	}

	quicksort(prg, func, vec, 0, vec->v.size()-1);

	return true;
}

static bool corefunc_unique(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable *vec;
       	if (v[0].type == Token::SYMBOL) {
		if (prg->variables[v[0].i].type == Variable::EXPRESSION) {
			static Variable var = evaluate_expression(prg, prg->variables[v[0].i].e);
			vec = &var;
		}
		else {
			vec = &as_variable(prg, v[0]);
		}
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected symbol at " + get_error_info(prg));
	}

	auto last = std::unique(vec->v.begin(), vec->v.end());
	vec->v.erase(last, vec->v.end());

	return true;
}

static Variable exprfunc_string_format(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;

	std::string result = sformat(prg, v, 0);

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

static bool stringfunc_set_char_at(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &s = as_variable(prg, v[0]);
	int index = as_number(prg, v[1]);
	uint32_t value = as_number(prg, v[2]);

	Variable *p;
	if (v[0].dereference) {
		p = dereference(prg, v[0]);
	}
	else {
		p = &s;
	}

	p->s = util::utf8_substr(p->s, 0, index) + util::utf8_char_to_string(value) + util::utf8_substr(p->s, index+1);

	return true;
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

static Variable exprfunc_string_ltrim(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);

	v1.s = util::ltrim(str);

	return v1;
}

static Variable exprfunc_string_rtrim(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable v1;
	v1.type = Variable::STRING;

	std::string str = as_string(prg, v[0]);

	v1.s = util::rtrim(str);

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
	
	if (vec.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

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
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

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

	Variable var;
	var.type = Variable::NUMBER;
	var.n = id.v.size();

	return var;
}

static bool vectorfunc_insert(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &id = as_variable(prg, v[0]);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	double index = as_number(prg, v[1]);

	id.type = Variable::VECTOR;

	if (index < 0 || index > id.v.size()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid index at " + get_error_info(prg));
	}

	Variable var;

	if (v[2].type == Token::NUMBER) {
		var.type = Variable::NUMBER;
		var.n = v[2].n;
	}
	else if (v[2].type == Token::SYMBOL) {
		var = as_variable(prg, v[2]);
		var.constant = false;
	}
	else {
		var.type = Variable::STRING;
		var.s = v[2].s;
	}

	id.v.insert(id.v.begin()+index, var);

	return true;
}

static bool vectorfunc_erase(Program *prg, const std::vector<Token> &v)
{

	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

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
	MIN_ARGS(1)

	for (size_t i = 0; i < v.size(); i++) {
		Variable &id = as_variable(prg, v[i]);

		if (id.constant) {
			throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
		}

		id.type = Variable::VECTOR;

		id.v.clear();
		id.v = std::vector<Variable>(); // set capacity to 0 (free memory)
	}

	return true;
}

static bool vectorfunc_reserve(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	int n = as_number(prg, v[1]);

	id.type = Variable::VECTOR;

	id.v.reserve(n);

	return true;
}

static bool mapfunc_clear(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	for (size_t i = 0; i < v.size(); i++) {
		Variable &id = as_variable(prg, v[i]);

		if (id.constant) {
			throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant map at " + get_error_info(prg));
		}

		id.type = Variable::MAP;

		id.m.clear();
	}

	return true;
}

static bool mapfunc_erase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &id = as_variable(prg, v[0]);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant map at " + get_error_info(prg));
	}

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

	SDL_IOStream *f = SDL_IOFromFile(filename.c_str(), mode.c_str());

	if (f == nullptr) {
		v1.n = -1;
		return v1;
	}

	info->files[info->file_id++] = f;

	return v1;
}

static Variable exprfunc_file_open_cpa(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string filename = as_string(prg, v[0]);
	
	File_Info *info = file_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = info->file_id;

	int sz;

	SDL_IOStream *f = util::open_file(filename.c_str(), &sz);

	if (f == nullptr) {
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
	INFO_EXISTS(info->files, id)

	SDL_CloseIO(info->files[id]);
	info->files.erase(info->files.find(id));

	return true;
}

static Variable exprfunc_file_read(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable var;
	var.type = Variable::STRING;

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_IOStream *f = info->files[id];

	bool skipped = false;

	while (true) {
		Uint8 c;
		SDL_ReadU8(f, &c);
		if (SDL_GetIOStatus(f) == SDL_IO_STATUS_EOF) {
			break;
		}
		bool space = isspace(c);
		if (skipped == false && !space) {
			skipped = true;
		}
		if (!space && skipped == true) {
			char buf[2];
			buf[0] = c;
			buf[1] = 0;
			var.s += buf;
		}
		else if (space) {
			break;
		}
	}

	return var;
}

static Variable exprfunc_file_read_line(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable var;
	var.type = Variable::STRING;

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_IOStream *f = info->files[id];

	while (true) {
		Uint8 c;
		SDL_ReadU8(f, &c);
		if (SDL_GetIOStatus(f) == SDL_IO_STATUS_EOF) {
			break;
		}
		char buf[2];
		buf[0] = c;
		buf[1] = 0;
		var.s += buf;
		if (c == '\n') {
			break;
		}
	}

	return var;
}

static Variable exprfunc_file_read_byte(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	Variable var;
	var.type = Variable::NUMBER;

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	Uint8 c;
	SDL_ReadU8(info->files[id], &c);

	var.n = c;

	return var;
}

static bool filefunc_write_byte(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	int b = as_number(prg, v[1]);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_IOStream *f = info->files[id];

	SDL_WriteU8(f, (Uint8)b);

	return true;
}

static bool filefunc_write(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string val = as_string(prg, v[1]);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_WriteIO(info->files[id], val.c_str(), val.length());

	return true;
}

static bool filefunc_print(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number(prg, v[0]);

	std::string val = sformat(prg, v, 1);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_WriteIO(info->files[id], val.c_str(), val.length());

	return true;
}

static Variable exprfunc_file_tell(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	Variable var;
	var.type = Variable::NUMBER;

	var.n = SDL_TellIO(info->files[id]);

	return var;
}

static bool filefunc_seek(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	Sint64 o = as_number(prg, v[1]);
	int whence = as_number(prg, v[2]);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_SeekIO(info->files[id], o, (SDL_IOWhence)whence);

	return true;
}

static Variable exprfunc_file_eof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	Variable var;
	var.type = Variable::NUMBER;

	var.n = SDL_GetIOStatus(info->files[id]) == SDL_IO_STATUS_EOF;

	return var;
}

static bool twinklefunc_text_fore(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int c = as_number(prg, v[0]);
	int c_b = as_number(prg, v[1]);

	twinkle::set_fore((twinkle::TWINKLE_COLOR)c, c_b);

	return true;
}

static bool twinklefunc_text_back(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int c = as_number(prg, v[0]);
	int c_b = as_number(prg, v[1]);

	twinkle::set_back((twinkle::TWINKLE_COLOR)c, c_b);

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

static Variable exprfunc_cfg_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string cfg_name = as_string(prg, v[0]);

	CFG_Info *info = cfg_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;

	std::map<std::string, Config_Value> val = load_cfg(prg, cfg_name);
	int id = info->cfg_id++;
	v1.n = id;
	info->cfgs[id] = val;

	return v1;
}

static bool cfgfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	CFG_Info *info = cfg_info(prg);
	INFO_EXISTS(info->cfgs, id)
	info->cfgs.erase(id);

	return true;
}

static Variable exprfunc_cfg_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string cfg_name = as_string(prg, v[1]);

	Variable v1;
	v1.type = Variable::NUMBER;

	bool success = save_cfg(prg, id, cfg_name);

	v1.n = success;

	return v1;
}

static Variable exprfunc_cfg_typeof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	Variable v1;
	v1.type = Variable::STRING;

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		v1.s = "unknown";
	}
	else if (info->cfgs[id][name].type == Variable::NUMBER) {
		v1.s = "number";
	}
	else {
		v1.s = "string";
	}

	return v1;
}

static Variable exprfunc_cfg_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	Variable v1;
	v1.type = Variable::NUMBER;

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		v1.n = 0;
	}
	else {
		v1.n = info->cfgs[id][name].n;
	}

	return v1;
}

static Variable exprfunc_cfg_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	Variable v1;
	v1.type = Variable::STRING;

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		v1.s = "";
	}
	else {
		v1.s = info->cfgs[id][name].s;
	}

	return v1;
}

static bool cfgfunc_set_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	double val = as_number(prg, v[2]);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	Config_Value value;
	value.type = Variable::NUMBER;
	value.n = val;

	info->cfgs[id][name] = value;

	return true;
}

static bool cfgfunc_set_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	std::string val = as_string(prg, v[2]);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	Config_Value value;
	value.type = Variable::STRING;
	value.s = val;

	info->cfgs[id][name] = value;

	return true;
}

static Variable exprfunc_cfg_exists(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	CFG_Info *info = cfg_info(prg);

	INFO_EXISTS(info->cfgs, id)

	bool found = info->cfgs[id].find(name) != info->cfgs[id].end();

	Variable v1;
	v1.type = Variable::NUMBER;
	v1.n = found;

	return v1;
}

static bool cfgfunc_erase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v[0]);
	std::string name = as_string(prg, v[1]);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	std::map<std::string, Config_Value>::iterator it;
	if ((it = info->cfgs[id].find(name)) == info->cfgs[id].end()) {
		return true;
	}
	info->cfgs[id].erase(it);

	return true;
}

static util::JSON *json_from_arg(Program *prg, const Token &t)
{
	if (t.type == Token::SYMBOL) {
		Variable &var = as_variable(prg, t);
		if (var.type == Variable::POINTER) {
			return shim::shim_json;
		}
		else {
			int id = as_number(prg, t);
			JSON_Info *info = json_info(prg);
			INFO_EXISTS(info->jsons, id)
			return info->jsons[id];
		}
	}
	return shim::shim_json;
}

static Variable exprfunc_json_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v[0]);

	JSON_Info *info = json_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = info->json_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		util::JSON *json = new util::JSON(name, load_from_filesystem);

		info->jsons[info->json_id++] = json;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static Variable exprfunc_json_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool array = as_number(prg, v[0]);

	JSON_Info *info = json_info(prg);

	Variable v1;
	v1.type = Variable::NUMBER;

	v1.n = info->json_id;

	try {
		util::JSON *json = new util::JSON(array);

		info->jsons[info->json_id++] = json;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool jsonfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	delete info->jsons[id];
	info->jsons.erase(info->jsons.find(id));

	return true;
}

static Variable exprfunc_json_exists(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	Variable v1;
	v1.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n != nullptr;

	return v1;
}

static Variable exprfunc_json_typeof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	Variable v1;
	v1.type = Variable::STRING;

	util::JSON::Node *n = json->get_root()->find(name);
	util::JSON::Node::Type t = n->get_type();

	switch (t) {
		case util::JSON::Node::STRING:
			v1.s = "string";
		case util::JSON::Node::BOOL:
			v1.s = "bool";
		case util::JSON::Node::DOUBLE:
			v1.s = "number";
		default:
			std::string val = n->get_value();
			if (val == "[array]") {
				v1.s = "array";
			}
			else {
				v1.s = "hash";
			}
	}

	return v1;
}

static Variable exprfunc_json_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	Variable v1;
	v1.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n->size();

	return v1;
}

static Variable exprfunc_json_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	Variable v1;
	v1.type = Variable::STRING;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.s = n->as_string();

	return v1;
}

static Variable exprfunc_json_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	Variable v1;
	v1.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n->as_double();

	return v1;
}

static Variable exprfunc_json_get_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	Variable v1;
	v1.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root()->find(name);
	v1.n = n->as_bool();

	return v1;
}

static bool jsonfunc_set_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	std::string val = as_string(prg, v[2]);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_string(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_set_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	double val = as_number(prg, v[2]);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_double(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_set_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	bool val = (bool)as_number(prg, v[2]);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_bool(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_add_array(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_array(name);

	return true;
}

static bool jsonfunc_add_hash(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_hash(name);

	return true;
}

static bool jsonfunc_remove(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string name = as_string(prg, v[1]);
	
	json->remove(name);

	return true;
}

static Variable exprfunc_json_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v[0]);
	std::string fn = as_string(prg, v[1]);

	Variable var;
	var.type = Variable::NUMBER;

	util::JSON::Node *n = json->get_root();
	std::string s = n->to_json();

	FILE *f = fopen(fn.c_str(), "w");
	if (f == nullptr) {
		var.n = 0;
	}
	else {
		fprintf(f, "%s", s.c_str());
		fclose(f);
		var.n = 1;
	}

	return var;
}

class BooBoo_Trigger : public util::Trigger
{
public:
	BooBoo_Trigger(Program *prg, std::string name, int func) :
		prg(prg),
		name(name),
		func(func)
	{
	}

	virtual void run()
	{
		std::vector<Token> v;
		Token t;
		t.type = Token::STRING;
		t.s = t.token = name;
		t.dereference = 0;
		v.push_back(t);
		call_void_function(prg, func, v, 0);
	}

	virtual ~BooBoo_Trigger()
	{
	}

private:
	Program *prg;
	std::string name;
	int func;
};

static bool jsonfunc_register_number(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(3)

	Variable &var = prg->variables[v[0].i];
	std::string name = as_string(prg, v[1]);
	bool readonly = (bool)as_number(prg, v[2]);

	BooBoo_Trigger *trigger;
	
	if (v.size() > 3) {
		trigger = new BooBoo_Trigger(prg, name, as_function(prg, v[3]));
	}
	else {
		trigger = nullptr;
	}
	
	util::JSON::Node *root = shim::shim_json->get_root();
	root->add_nested_double("game>" + name, &var.n, var.n, trigger, readonly);

	return true;
}

static bool jsonfunc_register_string(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(3)

	Variable &var = prg->variables[v[0].i];
	std::string name = as_string(prg, v[1]);
	bool readonly = (bool)as_number(prg, v[2]);
	
	BooBoo_Trigger *trigger;
	
	if (v.size() > 3) {
		trigger = new BooBoo_Trigger(prg, name, as_function(prg, v[3]));
	}
	else {
		trigger = nullptr;
	}
	
	util::JSON::Node *root = shim::shim_json->get_root();
	root->add_nested_string("game>" + name, &var.s, var.s, trigger, readonly);

	return true;
}

static Variable exprfunc_load_cpa(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable v1;
	v1.type = Variable::NUMBER;

	std::string name = as_string(prg, v[0]);

	CPA_Info *info = cpa_info(prg);

	v1.n = info->cpa_id;

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v[1]);
	}

	try {
		util::CPA *cpa;
		if (load_from_filesystem) {
			cpa = new util::CPA(name);
		}
		else {
			int sz;
			char *data = util::slurp_file(name, &sz);
			cpa = new util::CPA((Uint8 *)data, sz);
		}

		CPA *c = new CPA;
		c->cpa = cpa;

		info->cpas[info->cpa_id++] = c;
	}
	catch (util::Error &e) {
		v1.n = -1;
	}

	return v1;
}

static bool cpafunc_set_cpa(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	
	CPA_Info *info = cpa_info(prg);

	INFO_EXISTS(info->cpas, id)

	util::CPA *cpa = info->cpas[id]->cpa;

	shim::cpa = cpa;

	return true;
}

static bool cpafunc_set_default_cpa(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	shim::cpa = shim::default_cpa;

	return true;
}

static bool cpafunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v[0]);
	CPA_Info *info = cpa_info(prg);
	INFO_EXISTS(info->cpas, id)
	delete info->cpas[id]->cpa;
	info->cpas.erase(info->cpas.find(id));

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
	add_instruction("sort", corefunc_sort);
	add_instruction("unique", corefunc_unique);

	add_expression_handler("string_format", exprfunc_string_format);
	add_expression_handler("string_char_at", exprfunc_string_char_at);
	add_instruction("string_set_char_at", stringfunc_set_char_at);
	add_expression_handler("string_length", exprfunc_string_length);
	add_expression_handler("string_from_number", exprfunc_string_from_number);
	add_expression_handler("string_substr", exprfunc_string_substr);
	add_expression_handler("string_uppercase", exprfunc_string_uppercase);
	add_expression_handler("string_lowercase", exprfunc_string_lowercase);
	add_expression_handler("string_trim", exprfunc_string_trim);
	add_expression_handler("string_ltrim", exprfunc_string_ltrim);
	add_expression_handler("string_rtrim", exprfunc_string_rtrim);
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
	add_expression_handler("file_open_cpa", exprfunc_file_open_cpa);
	add_instruction("file_close", filefunc_close);
	add_expression_handler("file_read", exprfunc_file_read);
	add_expression_handler("file_read_line", exprfunc_file_read_line);
	add_expression_handler("file_read_byte", exprfunc_file_read_byte);
	add_instruction("file_write_byte", filefunc_write_byte);
	add_instruction("file_write", filefunc_write);
	add_instruction("file_print", filefunc_print);
	add_expression_handler("file_eof", exprfunc_file_eof);
	add_expression_handler("file_tell", exprfunc_file_tell);
	add_instruction("file_seek", filefunc_seek);
	
	add_instruction("text_fore", twinklefunc_text_fore);
	add_instruction("text_back", twinklefunc_text_back);
	add_instruction("text_reset", twinklefunc_reset);
	add_expression_handler("getch", exprfunc_twinkle_getch);
	add_instruction("text_clear", twinklefunc_clear);
	
	add_expression_handler("cfg_load", exprfunc_cfg_load);
	add_instruction("cfg_destroy", cfgfunc_destroy);
	add_expression_handler("cfg_save", exprfunc_cfg_save);
	add_expression_handler("cfg_typeof", exprfunc_cfg_typeof);
	add_expression_handler("cfg_get_number", exprfunc_cfg_get_number);
	add_expression_handler("cfg_get_string", exprfunc_cfg_get_string);
	add_instruction("cfg_set_number", cfgfunc_set_number);
	add_instruction("cfg_set_string", cfgfunc_set_string);
	add_expression_handler("cfg_exists", exprfunc_cfg_exists);
	add_instruction("cfg_erase", cfgfunc_erase);
	add_expression_handler("json_load", exprfunc_json_load);
	add_expression_handler("json_create", exprfunc_json_create);
	add_instruction("json_destroy", jsonfunc_destroy);
	add_expression_handler("json_exists", exprfunc_json_exists);
	add_expression_handler("json_typeof", exprfunc_json_typeof);
	add_expression_handler("json_size", exprfunc_json_size);
	add_expression_handler("json_get_string", exprfunc_json_get_string);
	add_expression_handler("json_get_number", exprfunc_json_get_number);
	add_expression_handler("json_get_bool", exprfunc_json_get_bool);
	add_instruction("json_set_string", jsonfunc_set_string);
	add_instruction("json_set_number", jsonfunc_set_number);
	add_instruction("json_set_bool", jsonfunc_set_bool);
	add_instruction("json_add_array", jsonfunc_add_array);
	add_instruction("json_add_hash", jsonfunc_add_hash);
	add_instruction("json_remove", jsonfunc_remove);
	add_expression_handler("json_save", exprfunc_json_save);
	add_instruction("json_register_number", jsonfunc_register_number);
	add_instruction("json_register_string", jsonfunc_register_string);
	add_expression_handler("cpa_load", exprfunc_load_cpa);
	add_instruction("cpa_set", cpafunc_set_cpa);
	add_instruction("cpa_set_default", cpafunc_set_default_cpa);
	add_instruction("cpa_destroy", cpafunc_destroy);
}

void end_lib_standard()
{
}

void standard_lib_destroy_program(Program *prg)
{
	File_Info *file_i = file_info(prg);
	for (size_t i = 0; i < file_i->files.size(); i++) {
		SDL_CloseIO(file_i->files[i]);
	}
	JSON_Info *json_i = json_info(prg);
	for (std::map<int, util::JSON *>::iterator i = json_i->jsons.begin(); i != json_i->jsons.end(); i++) {
		delete json_i->jsons[(*i).first];
	}
	CPA_Info *cpa_i = cpa_info(prg);
	for (std::map<int, CPA *>::iterator i = cpa_i->cpas.begin(); i != cpa_i->cpas.end(); i++) {
		delete ((*i).second)->cpa;
		delete (*i).second;
	}
	delete file_i;
	delete json_i;
	delete cpa_i;

	CFG_Info *cfg_i = cfg_info(prg);
	delete cfg_i;

	set_black_box(prg, "com.nooskewl.booboo.files", nullptr);
	set_black_box(prg, "com.nooskewl.booboo.cfg", nullptr);
	set_black_box(prg, "com.nooskewl.booboo.json", nullptr);
	set_black_box(prg, "com.nooskewl.booboo.cpa", nullptr);
}
