#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <regex>

#include <shlwapi.h>

#include <sys/stat.h>

#include <shim5/shim5.h>
#include <shim5/internal/audio.h>
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

File_Info *file_info(Program *prg)
{
	File_Info *info = (File_Info *)get_black_box("com.nooskewl.booboo.files");
	if (info == nullptr) {
		info = new File_Info;
		info->file_id = 0;
		set_black_box("com.nooskewl.booboo.files", info);
	}
	return info;
}

CFG_Info *cfg_info(Program *prg)
{
	CFG_Info *info = (CFG_Info *)booboo::get_black_box("com.nooskewl.booboo.cfg");
	if (info == nullptr) {
		info = new CFG_Info;
		info->cfg_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.cfg", info);
	}
	return info;
}

JSON_Info *json_info(Program *prg)
{
	JSON_Info *info = (JSON_Info *)booboo::get_black_box("com.nooskewl.booboo.json");
	if (info == nullptr) {
		info = new JSON_Info;
		info->json_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.json", info);
	}
	return info;
}

CPA_Info *cpa_info(Program *prg)
{
	CPA_Info *info = (CPA_Info *)booboo::get_black_box("com.nooskewl.booboo.cpa");
	if (info == nullptr) {
		info = new CPA_Info;
		info->cpa_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.cpa", info);
	}
	return info;
}

MML_Info *mml_info(Program *prg)
{
	MML_Info *info = (MML_Info *)booboo::get_black_box("com.nooskewl.booboo.mml");
	if (info == nullptr) {
		info = new MML_Info;
		info->mml_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.mml", info);
	}
	return info;
}

MML_Instance_Info *mml_instance_info(Program *prg)
{
	MML_Instance_Info *info = (MML_Instance_Info *)booboo::get_black_box("com.nooskewl.booboo.mml_instance");
	if (info == nullptr) {
		info = new MML_Instance_Info;
		info->instance_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.mml_instance", info);
	}
	return info;
}

Sample_Info *sample_info(Program *prg)
{
	Sample_Info *info = (Sample_Info *)booboo::get_black_box("com.nooskewl.booboo.sample");
	if (info == nullptr) {
		info = new Sample_Info;
		info->sample_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.sample", info);
	}
	return info;
}

Sample_Instance_Info *sample_instance_info(Program *prg)
{
	Sample_Instance_Info *info = (Sample_Instance_Info *)booboo::get_black_box("com.nooskewl.booboo.sample_instance");
	if (info == nullptr) {
		info = new Sample_Instance_Info;
		info->instance_id = 0;
		booboo::set_black_box("com.nooskewl.booboo.sample_instance", info);
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
	FILE *f;
#ifdef __GNUC__
	f = fopen(cfg_path(cfg_name).c_str(), "w");
	if (f == nullptr) {
		return false;
	}
#else
	errno_t err = fopen_s(&f, cfg_path(cfg_name).c_str(), "w");
	if (err) {
		return false;
	}
#endif

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
	std::string fmt = as_string(prg, v, skip);
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
				v1 = dereference(prg, v, _tok);
			}
			else {
				v1 = &as_variable(prg, v, _tok);
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
				evaluate_expression(prg, v1->e);
				if (IS_NUMBER(prg->result)) {
					format = (format == "") ? "g" : format;
					char buf[1000];
					if (format.find('c') != std::string::npos || format.find('d') != std::string::npos || format.find('x') != std::string::npos) {
						snprintf(buf, 1000, ("%" + format).c_str(), (int)prg->result.n);
					}
					else {
						snprintf(buf, 1000, ("%" + format).c_str(), prg->result.n);
					}
					val = buf;
				}
				else {
					if (IS_STRING(prg->result)) {
						val = prg->result.s;
					}
					else if (IS_VECTOR(prg->result)) {
						val = "-vector-";
					}
					else if (IS_MAP(prg->result)) {
						val = "-map-";
					}
					else if (IS_FUNCTION(prg->result)) {
						val = "-function-";
					}
					else if (IS_LABEL(prg->result)) {
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

static void exprfunc_getenv(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string get = as_string(prg, v, 0);

	char *ptr;
#ifdef __GNUC__
	ptr = getenv(get.c_str());
#else
	size_t sz;
	_dupenv_s(&ptr, &sz, get.c_str());
#endif

	prg->result.set_type(Variable::STRING);
	prg->result.s = ptr == nullptr ? "" : ptr;
}

static void exprfunc_getcwd(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	char buf[2048];
	char *res = getcwd(buf, 2048);

	prg->result.set_type(Variable::STRING);
	prg->result.s = res == nullptr ? "" : buf;
}

static bool corefunc_print(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string result = sformat(prg, v, 0);

	printf("%s", result.c_str());

	return true;
}

static void exprfunc_input(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result.set_type(Variable::STRING);

	if (std::cin.eof()) {
		prg->result.s = "";
	}
	else {
		std::cin >> prg->result.s;
	}
}

static void exprfunc_mkdir(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string path = as_string(prg, v, 0);

	bool success = util::mkdir(path);

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = success;
}

static void exprfunc_get_system_language(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result.set_type(Variable::STRING);
	prg->result.s = util::get_system_language();
}

static void exprfunc_get_full_path(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);

	prg->result.set_type(Variable::STRING);

	char buf[MAX_PATH];
	GetFullPathNameA(str.c_str(), MAX_PATH, buf, NULL);
	prg->result.s = buf;
}

static void exprfunc_list_drives(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result.set_type(Variable::VECTOR);
	
	DWORD d = GetLogicalDrives();
	for (int i = 0; i < 32; i++) {
		bool set = d & (1 << i);
		if (set) {
			Variable s;
			s.type = Variable::STRING;
			char buf[10];
			snprintf(buf, 10, "%c", 'A' + i);
			s.s = buf;
			prg->result.v.push_back(s);
		}
	}
}

static void exprfunc_list_directory(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string glob = as_string(prg, v, 0);

	prg->result.set_type(Variable::VECTOR);

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
		if (PathIsDirectoryA((path_part + "/" + fn).c_str())) {
			_is_dir = true;
		}
		Variable v;
		v.type = Variable::STRING;
		v.s = fn;
		filenames.v.push_back(v);
		v.type = Variable::NUMBER;
		v.n = _is_dir;
		is_dir.v.push_back(v);
	}

	prg->result.v.push_back(filenames);
	prg->result.v.push_back(is_dir);
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
		t.dereference = 0;
		t.type = Token::SYMBOL;
		t.i = i1;
		params.push_back(t);
		t.i = i2;
		params.push_back(t);
		call_function(prg, func, params, 0);
		if (prg->result.n == false) {
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
			evaluate_expression(prg, prg->variables[v[0].i].e);
			static Variable _v;
			_v = prg->result;
			vec = &_v;
		}
		else {
			vec = &as_variable(prg, v, 0);
		}
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected symbol at " + get_error_info(prg));
	}
	int func = as_function(prg, v, 1);

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
			evaluate_expression(prg, prg->variables[v[0].i].e);
			static Variable _v;
			_v = prg->result;
			vec = &_v;
		}
		else {
			vec = &as_variable(prg, v, 0);
		}
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Expected symbol at " + get_error_info(prg));
	}

	auto last = std::unique(vec->v.begin(), vec->v.end());
	vec->v.erase(last, vec->v.end());

	return true;
}

