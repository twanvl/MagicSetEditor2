//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/script.hpp>
#include <script/parser.hpp>
#include <script/to_value.hpp>
#include <util/error.hpp>
#include <util/io/package_manager.hpp> // for "include file" semi hack
#include <stack>

DECLARE_TYPEOF_COLLECTION(Variable);

#ifdef __WXMSW__
#define TokenType TokenType_ // some stupid windows header uses our name
#endif

String read_utf8_line(wxInputStream& input, bool eat_bom = true, bool until_eof = false);

extern ScriptValueP script_warning;
extern ScriptValueP script_warning_if_neq;

// ----------------------------------------------------------------------------- : Tokenizing : class

enum TokenType
{	TOK_NAME	// abc
,	TOK_INT		// 123
,	TOK_DOUBLE	// 123.0
,	TOK_STRING	// "asdf"
,	TOK_OPER	// + - * / . ;
,	TOK_LPAREN	// ( { [
,	TOK_RPAREN	// ) } ]
,	TOK_DUMMY	// placeholder for putBack
,	TOK_EOF		// end of input
};

/// Tokens produced by the TokenIterator
struct Token {
	TokenType type;
	String    value;
	bool      newline; ///< Is there a newline between this token and the previous one?
	size_t    pos;     ///< Start position of the token
	
	inline bool operator == (TokenType     t) const { return type  == t; }
	inline bool operator != (TokenType     t) const { return type  != t; }
	inline bool operator == (const String& s) const { return type != TOK_STRING && value == s; }
	inline bool operator != (const String& s) const { return type == TOK_STRING || value != s; }
	inline bool operator == (const Char*   s) const { return type != TOK_STRING && value == s; }
	inline bool operator != (const Char*   s) const { return type == TOK_STRING || value != s; }
};

enum OpenBrace 
{	BRACE_STRING		// "
,	BRACE_STRING_MODE	// fake brace for string mode
,	BRACE_PAREN			// (, [, {
};

/// Iterator over a string, one token at a time.
/** Also stores errors found when tokenizing or parsing */
class TokenIterator {
  public:
	TokenIterator(const String& str, Packaged* package, bool string_mode, vector<ScriptParseError>& errors);
	
	/// Peek at the next token, doesn't move to the one after that
	/** Can peek further forward by using higher values of offset.
	 *  offset=0 returns the last token that was read, or newline if putBack() was used.
	 */
	const Token& peek(size_t offset = 1);
	/// Retrieve the next token
	const Token& read();
	/// Put back a token
	/** Only one token can be correctly put back, the put back token will read as a newline.
	 */
	void putBack();
	
	/// Get a section of source code
	/** Known problems: does not work correctly when crossing an include-file border */
	String getSourceCode(size_t start, size_t end);
	/// Get the current line number
	int getLineNumber();
	
  private:
	String input;
	size_t pos;
	String filename;				///< Filename of include files, "" for the main input
	Packaged* package;				///< Package the input is from
	vector<Token>    buffer;		///< buffer of unread tokens, front() = current
	stack<OpenBrace> open_braces;	///< braces/quotes we entered from script mode
	bool             newline;		///< Did we just pass a newline?
	// more input?
	struct MoreInput {
		String input;
		size_t pos;
		String filename;
		Packaged* package;
	};
	stack<MoreInput> more;		///< Read tokens from here when we are done with the current input
	
	/// Add a token to the buffer, with the current newline value, resets newline
	void addToken(TokenType type, const String& value, size_t start);
	/// Read the next token, and add it to the buffer
	void readToken();
	/// Read the next token which is a string (after the opening ")
	void readStringToken();
	
  public:
	/// All errors found
	vector<ScriptParseError>& errors;
	/// Add an error message
	void add_error(const String& message);
	/// Expected some token instead of what was found, possibly a matching opening bracket is known
	void expected(const String& exp, const Token* opening = nullptr);
};

// ----------------------------------------------------------------------------- : Characters

