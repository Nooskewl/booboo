#ifndef BOOBOO_INTERNAL_H
#define BOOBOO_INTERNAL_H

namespace booboo {

struct Timer_Callback {
	int func;
	float time;
	std::vector<Token> tokens;
};

glm::mat4 BOOBOO_EXPORT to_glm_mat4(Variable &v);
Variable BOOBOO_EXPORT from_glm_mat4(glm::mat4 m);

extern std::vector<booboo::library_func> library;

extern BOOBOO_EXPORT std::vector<std::string> cli_args;

extern BOOBOO_EXPORT std::vector<Timer_Callback> timer_callbacks;
		
extern BOOBOO_EXPORT std::vector<std::string> function_breakpoints;
extern BOOBOO_EXPORT std::vector<std::string> file_breakpoints;

void BOOBOO_EXPORT my_throw(Error e);

Variable::Fish BOOBOO_EXPORT parse_fish(Program *prg, Program *func, std::string expr, Pass pass);
Variable::Expression BOOBOO_EXPORT parse_expression(Program *prg, Program *func, std::string expr, Pass pass);

std::string BOOBOO_EXPORT typeof_var(Variable *v1);

void BOOBOO_EXPORT debug(std::string text);

} // End namespace booboo

#endif // BOOBOO_INTERNAL_H