static void exprfunc_string_format(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string result = sformat(prg, v, 0);

	prg->result.set_type(Variable::STRING);
	prg->result.s = result;
}

static void exprfunc_string_char_at(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	std::string s = as_string(prg, v, 0);
	int index = as_number(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);
	
	uint32_t value = util::utf8_char(s, index);

	prg->result.n = value;
}

static bool stringfunc_set_char_at(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &s = as_variable(prg, v, 0);
	int index = as_number(prg, v, 1);
	uint32_t value = as_number(prg, v, 2);

	Variable *p;
	if (v[0].dereference) {
		p = dereference(prg, v, 0);
	}
	else {
		p = &s;
	}

	p->s = util::utf8_substr(p->s, 0, index) + util::utf8_char_to_string(value) + util::utf8_substr(p->s, index+1);

	return true;
}

static void exprfunc_string_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string s = as_string(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = util::utf8_len(s);
}

static void exprfunc_string_from_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	uint32_t n = as_number(prg, v, 0);

       	prg->result.set_type(Variable::STRING);

	prg->result.s = util::utf8_char_to_string(n);
}

static void exprfunc_string_substr(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	std::string str = as_string(prg, v, 0);

	int start = as_number(prg, v, 1);
	int count = -1;

	if (v.size() >= 3) {
		count = as_number(prg, v, 2);
	}

	prg->result.set_type(Variable::STRING);

	prg->result.s = util::utf8_substr(str, start, count);
}

static void exprfunc_string_uppercase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);

	prg->result.set_type(Variable::STRING);

	prg->result.s = util::uppercase(str);
}

static void exprfunc_string_lowercase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);

	prg->result.set_type(Variable::STRING);

	prg->result.s = util::lowercase(str);
}

static void exprfunc_string_trim(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);

	prg->result.set_type(Variable::STRING);

	prg->result.s = util::trim(str);
}

static void exprfunc_string_ltrim(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);

	prg->result.set_type(Variable::STRING);

	prg->result.s = util::ltrim(str);
}

static void exprfunc_string_rtrim(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);

	prg->result.set_type(Variable::STRING);

	prg->result.s = util::rtrim(str);
}

static void exprfunc_string_find(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	std::string str = as_string(prg, v, 0);
	std::string needle = as_string(prg, v, 1);

	size_t pos = str.find(needle);
       	
	prg->result.set_type(Variable::NUMBER);

	if (pos == std::string::npos) {
		prg->result.n = -1;
	}
	else {
		prg->result.n = pos;
	}
}

static void exprfunc_string_replace(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(3)

	std::string str = as_string(prg, v, 0);
	std::string regex = as_string(prg, v, 1);
	std::string fmt = as_string(prg, v, 2);

	bool ignore_case;
	if (v.size() > 3) {
		ignore_case = as_number(prg, v, 3);
	}
	else {
		ignore_case = false;
	}

	prg->result.set_type(Variable::STRING);

	std::regex pattern(regex, ignore_case ? std::regex_constants::ECMAScript | std::regex_constants::icase : std::regex_constants::ECMAScript);

	prg->result.s = std::regex_replace(str, pattern, fmt.c_str());
}

static void exprfunc_string_match(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	std::string str = as_string(prg, v, 0);
	std::string regex = as_string(prg, v, 1);

	bool ignore_case;
	if (v.size() > 2) {
		ignore_case = as_number(prg, v, 2);
	}
	else {
		ignore_case = false;
	}

	prg->result.set_type(Variable::VECTOR);

	std::regex pattern(regex, ignore_case ? std::regex_constants::ECMAScript | std::regex_constants::icase : std::regex_constants::ECMAScript);

	std::smatch match;

	if (std::regex_match(str, match, pattern)) {
		for (size_t i = 1; i < match.size(); i++) {
			std::ssub_match sub = match[i];
			Variable v;
			v.type = Variable::STRING;
			v.s = sub.str();
			prg->result.v.push_back(v);
		}
	}
}

static void exprfunc_string_matches(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	std::string str = as_string(prg, v, 0);
	std::string regex = as_string(prg, v, 1);
       	
	bool ignore_case;
	if (v.size() > 2) {
		ignore_case = as_number(prg, v, 2);
	}
	else {
		ignore_case = false;
	}

	prg->result.set_type(Variable::NUMBER);
	
	std::regex pattern(regex, ignore_case ? std::regex_constants::ECMAScript | std::regex_constants::icase : std::regex_constants::ECMAScript);

	prg->result.n = std::regex_search(str, pattern);
}

static void exprfunc_math_sin(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = sin(n);
}

static void exprfunc_math_cos(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = cos(n);
}

static void exprfunc_math_tan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = tan(n);
}

static void exprfunc_math_asin(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = asin(n);
}

static void exprfunc_math_acos(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = acos(n);
}

static void exprfunc_math_atan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = atan(n);
}

static void exprfunc_math_atan2(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	double n = as_number(prg, v, 0);
	double n2 = as_number(prg, v, 1);


	prg->result.set_type(Variable::NUMBER);

	prg->result.n = atan2(n, n2);
}

static void exprfunc_math_abs(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);
		
	prg->result.n = fabs(n);
}

static void exprfunc_math_pow(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	double n = as_number(prg, v, 0);
	double n2 = as_number(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = pow(n, n2);
}

static void exprfunc_math_sqrt(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = sqrt(n);
}

static void exprfunc_math_floor(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = floor(n);
}

static void exprfunc_math_ceil(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = ceil(n);
}

static void exprfunc_math_neg(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = -n;
}

static void exprfunc_math_intmod(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int n1 = (int)as_number(prg, v, 0);
	int n2 = (int)as_number(prg, v, 1);

       	prg->result.set_type(Variable::NUMBER);

	prg->result.n = n1 % n2;
}

static void exprfunc_math_fmod(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	double n1 = as_number(prg, v, 0);
	double n2 = as_number(prg, v, 1);

       	prg->result.set_type(Variable::NUMBER);

	prg->result.n = fmod(n1, n2);
}

static void exprfunc_math_sign(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);
		
	prg->result.n = sign(n);
}

static void exprfunc_math_exp(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);
		
	prg->result.n = exp(n);
}

static void exprfunc_math_hypot(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	double n = as_number(prg, v, 0);
	double n2 = as_number(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);
		
	prg->result.n = hypot(n, n2);
}

static void exprfunc_math_log(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);
		
	prg->result.n = log(n);
}

static void exprfunc_math_log10(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	double n = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);
		
	prg->result.n = log10(n);
}

static void exprfunc_math_min(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	double n = as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n = MIN(n, as_number(prg, v, i));
	}
	
	prg->result.set_type(Variable::NUMBER);
	prg->result.n = n;
}

static void exprfunc_math_max(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	double n = as_number(prg, v, 0);

	for (size_t i = 1; i < v.size(); i++) {
		n = MAX(n, as_number(prg, v, i));
	}
	
	prg->result.set_type(Variable::NUMBER);
	prg->result.n = n;
}

