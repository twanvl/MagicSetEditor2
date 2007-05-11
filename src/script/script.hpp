//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_SCRIPT_SCRIPT
#define HEADER_SCRIPT_SCRIPT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/value.hpp>

DECLARE_POINTER_TYPE(Script);

// ----------------------------------------------------------------------------- : Instructions

/// A type of instruction to be performed in a script
/** The reason for the negative values is that the compiler (at least MSVC) considers the bitfield signed
 */
enum InstructionType
	// Basic
{	I_NOP			= 0  ///< no operation, used as placeholder for extra data values
,	I_PUSH_CONST	= 1  ///< arg = value : push a constant onto the stack
,	I_POP			= 2  ///< pop the top value from the stack (used for ; operator)
,	I_JUMP			= 3  ///< arg = int   : move the instruction pointer to the given position
,	I_JUMP_IF_NOT	= 4  ///< arg = int   : move the instruction pointer if the top of the stack is false
	// Variables
,	I_GET_VAR		= 5  ///< arg = int   : find a variable, push its value onto the stack, it is an error if the variable is not found
,	I_SET_VAR		= 6  ///< arg = int   : assign the top value from the stack to a variable (doesn't pop)
	// Objects
,	I_MEMBER_C		= 7  ///< arg = name  : finds a member of the top of the stack replaces the top of the stack with the member
,	I_LOOP			= -8 ///< arg = int   : loop over the elements of an iterator, which is the *second* element of the stack (this allows for combing the results of multiple iterations)
					     ///<               at the end performs a jump and pops the iterator. note: The second element of the stack must be an iterator!
	// Functions
,	I_CALL			= -7 ///< arg = int, int+ : call the top item of the stack, with the given number of arguments (set with SET_VAR, but in the activation record of the call)
,	I_RET			= -6 ///< return from the current function
	// Simple instructions
,	I_UNARY			= -5 ///< pop 1 value,  apply a function, push the result
,	I_BINARY		= -4 ///< pop 2 values, apply a function, push the result
,	I_TERNARY		= -3 ///< pop 3 values, apply a function, push the result
};

/// Types of unary instructions (taking one argument from the stack)
enum UnaryInstructionType
{	I_ITERATOR_C		///< Make an iterator for a collection
,	I_NEGATE
,	I_NOT
};

/// Types of binary instructions (taking two arguments from the stack)
enum BinaryInstructionType
{	I_ITERATOR_R	///< Make an iterator for a range (two integers)
,	I_MEMBER		///< Member of an object
// Arithmatic
,	I_ADD			///< add     
,	I_SUB			///< subtract
,	I_MUL			///< multiply
,	I_DIV			///< divide  
,	I_MOD			///< modulus
// Logical
,	I_AND			///< logical and			
,	I_OR			///< logical or
// Comparison
,	I_EQ			///< operator ==
,	I_NEQ			///< operator !=
,	I_LT			///< operator <
,	I_GT			///< operator >
,	I_LE			///< operator <=
,	I_GE			///< operator >=
};

/// Types of ternary instructions (taking three arguments from the stack)
enum TernaryInstructionType
{	I_RGB			///< pop r,g,b, push a color value
};

/// An instruction in a script, consists of the opcode and data
/** If the opcode is one of I_UNARY,I_BINARY,I_TERNARY,
 *  Then the instr? member gives the actual instruction to perform
 */
struct Instruction {
	InstructionType instr : 4;
	union {
		unsigned int			data   : 28;
		UnaryInstructionType	instr1 : 28;
		BinaryInstructionType	instr2 : 28;
		TernaryInstructionType	instr3 : 28;
	};
};

// ----------------------------------------------------------------------------- : Variables

/// Return a unique name for a variable to allow for faster loopups
unsigned int string_to_variable(const String& s);

/// Get the name of a vaiable
/** Warning: this function is slow, it should only be used for error messages and such.
 */
String variable_to_string(unsigned int v);

// ----------------------------------------------------------------------------- : Script

/// A script that can be executed
/** Represent a function that returns a single value.
 *  The script is itself a ScriptValue
 */
class Script : public ScriptValue {
  public:
	
	virtual ScriptType type() const;
	virtual String typeName() const;
	virtual ScriptValueP eval(Context& ctx) const;
	virtual ScriptValueP dependencies(Context& ctx, const Dependency&) const;
	
	/// Add an instruction with no data
	void addInstruction(InstructionType t);
	/// Add an instruction with integer data
	void addInstruction(InstructionType t, unsigned int d);
	/// Add an instruction with constant data
	void addInstruction(InstructionType t, const ScriptValueP& c);
	/// Add an instruction with string data
	void addInstruction(InstructionType t, const String& s);
	
	/// Update an instruction to point to the current position
	/** The instruction at pos must be a jumping instruction, it is changed so the current position
	 *  'comes from' pos (in addition to other control flow).
	 *  The position must be a label just before the instruction to be updated.
	 */
	void comeFrom(unsigned int pos);
	/// Get the current instruction position
	unsigned int getLabel() const;
	
	/// Get access to the vector of instructions
	inline vector<Instruction>& getInstructions() { return instructions; }
	/// Get access to the vector of constants
	inline vector<ScriptValueP>& getConstants()   { return constants; }
	
	/// Output the instructions in a human readable format
	String dumpScript() const;
	/// Output an instruction in a human readable format
	String dumpInstr(unsigned int pos, Instruction i) const;
	
  private:
	/// Data of the instructions that make up this script
	vector<Instruction>  instructions;
	/// Constant values that can be referred to from the script
	vector<ScriptValueP> constants;
	
	friend class Context;
};

// ----------------------------------------------------------------------------- : EOF
#endif
