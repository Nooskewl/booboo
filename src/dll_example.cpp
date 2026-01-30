#include "booboo/booboo.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__ ((visibility("default"))) 
#endif

using namespace booboo;

static Variable exprfunc_seventeen(Program *prg, const std::vector<Token> &v)
{
	Variable var;
	var.type = Variable::NUMBER;
	var.n = 17;
	return var;
}

extern "C" {

void EXPORT booboo_start(void)
{
	add_expression_handler("seventeen", exprfunc_seventeen);
}

}
