//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "reader.hpp"
#include <util/vector2d.hpp>
#include <util/error.hpp>
#include <util/io/package_manager.hpp>

// ----------------------------------------------------------------------------- : Reader

Reader::Reader(const InputStreamP& input, const String& filename)
	: indent(0), expected_indent(0), just_opened(false)
	, filename(filename), line_number(0)
	, input(input), stream(*input)
{
	moveNext();
	handleAppVersion();
}

Reader::Reader(const String& filename)
	: indent(0), expected_indent(0), just_opened(false)
	, filename(filename), line_number(0)
	, input(packages.openFileFromPackage(filename)), stream(*input)
{
	moveNext();
	handleAppVersion();
}

void Reader::addAlias(Version end_version, const Char* a, const Char* b) {
	if (file_app_version < end_version) {
		aliasses[a] = b;
	}
}

void Reader::handleAppVersion() {
	if (enterBlock(_("mse_version"))) {
		handle(file_app_version);
		if (app_version < file_app_version) {
			handle_warning(_ERROR_2_("newer version", filename, file_app_version.toString()), false);
		}
		exitBlock();
	}
}

void Reader::warning(const String& msg) {
	warnings += String(_("\nOn line ")) << line_number << _(": \t") << msg;
}

void Reader::showWarnings() {
	if (!warnings.empty()) {
		handle_warning(_("Warnings while reading file:\n") + filename + _("\n") + warnings, false);
		warnings.clear();
	}
}

bool Reader::enterBlock(const Char* name) {
	if (just_opened) moveNext(); // on the key of the parent block, first move inside it
	if (indent != expected_indent) return false; // not enough indentation
	if (cannocial_name_compare(key, name)) {
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
	handled = true;
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
	line_number += 1;
	// read indentation
	indent = 0;
	while ((UInt)indent < line.size() && line.GetChar(indent) == _('\t')) {
		indent += 1;
	}
	// read key / value
	size_t pos = line.find_first_of(_(':'), indent);
	if (trim(line).empty() || line.GetChar(indent) == _('#')) {
		// empty line or comment
		key.clear();
		return;
	}
	if (input->Eof()) {
		// end of file
		indent = -1;
		return;
	}
	key   = cannocial_name_form(trim(line.substr(indent, pos - indent)));
	value = pos == String::npos ? _("") : trim_left(line.substr(pos+1));
}

void Reader::unknownKey() {
	// aliasses?
	map<String,String>::const_iterator it = aliasses.find(key);
	if (it != aliasses.end()) {
		if (aliasses.find(it->second) != aliasses.end()) {
			// alias points to another alias, don't follow it, there is the risk of infinite loops
		} else {
			// try this key instead
			key = it->second;
			return;
		}
	}
	if (indent == expected_indent) {
		warning(_("Unexpected key: '") + key + _("'"));
		do {
			moveNext();
		} while (indent > expected_indent);
	}
	// else: could be a nameless value, which doesn't call exitBlock to move past its own key
}

// ----------------------------------------------------------------------------- : Handling basic types

const String& Reader::getValue() {
	handled = true;
	if (!multi_line_str.empty()) {
		return multi_line_str;
	} else if (value.empty()) {
		// a multiline string
		bool first = true;
		// read all lines that are indented enough
		readLine();
		while (indent >= expected_indent && !input->Eof()) {
			if (!first) multi_line_str += _('\n');
			first = false;
			multi_line_str += line.substr(expected_indent); // strip expected indent
			readLine();
		}
		// moveNext(), but without emptying multi_line_str
		just_opened = false;
		while (key.empty() && !input->Eof()) {
			readLine();
		}
		return multi_line_str;
	} else {
		return value;
	}
}

template <> void Reader::handle(String& s) {
	s = getValue();
}
template <> void Reader::handle(int& i) {
	long l = 0;
	getValue().ToLong(&l);
	i = l;
}
template <> void Reader::handle(unsigned int& i) {
	long l = 0;
	getValue().ToLong(&l);
	i = abs(l); // abs, because it will seem strange if -1 comes out as MAX_INT
}
template <> void Reader::handle(double& d) {
	getValue().ToDouble(&d);
}
template <> void Reader::handle(bool& b) {
	b = (getValue()==_("true") || getValue()==_("1") || getValue()==_("yes"));
}
// ----------------------------------------------------------------------------- : Handling less basic util types

template <> void Reader::handle(Vector2D& vec) {
	if (!wxSscanf(getValue().c_str(), _("(%lf,%lf)"), &vec.x, &vec.y)) {
		throw ParseError(_("Expected (x,y)"));
	}
}

template <> void Reader::handle(Color& col) {
	UInt r,g,b;
	if (wxSscanf(getValue().c_str(),_("rgb(%u,%u,%u)"),&r,&g,&b)) {
		col.Set(r, g, b);
	}
}

template <> void Reader::handle(FileName& f) {
	if (clipboard_package()) {
		String str = getValue();
		if (!str.empty()) {
			// copy file into current package
			try {
				String packaged_name = clipboard_package()->newFileName(_("image"),_("")); // a new unique name in the package, assume it's an image
				OutputStreamP out    = clipboard_package()->openOut(packaged_name);
				InputStreamP  in     = Package::openAbsoluteFile(str);
				out->Write(*in); // copy
				f.assign(packaged_name);
			} catch (Error) {
				// ignore errors
			}
		} else {
			f.assign(str);
		}
	} else {
		handle(static_cast<String&>(f));
	}
}
