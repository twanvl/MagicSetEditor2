//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <script/functions/functions.hpp>
#include <script/functions/util.hpp>
#include <script/image.hpp>
#include <data/symbol_font.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/export_template.hpp>
#include <data/format/formats.hpp>
#include <util/tagged_string.hpp>
#include <gfx/generated_image.hpp>
#include <util/error.hpp>
#include <wx/wfstream.h>
#include <wx/filename.h>

// ----------------------------------------------------------------------------- : Utility

// Make sure we can export files to a data directory
void guard_export_info(const String& fun, bool need_template = false) {
  if (!export_info() && (!need_template || export_info()->export_template)) {
    throw ScriptError(_("Can only use ") + fun + _(" from export templates"));
  } else if (export_info()->directory_relative.empty()) {
    throw ScriptError(_("Can only use ") + fun + _(" when 'create directory' is set to true"));
  }
}

/// Find an absolute filename for a relative filename from an export template,
/// Returns the absolute filename, and may modify the relative name.
String get_export_full_path(String& rel_name) {
  ExportInfo& ei = *export_info();
  // the absolute path
  wxFileName fn(rel_name);
  fn.Normalize(wxPATH_NORM_ALL, ei.directory_absolute);
  if (!ei.allow_writes_outside) {
    // check if path is okay
    wxFileName fn2(_("x"));
    fn2.Normalize(wxPATH_NORM_ALL, ei.directory_absolute);
    String p1 = fn.GetFullPath();
    String p2 = fn2.GetFullPath();
    p2.resize(p2.size() - 1); // drop the x
    if (p2.empty() || p1.size() < p2.size() || p1.substr(0,p2.size()-1) != p2.substr(0,p2.size()-1)) {
      throw ScriptError(_("Not a relative filename: ") + rel_name);
    }
  }
  rel_name = fn.GetFullName(); // TODO: does this work correctly with subdirectories in target dir?
  return fn.GetFullPath();
}

void ensure_dir_valid(String& path) {
  wxFileName filename = path;
  if (!filename.DirExists())
    filename.Mkdir();
}

// ----------------------------------------------------------------------------- : HTML

// An HTML tag
struct Tag {
  Tag(const Char* open_tag, const Char* close_tag)
    : open_tag(open_tag), close_tag(close_tag), opened(0)
  {}
  const Char* open_tag;  ///< The tags to insert in HTML "<tag>"
  const Char* close_tag;  ///< The tags to insert in HTML "</tag>"
  int opened;        ///< How often is the tag opened in the input?
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
    // cancel out tags with pending tags
    write_pending_tags(ret);
    // close all open tags
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
  vector<Tag*>   tags;      ///< Tags opened in the html output
  vector<NegTag> pending_tags;  ///< Tags opened in the tagged string, but not (yet) in the output
  