static bool vectorfunc_init(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	Variable &vec = as_variable(prg, v, 0);
	
	if (vec.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	vec.type = Variable::VECTOR;
	vec.v.clear();

	for (size_t i = 1; i < v.size(); i++) {
		if (v[i].type == Token::NUMBER) {
			Variable var;
			var.type = Variable::NUMBER;
			var.n = as_number(prg, v, i);
			vec.v.push_back(var);
		}
		else if (v[i].type == Token::STRING) {
			Variable var;
			var.type = Variable::STRING;
			var.s = as_string(prg, v, i);
			vec.v.push_back(var);
		}
		else {
			Variable var = as_variable_resolve(prg, v, i);
			var.constant = false;
			vec.v.push_back(var);
		}
	}

	return true;
}

static bool vectorfunc_add(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	Variable &id = as_variable(prg, v, 0);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	id.type = Variable::VECTOR;

	for (size_t i  = 1; i < v.size(); i++) {
		Variable var = as_variable_resolve(prg, v, i);

		var.constant = false;

		id.v.push_back(var);
	}

	return true;
}

static void exprfunc_vector_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable *var = as_variable_pointer(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = var->v.size();
}

static bool vectorfunc_insert(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable &id = as_variable(prg, v, 0);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	double index = as_number(prg, v, 1);

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
		var = as_variable(prg, v, 2);
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

	Variable &id = as_variable(prg, v, 0);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	double index = as_number(prg, v, 1);

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
		Variable &id = as_variable(prg, v, i);

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

	Variable &id = as_variable(prg, v, 0);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant vector at " + get_error_info(prg));
	}

	int n = as_number(prg, v, 1);

	id.type = Variable::VECTOR;

	id.v.reserve(n);

	return true;
}

static void exprfunc_vector_it_start(Program *prg, const std::vector<Token> &v)
{
	Variable *vec = as_variable_pointer(prg, v, 0);

	prg->result.set_type(Variable::USER);
	prg->result.n = 0;
	prg->result.p = vec;
}

static void exprfunc_vector_it_end(Program *prg, const std::vector<Token> &v)
{
	Variable *vec = as_variable_pointer(prg, v, 0);

	prg->result.set_type(Variable::USER);
	prg->result.n = vec->v.size();
	prg->result.p = vec;
}

static void exprfunc_vector_it_inc(Program *prg, const std::vector<Token> &v)
{
	Variable *it = as_variable_pointer(prg, v, 0);
	int inc = as_number(prg, v, 1);

	prg->result.set_type(Variable::USER);
	prg->result.n = MIN(it->p->v.size(), it->n + inc);
	prg->result.p = it->p;
}

static void exprfunc_vector_it_get(Program *prg, const std::vector<Token> &v)
{
	Variable *it = as_variable_pointer(prg, v, 0);

	prg->result.set_type(Variable::POINTER);
	prg->result.p = &it->p->v[it->n];
}

static void exprfunc_vector_it_erase(Program *prg, const std::vector<Token> &v)
{
	Variable *it = as_variable_pointer(prg, v, 0);

	it->p->v.erase(it->p->v.begin()+it->n);
	
	prg->result.set_type(Variable::USER);
	prg->result.n = it->n;
	prg->result.p = it->p;
}

static bool mapfunc_clear(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	for (size_t i = 0; i < v.size(); i++) {
		Variable &id = as_variable(prg, v, i);

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

	Variable &id = as_variable(prg, v, 0);
	
	if (id.constant) {
		throw Error(std::string(__FUNCTION__) + ": " + "Attempt to change a constant map at " + get_error_info(prg));
	}

	std::string key = as_string(prg, v, 1);

	CHECK_MAP(id)

	std::map<std::string, Variable>::iterator it = id.m.find(key);

	if (it == id.m.end()) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid map key at " + get_error_info(prg));
	}

	id.m.erase(it);

	return true;
}

static void exprfunc_map_keys(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	Variable m = as_variable_resolve(prg, v, 0);

	CHECK_MAP(m)

	std::map<std::string, Variable>::iterator it;

	prg->result.set_type(Variable::VECTOR);

	for (it = m.m.begin(); it != m.m.end(); it++) {
		std::pair<std::string, Variable> p = *it;
		Variable var;
		var.type = Variable::STRING;
		var.s = p.first;
		prg->result.v.push_back(var);
	}
}

static void exprfunc_file_open(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	std::string filename = as_string(prg, v, 0);
	std::string mode = as_string(prg, v, 1);
	
	File_Info *info = file_info(prg);

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = info->file_id;

	SDL_IOStream *f = SDL_IOFromFile(filename.c_str(), mode.c_str());

	if (f == nullptr) {
		prg->result.n = -1;
		return;
	}

	info->files[info->file_id++] = f;
}

static void exprfunc_file_open_cpa(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string filename = as_string(prg, v, 0);
	
	File_Info *info = file_info(prg);

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = info->file_id;

	int sz;

	SDL_IOStream *f = util::open_file(filename.c_str(), &sz);

	if (f == nullptr) {
		prg->result.n = -1;
		return;
	}

	info->files[info->file_id++] = f;
}

static bool filefunc_close(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	
	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_CloseIO(info->files[id]);
	info->files.erase(info->files.find(id));

	return true;
}

static void exprfunc_file_read(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	prg->result.set_type(Variable::STRING);

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
			prg->result.s += buf;
		}
		else if (space) {
			break;
		}
	}
}

static void exprfunc_file_read_line(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	prg->result.set_type(Variable::STRING);

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
		prg->result.s += buf;
		if (c == '\n') {
			break;
		}
	}
}

static void exprfunc_file_read_byte(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	Uint8 c;
	SDL_ReadU8(info->files[id], &c);

	prg->result.n = c;
}

static void exprfunc_file_write_byte(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	int b = as_number(prg, v, 1);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	SDL_IOStream *f = info->files[id];

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = SDL_WriteU8(f, (Uint8)b);
}

static void exprfunc_file_write(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string val = as_string(prg, v, 1);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = SDL_WriteIO(info->files[id], val.c_str(), val.length()) == val.length();
}

static void exprfunc_file_print(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int id = as_number(prg, v, 0);

	std::string val = sformat(prg, v, 1);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = SDL_WriteIO(info->files[id], val.c_str(), val.length()) == val.length();
}

static void exprfunc_file_tell(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = SDL_TellIO(info->files[id]);
}

static void exprfunc_file_seek(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v, 0);
	Sint64 o = as_number(prg, v, 1);
	int whence = as_number(prg, v, 2);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = SDL_SeekIO(info->files[id], o, (SDL_IOWhence)whence) != -1;
}

static void exprfunc_file_eof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	File_Info *info = file_info(prg);
	INFO_EXISTS(info->files, id)

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = SDL_GetIOStatus(info->files[id]) == SDL_IO_STATUS_EOF;
}

static bool twinklefunc_text_fore(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int c = as_number(prg, v, 0);
	int c_b = as_number(prg, v, 1);

	twinkle::set_fore((twinkle::TWINKLE_COLOR)c, c_b);

	return true;
}

static bool twinklefunc_text_back(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int c = as_number(prg, v, 0);
	int c_b = as_number(prg, v, 1);

	twinkle::set_back((twinkle::TWINKLE_COLOR)c, c_b);

	return true;
}

static bool twinklefunc_reset(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	twinkle::reset();

	return true;
}

static void exprfunc_twinkle_getch(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)
	
	prg->result.set_type(Variable::NUMBER);

	prg->result.n = twinkle::getch();
}

