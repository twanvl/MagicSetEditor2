//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/util.hpp>
#include <util/error.hpp>
#include <util/rotation.hpp>
#include <wx/renderer.h>
#include <wx/stdpaths.h>
#include <gfx/gfx.hpp>

#if wxUSE_UXTHEME && defined(__WXMSW__)
  #include <wx/msw/uxtheme.h>
  #if defined(HAVE_VSSYM32)
    #include <vssym32.h>
  #else
    #include <tmschema.h>
  #endif
  #include <shlobj.h>
  #include <wx/mstream.h>
#endif

// ----------------------------------------------------------------------------- : Window related

// Id of the control that has the focus, or -1 if no control has the focus
int focused_control(const Window* window) {
  Window* focused_window = wxWindow::FindFocus();
  // is this window actually inside this panel?
  if (focused_window && wxWindow::FindWindowById(focused_window->GetId(), window) == focused_window) {
    return focused_window->GetId();
  } else {
    return -1; // no window has the focus, or it has a different parent/ancestor
  }
}

void set_status_text(Window* wnd, const String& s) {
  while (wnd) {
    wxFrame* f = dynamic_cast<wxFrame*>(wnd);
    if (f) {
      f->SetStatusText(s);
      return;
    }
    wnd = wnd->GetParent();
  }
}


// ----------------------------------------------------------------------------- : set_help_text
// The idea is as follows:
//  - store the help text of a window in its ClientObject as a StoredStatusString
//  - Connect event handlers that use set_status_text
//  - The event handlers should be members of an EvtHandler somewhere,
//     but it is wasteful to make an object. Instead use nullptr as a 'fake' EvtHandler.
//     then the event handling functions will be called with this==nullptr

struct StoredStatusString : public wxClientData {
  String s;
};

// Don't use this!
struct FakeEvtHandlerClass : public wxEvtHandler {
  void onControlEnter(wxMouseEvent& ev) {
    wxWindow* wnd = (wxWindow*)ev.GetEventObject();
    if (wnd) {
      StoredStatusString* d = static_cast<StoredStatusString*>(wnd->GetClientObject());
      set_status_text(wnd, d->s);
    }
    ev.Skip();
  }
  void onControlLeave(wxMouseEvent& ev) {
    set_status_text((wxWindow*)ev.GetEventObject(), wxEmptyString);
    ev.Skip();
  }
};

void set_help_text(Window* wnd, const String& s) {
  StoredStatusString* d = static_cast<StoredStatusString*>(wnd->GetClientObject());
  if (d) {
    // already called before
  } else {
    // first time
    d = new StoredStatusString;
    wnd->SetClientObject(d);
    wnd->Connect(wxEVT_ENTER_WINDOW,wxMouseEventHandler(FakeEvtHandlerClass::onControlEnter),nullptr,nullptr);
    wnd->Connect(wxEVT_LEAVE_WINDOW,wxMouseEventHandler(FakeEvtHandlerClass::onControlLeave),nullptr,nullptr);
  }
  d->s = s;
}


// ----------------------------------------------------------------------------- : DC related

/// Fill a DC with a single color
void clearDC(DC& dc, const wxBrush& brush) {
  wxSize size = dc.GetSize();
  wxPoint pos = dc.GetDeviceOrigin(); // don't you love undocumented methods?
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(brush);
  dc.DrawRectangle(-pos.x, -pos.y, size.GetWidth(), size.GetHeight());
}

void clearDC_black(DC& dc) {
  #if !BITMAPS_DEFAULT_BLACK
    // On windows 9x it seems that bitmaps are not black by default
    clearDC(dc, *wxBLACK_BRUSH);
  #endif
}

