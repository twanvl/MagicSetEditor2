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

// ----------------------------------------------------------------------------- : StatCategoryList

/// A list of fields of which the statistics can be shown
class StatCategoryList : public GalleryList {
  public:
	StatCategoryList(Window* parent, int id)
		: GalleryList(parent, id, wxVERTICAL)
	{
		item_size = wxSize(140, 23);
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
	RealRect rect(RealPoint(x + 23, y), RealSize(item_size.width - 30, item_size.height));
	String str = capitalize(cat.name);
	dc.SetFont(wxFont(10,wxSWISS,wxNORMAL,wxBOLD,false,_("Arial")));
	int w, h;
	dc.GetTextExtent(str, &w, &h);
	RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, RealSize(w,h), rect);
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

class StatsFilter : public CardListFilter {
  public:
	StatsFilter(Set& set, const vector<StatsDimensionP>& dims, const vector<String>& values)
		: set(set), dims(dims), values(values)
	{}
	virtual bool keep(const CardP& card) {
		Context& ctx = set.getContext(card);
		FOR_EACH_2(d, dims, v, values) {
			if (*d->script.invoke(ctx) != v) return false;
		}
		return true;
	}
  private:
	Set&                    set;
	vector<StatsDimensionP> dims;
	vector<String>          values;
};

// ----------------------------------------------------------------------------- : Selection

CardP StatsPanel::selectedCard() const {
	return card_list->getCard();
}
void StatsPanel::selectCard(const CardP& card) {
	card_list->setCard(card);
	
}
