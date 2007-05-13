//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
#include <util/tagged_string.hpp>
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
	// select first item
	selection = itemCount() > 0 ? 0 : NO_SELECTION;
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
	RealRect rect(RealPoint(x + 24, y), RealSize(item_size.x - 30, item_size.y));
	String str = capitalize(cat.name);
	dc.SetFont(*wxNORMAL_FONT);
	int w, h;
	dc.GetTextExtent(str, &w, &h);
	RealSize size = RealSize(w,h);
	RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, size, rect);
	dc.DrawText(str, (int)pos.x, (int)pos.y);
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
	splitter->SetSashGravity(0.6);
	splitter->SplitHorizontally(graph, card_list, -170);
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
	onCategorySelect();
}

void StatsPanel::onCommand(int id) {
	switch (id) {
		case ID_FIELD_LIST: {
			onCategorySelect();
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
			if (v.first->script.invoke(ctx)->toString() != v.second) return false;
		}
		return true;
	}
	
	vector<pair<StatsDimensionP, String> > values; ///< Values selected along each dimension
	Set& set;
};

void StatsPanel::onCategorySelect() {
	// change graph data
	if (categories->hasSelection()) {
		StatsCategory& cat = categories->getSelection();
		GraphDataPre d;
		cat.find_dimensions(set->game->statistics_dimensions);
		// create axes
		FOR_EACH(dim, cat.dimensions) {
			d.axes.push_back(new_intrusive4<GraphAxis>(
				dim->name,
				dim->colors.empty() ? AUTO_COLOR_EVEN : AUTO_COLOR_NO,
				dim->numeric,
				&dim->colors
				)
			);
		}
		// find values
		FOR_EACH(card, set->cards) {
			Context& ctx = set->getContext(card);
			GraphElementP e(new GraphElement);
			bool show = true;
			FOR_EACH(dim, cat.dimensions) {
				String value = untag(dim->script.invoke(ctx)->toString());
				e->values.push_back(value);
				if (value.empty() && !dim->show_empty) {
					// don't show this element
					show = false;
					break;
				}
			}
			if (show) {
				d.elements.push_back(e);
			}
		}
		// TODO graph->setLayout(cat.type)
		graph->setData(d);
		filterCards();
	}
}
void StatsPanel::onGraphSelect(wxCommandEvent&) {
	filterCards();
}

void StatsPanel::filterCards() {
	if (!categories->hasSelection()) return;
	intrusive_ptr<StatsFilter> filter(new StatsFilter(*set));
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