bool isAlpha_(Char c) { return isAlpha(c) || c==_('_'); }
bool isAlnum_(Char c) { return isAlnum(c) || c==_('_'); }
bool isOper  (Char c) { return wxStrchr(_("+-*/!.@%^&:=<>;,"),c) != nullptr; }
bool isLparen(Char c) { return c==_('(') || c==_('[') || c==_('{'); }
bool isRparen(Char c) { return c==_(')') || c==_(']') || c==_('}'); }
bool isDigitOrDot(Char c) { return isDigit(c) || c==_('.'); }
bool isLongOper(const String& s) { return s==_(":=") || s==_("==") || s==_("!=") || s==_("<=") || s==_(">="); }

// ----------------------------------------------------------------------------- : Tokenizing

TokenIterator::TokenIterator(const String& str, Packaged* package, bool string_mode, vector<ScriptParseError>& errors)
	: input(str)
	, pos(0)
	, package(package)
	, newline(false)
	, errors(errors)
{
	if (string_mode) {
		open_braces.push(BRACE_STRING_MODE);
		putBack();//dummy
		readStringToken();
	}
}

const Token& TokenIterator::peek(size_t offset) {
	// read the next token until we have enough
	while (buffer.size() <= offset) {
		readToken();
	}
	return buffer[offset];
}

const Token& TokenIterator::read() {
	if (!buffer.empty()) buffer.erase(buffer.begin());
	return peek(0);
}

void TokenIterator::putBack() {
	// Don't use addToken, because it canges newline
	// Also, we want to push_front
	Token t = {TOK_DUMMY, _(""), false};
	buffer.insert(buffer.begin(), t);
}

void TokenIterator::addToken(TokenType type, const String& value, size_t start) {
	Token t = {type, value, newline, start};
	buffer.push_back(t);
	newline = false;
}

void TokenIterator::readToken() {
	if (pos >= input.size()) {
		// done with input, is there more?
		if (!more.empty()) {
			input    = more.top().input;
			pos      = more.top().pos;
			filename = more.top().filename;
			package  = more.top().package;
			more.pop();
		} else {
			// EOF
			addToken(TOK_EOF, _("end of input"), input.size());
		}
		return;
	}
	// read a character from the input
	Char c = input.GetChar(pos++);
	if (c == _('\n')) {
		newline = true;
	} else if (isSpace(c)) {
		// ignore
	} else if (is_substr(input, pos-1, _("include file:"))) {
		// include a file
		// HACK: This is not really the right place for it, but it's the best I've got
		// filename
		pos += 12; // "nclude file:"
		size_t eol = input.find_first_of(_("\r\n"), pos);
		if (eol == String::npos) eol = input.size();
		String include_file = trim(input.substr(pos, eol - pos));
		// store the current input for later retrieval
		MoreInput m = {input, eol, filename, package};
		more.push(m);
		// read the entire file, and start at the beginning of it
		pos = 0;
		filename = include_file;
		InputStreamP is = package_manager.openFileFromPackage(package, include_file);
		input = read_utf8_line(*is, true, true);
	} else if (isAlpha(c) || c == _('_') || (isDigit(c) && !buffer.empty() && buffer.back() == _("."))) {
		// name, or a number after a . token, as in array.0
		size_t start = pos - 1;
		while (pos < input.size() && isAlnum_(input.GetChar(pos))) ++pos;
		addToken(TOK_NAME, cannocial_name_form(input.substr(start, pos-start)), start); // convert name to cannocial form
	} else if (isDigit(c)) {
		// number
		size_t start = pos - 1;
		while (pos < input.size() && isDigitOrDot(input.GetChar(pos))) ++pos;
		String num = input.substr(start, pos-start);
		addToken(
			num.find_first_of('.') == String::npos ? TOK_INT : TOK_DOUBLE,
			num, start
		);
	} else if (isOper(c)) {
		// operator
		if (pos < input.size() && isLongOper(input.substr(pos - 1, 2))) {
			// long operator
			addToken(TOK_OPER, input.substr(pos - 1, 2), pos-1);
			pos += 1;
		} else {
			addToken(TOK_OPER, input.substr(pos - 1, 1), pos-1);
		}
	} else if (c==_('"')) {
		// string
		open_braces.push(BRACE_STRING);
		readStringToken();
	} else if (c == _('}') && !open_braces.empty() && open_braces.top() != BRACE_PAREN) {
		// closing smart string, resume to string parsing
		//   "a{e}b"  -->  "a"  "{  e  }"  "b"
		addToken(TOK_RPAREN, _("}\""), pos-1);
		readStringToken();
	} else if (isLparen(c)) {
		// paranthesis/brace
		open_braces.push(BRACE_PAREN);
		addToken(TOK_LPAREN, String(1,c), pos-1);
	} else if (isRparen(c)) {
		// paranthesis/brace
		if (!open_braces.empty()) open_braces.pop();
		addToken(TOK_RPAREN, String(1,c), pos-1);
	} else if(c==_('#')) {
		// comment untill end of line
		while (pos < input.size() && input[pos] != _('\n')) ++pos;
	} else {
		add_error(_("Unknown character in script: '") + String(1,c) + _("'"));
		// just skip the character
	}
}

