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
class FindInfo;

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
  
	// --------------------------------------------------- : Clipboard
	virtual bool canCut() const;
	virtual bool canCopy() const;
	virtual bool canPaste() const;
	virtual void doCut();
	virtual void doCopy();
	virtual void doPaste();
	
	// --------------------------------------------------- : Searching (find/replace)

	virtual bool canFind()    const { return true; }
	virtual bool canReplace() const { return true; }
	virtual bool doFind      (wxFindReplaceData&);
	virtual bool doReplace   (wxFindReplaceData&);
	virtual bool doReplaceAll(wxFindReplaceData&);
  private:
	/// Do a search or replace action for the given FindInfo in all cards
	bool search(FindInfo& find, bool from_start);
	class SearchFindInfo;
	class ReplaceFindInfo;
	friend class CardsPanel::SearchFindInfo;
	friend class CardsPanel::ReplaceFindInfo;
  public:
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() const;
	virtual void selectCard(const CardP& card);
	virtual void selectFirstCard();
	
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
