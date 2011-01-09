//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/set/stats_panel.hpp>
#include <gui/control/graph.hpp>
#include <gui/control/gallery_list.hpp>
#include <gui/control/filtered_card_list.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>
#include <data/game.hpp>
#include <data/statistics.hpp>
#include <data/action/value.hpp>
#include <util/window_id.hpp>
#include <util/alignment.hpp>
#include <util/tagged_string.hpp>
#include <gfx/gfx.hpp>
#include <wx/splitter.h>

DECLARE_TYPEOF_COLLECTION(StatsDimensionP);
DECLARE_TYPEOF_COLLECTION(String);
DECLARE_TYPEOF_COLLECTION(size_t);
DECLARE_TYPEOF_COLLECTION(CardP);
typedef pair<StatsDimensionP,String> pair_StatsDimensionP_String;
DECLARE_TYPEOF_COLLECTION(pair_StatsDimensionP_String);

// ----------------------------------------------------------------------------- : StatCategoryList
#if !USE_DIMENSION_LISTS

/// A list of fields of which the statistics can be shown
class StatCategoryList : public GalleryList {
  public:
	StatCategoryList(Window* parent, int id)
		: GalleryList(parent, id, wxVERTICAL)
	{
		item_size = subcolumns[0].size = wxSize(150, 23);
	}
	
	void show(const GameP&);
	
	/// The selected category
	inline StatsCategory& getSelection() {
		return *categories.at(subcolumns[0].selection);
	}
	
  protected:
	virtual size_t itemCount() const;
	virtual void drawItem(DC& dc, int x, int y, size_t item);
	
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
	subcolumns[0].selection = itemCount() > 0 ? 0 : NO_SELECTION;
}

size_t StatCategoryList::itemCount() const {
	return categories.size();
}

void StatCategoryList::drawItem(DC& dc, int x, int y, size_t item) {
	StatsCategory& cat = *categories.at(item);
	// draw icon
	if (!cat.icon_filename.empty() && !cat.icon.Ok()) {
		InputStreamP file = game->openIn(cat.icon_filename);
		Image img(*file);
		if (img.HasMask()) img.InitAlpha(); // we can't handle masks
		if (img.Ok()) {
			cat.icon = Bitmap(resample_preserve_aspect(img, 21, 21));
		}
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
#else

/// A list of fields of which the statistics can be shown
class StatDimensionList : public GalleryList {
  public:
	StatDimensionList(Window* parent, int id, bool show_empty, int dimension_count = 3)
		: GalleryList(parent, id, wxVERTICAL, false)
		, dimension_count(dimension_count)
		, prefered_dimension_count(dimension_count)
		, show_empty(show_empty)
	{
		//item_size = wxSize(150, 23);
		subcolumns[0].size = wxSize(140,23);
		if (dimension_count > 0) {
			subcolumns[0].selection  = NO_SELECTION;
			subcolumns[0].can_select = false;
			active_subcolumn = 1;
		}
		// additional columns
		SubColumn col;
		col.selection = show_empty ? NO_SELECTION : 0;
		col.can_select = true;
		col.offset.x = subcolumns[0].size.x + SPACING;
		col.size = wxSize(18,23);
		for (int i = 0 ; i < dimension_count ; ++i) {
			subcolumns.push_back(col);
			col.offset.x += col.size.x + SPACING;
		}
		// total
		item_size = wxSize(col.offset.x - SPACING, 23);
	}
	
	void show(const GameP&);
	
	/// The selected category
	StatsDimensionP getSelection(size_t subcol=(size_t)-1) {
		size_t sel = subcolumns[subcol+1].selection - show_empty;
		if (sel >= itemCount()) return StatsDimensionP();
		else                    return dimensions.at(sel);
	}
	
	/// The number of dimensions shown
	const int dimension_count;
	size_t prefered_dimension_count;
	
	void restrictDimensions(size_t dims) {
		prefered_dimension_count = dims;
		active_subcolumn = min(active_subcolumn, dims);
		RefreshSelection();
	}
	
  protected:
	virtual size_t itemCount() const;
	virtual void drawItem(DC& dc, int x, int y, size_t item);
	
	virtual double subcolumnActivity(size_t col) const {
		return col-1 >= prefered_dimension_count ? 0.2 : 0.7;
	}
	
	virtual void onSelect(size_t item, size_t old_col, bool& changes) {
		// swap selection with another subcolumn?
		for (size_t j = 1 ; j < subcolumns.size() ; ++j) {
			if (j != active_subcolumn && subcolumns[j].selection == item &&
			                             subcolumns[active_subcolumn].selection != item) {
				subcolumns[j].selection = subcolumns[active_subcolumn].selection;
				changes = true;
				break;
			}
		}
		// update prefered dimension count?
		if (active_subcolumn > prefered_dimension_count) {
			prefered_dimension_count = active_subcolumn;
			changes = true;
			RefreshSelection();
		}
		// decrease dimension count? (toggle last dimension)
		if (!changes && old_col == active_subcolumn) {
			if (active_subcolumn == prefered_dimension_count && prefered_dimension_count > 1) {
				prefered_dimension_count -= 1;
				selectSubColumn(prefered_dimension_count);
				changes = true;
			} else if (active_subcolumn != prefered_dimension_count) {
				active_subcolumn = prefered_dimension_count = active_subcolumn;
				RefreshSelection();
				changes = true;
			}
		}
	}
	
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
	if (dimension_count > 0) {
		for (int j = 0 ; j < dimension_count ; ++j) {
			subcolumns[j+1].selection = itemCount() > 0 ? 0 : NO_SELECTION;
		}
		prefered_dimension_count = 1;
	} else {
		subcolumns[0].selection = itemCount() > 0 ? 0 : NO_SELECTION;
	}
}

size_t StatDimensionList::itemCount() const {
	return dimensions.size() + show_empty;
}

void StatDimensionList::drawItem(DC& dc, int x, int y, size_t item) {
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
		if (img.HasMask()) img.InitAlpha(); // we can't handle masks
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
	// draw selection icon
	for (size_t j = 1 ; j <= dimensions.size() ; ++j) {
		bool prefered = j <= prefered_dimension_count;
		if (isSelected(item,j)) {
			// TODO: different icons for different dimensions
			/*
			*/
			int cx = x + subcolumns[j].offset.x + subcolumns[j].size.x/2;
			int cy = y + subcolumns[j].offset.y + subcolumns[j].size.y/2;
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.SetBrush(prefered ? wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)
			                     : lerp(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT),wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW),0.5));
			dc.DrawCircle(cx,cy,6);
		}
	}
}

