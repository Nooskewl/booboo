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

struct Program;

class BOOBOO_EXPORT Error {
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

	int dereference;
};

bool BOOBOO_EXPORT jsonfunc_register_number(Program *prg, const std::vector<Token> &v);
bool BOOBOO_EXPORT jsonfunc_register_string(Program *prg, const std::vector<Token> &v);

struct BOOBOO_EXPORT Variable
{
	friend bool BOOBOO_EXPORT booboo::jsonfunc_register_number(Program *prg, const std::vector<Token> &v);
	friend bool BOOBOO_EXPORT booboo::jsonfunc_register_string(Program *prg, const std::vector<Token> &v);

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
		USER,
		UNTYPED
	};

	struct Expression
	{
		int i;
		std::string name;
		std::vector<Token> v;
		int dereference;

		Expression() {
		}

		Expression(const Expression &e) :
			i(e.i),
			name(e.name),
			v(e.v),
			dereference(e.dereference)
		{
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

		Fish(const Fish &f) :
			c_i(f.c_i),
			v(f.v),
			dereference(f.dereference)
		{
		}

		~Fish() {
		}
	};

	std::string name;

	bool constant;

	bool operator==(const Variable &var) const;

	Variable& operator=(const Variable &var);
	Variable(const Variable &var);
	
	Variable();

	~Variable();

	void set(const Variable &var);

	// This sets type and clears memory (vector/map), should be used when setting
	// type of prg->result to avoid copying that memory
	void set_type(Variable_Type type, bool clear_values = true);
	Variable_Type get_type();

	void clear();

	void set_n(double n);
	double get_n();

	void set_s(std::string s);
	std::string get_s();

	Variable *p;
	std::vector<Variable> v;
	std::map<std::string, Variable> m;
	Expression e;
	Fish f;

protected:
	Variable_Type type;
	double n;
	std::string s;

	void changed();
};

struct Statement {
	int method;
	std::vector<Token> data;
};

enum Pass {
	PASS0, // nothing started yet
	PASS1,
	PASS2
};

// This stuff is simply grouped to make swapping it out of the running program fast with a pointer
// when doing a function call (most of the state of the program remains the same but this stuff
// gets swapped out for the function's versions of these variables, then some of them like p/line
// get set to 0/1
struct Function_Swap {
	std::string name;

	std::string code;

	unsigned int p;
	unsigned int line;
	unsigned int start_line;

	std::vector<Statement> program;
	unsigned int pc;
	
	std::vector<int> line_numbers;
};

struct Program {
	Function_Swap *s;

	int compare_flag;
	bool break_flag;
	bool continue_flag;
	Pass complete_pass;
	Variable result;

	int var_i;
	int func_i;
	int expression_i;
	int fish_i;

	int num_vars;
	int num_consts;

	std::vector<Variable> variables;
	std::map<std::string, int> variables_map;
	std::map<std::string, int> function_name_map;
	std::vector<Program> functions;
	std::vector<std::string> function_names;
	std::vector<int> params;
	std::vector<std::string> param_names;
	std::vector<bool> ref;
	std::vector< std::map<std::string, int> > locals;
	std::vector< std::map<std::string, int> > backup;

	std::vector<int> real_line_numbers;
	std::vector<std::string> real_file_names;
};

enum F12 {
	F12_START,
	F12_END
};

typedef bool (*library_func)(Program *prg, const std::vector<Token> &v);
typedef std::string (*token_func)(Program *);
typedef void (*expression_func)(Program *prg, const std::vector<Token> &v);

// Call these before/after using BooBoo
void BOOBOO_EXPORT start();
void BOOBOO_EXPORT end();

// These are the how you create, destroy and run programs
Program BOOBOO_EXPORT *create_program(std::string code);
void BOOBOO_EXPORT destroy_program(Program *prg);
bool BOOBOO_EXPORT interpret(Program *prg, bool trigger_breakpoints = true);

// Functions calling
void BOOBOO_EXPORT call_function(Program *prg, int function, const std::vector<Token> &params, int ignore_params = 0);
void BOOBOO_EXPORT call_function(Program *prg, std::string function, const std::vector<Token> &params, int ignore_params = 0);

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
// use_result is faster when you aren't calling as_* after this call
Variable BOOBOO_EXPORT *as_variable_pointer(Program *prg, const std::vector<Token> &v, int index, bool use_result = false);
Variable BOOBOO_EXPORT &as_variable(Program *prg, const std::vector<Token> &v, int index);
// If you know result will not be overwritten by further calls, setting use_result is going to be faster because it avoids copying result into a temp static var
Variable BOOBOO_EXPORT as_variable_resolve(Program *prg, const std::vector<Token> &v, int index);
double BOOBOO_EXPORT as_number(Program *prg, const std::vector<Token> &v, int index);
std::string BOOBOO_EXPORT as_string(Program *prg, const std::vector<Token> &v, int index);
int BOOBOO_EXPORT as_label(Program *prg, const std::vector<Token> &v, int index);
int BOOBOO_EXPORT as_function(Program *prg, const std::vector<Token> &v, int index);
Variable BOOBOO_EXPORT as_pointer(Program *prg, const std::vector<Token> &v, int index);

// The black box allows you to store anything you want
void BOOBOO_EXPORT *get_black_box(std::string id);
void BOOBOO_EXPORT set_black_box(std::string id, void *data);

// If you have a variable of type SYMBOL then 'i' is the index you pass here to retrive the variable
Variable BOOBOO_EXPORT &get_variable(Program *prg, int index);

void BOOBOO_EXPORT evaluate_expression(Program *prg, const Variable::Expression &e);
Variable BOOBOO_EXPORT &go_fish(Program *prg, const Variable::Fish &f);

extern BOOBOO_EXPORT Variable *dereference(Program *prg, const std::vector<Token> &v, int index);

// This stuff can be used but it's used by the BooBoo interpreter
extern BOOBOO_EXPORT std::string reset_game_name;
extern BOOBOO_EXPORT std::string main_program_name;
extern BOOBOO_EXPORT int return_code;
extern BOOBOO_EXPORT bool quit;
extern BOOBOO_EXPORT bool callbacks_enabled;
extern BOOBOO_EXPORT std::string (*load_text)(std::string filename); // must be set
extern BOOBOO_EXPORT Program *prg;
extern BOOBOO_EXPORT Program *prg_func;
extern BOOBOO_EXPORT std::map<std::string, void *> black_box;

} // End namespace booboo

#define IS_NUMBER(v) ((v).get_type() == Variable::NUMBER)
#define IS_STRING(v) ((v).get_type() == Variable::STRING)
#define IS_VECTOR(v) ((v).get_type() == Variable::VECTOR)
#define IS_MAP(v) ((v).get_type() == Variable::MAP)
#define IS_LABEL(v) ((v).get_type() == Variable::LABEL)
#define IS_FUNCTION(v) ((v).get_type() == Variable::FUNCTION)
#define IS_EXPRESSION(v) ((v).get_type() == Variable::EXPRESSION)
#define IS_FISH(v) ((v).get_type() == Variable::FISH)
#define IS_POINTER(v) ((v).get_type() == Variable::POINTER)
#define IS_USER(v) ((v).get_type() == Variable::USER)

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
