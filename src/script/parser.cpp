//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

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
	
	inline bool operator == (TokenType     t) const { return type  == t; }
	inline bool operator != (TokenType     t) const { return type  != t; }
	inline bool operator == (const String& s) const { return type != TOK_STRING && value == s; }
	inline bool operator != (const String& s) const { return type == TOK_STRING || value != s; }
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
	TokenIterator(const String& str, bool string_mode, vector<ScriptParseError>& errors);
	
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
	String filename;				///< Filename of include files, "" for the main input
	vector<Token>    buffer;		///< buffer of unread tokens, front() = current
	stack<OpenBrace> open_braces;	///< braces/quotes we entered from script mode
	bool             newline;		///< Did we just pass a newline?
	// more input?
	struct MoreInput {
		String input;
		size_t pos;
		String filename;
	};
	stack<MoreInput> more;		///< Read tokens from here when we are done with the current input
	
	/// Add a token to the buffer, with the current newline value, resets newline
	void addToken(TokenType type, const String& value);
	/// Read the next token, and add it to the buffer
	void readToken();
	/// Read the next token which is a string (after the opening ")
	void readStringToken();
	
  public:
	/// All errors found
	vector<ScriptParseError>& errors;
	/// Add an error message
	void add_error(const String& message);
	/// Expected some token instead of what was found
	void expected(const String& exp);
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

TokenIterator::TokenIterator(const String& str, bool string_mode, vector<ScriptParseError>& errors)
	: input(str)
	, pos(0)
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

void TokenIterator::addToken(TokenType type, const String& value) {
	Token t = {type, value, newline};
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
			more.pop();
		} else {
			// EOF
			addToken(TOK_EOF, _("end of input"));
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
		MoreInput m = {input, eol, filename};
		more.push(m);
		// read the entire file, and start at the beginning of it
		pos = 0;
		filename = include_file;
		InputStreamP is = packages.openFileFromPackage(include_file);
		input = read_utf8_line(*is, true, true);
	} else if (isAlpha(c)) {
		// name
		size_t start = pos - 1;
		while (pos < input.size() && isAlnum_(input.GetChar(pos))) ++pos;
		addToken(TOK_NAME, cannocial_name_form(input.substr(start, pos-start))); // convert name to cannocial form
	} else if (isDigit(c)) {
		// number
		size_t start = pos - 1;
		while (pos < input.size() && isDigitOrDot(input.GetChar(pos))) ++pos;
		String num = input.substr(start, pos-start);
		addToken(
			num.find_first_of('.') == String::npos ? TOK_INT : TOK_DOUBLE,
			num
		);
	} else if (isOper(c)) {
		// operator
		if (pos < input.size() && isLongOper(input.substr(pos - 1, 2))) {
			// long operator
			addToken(TOK_OPER, input.substr(pos - 1, 2));
			pos += 1;
		} else {
			addToken(TOK_OPER, input.substr(pos - 1, 1));
		}
	} else if (c==_('"')) {
		// string
		open_braces.push(BRACE_STRING);
		readStringToken();
	} else if (c == _('}') && !open_braces.empty() && open_braces.top() != BRACE_PAREN) {
		// closing smart string, resume to string parsing
		//   "a{e}b"  -->  "a"  "{  e  }"  "b"
		addToken(TOK_RPAREN, _("}\""));
		readStringToken();
	} else if (isLparen(c)) {
		// paranthesis/brace
		open_braces.push(BRACE_PAREN);
		addToken(TOK_LPAREN, String(1,c));
	} else if (isRparen(c)) {
		// paranthesis/brace
		if (!open_braces.empty()) open_braces.pop();
		addToken(TOK_RPAREN, String(1,c));
	} else if(c==_('#')) {
		// comment untill end of line
		while (pos < input.size() && input[pos] != _('\n')) ++pos;
	} else {
		add_error(_("Unknown character in script: '") + String(1,c) + _("'"));
		// just skip the character
	}
}