void draw_checker(RotatedDC& dc, const RealRect& rect) {
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.DrawRectangle(rect);
  dc.SetBrush(Color(235,235,235));
  const double checker_size = 10;
  int odd = 0;
  for (double y = 0 ; y < rect.height ; y += checker_size) {
    for (double x = odd * checker_size ; x < rect.width ; x += checker_size * 2) {
      dc.DrawRectangle(RealRect(
                rect.x + x,
                rect.y + y,
                min(checker_size, rect.width  - x),
                min(checker_size, rect.height - y)
              ));
    }
    odd = 1 - odd;
  }
}


// ----------------------------------------------------------------------------- : Seekable stream

/// A wxInputStream in which we can seek, even if the underlying stream can't
/** Seeking is used by the wxImage format detection code, to peek at the header.
 *  So we only need to be able to seek back to the beginning.
 *  The solution is to keep a small buffer of the file header.
 */
class SeekableInputStream : public wxInputStream {
public:
  SeekableInputStream(wxInputStream& stream)
    : stream(stream) {
  }
  bool IsOk() const override {
    return stream.IsOk();
  }
  bool IsSeekable() const override {
    return pos < (wxFileOffset)BUFFER_SIZE;
  }
  size_t LastRead() const override {
    return last_read;
  }
  char Peek() override {
    if (pos < stream_pos) {
      return buffer[pos];
    } else {
      return stream.Peek();
    }
  }
  wxInputStream& Read(void* out_buffer, size_t size) override {
    last_read = 0;
    if (pos < stream_pos) {
      // take from buffer
      size_t n = min(size, min((size_t)stream_pos, BUFFER_SIZE) - (size_t)pos);
      memcpy(out_buffer, buffer + pos, n);
      size -= n;
      out_buffer = (char*)out_buffer + n;
      last_read += n;
      pos += n;
    }
    if (size > 0) {
      assert(pos == stream_pos);
      // take from stream
      stream.Read(out_buffer, size);
      size_t read = stream.LastRead();
      last_read += read;
      if (pos < (wxFileOffset)BUFFER_SIZE) {
        // update buffer
        memcpy(buffer + pos, out_buffer, min(BUFFER_SIZE - (size_t)pos, read));
      }
      stream_pos += read;
      pos += read;
    }
    return *this;
  }
  wxFileOffset SeekI(wxFileOffset target, wxSeekMode mode = wxFromStart) override {
    assert(mode == wxFromStart);
    assert(target < (wxFileOffset)BUFFER_SIZE);
    if (target < (wxFileOffset)BUFFER_SIZE) {
      pos = target;
      return target;
    } else {
      return wxInvalidOffset;
    }
  }
  wxFileOffset TellI() const override {
    return pos;
  }
protected:
  size_t OnSysRead(void* out_buffer, size_t size) override {
    Read(out_buffer, size);
    return last_read;
  }
private:
  wxInputStream& stream;
  static constexpr size_t BUFFER_SIZE = 16;
  Byte buffer[BUFFER_SIZE];
  // positions:
  // Invariant: pos < stream_pos ==> pos < BUFFER_SIZE && buffer[i] is valid for i<=pos
  wxFileOffset stream_pos = 0; // position in underlying stream
  wxFileOffset pos = 0; // position taking into account any seeking done.
  size_t last_read = 0;
};

// ----------------------------------------------------------------------------- : Image related

bool image_load_file(Image& image, wxInputStream &stream) {
  wxLogNull noLog; // prevent wx from showing popups about sRGB profiles and other notices
  if (!stream.IsSeekable()) {
    SeekableInputStream seek_stream(stream);
    return image.LoadFile(seek_stream);
  } else {
    return image.LoadFile(stream);
  }
}

bool image_load_file(Image& image, const wxString &name) {
  wxLogNull noLog;
  return image.LoadFile(name);
}

// ----------------------------------------------------------------------------- : Tool/menu bar

Image generate_disabled_image(Image const& image) {
  Image imgOut = image.ConvertToGreyscale();
  set_alpha(imgOut, 0.33);
  return imgOut;
}

