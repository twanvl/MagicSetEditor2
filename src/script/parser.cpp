//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/script.hpp>
#include <script/parser.hpp>
#include <util/error.hpp>
#include <stack>

DECLARE_TYPEOF_COLLECTION(int);

#ifdef __WXMSW__
#define TokenType TokenType_ // some stupid windows header uses our name
#endif

// ----------------------------------------------------------------------------- : Tokenizing : class

enum TokenType
{	TOK_NAME	// abc
,	TOK_INT		// 123
,	TOK_DOUBLE	// 123.0
,	TOK_STRING	// "asdf"
,	TOK_OPER	// + - * / . ;
,	TOK_LPAREN	// ( { [
,	TOK_RPAREN	// ) } ]
,	TOK_NEWLINE	// newline
,	TOK_EOF		// end of input
};

/// Tokens produced by the TokenIterator
struct Token {
	TokenType type;
	String    value;
	
	inline operator == (TokenType     t) const { return type  == t; }
	inline operator != (TokenType     t) const { return type  != t; }
	inline operator == (const String& s) const { return type != TOK_STRING && value == s; }
	inline operator != (const String& s) const { return type == TOK_STRING || value != s; }
};


/// Iterator over a string, one token at a time
class TokenIterator {
  public:
	TokenIterator(const String& str);
	
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
	
  private:
	String input;
	size_t pos;
	vector<Token> buffer;      // buffer of unread tokens, front() = current
	stack<bool>   open_braces; // braces we entered, true if the brace was from a smart string escape
	/// Read the next token, and add it to the buffer
	void addToken();
	/// Read the next token which is a string (after the opening ")
	void addStringToken();
};

// ----------------------------------------------------------------------------- : Characters

bool isAlpha_(Char c) { return isAlpha(c) || c==_('_'); }
bool isAlnum_(Char c) { return isAlnum(c) || c==_('_'); }
bool isOper  (Char c) { return c==_('+') || c==_('-') || c==_('*') || c==_('/') || c==_('!') || c==_('.') ||
                               c==_(':') || c==_('=') || c==_('<') || c==_('>') || c==_(';') || c==_(',');  }
bool isLparen(Char c) { return c==_('(') || c==_('[') || c==_('{'); }
bool isRparen(Char c) { return c==_(')') || c==_(']') || c==_('}'); }
bool isDigitOrDot(Char c) { return isDigit(c) || c==_('.'); }
bool isLongOper(const String& s) { return s==_(":=") || s==_("==") || s==_("!=") || s==_("<=") || s==_(">="); }

// ----------------------------------------------------------------------------- : Tokenizing

TokenIterator::TokenIterator(const String& str)
	: input(str)
	, pos(0)
{}

const Token& TokenIterator::peek(size_t offset) {
	// read the next token until we have enough
	while (buffer.size() <= offset) {
		addToken();
	}
	return buffer[offset];
}

const Token& TokenIterator::read() {
	if (!buffer.empty()) buffer.erase(buffer.begin());
	return peek(0);
}

void TokenIterator::putBack() {
	Token t = {TOK_NEWLINE, _("\n")};
	buffer.insert(buffer.begin(), t);
}