static void exprfunc_twinkle_kbhit(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)
	
	prg->result.set_type(Variable::NUMBER);

	prg->result.n = twinkle::kbhit();
}

static void exprfunc_twinkle_get_console_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	int w, h;
	twinkle::get_console_size(&w, &h);
	
	prg->result.set_type(Variable::VECTOR);

	Variable var;
	var.type = Variable::NUMBER;

	var.n = w;
	prg->result.v.push_back(var);

	var.n = h;
	prg->result.v.push_back(var);
}

static bool twinklefunc_set_cursor_pos(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int x = as_number(prg, v, 0);
	int y = as_number(prg, v, 1);

	twinkle::set_cursor_pos(x, y);

	return true;
}

static bool twinklefunc_clear(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	twinkle::clear();

	return true;
}

static void exprfunc_cfg_load(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string cfg_name = as_string(prg, v, 0);

	CFG_Info *info = cfg_info(prg);

	prg->result.set_type(Variable::NUMBER);

	std::map<std::string, Config_Value> val = load_cfg(prg, cfg_name);
	int id = info->cfg_id++;
	prg->result.n = id;
	info->cfgs[id] = val;
}

static bool cfgfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	CFG_Info *info = cfg_info(prg);
	INFO_EXISTS(info->cfgs, id)
	info->cfgs.erase(id);

	return true;
}

static void exprfunc_cfg_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string cfg_name = as_string(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);

	bool success = save_cfg(prg, id, cfg_name);

	prg->result.n = success;
}

static void exprfunc_cfg_typeof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	prg->result.set_type(Variable::STRING);

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		prg->result.s = "unknown";
	}
	else if (info->cfgs[id][name].type == Variable::NUMBER) {
		prg->result.s = "number";
	}
	else {
		prg->result.s = "string";
	}
}

static void exprfunc_cfg_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	prg->result.set_type(Variable::NUMBER);

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		prg->result.n = 0;
	}
	else {
		prg->result.n = info->cfgs[id][name].n;
	}
}

static void exprfunc_cfg_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	prg->result.set_type(Variable::STRING);

	if (info->cfgs[id].find(name) == info->cfgs[id].end()) {
		prg->result.s = "";
	}
	else {
		prg->result.s = info->cfgs[id][name].s;
	}
}

static bool cfgfunc_set_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	double val = as_number(prg, v, 2);

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

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	std::string val = as_string(prg, v, 2);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	Config_Value value;
	value.type = Variable::STRING;
	value.s = val;

	info->cfgs[id][name] = value;

	return true;
}

static void exprfunc_cfg_exists(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);

	CFG_Info *info = cfg_info(prg);

	INFO_EXISTS(info->cfgs, id)

	bool found = info->cfgs[id].find(name) != info->cfgs[id].end();

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = found;
}

static bool cfgfunc_erase(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	std::string name = as_string(prg, v, 1);

	CFG_Info *info = cfg_info(prg);
	
	INFO_EXISTS(info->cfgs, id)

	std::map<std::string, Config_Value>::iterator it;
	if ((it = info->cfgs[id].find(name)) == info->cfgs[id].end()) {
		return true;
	}
	info->cfgs[id].erase(it);

	return true;
}

static util::JSON *json_from_arg(Program *prg, const std::vector<Token> &v, int index)
{
	if (v[index].type == Token::SYMBOL) {
		Variable &var = as_variable(prg, v, index);
		if (var.type == Variable::POINTER) {
			return shim::shim_json;
		}
		else {
			int id = as_number(prg, v, index);
			JSON_Info *info = json_info(prg);
			INFO_EXISTS(info->jsons, id)
			return info->jsons[id];
		}
	}
	return shim::shim_json;
}

static void exprfunc_json_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v, 0);

	JSON_Info *info = json_info(prg);

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v, 1);
	}

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = info->json_id;

	try {
		util::JSON *json = new util::JSON(name, load_from_filesystem);

		info->jsons[info->json_id++] = json;
	}
	catch (util::Error &e) {
		prg->result.n = -1;
	}
}

static void exprfunc_json_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	bool array = as_number(prg, v, 0);

	JSON_Info *info = json_info(prg);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = info->json_id;

	try {
		util::JSON *json = new util::JSON(array);

		info->jsons[info->json_id++] = json;
	}
	catch (util::Error &e) {
		prg->result.n = -1;
	}
}

static bool jsonfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	JSON_Info *info = json_info(prg);
	INFO_EXISTS(info->jsons, id)
	delete info->jsons[id];
	info->jsons.erase(info->jsons.find(id));

	return true;
}

static void exprfunc_json_exists(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	prg->result.set_type(Variable::NUMBER);

	util::JSON::Node *n = json->get_root()->find(name);
	prg->result.n = n != nullptr;
}

static void exprfunc_json_typeof(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	prg->result.set_type(Variable::STRING);

	util::JSON::Node *n = json->get_root()->find(name);
	util::JSON::Node::Type t = n->get_type();

	switch (t) {
		case util::JSON::Node::STRING:
			prg->result.s = "string";
		case util::JSON::Node::BOOL:
			prg->result.s = "bool";
		case util::JSON::Node::DOUBLE:
			prg->result.s = "number";
		default:
			std::string val = n->get_value();
			if (val == "[array]") {
				prg->result.s = "array";
			}
			else {
				prg->result.s = "hash";
			}
	}
}

static void exprfunc_json_size(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	prg->result.set_type(Variable::NUMBER);

	util::JSON::Node *n = json->get_root()->find(name);
	prg->result.n = n->size();
}

static void exprfunc_json_get_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	prg->result.set_type(Variable::STRING);

	util::JSON::Node *n = json->get_root()->find(name);
	prg->result.s = n->as_string();
}

static void exprfunc_json_get_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	prg->result.set_type(Variable::NUMBER);

	util::JSON::Node *n = json->get_root()->find(name);
	prg->result.n = n->as_double();
}

static void exprfunc_json_get_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	prg->result.set_type(Variable::NUMBER);

	util::JSON::Node *n = json->get_root()->find(name);
	prg->result.n = n->as_bool();
}

