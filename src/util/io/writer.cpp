//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "writer.hpp"
#include <util/vector2d.hpp>
#include <util/error.hpp>
#include <util/version.hpp>
#include <util/io/package.hpp>

// ----------------------------------------------------------------------------- : Writer

Writer::Writer(const OutputStreamP& output)
	: output(output)
	, stream(*output)
	, indentation(0)
	, just_opened(false)
{
	stream.WriteString(BYTE_ORDER_MARK);
}


void Writer::handleAppVersion() {
	handle(_("mse_version"), app_version);
}

void Writer::enterBlock(const Char* name) {
	// indenting into a sub-block?
	if (just_opened) {
		writeKey();
		stream.WriteString(_(":\n"));
	}
	// don't write the key yet
	indentation += 1;
	opened_key = cannocial_name_form(name);
	just_opened = true;
}

void Writer::exitBlock() {
	assert(indentation > 0);
	indentation -= 1;
	just_opened = false;
}

void Writer::writeKey() {
	writeIndentation();
	writeUTF8(stream, opened_key);
}
void Writer::writeIndentation() {
	for(int i = 1 ; i < indentation ; ++i) {
		stream.PutChar(_('\t'));
	}
}

// ----------------------------------------------------------------------------- : Handling basic types

void Writer::handle(const String& value) {
	if (!just_opened) {
		throw InternalError(_("Can only write a value in a key that was just opened"));
	}
	// write indentation and key
	writeKey();
	writeUTF8(stream, _(": "));
	if (value.find_first_of(_('\n')) != String::npos) {
		// multiline string
		stream.PutChar(_('\n'));
		indentation += 1;
		// split lines, and write each line
		size_t start = 0, end, size = value.size();
		while (start < size) {
			end = value.find_first_of(_("\n\r"), start); // until end of line
			// write the line
			writeIndentation();
			writeUTF8(stream, value.substr(start, end - start));
			stream.PutChar(_('\n'));
			// Skip \r and \n
			if (end == String::npos) break;
			start = end + 1;
			if (start < size) {
				Char c1 = value.GetChar(start - 1);
				Char c2 = value.GetChar(start);
				// skip second character of \r\n or \n\r
				if (c1 != c2 && (c2 == _('\r') || c2 == _('\n')))  start += 1;
			}
		}
		indentation -= 1;
	} else {
		writeUTF8(stream, value);
	}
	stream.PutChar(_('\n'));
	just_opened = false;
}

template <> void Writer::handle(const int& value) {
	handle(String() << value);
}
template <> void Writer::handle(const unsigned int& value) {
	handle(String() << value);
}
template <> void Writer::handle(const double& value) {
	handle(String() << value);
}
template <> void Writer::handle(const bool& value) {
	handle(value ? _("true") : _("false"));
}

// ----------------------------------------------------------------------------- : Handling less basic util types

template <> void Writer::handle(const Vector2D& vec) {
	handle(String::Format(_("(%.10lf,%.10lf)"), vec.x, vec.y));
}
template <> void Writer::handle(const Color& col) {
	handle(String::Format(_("rgb(%u,%u,%u)"), col.Red(), col.Green(), col.Blue()));
}

template <> void Writer::handle(const FileName& value) {
	if (clipboard_package() && !value.empty()) {
		// use absolute names on clipboard
		try {
			handle(clipboard_package()->absoluteName(value));
		} catch (const Error&) {
			// ignore errors
		}
	} else {
		handle(static_cast<const String&>(value));
		if (writing_package()) {
			writing_package()->referenceFile(value);
		}
	}
}