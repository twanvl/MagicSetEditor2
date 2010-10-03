//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include "writer.hpp"
#include <util/vector2d.hpp>
#include <util/error.hpp>
#include <util/version.hpp>
#include <util/io/package.hpp>
#include <boost/logic/tribool.hpp>
using boost::tribool;

// ----------------------------------------------------------------------------- : Writer

Writer::Writer(const OutputStreamP& output, Version file_app_version)
	: indentation(0)
	, output(output), stream(*output)
{
	stream.WriteString(BYTE_ORDER_MARK);
	handle(_("mse_version"), file_app_version);
}



void Writer::enterBlock(const Char* name) {
	// don't write the key yet
	pending_opened.push_back(name);
}

void Writer::exitBlock() {
	if (pending_opened.empty()) {
		assert(indentation > 0);
		indentation -= 1;
	} else {
		// this block was apparently empty, ignore it
		pending_opened.pop_back();
	}
}

void Writer::writePending() {
	// In enterBlock we have delayed the actual writing of the keys until this point
	// here we write all the pending keys, and increase indentation along the way.
	for (size_t i = 0 ; i < pending_opened.size() ; ++i) {
		if (i > 0) {
			// before entering a sub-block, write a colon after the parent's name
			stream.WriteString(_(":\n"));
		}
		indentation += 1;
		writeIndentation();
		writeUTF8(stream, canonical_name_form(pending_opened[i]));
	}
	pending_opened.clear();
}

void Writer::writeIndentation() {
	for(int i = 1 ; i < indentation ; ++i) {
		stream.PutChar(_('\t'));
	}
}

// ----------------------------------------------------------------------------- : Handling basic types

void Writer::handle(const String& value) {
	if (pending_opened.empty()) {
		throw InternalError(_("Can only write a value in a key that was just opened"));
	}
	writePending();
	// write indentation and key
	if (value.find_first_of(_('\n')) != String::npos || (!value.empty() && isSpace(value.GetChar(0)))) {
		// multiline string, or contains leading whitespace
		stream.WriteString(_(":\n"));
		indentation += 1;
		// split lines, and write each line
		size_t start = 0, end, size = value.size();
		while (start < size) {
			end = value.find_first_of(_("\n\r"), start); // until end of line
			// write the line
			writeIndentation();
			writeUTF8(stream, value.substr(start, end - start));
			// Skip \r and \n
			if (end == String::npos) break;
			stream.PutChar(_('\n'));
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
		stream.WriteString(_(": "));
		writeUTF8(stream, value);
	}
	stream.PutChar(_('\n'));
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
template <> void Writer::handle(const tribool& value) {
	if (!indeterminate(value)) {
		handle(value ? _("true") : _("false"));
	}
}

// ----------------------------------------------------------------------------- : Handling less basic util types

template <> void Writer::handle(const wxDateTime& date) {
	if (date.IsValid()) {
		handle(date.Format(_("%Y-%m-%d %H:%M:%S")));
	}
}
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
