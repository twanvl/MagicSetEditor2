//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_STATS_PANEL
#define HEADER_GUI_SET_STATS_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>
#include <data/graph_type.hpp>

class StatCategoryList;
class StatDimensionList;
class GraphControl;
class FilteredCardList;
class IconMenu;

// Pick the style here:
#define USE_DIMENSION_LISTS 1
#define USE_SEPARATE_DIMENSION_LISTS 0

// ----------------------------------------------------------------------------- : StatsPanel

/// A panel for showing statistics on cards
class StatsPanel : public SetWindowPanel {
  public:
	StatsPanel(Window* parent, int id);
	~StatsPanel();
	
	// --------------------------------------------------- : UI
	
	virtual void onChangeSet();
	virtual void onAction(const Action&, bool undone);
	
	virtual void initUI   (wxToolBar*, wxMenuBar*);
	virtual void destroyUI(wxToolBar*, wxMenuBar*);
	virtual void onUpdateUI(wxUpdateUIEvent&);
	virtual void onCommand(int id);
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() const;
	virtual void selectCard(const CardP& card);
	
	// --------------------------------------------------- : Data
  private:
	DECLARE_EVENT_TABLE();
	
	#if USE_SEPARATE_DIMENSION_LISTS
		StatDimensionList* dimensions[3];
	#elif USE_DIMENSION_LISTS
		StatDimensionList* dimensions;
	#else
		StatCategoryList* categories;
	#endif
	GraphControl*     graph;
	FilteredCardList* card_list;
	IconMenu*         menuGraph;
	
	CardP card;      ///< Selected card
	bool up_to_date; ///< Are the graph and card list up to date?
	bool active;     ///< Is this panel selected?
	
	void initControls();
	
	void onChange();
	void onGraphSelect(wxCommandEvent&);
	void showCategory(const GraphType* prefer_layout = nullptr);
	void showLayout(GraphType);
	void filterCards();
};

// ----------------------------------------------------------------------------- : EOF
#endif
