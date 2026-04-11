#define SDL_MAIN_HANDLED 1

#include <climits>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

#include <shim5/shim5.h>

using namespace noo;

#include "booboo/booboo.h"
#include "booboo/standard_lib.h"
#include "booboo/internal.h"

int main(int argc, char **argv)
{
	for (int i = 0 ; i < argc; i++) {
		booboo::cli_args.push_back(argv[i]);
	}

	shim::organisation_name = "Nooskewl";
	shim::game_name = "BooBoo";

	booboo::load_text = util::load_text_from_filesystem;

	std::string fn = argc >= 2 ? argv[1] : "";

	if (fn != "") {
		struct stat s;
		if (stat(fn.c_str(), &s) == 0 && (s.st_mode & S_IFMT) == S_IFDIR) {
			chdir(fn.c_str());
			fn = "";
		}
		else {
			int pos = fn.length()-1;
#ifdef _WIN32
			while (pos > 0 && (fn[pos] != '/' && fn[pos] != '\\')) {
#else
			while (pos > 0 && fn[pos] != '/') {
#endif
				pos--;
			}
#ifdef _WIN32
			if (fn[pos] == '/' || fn[pos] == '\\') {
#else
			if (fn[pos] == '/') {
#endif
				chdir(fn.substr(0, pos).c_str());
				fn = fn.substr(pos+1);
			}
		}
	}

	try {

	booboo::start();
	start_lib_standard();
	
	int obfuscate_index = util::check_args(argc, argv, "+no-obfuscate");
	if (obfuscate_index > 0) {
		std::string filename = std::string(argv[obfuscate_index+1]);
		std::fstream *f = new std::fstream;
		f->open(filename, std::fstream::in);
		std::string s;
		while (!f->eof()) {
			(*f) >> s;
			if (s != "") {
				booboo::add_special_function(s);
			}
		}
		f->close();
	}

	std::string dlls = util::load_text_from_filesystem("dll.txt");
	util::Tokenizer tok(dlls, '\n');
	std::string dll;
	while ((dll = tok.next()) != "") {
		dll = util::trim(dll);
#ifdef _WIN32
		dll += ".dll";
		HMODULE m = LoadLibrary(dll.c_str());
		if (m != NULL) {
			BOOBOO_DLL_START_FUNC func = (BOOBOO_DLL_START_FUNC)GetProcAddress(m, "booboo_start");
			if (func != NULL) {
				(*func)();
			}
		}
#else
		dll = "lib" + dll + ".so";
		char buf[4096];
		getcwd(buf, 4096);
		dll = std::string(buf) + "/" + dll;
		void *handle = dlopen(dll.c_str(), RTLD_LAZY);
		if (handle != nullptr) {
			BOOBOO_DLL_START_FUNC func = (BOOBOO_DLL_START_FUNC)dlsym(handle, "booboo_start");
			if (func != nullptr) {
				(*func)();
			}
		}
#endif
	}

again:
	booboo::quit = false;
	bool was_reset = false;

	if (booboo::reset_game_name != "") {
		fn = booboo::reset_game_name;
		booboo::reset_game_name = "";
		was_reset = true;
	}

	std::string code;

	if (was_reset) {
		try {
			code = util::load_text_from_filesystem(fn);
		}
		catch (booboo::Error &e) {
			printf("Program is missing or corrupt!\n");
			exit(1);
		}

		booboo::main_program_name = fn;
	}
	else {
		if (fn != "") {
			try {
				code = util::load_text_from_filesystem(fn);
			}
			catch (booboo::Error &e) {
				printf("Program is missing or corrupt!\n");
				exit(1);
			}

			booboo::main_program_name = fn;
		}
		else {
			try {
				code = util::load_text_from_filesystem("main.boo");
			}
			catch (booboo::Error &e) {
				printf("Program is missing or corrupt!\n");
				exit(1);
			}
			
			booboo::main_program_name = "main.boo";
		}
	}

	booboo::prg = booboo::create_program(code);

	bool ob = false;
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "+obfuscate") {
			ob = true;
			break;
		}
	}

	if (ob) {
		booboo::obfuscate(booboo::prg);
	}
	else {
		while (booboo::interpret(booboo::prg)) {
		}
	}

	standard_lib_destroy_program(booboo::prg);
	booboo::destroy_program(booboo::prg);

	if (booboo::reset_game_name != "") {
		fn = "";
		goto again;
	}

	end_lib_standard();
	booboo::end();

	}
	catch (booboo::Error &e) {
		printf("%s\n", e.error_message.c_str());
	}

	return booboo::return_code;
}
