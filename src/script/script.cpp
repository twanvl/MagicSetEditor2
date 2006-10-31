//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/script.hpp>
#include <script/context.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Variables

typedef map<String, unsigned int> Variables;
Variables variables;
DECLARE_TYPEOF(Variables);

/// Return a unique name for a variable to allow for faster loopups
unsigned int string_to_variable(const String& s) {
	map<String, unsigned int>::iterator it = variables.find(s);
	if (it == variables.end()) {
		unsigned int v = (unsigned int)variables.size();
		variables.insert(make_pair(s,v));
		return v;
	} else {
		return it->second;
	}
}

/// Get the name of a vaiable
/** Warning: this function is slow, it should only be used for error messages and such.
 */
String variable_to_string(unsigned int v) {
	FOR_EACH(vi, variables) {
		if (vi.second == v) return vi.first;
	}
	throw ScriptError(String(_("Variable not found: ")) << v);
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
	//if (t == I_MEMBER_V && !instructions.empty() && instructions.back().instr == I_PUSH_CONST) {
	//	// optimize: push x ; member_v -->  member x
	//	instructions.back().instr = I_MEMBER;
	//} else {
	Instruction i = {t, 0};
	instructions.push_back(i);
	//}
}
void Script::addInstruction(InstructionType t, unsigned int d) {
	Instruction i = {t, d};
	instructions.push_back(i);
}
void Script::addInstruction(InstructionType t, const ScriptValueP& c) {
	constants.push_back(c);
	Instruction i = {t, (unsigned int)constants.size() - 1};
	instructions.push_back(i);
}
void Script::addInstruction(InstructionType t, const String& s) {
	constants.push_back(toScript(s));
	Instruction i = {t, (unsigned int)constants.size() - 1};
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