void set_menu_item_image(wxMenuItem* item, const String& resource) {
  #if defined(__WXGTK__)
    if (item->GetKind() == wxITEM_CHECK) return; // check items can't have icons
  #endif
  Image bitmap = load_resource_tool_image(resource);
  #if defined(__WXMSW__)
    Image disabled_bitmap = generate_disabled_image(bitmap);
    item->SetBitmaps(bitmap, bitmap);
    item->SetDisabledBitmap(disabled_bitmap);
  #else
    item->SetBitmap(bitmap);
  #endif
}

wxMenuItem* make_menu_item(wxMenu* menu, int id, const char* resource, const String& text, const String& help, wxItemKind kind, wxMenu* submenu) {
  wxMenuItem* item = new wxMenuItem(menu, id, text, help, kind, submenu);
  if (resource) set_menu_item_image(item, resource);
  return item;
}
wxMenuItem* make_menu_item_tr(wxMenu* menu, int id, const char* resource, const String& locale_key, wxItemKind kind, wxMenu* submenu) {
  wxMenuItem* item = new wxMenuItem(menu, id, tr(LOCALE_CAT_MENU, locale_key), tr(LOCALE_CAT_HELP, locale_key), kind, submenu);
  if (resource) set_menu_item_image(item, resource);
  return item;
}
wxMenuItem* add_menu_item(wxMenu* menu, int id, const char* resource, const String& text, const String& help, wxItemKind kind, wxMenu* submenu) {
  wxMenuItem* item = make_menu_item(menu, id, resource, text, help, kind, submenu);
  menu->Append(item);
  return item;
}
wxMenuItem* add_menu_item_tr(wxMenu* menu, int id, const char* resource, const String& locale_key, wxItemKind kind, wxMenu* submenu) {
  wxMenuItem* item = make_menu_item_tr(menu, id, resource, locale_key, kind, submenu);
  menu->Append(item);
  return item;
}

wxToolBarToolBase* add_tool(wxToolBar* toolbar, int id, const char* resource, const String& label, const String& tooltip, const String& help, wxItemKind kind) {
  if (resource) {
    // Note: the bitmap must match the toolbar bitmap size, otherwise disabled and normal bitmap look different
    // if the size doesn't match, we center the image
    auto size = toolbar->GetToolBitmapSize();
    Image bitmap = load_resource_tool_image(resource);
    if (bitmap.GetSize() != size) {
      wxPoint pos((size.GetWidth() - bitmap.GetWidth()) / 2, (size.GetHeight() - bitmap.GetHeight()) / 2);
      bitmap.Resize(size, pos);
    }
    #if defined(__WXGTK__)
      wxBitmap disabled_bitmap = wxNullBitmap; // gtk can produce decent disabled bitmaps by itself
    #else
      Image disabled_bitmap = generate_disabled_image(bitmap);
    #endif
    return toolbar->AddTool(id, label, bitmap, disabled_bitmap, kind, tooltip, help);
  } else {
    return toolbar->AddTool(id, label, wxNullBitmap, wxNullBitmap, kind, tooltip,  help);
  }
}

wxToolBarToolBase* add_tool_tr(wxToolBar* toolbar, int id, const char* resource, const String& locale_key, bool label, wxItemKind kind) {
  return add_tool(toolbar, id, resource, label ? tr(LOCALE_CAT_TOOL,locale_key) : String(), tr(LOCALE_CAT_TOOLTIP, locale_key), tr(LOCALE_CAT_HELP, locale_key), kind);
}

// ----------------------------------------------------------------------------- : Resource related

