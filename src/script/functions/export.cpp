//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <data/symbol_font.hpp>
#include <util/tagged_string.hpp>
#include <util/error.hpp>

// ----------------------------------------------------------------------------- : HTML

// An HTML tag
struct Tag {
	Tag(const Char* open_tag, const Char* close_tag)
		: open_tag(open_tag), close_tag(close_tag), opened(0)
	{}
	const Char* open_tag;	///< The tags to insert in HTML "<tag>"
	const Char* close_tag;	///< The tags to insert in HTML "</tag>"
	int opened;				///< How often is the tag opened in the input?
	/// Write an open or close tag to a string if needed
	void write(String& ret, bool close) {
		if (close) {
			if (--opened == 0) {
				ret += close_tag;
			}
		} else {
			if (++opened == 1) {
				ret += open_tag;
			}
		}
	}
};

// A tag, or a close tag
struct NegTag {
	Tag* tag;
	bool neg; // a close tag instead of an open tag
	NegTag(Tag* tag, bool neg) : tag(tag), neg(neg) {}
};

DECLARE_TYPEOF_COLLECTION(NegTag);

/// A stack of opened HTML tags
class TagStack {
  public:
	void open(String& ret, Tag& tag) {
		add(ret, NegTag(&tag, false));
	}
	void close(String& ret, Tag& tag) {
		add(ret, NegTag(&tag, true));
	}
	// Close all tags, should be called at end of input
	void close_all(String& ret) {
		pending_tags.clear();
		while (!tags.empty()) {
			tags.back()->write(ret, true);
			tags.pop_back();
		}
	}
	// Write all pending tags, should be called before non-tag output
	void write_pending_tags(String& ret) {
		FOR_EACH(t, pending_tags) {
			t.tag->write(ret, t.neg);
			if (!t.neg) tags.push_back(t.tag);
		}
		pending_tags.clear();
	}
	
  private:
	vector<Tag*>   tags;			///< Tags opened in the html output
	vector<NegTag> pending_tags;	///< Tags opened in the tagged string, but not (yet) in the output
	
	void add(String& ret, const NegTag& tag) {
		// Cancel out with pending tag?
		for (size_t i = pending_tags.size() - 1 ; i >= 0 ; --i) {
			if (pending_tags[i].tag == tag.tag) {
				if (pending_tags[i].neg != tag.neg) {
					pending_tags.erase(pending_tags.begin() + i);
					return;
				} else {
					break; // look no further
				}
			}
		}
		// Cancel out with existing tag?
		if (tag.neg) {
			for (size_t i = tags.size() - 1 ; i >= 0 ; --i) {
				if (tags[i] == tag.tag) {
					// cancel out with existing tag i, e.g. <b>:
					// situation was         <a><b><c>text
					// situation will become <a><b><c>text</c></b><c>
					vector<NegTag> reopen;
					for (size_t j = tags.size() - 1 ; j > i ; --j) {
						pending_tags.push_back(NegTag(tags[j], true));  // close tag, top down
						tags.pop_back();
					}
					pending_tags.push_back(tag);						// now close tag i
					for (size_t j = i + 1 ; j < tags.size() ; ++j) {
						pending_tags.push_back(NegTag(tags[j], false)); // reopen later, bottom up
						tags.pop_back();
					}
					tags.resize(i);
					return;
				}
			}
		}
		// Just insert normally
		pending_tags.push_back(tag);
	}
};

String symbols_to_html(const String& str, const SymbolFontP& symbol_font) {
	return str; // TODO
}

String to_html(const String& str_in, const SymbolFontP& symbol_font) {
	String str = remove_tag_contents(str,_("<sep-soft"));
	String ret;
	Tag bold  (_("<b>"), _("</b>")),
        italic(_("<i>"), _("</i>")),
        symbol(_("<span class=\"symbol\">"), _("</span>"));
	TagStack tags;
	String symbols;
	for (size_t i = 0 ; i < str.size() ; ) {
		Char c = str.GetChar(i);
		if (c == _('<')) {
			++i;
			if        (is_substr(str, i, _("b"))) {
				tags.open (ret, bold);
			} else if (is_substr(str, i, _("/b"))) {
				tags.close(ret, bold);
			} else if (is_substr(str, i, _("i"))) {
				tags.open (ret, italic);
			} else if (is_substr(str, i, _("/i"))) {
				tags.close(ret, italic);
			} else if (is_substr(str, i, _("sym"))) {
				tags.close(ret, symbol);
			} else if (is_substr(str, i, _("/sym"))) {
				if (!symbols.empty()) {
					// write symbols in a special way
					tags.write_pending_tags(ret);
					ret += symbols_to_html(symbols, symbol_font);
					symbols.clear();
				}
				tags.close(ret, symbol);
			}
			i = skip_tag(str, i-1);
		} else {
			// normal character
			tags.write_pending_tags(ret);
			++i;
			if (symbol.opened > 0 && symbol_font) {
				symbols += c; // write as symbols instead
			} else {
				if (c == _('\1')) { // escape <
					ret += _("&lt;");
				} else if (c == _('&')) {  // escape &
					ret += _("&amp;");
				} else if (c >= 0x80) {    // escape non ascii
					ret += String(_("&#")) << (int)c << _(';');
				} else {
					ret += c;
				}
			}
		}
	}
	// end of input
	if (!symbols.empty()) {
		tags.write_pending_tags(ret);
		ret += symbols_to_html(symbols, symbol_font);
		symbols.clear();
	}
	tags.close_all(ret);
	return ret;
}

// convert a tagged string to html
SCRIPT_FUNCTION(to_html) {
	SCRIPT_PARAM(String, input);
	SymbolFontP symbol_font; // TODO
	SCRIPT_RETURN(to_html(input, symbol_font));
}

// ----------------------------------------------------------------------------- : Text

// convert a tagged string to plain text
SCRIPT_FUNCTION(to_text) {
	SCRIPT_PARAM(String, input);
	SCRIPT_RETURN(untag_hide_sep(input));
}

// ----------------------------------------------------------------------------- : Files

// copy from source package -> destination package, return new filename (relative)
SCRIPT_FUNCTION(copy_file) {
	throw InternalError(_("TODO: copy_file")); // TODO
}

// write a file to the destination package.
// if 'filename' is not set, writes to the 'main' output file.
SCRIPT_FUNCTION(write_file) {
	throw InternalError(_("TODO: write_file")); // TODO
}

// write an ImageValue to a new file, return the filename
// if the image was not written, return nil
// TODO: write a ScriptImage?
SCRIPT_FUNCTION(image_to_file) {
	throw InternalError(_("TODO: image_to_file")); // TODO
}

// render a card, and write the image to a file
SCRIPT_FUNCTION(render_to_file) {
	throw InternalError(_("TODO: render_to_file")); // TODO
}

// ----------------------------------------------------------------------------- : Init

void init_script_export_functions(Context& ctx) {
	ctx.setVariable(_("to html"),        script_to_html);
	ctx.setVariable(_("to text"),        script_to_text);
	ctx.setVariable(_("copy file"),      script_copy_file);
	ctx.setVariable(_("write file"),     script_write_file);
	ctx.setVariable(_("image to file"),  script_image_to_file);
	ctx.setVariable(_("write image"),    script_image_to_file);
	ctx.setVariable(_("render to file"), script_render_to_file);
}
