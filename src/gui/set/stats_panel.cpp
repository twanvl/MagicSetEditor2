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
		return *categories.at(selection);
	}
	
  protected:
	virtual size_t itemCount() const;
	virtual void drawItem(DC& dc, int x, int y, size_t item, bool selected);
	
  private:
	GameP game;
	vector<StatsCategoryP> categories; ///< Categories, sorted by position_hint
};

struct ComparePositionHint{
	inline bool operator () (const StatsCategoryP& a, const StatsCategoryP& b) {
		return a->position_hint < b->position_hint;
	}
};

void StatCategoryList::show(const GameP& game) {
	this->game = game;
	categories = game->statistics_categories;
	stable_sort(categories.begin(), categories.end(), ComparePositionHint());
	update();
	// select first item
	selection = itemCount() > 0 ? 0 : NO_SELECTION;
}

size_t StatCategoryList::itemCount() const {
	return categories.size();
}

void StatCategoryList::drawItem(DC& dc, int x, int y, size_t item, bool selected) {
	StatsCategory& cat = *categories.at(item);
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

// ----------------------------------------------------------------------------- : StatDimensionList

/// A list of fields of which the statistics can be shown
class StatDimensionList : public GalleryList {
  public:
	StatDimensionList(Window* parent, int id, bool show_empty)
		: GalleryList(parent, id, wxVERTICAL)
		, show_empty(show_empty)
	{
		item_size = wxSize(150, 23);
	}
	
	void show(const GameP&);
	
	/// The selected category
	inline StatsDimensionP getSelection() {
		if (show_empty && selection == 0) return StatsDimensionP();
		return dimensions.at(selection - show_empty);
	}
	
  protected:
	virtual size_t itemCount() const;
	virtual void drawItem(DC& dc, int x, int y, size_t item, bool selected);
	
  private:
	GameP game;
	bool show_empty;
	vector<StatsDimensionP> dimensions; ///< Dimensions, sorted by position_hint
};

struct ComparePositionHint2{
	inline bool operator () (const StatsDimensionP& a, const StatsDimensionP& b) {
		return a->position_hint < b->position_hint;
	}
};

void StatDimensionList::show(const GameP& game) {
	this->game = game;
	dimensions = game->statistics_dimensions;
	stable_sort(dimensions.begin(), dimensions.end(), ComparePositionHint2());
	update();
	// select first item
	selection = itemCount() > 0 ? 0 : NO_SELECTION;
}

size_t StatDimensionList::itemCount() const {
	return dimensions.size() + show_empty;
}

void StatDimensionList::drawItem(DC& dc, int x, int y, size_t item, bool selected) {
	if (show_empty && item == 0) {
		RealRect rect(RealPoint(x + 24, y), RealSize(item_size.x - 30, item_size.y));
		String str = _("None");
		dc.SetFont(*wxNORMAL_FONT);
		int w, h;
		dc.GetTextExtent(str, &w, &h);
		RealSize size = RealSize(w,h);
		RealPoint pos = align_in_rect(ALIGN_MIDDLE_LEFT, size, rect);
		dc.DrawText(str, (int)pos.x, (int)pos.y);
		return;
	}
	StatsDimension& dim = *dimensions.at(item - show_empty);
	// draw icon
	if (!dim.icon_filename.empty() && !dim.icon.Ok()) {
		InputStreamP file = game->openIn(dim.icon_filename);
		Image img(*file);
		Image resampled(21, 21);
		resample_preserve_aspect(img, resampled);
		if (img.Ok()) dim.icon = Bitmap(resampled);
	}
	if (dim.icon.Ok()) {
		dc.DrawBitmap(dim.icon, x+1, y+1);
	}
	// draw name
	RealRect rect(RealPoint(x + 24, y), RealSize(item_size.x - 30, item_size.y));
	String str = capitalize(dim.name);
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
	, up_to_date(true), active(false)
{
	// init controls
	wxSplitterWindow* splitter;
	//%categories = new StatCategoryList(this, ID_FIELD_LIST);
	for (int i = 0 ; i < 3 ; ++i) {
		dimensions[i] = new StatDimensionList(this, ID_FIELD_LIST, i > 0);
	}
	splitter   = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	graph      = new GraphControl    (splitter, wxID_ANY);
	card_list  = new FilteredCardList(splitter, wxID_ANY);
	// init splitter
	splitter->SetMinimumPaneSize(100);
	splitter->SetSashGravity(0.6);
	splitter->SplitHorizontally(graph, card_list, -170);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	//%s->Add(categories, 0, wxEXPAND | wxRIGHT, 2);
	wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
		s2->Add(dimensions[0], 1, wxBOTTOM, 2);
		s2->Add(dimensions[1], 1, wxBOTTOM, 2);
		s2->Add(dimensions[2], 1);
	s->Add(s2, 0, wxEXPAND | wxRIGHT, 2);
	s->Add(splitter,   1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
}

void StatsPanel::onChangeSet() {
	card_list->setSet(set);
	//%categories->show(set->game);
	for (int i = 0 ; i < 3 ; ++i) dimensions[i]->show(set->game);
	onChange();
}

void StatsPanel::onAction(const Action&, bool undone) {
	onChange();
}

void StatsPanel::initUI   (wxToolBar*, wxMenuBar*) {
	active = true;
	if (!up_to_date) showCategory();
}
void StatsPanel::destroyUI(wxToolBar*, wxMenuBar*) {
	active = false;
}

void StatsPanel::onCommand(int id) {
	switch (id) {
		case ID_FIELD_LIST: {
			onChange();
			break;
		}
	}
}

// ----------------------------------------------------------------------------- : Filtering card list

bool chosen(const String& choice, const String& input);

class StatsFilter : public CardListFilter {
  public:
	StatsFilter(Set& set)
		: set(set)
	{}
	virtual bool keep(const CardP& card) {
		Context& ctx = set.getContext(card);
		FOR_EACH(v, values) {
			StatsDimension& dim = *v.first;
			String value = untag(dim.script.invoke(ctx)->toString());
			if (dim.split_list) {
				if (!chosen(v.second, value)) return false;
			} else {
				if (value != v.second) return false;
			}
		}
		return true;
	}
	
	vector<pair<StatsDimensionP, String> > values; ///< Values selected along each dimension
	Set& set;
};

void StatsPanel::onChange() {
	if (active) {
		showCategory();
	} else {
		up_to_date = false; // update later
	}
}

void StatsPanel::showCategory() {
	up_to_date = true;
	// change graph data
	GraphDataPre d;
	// create axes
	vector<StatsDimensionP> dims;
	for (int i = 0 ; i < 3 ; ++i) {
		StatsDimensionP dim = dimensions[i]->getSelection();
		if (!dim) continue;
		dims.push_back(dim);
		d.axes.push_back(new_intrusive5<GraphAxis>(
			dim->name,
			dim->colors.empty() ? AUTO_COLOR_EVEN : AUTO_COLOR_NO,
			dim->numeric,
			&dim->colors,
			dim->groups.empty() ? nullptr : &dim->groups
			)
		);
	}
	// find values
	FOR_EACH(card, set->cards) {
		Context& ctx = set->getContext(card);
		GraphElementP e(new GraphElement);
		bool show = true;
		FOR_EACH(dim, dims) {
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
	// split lists
	size_t dim_id = 0;
	FOR_EACH(dim, dims) {
		if (dim->split_list) d.splitList(dim_id);
		++dim_id;
	}
	graph->setLayout(dims.size() == 1 ? GRAPH_TYPE_BAR
	                :dims.size() == 2 ? GRAPH_TYPE_STACK
	                :                   GRAPH_TYPE_SCATTER_PIE);
	graph->setData(d);
	filterCards();
	/*
	if (categories->hasSelection()) {
		StatsCategory& cat = categories->getSelection();
		GraphDataPre d;
		cat.find_dimensions(set->game->statistics_dimensions);
		// create axes
		FOR_EACH(dim, cat.dimensions) {
			d.axes.push_back(new_intrusive5<GraphAxis>(
				dim->name,
				dim->colors.empty() ? AUTO_COLOR_EVEN : AUTO_COLOR_NO,
				dim->numeric,
				&dim->colors,
				dim->groups.empty() ? nullptr : &dim->groups
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
		// split lists
		size_t dim_id = 0;
		FOR_EACH(dim, cat.dimensions) {
			if (dim->split_list) d.splitList(dim_id);
			++dim_id;
		}
		graph->setLayout(cat.type);
		graph->setData(d);
		filterCards();
	}
	*/
}
void StatsPanel::onGraphSelect(wxCommandEvent&) {
	filterCards();
}

void StatsPanel::filterCards() {
	/*if (!categories->hasSelection()) return;
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
	*/
	intrusive_ptr<StatsFilter> filter(new StatsFilter(*set));
	int dims = 0;
	for (int i = 0 ; i < 3 ; ++i) {
		StatsDimensionP dim = dimensions[i]->getSelection();
		if (!dim) continue;
		++dims;
		if (graph->hasSelection(i)) {
			filter->values.push_back(make_pair(dim, graph->getSelection(i)));
		}
	}
	if (dims == 0) return;
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
