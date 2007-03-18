//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/card.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <data/field/image.hpp>
#include <util/tagged_string.hpp>
#include <wx/wfstream.h>

DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : MtgEditorFileFormat

/// The file format of Mtg Editor files
class MtgEditorFileFormat : public FileFormat {
  public:
	virtual String extension()          { return _("set"); }
	virtual String name()               { return _("Mtg Editor files (*.set)"); }
	virtual bool canImport()            { return true; }
	virtual bool canExport(const Game&) { return false; }
	virtual SetP importSet(const String& filename);
  private:
	// Filter: se filename -> image directory
	// based on MtgEditor's: CardSet.getImageFolder
	String filter1(const String& str);
	// Filter: card name -> image name
	// based on MtgEditor's: Tools::purgeSpecialChars
	String filter2(const String& str);
	// Remove mtg editor tags
	void untag(const CardP& card, const String& field);
	// Translate all tags, mana tags get converted to <sym>, other tags are removed
	void translateTags(String& value);
};

FileFormatP mtg_editor_file_format() {
	return new_shared<MtgEditorFileFormat>();
}

// ----------------------------------------------------------------------------- : Importing

SetP MtgEditorFileFormat::importSet(const String& filename) {
	wxFileInputStream f(filename);
	#ifdef UNICODE
		wxTextInputStream file(f, _('\n'), wxConvLibc);
	#else
		wxTextInputStream file(f);
	#endif
	// create set
	SetP set(new Set(Game::byName(_("magic"))));
	
	// parsing state
	CardP current_card;
	Defaultable<String>* target = nullptr; // value we are currently reading
	String layout = _("8e");
	String set_date, card_date;
	bool first = true;
	
	// read file
	while (!f.Eof()) {
		// read a line
		if (!current_card) current_card = new_shared1<Card>(*set->game);
		String line = file.ReadLine();
		if        (line == _("#SET###########")) {										// set.title
			target = &set->value<TextValue>(_("title")).value;
		} else if (line == _("#SETDATE#######")) {										// date
			// remember date for generation of illustration filename
			target = nullptr;
			set_date = file.ReadLine();
		} else if (line == _("#SHOWRARITY####") ||
		           line == _("#FONTS#########") || line == _("#COUNT#########")) {		// ignore
			target = nullptr;
			file.ReadLine();
		} else if (line == _("#LAYOUT########")) {										// layout type
			target = nullptr;
			layout = file.ReadLine();
		} else if (line == _("#CARD##########") || line == _("#EOF###########")) {	// begin/end of card
			if (!first) {
				// First [CARD##] indicates only the start of a card, subsequent ones also the end of the previous
				// card. We only care about the latter
				// remove all tags from all text values
				untag(current_card, _("name"));
				untag(current_card, _("super type"));
				untag(current_card, _("sub type"));
				untag(current_card, _("casting cost"));
				untag(current_card, _("flavor text"));
				untag(current_card, _("illustrator"));
				untag(current_card, _("copyright"));
				untag(current_card, _("power"));
				untag(current_card, _("toughness"));
				// translate mtg editor tags to mse2 tags
				translateTags(current_card->value<TextValue>(_("rule text")).value.mutate());
				// add the card to the set
				set->cards.push_back(current_card);
			}
			first = false;
			current_card = new_shared1<Card>(*set->game);
			target = &current_card->value<TextValue>(_("name")).value;
		} else if (line == _("#DATE##########")) {										// date
			// remember date for generation of illustration filename
			target = nullptr;
			card_date = file.ReadLine();
		} else if (line == _("#TYPE##########")) {										// super type
			target = &current_card->value<TextValue>(_("super type")).value;
		} else if (line == _("#SUBTYPE#######")) {										// sub type
			target = &current_card->value<TextValue>(_("sub type")).value;
		} else if (line == _("#COST##########")) {										// casting cost
			target = &current_card->value<TextValue>(_("casting cost")).value;
		} else if (line == _("#RARITY########") || line == _("#FREQUENCY#####")) {	// rarity
			target = 0;
			line = file.ReadLine();
			if      (line == _("0")) current_card->value<ChoiceValue>(_("rarity")).value.assign(_("common"));
			else if (line == _("1")) current_card->value<ChoiceValue>(_("rarity")).value.assign(_("uncommon"));
			else                     current_card->value<ChoiceValue>(_("rarity")).value.assign(_("rare"));
		} else if (line == _("#COLOR#########")) {										// card color
			target = 0;
			line = file.ReadLine();
			current_card->value<ChoiceValue>(_("card color")).value.assign(line);
		} else if (line == _("#AUTOBG########")) {										// card color.isDefault
			target = 0;
			line = file.ReadLine();
			if (line == _("TRUE")) {
				current_card->value<ChoiceValue>(_("card color")).value.makeDefault();
			}
		} else if (line == _("#RULETEXT######")) {										// rule text
			target = &current_card->value<TextValue>(_("rule text")).value;
		} else if (line == _("#FLAVORTEXT####")) {										// flavor text
			target = &current_card->value<TextValue>(_("flavor text")).value;
		} else if (line == _("#ARTIST########")) {										// illustrator
			target = &current_card->value<TextValue>(_("illustrator")).value;
		} else if (line == _("#COPYRIGHT#####")) {										// copyright
			target = &current_card->value<TextValue>(_("copyright")).value;
		} else if (line == _("#POWER#########")) {										// power
			target = &current_card->value<TextValue>(_("power")).value;
		} else if (line == _("#TOUGHNESS#####")) {										// toughness
			target = &current_card->value<TextValue>(_("toughness")).value;
		} else if (line == _("#ILLUSTRATION##") || line == _("#ILLUSTRATION8#")) {		// image
			target = 0;
			line = file.ReadLine();
			if (!wxFileExists(line)) {
				// based on card name and date
				line = filter1(filename) + set_date + _("/") +
				       filter2(current_card->value<TextValue>(_("name")).value) + card_date + _(".jpg");
			}
			// copy image into set
			if (wxFileExists(line)) {
				String image_file = set->newFileName(_("image"),_(""));
				if (wxCopyFile(line, set->nameOut(image_file), true)) {
					current_card->value<ImageValue>(_("image")).filename = image_file;
				}
			}
		} else if (line == _("#TOMBSTONE#####")) {										// tombstone
			target = 0;
			line = file.ReadLine();
			current_card->value<ChoiceValue>(_("card symbol")).value.assign(
				line==_("TRUE") ? _("tombstone") : _("none")
			);
		} else {
			// normal text
			if (target != 0) {															// value of a text field
				if (!target->isDefault()) target->mutate() += _("\n");
				target->mutate() += line;
			} else {
				throw ParseError(_("Error in Mtg Editor file, unexpected text:\n") + line);
			}
		}
	}
	
	// set defaults for artist and copyright to that of the first card
	if (!set->cards.empty()) {
		String artist    = set->cards[0]->value<TextValue>(_("illustrator")).value;
		String copyright = set->cards[0]->value<TextValue>(_("copyright"))  .value;
		set->value<TextValue>(_("artist"))   .value.assign(artist);
		set->value<TextValue>(_("copyright")).value.assign(copyright);
		// which cards have this value?
		FOR_EACH(card, set->cards) {
			Defaultable<String>& card_artist    = card->value<TextValue>(_("illustrator")).value;
			Defaultable<String>& card_copyright = card->value<TextValue>(_("copyright"))  .value;
			if (card_artist    == artist)    card_artist.makeDefault();
			if (card_copyright == copyright) card_copyright.makeDefault();
		}
	}
	
	// Load stylesheet
	if (layout != _("8e")) {
		try {
			set->stylesheet = StyleSheet::byGameAndName(*set->game, _("old"));
		} catch (const Error&) {
			// If old style doesn't work try the new one
			set->stylesheet = StyleSheet::byGameAndName(*set->game, _("new"));
		}
	} else {
		set->stylesheet = StyleSheet::byGameAndName(*set->game, _("new"));
	}
	
	set->validate();
	return set;
}

