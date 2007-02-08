//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/control/native_look_editor.hpp>
#include <gui/value/editor.hpp>
#include <gui/util.hpp>
#include <data/stylesheet.hpp>

DECLARE_TYPEOF_COLLECTION(ValueViewerP);
DECLARE_TYPEOF_NO_REV2(IndexMap<FieldP,StyleP>);

// ----------------------------------------------------------------------------- : NativeLookEditor

NativeLookEditor::NativeLookEditor(Window* parent, int id, long style)
	: DataEditor(parent, id, style)
{}

Rotation NativeLookEditor::getRotation() const {
	return Rotation(0, RealRect(RealPoint(0,0),GetClientSize()));
}

void NativeLookEditor::draw(DC& dc) {
	RotatedDC rdc(dc, getRotation(), false);
	DataViewer::draw(rdc, wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
}
void NativeLookEditor::drawViewer(RotatedDC& dc, ValueViewer& v) {
	if (!shouldDraw(v)) return;
	// draw background
	Style& s = *v.getStyle();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	dc.DrawRectangle(s.getRect().grow(1));
	// draw label
	dc.SetFont(*wxNORMAL_FONT);
	// TODO : tr using stylesheet or using game?
	dc.DrawText(tr(*set->game, s.fieldP->name, capitalize_sentence(s.fieldP->name)),
	            RealPoint(margin_left, s.top + 1));
	// draw 3D border
	draw_control_border(this, dc.getDC(), RealRect(s.left - 1, s.top - 1, s.width + 2, s.height + 2));
	// draw viewer
	v.draw(dc);
}

void NativeLookEditor::resizeViewers() {
	// size stuff
	double y = margin;
	int w;
	GetClientSize(&w, 0);
	const int default_height = 17;
	// Set editor sizes
	FOR_EACH(v, viewers) {
		StyleP s = v->getStyle();
		s->left = margin + label_width;
		s->top  = y;
		s->width  = w - s->left - margin;
		s->height = default_height;
		ValueEditor* e = v->getEditor();
		if (e) e->determineSize();
		y += s->height + vspace;
	}
}

void NativeLookEditor::onInit() {
	// Give viewers a chance to show/hide controls (scrollbar) when selecting other editors
	FOR_EACH_EDITOR {
		e->onShow(true);
	}
	resizeViewers();
}

wxSize NativeLookEditor::DoGetBestSize() const {
	return wxSize(200, 200);
}
void NativeLookEditor::onSize(wxSizeEvent& ev) {
	resizeViewers();
	Refresh(false);
}

BEGIN_EVENT_TABLE(NativeLookEditor, DataEditor)
	EVT_SIZE        (NativeLookEditor::onSize)
END_EVENT_TABLE()


// ----------------------------------------------------------------------------- : SetInfoEditor

SetInfoEditor::SetInfoEditor(Window* parent, int id, long style)
	: NativeLookEditor(parent, id, style)
{}

void SetInfoEditor::onChangeSet() {
	setStyles(set->stylesheet, set->stylesheet->set_info_style);
	setData(set->data);
}

// ----------------------------------------------------------------------------- : StylingEditor

StylingEditor::StylingEditor(Window* parent, int id, long style)
	: NativeLookEditor(parent, id, style)
{}

void StylingEditor::showStylesheet(const StyleSheetP& stylesheet) {
	this->stylesheet = stylesheet;
	setStyles(set->stylesheet, stylesheet->styling_style);
	setData(set->stylingDataFor(*stylesheet));
}

void StylingEditor::onChangeSet() {
	showStylesheet(set->stylesheet);
}