  void add(String& ret, const NegTag& tag) {
    // Cancel out with pending tag?
    for (int i = (int)pending_tags.size() - 1 ; i >= 0 ; --i) {
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
      for (int i = (int)tags.size() - 1 ; i >= 0 ; --i) {
        if (tags[i] == tag.tag) {
          // cancel out with existing tag i, e.g. <b>:
          // situation was         <a><b><c>text
          // situation will become <a><b><c>text</c></b><c>
          vector<NegTag> reopen;
          for (int j = (int)tags.size() - 1 ; j > i ; --j) {
            pending_tags.push_back(NegTag(tags[j], true));  // close tag, top down
            tags.pop_back();
          }
          pending_tags.push_back(tag);            // now close tag i
          for (int j = i + 1 ; j < (int)tags.size() ; ++j) {
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

// html-escape a string
String html_escape(const String& str) {
  String ret;
  FOR_EACH_CONST(c, str) {
    if (c == ESCAPED_LANGLE || c == _('<')) { // escape <
      ret += _("&lt;");
    } else if (c == _('>')) {  // escape >
      ret += _("&gt;");
    } else if (c == _('&')) {  // escape &
      ret += _("&amp;");
    } else if (c == _('\'')) {  // escape '
      ret += _("&#39;");
    } else if (c == _('\"')) {  // escape "
      ret += _("&quot;");
    } else if (c >= 0x80) {    // escape non ascii
      ret += String(_("&#")) << (int)c << _(';');
    } else {
      ret += c;
    }
  }
  return ret;
}

// write symbols to html
String symbols_to_html(const String& str, SymbolFont& symbol_font, double size) {
  guard_export_info(_("symbols_to_html"));
  ExportInfo& ei = *export_info();
  vector<SymbolFont::DrawableSymbol> symbols;
  symbol_font.split(str, symbols);
  String html;
  FOR_EACH(sym, symbols) {
    String filename = symbol_font.name() + _("-") + clean_filename(sym.text) + _(".png");
    map<String,wxSize>::iterator it = ei.exported_images.find(filename);
    if (it == ei.exported_images.end()) {
      // save symbol image
      Image img = symbol_font.getImage(size, sym);
      wxFileName fn;
      fn.SetPath(ei.directory_absolute);
      fn.SetFullName(filename);
      img.SaveFile(fn.GetFullPath());
      it = ei.exported_images.insert(make_pair(filename, wxSize(img.GetWidth(), img.GetHeight()))).first;
    }
    html += _("<img src='") + filename + _("' alt='") + html_escape(sym.text)
         +  _("' width='")  + (String() << it->second.x)
         +  _("' height='") + (String() << it->second.y) + _("'>");
  }
  return html;
}

String to_html(const String& str_in, const SymbolFontP& symbol_font, double symbol_size) {
  String str = remove_tag_contents(str_in,_("<sep-soft"));
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
        tags.open (ret, symbol);
      } else if (is_substr(str, i, _("/sym"))) {
        if (!symbols.empty()) {
          // write symbols in a special way
          tags.write_pending_tags(ret);
          ret += symbols_to_html(symbols, *symbol_font, symbol_size);
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
        c = untag_char(c);
        if (c == _('<')) { // escape <
          ret += _("&lt;");
        } else if (c == _('&')) {  // escape &
          ret += _("&amp;");
        } else if (c >= 0x80) {    // escape non ascii
          ret += String(_("&#")) << (int)c << _(';');
        } else if (c == _('\n')) {
          ret += _("<br>\n");
        } else {
          ret += c;
        }
      }
    }
  }
  // end of input
  if (!symbols.empty()) {
    tags.write_pending_tags(ret);
    ret += symbols_to_html(symbols, *symbol_font, symbol_size);
    symbols.clear();
  }
  tags.close_all(ret);
  return ret;
}

// convert a tagged string to html
SCRIPT_FUNCTION(to_html) {
  SCRIPT_PARAM_C(String, input);
  // symbol font?
  SymbolFontP symbol_font;
  SCRIPT_OPTIONAL_PARAM_N(String, _("symbol_font"), font_name) {
    symbol_font = SymbolFont::byName(font_name);
    symbol_font->update(ctx);
  }
  SCRIPT_OPTIONAL_PARAM_(double, symbol_font_size);
  if (symbol_font_size <= 0) symbol_font_size = 12; // a default
  SCRIPT_RETURN(to_html(input, symbol_font, symbol_font_size));
}

// convert a symbol string to html
SCRIPT_FUNCTION(symbols_to_html) {
  SCRIPT_PARAM_C(String, input);
  SCRIPT_PARAM_N(String, _("symbol_font"), font_name);
  SCRIPT_OPTIONAL_PARAM_(double, symbol_font_size);
  SymbolFontP symbol_font = SymbolFont::byName(font_name);
  symbol_font->update(ctx);
  if (symbol_font_size <= 0) symbol_font_size = 12; // a default
  SCRIPT_RETURN(symbols_to_html(input, *symbol_font, symbol_font_size));
}

// ----------------------------------------------------------------------------- : BB Code

String to_bbcode(const String& str_in) {
  String str = remove_tag_contents(str_in,_("<sep-soft"));
  String ret;
  Tag bold  (_("[b]"), _("[/b]")),
        italic(_("[i]"), _("[/i]"));
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
      } /*else if (is_substr(str, i, _("sym"))) {
        tags.open (ret, symbol);
      } else if (is_substr(str, i, _("/sym"))) {
        if (!symbols.empty()) {
          // write symbols in a special way
          tags.write_pending_tags(ret);
          ret += symbols_to_html(symbols, symbol_font);
          symbols.clear();
        }
        tags.close(ret, symbol);
      }*/
      i = skip_tag(str, i-1);
    } else {
      // normal character
      tags.write_pending_tags(ret);
      ++i;
//      if (symbol.opened > 0 && symbol_font) {
//        symbols += c; // write as symbols instead
//      } else {
        ret += untag_char(c);
//      }
    }
  }
  // end of input
/*  if (!symbols.empty()) {
    tags.write_pending_tags(ret);
    ret += symbols_to_html(symbols, symbol_font);
    symbols.clear();
  }*/
  tags.close_all(ret);
  return ret;
}

// convert a tagged string to BBCode
SCRIPT_FUNCTION(to_bbcode) {
  SCRIPT_PARAM_C(String, input);
  throw InternalError(_("TODO: to_bbcode"));
//  SCRIPT_RETURN(to_bbcode(input, symbol_font));
}

// ----------------------------------------------------------------------------- : Text

// convert a tagged string to plain text
SCRIPT_FUNCTION(to_text) {
  SCRIPT_PARAM_C(String, input);
  SCRIPT_RETURN(untag_hide_sep(input));
}

// ----------------------------------------------------------------------------- : Files

// copy from source package -> destination directory, return new filename (relative)
SCRIPT_FUNCTION(copy_file) {
  guard_export_info(_("copy_file"));
  SCRIPT_PARAM_C(String, input); // file to copy
  // output path
  String out_name = input;
  String out_path = get_export_full_path(out_name);
  // copy
  ExportInfo& ei = *export_info();
  auto in = ei.export_template->openIn(input);
  ensure_dir_valid(out_path);
  wxFileOutputStream out(out_path);
  if (!out.Ok()) throw Error(_("Unable to open file '") + out_path + _("' for output"));
  out.Write(*in);
  SCRIPT_RETURN(out_name);
}

// write a file to the destination directory
SCRIPT_FUNCTION(write_text_file) {
  guard_export_info(_("write_text_file"));
  SCRIPT_PARAM_C(String, input); // text to write
  SCRIPT_PARAM(String, file); // file to write to
  // output path
  String out_path = get_export_full_path(file);
  // write
  ensure_dir_valid(out_path);
  wxFileOutputStream out(out_path);
  if (!out.Ok()) throw Error(_("Unable to open file '") + out_path + _("' for output"));
  wxTextOutputStream tout(out);
  tout.WriteString(BYTE_ORDER_MARK);
  tout.WriteString(input);
  SCRIPT_RETURN(file);
}

SCRIPT_FUNCTION(write_image_file) {
  guard_export_info(_("write_image_file"));
  // output path
  SCRIPT_PARAM(String, file); // file to write to
  String out_path = get_export_full_path(file);
  // duplicates?
  ExportInfo& ei = *export_info();
  if (ei.exported_images.find(file) != ei.exported_images.end()) {
    SCRIPT_RETURN(file); // already written an image with this name
  }
  // get image
  SCRIPT_PARAM_C(ScriptValueP, input);
  SCRIPT_OPTIONAL_PARAM_(int, width);
  SCRIPT_OPTIONAL_PARAM_(int, height);
  ScriptObject<CardP>* card = dynamic_cast<ScriptObject<CardP>*>(input.get()); // is it a card?
  Image image;
  GeneratedImage::Options options(width, height, ei.export_template.get(), ei.set.get());
  if (card) {
    image = conform_image(export_bitmap(ei.set, card->getValue()).ConvertToImage(), options);
  } else {
    image = input->toImage()->generateConform(options);
  }
  if (!image.Ok()) throw Error(_("Unable to generate image for file ") + file);
  // write
  ensure_dir_valid(out_path);
  image.SaveFile(out_path);
  ei.exported_images.insert(make_pair(file, wxSize(image.GetWidth(), image.GetHeight())));
  SCRIPT_RETURN(file);
}

SCRIPT_FUNCTION(write_set_file) {
  guard_export_info(_("write_set_file"));
  // output path
  SCRIPT_PARAM(String, file); // file to write to
  String out_path = get_export_full_path(file);
  // export
  SCRIPT_PARAM_C(Set*, set);
  ensure_dir_valid(out_path);
  set->saveCopy(out_path); // TODO: use export_set instead?
  SCRIPT_RETURN(file);
  
}

SCRIPT_FUNCTION(sanitize) {
  SCRIPT_PARAM_C(String, input);
  //TODO
  SCRIPT_RETURN(input);
}

// ----------------------------------------------------------------------------- : Init

void init_script_export_functions(Context& ctx) {
  ctx.setVariable(_("to_html"),          script_to_html);
  ctx.setVariable(_("symbols_to_html"),  script_symbols_to_html);
  ctx.setVariable(_("to_text"),          script_to_text);
  ctx.setVariable(_("copy_file"),        script_copy_file);
  ctx.setVariable(_("write_text_file"),  script_write_text_file);
  ctx.setVariable(_("write_image_file"), script_write_image_file);
  ctx.setVariable(_("write_set_file"),   script_write_set_file);
  ctx.setVariable(_("sanitize"),         script_sanitize);
}
