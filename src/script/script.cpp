//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

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
	map<String, unsigned int>::iterator it = variables.find(s);
	if (it == variables.end()) {
		#ifdef _DEBUG
			variable_names.push_back(s);
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
	
void Script::addInstruction(InstructionType t) {
	Instruction i = {t, {0}};
	instructions.push_back(i);
}
void Script::addInstruction(InstructionType t, unsigned int d) {
	if (t == I_BINARY && d == I_MEMBER && !instructions.empty() && instructions.back().instr == I_PUSH_CONST) {
		// optimize: push x ; member -->  member_c x
		instructions.back().instr = I_MEMBER_C;
		return;
	}
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
	     || instructions.at(pos).instr == I_LOOP);
	instructions.at(pos).data = (unsigned int)instructions.size();
}

unsigned int Script::getLabel() const {
	return (unsigned int)instructions.size();
}

DECLARE_TYPEOF_COLLECTION(Instruction);

#if 0 // debugging

String Script::dumpScript() const {
	String ret;
	int pos = 0;
	FOR_EACH_CONST(i, instructions) {
		ret += dumpInstr(pos++, i) + "\n";
	}
	return ret;
}

String Script::dumpInstr(unsigned int pos, Instruction i) const {
	String ret = lexical_cast<String>(pos) + ":\t";
	// instruction
	switch (i.instr) {
		case I_NOP:			ret += "nop";		break;
		case I_PUSH_CONST:	ret += "push";		break;
		case I_POP:			ret += "pop";		break;
		case I_JUMP:		ret += "jump";		break;
		case I_JUMP_IF_NOT:	ret += "jnz";		break;
		case I_GET_VAR:		ret += "get";		break;
		case I_SET_VAR:		ret += "set";		break;
		case I_MEMBER_C:	ret += "member_c";	break;
		case I_LOOP:		ret += "loop";		break;
		case I_CALL:		ret += "call";		break;
		case I_RET:			ret += "ret";		break;
		case I_UNARY:		ret += "unary\t";
			switch (i.instr1) {
				case I_ITERATOR_C:	ret += "iterator_c";break;
				case I_NEGATE:		ret += "negate";	break;
				case I_NOT:			ret += "not";		break;
			}
			break;
		case I_BINARY:		ret += "binary\t";
			switch (i.instr2) {
				case I_ITERATOR_R:	ret += "iterator_r";break;
				case I_MEMBER:		ret += "member";	break;
				case I_ADD:			ret += "+";			break;
				case I_SUB:			ret += "-";			break;
				case I_MUL:			ret += "*";			break;
				case I_DIV:			ret += "/";			break;
				case I_MOD:			ret += "*";			break;
				case I_AND:			ret += "and";		break;
				case I_OR:			ret += "or";		break;
				case I_EQ:			ret += "==";		break;
				case I_NEQ:			ret += "!=";		break;
				case I_LT:			ret += "<";			break;
				case I_GT:			ret += ">";			break;
				case I_LE:			ret += "<=";		break;
				case I_GE:			ret += ">=";		break;
			}
			break;
		case I_TERNARY:		ret += "ternary\t";
			switch (i.instr3) {
				case I_RGB:			ret += "rgb";		break;
			}
			break;
	}
	// arg
	switch (i.instr) {
		case I_PUSH_CONST: case I_MEMBER_C:							// const
			ret += "\t" + (String)*constants[i.data];
			ret += "\t(" + constants[i.data]->typeName();
			ret += ", #" + lexical_cast<String>(i.data) + ")";
			break;
		case I_JUMP: case I_JUMP_IF_NOT: case I_LOOP: case I_CALL:	// int
			ret += "\t" + lexical_cast<String>(i.data);
			break;
		case I_GET_VAR: case I_SET_VAR: case I_NOP:					// variable
			ret += "\t" + variable_to_string(i.data) + "\t$" + lexical_cast<String>(i.data);
			break;
	}
	return ret;
}

#endif


// ----------------------------------------------------------------------------- : Backtracing

const Instruction* Script::backtraceSkip(const Instruction* instr, int to_skip) const {
	for (;instr >= &instructions[0] && 
	       (to_skip   || // we have something to skip
	        instr >= &instructions[1] && (instr-1)->instr == I_JUMP // always look inside a jump
	       ) ; --instr) {
		// skip an instruction
		switch (instr->instr) {
			case I_PUSH_CONST:
			case I_GET_VAR:
				to_skip -= 1; break; // nett stack effect +1
			case I_POP:
			case I_BINARY:
				to_skip += 1; break; // nett stack effect 1-2 == -1
			case I_TERNARY:
				to_skip += 2; break; // nett stack effect 1-3 == -1
			case I_CALL:
				to_skip += instr->data; // arguments of call
				break;
			case I_MAKE_OBJECT:
				to_skip += 2 * instr->data - 1;
				break;
			case I_JUMP: {
				// jumps outputed by the parser are always backwards
				// and there will be a way not to take this jump
				// the part in between will have no significant stack effect
				assert(&instructions[instr->data] < instr);
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
			case I_RET: case I_JUMP_IF_NOT: case I_LOOP:
				return nullptr; // give up
			default:
				break; // nett stack effect 0
		}
	}
	return instr >= &instructions[0] ? instr : nullptr;
}