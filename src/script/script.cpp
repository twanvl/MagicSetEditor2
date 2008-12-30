//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/script.hpp>
#include <script/context.hpp>
#include <script/to_value.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Variables

typedef map<String, Variable> Variables;
Variables variables;
DECLARE_TYPEOF(Variables);
#ifdef _DEBUG
	vector<String> variable_names;
#endif

/// Return a unique name for a variable to allow for faster loopups
Variable string_to_variable(const String& s) {
	Variables::iterator it = variables.find(s);
	if (it == variables.end()) {
		#ifdef _DEBUG
			variable_names.push_back(s);
			assert(s == cannocial_name_form(s)); // only use cannocial names
		#endif
		Variable v = (Variable)variables.size();
		variables.insert(make_pair(s,v));
		return v;
	} else {
		return it->second;
	}
}

/// Get the name of a vaiable
/** Warning: this function is slow, it should only be used for error messages and such.
 */
String variable_to_string(Variable v) {
	FOR_EACH(vi, variables) {
		if (vi.second == v) return replace_all(vi.first, _(" "), _("_"));
	}
	throw InternalError(String(_("Variable not found: ")) << v);
}

// ----------------------------------------------------------------------------- : CommonVariables

void init_script_variables() {
	#define VarN(X,name) if (SCRIPT_VAR_##X != string_to_variable(name)) assert(false);
	#define Var(X)       VarN(X,_(#X))
	Var(input);
	Var(_1);
	Var(_2);
	Var(in);
	Var(match);
	Var(replace);
	VarN(in_context,_("in context"));
	Var(recursive);
	Var(order);
	Var(begin);
	Var(end);
	Var(filter);
	Var(choice);
	Var(choices);
	Var(format);
	Var(tag);
	Var(contents);
	Var(set);
	Var(game);
	Var(stylesheet);
	VarN(card_style,_("card style"));
	Var(card);
	Var(styling);
	Var(value);
	Var(condition);
	Var(language);
	assert(variables.size() == SCRIPT_VAR_CUSTOM_FIRST);
}

// ----------------------------------------------------------------------------- : Script

ScriptType Script::type() const {
	return SCRIPT_FUNCTION;
}
String Script::typeName() const {
	return _("function");
}
ScriptValueP Script::eval(Context& ctx) const {
	return ctx.eval(*this);
}
ScriptValueP Script::dependencies(Context& ctx, const Dependency& dep) const {
	return ctx.dependencies(dep, *this);
}

static const unsigned int INVALID_ADDRESS = 0x03FFFFFF;

unsigned int Script::addInstruction(InstructionType t) {
	assert( t == I_JUMP || t == I_JUMP_IF_NOT || t == I_LOOP || t == I_LOOP_WITH_KEY);
	Instruction i = {t, {INVALID_ADDRESS}};
	instructions.push_back(i);
	return getLabel() - 1;
}
void Script::addInstruction(InstructionType t, unsigned int d) {
	// Don't optimize ...I_PUSH_CONST x; I_MEMBER... to I_MEMBER_C
	// because the code could be something[if a then "x" else "y"]
	// the last instruction before I_MEMBER is I_PUSH_CONST "y", but this is only one branch of the if
	/*if (t == I_BINARY && d == I_MEMBER && !instructions.empty() && instructions.back().instr == I_PUSH_CONST) {
		// optimize: push x ; member -->  member_c x
		instructions.back().instr = I_MEMBER_C;
		return;
	}*/
	Instruction i = {t, {d}};
	instructions.push_back(i);
}
void Script::addInstruction(InstructionType t, const ScriptValueP& c) {
	constants.push_back(c);
	Instruction i = {t, {(unsigned int)constants.size() - 1}};
	instructions.push_back(i);
}
void Script::addInstruction(InstructionType t, const String& s) {
	constants.push_back(to_script(s));
	Instruction i = {t, {(unsigned int)constants.size() - 1}};
	instructions.push_back(i);
}

