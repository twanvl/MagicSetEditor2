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

class StatFieldList;
class GraphControl;
class FilteredCardList;

// ----------------------------------------------------------------------------- : StatsPanel

class StatsPanel : public SetWindowPanel {
  public:
	StatsPanel(Window* parent, int id);
	
//	virtual void onUpdateUI(wxUpdateUIEvent& e);
//	virtual void onCommand(int id);
	
  private:
	StatFieldList*    fields;
	GraphControl*     graph;
	FilteredCardList* card_list;
};

// ----------------------------------------------------------------------------- : EOF
#endif