static bool jsonfunc_set_string(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	std::string val = as_string(prg, v, 2);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_string(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_set_number(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	double val = as_number(prg, v, 2);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_double(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_set_bool(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	bool val = (bool)as_number(prg, v, 2);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_bool(name, nullptr, val, NULL, true);

	return true;
}

static bool jsonfunc_add_array(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_array(name);

	return true;
}

static bool jsonfunc_add_hash(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	util::JSON::Node *n = json->get_root();
	n->add_nested_hash(name);

	return true;
}

static bool jsonfunc_remove(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string name = as_string(prg, v, 1);
	
	json->remove(name);

	return true;
}

static void exprfunc_json_save(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	util::JSON *json = json_from_arg(prg, v, 0);
	std::string fn = as_string(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);

	util::JSON::Node *n = json->get_root();
	std::string s = n->to_json();

	FILE *f;
#ifdef __GNUC__
	f = fopen(fn.c_str(), "w");
	if (f == nullptr) {
		prg->result.n = 0;
	}
#else
	errno_t err = fopen_s(&f, fn.c_str(), "w");
	if (err) {
		prg->result.n = 0;
	}
#endif
	else {
		fprintf(f, "%s", s.c_str());
		fclose(f);
		prg->result.n = 1;
	}
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
		t.s = name;
		t.dereference = 0;
		v.push_back(t);
		call_function(prg, func, v, 0);
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
	std::string name = as_string(prg, v, 1);
	bool readonly = (bool)as_number(prg, v, 2);

	BooBoo_Trigger *trigger;
	
	if (v.size() > 3) {
		trigger = new BooBoo_Trigger(prg, name, as_function(prg, v, 3));
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
	std::string name = as_string(prg, v, 1);
	bool readonly = (bool)as_number(prg, v, 2);
	
	BooBoo_Trigger *trigger;
	
	if (v.size() > 3) {
		trigger = new BooBoo_Trigger(prg, name, as_function(prg, v, 3));
	}
	else {
		trigger = nullptr;
	}
	
	util::JSON::Node *root = shim::shim_json->get_root();
	root->add_nested_string("game>" + name, &var.s, var.s, trigger, readonly);

	return true;
}

static void exprfunc_load_cpa(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v, 0);

	CPA_Info *info = cpa_info(prg);

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v, 1);
	}

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = info->cpa_id;

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
		prg->result.n = -1;
	}
}

static bool cpafunc_set_cpa(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	
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

	int id = as_number(prg, v, 0);
	CPA_Info *info = cpa_info(prg);
	INFO_EXISTS(info->cpas, id)
	delete info->cpas[id]->cpa;
	info->cpas.erase(info->cpas.find(id));

	return true;
}

static void exprfunc_get_audio_properties(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result.set_type(Variable::VECTOR);

	Variable var;
	var.type = Variable::NUMBER;

	var.n = audio::internal::audio_context.device_spec.freq;
	prg->result.v.push_back(var);
	var.n = audio::internal::audio_context.device_spec.channels;
	prg->result.v.push_back(var);
}

static void exprfunc_mml_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	std::string str = as_string(prg, v, 0);
	
       	prg->result.set_type(Variable::NUMBER);

	MML_Info *info = mml_info(prg);

	prg->result.n = info->mml_id;

	Uint8 *bytes = (Uint8 *)str.c_str();
	SDL_IOStream *file = SDL_IOFromMem(bytes, str.length());

	try {
		audio::MML *mml = new audio::MML(file); // this closes the file
		info->mmls[info->mml_id++] = mml;
	}
	catch (util::Error &e) {
		prg->result.n = -1;
	}
}

static bool mmlfunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	MML_Info *info = mml_info(prg);
	INFO_EXISTS(info->mmls, id)
	delete info->mmls[id];
	info->mmls.erase(info->mmls.find(id));

	return true;
}

static void exprfunc_mml_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	MML_Info *info = mml_info(prg);
	INFO_EXISTS(info->mmls, id)
	prg->result.set_type(Variable::NUMBER);
	prg->result.n = audio::samples_to_millis(info->mmls[id]->get_length(), audio::internal::audio_context.device_spec.freq);
}

static void exprfunc_mml_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v, 0);

	MML_Info *info = mml_info(prg);

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v, 1);
	}

       	prg->result.set_type(Variable::NUMBER);

	prg->result.n = info->mml_id;

	try {
		audio::MML *mml = new audio::MML(name, load_from_filesystem);
		info->mmls[info->mml_id++] = mml;
	}
	catch (util::Error &e) {
		prg->result.n = -1;
	}
}

struct MML_Callback_Data
{
	Program *prg;
	int function;
	int id;
};

static void mml_callback(void *data)
{
	MML_Callback_Data *d = static_cast<MML_Callback_Data *>(data);
	std::vector<Token> v;
	Token t;
	t.type = Token::NUMBER;
	t.n = d->id;
	t.dereference = 0;
	v.push_back(t);
	if (booboo::callbacks_enabled) {
		call_function(d->prg, d->function, v, 0);
	}
	delete d;
}

static void exprfunc_mml_play(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int id = as_number(prg, v, 0);

	double volume;
	bool loop;

	if (v.size() > 1) {
		volume = as_number(prg, v, 1);
	}
	else {
		volume = 1.0;
	}

	if (v.size() > 2) {
		loop = as_number(prg, v, 2);
	}
	else {
		loop = false;
	}

	float pan;

	if (v.size() > 3) {
		pan = as_number(prg, v, 3);
	}
	else {
		pan = 0.0f;
	}

	MML_Info *info = mml_info(prg);

	INFO_EXISTS(info->mmls, id)

	audio::MML *mml = info->mmls[id];

	MML_Instance_Info *iinfo = mml_instance_info(prg);

	util::Callback callback = nullptr;
	void *callback_data = nullptr;

	if (v.size() > 4) {
		MML_Callback_Data *d = new MML_Callback_Data;
		d->prg = prg;
		d->function = as_function(prg, v, 4);
		d->id = iinfo->instance_id;
		callback_data = d;
		callback = mml_callback;
	}

	MML_Instance *i = new MML_Instance;
	i->mml = mml;
	i->instance = mml->play(volume, loop, pan, callback, callback_data);

	int inst = iinfo->instance_id;

	iinfo->instances[iinfo->instance_id++] = i;

	// clean up old instances
	for (std::map<int, MML_Instance *>::iterator it = iinfo->instances.begin(); it != iinfo->instances.end();) {
		std::pair<int, MML_Instance *> p = *it;
		if (p.second->mml->track_active(p.second->instance) == false) {
			it = iinfo->instances.erase(it);
		}
		else {
			it++;
		}
	}

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = inst;
}

static void exprfunc_mml_num_tracks(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	MML_Info *info = mml_info(prg);

	INFO_EXISTS(info->mmls, id)

	audio::MML *mml = info->mmls[id];

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = mml->get_num_tracks();
}

static bool mmlfunc_set_sample(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	int id = as_number(prg, v, 0);
	int index = as_number(prg, v, 1);
	int sid = as_number(prg, v, 2);

	MML_Info *info = mml_info(prg);

	INFO_EXISTS(info->mmls, id)

	audio::MML *mml = info->mmls[id];

	Sample_Info *sinfo = sample_info(prg);

	INFO_EXISTS(sinfo->samples, sid)

	audio::Sample *sample = sinfo->samples[sid];

	mml->set_sample(index, sample);

	return true;
}

static bool mmlfunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(id) == info->instances.end()) {
		return true;
	}

	MML_Instance *i = info->instances[id];

	i->mml->stop(i->instance);

	return true;
}

static bool mmlfunc_set_volume(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int inst = as_number(prg, v, 0);
	float vol = as_number(prg, v, 1);

	int track;

	if (v.size() > 2) {
		track = as_number(prg, v, 2);
	}
	else {
		track = -1;
	}

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		return true;
	}

	MML_Instance *i = info->instances[inst];

	i->mml->set_master_volume(i->instance, vol, track);

	return true;
}

static bool mmlfunc_set_pan(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int inst = as_number(prg, v, 0);
	float pan = as_number(prg, v, 1);

	int track;

	if (v.size() > 2) {
		track = as_number(prg, v, 2);
	}
	else {
		track = -1;
	}

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		return true;
	}

	MML_Instance *i = info->instances[inst];

	i->mml->set_pan(i->instance, pan, track);

	return true;
}