void Script::comeFrom(unsigned int pos) {
	assert( instructions.at(pos).instr == I_JUMP
	     || instructions.at(pos).instr == I_JUMP_IF_NOT
	     || instructions.at(pos).instr == I_LOOP
	     || instructions.at(pos).instr == I_LOOP_WITH_KEY);
	assert( instructions.at(pos).data == INVALID_ADDRESS );
	instructions.at(pos).data = (unsigned int)instructions.size();
}

unsigned int Script::getLabel() const {
	return (unsigned int)instructions.size();
}

DECLARE_TYPEOF_COLLECTION(Instruction);

#ifdef _DEBUG // debugging

String Script::dumpScript() const {
	String ret;
	int pos = 0;
	FOR_EACH_CONST(i, instructions) {
		wxLogDebug(dumpInstr(pos, i));
		ret += dumpInstr(pos++, i) + _("\n");
	}
	return ret;
}

String Script::dumpInstr(unsigned int pos, Instruction i) const {
	String ret = String::Format(_("%d:\t"),pos);
	// instruction
	switch (i.instr) {
		case I_NOP:			ret += _("nop");		break;
		case I_PUSH_CONST:	ret += _("push");		break;
		case I_JUMP:		ret += _("jump");		break;
		case I_JUMP_IF_NOT:	ret += _("jnz");		break;
		case I_GET_VAR:		ret += _("get");		break;
		case I_SET_VAR:		ret += _("set");		break;
		case I_MEMBER_C:	ret += _("member_c");	break;
		case I_LOOP:		ret += _("loop");		break;
		case I_LOOP_WITH_KEY:ret += _("loop with key"); break;
		case I_MAKE_OBJECT:	ret += _("make object");break;
		case I_CALL:		ret += _("call");		break;
		case I_CLOSURE:		ret += _("closure");	break;
		case I_UNARY:		ret += _("unary\t");
			switch (i.instr1) {
				case I_ITERATOR_C:	ret += _("iterator_c");	break;
				case I_NEGATE:		ret += _("negate");		break;
				case I_NOT:			ret += _("not");		break;
			}
			break;
		case I_BINARY:		ret += _("binary\t");
			switch (i.instr2) {
				case I_POP:			ret += _("pop");		break;
				case I_ITERATOR_R:	ret += _("iterator_r");	break;
				case I_MEMBER:		ret += _("member");		break;
				case I_ADD:			ret += _("+");			break;
				case I_SUB:			ret += _("-");			break;
				case I_MUL:			ret += _("*");			break;
				case I_DIV:			ret += _("/");			break;
				case I_MOD:			ret += _("mod");		break;
				case I_AND:			ret += _("and");		break;
				case I_OR:			ret += _("or");			break;
				case I_XOR:			ret += _("xor");		break;
				case I_EQ:			ret += _("==");			break;
				case I_NEQ:			ret += _("!=");			break;
				case I_LT:			ret += _("<");			break;
				case I_GT:			ret += _(">");			break;
				case I_LE:			ret += _("<=");			break;
				case I_GE:			ret += _(">=");			break;
				case I_OR_ELSE:		ret += _("or else");	break;
			}
			break;
		case I_TERNARY:		ret += _("ternary\t");
			switch (i.instr3) {
				case I_RGB:			ret += _("rgb");		break;
			}
			break;
		case I_QUATERNARY:	ret += _("quaternary\t");
			switch (i.instr3) {
				case I_RGBA:		ret += _("rgba");		break;
			}
			break;
		case I_DUP:			ret += _("dup");				break;
	}
	// arg
	switch (i.instr) {
		case I_PUSH_CONST: case I_MEMBER_C:							// const
			ret += _("\t") + constants[i.data]->typeName();
			break;
		case I_JUMP: case I_JUMP_IF_NOT: case I_LOOP: case I_LOOP_WITH_KEY: case I_MAKE_OBJECT: case I_CALL: case I_CLOSURE: case I_DUP:	// int
			ret += String::Format(_("\t%d"), i.data);
			break;
		case I_GET_VAR: case I_SET_VAR: case I_NOP:					// variable
			ret += _("\t") + variable_to_string((Variable)i.data);
			break;
	}
	return ret;
}

#endif


