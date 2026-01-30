#ifndef BOOBOO_INTERNAL_H
#define BOOBOO_INTERNAL_H

namespace booboo {

struct Statement {
	int method;
	std::vector<Token> data;
	std::string name;
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

	Variable result;
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
	int obfuscated_count;

	std::vector<Variable> variables;
	std::map<std::string, int> variables_map;
	std::map<std::string, int> function_name_map;
	std::vector<Program> functions;
	std::vector<std::string> orig_function_names;
	std::vector<std::string> function_names;
	std::vector<int> params;
	std::vector<std::string> param_names;
	std::vector<bool> ref;
	std::vector< std::map<std::string, int> > locals;
	std::vector< std::map<std::string, int> > backup;

	std::vector<int> real_line_numbers;
	std::vector<std::string> real_file_names;

	std::map<std::string, void *> black_box;
};

glm::mat4 BOOBOO_EXPORT to_glm_mat4(Variable &v);
Variable BOOBOO_EXPORT from_glm_mat4(glm::mat4 m);

extern std::vector<booboo::library_func> library;

extern BOOBOO_EXPORT std::vector<std::string> cli_args;

extern BOOBOO_EXPORT int num_ops;

} // End namespace booboo

#endif // BOOBOO_INTERNAL_H
