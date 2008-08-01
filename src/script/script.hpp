//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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
{	I_NOP			= 0  ///< arg = *          : no operation, used as placeholder for extra data values
,	I_PUSH_CONST	= 1  ///< arg = const val  : push a constant onto the stack
,	I_JUMP			= 2  ///< arg = address    : move the instruction pointer to the given position
,	I_JUMP_IF_NOT	= 3  ///< arg = address    : move the instruction pointer if the top of the stack is false
	// Variables
,	I_GET_VAR		= 4  ///< arg = var        : find a variable, push its value onto the stack, it is an error if the variable is not found
,	I_SET_VAR		= 5  ///< arg = var        : assign the top value from the stack to a variable (doesn't pop)
	// Objects
,	I_MEMBER_C		= 6  ///< arg = const name : finds a member of the top of the stack replaces the top of the stack with the member
,	I_LOOP			= 7  ///< arg = address    : loop over the elements of an iterator, which is the *second* element of the stack (this allows for combing the results of multiple iterations)
					     ///<                    at the end performs a jump and pops the iterator. note: The second element of the stack must be an iterator!
,	I_MAKE_OBJECT   = 8  ///< arg = int        : make a list/map with n elements, pops 2n values of the stack, n key/value pairs
	// Functions
,	I_CALL			= 9  ///< arg = int, n*var : call the top item of the stack, with the given number of arguments (set with SET_VAR, but in the activation record of the call)
,	I_CLOSURE       = 10 ///< arg = int, n*var : construct a call closure object with the given arguments
	// Simple instructions
,	I_UNARY			= 11 ///< arg = 1ary instr : pop 1 value,  apply a function, push the result
,	I_BINARY		= 12 ///< arg = 2ary instr : pop 2 values, apply a function, push the result
,	I_TERNARY		= 13 ///< arg = 3ary instr : pop 3 values, apply a function, push the result
,	I_QUATERNARY	= 14 ///< arg = 4ary instr : pop 4 values, apply a function, push the result
,	I_DUP			= 15 ///< arg = int        : duplicate the k-from-top element of the stack
};

/// Types of unary instructions (taking one argument from the stack)
enum UnaryInstructionType
{	I_ITERATOR_C		///< Make an iterator for a collection
,	I_NEGATE
,	I_NOT
};

/// Types of binary instructions (taking two arguments from the stack)
enum BinaryInstructionType
{	I_POP			///< Pop the top value of the stack
,	I_ITERATOR_R	///< Make an iterator for a range (two integers)
,	I_MEMBER		///< Member of an object
// Arithmatic
,	I_ADD			///< add
,	I_SUB			///< subtract
,	I_MUL			///< multiply
,	I_FDIV			///< floating point division
,	I_DIV			///< integer division
,	I_MOD			///< modulus
,	I_POW			///< power
// Logical
,	I_AND			///< logical and
,	I_OR			///< logical or
,	I_XOR			///< logical xor
// Comparison
,	I_EQ			///< operator ==
,	I_NEQ			///< operator !=
,	I_LT			///< operator <
,	I_GT			///< operator >
,	I_LE			///< operator <=
,	I_GE			///< operator >=
,	I_MIN			///< minimum
,	I_MAX			///< maximum
// Error handling
,	I_OR_ELSE		///< if a != error then a else b
};

/// Types of ternary instructions (taking three arguments from the stack)
enum TernaryInstructionType
{	I_RGB			///< pop r,g,b, push a color value
};

/// Types of quaternary instructions (taking four arguments from the stack)
enum QuaternaryInstructionType
{	I_RGBA			///< pop r,g,b,a, push an acolor value
};

/// An instruction in a script, consists of the opcode and data
/** If the opcode is one of I_UNARY,I_BINARY,I_TERNARY,I_QUATERNARY,
 *  Then the instr? member gives the actual instruction to perform
 */
struct Instruction {
	InstructionType instr : 5;
	union {
		unsigned int				data   : 27;
		UnaryInstructionType		instr1 : 27;
		BinaryInstructionType		instr2 : 27;
		TernaryInstructionType		instr3 : 27;
		QuaternaryInstructionType	instr4 : 27;
	};
};

// ----------------------------------------------------------------------------- : Variables

// for faster lookup from code
enum Variable
{	SCRIPT_VAR_input
,	SCRIPT_VAR__1
,	SCRIPT_VAR__2
,	SCRIPT_VAR_in
,	SCRIPT_VAR_match
,	SCRIPT_VAR_replace
,	SCRIPT_VAR_in_context
,	SCRIPT_VAR_recursive
,	SCRIPT_VAR_order
,	SCRIPT_VAR_begin
,	SCRIPT_VAR_end
,	SCRIPT_VAR_filter
,	SCRIPT_VAR_choice
,	SCRIPT_VAR_choices
,	SCRIPT_VAR_format
,	SCRIPT_VAR_tag
,	SCRIPT_VAR_contents
,	SCRIPT_VAR_set
,	SCRIPT_VAR_game
,	SCRIPT_VAR_stylesheet
,	SCRIPT_VAR_card_style
,	SCRIPT_VAR_card
,	SCRIPT_VAR_styling
,	SCRIPT_VAR_value
,	SCRIPT_VAR_condition
,	SCRIPT_VAR_CUSTOM_FIRST // other variables start from here
,	SCRIPT_VAR_CUSTOM_LOTS = 0xFFFFFF // ensure that sizeof(Variable) is large enough
};

/// Return a unique name for a variable to allow for faster loopups
Variable string_to_variable(const String& s);

/// Get the name of a vaiable
/** Warning: this function is slow, it should only be used for error messages and such.
 */
String variable_to_string(Variable v);

/// initialze the script variables
void init_script_variables();


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
	
	/// Add a jump instruction, later comeFrom should be called on the returned value
	unsigned int addInstruction(InstructionType t);
	/// Add an instruction with integer data
	void addInstruction(InstructionType t, unsigned int d);
	/// Add an instruction with constant data
	void addInstruction(InstructionType t, const ScriptValueP& c);
	/// Add an instruction with string data
	void addInstruction(InstructionType t, const String& s);
	
	/// Update an instruction to point to the current position
	/** The instruction at pos must be a jumping instruction, it is changed so the current position
	 *  'comes from' pos (in addition to other control flow).
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
	
	/// Do a backtrace for error messages.
	/** Starting from instr, move backwards until the nett stack effect
	 *  of the skipped instructions is equal to_skip.
	 *  If the backtrace fails, returns nullptr
	 */
	const Instruction* backtraceSkip(const Instruction* instr, int to_skip) const;
	/// Find the name of an instruction
	String instructionName(const Instruction* instr) const;
	
	friend class Context;
};

// ----------------------------------------------------------------------------- : EOF
#endif
