//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_RANDOM_PACK_PANEL
#define HEADER_GUI_SET_RANDOM_PACK_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>
#include <wx/spinctrl.h>

class CardViewer;
class RandomCardList;
DECLARE_POINTER_TYPE(PackType);

// ----------------------------------------------------------------------------- : RandomPackPanel

/// A SetWindowPanel for creating random booster packs
class RandomPackPanel : public SetWindowPanel {
  public:
	RandomPackPanel(Window* parent, int id);
	~RandomPackPanel();
	
	// --------------------------------------------------- : UI
	
	virtual void onChangeSet();
	
	virtual void initUI   (wxToolBar* tb, wxMenuBar* mb);
	virtual void destroyUI(wxToolBar* tb, wxMenuBar* mb);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	
	// --------------------------------------------------- : Selection
	virtual void selectCard(const CardP& card);
	
	// --------------------------------------------------- : Clipboard
	
	virtual bool canCopy()  const;
	virtual void doCopy();
	
  private:
	CardViewer*       preview;		///< Card preview
	RandomCardList*   card_list;	///< The list of cards
	wxTextCtrl*       seed;			///< Seed value
	wxFlexGridSizer*  packsSizer;
	wxFlexGridSizer*  totalsSizer;
	wxButton*         generate_button;
	wxRadioButton*    seed_random, *seed_fixed;
	
	struct PackItem {
		PackTypeP     pack;
		wxStaticText* label;
		wxSpinCtrl*   value;
	};
	vector<PackItem> packs;
	
	struct TotalItem {
	};
	vector<TotalItem> totals;
	int total_packs;
	
	/// Update the total count of each card type
	void updateTotals();
	/// Get a seed value
	int getSeed();
	void setSeed(int seed);
	/// Generate the cards
	void generate();
	/// Store the settings
	void storeSettings();
  public:
	typedef PackItem PackItem_for_typeof;
};

// ----------------------------------------------------------------------------- : EOF
#endif