#endif
// ----------------------------------------------------------------------------- : StatsPanel

StatsPanel::StatsPanel(Window* parent, int id)
	: SetWindowPanel(parent, id)
	, menuGraph(nullptr)
	, up_to_date(true), active(false)
{
	// delayed initialization by initControls()
}

void StatsPanel::initControls() {
	// init controls
	wxSplitterWindow* splitter;
	#if USE_SEPARATE_DIMENSION_LISTS
		for (int i = 0 ; i < 3 ; ++i) {
			dimensions[i] = new StatDimensionList(this, ID_FIELD_LIST, i > 0);
		}
	#elif USE_DIMENSION_LISTS
		dimensions = new StatDimensionList(this, ID_FIELD_LIST, false, 3);
	#else
		categories = new StatCategoryList(this, ID_FIELD_LIST);
	#endif
	splitter   = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	graph      = new GraphControl    (splitter, wxID_ANY);
	card_list  = new FilteredCardList(splitter, wxID_ANY);
	// init splitter
	splitter->SetMinimumPaneSize(100);
	splitter->SetSashGravity(0.6);
	splitter->SplitHorizontally(graph, card_list, -170);
	// init sizer
	wxSizer* s = new wxBoxSizer(wxHORIZONTAL);
	#if USE_SEPARATE_DIMENSION_LISTS
		wxSizer* s2 = new wxBoxSizer(wxVERTICAL);
			s2->Add(dimensions[0], 1, wxBOTTOM, 2);
			s2->Add(dimensions[1], 1, wxBOTTOM, 2);
			s2->Add(dimensions[2], 1);
		s->Add(s2, 0, wxEXPAND | wxRIGHT, 2);
	#elif USE_DIMENSION_LISTS
		s->Add(dimensions, 0, wxEXPAND | wxRIGHT, 2);
	#else
		s->Add(categories, 0, wxEXPAND | wxRIGHT, 2);
	#endif
	s->Add(splitter,   1, wxEXPAND);
	s->SetSizeHints(this);
	SetSizer(s);
	
	// init menu
	menuGraph = new IconMenu();
		menuGraph->Append(ID_GRAPH_PIE,         _("graph_pie"),         _MENU_("pie"),         _HELP_("pie"),         wxITEM_CHECK);
		menuGraph->Append(ID_GRAPH_BAR,         _("graph_bar"),         _MENU_("bar"),         _HELP_("bar"),         wxITEM_CHECK);
		menuGraph->Append(ID_GRAPH_STACK,       _("graph_stack"),       _MENU_("stack"),       _HELP_("stack"),       wxITEM_CHECK);
		menuGraph->Append(ID_GRAPH_SCATTER,     _("graph_scatter"),     _MENU_("scatter"),     _HELP_("scatter"),     wxITEM_CHECK);
		menuGraph->Append(ID_GRAPH_SCATTER_PIE, _("graph_scatter_pie"), _MENU_("scatter pie"), _HELP_("scatter pie"), wxITEM_CHECK);
}

