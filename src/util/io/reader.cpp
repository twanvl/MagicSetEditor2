//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "reader.hpp"
#include <util/vector2d.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : Reader

Reader::Reader(const InputStreamP& input, String filename)
	: input(input), filename(filename), lineNumber(0)
	, indent(0), expectedIndent(0), justOpened(false)
	, stream(*input)
{
	moveNext();
}

bool Reader::enterBlock(const Char* name) {
	if (justOpened) moveNext(); // on the key of the parent block, first move inside it
	if (indent != expectedIndent) return false; // not enough indentation
	if (name == key) {
		justOpened = true;
		expectedIndent += 1; // the indent inside the block must be at least this much
		return true;
	} else {
		return false;
	}
}

void Reader::exitBlock() {
	assert(expectedIndent > 0);
	expectedIndent -= 1;
	multiLineStr.clear();
	if (justOpened) moveNext(); // leave this key
	// Dump the remainder of the block
	// TODO: issue warnings?
	while (indent > expectedIndent) {
		moveNext();
	}
}

void Reader::moveNext() {
	justOpened = false;
	key.clear();
	multiLineStr.clear();
	indent = -1; // if no line is read it never has the expected indentation
	// repeat until we have a good line
	while (key.empty() && !input->Eof()) {
		readLine();
	}
	// did we reach the end of the file?
	if (key.empty() && input->Eof()) {
		lineNumber += 1;
		indent = -1;
	}
}

void Reader::readLine() {
	// fix UTF8 in ascii builds; skip BOM
	line = decodeUTF8BOM(stream.ReadLine());
	// read indentation
	indent = 0;
	while ((UInt)indent < line.size() && line.GetChar(indent) == _('\t')) {
		indent += 1;
	}
	// read key / value
	size_t pos = line.find_first_of(_(':'), indent);
	key   = trim(line.substr(indent, pos - indent));
	value = pos == String::npos ? _("") : trimLeft(line.substr(pos+1));
	// we read a line
	lineNumber += 1;
	// was it a comment?
	if (!key.empty() && key.GetChar(0) == _('#')) {
		key.clear();
	}
}

// ----------------------------------------------------------------------------- : Handling basic types

template <> void Reader::handle(String& s) {
	if (!multiLineStr.empty()) {
		s = multiLineStr;
	} else if (value.empty()) {
		// a multiline string
		bool first = true;
		// read all lines that are indented enough
		readLine();
		while (indent >= expectedIndent) {
			if (!first) value += '\n';
			first = false;
			multiLineStr += line.substr(expectedIndent); // strip expected indent
			readLine();
		}
		// moveNext(), but without emptying multiLineStr
		justOpened = false;
		while (key.empty() && !input->Eof()) {
			readLine();
		}
		s = multiLineStr;
	} else {
		s = value;
	}
}
template <> void Reader::handle(int& i) {
	long l = 0;
	value.ToLong(&l);
	i = l;
}
template <> void Reader::handle(double& d) {
	value.ToDouble(&d);
}
template <> void Reader::handle(bool& b) {
	b = (value==_("true") || value==_("1") || value==_("yes"));
}

// ----------------------------------------------------------------------------- : Handling less basic util types

template <> void Reader::handle(Vector2D& vec) {
	if (!wxSscanf(value.c_str(), _("(%lf,%lf)"), &vec.x, &vec.y)) {
		throw ParseError(_("Expected (x,y)"));
	}
}