void TokenIterator::addToken() {
	if (pos >= input.size()) {
		// EOF
		Token t = {TOK_EOF, _("end of input")};
		buffer.push_back(t);
		return;
	}
	// read a character from the input
	Char c = input.GetChar(pos++);
	if (c == _('\n')) {
		Token t = {TOK_NEWLINE, _("newline")};
		buffer.push_back(t);
	} else if (isSpace(c)) {
		// ignore
	} else if (isAlpha(c)) {
		// name
		size_t start = pos - 1;
		while (pos < input.size() && isAlnum_(input.GetChar(pos))) ++pos;
		Token t = {TOK_NAME, cannocial_name_form(input.substr(start, pos-start)) }; // convert name to cannocial form
		buffer.push_back(t);
	} else if (isDigit(c)) {
		// number
		size_t start = pos - 1;
		while (pos < input.size() && isDigitOrDot(input.GetChar(pos))) ++pos;
		String num = input.substr(start, pos-start);
		Token t = {
			num.find_first_of('.') == String::npos ? TOK_INT : TOK_DOUBLE,
			num
		};
		buffer.push_back(t);
	} else if (isOper(c)) {
		// operator
		Token t = { TOK_OPER };
		if (pos < input.size() && isLongOper(input.substr(pos - 1, 2))) {
			// long operator
			t.value = input.substr(pos - 1, 2);
			pos += 1;
		} else {
			t.value = input.substr(pos - 1, 1);
		}
		buffer.push_back(t);
	} else if (c==_('"')) {
		// string
		addStringToken();
	} else if (c == _('}') && !open_braces.empty() && open_braces.top()) {
		// closing smart string, resume to string parsing
		//   "a{e}b"  -->  "a"  "{  e  }"  "b"
		open_braces.pop();
		Token t2 = {TOK_RPAREN, _("}\"")};
		buffer.push_back(t2);
		addStringToken();
	} else if (isLparen(c)) {
		// paranthesis/brace
		open_braces.push(false);
		Token t = { TOK_LPAREN, String(1,c) };
		buffer.push_back(t);
	} else if (isRparen(c)) {
		// paranthesis/brace
		if (!open_braces.empty()) open_braces.pop();
		Token t = { TOK_RPAREN, String(1,c) };
		buffer.push_back(t);
	} else if(c==_('#')) {
		// comment untill end of line
		while (pos < input.size() && input[pos] != _('\n')) ++pos;
	} else {
		throw ScriptParseError(_("Unknown character in script: '") + String(1,c) + _("'"));
	}
}

void TokenIterator::addStringToken() {
	Token t = {TOK_STRING};
	while (true) {
		if (pos >= input.size()) throw ScriptParseError(_("Unexpected end of input in string constant"));
		Char c = input[pos++];			//% input.GetChar(pos++);
		// parse the string constant
		if (c == _('"')) {
			// end of string
			buffer.push_back(t);
			return;
		} else if (c == _('\\')) {
			// escape
			if (pos >= input.size()) throw ScriptParseError(_("Unexpected end of input in string constant"));
			c = input[pos++];
			if (c == _('n')) t.value += _('\n');
			if (c == _('<')) t.value += _('\1'); // escape for <
			else             t.value += c;       // \ or { or "
		} else if (c == _('{')) {
			// smart string
			//   "a{e}b"  -->  "a"  "{  e  }"  "b"
			buffer.push_back(t);
			open_braces.push(true);
			Token t2 = {TOK_LPAREN, _("\"{")};
			buffer.push_back(t2);
			return;
		} else {
			t.value += c;
		}
	}
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
,	PREC_UNARY		// - not    (unary operators)
,	PREC_FUN		// [] () .  (function call, member)
,	PREC_STRING		// +{ }+    (smart string operators)
,	PREC_NONE
};

/// Parse an expression
/** @param input   Read tokens from the input
 *  @param scrip   Add resulting instructions to the script
 *  @param minPrec Minimum precedence level for operators
 *  NOTE: The net stack effect of an expression should be +1
 */
void parseExpr(TokenIterator& input, Script& script, Precedence minPrec);

/// Parse an expression, possibly with operators applied. Optionally adds an instruction at the end.
/** @param input     Read tokens from the input
 *  @param scrip     Add resulting instructions to the script
 *  @param minPrec   Minimum precedence level for operators
 *  @param closeWith Add this instruction at the end
 *  @param closeWithData Data for the instruction at the end
 *  NOTE: The net stack effect of an expression should be +1
 */
void parseOper(TokenIterator& input, Script& script, Precedence minPrec, InstructionType closeWith = I_NOP, int closeWithData = 0);

