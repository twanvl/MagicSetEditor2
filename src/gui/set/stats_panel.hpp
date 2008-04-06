//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_STATS_PANEL
#define HEADER_GUI_SET_STATS_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class StatCategoryList;
class StatDimensionList;
class GraphControl;
class FilteredCardList;

// ----------------------------------------------------------------------------- : StatsPanel

/// A panel for showing statistics on cards
class StatsPanel : public SetWindowPanel {
  public:
	StatsPanel(Window* parent, int id);
	
	// --------------------------------------------------- : UI
	
	virtual void onChangeSet();
	virtual void onAction(const Action&, bool undone);
	
	virtual void initUI   (wxToolBar*, wxMenuBar*);
	virtual void destroyUI(wxToolBar*, wxMenuBar*);
	virtual void onCommand(int id);
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() const;
	virtual void selectCard(const CardP& card);
	
	// --------------------------------------------------- : Data
  private:
	DECLARE_EVENT_TABLE();
	
	StatCategoryList* categories;
	StatDimensionList* dimensions[3];
	GraphControl*     graph;
	FilteredCardList* card_list;
	
	bool up_to_date; ///< Are the graph and card list up to date?
	bool active;     ///< Is this panel selected?
	
	void onChange();
	void onGraphSelect(wxCommandEvent&);
	void showCategory();
	void filterCards();
};

// ----------------------------------------------------------------------------- : EOF
#endif