StatsPanel::~StatsPanel() {
	delete menuGraph;
}

void StatsPanel::onChangeSet() {
	if (!isInitialized()) return;
	card_list->setSet(set);
	#if USE_SEPARATE_DIMENSION_LISTS
		for (int i = 0 ; i < 3 ; ++i) dimensions[i]->show(set->game);
	#elif USE_DIMENSION_LISTS
		dimensions->show(set->game);
	#else
		categories->show(set->game);
	#endif
	card = CardP();
	onChange();
}

void StatsPanel::onAction(const Action& action, bool undone) {
	if (!isInitialized()) return;
	TYPE_CASE_(action, ScriptValueEvent) {
		// ignore style only stuff
	} else {
		onChange();
	}
}

void StatsPanel::initUI   (wxToolBar* tb, wxMenuBar* mb) {
	// Controls
	if (!isInitialized()) {
		wxBusyCursor busy;
		initControls();
		CardP cur_card = card;
		onChangeSet();
		selectCard(cur_card);
	}
	// we are active
	active = true;
	if (!up_to_date) showCategory();
	// Toolbar
	#if USE_DIMENSION_LISTS || USE_SEPARATE_DIMENSION_LISTS
		tb->AddTool(ID_GRAPH_PIE,         _(""), load_resource_tool_image(_("graph_pie")),         wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("pie"),         _HELP_("pie"));
		tb->AddTool(ID_GRAPH_BAR,         _(""), load_resource_tool_image(_("graph_bar")),         wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("bar"),         _HELP_("bar"));
		tb->AddTool(ID_GRAPH_STACK,       _(""), load_resource_tool_image(_("graph_stack")),       wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("stack"),       _HELP_("stack"));
		tb->AddTool(ID_GRAPH_SCATTER,     _(""), load_resource_tool_image(_("graph_scatter")),     wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("scatter"),     _HELP_("scatter"));
		tb->AddTool(ID_GRAPH_SCATTER_PIE, _(""), load_resource_tool_image(_("graph_scatter_pie")), wxNullBitmap, wxITEM_CHECK, _TOOLTIP_("scatter pie"), _HELP_("scatter pie"));
		tb->Realize();
		// Menu
		mb->Insert(2, menuGraph, _MENU_("graph"));
	#endif
}
void StatsPanel::destroyUI(wxToolBar* tb, wxMenuBar* mb) {
	active = false;
	#if USE_DIMENSION_LISTS || USE_SEPARATE_DIMENSION_LISTS
		// Toolbar
		tb->DeleteTool(ID_GRAPH_PIE);
		tb->DeleteTool(ID_GRAPH_BAR);
		tb->DeleteTool(ID_GRAPH_STACK);
		tb->DeleteTool(ID_GRAPH_SCATTER);
		tb->DeleteTool(ID_GRAPH_SCATTER_PIE);
		// Menus
		mb->Remove(2);
	#endif
}

void StatsPanel::onUpdateUI(wxUpdateUIEvent& ev) {
	if (!isInitialized()) return;
	switch (ev.GetId()) {
		case ID_GRAPH_PIE: case ID_GRAPH_BAR: case ID_GRAPH_STACK: case ID_GRAPH_SCATTER: case ID_GRAPH_SCATTER_PIE: {
			GraphType type = (GraphType)(ev.GetId() - ID_GRAPH_PIE);
			ev.Check(graph->getLayout() == type);
			#if USE_SEPARATE_DIMENSION_LISTS
				ev.Enable(graph->getDimensionality() == dimensionality(type));
			#endif
			break;
		}
	}
}

void StatsPanel::onCommand(int id) {
	if (!isInitialized()) return;
	switch (id) {
		case ID_FIELD_LIST: {
			onChange();
			break;
		}
		case ID_GRAPH_PIE: case ID_GRAPH_BAR: case ID_GRAPH_STACK: case ID_GRAPH_SCATTER: case ID_GRAPH_SCATTER_PIE: {
			GraphType type = (GraphType)(id - ID_GRAPH_PIE);
			showLayout(type);
			break;
		}
	}
}