ScriptP parse(const String& s) {
	TokenIterator input(s);
	ScriptP script(new Script);
	parseOper(input, *script, PREC_ALL, I_RET);
	if (input.peek() != TOK_EOF) {
		throw ScriptParseError(_("end of input"), input.peek().value);
	} else {
		return script;
	}
}

// Expect a token, throws if it is not found
void expectToken(TokenIterator& input, const Char* expect) {
	Token token = input.read();
	while (token == TOK_NEWLINE) token = input.read(); // skip newlines
	if (token != expect) {
		throw ScriptParseError(expect, token.value);
	}
}

void parseExpr(TokenIterator& input, Script& script, Precedence minPrec) {
	// usually loop only once, unless we encounter newlines
	while (true) {
		const Token& token = input.read();
		if (token == _("(")) {
			// Parentheses = grouping for precedence of expressions
			parseOper(input, script, PREC_ALL);
			expectToken(input, _(")"));
		} else if (token == _("{")) {
			// {} = function block. Parse a new Script
			intrusive_ptr<Script> subScript(new Script);
			parseOper(input, *subScript, PREC_ALL, I_RET);
			expectToken(input, _("}"));
			script.addInstruction(I_PUSH_CONST, subScript);
		} else if (minPrec <= PREC_UNARY && token == _("-")) {
			parseOper(input, script, PREC_UNARY, I_UNARY, I_NEGATE); // unary negation
		} else if (token == TOK_NAME) {
			if (minPrec <= PREC_UNARY && token == _("not")) {
				parseOper(input, script, PREC_UNARY, I_UNARY, I_NOT); // unary not
			} else if (token == _("true")) {
				script.addInstruction(I_PUSH_CONST, script_true); // boolean constant : true
			} else if (token == _("false")) {
				script.addInstruction(I_PUSH_CONST, script_false); // boolean constant : false
			} else if (token == _("if")) {
				// if AAA then BBB else CCC
				unsigned int jmpElse, jmpEnd;
				parseOper(input, script, PREC_SET);						// AAA
				jmpElse = script.getLabel();							//		jmp_else:
				script.addInstruction(I_JUMP_IF_NOT, 0xFFFF);			//		jnz lbl_else
				expectToken(input, _("then"));							// then
				parseOper(input, script, PREC_SET);						// BBB
				jmpEnd = script.getLabel();								//		jmp_end:
				script.addInstruction(I_JUMP, 0xFFFF);					//		jump lbl_end
				script.comeFrom(jmpElse);								//		lbl_else:
				if (input.read() == _("else")) {						// else
					parseOper(input, script, PREC_SET);					// CCC
				} else {
					script.addInstruction(I_PUSH_CONST, script_nil);
				}
				script.comeFrom(jmpEnd);								//		lbl_end:
			} else if (token == _("for")) {
				unsigned int lblStart;
				// the loop body should have a net stack effect of 0, but the entire expression of +1
				// solution: add all results from the body, start with nil
				if (input.peek() == _("each")) {
					// for each AAA in BBB do CCC
					input.read();										// each
					Token name = input.read();							// AAA
					if (name != TOK_NAME) throw ScriptParseError(_("name"), name.value);
					expectToken(input, _("in"));						// in
					parseOper(input, script, PREC_SET);					// BBB
					script.addInstruction(I_UNARY, I_ITERATOR_C);		//		iterator_collection
					script.addInstruction(I_PUSH_CONST, script_nil);	//		push nil
					lblStart = script.getLabel();						//		lbl_start:
					script.addInstruction(I_LOOP, 0xFFFF);				//		loop
					expectToken(input, _("do"));						// do
					script.addInstruction(I_SET_VAR,
										stringToVariable(name.value));	//		set name
					script.addInstruction(I_POP);						//		 pop
					parseOper(input, script, PREC_SET, I_BINARY, I_ADD);// CCC;	add
					script.addInstruction(I_JUMP, lblStart);			//		jump lbl_start
					script.comeFrom(lblStart);							//		lbl_end:
				} else {
					// for AAA from BBB to CCC do DDD
					Token name = input.read();							// AAA
					expectToken(input, _("from"));						// from
					parseOper(input, script, PREC_SET);					// BBB
					expectToken(input, _("to"));						// to
					parseOper(input, script, PREC_SET);					// CCC
					script.addInstruction(I_BINARY, I_ITERATOR_R);		//		iterator_range
					script.addInstruction(I_PUSH_CONST, script_nil);	//		push nil
					lblStart = script.getLabel();						//		lbl_start:
					script.addInstruction(I_LOOP, 0xFFFF);				//		loop
					expectToken(input, _("do"));						// do
					script.addInstruction(I_SET_VAR,
										stringToVariable(name.value));	//		set name
					script.addInstruction(I_POP);						//		 pop
					parseOper(input, script, PREC_SET, I_BINARY, I_ADD);// DDD;	add
					script.addInstruction(I_JUMP, lblStart);			//		jump lbl_start
					script.comeFrom(lblStart);							//		lbl_end:
				}
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
			} else {
				// variable
				unsigned int var = stringToVariable(token.value);
				script.addInstruction(I_GET_VAR, var);
			}
		} else if (token == TOK_INT) {
			long l = 0;
			//l = lexical_cast<long>(token.value);
			token.value.ToLong(&l);
			script.addInstruction(I_PUSH_CONST, toScript(l));
		} else if (token == TOK_DOUBLE) {
			double d = 0;
			//d = lexical_cast<double>(token.value);
			token.value.ToDouble(&d);
			script.addInstruction(I_PUSH_CONST, toScript(d));
		} else if (token == TOK_STRING) {
			script.addInstruction(I_PUSH_CONST, toScript(token.value));
		} else if (token == TOK_NEWLINE) {
			continue; // ignore
		} else {
			throw ScriptParseError(_("Unexpected token '") + token.value + _("'"));
		}
		break;
	}
}