void TokenIterator::readStringToken() {
	size_t start = max((size_t)1, pos) - 1;
	String str;
	while (true) {
		if (pos >= input.size()) {
			if (!open_braces.empty() && open_braces.top() == BRACE_STRING_MODE) {
				// in string mode: end of input = end of string
				addToken(TOK_STRING, str, start);
				return;
			} else {
				add_error(_("Unexpected end of input in string constant"));
				// fix up
				addToken(TOK_STRING, str, start);
				return;
			}
		}
		Char c = input.GetChar(pos++);
		// parse the string constant
		if (c == _('"') && !open_braces.empty() && open_braces.top() == BRACE_STRING) {
			// end of string
			addToken(TOK_STRING, str, start);
			open_braces.pop();
			return;
		} else if (c == _('\\')) {
			// escape
			if (pos >= input.size()) {
				add_error(_("Unexpected end of input in string constant"));
				// fix up
				addToken(TOK_STRING, str, start);
				return;
			}
			c = input.GetChar(pos++);
			if      (c == _('n')) str += _('\n');
			else if (c == _('<')) str += _('\1'); // escape for <
			else if (c == _('\\') || c == _('"') || c == _('{') || c == _('}')) {
				str += c;       // \ or { or "
			} else {
				add_error(String::Format(_("Invalid string escape sequence: \"\\%c\""),c));
				str += _('\\') + c; // ignore
			}
		} else if (c == _('{')) {
			// smart string
			//   "a{e}b"  -->  "a"  "{  e  }"  "b"
			addToken(TOK_STRING, str, start);
			addToken(TOK_LPAREN, _("\"{"), pos-1);
			return;
		} else {
			str += c;
		}
	}
}

int line_number(size_t pos, const String& input) {
	int line = 1;
	for (size_t i = 0 ; i < input.size() && i < pos ; ++i) {
		if (input.GetChar(i) == _('\n')) line++;
	}
	return line;
}

void TokenIterator::add_error(const String& message) {
	if (!errors.empty() && errors.back().start == pos) return; // already an error here
	// add error message
	errors.push_back(ScriptParseError(pos, line_number(pos,input), filename, message));
}
void TokenIterator::expected(const String& expected, const Token* opening) {
	size_t error_pos = peek(0).pos;
	if (!errors.empty() && errors.back().start == pos) return; // already an error here
	// add error message
	if (opening) {
		errors.push_back(ScriptParseError(opening->pos, error_pos, line_number(opening->pos,input), filename, opening->value, expected, peek(0).value));
	} else {
		errors.push_back(ScriptParseError(error_pos, line_number(error_pos,input), filename, expected, peek(0).value));
	}
}


String TokenIterator::getSourceCode(size_t start, size_t end) {
	start = min(start, input.size());
	end   = min(end,   input.size());
	return input.substr(start, end-start);
}
int TokenIterator::getLineNumber() {
	return line_number(peek(0).pos, input);
}

// ----------------------------------------------------------------------------- : Parsing