void TokenIterator::readStringToken() {
	String str;
	while (true) {
		if (pos >= input.size()) {
			if (!open_braces.empty() && open_braces.top() == BRACE_STRING_MODE) {
				// in string mode: end of input = end of string
				addToken(TOK_STRING, str);
				return;
			} else {
				add_error(_("Unexpected end of input in string constant"));
				// fix up
				addToken(TOK_STRING, str);
				return;
			}
		}
		Char c = input.GetChar(pos++);
		// parse the string constant
		if (c == _('"') && !open_braces.empty() && open_braces.top() == BRACE_STRING) {
			// end of string
			addToken(TOK_STRING, str);
			open_braces.pop();
			return;
		} else if (c == _('\\')) {
			// escape
			if (pos >= input.size()) {
				add_error(_("Unexpected end of input in string constant"));
				// fix up
				addToken(TOK_STRING, str);
				return;
			}
			c = input.GetChar(pos++);
			if      (c == _('n')) str += _('\n');
			else if (c == _('<')) str += _('\1'); // escape for <
			else                  str += c;       // \ or { or "
		} else if (c == _('{')) {
			// smart string
			//   "a{e}b"  -->  "a"  "{  e  }"  "b"
			addToken(TOK_STRING, str);
			addToken(TOK_LPAREN, _("\"{"));
			return;
		} else {
			str += c;
		}
	}
}