static bool mmlfunc_set_tempo(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(2)

	int inst = as_number(prg, v, 0);
	int bpm = as_number(prg, v, 1);

	int track;

	if (v.size() > 2) {
		track = as_number(prg, v, 2);
	}
	else {
		track = -1;
	}

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		return true;
	}

	MML_Instance *i = info->instances[inst];

	i->mml->set_tempo(i->instance, bpm, track);

	return true;
}

static void exprfunc_mml_get_volume(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int inst = as_number(prg, v, 0);
	int track = as_number(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		prg->result.n = 1.0f;
	}
	else {
		MML_Instance *i = info->instances[inst];
		prg->result.n = i->mml->get_master_volume(i->instance, track);
	}
}

static void exprfunc_mml_get_tempo(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int inst = as_number(prg, v, 0);
	int track = as_number(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		prg->result.n = 120;
	}
	else {
		MML_Instance *i = info->instances[inst];
		prg->result.n = i->mml->get_tempo(i->instance, track);
	}
}

static void exprfunc_mml_get_pan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int inst = as_number(prg, v, 0);
	int track = as_number(prg, v, 1);

	prg->result.set_type(Variable::NUMBER);

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		prg->result.n = 0.0f;
	}
	else {
		MML_Instance *i = info->instances[inst];
		prg->result.n = i->mml->get_pan(i->instance, track);
	}
}

static void exprfunc_mml_elapsed(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int inst = as_number(prg, v, 0);

	prg->result.set_type(Variable::NUMBER);

	MML_Instance_Info *info = mml_instance_info(prg);

	if (info->instances.find(inst) == info->instances.end()) {
		prg->result.n = 0;
	}
	else {
		MML_Instance *i = info->instances[inst];
		prg->result.n = audio::samples_to_millis(i->mml->get_elapsed(i->instance), audio::internal::audio_context.device_spec.freq);
	}
}

static void exprfunc_sample_load(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	std::string name = as_string(prg, v, 0);

	Sample_Info *info = sample_info(prg);

	bool load_from_filesystem = false;
	if (v.size() > 1) {
		load_from_filesystem = as_number(prg, v, 1);
	}

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = info->sample_id;

	try {
		audio::Sample *sample = new audio::Sample(name, load_from_filesystem);
		info->samples[info->sample_id++] = sample;
	}
	catch (util::Error &e) {
		prg->result.n = -1;
	}
}

static void exprfunc_sample_create(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(3)

	Variable vec = as_variable_resolve(prg, v, 0);
	int freq = as_number(prg, v, 1);
	int channels = as_number(prg, v, 2);

	Sample_Info *info = sample_info(prg);

	prg->result.set_type(Variable::NUMBER);

	prg->result.n = info->sample_id;

	Uint8 *data = new Uint8[2*vec.v.size()];
	Sint16 *p = (Sint16 *)data;

	for (size_t i = 0; i < vec.v.size(); i++) {
		*p++ = vec.v[i].n;
	}

	try {
		audio::Sample *sample = new audio::Sample(data, 2*vec.v.size(), freq, channels);

		info->samples[info->sample_id++] = sample;
	}
	catch (util::Error &e) {
		prg->result.n = -1;
	}
}

static bool samplefunc_destroy(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);
	Sample_Info *info = sample_info(prg);
	INFO_EXISTS(info->samples, id)
	delete info->samples[id];
	info->samples.erase(info->samples.find(id));

	return true;
}

struct Sample_Callback_Data
{
	Program *prg;
	int function;
	int id;
};

static void sample_callback(void *data)
{
	Sample_Callback_Data *d = static_cast<Sample_Callback_Data *>(data);
	std::vector<Token> v;
	Token t;
	t.type = Token::NUMBER;
	t.n = d->id;
	t.dereference = 0;
	v.push_back(t);
	if (booboo::callbacks_enabled) {
		call_function(d->prg, d->function, v, 0);
	}
	delete d;
}

static bool sample_exists(audio::Sample_Instance *inst)
{
	bool found = false;
	std::vector<audio::Sample_Instance *>::iterator it;
	for (it = audio::internal::audio_context.playing_samples.begin(); it != audio::internal::audio_context.playing_samples.end(); it++) {
		audio::Sample_Instance *s = *it;
		if (s == inst) {
			found = true;
			break;
		}
	}
	return found;
}

static void exprfunc_sample_play(Program *prg, const std::vector<Token> &v)
{
	MIN_ARGS(1)

	int id = as_number(prg, v, 0);

	double volume;
	bool loop;

	if (v.size() > 1) {
		volume = as_number(prg, v, 1);
	}
	else {
		volume = 1.0;
	}

	if (v.size() > 2) {
		loop = as_number(prg, v, 2);
	}
	else {
		loop = false;
	}

	float pan;

	if (v.size() > 3) {
		pan = as_number(prg, v, 3);
	}
	else {
		pan = 0.0f;
	}

	Sample_Info *info = sample_info(prg);

	INFO_EXISTS(info->samples, id)

	audio::Sample *sample = info->samples[id];

	int millis;

	if (v.size() > 4) {
		millis = as_number(prg, v, 4);
	}
	else {
		millis = audio::samples_to_millis(sample->get_length(), sample->get_frequency());
	}

	Sample_Instance_Info *iinfo = sample_instance_info(prg);

	util::Callback callback = nullptr;
	void *callback_data = nullptr;

	if (v.size() > 5) {
		Sample_Callback_Data *d = new Sample_Callback_Data;
		d->prg = prg;
		d->function = as_function(prg, v, 5);
		d->id = iinfo->instance_id;
		callback_data = d;
		callback = sample_callback;
	}

	audio::Sample_Instance *inst = sample->play_stretched(volume, 0, audio::millis_to_samples(millis), loop, pan, callback, callback_data);

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = iinfo->instance_id;

	iinfo->instances[iinfo->instance_id++] = inst;

	// clean up old instances
	audio::lock_mutex();
	for (std::map<int, audio::Sample_Instance *>::iterator it = iinfo->instances.begin(); it != iinfo->instances.end();) {
		std::pair<int, audio::Sample_Instance *> p = *it;
		bool found = false;
		std::vector<audio::Sample_Instance *>::iterator it2;
		for (it2 = audio::internal::audio_context.playing_samples.begin(); it2 != audio::internal::audio_context.playing_samples.end(); it2++) {
			audio::Sample_Instance *s2 = *it2;
			if (s2 == p.second) {
				found = true;
				break;
			}
		}
		if (found) {
			it++;
		}
		else {
			it = iinfo->instances.erase(it);
		}
	}
	audio::unlock_mutex();
}

static bool samplefunc_seek(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	int millis = as_number(prg, v, 1);

	Sample_Instance_Info *info = sample_instance_info(prg);

	if (info->instances.find(id) == info->instances.end()) {
		return true;
	}

	audio::lock_mutex();

	bool found = sample_exists(info->instances[id]);

	if (found) {
		info->instances[id]->offset = audio::millis_to_samples(millis, audio::internal::audio_context.device_spec.freq);
	}

	audio::unlock_mutex();

	return true;
}

static void exprfunc_sample_length(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	Sample_Info *info = sample_info(prg);

	INFO_EXISTS(info->samples, id)

	audio::Sample *sample = info->samples[id];

	int millis = audio::samples_to_millis(sample->get_length(), sample->get_frequency());

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = millis;
}