Image load_resource_image(const String& name) {
  #if defined(__WXMSW__) && !defined(__GNUC__)
    // Load resource
    // based on wxLoadUserResource
    // The image can be in an IMAGE resource, in any file format
    HRSRC hResource = ::FindResource(wxGetInstance(), name.wc_str(), _("IMAGE"));
    if ( hResource == 0 ) throw InternalError(String::Format(_("Resource not found: %s"), name));
    
    HGLOBAL hData = ::LoadResource(wxGetInstance(), hResource);
    if ( hData == 0 ) throw InternalError(String::Format(_("Resource not an image: %s"), name));
    
    char* data = (char *)::LockResource(hData);
    if ( !data ) throw InternalError(String::Format(_("Resource cannot be locked: %s"), name));
    
    int len = ::SizeofResource(wxGetInstance(), hResource);
    wxMemoryInputStream stream(data, len);

    wxLogNull noLog;
    return wxImage(stream);
  #elif defined(__GNUC__)
    static String path = wxStandardPaths::Get().GetDataDir() + _("/resource/");
    String file = path + name;
    wxImage resource;
    if (wxFileExists(file + _(".png"))) image_load_file(resource, file + _(".png"));
    else if (wxFileExists(file + _(".bmp"))) image_load_file(resource, file + _(".bmp"));
    else if (wxFileExists(file + _(".ico"))) image_load_file(resource, file + _(".ico"));
    else if (wxFileExists(file + _(".cur"))) image_load_file(resource, file + _(".cur"));
    if (resource.Ok()) return resource;
        static String local_path = wxStandardPaths::Get().GetUserDataDir() + _("/resource/");
        file = local_path + name;
        if (wxFileExists(file + _(".png"))) image_load_file(resource, file + _(".png"));
        else if (wxFileExists(file + _(".bmp"))) image_load_file(resource, file + _(".bmp"));
        else if (wxFileExists(file + _(".ico"))) image_load_file(resource, file + _(".ico"));
        else if (wxFileExists(file + _(".cur"))) image_load_file(resource, file + _(".cur"));
        if (!resource.Ok()) handle_error(InternalError(String(_("Cannot find resource file at ")) + path + name + _(" or ") + file));
    return resource;
  #else
    #error Handling of resource loading needs to be declared.
  #endif
}

wxCursor load_resource_cursor(const String& name) {
  #if defined(__WXMSW__) && !defined(__GNUC__)
    wxLogNull noLog;
    return wxCursor(_("cursor/") + name, wxBITMAP_TYPE_CUR_RESOURCE);
  #else
    return wxCursor(load_resource_image(_("cursor/") + name));
  #endif
}

wxIcon load_resource_icon(const String& name) {
  wxLogNull noLog;
  #if defined(__WXMSW__) && !defined(__GNUC__)
    return wxIcon(_("icon/") + name);
  #else
    static String path = wxStandardPaths::Get().GetDataDir() + _("/resource/icon/");
        static String local_path = wxStandardPaths::Get().GetUserDataDir() + _("/resource/icon/");
        if (wxFileExists(path + name + _(".ico"))) return wxIcon(path + name + _(".ico"), wxBITMAP_TYPE_ICO);
        else return wxIcon(local_path + name + _(".ico"), wxBITMAP_TYPE_ICO);
  #endif
}

wxImage load_resource_tool_image(const String& name) {
  return load_resource_image(_("tool/") + name);
}


#if defined(_UNICODE) && defined(_MSC_VER) && _MSC_VER >= 1400
// manifest to use new-style controls in Windows Vista / Windows 7
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

// ----------------------------------------------------------------------------- : Platform look

#if wxUSE_UXTHEME && defined(__WXMSW__)
RECT msw_rect(wxRect const& rect, int dl = 0, int dt = 0, int dr = 0, int db=0) {
  RECT r;
  r.left = rect.x - dl;
  r.top = rect.y - dt;
  r.right = rect.x + rect.width + dr;
  r.bottom = rect.y + rect.height + db;
  return r;
}
#endif

