//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/action/keyword.hpp>
#include <data/keyword.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <script/parser.hpp>
#include <util/tagged_string.hpp>
#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(KeywordModeP);

// ----------------------------------------------------------------------------- : Add Keyword

AddKeywordAction::AddKeywordAction(Adding, Set& set, const KeywordP& keyword)
	: KeywordListAction(set), adding(true), keyword(keyword ? keyword : new_intrusive<Keyword>())
	, keyword_id(set.keywords.size())
{
	// find default mode
	FOR_EACH(mode, set.game->keyword_modes) {
		if (mode->is_default) {
			this->keyword->mode = mode->name;
			break;
		}
	}
}
AddKeywordAction::AddKeywordAction(Removing, Set& set, const KeywordP& keyword)
	: KeywordListAction(set), adding(false), keyword(keyword)
	// find the keyword_id of the keyword we want to remove
	, keyword_id(find(set.keywords.begin(), set.keywords.end(), keyword) - set.keywords.begin())
{
	if (keyword_id >= set.keywords.size()) {
		throw InternalError(_("Keyword to remove not found in set"));
	}
}

String AddKeywordAction::getName(bool to_undo) const {
	return adding ? _("Add keyword") : _("Remove keyword");
}

void AddKeywordAction::perform(bool to_undo) {
	if (adding != to_undo) {
		assert(keyword_id <= set.keywords.size());
		set.keywords.insert(set.keywords.begin() + keyword_id, keyword);
	} else {
		assert(keyword_id < set.keywords.size());
		set.keywords.erase(set.keywords.begin() + keyword_id);
	}
	set.keyword_db.clear();
}

// ----------------------------------------------------------------------------- : Changing keywords

KeywordReminderTextValue::KeywordReminderTextValue(const TextFieldP& field, Keyword* keyword, bool editable)
	: KeywordTextValue(field, keyword, &keyword->reminder.getUnparsed(), editable)
{}

void KeywordReminderTextValue::store() {
	if (!editable) {
		retrieve();
		return;
	}
	// new value
	String new_value = untag(value);
	// Try to parse the script
	vector<ScriptParseError> parse_errors;
	ScriptP new_script = parse(new_value, nullptr, true, parse_errors);
	if (parse_errors.empty()) {
		// parsed okay, assign
		errors.clear();
		keyword.reminder.getScriptP()  = new_script;
		keyword.reminder.getUnparsed() = new_value;
	} else {
		// parse errors, report
		errors = ScriptParseErrors(parse_errors).what();
	}
	// re-highlight input, show errors
	highlight(new_value, parse_errors);
}

void KeywordReminderTextValue::retrieve() {
	vector<ScriptParseError> no_errors;
	highlight(*underlying, no_errors);
}

void KeywordReminderTextValue::highlight(const String& code, const vector<ScriptParseError>& errors) {
	// Add tags to indicate code / syntax highlight
	// i.e.  bla {if code "x" } bla
	// becomes:
	//       bla <code>{<code-kw>if</code-kw> code "<code-string>x</code-string>" } bla
	String new_value;
	vector<int> in_brace; // types of braces we are in, 0 for code brace, 1 for string escapes
	bool in_string = true; 
	vector<ScriptParseError>::const_iterator error = errors.begin();
	for (size_t pos = 0 ; pos < code.size() ; ) {
		// error underlining
		while (error != errors.end() && error->start == error->end) ++error;
		if (error != errors.end()) {
			if (error->start == pos) {
				new_value += _("<error>");
			}
			if (error->end == pos) {
				++error;
				if (error == errors.end() || error->start > pos) {
					new_value += _("</error>");
				} else {
					// immediatly open again
				}
			}
		}
		// process a character
		Char c = code.GetChar(pos);
		if (c == _('<')) {
			new_value += _('\1'); // escape
			++pos;
		} else if (c == _('{')) {
			if (in_string) {
				new_value += _("<code>");
				in_brace.push_back(1);
				in_string = false;
			} else {
				in_brace.push_back(0);
			}
			new_value += c;
			++pos;
		} else if (c == _('}') && !in_string) {
			new_value += c;
			if (!in_brace.empty() && in_brace.back() == 1) {
				new_value += _("</code>");
				in_string = true;
			}
			if (!in_brace.empty()) in_brace.pop_back();
			++pos;
		} else if (c == _('"')) {
			if (in_string) {
				in_string = false;
				new_value += _("\"</code-str><code>");
			} else {
				in_string = true;
				new_value += _("</code><code-str>\"");
			}
			++pos;
		} else if (c == _('\\') && in_string && pos + 1 < code.size()) {
			new_value += c + code.GetChar(pos + 1); // escape code
			pos += 2;
		} else if (is_substr(code, pos, _("if ")) && !in_string) {
			new_value += _("<code-kw>if</code-kw> ");
			pos += 3;
		} else if (is_substr(code, pos, _("then ")) && !in_string) {
			new_value += _("<code-kw>then</code-kw> ");
			pos += 5;
		} else if (is_substr(code, pos, _("else ")) && !in_string) {
			new_value += _("<code-kw>else</code-kw> ");
			pos += 5;
		} else if (is_substr(code, pos, _("for ")) && !in_string) {
			new_value += _("<code-kw>for</code-kw> ");
			pos += 4;
		} else if (is_substr(code, pos, _("param")) && !in_string) {
			// parameter reference
			size_t end = code.find_first_not_of(_("0123456789"), pos + 5);
			if (end == String::npos) end = code.size();
			String param = code.substr(pos, end-pos);
			new_value += _("<ref-") + param + _(">") + param + _("</ref-") + param + _(">");
			pos = end;
		} else {
			new_value += c;
			++pos;
		}
	}
	// set
	value = new_value;
}

// ----------------------------------------------------------------------------- : Changing keywords : mode

ChangeKeywordModeAction::ChangeKeywordModeAction(Keyword& keyword, const String& new_mode)
	: keyword(keyword), mode(new_mode)
{}

String ChangeKeywordModeAction::getName(bool to_undo) const {
	return _("Keyword mode");
}

void ChangeKeywordModeAction::perform(bool to_undo) {
	swap(keyword.mode, mode);
}