// ----------------------------------------------------------------------------- : Backtracing

const Instruction* Script::backtraceSkip(const Instruction* instr, int to_skip) const {
	unsigned int initial = instr - &instructions[0];
	for (;instr >= &instructions[0] && 
	       (to_skip   || // we have something to skip
	        instr >= &instructions[1] && (
	              (instr-1)->instr == I_JUMP // always look inside a jump
	           || (instr-1)->instr == I_NOP  // and skip nops
	           )
	       ) ; --instr) {
		// skip an instruction
		switch (instr->instr) {
			case I_PUSH_CONST:
			case I_GET_VAR: case I_DUP:
				to_skip -= 1; break; // nett stack effect +1
			case I_BINARY:
				to_skip += 1; break; // nett stack effect 1-2 == -1
			case I_TERNARY:
				to_skip += 2; break; // nett stack effect 1-3 == -2
			case I_QUATERNARY:
				to_skip += 3; break; // nett stack effect 1-4 == -3
			case I_CALL: case I_CLOSURE:
				to_skip += instr->data; // arguments of call
				break;
			case I_MAKE_OBJECT:
				to_skip += 2 * instr->data - 1;
				break;
			case I_JUMP: {
				if (instr->data > initial) {
					// forward jump, so we were in an else branch all along, ignore this jump
					return instr + 1;
				}
				// there will be a way not to take this jump
				// the part in between will have no significant stack effect
				unsigned int after_jump = instr + 1 - &instructions[0];
				for (--instr ; instr >= &instructions[0] ; --instr) {
					if (instr->instr == I_LOOP && instr->data == after_jump) {
						// code looks like
						//  1   (nettstack+1)  iterator
						//  2   (nettstack+1)  accumulator (usually push nil)
						//   loop:
						//  3   I_LOOP        end
						//  4   (netstack+0)
						//  5   I_JUMP        loop
						//   end:
						// we have not handled anything for this loop, current position is 2,
						// we need to skip two things (iterator+accumulator) instead of one
						to_skip += 1;
						break;
					} else if (instr->instr == I_LOOP_WITH_KEY && instr->data == after_jump) {
						// same as above,
						// we need to skip two things (iterator+accumulator) instead of one
						to_skip += 1;
						break;
					} else if (instr->instr == I_JUMP_IF_NOT && instr->data == after_jump) {
						// code looks like
						//  1   (nettstack+1)
						//  2   I_JUMP_IF_NOT else
						//  3   (nettstack+1)
						//  4   I_JUMP        end
						//   else:
						//  5   (nettstack+1)
						//   end:
						// we have already handled 4..5, current position is 2,
						// we need to skip an additional item for 1
						to_skip += 1;
						break;
					}
				}
				++instr; // compensate for the -- in the outer loop
				break;
			}
			case I_JUMP_IF_NOT: case I_LOOP: case I_LOOP_WITH_KEY:
				return nullptr; // give up
			default:
				break; // nett stack effect 0
		}
	}
	return instr >= &instructions[0] ? instr : nullptr;
}

String Script::instructionName(const Instruction* instr) const {
	if (instr < &instructions[0] || instr >= &instructions[instructions.size()]) return _("??\?");
	if (instr->instr == I_GET_VAR) {
		return variable_to_string((Variable)instr->data);
	} else if (instr->instr == I_MEMBER_C) {
		return instructionName(backtraceSkip(instr - 1, 0))
		     + _(".")
		     + constants[instr->data]->toString();
	} else if (instr->instr == I_BINARY && instr->instr2 == I_MEMBER) {
		return _("??\?[...]");
	} else if (instr->instr == I_BINARY && instr->instr2 == I_ADD) {
		return _("??? + ???");
	} else if (instr->instr == I_NOP) {
		return _("??\?(...)");
	} else if (instr->instr == I_CALL) {
		return instructionName(backtraceSkip(instr - 1, instr->data)) + _("(...)");
	} else if (instr->instr == I_CLOSURE) {
		return instructionName(backtraceSkip(instr - 1, instr->data)) + _("@(...)");
	} else {
		return _("??\?");
	}
}