// Draw a basic 3D border
void draw3DBorder(DC& dc, int x1, int y1, int x2, int y2) {
  dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));
  dc.DrawLine(x1, y1, x2, y1);
  dc.DrawLine(x1, y1, x1, y2);
  dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
  dc.DrawLine(x1-1, y1-1, x2+1, y1-1);
  dc.DrawLine(x1-1, y1-1, x1-1, y2+1);
  dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
  dc.DrawLine(x1, y2, x2, y2);
  dc.DrawLine(x2, y1, x2, y2+1);
  dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT));
  dc.DrawLine(x1-1, y2+1, x2+1, y2+1);
  dc.DrawLine(x2+1, y1-1, x2+1, y2+2);
}

void draw_control_box(Window* win, DC& dc, const wxRect& rect, bool focused, bool enabled) {
  #if wxUSE_UXTHEME && defined(__WXMSW__)
    RECT r = msw_rect(rect, 1,1,1,1);
    if (wxUxThemeIsActive()) {
      HTHEME hTheme = (HTHEME)::OpenThemeData(GetHwndOf(win), VSCLASS_EDIT);
      if (hTheme) {
        ::DrawThemeBackground(
          hTheme,
          (HDC)dc.GetHDC(),
          EP_EDITTEXT,
          !enabled ? ETS_DISABLED : focused ? ETS_NORMAL : ETS_NORMAL,
          &r,
          NULL
        );
        return;
      }
    }
  #endif
  // otherwise, draw a standard border
  // clear the background
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
  dc.DrawRectangle(rect);
  // draw the border
  #if defined(__WXMSW__)
    r.left   = rect.x - 2;
    r.top    = rect.y - 2;
    r.right  = rect.x + rect.width  + 2;
    r.bottom = rect.y + rect.height + 2;
    DrawEdge((HDC)dc.GetHDC(), &r, EDGE_SUNKEN, BF_RECT);
  #else
    // draw a 3D border
    draw3DBorder(dc, rect.x - 1, rect.y - 1, rect.x + rect.width, rect.y + rect.height);
  #endif
}

void draw_button(Window* win, DC& dc, const wxRect& rect, bool focused, bool down, bool enabled) {
  #if wxVERSION_NUMBER >= 2700
    wxRendererNative& rn = wxRendererNative::GetDefault();
    rn.DrawPushButton(win, dc, rect, (focused ? wxCONTROL_FOCUSED : 0) | (down ? wxCONTROL_PRESSED : 0) | (enabled ? 0 : wxCONTROL_DISABLED));
  #else
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
    dc.SetPen(wxSystemSettings::GetColour(down ? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_BTNHIGHLIGHT));
    dc.DrawLine(rect.x,rect.y,rect.x+rect.width,rect.y);
    dc.DrawLine(rect.x,rect.y,rect.x,rect.y+rect.height);
    dc.SetPen(wxSystemSettings::GetColour(down ? wxSYS_COLOUR_BTNHIGHLIGHT : wxSYS_COLOUR_BTNSHADOW));
    dc.DrawLine(rect.x+rect.width-1,rect.y,rect.x+rect.width-1,rect.y+rect.height);
    dc.DrawLine(rect.x,rect.y+rect.height-1,rect.x+rect.width,rect.y+rect.height-1);
  #endif
}

// portable, based on wxRendererGeneric::DrawComboBoxDropButton
void draw_menu_arrow(Window* win, DC& dc, const wxRect& rect, bool active) {
  wxPoint pt[] =
    {  wxPoint(0, 0)
    ,  wxPoint(4, 4)
    ,  wxPoint(0, 8)
    };
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(wxSystemSettings::GetColour(active ? wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));
  dc.DrawPolygon(3, pt, rect.x + rect.width - 6, rect.y + (rect.height - 9) / 2);
}

