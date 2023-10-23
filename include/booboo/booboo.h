#ifndef BOOBOO_H
#define BOOBOO_H

#include <string>
#include <vector>
#include <map>

namespace booboo {

class Program;

class Error {
public:
	Error();
	Error(std::string error_message);
	virtual ~Error();
	
	std::string error_message;
};

struct Variable
{
	enum Variable_Type {
		NUMBER = 0,
		STRING,
		VECTOR,
		LABEL,
		FUNCTION,
		POINTER
	} type;

	std::string name;

	double n;
	std::string s;
	std::vector<Variable> v;
	Variable *p;
};

struct Token {
	enum Token_Type {
		STRING = 0,
		SYMBOL, // alphanumeric and underscores like variable names
		NUMBER
	};

	Token_Type type;

	std::string s;
	double n;
	int i;

	std::string token;
};

// These are for adding syntax
typedef bool (*library_func)(Program *prg, std::vector<Token> &v);

typedef std::string (*token_func)(Program *);

// Call these before/after using BooBoo
void start();
void end();

// These are the how you create, destroy and run programs
Program *create_program(std::string code);
void destroy_program(Program *prg);
bool interpret(Program *prg);

// Functions calling
void call_function(Program *prg, int function, std::vector<Token> &params, Variable &result, int ignore_params = 0);
void call_function(Program *prg, std::string function, std::vector<Token> &params, Variable &result, int ignore_params = 0);
void call_void_function(Program *prg, int function, std::vector<Token> &params, int ignore_params = 0);
void call_void_function(Program *prg, std::string function, std::vector<Token> &params, int ignore_params = 0);

// To create token vectors for calling functions
Token token_number(std::string token, double n);
Token token_string(std::string token, std::string s);

// For dealing with results
double get_number(Variable &v);
std::string get_string(Variable &v);
std::vector<Variable> get_vector(Variable &v);

// Add a library function
void add_syntax(std::string name, library_func func);
void add_token(char token, token_func func);

// For error handling
int get_line_num(Program *prg);
std::string get_file_name(Program *prg);
std::string get_error_info(Program *prg);

// These are helpful within your own library functions
Variable &as_variable(Program *prg, Token &t);
double as_number(Program *prg, Token &t);
std::string as_string(Program *prg, Token &t);
int as_label(Program *prg, Token &t);
int as_function(Program *prg, Token &t);

// This one adds basic syntax like arithmetic and vector manipulation
void start_lib_basics();

// The black box allows you to store anything you want
void *get_black_box(Program *prg, std::string id);
void set_black_box(Program *prg, std::string id, void *data);

// If you have a variable of type SYMBOL then 'i' is the index you pass here to retrive the variable
Variable &get_variable(Program *prg, int index);

// This stuff can be used but it's used by the BooBoo interpreter
extern std::string reset_game_name;
extern std::string main_program_name;
extern int return_code;
extern bool quit;

} // End namespace booboo

// You can use this at the start of your library functions to ensure correct number of arguments
// (if you are not using variable number of arguments)
#define COUNT_ARGS(n) if (v.size() != n) throw Error(std::string(__FUNCTION__) + ": " + "Incorrect number of arguments at " + get_error_info(prg));

#endif // BOOBOO_H