static void exprfunc_sample_elapsed(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	Sample_Instance_Info *info = sample_instance_info(prg);

	prg->result.set_type(Variable::NUMBER);

	audio::lock_mutex();

	bool found = sample_exists(info->instances[id]);

	if (found) {
		prg->result.n = audio::samples_to_millis(info->instances[id]->offset, audio::internal::audio_context.device_spec.freq);
	}
	else {
		prg->result.n = 0;
	}

	audio::unlock_mutex();
}

static void exprfunc_sample_get_volume(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	Sample_Instance_Info *info = sample_instance_info(prg);

	prg->result.set_type(Variable::NUMBER);

	audio::lock_mutex();

	bool found = sample_exists(info->instances[id]);

	if (found) {
		prg->result.n = info->instances[id]->volume;
	}
	else {
		prg->result.n = 1.0f;
	}

	audio::unlock_mutex();
}

static void exprfunc_sample_get_pan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	Sample_Instance_Info *info = sample_instance_info(prg);

	prg->result.set_type(Variable::NUMBER);

	audio::lock_mutex();

	bool found = sample_exists(info->instances[id]);

	if (found) {
		prg->result.n = info->instances[id]->pan;
	}
	else {
		prg->result.n = 0.0f;
	}

	audio::unlock_mutex();
}

static bool samplefunc_stop(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int id = as_number(prg, v, 0);

	Sample_Instance_Info *info = sample_instance_info(prg);

	if (info->instances.find(id) == info->instances.end()) {
		return true;
	}

	audio::Sample::stop_instance(info->instances[id]);

	return true;
}

static bool samplefunc_set_volume(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	float vol = as_number(prg, v, 1);

	Sample_Instance_Info *info = sample_instance_info(prg);

	audio::lock_mutex();

	bool found = sample_exists(info->instances[id]);

	if (found) {
		info->instances[id]->volume = vol;
	}

	audio::unlock_mutex();

	return true;
}

static bool samplefunc_set_pan(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	int id = as_number(prg, v, 0);
	float pan = as_number(prg, v, 1);

	Sample_Instance_Info *info = sample_instance_info(prg);

	audio::lock_mutex();

	bool found = sample_exists(info->instances[id]);

	if (found) {
		info->instances[id]->pan = pan;
	}

	audio::unlock_mutex();

	return true;
}

static bool miscfunc_inspect(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	char buf[1000];

	if (v[0].type == Token::NUMBER) {
		snprintf(buf, 1000, "%g", v[0].n);
	}
	else if (v[0].type == Token::SYMBOL) {
		Variable &var = get_variable(prg, v[0].i);
		if (IS_NUMBER(var)) {
			snprintf(buf, 1000, "%g", var.n);
		}
		else if (IS_STRING(var)) {
			snprintf(buf, 1000, "%s", var.s.c_str());
		}
		else if (IS_VECTOR(var)) {
			snprintf(buf, 1000, "-vector-");
		}
		else if (IS_MAP(var)) {
			snprintf(buf, 1000, "-map-");
		}
		else if (IS_FUNCTION(var)) {
			snprintf(buf, 1000, "-function-");
		}
		else if (IS_LABEL(var)) {
			snprintf(buf, 1000, "-label-");
		}
		else if (IS_POINTER(var)) {
			snprintf(buf, 1000, "-pointer-");
		}
	}
	else {
		strcpy_s(buf, 1000, "Unknown");
	}

	gui::popup("INSPECTOR", buf, gui::OK);

	return true;
}

static bool miscfunc_delay(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(1)

	int millis = (int)as_number(prg, v, 0);
	SDL_Delay(millis);
	return true;
}

static void exprfunc_misc_get_ticks(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result.set_type(Variable::NUMBER);
	prg->result.n = SDL_GetTicks();
}

static bool miscfunc_die(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(2)

	Variable &expr = prg->variables[v[0].i];
	std::string msg = as_string(prg, v, 1);

	CHECK_EXPRESSION(expr)

	evaluate_expression(prg, expr.e);

	if (prg->result.n == 0) {
	Variable &expr = prg->variables[v[2].i];
		gui::popup("Error", msg, gui::OK);
		exit(1);
	}

	return true;	
}

static void exprfunc_misc_file_list(Program *prg, const std::vector<Token> &v)
{
	COUNT_ARGS(0)

	prg->result.set_type(Variable::VECTOR);

	std::vector<std::string> l = shim::cpa->get_all_filenames();

	for (size_t i = 0; i < l.size(); i++) {
		Variable var;
		var.type = Variable::STRING;
		var.s = l[i];
		prg->result.v.push_back(var);
	}
}

// This stdio to SDL_IOStream wrapper is from the SDL3 migration README

typedef struct IOStreamStdioFPData
{
    FILE *fp;
    bool autoclose;
} IOStreamStdioFPData;

static Sint64 SDLCALL stdio_seek(void *userdata, Sint64 offset, SDL_IOWhence whence)
{
    FILE *fp = ((IOStreamStdioFPData *) userdata)->fp;
    int stdiowhence;

    switch (whence) {
    case SDL_IO_SEEK_SET:
        stdiowhence = SEEK_SET;
        break;
    case SDL_IO_SEEK_CUR:
        stdiowhence = SEEK_CUR;
        break;
    case SDL_IO_SEEK_END:
        stdiowhence = SEEK_END;
        break;
    default:
        SDL_SetError("Unknown value for 'whence'");
        return -1;
    }

    if (fseek(fp, (long)offset, stdiowhence) == 0) {
        const Sint64 pos = ftell(fp);
        if (pos < 0) {
            SDL_SetError("Couldn't get stream offset");
            return -1;
        }
        return pos;
    }
    SDL_SetError("Couldn't seek in stream");
    return -1;
}

static size_t SDLCALL stdio_read(void *userdata, void *ptr, size_t size, SDL_IOStatus *status)
{
    FILE *fp = ((IOStreamStdioFPData *) userdata)->fp;
    const size_t bytes = fread(ptr, 1, size, fp);
    if (bytes == 0 && ferror(fp)) {
        SDL_SetError("Couldn't read stream");
    }
    return bytes;
}

static size_t SDLCALL stdio_write(void *userdata, const void *ptr, size_t size, SDL_IOStatus *status)
{
    FILE *fp = ((IOStreamStdioFPData *) userdata)->fp;
    const size_t bytes = fwrite(ptr, 1, size, fp);
    if (bytes == 0 && ferror(fp)) {
        SDL_SetError("Couldn't write stream");
    }
    return bytes;
}

static bool SDLCALL stdio_close(void *userdata)
{
    IOStreamStdioFPData *rwopsdata = (IOStreamStdioFPData *) userdata;
    bool status = true;
    if (rwopsdata->autoclose) {
        if (fclose(rwopsdata->fp) != 0) {
            SDL_SetError("Couldn't close stream");
            status = false;
        }
    }
    return status;
}

