#ifndef BOOBOO_H
#define BOOBOO_H

#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

#ifdef _WIN32
#ifdef BOOBOO_STATIC
#define BOOBOO_EXPORT
#else
#ifdef BOOBOO_LIB_BUILD
#define BOOBOO_EXPORT __declspec(dllexport)
#else
#define BOOBOO_EXPORT __declspec(dllimport)
#endif
#endif
#else
#ifdef BOOBOO_LIB_BUILD
#define BOOBOO_EXPORT __attribute__((visibility("default")))
#else
#define BOOBOO_EXPORT
#endif
#endif

namespace booboo {

class Program;

class BOOBOO_EXPORT Error {
public:
	BOOBOO_EXPORT Error();
	BOOBOO_EXPORT Error(std::string error_message);
	BOOBOO_EXPORT virtual ~Error();
	
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
	union {
		double n;
		int i;
	};

	std::string token;

	int dereference;
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
		EXPRESSION,
		FISH,
		POINTER,
		UNTYPED
	} type;

	struct Expression
	{
		int i;
		std::string name;
		std::vector<Token> v;
		int dereference;

		Expression() {
		}

		Expression(const Expression &e) {
			i = e.i;
			name = e.name;
			v = e.v;
		}

		~Expression() {
		}
	};

	struct Fish
	{
		int c_i;
		std::vector<Token> v;
		int dereference;

		Fish() {
		}

		Fish(const Fish &f) {
			c_i = f.c_i;
			v = f.v;
		}

		~Fish() {
		}
	};

	std::string name;

	double n;
	Variable *p;
	std::string s;
	std::vector<Variable> v;
	std::map<std::string, Variable> m;
	Expression e;
	Fish f;

	std::string obfuscated;

	bool constant;

	BOOBOO_EXPORT bool operator==(const Variable &var) const;

	BOOBOO_EXPORT Variable(const Variable &var);

	BOOBOO_EXPORT Variable();

	BOOBOO_EXPORT ~Variable();
};

typedef bool (*library_func)(Program *prg, const std::vector<Token> &v);
typedef std::string (*token_func)(Program *);
typedef Variable (*expression_func)(Program *prg, const std::vector<Token> &v);

// Call these before/after using BooBoo
void BOOBOO_EXPORT start();
void BOOBOO_EXPORT end();

// These are the how you create, destroy and run programs
Program BOOBOO_EXPORT *create_program(std::string code);
void BOOBOO_EXPORT destroy_program(Program *prg);
bool BOOBOO_EXPORT interpret(Program *prg);

// Special functions are left unobfuscated
void BOOBOO_EXPORT add_special_function(std::string name);

// Functions calling
void BOOBOO_EXPORT call_function(Program *prg, int function, const std::vector<Token> &params, Variable &result, int ignore_params = 0);
void BOOBOO_EXPORT call_function(Program *prg, std::string function, const std::vector<Token> &params, Variable &result, int ignore_params = 0);
void BOOBOO_EXPORT call_void_function(Program *prg, int function, const std::vector<Token> &params, int ignore_params = 0);
void BOOBOO_EXPORT call_void_function(Program *prg, std::string function, const std::vector<Token> &params, int ignore_params = 0);

// To create token vectors for calling functions
Token BOOBOO_EXPORT token_number(std::string token, double n);
Token BOOBOO_EXPORT token_string(std::string token, std::string s);

// For dealing with results
double BOOBOO_EXPORT get_number(Variable &v);
std::string BOOBOO_EXPORT get_string(Variable &v);
std::vector<Variable> BOOBOO_EXPORT get_vector(Variable &v);

// Add a library function
void BOOBOO_EXPORT add_instruction(std::string name, library_func func);
// Add a token handler
void BOOBOO_EXPORT add_token_handler(char token, token_func func);
// Handler for things within an expression (parenthesis)
void BOOBOO_EXPORT add_expression_handler(std::string name, expression_func func);

// For error handling
int BOOBOO_EXPORT get_line_num(Program *prg);
std::string BOOBOO_EXPORT get_file_name(Program *prg);
std::string BOOBOO_EXPORT get_error_info(Program *prg);

