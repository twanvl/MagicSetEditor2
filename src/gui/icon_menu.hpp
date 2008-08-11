//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_ICON_MENU
#define HEADER_GUI_ICON_MENU

// ----------------------------------------------------------------------------- : Includes

#include "../util/prec.hpp"

// ----------------------------------------------------------------------------- : IconMenu

/// Helper class for menus with icons
/** This class functions just like a normal wxMenu.
 *  The difference is that Append takes an extra parameter:
 *    the resource name of the bitmap to use.
 *  Bitmaps are resized (cut) to 16x16 pixels.
 */
class IconMenu : public wxMenu {
  public:
	/// Append a menu item, with an image (loaded from a resource)
	void Append(int id, const String& resource, const String& text, const String& help, int style = wxITEM_NORMAL, wxMenu* submenu = nullptr);
	/// Append a menu item, without an image
	void Append(int id, const String& text, const String& help);
	/// Append a menu item, without an image
	void Append(int id, const String& text, const String& help, wxMenu* submenu);
	/// Append a menu item, without an image
	void Append(wxMenuItem* item);
	/// Insert a menu item, without an image
	void Insert(size_t pos, int id, const String& text, const String& help);
};

void set_menu_item_image(wxMenuItem* menuitem, const String& resource);

// ----------------------------------------------------------------------------- : EOF
#endif