SDL_IOStream *SDL_RWFromFP(FILE *fp, bool autoclose)
{
    SDL_IOStreamInterface iface;
    IOStreamStdioFPData *rwopsdata;
    SDL_IOStream *rwops;

    rwopsdata = (IOStreamStdioFPData *) SDL_malloc(sizeof (*rwopsdata));
    if (!rwopsdata) {
        return NULL;
    }

    SDL_INIT_INTERFACE(&iface);
    /* There's no stdio_size because SDL_GetIOSize emulates it the same way we'd do it for stdio anyhow. */
    iface.seek = stdio_seek;
    iface.read = stdio_read;
    iface.write = stdio_write;
    iface.close = stdio_close;

    rwopsdata->fp = fp;
    rwopsdata->autoclose = autoclose;

    rwops = SDL_OpenIO(&iface, rwopsdata);
    if (!rwops) {
        iface.close(rwopsdata);
    }
    return rwops;
}

void start_lib_standard()
{
	File_Info *info = file_info(prg);
	SDL_IOStream *in = SDL_RWFromFP(stdin, false);
	SDL_IOStream *out = SDL_RWFromFP(stdout, false);
	SDL_IOStream *err = SDL_RWFromFP(stderr, false);
	info->files[info->file_id++] = in;
	info->files[info->file_id++] = out;
	info->files[info->file_id++] = err;

	add_expression_handler("getenv", exprfunc_getenv);
	add_expression_handler("getcwd", exprfunc_getcwd);
	add_expression_handler("list_directory", exprfunc_list_directory);
	add_instruction("print", corefunc_print);
	add_expression_handler("input", exprfunc_input);
	add_expression_handler("mkdir", exprfunc_mkdir);
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
	add_expression_handler("string_find", exprfunc_string_find);
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
	add_expression_handler("vector_it_start", exprfunc_vector_it_start);
	add_expression_handler("vector_it_end", exprfunc_vector_it_end);
	add_expression_handler("vector_it_get", exprfunc_vector_it_get);
	add_expression_handler("vector_it_erase", exprfunc_vector_it_erase);
	add_expression_handler("vector_it_inc", exprfunc_vector_it_inc);

	add_instruction("map_clear", mapfunc_clear);
	add_instruction("map_erase", mapfunc_erase);
	add_expression_handler("map_keys", exprfunc_map_keys);

	add_expression_handler("file_open", exprfunc_file_open);
	add_expression_handler("file_open_cpa", exprfunc_file_open_cpa);
	add_instruction("file_close", filefunc_close);
	add_expression_handler("file_read", exprfunc_file_read);
	add_expression_handler("file_read_line", exprfunc_file_read_line);
	add_expression_handler("file_read_byte", exprfunc_file_read_byte);
	add_expression_handler("file_write_byte", exprfunc_file_write_byte);
	add_expression_handler("file_write", exprfunc_file_write);
	add_expression_handler("file_print", exprfunc_file_print);
	add_expression_handler("file_eof", exprfunc_file_eof);
	add_expression_handler("file_tell", exprfunc_file_tell);
	add_expression_handler("file_seek", exprfunc_file_seek);
	
	add_instruction("text_fore", twinklefunc_text_fore);
	add_instruction("text_back", twinklefunc_text_back);
	add_instruction("text_reset", twinklefunc_reset);
	add_expression_handler("getch", exprfunc_twinkle_getch);
	add_expression_handler("kbhit", exprfunc_twinkle_kbhit);
	add_expression_handler("get_console_size", exprfunc_twinkle_get_console_size);
	add_instruction("text_set_cursor_pos", twinklefunc_set_cursor_pos);
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
	add_expression_handler("get_audio_properties", exprfunc_get_audio_properties);
	add_expression_handler("mml_create", exprfunc_mml_create);
	add_expression_handler("mml_load", exprfunc_mml_load);
	add_expression_handler("mml_length", exprfunc_mml_length);
	add_instruction("mml_destroy", mmlfunc_destroy);
	add_expression_handler("mml_play", exprfunc_mml_play);
	add_expression_handler("mml_num_tracks", exprfunc_mml_num_tracks);
	add_instruction("mml_set_sample", mmlfunc_set_sample);
	add_instruction("mml_stop", mmlfunc_stop);
	add_instruction("mml_set_volume", mmlfunc_set_volume);
	add_instruction("mml_set_pan", mmlfunc_set_pan);
	add_instruction("mml_set_tempo", mmlfunc_set_tempo);
	add_expression_handler("mml_get_volume", exprfunc_mml_get_volume);
	add_expression_handler("mml_get_tempo", exprfunc_mml_get_tempo);
	add_expression_handler("mml_get_pan", exprfunc_mml_get_pan);
	add_expression_handler("mml_elapsed", exprfunc_mml_elapsed);
	add_expression_handler("sample_load", exprfunc_sample_load);
	add_expression_handler("sample_create", exprfunc_sample_create);
	add_instruction("sample_destroy", samplefunc_destroy);
	add_expression_handler("sample_play", exprfunc_sample_play);
	add_instruction("sample_stop", samplefunc_stop);
	add_instruction("sample_set_volume", samplefunc_set_volume);
	add_instruction("sample_set_pan", samplefunc_set_pan);
	add_instruction("sample_seek", samplefunc_seek);
	add_expression_handler("sample_length", exprfunc_sample_length);
	add_expression_handler("sample_elapsed", exprfunc_sample_elapsed);
	add_expression_handler("sample_get_volume", exprfunc_sample_get_volume);
	add_expression_handler("sample_get_pan", exprfunc_sample_get_pan);
	add_instruction("inspect", miscfunc_inspect);
	add_instruction("delay", miscfunc_delay);
	add_expression_handler("get_ticks", exprfunc_misc_get_ticks);
	add_expression_handler("file_list", exprfunc_misc_file_list);
	add_instruction("die", miscfunc_die);
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
	MML_Instance_Info *mml_instance_i = mml_instance_info(prg);
	for (std::map<int, MML_Instance *>::iterator i = mml_instance_i->instances.begin(); i != mml_instance_i->instances.end(); i++) {
		delete mml_instance_i->instances[(*i).first];
	}
	MML_Info *mml_i = mml_info(prg);
	for (std::map<int, audio::MML *>::iterator i = mml_i->mmls.begin(); i != mml_i->mmls.end();) {
		audio::MML *mml = mml_i->mmls[(*i).first];
		i = mml_i->mmls.erase(i);
		delete mml;
	}
	Sample_Info *sample_i = sample_info(prg);
	for (std::map<int, audio::Sample *>::iterator i = sample_i->samples.begin(); i != sample_i->samples.end(); i++) {
		delete sample_i->samples[(*i).first];
	}
	Sample_Instance_Info *sample_instance_i = sample_instance_info(prg);
	delete file_i;
	delete json_i;
	delete cpa_i;
	delete mml_i;
	delete mml_instance_i;
	delete sample_i;
	delete sample_instance_i;

	CFG_Info *cfg_i = cfg_info(prg);
	delete cfg_i;

	set_black_box("com.nooskewl.booboo.files", nullptr);
	set_black_box("com.nooskewl.booboo.cfg", nullptr);
	set_black_box("com.nooskewl.booboo.json", nullptr);
	set_black_box("com.nooskewl.booboo.cpa", nullptr);
	set_black_box("com.nooskewl.booboo.mml", nullptr);
	set_black_box("com.nooskewl.booboo.mml_instance", nullptr);
	set_black_box("com.nooskewl.booboo.sample", nullptr);
	set_black_box("com.nooskewl.booboo.sample_instance", nullptr);
}
