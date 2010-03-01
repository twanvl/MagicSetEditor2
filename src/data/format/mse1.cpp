//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/card.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/image.hpp>
#include <wx/wfstream.h>

// ----------------------------------------------------------------------------- : MSE1FileFormat

/// The file format of MSE1 files
class MSE1FileFormat : public FileFormat {
  public:
	virtual String extension()          { return _("mse"); }
	virtual String name()               { return _("Magic Set Editor version 1 files (*.mse)"); }
	virtual bool canImport()            { return true; }
	virtual bool canExport(const Game&) { return false; }
	virtual SetP importSet(const String& filename);
};

FileFormatP mse1_file_format() {
	return new_intrusive<MSE1FileFormat>();
}

// ----------------------------------------------------------------------------- : Importing

// read a card from a mse1 file, add to the set when done
void read_mse1_card(Set& set, wxFileInputStream& f, wxTextInputStream& file);

SetP MSE1FileFormat::importSet(const String& filename) {
	wxFileInputStream f(filename);
	#ifdef UNICODE
		wxTextInputStream file(f, _('\n'), wxConvLibc);
	#else
		wxTextInputStream file(f);
	#endif
	// create set
	SetP set(new Set(Game::byName(_("magic"))));
	
	// file version check
	String format = file.ReadLine();
	if (format.substr(0,8) != _("MTG Set8")) {
		throw ParseError(_("Expected MSE format version 8\nTo convert files made with older versions of Magic Set Editor:\n  1. Download the latest version 1 from http:;//magicsetedtitor.sourceforge.net\n  2. Open the set, then save the set\n  3. Try to open them again in this program."));
	}
	// read general info
	set->value<TextValue>(_("title"))    .value = file.ReadLine();
	set->value<TextValue>(_("artist"))   .value = file.ReadLine();
	set->value<TextValue>(_("copyright")).value = file.ReadLine();
	file.ReadLine(); // border color, ignored
	String stylesheet = file.ReadLine();
	set->apprentice_code = file.ReadLine(); // apprentice prefix
	file.ReadLine(); // 'formatN'?, not even used by MSE1 :S, ignored
	file.ReadLine(); // 'formatS'?, same, ignored
	file.ReadLine(); // symbol filename, ignored
	file.ReadLine(); // use black symbol for all rarities, ignored
	String desc, line;
	while (!f.Eof()) {
		line = file.ReadLine();
		if (line == _("\xFF")) break;
		desc += line;
	}
	set->value<TextValue>(_("description")).value = desc;
	
	// load stylesheet
	if (stylesheet.substr(0,3) == _("old")) {
		try {
			set->stylesheet = StyleSheet::byGameAndName(*set->game, _("old"));
		} catch (const Error&) {
			// If old style doesn't work try the new one
			set->stylesheet = StyleSheet::byGameAndName(*set->game, _("new"));
		}
	} else {
		set->stylesheet = StyleSheet::byGameAndName(*set->game, _("new"));
	}
	
	// read cards
	CardP current_card;
	while (!f.Eof()) {
		read_mse1_card(*set, f, file);
	}
	
	// done
	set->validate();
	return set;
}

void read_mse1_card(Set& set, wxFileInputStream& f, wxTextInputStream& file) {
	CardP card(new Card(*set.game));
	while (!f.Eof()) {
		// read a line
		String line = file.ReadLine();
		if (line.empty()) continue;
		Char type = line.GetChar(0);
		line = line.substr(1);
		// interpret this line
		switch (type) {
			case 'A': {		// done
				set.cards.push_back(card);
				return;
			} case 'B': {	// name
				card->value<TextValue>(_("name"))        .value.assign(line);
				break;
			} case 'C': case 'D': { // image filename
				String image_file = set.newFileName(_("image"),_("")); // a new unique name in the package
				if (wxCopyFile(line, set.nameOut(image_file), true)) {
					card->value<ImageValue>(_("image"))  .filename = image_file;
				}
				break;
			} case 'E':	{	// super type
				card->value<TextValue>(_("super type"))  .value.assign(line);
				break;
			} case 'F': {	// sub type
				card->value<TextValue>(_("sub type"))    .value.assign(line);
				break;
			} case 'G': {	// casting cost
				card->value<TextValue>(_("casting cost")).value.assign(line);
				break;
			} case 'H': {	// rarity
				String rarity;
				if      (line == _("(U)")) rarity = _("uncommon");
				else if (line == _("(R)")) rarity = _("rare");
				else                       rarity = _("common");
				card->value<ChoiceValue>(_("rarity"))    .value.assign(rarity);
				break;
			} case 'I': {	// power/thoughness
				size_t pos = line.find_first_of(_('/'));
				if (pos != String::npos) {
					card->value<TextValue>(_("power"))       .value.assign(line.substr(0, pos));
					card->value<TextValue>(_("toughness"))   .value.assign(line.substr(pos+1));
				}
				break;
			} case 'J': {	// rule text or part of text
				Defaultable<String>& text = card->value<TextValue>(_("rule text")).value;
				if (!text().empty()) text.mutate() += _('\n');
				text.mutate() += line;
				break;
			} case 'K':	{	// flavor text or part of text
				Defaultable<String>& text = card->value<TextValue>(_("flavor text")).value;
				if (!text().empty()) text.mutate() += _('\n');
				text.mutate() += line;
				break;
			} case 'L': {	// card color (if not default)
				// decode color
				String color;
				if      (line == _("1")) color = _("white");
				else if (line == _("2")) color = _("blue");
				else if (line == _("3")) color = _("black");
				else if (line == _("4")) color = _("red");
				else if (line == _("5")) color = _("green");
				else if (line == _("6")) color = _("colorless");
				else if (line == _("7")) color = _("land");
				else if (line == _("9")) color = _("multicolor");
				else                     color = _("colorless");
				card->value<ChoiceValue>(_("card color")).value.assign(color);
				break;
			} default: {
				throw ParseError(_("Not a valid MSE1 file"));
			}
		}
	}
}