// ----------------------------------------------------------------------------- : Filtering

String MtgEditorFileFormat::filter1(const String& str) {
	String before, after, ret;
	// split path name
	size_t pos = str.find_last_of(_("/\\"));
	if (pos == String::npos) {
		before = _(""); after = str;
	} else {
		before = str.substr(0, pos + 1);
		after  = str.substr(pos + 1);
	}
	// filter
	FOR_EACH(c, after) {
		if (isAlnum(c)) ret += c;
		else            ret += _('_');
	}
	return before + ret;
}

String MtgEditorFileFormat::filter2(const String& str) {
	String ret;
	FOR_EACH_CONST(c, str) {
		if (isAlnum(c))                  ret += c;
		else if (c==_(' ') || c==_('-')) ret += _('_');
	}
	return ret;
}

void MtgEditorFileFormat::untag(const CardP& card, const String& field) {
	Defaultable<String>& value = card->value<TextValue>(field).value;
	value.assignDontChangeDefault(untag_no_escape(value));
}


void MtgEditorFileFormat::translateTags(String& value) {
	// Translate tags
	String ret;
	size_t pos = 0;
	while (pos < value.size()) {
		Char c = value.GetChar(pos);
		++pos;
		if (c == _('<')) {
			String tag;
			while (pos < value.size()) {
				c = value.GetChar(pos);
				++pos;
				if (c == _('>'))  break;
				else tag += c;
			}
			tag.MakeUpper();
			unsigned long number;
			if (tag==_("W") || tag==_("U") || tag==_("B") || tag==_("R") || tag==_("G") || tag==_("X") || tag==_("Y") || tag==_("Z")) {
				ret += _("<sym>") + tag + _("</sym>");
			} else if (tag.ToULong(&number)) {
				ret += _("<sym>") + tag + _("</sym>");
			} else if (tag==_("T") || tag==_("TAP")) {
				ret += _("<sym>T</sym>");
			} else if (tag==_("THIS")) {
				ret += _("~");
			}
		} else {
			ret += c;
		}
	}
	// Join adjecent symbol sections
	value = replace_all(ret, _("</sym><sym>"), _(""));
}
