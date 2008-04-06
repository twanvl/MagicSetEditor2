//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FORMAT_CLIPBOARD
#define HEADER_DATA_FORMAT_CLIPBOARD

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/dataobj.h>

DECLARE_POINTER_TYPE(Set);
DECLARE_POINTER_TYPE(Card);
DECLARE_POINTER_TYPE(Keyword);

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

// ----------------------------------------------------------------------------- : KeywordDataObject

/// The data format for keywords on the clipboard
class KeywordDataObject : public wxTextDataObject {
  public:
	/// Name of the format of MSE keywords
	static wxDataFormat format;
	
	KeywordDataObject();
	/// Store a keyword
	KeywordDataObject(const SetP& set, const KeywordP& card);
	
	/// Retrieve a keyword, only if it is made with the same game as set
	KeywordP getKeyword(const SetP& set);
};

// ----------------------------------------------------------------------------- : Card on clipboard

/// A DataObject for putting a card on the clipboard, in multiple formats
class CardOnClipboard : public wxDataObjectComposite {
  public:
	CardOnClipboard(const SetP& set, const CardP& card);
};

// ----------------------------------------------------------------------------- : EOF
#endif