/// Precedence levels for parsing, higher = tighter
enum Precedence
{	PREC_ALL
,	PREC_NEWLINE	// newline ;
,	PREC_SEQ		// ;
,	PREC_SET		// :=
,	PREC_AND		// and or
,	PREC_CMP		// == != < > <= >=
,	PREC_ADD		// + -
,	PREC_MUL		// * / mod
,	PREC_POW		// ^		(right associative)
,	PREC_UNARY		// - not    (unary operators)
,	PREC_FUN		// [] () .  (function call, member)
,	PREC_STRING		// +{ }+    (smart string operators)
,	PREC_NONE
};

/// Parse an expression
/** @param input    Read tokens from the input
 *  @param scrip    Add resulting instructions to the script
 *  @param min_prec Minimum precedence level for operators
 *  NOTE: The net stack effect of an expression should be +1
 */
void parseExpr(TokenIterator& input, Script& script, Precedence min_prec);

/// Parse an expression, possibly with operators applied. Optionally adds an instruction at the end.
/** @param input           Read tokens from the input
 *  @param script          Add resulting instructions to the script
 *  @param min_prec        Minimum precedence level for operators
 *  @param close_with      Add this instruction at the end, or I_NOP for no instruction
 *  @param close_with_data Data for the instruction at the end
 *  NOTE: The net stack effect of an expression should be +1
 */
void parseOper(TokenIterator& input, Script& script, Precedence min_prec, InstructionType close_with = I_NOP, int close_with_data = 0);

/// Parse call arguments, "(...)"
void parseCallArguments(TokenIterator& input, Script& script, vector<Variable>& arguments);


ScriptP parse(const String& s, Packaged* package, bool string_mode, vector<ScriptParseError>& errors_out) {
	errors_out.clear();
	// parse
	TokenIterator input(s, package, string_mode, errors_out);
	ScriptP script(new Script);
	parseOper(input, *script, PREC_ALL);
	Token eof = input.read();
	if (eof != TOK_EOF) {
		input.expected(_("end of input"));
	}
	// were there errors?
	if (errors_out.empty()) {
		return script;
	} else {
		return ScriptP();
	}
}

ScriptP parse(const String& s, Packaged* package, bool string_mode) {
	vector<ScriptParseError> errors;
	ScriptP script = parse(s, package, string_mode, errors);
	if (!errors.empty()) {
		throw ScriptParseErrors(errors);
	}
	return script;
}


// Expect a token, adds an error if it is not found
bool expectToken(TokenIterator& input, const Char* expect, const Token* opening = nullptr, const Char* name_in_error = nullptr) {
	Token token = input.read();
	if (token == expect) {
		return true;
	} else {
		input.expected(name_in_error ? name_in_error : expect, opening);
		return false;
	}
}

