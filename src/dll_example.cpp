#include "booboo/booboo.h"

using namespace booboo;

static Variable exprfunc_seventeen(Program *prg, const std::vector<Token> &v)
{
	Variable var;
	var.type = Variable::NUMBER;
	var.n = 17;
	return var;
}

extern "C" {

void __declspec(dllexport) booboo_start(void)
{
	add_expression_handler("seventeen", exprfunc_seventeen);
}

}
