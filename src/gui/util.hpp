//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

/** @file gui/util.hpp
 *  Utility functions for use in the gui. Most are related to drawing.
 */
 
// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class RotatedDC;
class RealRect;

// ----------------------------------------------------------------------------- : Window related

/// Id of the control that has the focus in the given window, or -1 if no control has the focus
int focused_control(const Window* window);

/// (Try to) set the status text of a parent of window
void set_status_text(Window* window, const String& text);

/// Set the help text for a window, it will be shown in the status bar on mouse over
void set_help_text(Window* window, const String& text);

// ----------------------------------------------------------------------------- : DC related

/// Fill a DC with a single color
void clearDC(DC& dc, const wxBrush& brush);

/// Fill a newly allocated DC with black, if the platform doesn't do so automaticly
void clearDC_black(DC& dc);

/// Draw a checkerboard pattern
void draw_checker(RotatedDC& dc, const RealRect&);

// ----------------------------------------------------------------------------- : Image related

/// Proxy around Image.LoadFile that suppresses errors. 
bool image_load_file(Image& image, wxInputStream &stream);

/// Proxy around Image.LoadFile that suppresses errors. 
bool image_load_file(Image& image, const wxString &name);

// ----------------------------------------------------------------------------- : Tool/menu bar

/// Generate an image to use for the disabled state of menu and toolbar items
Image generate_disabled_image(Image const& image);

void set_menu_item_image(wxMenuItem* item, const String& resource);

// menu items with images.
// resource may be nullptr
wxMenuItem* make_menu_item(wxMenu*, int id, const char* resource, const String& text, const String& help, wxItemKind kind = wxITEM_NORMAL, wxMenu* submenu = nullptr);
wxMenuItem* make_menu_item_tr(wxMenu*, int id, const char* resource, const String& locale_key, wxItemKind kind = wxITEM_NORMAL, wxMenu* submenu = nullptr);
wxMenuItem* add_menu_item(wxMenu*, int id, const char* resource, const String& text, const String& help, wxItemKind kind = wxITEM_NORMAL, wxMenu* submenu = nullptr);
wxMenuItem* add_menu_item_tr(wxMenu*, int id, const char* resource, const String& locale_key, wxItemKind kind = wxITEM_NORMAL, wxMenu* submenu = nullptr);

// toolbars with images loaded from resource
// resource may be nullptr
wxToolBarToolBase* add_tool(wxToolBar*, int id, const char* resource, const String& label, const String& tooltip, const String& help, wxItemKind kind = wxITEM_NORMAL);
// add a tool to a toolbar, translate tooltip and help using locale
wxToolBarToolBase* add_tool_tr(wxToolBar*, int id, const char* resource, const String& locale_key, bool label = false, wxItemKind kind = wxITEM_NORMAL);

// ----------------------------------------------------------------------------- : Resource related

/// Load an image from a resource
Image load_resource_image(const String& name);

/// Load a cursor from a resource
wxCursor load_resource_cursor(const String& name);

/// Load an icon from a resource
wxIcon load_resource_icon(const String& name);

/// Load an image for use in a toolbar (filename: tool/...) from a resource
wxImage load_resource_tool_image(const String& name);

// ----------------------------------------------------------------------------- : Platform look

/// Draws a box for a control *around* a rect
/** Based on wxRendererXP::DrawComboBoxDropButton */
void draw_control_box(Window* win, DC& dc, const wxRect& rect, bool focused, bool enabled = true);

/// Draw a (possibly themed) button
void draw_button(Window* win, DC& dc, const wxRect& rect, bool focused, bool down, bool enabled);

/// Draws an arrow for a menu item indicating it has a sub menu
void draw_menu_arrow(Window* win, DC& dc, const wxRect& rect, bool active);

/// Draws a drop down arrow corresponding to that used by a combo box
void draw_drop_down_arrow(Window* win, DC& dc, const wxRect& rect, bool active);

/// Draws a check box
void draw_checkbox(Window* win, DC& dc, const wxRect& rect, bool checked, bool enabled = true);

/// Draws a radio button
void draw_radiobox(Window* win, DC& dc, const wxRect& rect, bool checked, bool enabled = true);

/// Draws a (fancy) selection rectangle
void draw_selection_rectangle(Window* win, DC& dc, const wxRect& rect, bool selected = true, bool focused = true, bool hot = false);

/// Enable fancy themed selection rectangle on windows
void enable_themed_selection_rectangle(Window* win);

