#define SDL_MAIN_HANDLED 1

#include <climits>
#include <iostream>
#include <fstream>

#include <sys/stat.h>

#include <shim5/shim5.h>
#include <shim5/internal/audio.h>
using namespace noo;

#include "booboo/booboo.h"
#include "booboo/standard_lib.h"
#include "booboo/internal.h"

bool ctrl_c_handler(int signal) {

    if (signal == CTRL_C_EVENT) {
	    exit(0);
    }
    return true;
}

int main(int argc, char **argv)
{
	booboo::callbacks_enabled = true;

	for (int i = 0 ; i < argc; i++) {
		booboo::cli_args.push_back(argv[i]);
	}

	shim::organisation_name = "Nooskewl";
	shim::game_name = "BooBoo";

	if (shim::static_start_all(SDL_INIT_AUDIO) == false) {
		return 1;
	}

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_c_handler, TRUE);

	shim::argc = argc;
	shim::argv = argv;
	shim::error_level = 3;
	shim::log_tags = false;

	if (util::start() == false || audio::start() == false) {
		return 1;
	}

	booboo::load_text = util::load_text_from_filesystem;

	std::string fn = argc >= 2 ? argv[1] : "";

	if (fn != "") {
		struct stat s;
		if (stat(fn.c_str(), &s) == 0 && (s.st_mode & S_IFMT) == S_IFDIR) {
			_chdir(fn.c_str());
			fn = "";
		}
		else {
			int pos = fn.length()-1;
			while (pos > 0 && (fn[pos] != '/' && fn[pos] != '\\')) {
				pos--;
			}
			if (fn[pos] == '/' || fn[pos] == '\\') {
				_chdir(fn.substr(0, pos).c_str());
				fn = fn.substr(pos+1);
			}
		}
	}

	try {

	booboo::start();
	start_lib_standard();
	
	std::string dlls = util::load_text_from_filesystem("dll.txt");
	util::Tokenizer tok(dlls, '\n');
	std::string dll;
	while ((dll = tok.next()) != "") {
		dll = util::trim(dll);
		dll += ".dll";
		HMODULE m = LoadLibraryA(dll.c_str());
		if (m != NULL) {
			BOOBOO_DLL_START_FUNC func = (BOOBOO_DLL_START_FUNC)GetProcAddress(m, "booboo_start");
			if (func != NULL) {
				(*func)();
			}
		}
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

	if (util::bool_arg(false, argc, argv, "debug")) {
		shim::debug = true;
		booboo::debug("Debugging '" + booboo::main_program_name + "'... Type 'help' for help...");
	}

	while (booboo::interpret(booboo::prg)) {
		audio::lock_mutex();
		int sz = audio::internal::audio_callbacks.size();
		for (int i = 0; i < sz; i++) {
			util::Callback cb = audio::internal::audio_callbacks.back();
			void *d = audio::internal::audio_callback_data.back();
			cb(d);
			audio::internal::audio_callbacks.pop_back();
			audio::internal::audio_callback_data.pop_back();
		}
		audio::unlock_mutex();
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
