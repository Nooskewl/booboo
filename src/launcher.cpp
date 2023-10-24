#if defined __linux__ || defined __LP64__
#include <unistd.h>
#endif

#include <sys/stat.h>

#include "booboo/booboo.h"

booboo::Program *prg;

static std::string load_text_from_filesystem(std::string filename)
{
	int _sz;
	struct stat st;
	int r = stat(filename.c_str(), &st);
	if (r == 0) {
		_sz = st.st_size;
	}
	else {
		throw booboo::Error("Error getting file size: " + filename);
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

int main(int argc, char **argv)
{
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
			code = load_text_from_filesystem(fn);
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
				code = load_text_from_filesystem(fn);
			}
			catch (booboo::Error &e) {
				printf("Program is missing or corrupt!\n");
				exit(1);
			}

			booboo::main_program_name = fn;
		}
		else {
			try {
				code = load_text_from_filesystem("main.boo");
			}
			catch (booboo::Error &e) {
				printf("Program is missing or corrupt!\n");
				exit(1);
			}
			
			booboo::main_program_name = "main.boo";
		}
	}

	prg = booboo::create_program(code);
	
	// This does one token at a time, and we want it to finish the main body so it's a loop
	while (booboo::interpret(prg)) {
	}

#ifdef DUMP
	printf("main:\n");
	for (size_t i = 0; i < prg->program.size(); i++) {
		booboo::Statement &s = prg->program[i];
		printf("%d ", s.method);
		for (size_t j = 0; j < s.data.size(); j++) {
			printf("%s ", s.data[j].token.c_str());
		}
		printf("\n");
	}
	std::map<std::string, booboo::Program>::iterator it;
	for (it = prg->functions.begin(); it != prg->functions.end(); it++) {
		booboo::Program prg = (*it).second;
		printf("%s:\n", (*it).first.c_str());
		for (size_t i = 0; i < prg->program.size(); i++) {
			booboo::Statement &s = prg->program[i];
			printf("%d ", s.method);
			for (size_t j = 0; j < s.data.size(); j++) {
				printf("%s ", s.data[j].token.c_str());
			}
			printf("\n");
		}
		std::map<std::string, booboo::Label>::iterator it2;
		for (it2 = prg->labels.begin(); it2 != prg->labels.end(); it2++) {
			std::string label = (*it2).first;
			printf("label %s %d\n", label.c_str(), (*it2).second.pc);
		}
	}
#endif
	
	booboo::destroy_program(prg);

	if (booboo::reset_game_name != "") {
		fn = "";
		goto again;
	}

	booboo::end();

	}
	catch (booboo::Error &e) {
		printf("%s\n", e.error_message.c_str());
	}

	return booboo::return_code;
}