// ----------------------------------------------------------------------------- : Updating graph

void StatsPanel::onChange() {
	if (active) {
		showCategory();
	} else {
		up_to_date = false; // update later
	}
}

void StatsPanel::showCategory(const GraphType* prefer_layout) {
	up_to_date = true;
	// find dimensions and layout
	#if USE_DIMENSION_LISTS || USE_SEPARATE_DIMENSION_LISTS
		// dimensions
		vector<StatsDimensionP> dims;
		#if USE_SEPARATE_DIMENSION_LISTS
			for (int i = 0 ; i < 3 ; ++i) {
				StatsDimensionP dim = dimensions[i]->getSelection();
				if (dim) dims.push_back(dim);
			}
		#else // USE_DIMENSION_LISTS
			for (size_t i = 0 ; i < dimensions->prefered_dimension_count ; ++i) {
				StatsDimensionP dim = dimensions->getSelection(i);
				if (dim) dims.push_back(dim);
			}
		#endif
		// layout
		GraphType layout = prefer_layout ? *prefer_layout : graph->getLayout();
		if (dimensionality(layout) != dims.size()) {
			// we must switch to another layout
			layout = dims.size() == 1 ? GRAPH_TYPE_BAR
			       : dims.size() == 2 ? (layout == GRAPH_TYPE_SCATTER_PIE || dims[1]->numeric
			                              ? GRAPH_TYPE_SCATTER : GRAPH_TYPE_STACK)
			       :                    GRAPH_TYPE_SCATTER_PIE;
		}
	#else
		if (!categories->hasSelection()) return
		StatsCategory& cat = categories->getSelection();
		// dimensions
		cat.find_dimensions(set->game->statistics_dimensions);
		vector<StatsDimensionP>& dims = cat.dimensions;
		// layout
		GraphType layout = cat.type;
	#endif
	// create axes
	GraphDataPre d;
	FOR_EACH(dim, dims) {
		d.axes.push_back(intrusive(new GraphAxis(
			dim->name,
			dim->colors.empty() ? AUTO_COLOR_EVEN : AUTO_COLOR_NO,
			dim->numeric,
			dim->bin_size,
			&dim->colors,
			dim->groups.empty() ? nullptr : &dim->groups
			)
		));
	}
	// find values for each card
	for (size_t i = 0 ; i < set->cards.size() ; ++i) {
		Context& ctx = set->getContext(set->cards[i]);
		GraphElementP e(new GraphElement(i));
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
	// update graph and card list
	graph->setLayout(layout, true);
	graph->setData(d);
	filterCards();
}
void StatsPanel::showLayout(GraphType layout) {
	#if USE_DIMENSION_LISTS && !USE_SEPARATE_DIMENSION_LISTS
		// make sure we have the right number of data dimensions
		if (dimensions->prefered_dimension_count != dimensionality(layout)) {
			dimensions->restrictDimensions(dimensionality(layout));
			showCategory(&layout); // full update
			return;
		}
	#endif
	graph->setLayout(layout);
	graph->Refresh(false);
}

void StatsPanel::onGraphSelect(wxCommandEvent&) {
	filterCards();
}

// ----------------------------------------------------------------------------- : Filtering card list

class StatsFilter : public CardListFilter {
  public:
	StatsFilter(GraphData& data, const vector<int> match) {
		data.indices(match, indices);
	}
	virtual void getItems(const vector<CardP>& cards, vector<VoidP>& out) const {
		FOR_EACH_CONST(idx, indices) {
			out.push_back(cards.at(idx));
		}
	}
	
	vector<size_t> indices; ///< Indices of cards to select
};

void StatsPanel::filterCards() {
	intrusive_ptr<StatsFilter> filter(new StatsFilter(*graph->getData(), graph->getSelectionIndices()));
	card_list->setFilter(filter);
}

// ----------------------------------------------------------------------------- : Events

BEGIN_EVENT_TABLE(StatsPanel, wxPanel)
	EVT_GRAPH_SELECT(wxID_ANY, StatsPanel::onGraphSelect)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------- : Selection

CardP StatsPanel::selectedCard() const {
	if (!isInitialized()) return CardP();
	return card_list->getCard();
}
void StatsPanel::selectCard(const CardP& card) {
	this->card = card;
	if (!isInitialized()) return;
	card_list->setCard(card);
	
}
