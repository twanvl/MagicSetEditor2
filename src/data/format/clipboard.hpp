//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
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
class CardsDataObject : public wxTextDataObject {
  public:
	/// Name of the format of MSE cards
	static wxDataFormat format;
	
	CardsDataObject();
	/// Store a card
	CardsDataObject(const SetP& set, const vector<CardP>& cards);
	
	/// Retrieve the cards, only if it is made with the same game as set
	/** Return true if the cards are correctly retrieved, and there is at least one card */
	bool getCards(const SetP& set, vector<CardP>& out);
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

/// A DataObject for putting one or more cards on the clipboard, in multiple formats
class CardsOnClipboard : public wxDataObjectComposite {
  public:
	CardsOnClipboard(const SetP& set, const vector<CardP>& cards);
};

// ----------------------------------------------------------------------------- : EOF
#endif