void parseExpr(TokenIterator& input, Script& script, Precedence minPrec) {
	// usually loop only once, unless we encounter newlines
	while (true) {
		Token token = input.read();
		if (token == _("(")) {
			// Parentheses = grouping for precedence of expressions
			parseOper(input, script, PREC_ALL);
			expectToken(input, _(")"), &token);
		} else if (token == _("{")) {
			// {} = function block. Parse a new Script
			intrusive_ptr<Script> subScript(new Script);
			parseOper(input, *subScript, PREC_ALL);
			expectToken(input, _("}"), &token);
			script.addInstruction(I_PUSH_CONST, subScript);
		} else if (token == _("[")) {
			// [] = list or map literal
			unsigned int count = 0;
			Token t = input.peek();
			while (t != _("]") && t != TOK_EOF) {
				if (input.peek(2) == _(":") && (t.type == TOK_NAME || t.type == TOK_INT || t.type == TOK_STRING)) {
					// name: ...
					script.addInstruction(I_PUSH_CONST, to_script(t.value));
					input.read(); // skip the name
					input.read(); // and the :
				} else {
					// implicit numbered element
					script.addInstruction(I_PUSH_CONST, script_nil);
				}
				parseOper(input, script, PREC_AND);
				++count;
				t = input.peek();
				if (t == _(",")) {
					// Comma separating the elements
					input.read();
					t = input.peek();
				}
			}
			expectToken(input, _("]"), &token);
			script.addInstruction(I_MAKE_OBJECT, count);
		} else if (minPrec <= PREC_UNARY && token == _("-")) {
			parseOper(input, script, PREC_UNARY, I_UNARY, I_NEGATE); // unary negation
		} else if (token == TOK_NAME) {
			if (minPrec <= PREC_UNARY && token == _("not")) {
				parseOper(input, script, PREC_UNARY, I_UNARY, I_NOT); // unary not
			} else if (token == _("true")) {
				script.addInstruction(I_PUSH_CONST, script_true); // boolean constant : true
			} else if (token == _("false")) {
				script.addInstruction(I_PUSH_CONST, script_false); // boolean constant : false
			} else if (token == _("nil")) {
				script.addInstruction(I_PUSH_CONST, script_nil); // universal constant : nil
			} else if (token == _("if")) {
				// if AAA then BBB else CCC
				parseOper(input, script, PREC_AND);							// AAA
				unsigned jmpElse = script.addInstruction(I_JUMP_IF_NOT);	//		jnz lbl_else
				expectToken(input, _("then"));								// then
				parseOper(input, script, PREC_SET);							// BBB
				unsigned jmpEnd = script.addInstruction(I_JUMP);			//		jump lbl_end
				script.comeFrom(jmpElse);									//		lbl_else:
				if (input.peek() == _("else")) {							//else
					input.read();
					parseOper(input, script, PREC_SET);						// CCC
				} else {
					script.addInstruction(I_PUSH_CONST, script_nil);
				}
				script.comeFrom(jmpEnd);									//		lbl_end:
			} else if (token == _("for")) {
				// the loop body should have a net stack effect of 0, but the entire expression of +1
				// solution: add all results from the body, start with nil
				bool is_each = input.peek() == _("each");
				if (is_each) {
					// for each AAA(:BBB) in CCC do EEE
					input.read();										// each?
				} else {
					// for AAA(:BBB) from CCC to DDD do EEE
				}
				// name
				Token name = input.read();								// AAA
				if (name != TOK_NAME) {
					input.expected(_("name"));
				}
				Variable var = string_to_variable(name.value);
				// key:value?
				bool with_key = input.peek() == _(":");
				Variable key = (Variable)-1;
				if (with_key) {
					input.read();										// :
					name = input.read();								// BBB
					if (name != TOK_NAME) {
						input.expected(_("name"));
					}
					key = string_to_variable(name.value);
					swap(var,key);
				}
				// iterator
				if (is_each) {
					expectToken(input, _("in"));						// in
					parseOper(input, script, PREC_AND);					// CCC
					script.addInstruction(I_UNARY, I_ITERATOR_C);		//		iterator_collection
				} else {
					expectToken(input, _("from"));						// from
					parseOper(input, script, PREC_AND);					// CCC
					expectToken(input, _("to"));						// to
					parseOper(input, script, PREC_AND);					// DDD
					script.addInstruction(I_BINARY, I_ITERATOR_R);		//		iterator_range
				}
				script.addInstruction(I_PUSH_CONST, script_nil);		//		push nil
				unsigned lblStart = script.addInstruction(with_key
										? I_LOOP_WITH_KEY				//		lbl_start: loop_with_key lbl_end
										: I_LOOP);						//		lbl_start: loop lbl_end
				expectToken(input, _("do"));							// do
				if (with_key) {
					script.addInstruction(I_SET_VAR, key);				//		set key_name
					script.addInstruction(I_BINARY, I_POP);				//		 pop
				}
				script.addInstruction(I_SET_VAR, var);					//		set name
				script.addInstruction(I_BINARY, I_POP);					//		 pop
				parseOper(input, script, PREC_SET, I_BINARY, I_ADD);	// EEE;	add
				script.addInstruction(I_JUMP, lblStart);				//		jump lbl_start
				script.comeFrom(lblStart);								//		lbl_end:
			} else if (token == _("rgb")) {
				// rgb(r, g, b)
				expectToken(input, _("("));
				parseOper(input, script, PREC_ALL); // r
				expectToken(input, _(","));
				parseOper(input, script, PREC_ALL); // g
				expectToken(input, _(","));
				parseOper(input, script, PREC_ALL); // b
				expectToken(input, _(")"));
				script.addInstruction(I_TERNARY, I_RGB);
			} else if (token == _("rgba")) {
				// rgba(r, g, b, a)
				expectToken(input, _("("));
				parseOper(input, script, PREC_ALL); // r
				expectToken(input, _(","));
				parseOper(input, script, PREC_ALL); // g
				expectToken(input, _(","));
				parseOper(input, script, PREC_ALL); // b
				expectToken(input, _(","));
				parseOper(input, script, PREC_ALL); // a
				expectToken(input, _(")"));
				script.addInstruction(I_QUATERNARY, I_RGBA);
			} else if (token == _("min") || token == _("max")) {
				// min(x,y,z,...)
				unsigned int op = token == _("min") ? I_MIN : I_MAX;
				expectToken(input, _("("));
				parseOper(input, script, PREC_ALL); // first
				while(input.peek() == _(",")) {
					expectToken(input, _(","));
					parseOper(input, script, PREC_ALL); // second, third, etc.
					script.addInstruction(I_BINARY, op);
				}
				expectToken(input, _(")"), &token);
			} else if (token == _("assert")) {
				// assert(condition)
				expectToken(input, _("("));
				size_t start = input.peek().pos;
				int line = input.getLineNumber();
				size_t function_pos = script.getConstants().size();
				script.addInstruction(I_PUSH_CONST, script_warning);
				parseOper(input, script, PREC_ALL); // condition
				size_t end = input.peek().pos;
				String message = String::Format(_("Assertion failure on line %d:\n   expected: "), line) + input.getSourceCode(start,end);
				expectToken(input, _(")"), &token);
				if (script.getInstructions().back().instr == I_BINARY && script.getInstructions().back().instr2 == I_EQ) {
					// compile "assert(x == y)" into
					//    warning_if_neq("condition", _1: x, _2: y)
					message += _("\n   found: ");
					script.getConstants()[function_pos] = script_warning_if_neq;
					script.getInstructions().pop_back();						// POP == instruction
					script.addInstruction(I_PUSH_CONST, message);				// push "condition"
					script.addInstruction(I_CALL, 3);							// call
					script.addInstruction(I_NOP,  SCRIPT_VAR__1);				// (_1:)
					script.addInstruction(I_NOP,  SCRIPT_VAR__2);				// (_2:)
					script.addInstruction(I_NOP,  SCRIPT_VAR_input);			// (input:)
				} else {
					// compile into:  warning("condition", condition: not condition)
					script.addInstruction(I_UNARY, I_NOT);						// not
					script.addInstruction(I_PUSH_CONST, message);				// push "condition"
					script.addInstruction(I_CALL, 2);							// call
					script.addInstruction(I_NOP,  SCRIPT_VAR_condition);		// (condition:)
					script.addInstruction(I_NOP,  SCRIPT_VAR_input);			// (input:)
				}
			} else {
				// variable
				Variable var = string_to_variable(token.value);
				script.addInstruction(I_GET_VAR, var);
			}
		} else if (token == TOK_INT) {
			long l = 0;
			//l = lexical_cast<long>(token.value);
			token.value.ToLong(&l);
			script.addInstruction(I_PUSH_CONST, to_script(l));
		} else if (token == TOK_DOUBLE) {
			double d = 0;
			//d = lexical_cast<double>(token.value);
			token.value.ToDouble(&d);
			script.addInstruction(I_PUSH_CONST, to_script(d));
		} else if (token == TOK_STRING) {
			script.addInstruction(I_PUSH_CONST, to_script(token.value));
		} else {
			input.expected(_("expression"));
			return;
		}
		break;
	}
}

