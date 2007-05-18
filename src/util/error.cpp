//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/error.hpp>

DECLARE_TYPEOF_COLLECTION(ScriptParseError);

// ----------------------------------------------------------------------------- : Error types

Error::Error(const String& message)
	: message(message)
{}

Error::~Error() {}

String Error::what() const {
	return message;
}

// ----------------------------------------------------------------------------- : Parse errors

ScriptParseError::ScriptParseError(size_t pos, const String& error)
	: start(pos), end(pos)
	, ParseError(error)
{}
ScriptParseError::ScriptParseError(size_t pos, const String& exp, const String& found)
	: start(pos), end(pos + found.size())
	, ParseError(_("Expected '") + exp + _("' instead of '") + found + _("'"))
{}
String ScriptParseError::what() const {
	return String(_("(")) << (int)start << _("): ") << Error::what();
}

String concat(const vector<ScriptParseError>& errors) {
	String total;
	FOR_EACH_CONST(e, errors) {
		if (!total.empty()) total += _("\n");
		total += e.what();
	}
	return total;
}
ScriptParseErrors::ScriptParseErrors(const vector<ScriptParseError>& errors)
	: ParseError(concat(errors))
{}

// ----------------------------------------------------------------------------- : Error handling

// Errors for which a message box was already shown
vector<String> previous_errors;
String pending_errors;
String pending_warnings;
DECLARE_TYPEOF_COLLECTION(String);
wxCriticalSection crit_error_handling;

void handle_error(const String& e, bool allow_duplicate = true, bool now = true) {
	// Thread safety
	wxCriticalSectionLocker lock(crit_error_handling);
	// Check duplicates
	if (!allow_duplicate) {
		FOR_EACH(pe, previous_errors) {
			if (e == pe) return;
		}
		previous_errors.push_back(e);
	}
	// Only show errors in the main thread
	if (!now || !wxThread::IsMain()) {
		if (!pending_errors.empty()) pending_errors += _("\n\n");
		pending_errors += e;
		return;
	}
	// show message
	wxMessageBox(e, _("Error"), wxOK | wxICON_ERROR);
}

void handle_error(const Error& e, bool allow_duplicate, bool now) {
	handle_error(e.what(), allow_duplicate, now);
}

void handle_warning(const String& w, bool now) {
	// Check duplicates
	// TODO: thread safety
	// Only show errors in the main thread
	if (!now || !wxThread::IsMain()) {
		if (!pending_warnings.empty()) pending_warnings += _("\n\n");
		pending_warnings += w;
		return;
	}
	// show message
	wxMessageBox(w, _("Warning"), wxOK | wxICON_EXCLAMATION);
}

void handle_pending_errors() {
	assert(wxThread::IsMain());
	if (!pending_errors.empty()) {
		handle_error(pending_errors);
		pending_errors.clear();
	}
	if (!pending_warnings.empty()) {
		handle_warning(pending_warnings);
		pending_warnings.clear();
	}
}
