//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/context.hpp>
#include <util/error.hpp>
#include <iostream>

// ----------------------------------------------------------------------------- : Context

Context::Context()
	: level(0)
{}

// ----------------------------------------------------------------------------- : Evaluate

// Perform a unary simple instruction, store the result in a (not in *a)
void instrUnary  (UnaryInstructionType   i, ScriptValueP& a);

// Perform a binary simple instruction, store the result in a (not in *a)
void instrBinary (BinaryInstructionType  i, ScriptValueP& a, const ScriptValueP& b);

// Perform a ternary simple instruction, store the result in a (not in *a)
void instrTernary(TernaryInstructionType i, ScriptValueP& a, const ScriptValueP& b, const ScriptValueP& c);


ScriptValueP Context::eval(const Script& script, bool useScope) {
	size_t stack_size = stack.size();
	size_t scope = useScope ? openScope() : 0;
	try {
		// Instruction pointer
		const Instruction* instr = &script.instructions[0];
		
		// Loop until we are done
		while (true) {
			assert(instr < &*script.instructions.end());
			// debug
//			cout << script.dumpInstr(instr - &script.instructions[0], *instr) << endl;
			// Evaluate the current instruction
			Instruction i = *instr++;
			switch (i.instr) {
				case I_NOP: break;
				// Push a constant
				case I_PUSH_CONST: {
					stack.push_back(script.constants[i.data]);
					break;
				}
				// Pop top value
				case I_POP: {
					stack.pop_back();
					break;
				}
				// Jump
				case I_JUMP: {
					instr = &script.instructions[i.data];
					break;
				}
				// Conditional jump
				case I_JUMP_IF_NOT: {
					int condition = *stack.back();
					stack.pop_back();
					if (!condition) {
						instr = &script.instructions[i.data];
					}
					break;
				}
				
				// Get a variable
				case I_GET_VAR: {
					ScriptValueP value = variables[i.data].value;
					if (!value) throw ScriptError(_("Variable not set: ") + variable_to_string(i.data));
					stack.push_back(value);
					break;
				}
				// Set a variable
				case I_SET_VAR: {
					setVariable(i.data, stack.back());
					break;
				}
				
				// Get an object member
				case I_MEMBER_C: {
					stack.back() = stack.back()->getMember(*script.constants[i.data]);
					break;
				}
				// Loop over a container, push next value or jump
				case I_LOOP: {
					ScriptValueP& it = stack[stack.size() - 2]; // second element of stack
					assert(dynamic_pointer_cast<ScriptIterator>(it)); // top of stack must be an iterator
					ScriptValueP val = static_pointer_cast<ScriptIterator>(it)->next();
					if (val) {
						stack.push_back(val);
					} else {
						stack.erase(stack.end() - 2); // remove iterator
						instr = &script.instructions[i.data];
					}
					break;
				}
				
				// Function call
				case I_CALL: {
					// new scope
					size_t scope = openScope();
					// prepare arguments
					for (unsigned int j = 0 ; j < i.data ; ++j) {
						setVariable(instr[i.data - j - 1].data, stack.back());
						stack.pop_back();
					}
					instr += i.data; // skip arguments
					// get function and call
					stack.back() = stack.back()->eval(*this);
					// restore scope
					closeScope(scope);
					break;
				}
				// Function return
				case I_RET: {
					// restore shadowed variables
					if (useScope) closeScope(scope);
					// return top of stack
					ScriptValueP result = stack.back();
					stack.pop_back();
					assert(stack.size() == stack_size); // we end up with the same stack
					return result;
				}
				
				// Simple instruction: unary
				case I_UNARY: {
					instrUnary(i.instr1, stack.back());
//					cout << "\t\t-> " << (String)*stack.back() << endl;
					break;
				}
				// Simple instruction: binary
				case I_BINARY: {
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					instrBinary(i.instr2, a, b);
//					cout << "\t\t-> " << (String)*stack.back() << endl;
					break;
				}
				// Simple instruction: ternary
				case I_TERNARY: {
					ScriptValueP  c = stack.back(); stack.pop_back();
					ScriptValueP  b = stack.back(); stack.pop_back();
					ScriptValueP& a = stack.back();
					instrTernary(i.instr3, a, b, c);
//					cout << "\t\t-> " << (String)*stack.back() << endl;
					break;
				}
			}
		}
		
	} catch (...) {
		// cleanup after an exception
		if (useScope) closeScope(scope); // restore scope
		stack.resize(stack_size);     // restore stack
		throw; // rethrow
	}
}

void Context::setVariable(const String& name, const ScriptValueP& value) {
	setVariable(string_to_variable(name), value);
}

void Context::setVariable(int name, const ScriptValueP& value) {
	Variable& var = variables[name];
	if (var.value && var.level < level) {
		// keep shadow copy
		Binding bind = {name, var};
		shadowed.push_back(bind);
	}
	var.level = level;
	var.value = value;
}

