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
	: input(input), filename(filename), line_number(0)
	, indent(0), expected_indent(0), just_opened(false)
	, stream(*input)
{
	moveNext();
}

bool Reader::enterBlock(const Char* name) {
	if (just_opened) moveNext(); // on the key of the parent block, first move inside it
	if (indent != expected_indent) return false; // not enough indentation
	if (name == key) {
		just_opened = true;
		expected_indent += 1; // the indent inside the block must be at least this much
		return true;
	} else {
		return false;
	}
}

void Reader::exitBlock() {
	assert(expected_indent > 0);
	expected_indent -= 1;
	multi_line_str.clear();
	if (just_opened) moveNext(); // leave this key
	// Dump the remainder of the block
	// TODO: issue warnings?
	while (indent > expected_indent) {
		moveNext();
	}
}

void Reader::moveNext() {
	just_opened = false;
	key.clear();
	multi_line_str.clear();
	indent = -1; // if no line is read it never has the expected indentation
	// repeat until we have a good line
	while (key.empty() && !input->Eof()) {
		readLine();
	}
	// did we reach the end of the file?
	if (key.empty() && input->Eof()) {
		line_number += 1;
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
	key   = cannocial_name_form(trim(line.substr(indent, pos - indent)));
	value = pos == String::npos ? _("") : trim_left(line.substr(pos+1));
	// we read a line
	line_number += 1;
	// was it a comment?
	if (!key.empty() && key.GetChar(0) == _('#')) {
		key.clear();
	}
}

// ----------------------------------------------------------------------------- : Handling basic types

template <> void Reader::handle(String& s) {
	if (!multi_line_str.empty()) {
		s = multi_line_str;
	} else if (value.empty()) {
		// a multiline string
		bool first = true;
		// read all lines that are indented enough
		readLine();
		while (indent >= expected_indent) {
			if (!first) value += '\n';
			first = false;
			multi_line_str += line.substr(expected_indent); // strip expected indent
			readLine();
		}
		// moveNext(), but without emptying multiLineStr
		just_opened = false;
		while (key.empty() && !input->Eof()) {
			readLine();
		}
		s = multi_line_str;
	} else {
		s = value;
	}
}
template <> void Reader::handle(int& i) {
	long l = 0;
	value.ToLong(&l);
	i = l;
}
template <> void Reader::handle(unsigned int& i) {
	long l = 0;
	value.ToLong(&l);
	i = abs(l); // abs, because it will seem strange if -1 comes out as MAX_INT
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
