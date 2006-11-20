//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FORMAT_CLIPBOARD
#define HEADER_DATA_FORMAT_CLIPBOARD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/dataobj.h>

DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Card);

// ----------------------------------------------------------------------------- : CardDataObject

/// The data format for cards on the clipboard
class CardDataObject : public wxTextDataObject {
  public:
	/// Name of the format of MSE cards
	static wxDataFormat format;
	
	CardDataObject();
	/// Store a card
	CardDataObject(const SetP& set, const CardP& card);
	
	/// Retrieve a card, only if it is made with the same game as set
	CardP getCard(const SetP& set);
};

// ----------------------------------------------------------------------------- : Card on clipboard

/// A DataObject for putting a card on the clipboard, in multiple formats
class CardOnClipboard : public wxDataObjectComposite {
  public:
	CardOnClipboard(const SetP& set, const CardP& card);
};

// ----------------------------------------------------------------------------- : EOF
#endif
