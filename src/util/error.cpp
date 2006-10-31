//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Error types

Error::Error(const String& message)
	: message(message)
{}

Error::~Error() {}

String Error::what() const {
	return message;
}

// ----------------------------------------------------------------------------- : Error handling

// Errors for which a message box was already shown
vector<String> previous_errors;
String pending_error;
DECLARE_TYPEOF_COLLECTION(String);

void handle_error(const String& e, bool allow_duplicate = true, bool now = true) {
	// Check duplicates
	if (!allow_duplicate) {
		FOR_EACH(pe, previous_errors) {
			if (e == pe)  return;
		}
		previous_errors.push_back(e);
	}
	// Only show errors in the main thread
	if (!now || !wxThread::IsMain()) {
		if (!pending_error.empty()) pending_error += _("\n\n");
		pending_error += e;
		return;
	}
	// show message
	wxMessageBox(e, _("Error"), wxOK | wxICON_ERROR);
}

void handle_error(const Error& e, bool allow_duplicate, bool now) {
	handle_error(e.what(), allow_duplicate, now);
}

void handle_pending_errors() {
	assert(wxThread::IsMain());
	if (!pending_error.empty()) {
		handle_error(pending_error);
		pending_error.clear();
	}
}
