//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	
	// --------------------------------------------------- : Meta information
	
	virtual String shortName()   { return _("<undefined>"); } ///< for tab bar
	virtual String longName()    { return shortName(); }      ///< for menu
	virtual String description() { return _("<undefined>"); } ///< for status bar
	virtual String helpFile()    { return _(""); }            ///< help file to use when this panel is active
	
	// --------------------------------------------------- : UI
	
	/// Init extra toolbar items and menus needed for this panel.
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb) {}
	/// Destroy the extra items added by initUI.
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb) {}
	/// Update the UI by enabling/disabling items.
	/** Note: copy/paste and find/replace are not handled here.
	 */
	virtual void onUpdateUI(wxUpdateUIEvent& e) {}
	/// Respond to one of those extra menu/tool items
	virtual void onCommand(int id) {}
	
	// --------------------------------------------------- : Actions/Events
	
	/// Should return true if this panel wants to get focus to show an action
	virtual bool wantsToHandle(const Action&) { return false; }
	/// Handle an action that changes the current set
	virtual void onAction(const Action&) {}
	/// The settings for rendering cards have changed, refresh card viewers/editors
	virtual void onRenderSettingsChange() {}
	
	// --------------------------------------------------- : Clipboard
	virtual bool canPaste() { return false; }			///< Is pasting possible?
	virtual bool canCopy()  { return false; }			///< Is copying possible?
	virtual bool canCut()   { return canCopy(); }		///< Is cutting possible?
	virtual void doPaste()  {}							///< Paste the contents of the clipboard
	virtual void doCopy()   {}							///< Copy the selection to the clipboard
	virtual void doCut()    {}							///< Cut the selection to the clipboard
	
	// --------------------------------------------------- : Searching (find/replace)
	virtual bool canFind()    { return false; }			///< Is finding possible?
	virtual bool canReplace() { return false; }			///< Is replacing possible?
	virtual bool doFind(wxFindReplaceData&)    { return false; }	///< Find the next math
	virtual bool doReplace(wxFindReplaceData&) { return false; }	///< Replace the next match
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() { return CardP(); }	///< Return the currently selected card, or CardP()
	virtual void  selectCard(CardP card) {}				///< Switch the view to another card
	
  protected:
	// --------------------------------------------------- : Helper functions for UI
	/// Enable/disable a tool or menu item
//	void enable(wxToolBar* tb, wxMenuBar* mb, int id, bool enable);
//		mb->Enable(id, enable)
//		tb->EnableTool(id, enable)
		
	/// Id of the control that has the focus, or -1 if no control has the focus
	int focusedControl();
//		Window* focusedWindow = findFocus()
//		// is this window actually inside this panel?
//		if focusedWindow && findWindowById(focusedWindow->id, &this) == focusedWindow
//			return focusedWindow->id
//		else
//			return -1 // no window has the focus, or it has a different parent/ancestor
};

// ----------------------------------------------------------------------------- : EOF
#endif
