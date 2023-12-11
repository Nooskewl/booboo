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

struct Variable
{
	enum Variable_Type {
		NUMBER = 0,
		STRING,
		VECTOR,
		MAP,
		LABEL,
		FUNCTION,
		POINTER,
		EXPRESSION,
		FISH
	} type;

	struct Expression
	{
		int i;
		std::string name;
		std::vector<Token> v;
	};

	struct Fish
	{
		int c_i;
		std::vector<Token> v;
	};

	std::string name;

	double n;
	std::string s;
	std::vector<Variable> v;
	std::map<std::string, Variable> m;
	Variable *p;
	Expression e;
	Fish f;
};

typedef bool (*library_func)(Program *prg, std::vector<Token> &v);
typedef std::string (*token_func)(Program *);
typedef double (*expression_func)(Program *prg, std::vector<Token> &v);

// Call these before/after using BooBoo
void start();
void end();

// These are the how you create, destroy and run programs
Program *create_program(std::string code);
void destroy_program(Program *prg);
bool interpret(Program *prg, int instructions);

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
void add_instruction(std::string name, library_func func);
// Add a token handler
void add_token_handler(char token, token_func func);
// Handler for things within an expression (parenthesis)
void add_expression_handler(std::string name, expression_func func);

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
// You can comment out the first line and uncomment the second to skip error
// checking and get a small speed improvement
#define MIN_ARGS(n) if (v.size() < n) throw Error(std::string(__FUNCTION__) + ": " + "Incorrect number of arguments at " + get_error_info(prg));
//#define MIN_ARGS(n)
#define COUNT_ARGS(n) if (v.size() != n) throw Error(std::string(__FUNCTION__) + ": " + "Incorrect number of arguments at " + get_error_info(prg));
//#define COUNT_ARGS(n)

#endif // BOOBOO_H
