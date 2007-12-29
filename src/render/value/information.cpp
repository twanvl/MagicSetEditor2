//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/information.hpp>
#include <render/card/viewer.hpp>

// ----------------------------------------------------------------------------- : InfoValueViewer

IMPLEMENT_VALUE_VIEWER(Info);

void InfoValueViewer::draw(RotatedDC& dc) {
	dc.SetPen(*wxTRANSPARENT_PEN);
	if (nativeLook()) {
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
		dc.SetFont(wxFont(16, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _("Arial")));
	} else {
		dc.SetTextForeground(style().font.color);
		dc.SetBrush(style().background_color);
		dc.SetFont(style().font, 1.0);
	}
	// draw background
	RealRect rect = style().getInternalRect();
	dc.DrawRectangle(rect.grow(2));
	// draw text
	rect = rect.move(
		style().padding_left,
		style().padding_top,
		-style().padding_left - style().padding_right,
		-style().padding_top  - style().padding_bottom
	);
	RealSize size = dc.GetTextExtent(value().value);
	dc.DrawText(value().value, align_in_rect(style().alignment, size, rect));
}
