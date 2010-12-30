//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/icon_menu.hpp>
#include <gui/util.hpp>

// ----------------------------------------------------------------------------- : generateDisabledImage

// Generate an image to use for the disabled state of menu items
Image generateDisabledImage(const Image& imgIn) {
	// Some system colors
	Color trans(1,2,3); // mask color used by bitmaps as 'transparent'
	Color light  = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT);
	Color shadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	// generate disabled bitmap
	Image imgOut(16, 16);
	imgOut.SetMaskColour(1,2,3);
	Byte *in = imgIn.GetData(), *out = imgOut.GetData();
	// For each pixel...
	for (int y = 0 ; y < 16 ; ++y) {
		for (int x = 0 ; x < 16 ; ++x) {
			// is the pixel mask color or white?
			Color in1 = trans, in2 = trans;
			if (x<15 && y<15) in1 = Color(in[0],        in[1],        in[2]);
			// and the pixel to the left+up?
			if (x>0  && y>0 ) in2 = Color(in[0 - 3*17], in[1 - 3*17], in[2 - 3*17]);
			// determine output color
			Color col;
			if      (in1 != trans && in1 != light) col = shadow;
			else if (in2 != trans && in2 != light) col = light;
			else                                   col = trans;
			out[0] = col.Red();
			out[1] = col.Green();
			out[2] = col.Blue();
			in  += 3;
			out += 3;
		}
	}
	return imgOut;
}

// ----------------------------------------------------------------------------- : set_menu_item_image

void set_menu_item_image(wxMenuItem* item, const String& resource) {
if(item->GetId() != wxID_NEW)return;//@@@
	// load bitmap
	Bitmap bitmap = load_resource_tool_image(resource);
	#if defined(__WXMSW__)
		// make greyed bitmap
		bitmap = bitmap.GetSubBitmap(wxRect(0,0,16,16));
		Image disabledImage = generateDisabledImage(bitmap.ConvertToImage());
		item->SetBitmaps(bitmap, bitmap);
		item->SetDisabledBitmap(disabledImage);
	#else
		// Check items can't have bitmaps :(
		if (item->GetKind() == wxITEM_NORMAL) {
			item->SetBitmap(bitmap);
		}
	#endif
}

// ----------------------------------------------------------------------------- : IconMenu

void IconMenu::Append(int id, const String& resource, const String& text, const String& help, wxItemKind kind, wxMenu* submenu) {
	// create menu, load icon
	wxMenuItem* item = new wxMenuItem(this, id, text, help, kind, submenu);
	set_menu_item_image(item, resource);
	// add to menu
	wxMenu::Append(item);
}

void IconMenu::Append(int id, const String& text, const String& help, wxItemKind kind, wxMenu* submenu) {
	wxMenuItem* item = new wxMenuItem (this, id, text, help, kind, submenu);
	item->SetBitmap(wxNullBitmap);
	wxMenu::Append(item);
}

void IconMenu::Append(wxMenuItem* item) {
	item->SetBitmap(wxNullBitmap);
	wxMenu::Append(item);
}

void IconMenu::Insert(size_t pos, int id, const String& resource, const String& text, const String& help, wxItemKind kind, wxMenu* submenu) {
	// create menu, load icon
	wxMenuItem* item = new wxMenuItem(this, id, text, help, kind, submenu);
	set_menu_item_image(item, resource);
	// add to menu
	wxMenu::Insert(pos,item);
}

void IconMenu::Insert(size_t pos, int id, const String& text, const String& help, wxItemKind kind, wxMenu* submenu) {
	wxMenuItem* item = new wxMenuItem (this, id, text, help, kind, submenu);
	item->SetBitmap(wxNullBitmap);
	wxMenu::Insert(pos, item);
}