void parseOper(TokenIterator& input, Script& script, Precedence minPrec, InstructionType closeWith, int closeWithData) {
	parseExpr(input, script, minPrec); // first argument
	bool newlines = false; // did we skip any newlines?
	// read any operators after an expression
	// EBNF:                    expr = expr | expr oper expr
	// without left recursion:  expr = expr (oper expr)*
	while (true) {
		const Token& token = input.read();
		bool newlines2 = newlines;
		newlines = false;
		if (token == TOK_OPER || token == TOK_NAME) {
			if (minPrec <= PREC_SEQ && token==_(";")) {
				Token next = input.peek(1);
				if (next == TOK_RPAREN || next == TOK_EOF) {
					// allow ; at end of expression without errors
					return;
				}
				script.addInstruction(I_POP); // discard result of first expression
				parseOper(input, script, PREC_SET);
			} else if (minPrec <= PREC_SET && token==_(":=")) {
				// We made a mistake, the part before the := should be a variable name,
				// not an expression. Remove that instruction.
				Instruction instr = script.getInstructions().back();
				if (instr.instr != I_GET_VAR) {
					throw ScriptParseError(_("Can only assign to variables"));
				} else {
					script.getInstructions().pop_back();
					parseOper(input, script, PREC_SET,  I_SET_VAR, instr.data);
				}
			}
			else if (minPrec <= PREC_AND    && token==_("and"))   parseOper(input, script, PREC_CMP,   I_BINARY, I_AND);
			else if (minPrec <= PREC_AND    && token==_("or" ))   parseOper(input, script, PREC_CMP,   I_BINARY, I_OR);
			else if (minPrec <= PREC_CMP    && token==_("="))     parseOper(input, script, PREC_ADD,   I_BINARY, I_EQ);
			else if (minPrec <= PREC_CMP    && token==_("=="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_EQ);
			else if (minPrec <= PREC_CMP    && token==_("!="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_NEQ);
			else if (minPrec <= PREC_CMP    && token==_("<"))     parseOper(input, script, PREC_ADD,   I_BINARY, I_LT);
			else if (minPrec <= PREC_CMP    && token==_(">"))     parseOper(input, script, PREC_ADD,   I_BINARY, I_GT);
			else if (minPrec <= PREC_CMP    && token==_("<="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_LE);
			else if (minPrec <= PREC_CMP    && token==_(">="))    parseOper(input, script, PREC_ADD,   I_BINARY, I_GE);
			else if (minPrec <= PREC_ADD    && token==_("+"))     parseOper(input, script, PREC_MUL,   I_BINARY, I_ADD);
			else if (minPrec <= PREC_ADD    && token==_("-"))     parseOper(input, script, PREC_MUL,   I_BINARY, I_SUB);
			else if (minPrec <= PREC_MUL    && token==_("*"))     parseOper(input, script, PREC_UNARY, I_BINARY, I_MUL);
			else if (minPrec <= PREC_MUL    && token==_("/"))     parseOper(input, script, PREC_UNARY, I_BINARY, I_DIV);
			else if (minPrec <= PREC_MUL    && token==_("mod"))   parseOper(input, script, PREC_UNARY, I_BINARY, I_MOD);
			else if (minPrec <= PREC_FUN    && token==_(".")) { // get member by name
				const Token& token = input.read();
				if (token == TOK_NAME || token == TOK_INT || token == TOK_DOUBLE || token == TOK_STRING) {
					script.addInstruction(I_MEMBER_C, token.value);
				} else {
					throw ScriptParseError(_("name"), input.peek().value);
				}
			} else {
				input.putBack();
				newlines = newlines2; // remember newlines
				break; // unknown operator
			}
		} else if (token==TOK_LPAREN) {
			if (minPrec <= PREC_FUN && token==_("(")) {
				// function call, read arguments
				vector<int> arguments;
				Token t = input.peek();
				while (t != _(")")) {
					if (input.peek(2) == _(":")) {
						// name: ...
						arguments.push_back(stringToVariable(t.value));
						input.read(); // skip the name
						input.read(); // and the :
						parseOper(input, script, PREC_SEQ);
					} else {
						// implicit "input" argument
						arguments.push_back(stringToVariable(_("input")));
						parseOper(input, script, PREC_SEQ);
					}
					t = input.peek();
					if (t == _(",")) {
						// Comma separating the arguments
						input.read();
						t = input.peek();
					}
				}
				input.read(); // skip the )
				// generate instruction
				script.addInstruction(I_CALL, (unsigned int)arguments.size());
				FOR_EACH(arg,arguments) {
					script.addInstruction(I_NOP, arg);
				}
			} else if (minPrec <= PREC_FUN && token==_("[")) { // get member by expr
				parseOper(input, script, PREC_ALL, I_BINARY, I_MEMBER);
				expectToken(input, _("]"));
			} else if (minPrec <= PREC_STRING && token==_("\"{")) {
				// for smart strings: "x" {{ e }} "y"
				parseOper(input, script, PREC_ALL,  I_BINARY, I_ADD);	// e
				expectToken(input, _("}\""));
				parseOper(input, script, PREC_NONE, I_BINARY, I_ADD);	// y
			} else {
				input.putBack();
				newlines = newlines2; // remember newlines
				break; // unknown LPAREN, has to be {
			}
		} else if (token == TOK_NEWLINE) {
			const Token& next = input.peek(1);
			if (minPrec <= PREC_NEWLINE && (next == TOK_NAME || next == TOK_LPAREN)) {
				// function as ;
				script.addInstruction(I_POP);
				parseOper(input, script, PREC_SET);
			} else {
				// skip newlines
				newlines = true;
			}
		} else {
			input.putBack();
			newlines = newlines2; // remember newlines
			break;
		}
	}
	if (newlines) {
		// we accidentally ate a newline, restore it
		input.putBack();
	}
	// add closing instruction
	if (closeWith != I_NOP) {
		script.addInstruction(closeWith, closeWithData);
	}
}
