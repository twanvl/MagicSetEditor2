//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "writer.hpp"
#include <util/vector2d.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Writer

Writer::Writer(const OutputStreamP& output)
	: output(output)
	, stream(*output)
	, indentation(0)
	, justOpened(false)
{
	stream.WriteString(BYTE_ORDER_MARK);
}


void Writer::enterBlock(const Char* name) {
	// indenting into a sub-block?
	if (justOpened) {
		writeKey();
		stream.WriteString(_(":\n"));
	}
	// don't write the key yet
	indentation += 1;
	openedKey = name;
	justOpened = true;
}

void Writer::exitBlock() {
	assert(indentation > 0);
	indentation -= 1;
	justOpened = false;
}

void Writer::writeKey() {
	writeIndentation();
	writeUTF8(stream, openedKey);
}
void Writer::writeIndentation() {
	for(int i = 1 ; i < indentation ; ++i) {
		stream.PutChar(_('\t'));
	}
}

// ----------------------------------------------------------------------------- : Handling basic types

void Writer::handle(const String& value) {
	if (!justOpened) {
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
	justOpened = false;
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
	String formated;
	formated.Printf(_("(%.10lf,%.10lf)"), vec.x, vec.y);
	handle(formated);
}
