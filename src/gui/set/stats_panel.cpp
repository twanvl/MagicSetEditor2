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
#include <data/game.hpp>
#include <data/statistics.hpp>
#include <util/window_id.hpp>
#include <util/alignment.hpp>
#include <gfx/gfx.hpp>
#include <wx/splitter.h>

DECLARE_TYPEOF_COLLECTION(StatsDimensionP);
DECLARE_TYPEOF_COLLECTION(String);
DECLARE_TYPEOF_COLLECTION(CardP);
typedef pair<StatsDimensionP,String> pair_StatsDimensionP_String;
DECLARE_TYPEOF_COLLECTION(pair_StatsDimensionP_String);

// ----------------------------------------------------------------------------- : StatCategoryList

/// A list of fields of which the statistics can be shown
class StatCategoryList : public GalleryList {
  public:
	StatCategoryList(Window* parent, int id)
		: GalleryList(parent, id, wxVERTICAL)
	{
		item_size = wxSize(150, 23);
	}
	
	void show(const GameP&);
	
	/// The selected category
	inline StatsCategory& getSelection() {
		return *game->statistics_categories.at(selection);
	}
	
  protected:
	virtual size_t itemCount() const;
	virtual void drawItem(DC& dc, int x, int y, size_t item, bool selected);

  private:
	GameP game;
};

void StatCategoryList::show(const GameP& game) {
	this->game = game;
	update();
}

size_t StatCategoryList::itemCount() const {
	return game ? game->statistics_categories.size() : 0;
}

void StatCategoryList::drawItem(DC& dc, int x, int y, size_t item, bool selected) {
	StatsCategory& cat = *game->statistics_categories.at(item);
	// draw icon
	if (!cat.icon_filename.empty() && !cat.icon.Ok()) {
		InputStreamP file = game->openIn(cat.icon_filename);
		Image img(*file);
		Image resampled(21, 21);
		resample_preserve_aspect(img, resampled);
		if (img.Ok()) cat.icon = Bitmap(resampled);
	}
	if (cat.icon.Ok()) {
		dc.DrawBitmap(cat.icon, x+1, y+1);
	}
	// draw name
	RealRect rect(RealPoint(x + 24, y), RealSize(item_size.width - 30, item_size.height));
	String str = capitalize(cat.name);
//	dc.SetFont(wxFont(9.5 * text_scaling, wxSWISS, wxNORMAL, wxNORMAL, false,_("Arial")));
	dc.SetFont(*wxNORMAL_FONT);
	int w, h;
	dc.GetTextExtent(str, &w, &h);
	RealSize size = RealSize(w,h);
	RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, size, rect);
//	draw_resampled_text(dc, RealRect(pos, size), 0, 0, 0, str);
	dc.DrawText(str, pos.x, pos.y);
}

// ----------------------------------------------------------------------------- : StatsPanel

StatsPanel::StatsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
{
	// init controls
	wxSplitterWindow* splitter;
	categories = new StatCategoryList(this, ID_FIELD_LIST);
	splitter   = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
	graph      = new GraphControl    (splitter, wxID_ANY);
	card_list  = new FilteredCardList(splitter, wxID_ANY);
	// init splitter
	splitter->SetMinimumPaneSize(100);
	splitter->SetSashGravity(1.0);
	splitter->SplitHorizontally(graph, card_list, -100);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	s->Add(categories, 0, wxEXPAND | wxRIGHT, 2);
	s->Add(splitter,   1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
}

void StatsPanel::onChangeSet() {
	card_list->setSet(set);
	categories->show(set->game);
}

void StatsPanel::onCommand(int id) {
	switch (id) {
		case ID_FIELD_LIST: {
			// change graph data
			if (categories->hasSelection()) {
				StatsCategory& cat = categories->getSelection();
				GraphDataPre d;
				FOR_EACH(dim, cat.dimensions) {
					d.axes.push_back(new_shared3<GraphAxis>(dim->name, true, dim->numeric));
				}
				FOR_EACH(card, set->cards) {
					Context& ctx = set->getContext(card);
					GraphElementP e(new GraphElement);
					FOR_EACH(dim, cat.dimensions) {
						e->values.push_back(*dim->script.invoke(ctx));
					}
					d.elements.push_back(e);
				}
				graph->setData(d);
			}
			break;
		}
	}
}

// ----------------------------------------------------------------------------- : Filtering card list

class StatsFilter : public CardListFilter {
  public:
	StatsFilter(Set& set)
		: set(set)
	{}
	virtual bool keep(const CardP& card) {
		Context& ctx = set.getContext(card);
		FOR_EACH(v, values) {
			if (*v.first->script.invoke(ctx) != v.second) return false;
		}
		return true;
	}
	
	vector<pair<StatsDimensionP, String> > values; ///< Values selected along each dimension
	Set& set;
};

void StatsPanel::onGraphSelect(wxCommandEvent&) {
	if (!categories->hasSelection()) return;
	shared_ptr<StatsFilter> filter(new StatsFilter(*set));
	StatsCategory& cat = categories->getSelection();
	vector<pair<StatsDimensionP, String> > values;
	int i = 0;
	FOR_EACH(dim, cat.dimensions) {
		if (graph->hasSelection(i)) {
			filter->values.push_back(make_pair(dim, graph->getSelection(i)));
		}
		i++;
	}
	card_list->setFilter(filter);
}

BEGIN_EVENT_TABLE(StatsPanel, wxPanel)
	EVT_GRAPH_SELECT(wxID_ANY, StatsPanel::onGraphSelect)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : Selection

CardP StatsPanel::selectedCard() const {
	return card_list->getCard();
}
void StatsPanel::selectCard(const CardP& card) {
	card_list->setCard(card);
	
}
