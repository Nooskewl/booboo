#ifndef BOOBOO_INTERNAL_H
#define BOOBOO_INTERNAL_H

namespace booboo {

struct Timer_Callback {
	int func;
	Uint32 time;
	std::vector<Token> tokens;
};

glm::mat4 BOOBOO_EXPORT to_glm_mat4(Variable &v);
Variable BOOBOO_EXPORT from_glm_mat4(glm::mat4 m);

extern std::vector<booboo::library_func> library;

extern BOOBOO_EXPORT std::vector<std::string> cli_args;

extern BOOBOO_EXPORT std::vector<Timer_Callback> timer_callbacks;

} // End namespace booboo

#endif // BOOBOO_INTERNAL_H
