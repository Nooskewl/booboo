#ifndef BOOBOO_INTERNAL_H
#define BOOBOO_INTERNAL_H

namespace booboo {

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
	
	std::vector<int> line_numbers;
};

struct Program {
	Function_Swap *s;

	int compare_flag;
	Pass complete_pass;

	int tmps;

	std::vector<Variable> variables;
	std::map<std::string, int> variables_map;
	std::map<std::string, int> function_name_map;
	std::vector<Program> functions;
	std::vector<int> params;
	std::vector< std::map<std::string, int> > locals;
	std::map<std::string, int> backup;

	std::vector<int> real_line_numbers;
	std::vector<std::string> real_file_names;

	std::map<std::string, void *> black_box;
};

std::string itos(int i);

inline Variable &as_variable_inline(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (prg->variables[t.i].type == Variable::POINTER) {
		return *prg->variables[t.i].p;
	}
	if (prg->variables[t.i].type == Variable::FISH) {
		return go_fish(prg, prg->variables[t.i].f);
	}
	return prg->variables[t.i];
}

inline double as_number_inline(Program *prg, const Token &t)
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
		else if (v->type == Variable::EXPRESSION) {
			Variable var = evaluate_expression(prg, v->e);
			if (var.type == Variable::NUMBER) {
				return var.n;
			}
			else if (var.type == Variable::STRING) {
				return atof(var.s.c_str());
			}
			else {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
			}
		}
		else if (v->type == Variable::STRING) {
			return atof(v->s.c_str());
		}
		else if (v->type == Variable::FISH) {
			Variable &var = go_fish(prg, v->f);
			if (var.type == Variable::NUMBER) {
				return var.n;
			}
			else if (var.type == Variable::STRING) {
				return atof(var.s.c_str());
			}
			else {
				throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
			}
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}
	else if (t.type == Token::STRING) {
		return atof(t.s.c_str());
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
}

inline std::string as_string_inline(Program *prg, const Token &t)
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
		else if (v->type == Variable::NUMBER) {
			char buf[1000];
			snprintf(buf, 1000, "%g", v->n);
			return buf;
		}
		else if (v->type == Variable::EXPRESSION) {
			Variable var = evaluate_expression(prg, v->e);
			if (var.type != Variable::STRING) {
				throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
			}
			return var.s;
		}
		else if (v->type == Variable::FISH) {
			Variable &var = go_fish(prg, v->f);
			if (var.type != Variable::STRING) {
				throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
			}
			return var.s;
		}
		else {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
	}
	else if (t.type == Token::STRING) {
		return t.s;
	}
	else {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
}

inline int as_label_inline(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (prg->variables[t.i].type == Variable::POINTER) {
		if (prg->variables[t.i].p->type != Variable::LABEL) {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
		return prg->variables[t.i].p->n;
	}
	Variable *v = &prg->variables[t.i];
	if (v->type == Variable::FISH) {
		Variable &var = go_fish(prg, v->f);
		if (var.type != Variable::LABEL) {
			throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
		}
		return var.n;
	}
	else if (v->type != Variable::LABEL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	return v->n;
}

inline int as_function_inline(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	if (prg->variables[t.i].type == Variable::POINTER) {
		if (prg->variables[t.i].p->type != Variable::FUNCTION) {
			throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
		}
		return prg->variables[t.i].p->n;
	}
	Variable *v = &prg->variables[t.i];
	if (v->type == Variable::FISH) {
		Variable &var = go_fish(prg, v->f);
		if (var.type != Variable::FUNCTION) {
			throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
		}
		return var.n;
	}
	else if (v->type != Variable::FUNCTION) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	return v->n;
}

inline Variable &as_pointer_inline(Program *prg, const Token &t)
{
	if (t.type != Token::SYMBOL) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	Variable *v = &prg->variables[t.i];
	if (v->type == Variable::FISH) {
		Variable &var = go_fish(prg, v->f);
		if (var.type != Variable::POINTER) {
			throw Error(std::string(__FUNCTION__) + ": " + "Fished out the wrong type at " + get_error_info(prg));
		}
		return *v;
	}
	else if (v->type != Variable::POINTER) {
		throw Error(std::string(__FUNCTION__) + ": " + "Invalid type at " + get_error_info(prg));
	}
	return *v;
}

extern std::vector<booboo::library_func> library;

inline bool interpret_inline(Program *prg)
{
	if (prg->s->pc >= prg->s->program.size()) {
		return false;
	}

	Statement &s = prg->s->program[prg->s->pc];

	unsigned int pc_bak = prg->s->pc;

	library_func func = library[s.method];
	bool ret = func(prg, s.data);

	if (pc_bak == prg->s->pc) {
		prg->s->pc++;
	}

	return ret;
}

} // End namespace booboo

#endif // BOOBOO_INTERNAL_H
