//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SET_STATS_PANEL
#define HEADER_GUI_SET_STATS_PANEL

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/panel.hpp>

class StatCategoryList;
class GraphControl;
class FilteredCardList;

// ----------------------------------------------------------------------------- : StatsPanel

/// A panel for showing statistics on cards
class StatsPanel : public SetWindowPanel {
  public:
	StatsPanel(Window* parent, int id);
	
	// --------------------------------------------------- : UI
	
	virtual void onChangeSet();
	virtual void onCommand(int id);
	
	// --------------------------------------------------- : Selection
	virtual CardP selectedCard() const;
	virtual void selectCard(const CardP& card);
	
	// --------------------------------------------------- : Data
  private:
	DECLARE_EVENT_TABLE();
	
	StatCategoryList* categories;
	GraphControl*     graph;
	FilteredCardList* card_list;
	
	void onGraphSelect(wxCommandEvent&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
