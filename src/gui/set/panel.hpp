//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_PANEL
#define HEADER_GUI_SET_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/set.hpp>

class wxFindReplaceData;

// ----------------------------------------------------------------------------- : SetWindowPanel

/// A panel that is one of the panels in the set window.
/** This class is a virtual base class for all actual panels used in the set window.
 */
class SetWindowPanel : public wxPanel, public SetView {
  public:
	SetWindowPanel(Window* parent, int id, bool autoTabbing = false);
  
	/// We will probably want to respond to set changes
	virtual void onSetChange() {}
	
//	// --------------------------------------------------- : Meta information
//	
//	virtual String helpFile()    { return _(""); } ///< help file to use when this panel is active
	
	// --------------------------------------------------- : UI
	
	/// Init extra toolbar items and menus needed for this panel.
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb) {}
	/// Destroy the extra items added by initUI.
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb) {}
	/// Update the UI by enabling/disabling items.
	/** Note: copy/paste and find/replace are not handled here.
	 */
	virtual void onUpdateUI(wxUpdateUIEvent&) {}
	/// Respond to one of those extra menu/tool items
	virtual void onCommand(int id) {}
	
	// --------------------------------------------------- : Actions/Events
	
	/// Should return true if this panel wants to get focus to show an action
	virtual bool wantsToHandle(const Action&, bool undone) const { return false; }
	/// Handle an action that changes the current set
	virtual void onAction(const Action&, bool undone) {}
	
	// --------------------------------------------------- : Clipboard
	virtual bool canPaste() const { return false; }		///< Is pasting possible?
	virtual bool canCopy()  const { return false; }		///< Is copying possible?
	virtual bool canCut()   const { return canCopy(); }	///< Is cutting possible?
	virtual void doPaste()  {}							///< Paste the contents of the clipboard
	virtual void doCopy()   {}							///< Copy the selection to the clipboard
	virtual void doCut()    {}							///< Cut the selection to the clipboard
	
	// --------------------------------------------------- : Searching (find/replace)
	virtual bool canFind()    const { return false; }				///< Is finding possible?
	virtual bool canReplace() const { return false; }				///< Is replacing possible?
	virtual bool doFind      (wxFindReplaceData&) { return false; }	///< Find the next math
	virtual bool doReplace   (wxFindReplaceData&) { return false; }	///< Replace the next match
	virtual bool doReplaceAll(wxFindReplaceData&) { return false; }	///< Replace all matches
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() const { return CardP(); }	///< Return the currently selected card, or CardP()
	virtual void  selectCard(const CardP& card) {}			///< Switch the view to another card
	virtual void  selectFirstCard() {}						///< Switch the view to the first card
};

// ----------------------------------------------------------------------------- : EOF
#endif
