#ifndef BOOBOO_INTERNAL_H
#define BOOBOO_INTERNAL_H

namespace booboo {

struct Label {
	std::string name;
	int pc;
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

	Variable result;
	std::vector<Statement> program;
	unsigned int pc;
};

struct Program {
	Function_Swap *s;

	int compare_flag;
	Pass complete_pass;

	std::vector<Variable> variables;
	std::map<std::string, int> variables_map;
	std::map<std::string, int> function_name_map;
	std::vector<Program> functions;
	std::vector<int> params;

	std::vector<int> line_numbers;

	std::vector<int> real_line_numbers;
	std::vector<std::string> real_file_names;

	std::map<std::string, void *> black_box;
};

std::string itos(int i);

// This automatically gets called when you call booboo::start
void start_lib_core();

inline Variable &as_variable_inline(Program *prg, Token &t)
{
	if (prg->variables[t.i].type == Variable::POINTER) {
		return *prg->variables[t.i].p;
	}
	return prg->variables[t.i];
}

inline double as_number_inline(Program *prg, Token &t)
{
	if (t.type == Token::NUMBER) {
		return t.n;
	}
	else if (t.type == Token::SYMBOL) {
		Variable *v;
		if (prg->variables[t.i].type == Variable::POINTER) {
			v = prg->variables[t.i].p;
		}
		else {
		      v	= &prg->variables[t.i];
		}
		if (v->type == Variable::NUMBER) {
			return v->n;
		}
		else {
			return atof(v->s.c_str());
		}
	}
	else {
		return atof(t.s.c_str());
	}
}

inline std::string as_string_inline(Program *prg, Token &t)
{
	if (t.type == Token::NUMBER) {
		char buf[1000];
		snprintf(buf, 1000, "%g", t.n);
		return buf;
	}
	else if (t.type == Token::SYMBOL) {
		Variable *v;
		if (prg->variables[t.i].type == Variable::POINTER) {
			v = prg->variables[t.i].p;
		}
		else {
		      v	= &prg->variables[t.i];
		}
		if (v->type == Variable::STRING) {
			return v->s;
		}
		else {
			char buf[1000];
			snprintf(buf, 1000, "%g", v->n);
			return buf;
		}
	}
	else {
		return t.s;
	}
}

inline int as_label_inline(Program *prg, Token &t)
{
	if (prg->variables[t.i].type == Variable::POINTER) {
		return prg->variables[t.i].p->n;
	}
	return prg->variables[t.i].n;
}

inline int as_function_inline(Program *prg, Token &t)
{
	if (prg->variables[t.i].type == Variable::POINTER) {
		return prg->variables[t.i].p->n;
	}
	return prg->variables[t.i].n;
}

} // End namespace booboo

#endif // BOOBOO_INTERNAL_H