// These are helpful within your own library functions
Variable BOOBOO_EXPORT &as_variable(Program *prg, const Token &t);
Variable BOOBOO_EXPORT as_variable_resolve(Program *prg, const Token &t);
double BOOBOO_EXPORT as_number(Program *prg, const Token &t);
std::string BOOBOO_EXPORT as_string(Program *prg, const Token &t);
int BOOBOO_EXPORT as_label(Program *prg, const Token &t);
int BOOBOO_EXPORT as_function(Program *prg, const Token &t);
Variable BOOBOO_EXPORT as_pointer(Program *prg, const Token &t);

// The black box allows you to store anything you want
void BOOBOO_EXPORT *get_black_box(Program *prg, std::string id);
void BOOBOO_EXPORT set_black_box(Program *prg, std::string id, void *data);

void BOOBOO_EXPORT obfuscate(Program *prg);

// If you have a variable of type SYMBOL then 'i' is the index you pass here to retrive the variable
Variable BOOBOO_EXPORT &get_variable(Program *prg, int index);

Variable BOOBOO_EXPORT evaluate_expression(Program *prg, const Variable::Expression &e);
Variable BOOBOO_EXPORT &go_fish(Program *prg, const Variable::Fish &f);

extern BOOBOO_EXPORT Variable *dereference(Program *prg, const Token &t);

// This stuff can be used but it's used by the BooBoo interpreter
extern BOOBOO_EXPORT std::string reset_game_name;
extern BOOBOO_EXPORT std::string main_program_name;
extern BOOBOO_EXPORT int return_code;
extern BOOBOO_EXPORT bool quit;
extern BOOBOO_EXPORT std::string (*load_text)(std::string filename); // must be set
extern BOOBOO_EXPORT Program *prg;

} // End namespace booboo

#define IS_NUMBER(v) ((v).type == Variable::NUMBER)
#define IS_STRING(v) ((v).type == Variable::STRING)
#define IS_VECTOR(v) ((v).type == Variable::VECTOR)
#define IS_MAP(v) ((v).type == Variable::MAP)
#define IS_LABEL(v) ((v).type == Variable::LABEL)
#define IS_FUNCTION(v) ((v).type == Variable::FUNCTION)
#define IS_EXPRESSION(v) ((v).type == Variable::EXPRESSION)
#define IS_FISH(v) ((v).type == Variable::FISH)
#define IS_POINTER(v) ((v).type == Variable::POINTER)

#if 1
// You can use this at the start of your library functions to ensure correct number of arguments
#define MIN_ARGS(n) if (v.size() < n) throw Error(std::string(__FUNCTION__) + ": " + "Incorrect number of arguments at " + get_error_info(prg));
#define COUNT_ARGS(n) if (v.size() != n) throw Error(std::string(__FUNCTION__) + ": " + "Incorrect number of arguments at " + get_error_info(prg));
		
#define CHECK_NUMBER(v) \
	if (!IS_NUMBER(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected number at " + get_error_info(prg)); \
	}
#define CHECK_STRING(v) \
	if (!IS_STRING(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected string at " + get_error_info(prg)); \
	}
#define CHECK_VECTOR(v) \
	if (!IS_VECTOR(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected vector at " + get_error_info(prg)); \
	}
#define CHECK_MAP(v) \
	if (!IS_MAP(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected map at " + get_error_info(prg)); \
	}
#define CHECK_LABEL(v) \
	if (!IS_LABEL(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected label at " + get_error_info(prg)); \
	}
#define CHECK_FUNCTION(v) \
	if (!IS_FUNCTION(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected function at " + get_error_info(prg)); \
	}
#define CHECK_EXPRESSION(v) \
	if (!IS_EXPRESSION(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected expression at " + get_error_info(prg)); \
	}
#define CHECK_FISH(v) \
	if (!IS_FISH(v)) { \
		throw Error(std::string(__FUNCTION__) + ": " + "Expected fish at " + get_error_info(prg)); \
	}
#endif

#if 0
#define MIN_ARGS(n)
#define COUNT_ARGS(n)
#define CHECK_NUMBER(v)
#define CHECK_STRING(v)
#define CHECK_VECTOR(v)
#define CHECK_MAP(v)
#define CHECK_LABEL(v)
#define CHECK_FUNCTION(v)
#define CHECK_EXPRESSION(v)
#define CHECK_FISH(v)
#endif

extern "C" {
	typedef void (*BOOBOO_DLL_START_FUNC)(void);
}

#endif // BOOBOO_H
