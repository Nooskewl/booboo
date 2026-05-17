#ifndef BOOBOO_INTERNAL_H
#define BOOBOO_INTERNAL_H

namespace booboo {

glm::mat4 BOOBOO_EXPORT to_glm_mat4(Variable &v);
Variable BOOBOO_EXPORT from_glm_mat4(glm::mat4 m);

extern std::vector<booboo::library_func> library;

extern BOOBOO_EXPORT std::vector<std::string> cli_args;

} // End namespace booboo

#endif // BOOBOO_INTERNAL_H