void draw_drop_down_arrow(Window* win, DC& dc, const wxRect& rect, bool active) {
  wxRendererNative& rn = wxRendererNative::GetDefault();
  int w = wxSystemSettings::GetMetric(wxSYS_VSCROLL_ARROW_X); // drop down arrow is same size
  if (w == -1) {
    w = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X); // Try just the scrollbar, then.
  }
  rn.DrawComboBoxDropButton(win, dc,
    wxRect(rect.x + rect.width - w, rect.y, w, rect.height)
    , active ? wxCONTROL_PRESSED : 0);
}

void draw_checkbox(const Window* win, DC& dc, const wxRect& rect, bool checked, bool enabled) {
  #if defined(__WXMSW__)
    if (win == nullptr) win = dc.GetWindow();
  #endif
  if (win) {
    wxRendererNative& rn = wxRendererNative::GetDefault();
    rn.DrawCheckBox(const_cast<wxWindow*>(win), dc, rect, (checked ? wxCONTROL_CHECKED : 0) | (enabled ? 0 : wxCONTROL_DISABLED));
    return;
  } else {
    // portable version
    if (checked) {
      dc.DrawCheckMark(wxRect(rect.x+1,rect.y+1,rect.width-2,rect.height-2));
    }
    dc.SetPen(wxSystemSettings::GetColour(enabled ? wxSYS_COLOUR_WINDOWTEXT: wxSYS_COLOUR_GRAYTEXT));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
  }
}

void draw_radiobox(const Window* win, DC& dc, const wxRect& rect, bool checked, bool enabled) {
  #if defined(__WXMSW__)
    if (win == nullptr) win = dc.GetWindow();
  #endif
  if (win) {
    wxRendererNative& rn = wxRendererNative::GetDefault();
    rn.DrawRadioBitmap(const_cast<wxWindow*>(win), dc, rect, (checked ? wxCONTROL_CHECKED : 0) | (enabled ? 0 : wxCONTROL_DISABLED));
    return;
  } else {
    // circle drawing on windows looks absolutely horrible
    // so use rounded rectangles instead
    dc.SetPen(wxSystemSettings::GetColour(enabled ? wxSYS_COLOUR_WINDOWTEXT: wxSYS_COLOUR_GRAYTEXT));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    //dc.DrawEllipse(rect.x, rect.y, rect.width, rect.height);
    dc.DrawRoundedRectangle(rect.x, rect.y, rect.width, rect.height, rect.width*0.5-1);
    if (checked) {
      dc.SetBrush(wxSystemSettings::GetColour(enabled ? wxSYS_COLOUR_WINDOWTEXT: wxSYS_COLOUR_GRAYTEXT));
      dc.SetPen(*wxTRANSPARENT_PEN);
      //dc.DrawEllipse(rect.x+2,rect.y+2,rect.width-4,rect.height-4);
      dc.DrawRoundedRectangle(rect.x+3, rect.y+3, rect.width-6, rect.height-6, rect.width*0.5-4);
    }
  }
}

void draw_selection_rectangle(Window* win, DC& dc, const wxRect& rect, bool selected, bool focused, bool hot) {
  #if wxUSE_UXTHEME && defined(__WXMSW__)
    HTHEME hTheme = (HTHEME)::OpenThemeData(GetHwndOf(win), VSCLASS_LISTVIEW);
    if (hTheme) {
      RECT r = msw_rect(rect);
      ::DrawThemeBackground(
        hTheme,
        (HDC)dc.GetHDC(),
        LVP_LISTITEM,
        hot&&selected ? LISS_HOTSELECTED : hot ? LISS_HOT :selected&&focused ? LISS_SELECTED : selected ? LISS_SELECTEDNOTFOCUS : LISS_NORMAL,
        &r,
        NULL
      );
      return;
    }
  #endif
}

void enable_themed_selection_rectangle(Window* win) {
  #if wxUSE_UXTHEME && defined(__WXMSW__)
    if (wxUxThemeIsActive()) {
      ::SetWindowTheme((HWND)win->GetHWND(), L"Explorer", NULL);
    }
  #endif
}