void TokenIterator::add_error(const String& message) {
	if (!errors.empty() && errors.back().start == pos) return; // already an error here
	// find line number
	int line = 1;
	for (size_t i = 0 ; i < input.size() && i < pos ; ++i) {
		if (input.GetChar(i) == _('\n')) line++;
	}
	errors.push_back(ScriptParseError(pos, line, filename, message));
}
void TokenIterator::expected(const String& expected) {
	size_t error_pos = pos - peek(0).value.size();
	if (!errors.empty() && errors.back().start == pos) return; // already an error here
	// find line number
	int line = 1;
	for (size_t i = 0 ; i < input.size() && i < error_pos ; ++i) {
		if (input.GetChar(i) == _('\n')) line++;
	}
	errors.push_back(ScriptParseError(error_pos, line, filename, expected, peek(0).value));
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
 *  @param script    Add resulting instructions to the script
 *  @param minPrec   Minimum precedence level for operators
 *  @param closeWith Add this instruction at the end
 *  @param closeWithData Data for the instruction at the end
 *  NOTE: The net stack effect of an expression should be +1
 */
void parseOper(TokenIterator& input, Script& script, Precedence minPrec, InstructionType closeWith = I_NOP, int closeWithData = 0);


ScriptP parse(const String& s, bool string_mode, vector<ScriptParseError>& errors_out) {
	errors_out.clear();
	// parse
	TokenIterator input(s, string_mode, errors_out);
	ScriptP script(new Script);
	parseOper(input, *script, PREC_ALL, I_RET);
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

ScriptP parse(const String& s, bool string_mode) {
	vector<ScriptParseError> errors;
	ScriptP script = parse(s, string_mode, errors);
	if (!errors.empty()) {
		throw ScriptParseErrors(errors);
	}
	return script;
}


// Expect a token, adds an error if it is not found
bool expectToken(TokenIterator& input, const Char* expect, const Char* name_in_error = nullptr) {
	Token token = input.read();
	if (token == expect) {
		return true;
	} else {
		input.expected(name_in_error ? name_in_error : expect);
		return false;
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
			expectToken(input, _("]"));
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
				unsigned int jmpElse, jmpEnd;
				parseOper(input, script, PREC_AND);						// AAA
				jmpElse = script.getLabel();							//		jmp_else:
				script.addInstruction(I_JUMP_IF_NOT, 0xFFFFFFFF);		//		jnz lbl_else
				expectToken(input, _("then"));							// then
				parseOper(input, script, PREC_SET);						// BBB
				jmpEnd = script.getLabel();								//		jmp_end:
				script.addInstruction(I_JUMP, 0xFFFFFFFF);				//		jump lbl_end
				script.comeFrom(jmpElse);								//		lbl_else:
				if (input.peek() == _("else")) {						// else
					input.read();
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
					if (name != TOK_NAME) {
						input.expected(_("name"));
					}
					expectToken(input, _("in"));						// in
					parseOper(input, script, PREC_AND);					// BBB
					script.addInstruction(I_UNARY, I_ITERATOR_C);		//		iterator_collection
					script.addInstruction(I_PUSH_CONST, script_nil);	//		push nil
					lblStart = script.getLabel();						//		lbl_start:
					script.addInstruction(I_LOOP, 0xFFFFFFFF);			//		loop
					expectToken(input, _("do"));						// do
					script.addInstruction(I_SET_VAR,
										string_to_variable(name.value));//		set name
					script.addInstruction(I_POP);						//		 pop
					parseOper(input, script, PREC_SET, I_BINARY, I_ADD);// CCC;	add
					script.addInstruction(I_JUMP, lblStart);			//		jump lbl_start
					script.comeFrom(lblStart);							//		lbl_end:
				} else {
					// for AAA from BBB to CCC do DDD
					Token name = input.read();							// AAA
					expectToken(input, _("from"));						// from
					parseOper(input, script, PREC_AND);					// BBB
					expectToken(input, _("to"));						// to
					parseOper(input, script, PREC_AND);					// CCC
					script.addInstruction(I_BINARY, I_ITERATOR_R);		//		iterator_range
					script.addInstruction(I_PUSH_CONST, script_nil);	//		push nil
					lblStart = script.getLabel();						//		lbl_start:
					script.addInstruction(I_LOOP, 0xFFFFFFFF);			//		loop
					expectToken(input, _("do"));						// do
					script.addInstruction(I_SET_VAR,
										string_to_variable(name.value));//		set name
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
				unsigned int var = string_to_variable(token.value);
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
	parseExpr(input, script, minPrec); // first argument
	// read any operators after an expression
	// EBNF:                    expr = expr | expr oper expr
	// without left recursion:  expr = expr (oper expr)*
	while (true) {
		const Token& token = input.read();
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
			script.addInstruction(I_POP); // discard result of first expression
			parseOper(input, script, PREC_SET);
		} else if (minPrec <= PREC_SET && token==_(":=")) {
			// We made a mistake, the part before the := should be a variable name,
			// not an expression. Remove that instruction.
			Instruction instr = script.getInstructions().back();
			if (instr.instr != I_GET_VAR) {
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
				parseOper(input, script, PREC_CMP,   I_BINARY, I_OR_ELSE);
			} else {
				parseOper(input, script, PREC_CMP,   I_BINARY, I_OR);
			}
		}
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
		else if (minPrec <= PREC_MUL    && token==_("*"))     parseOper(input, script, PREC_UNARY, I_BINARY, I_MUL);
		else if (minPrec <= PREC_MUL    && token==_("/"))     parseOper(input, script, PREC_UNARY, I_BINARY, I_DIV);
		else if (minPrec <= PREC_MUL    && token==_("mod"))   parseOper(input, script, PREC_UNARY, I_BINARY, I_MOD);
		else if (minPrec <= PREC_FUN    && token==_(".")) { // get member by name
			const Token& token = input.read();
			if (token == TOK_NAME || token == TOK_INT || token == TOK_DOUBLE || token == TOK_STRING) {
				script.addInstruction(I_MEMBER_C, token.value);
			} else {
				input.expected(_("name"));
			}
		} else if (minPrec <= PREC_FUN && token==_("[")) { // get member by expr
			parseOper(input, script, PREC_ALL, I_BINARY, I_MEMBER);
			expectToken(input, _("]"));
		} else if (minPrec <= PREC_FUN && token==_("(")) {
			// function call, read arguments
			vector<Variable> arguments;
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
					arguments.push_back(string_to_variable(_("input")));
					parseOper(input, script, PREC_SEQ);
				}
				t = input.peek();
				if (t == _(",")) {
					// Comma separating the arguments
					input.read();
					t = input.peek();
				}
			}
			expectToken(input, _(")"));
			// generate instruction
			script.addInstruction(I_CALL, (unsigned int)arguments.size());
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
			if (expectToken(input, _("}\""), _("}"))) {
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
			script.addInstruction(I_POP);
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
