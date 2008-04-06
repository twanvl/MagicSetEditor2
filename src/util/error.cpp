//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
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

ScriptParseError::ScriptParseError(size_t pos, int line, const String& filename, const String& error)
	: ParseError(error)
	, start(pos), end(pos), line(line), filename(filename)
{}
ScriptParseError::ScriptParseError(size_t pos, int line, const String& filename, const String& exp, const String& found)
	: ParseError(_("Expected '") + exp + _("' instead of '") + found + _("'"))
	, start(pos), end(pos + found.size()), line(line), filename(filename)
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
vector<String> previous_warnings;
String pending_errors;
String pending_warnings;
DECLARE_TYPEOF_COLLECTION(String);
wxCriticalSection crit_error_handling;

void show_pending_errors();
void show_pending_warnings();

void handle_error(const String& e, bool allow_duplicate = true, bool now = true) {
	{
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
		if (!pending_errors.empty()) pending_errors += _("\n\n");
		pending_errors += e;
	}
	// show messages
	if (now && wxThread::IsMain()) {
		show_pending_warnings(); // warnings are older, show them first
		show_pending_errors();
	}
}

void handle_error(const Error& e, bool allow_duplicate, bool now) {
	handle_error(e.what(), allow_duplicate, now);
}

void handle_warning(const String& w, bool now) {
	{
		// Check duplicates
		wxCriticalSectionLocker lock(crit_error_handling);
		// Check duplicates
		FOR_EACH(pw, previous_warnings) {
			if (w == pw) return;
		}
		previous_warnings.push_back(w);
		// Only show errors in the main thread
		if (!pending_warnings.empty()) pending_warnings += _("\n\n");
		pending_warnings += w;
	}
	// show messages
	if (now && wxThread::IsMain()) {
		show_pending_errors();
		show_pending_warnings();
	}
}

void handle_pending_errors() {
	show_pending_errors();
	show_pending_warnings();
}

void show_pending_errors() {
	assert(wxThread::IsMain());
	wxCriticalSectionLocker lock(crit_error_handling);
	if (!pending_errors.empty()) {
		wxMessageBox(pending_errors, _("Error"), wxOK | wxICON_ERROR);
		pending_errors.clear();
	}
}
void show_pending_warnings() {
	assert(wxThread::IsMain());
	wxCriticalSectionLocker lock(crit_error_handling);
	if (!pending_warnings.empty()) {
		wxMessageBox(pending_warnings, _("Warning"), wxOK | wxICON_EXCLAMATION);
		pending_warnings.clear();
	}
}