void parseOper(TokenIterator& input, Script& script, Precedence minPrec, InstructionType closeWith, int closeWithData) {
	size_t added = script.getInstructions().size(); // number of instructions added
	parseExpr(input, script, minPrec); // first argument
	added = script.getInstructions().size() - added;
	// read any operators after an expression
	// EBNF:                    expr = expr | expr oper expr
	// without left recursion:  expr = expr (oper expr)*
	while (true) {
		Token token = input.read();
		if (token != TOK_OPER && token != TOK_NAME && token!=TOK_LPAREN &&
		    !((token == TOK_STRING || token == TOK_INT || token == TOK_DOUBLE) && minPrec <= PREC_NEWLINE && token.newline)) {
			// not an operator-like token
			input.putBack();
			break;
		}
		if (minPrec <= PREC_SEQ && token==_(";")) {
			Token next = input.peek(1);
			if (next == TOK_RPAREN || next == TOK_EOF) {
				// allow ; at end of expression without errors
				break;
			}
			script.addInstruction(I_BINARY, I_POP); // discard result of first expression
			parseOper(input, script, PREC_SET);
		} else if (minPrec <= PREC_SET && token==_(":=")) {
			// We made a mistake, the part before the := should be a variable name,
			// not an expression. Remove that instruction.
			Instruction& instr = script.getInstructions().back();
			if (added != 1 || instr.instr != I_GET_VAR) {
				input.add_error(_("Can only assign to variables"));
			}
			script.getInstructions().pop_back();
			parseOper(input, script, PREC_SET,  I_SET_VAR, instr.data);
		}
		else if (minPrec <= PREC_AND    && token==_("and"))   parseOper(input, script, PREC_CMP,   I_BINARY, I_AND);
		else if (minPrec <= PREC_AND    && token==_("or" )) {
			Token t = input.peek();
			if (t == _("else")) {// or else
				input.read(); // skip else
				parseOper(input, script, PREC_ADD,   I_BINARY, I_OR_ELSE);
			} else {
				parseOper(input, script, PREC_CMP,   I_BINARY, I_OR);
			}
		}
		else if (minPrec <= PREC_AND    && token==_("xor"))   parseOper(input, script, PREC_CMP,   I_BINARY, I_XOR);
		else if (minPrec <= PREC_CMP    && token==_("=")) {
			if (minPrec <= PREC_SET) {
				input.add_error(_("Use of '=', did you mean ':=' or '=='?"));
			}
			parseOper(input, script, PREC_ADD,   I_BINARY, I_EQ);
		}
		else if (minPrec <= PREC_CMP    && token==_("=="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_EQ);
		else if (minPrec <= PREC_CMP    && token==_("!="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_NEQ);
		else if (minPrec <= PREC_CMP    && token==_("<"))     parseOper(input, script, PREC_ADD,   I_BINARY, I_LT);
		else if (minPrec <= PREC_CMP    && token==_(">"))     parseOper(input, script, PREC_ADD,   I_BINARY, I_GT);
		else if (minPrec <= PREC_CMP    && token==_("<="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_LE);
		else if (minPrec <= PREC_CMP    && token==_(">="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_GE);
		else if (minPrec <= PREC_ADD    && token==_("+"))     parseOper(input, script, PREC_MUL,   I_BINARY, I_ADD);
		else if (minPrec <= PREC_ADD    && token==_("-"))     parseOper(input, script, PREC_MUL,   I_BINARY, I_SUB);
		else if (minPrec <= PREC_MUL    && token==_("*"))     parseOper(input, script, PREC_POW,   I_BINARY, I_MUL);
		else if (minPrec <= PREC_MUL    && token==_("/"))     parseOper(input, script, PREC_POW,   I_BINARY, I_FDIV);
		else if (minPrec <= PREC_MUL    && token==_("div"))   parseOper(input, script, PREC_POW,   I_BINARY, I_DIV);
		else if (minPrec <= PREC_MUL    && token==_("mod"))   parseOper(input, script, PREC_POW,   I_BINARY, I_MOD);
		else if (minPrec <= PREC_POW    && token==_("^"))     parseOper(input, script, PREC_POW,   I_BINARY, I_POW);
		else if (minPrec <= PREC_FUN    && token==_(".")) { // get member by name
			input.peek(1); // peek ahead, so the next token can see the preceding "."
			               // that forces the next token to always be a TOK_NAME instead of TOK_INT/TOK_DOUBLE
			               // (this is a bit of a hack)
			const Token& token = input.read();
			if (token == TOK_NAME || token == TOK_STRING) {
				script.addInstruction(I_MEMBER_C, token.value);
			} else {
				input.expected(_("name"));
			}
		} else if (minPrec <= PREC_FUN && token==_("[") && !token.newline) { // get member by expr
			size_t before = script.getInstructions().size();
			parseOper(input, script, PREC_SET);
			if (script.getInstructions().size() == before + 1 && script.getInstructions().back().instr == I_PUSH_CONST) {
				// optimize:
				//   PUSH_CONST x
				//   MEMBER
				// becomes
				//   MEMBER_CONST x
				script.getInstructions().back().instr = I_MEMBER_C;
			} else {
				script.addInstruction(I_BINARY, I_MEMBER);
			}
			expectToken(input, _("]"), &token);
		} else if (minPrec <= PREC_FUN && token==_("(") && !token.newline) {
			// function call, read arguments
			vector<Variable> arguments;
			parseCallArguments(input, script, arguments);
			expectToken(input, _(")"), &token);
			// generate instruction
			script.addInstruction(I_CALL, (unsigned int)arguments.size());
			FOR_EACH(arg,arguments) {
				script.addInstruction(I_NOP, arg);
			}
		} else if (minPrec <= PREC_FUN && token==_("@")) {
			// closure call, read arguments
			vector<Variable> arguments;
			expectToken(input, _("("));
			parseCallArguments(input, script, arguments);
			expectToken(input, _(")"), &token);
			// generate instruction
			script.addInstruction(I_CLOSURE, (unsigned int)arguments.size());
			FOR_EACH(arg,arguments) {
				script.addInstruction(I_NOP, arg);
			}
		} else if (minPrec <= PREC_STRING && token==_("\"{")) {
			// for smart strings: "x" {{ e }} "y"
			// optimize: "" + e  ->  e
			Instruction i = script.getInstructions().back();
			if (i.instr == I_PUSH_CONST && script.getConstants()[i.data]->toString().empty()) {
				script.getInstructions().pop_back();
				parseOper(input, script, PREC_ALL);						// e
			} else {
				parseOper(input, script, PREC_ALL, I_BINARY, I_ADD);	// e
			}
			if (expectToken(input, _("}\""), &token, _("}"))) {
				parseOper(input, script, PREC_NONE);					// y
				// optimize: e + ""  -> e
				i = script.getInstructions().back();
				if (i.instr == I_PUSH_CONST && script.getConstants()[i.data]->toString().empty()) {
					script.getInstructions().pop_back();
				} else {
					script.addInstruction(I_BINARY, I_ADD);
				}
			}
		} else if (minPrec <= PREC_NEWLINE && token.newline) {
			// newline functions as ;
			// only if we don't match another token!
			input.putBack();
			script.addInstruction(I_BINARY, I_POP);
			parseOper(input, script, PREC_SET);
		} else {
			input.putBack();
			break;
		}
	}
	// add closing instruction
	if (closeWith != I_NOP) {
		script.addInstruction(closeWith, closeWithData);
	}
}

void parseCallArguments(TokenIterator& input, Script& script, vector<Variable>& arguments) {
	Token t = input.peek();
	while (t != _(")") && t != TOK_EOF) {
		if (input.peek(2) == _(":") && t.type == TOK_NAME) {
			// name: ...
			arguments.push_back(string_to_variable(t.value));
			input.read(); // skip the name
			input.read(); // and the :
			parseOper(input, script, PREC_SEQ);
		} else {
			// implicit "input" argument
			arguments.push_back(SCRIPT_VAR_input);
			parseOper(input, script, PREC_SEQ);
		}
		t = input.peek();
		if (t == _(",")) {
			// Comma separating the arguments
			input.read();
			t = input.peek();
		}
	}
}
