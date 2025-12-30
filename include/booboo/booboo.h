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
	union {
		double n;
		int i;
	};

	std::string token;

	bool dereference;
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

	Variable(const Variable &var)
	{
		type = var.type;
		name = var.name;
		obfuscated = var.obfuscated;
		constant = var.constant;

		switch (type) {
			case NUMBER:
			case FUNCTION:
			case LABEL:
				n = var.n;
				break;
			case STRING:
				s = var.s;
				break;
			case VECTOR:
				v = var.v;
				break;
			case MAP:
				m = var.m;
				break;
			case EXPRESSION:
				e = var.e;
				break;
			case FISH:
				f = var.f;
				break;
			case POINTER:
				p = var.p;
				break;
			default:
				n = var.n;
				s = var.s;
				v = var.v;
				m = var.m;
				e = var.e;
				f = var.f;
				p = var.p;
				break;
		}
	}

	Variable() :
		type(UNTYPED),
		constant(false)
	{
	}

	~Variable()
	{
	}
};

typedef bool (*library_func)(Program *prg, const std::vector<Token> &v);
typedef std::string (*token_func)(Program *);
typedef Variable (*expression_func)(Program *prg, const std::vector<Token> &v);

// Call these before/after using BooBoo
void start();
void end();

// These are the how you create, destroy and run programs
Program *create_program(std::string code);
void destroy_program(Program *prg);
bool interpret(Program *prg);

// Special functions are left unobfuscated
void add_special_function(std::string name);

// Functions calling
void call_function(Program *prg, int function, const std::vector<Token> &params, Variable &result, int ignore_params = 0);
void call_function(Program *prg, std::string function, const std::vector<Token> &params, Variable &result, int ignore_params = 0);
void call_void_function(Program *prg, int function, const std::vector<Token> &params, int ignore_params = 0);
void call_void_function(Program *prg, std::string function, const std::vector<Token> &params, int ignore_params = 0);

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
Variable &as_variable(Program *prg, const Token &t);
Variable as_variable_resolve(Program *prg, const Token &t);
double as_number(Program *prg, const Token &t);
std::string as_string(Program *prg, const Token &t);
int as_label(Program *prg, const Token &t);
int as_function(Program *prg, const Token &t);

// The black box allows you to store anything you want
void *get_black_box(Program *prg, std::string id);
void set_black_box(Program *prg, std::string id, void *data);

void obfuscate(Program *prg);

// If you have a variable of type SYMBOL then 'i' is the index you pass here to retrive the variable
Variable &get_variable(Program *prg, int index);

Variable evaluate_expression(Program *prg, const Variable::Expression &e);
Variable &go_fish(Program *prg, const Variable::Fish &f);

// This stuff can be used but it's used by the BooBoo interpreter
extern std::string reset_game_name;
extern std::string main_program_name;
extern int return_code;
extern bool quit;
extern std::string (*load_text)(std::string filename); // must be set

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

#endif // BOOBOO_H
