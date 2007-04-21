//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_CARDS_PANEL
#define HEADER_GUI_SET_CARDS_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class wxSplitterWindow;
class ImageCardList;
class DataEditor;
class TextCtrl;
class IconMenu;
class HoverButton;

// ----------------------------------------------------------------------------- : CardsPanel

/// A card list and card editor panel
class CardsPanel : public SetWindowPanel {
  public:
	CardsPanel(Window* parent, int id);
	~CardsPanel();
	
	virtual void onChangeSet();
	
	// --------------------------------------------------- : UI
	
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	
	// --------------------------------------------------- : Actions
	
	virtual bool wantsToHandle(const Action&, bool undone) const;
  public:
  
	// --------------------------------------------------- : Clipboard
	virtual bool canCut() const;
	virtual bool canCopy() const;
	virtual bool canPaste() const;
	virtual void doCut();
	virtual void doCopy();
	virtual void doPaste();
	
	// --------------------------------------------------- : Searching (find/replace)
#if 0
	virtual bool canFind() const;
	virtual bool canReplace() const;
	virtual bool doFind(wxFindReplaceData& what);
	virtual bool doReplace(wxFindReplaceData& what);
  private:
	// Functions that handle finding
	typedef void (CardsPanel::*FindHandler)(const CardP&, const TextValueP&, const size_t, const size_t, wxFindReplaceData&);
		
	/// Execute a find (or replace), and start with the currently selected card and value
	/** if findSame==true then find will also find the currently highlighted word
	 *  Returns true if found
	 */
	bool find(FindReplaceData& what, const FindHandler& handler, bool findSame = false);
	
	/// find handler : select found value
	void handleFind(const CardP& card, const TextValueP& value, size_t start, size_t end, FindReplaceData& what);
	
	/// replace handler : replace found value, move selection to end
	void handleReplace(const CardP& card, const TextValueP& value, size_t start, size_t end, FindReplaceData& what);
	
	/// Find in all cards
	/** NOTE: this function is essentially the same as findInCard */
	bool findInCards(const CardP& firstCard, const ValueP& firstValue, int firstChar, FindReplaceData& what, const FindHandler& handler);
	
	/// Find in a card, if firstValue is specified start searching there
	/** NOTE: this function is essentially the same as findInCards */
	bool findInCard(const CardP& card, const ValueP& firstValue, int firstChar, FindReplaceData& what, const FindHandler& handler);
	
	/// Find the current search string in the specified value
	/** if searchDir = up searches from the end and only before firstChar, unless firstChar == -1 */
	bool findInValue(const CardP& crd_, virtual const ValueP& value, int firstChar, FindReplaceData& what, const FindHandler& handler);
#endif
  public:
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() const;
	virtual void selectCard(const CardP& card);
	
  private:
	// --------------------------------------------------- : Controls
	wxSplitterWindow* splitter;
	DataEditor*       editor;
	ImageCardList*    card_list;
	TextCtrl*         notes;
	HoverButton*      collapse_notes;
	
	// --------------------------------------------------- : Menus & tools
	IconMenu* menuCard, *menuFormat;
	wxMenuItem* insertSymbolMenu; // owned by menuFormat, but submenu owned by SymbolFont
};

// ----------------------------------------------------------------------------- : EOF
#endif