ScriptValueP Context::getVariable(const String& name) {
	ScriptValueP value = variables[string_to_variable(name)].value;
	if (!value) throw ScriptError(_("Variable not set: ") + name);
	return value;
}

ScriptValueP Context::getVariableOpt(const String& name) {
	return variables[string_to_variable(name)].value;
}


size_t Context::openScope() {
	level += 1;
	return shadowed.size();
}
void Context::closeScope(size_t scope) {
	assert(level > 0);
	assert(scope <= shadowed.size());
	level -= 1;
	// restore shadowed variables
	while (shadowed.size() > scope) {
		variables[shadowed.back().variable] = shadowed.back().value;
		shadowed.pop_back();
	}
}

// ----------------------------------------------------------------------------- : Simple instructions : unary

void instrUnary  (UnaryInstructionType   i, ScriptValueP& a) {
	switch (i) {
		case I_ITERATOR_C:
			a = a->makeIterator();
			break;
		case I_NEGATE:
			a = toScript(-(int)*a);
			break;
		case I_NOT:
			a = toScript(!(int)*a);
			break;
	}
}

// ----------------------------------------------------------------------------- : Simple instructions : binary

// operator on ints
#define OPERATOR_I(OP)											\
	a = toScript((int)*a  OP  (int)*b);							\
	break

// operator on doubles or ints
#define OPERATOR_DI(OP)											\
	if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {			\
		a = toScript((double)*a  OP  (double)*b);				\
	} else {													\
		a = toScript((int)*a     OP  (int)*b);					\
	}															\
	break

// operator on strings or doubles or ints
#define OPERATOR_SDI(OP)										\
	if (at == SCRIPT_STRING || bt == SCRIPT_STRING) {			\
		a = toScript((String)*a  OP  (String)*b);				\
	} else if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {	\
		a = toScript((double)*a  OP  (double)*b);				\
	} else {													\
		a = toScript((int)*a     OP  (int)*b);					\
	}															\
	break

/// Composition of two functions
class ScriptCompose : public ScriptValue {
  public:
	ScriptCompose(ScriptValueP a, ScriptValueP b) : a(a), b(b) {}
	
	virtual ScriptType type() const { return SCRIPT_FUNCTION; }
	virtual String typeName() const { return _("replace_rule"); }
	virtual ScriptValueP eval(Context& ctx) const {
		ctx.setVariable(_("input"), a->eval(ctx));
		return b->eval(ctx);
	}
	virtual ScriptValueP dependencies(Context& ctx, const Dependency& dep) const {
		ctx.setVariable(_("input"), a->dependencies(ctx, dep));
		return b->dependencies(ctx, dep);
	}
  private:
	ScriptValueP a,b;
};

void instrBinary (BinaryInstructionType  i, ScriptValueP& a, const ScriptValueP& b) {
	ScriptType at = a->type(), bt = b->type();
	switch (i) {
		case I_MEMBER:
			a = a->getMember(*b);
			break;
		case I_ITERATOR_R:
			a = rangeIterator(*a, *b);
			break;
		case I_ADD: // add is quite overloaded
			if (at == SCRIPT_NIL) {
				a = b;
			} else if (bt == SCRIPT_NIL) {
				// a = a;
			} else if (at == SCRIPT_FUNCTION && bt == SCRIPT_FUNCTION) {
				a = new_intrusive2<ScriptCompose>(a, b);
			} else if (at == SCRIPT_STRING || bt == SCRIPT_STRING) {
				a = toScript((String)*a  +  (String)*b);
			} else if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {
				a = toScript((double)*a  +  (double)*b);
			} else if (at == SCRIPT_INT    || bt == SCRIPT_INT) {
				a = toScript((int)*a     +  (int)*b);
			} else {
				a = toScript((String)*a  +  (String)*b);
			}
			break;
		case I_SUB:		OPERATOR_DI(-);
		case I_MUL:		OPERATOR_DI(*);
		case I_DIV:		OPERATOR_DI(/);
		case I_MOD:
			if (at == SCRIPT_DOUBLE || bt == SCRIPT_DOUBLE) {
				a = toScript(fmod((double)*a, (double)*b));
			} else {
				a = toScript((int)*a % (int)*b);
			}
			break;
		case I_AND:		OPERATOR_I(&&);
		case I_OR:		OPERATOR_I(||);
		case I_EQ:		OPERATOR_SDI(==);
		case I_NEQ:		OPERATOR_SDI(!=);
		case I_LT:		OPERATOR_DI(<);
		case I_GT:		OPERATOR_DI(>);
		case I_LE:		OPERATOR_DI(<=);
		case I_GE:		OPERATOR_DI(>=);
	}
}

// ----------------------------------------------------------------------------- : Simple instructions : ternary

void instrTernary(TernaryInstructionType i, ScriptValueP& a, const ScriptValueP& b, const ScriptValueP& c) {
	switch (i) {
		case I_RGB:
			a = toScript(Color((int)*a, (int)*b, (int)*c));
			break;
	}
}
