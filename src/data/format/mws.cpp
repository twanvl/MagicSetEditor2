//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/card.hpp>
#include <data/field/text.hpp>
#include <data/field/choice.hpp>
#include <util/string.hpp>
#include <util/tagged_string.hpp>
#include <wx/wfstream.h>

DECLARE_TYPEOF_COLLECTION(CardP);

// ----------------------------------------------------------------------------- : Utilities

/// Convert a tagged string to MWS format: \t\t before each line beyond the first
String untag_mws(const String& str) {
	// TODO : em dashes?
	return replace_all(untag(curly_quotes(str,false)), _("\n"), _("\n\t\t") );
}
//String untag_mws(const Defaultable<String>& str) {
//	str.
//}

/// Code for card color in MWS format
String card_color_mws(const String& col) {
	if (col == _("white"))     return _("W");
	if (col == _("blue"))      return _("U");
	if (col == _("black"))     return _("B");
	if (col == _("red"))       return _("R");
	if (col == _("green"))     return _("G");
	if (col == _("artifact"))  return _("Art");
	if (col == _("colorless")) return _("Art");
	if (col.find(_("land")) != String::npos) {
		return _("Lnd"); // land
	} else {
		return _("Gld"); // multicolor
	}
}

/// Code for card rarity, used for MWS and Apprentice
String card_rarity_code(const String& rarity) {
	if (rarity == _("rare"))     return _("R");
	if (rarity == _("uncommon")) return _("U");
	else                         return _("C");
}

// ----------------------------------------------------------------------------- : export_mws

void export_mws(Window* parent, const SetP& set) {
	if (!set->game->isMagic()) {
		throw Error(_("Can only export Magic sets to Magic Workstation"));
	}
	
	// Select filename
	String name = wxFileSelector(_("Export to file"),_(""),_(""),_(""),
		                         _("Text files (*.txt)|*.txt|All Files|*"),
		                         wxSAVE | wxOVERWRITE_PROMPT, parent);
	if (name.empty()) return;
	wxBusyCursor busy;
	// Open file
	wxFileOutputStream f(name);
	wxTextOutputStream file(f, wxEOL_DOS);
	
	// Write header
	file.WriteString(set->value<TextValue>(_("title")).value + _(" Spoiler List\n"));
	file.WriteString(_("Set exported using Magic Set Editor 2, version ") + app_version.toString() + _("\n\n"));
	wxDateTime now = wxDateTime::Now();
	file.WriteString(_("Spoiler List created on ") + now.FormatISODate() + _(" ") + now.FormatISOTime());
	file.WriteString(_("\n\n"));
	
	// Write cards
	FOR_EACH(card, set->cards) {
		file.WriteString(_("Card Name:\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("name")).value));
		file.WriteString(_("\nCard Color:\t"));
		file.WriteString(card_color_mws(card->value<ChoiceValue>(_("card color")).value));
		file.WriteString(_("\nMana Cost:\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("casting cost")).value));
		file.WriteString(_("\nType & Class:\t"));
		String sup_type = untag_mws(card->value<TextValue>(_("super type")).value);
		String sub_type = untag_mws(card->value<TextValue>(_("sub type")).value);
		if (sub_type.empty()) {
			file.WriteString(sup_type);
		} else {
			file.WriteString(sup_type + _(" - ") + sub_type);
		}
		file.WriteString(_("\nPow/Tou:\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("pt")).value));
		file.WriteString(_("\nCard Text:\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("rule text")).value));
		file.WriteString(_("\nFlavor Text:\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("flavor text")).value));
		file.WriteString(_("\nArtist:\t\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("illustrator")).value));
		file.WriteString(_("\nRarity:\t\t"));
		file.WriteString(card_rarity_code(card->value<ChoiceValue>(_("rarity")).value));
		file.WriteString(_("\nCard #:\t\t"));
		file.WriteString(untag_mws(card->value<TextValue>(_("card number")).value));
		file.WriteString(_("\n\n"));
	}
}
