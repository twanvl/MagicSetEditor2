//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/util.hpp>
#include <util/error.hpp>
#include <util/rotation.hpp>
#include <wx/mstream.h>

// ----------------------------------------------------------------------------- : DC related

/// Fill a DC with a single color
void clearDC(DC& dc, const wxBrush& brush) {
	wxSize size = dc.GetSize();
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(brush);
	dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
}

void draw_checker(RotatedDC& dc, const RealRect& rect) {
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.DrawRectangle(rect);
	dc.SetBrush(Color(235,235,235));
	const double checker_size = 10;
	int odd = 0;
	for (double y = 0 ; y < rect.size.height ; y += checker_size) {
		for (double x = odd * checker_size ; x < rect.size.width ; x += checker_size * 2) {
			dc.DrawRectangle(RealRect(
								rect.position.x + x,
								rect.position.y + y,
								min(checker_size, rect.size.width  - x),
								min(checker_size, rect.size.height - y)
							));
		}
		odd = 1 - odd;
	}
}

// ----------------------------------------------------------------------------- : Image related

Image load_resource_image(String name) {
	#ifdef __WXMSW__
		// Load resource
		// based on wxLoadUserResource
		// The image can be in an IMAGE resource, in any file format
		HRSRC hResource = ::FindResource(wxGetInstance(), name, _("IMAGE"));
		if ( hResource == 0 ) throw InternalError(_("Resource not found: ") + name);
		
		HGLOBAL hData = ::LoadResource(wxGetInstance(), hResource);
		if ( hData == 0 ) throw InternalError(_("Resource not an image: ") + name);
		
		char* data = (char *)::LockResource(hData);
		if ( !data ) throw InternalError(_("Resource cannot be locked: ") + name);

		int len = ::SizeofResource(wxGetInstance(), hResource);
		wxMemoryInputStream stream(data, len);
		return wxImage(stream);
	#endif
}