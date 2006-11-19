//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/set/stats_panel.hpp>
#include <gui/control/graph.hpp>
#include <gui/control/gallery_list.hpp>
#include <gui/control/filtered_card_list.hpp>
#include <wx/splitter.h>

// ----------------------------------------------------------------------------- : StatFieldList

/// A list of fields of which the statistics can be shown
class StatFieldList : public GalleryList {
  public:
	StatFieldList(Window* parent, int id)
		: GalleryList(parent, id, wxVERTICAL)
	{
		item_size = wxSize(100, 30);
	}
	
  protected:
	virtual size_t itemCount() const;
	virtual void drawItem(DC& dc, int x, int y, size_t item, bool selected);
};

size_t StatFieldList::itemCount() const {
	return 0; // TODO
}

void StatFieldList::drawItem(DC& dc, int x, int y, size_t item, bool selected) {
	// TODO
}

// ----------------------------------------------------------------------------- : StatsPanel

StatsPanel::StatsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	wxSplitterWindow* splitter;
	fields    = new StatFieldList   (this, wxID_ANY);
	splitter  = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	graph     = new GraphControl    (splitter, wxID_ANY);
	card_list = new FilteredCardList(splitter, wxID_ANY);
	// init splitter
	splitter->SetMinimumPaneSize(100);
	splitter->SetSashGravity(1.0);
	splitter->SplitHorizontally(graph, card_list, -100);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(fields,   0, wxEXPAND | wxRIGHT, 2);
	s->Add(splitter, 1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
}
