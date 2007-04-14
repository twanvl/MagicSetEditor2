//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/image_slice_window.hpp>
#include <gui/util.hpp>
#include <util/window_id.hpp>
#include <util/rotation.hpp>
#include <gfx/gfx.hpp>
#include <wx/spinctrl.h>
#include <wx/dcbuffer.h>

// ----------------------------------------------------------------------------- : ImageSlice

ImageSlice::ImageSlice(const Image& source, const wxSize& target_size)
	: source(source), target_size(target_size)
	, selection(0, 0, source.GetWidth(), source.GetHeight())
	, allow_outside(false), aspect_fixed(true)
	, sharpen(true), sharpen_amount(25)
{}

void ImageSlice::constrain() {
	sharpen_amount = min(100, max(0, sharpen_amount));
	// minimum size
	selection.width  = max(1, selection.width);
	selection.height = max(1, selection.height);
	// inside source
	if (!allow_outside) {
		selection.width  = min(selection.width,  source.GetWidth());
		selection.height = min(selection.height, source.GetHeight());
		selection.x      = max(selection.x, 0);
		selection.y      = max(selection.y, 0);
		selection.x      = min(selection.x, source.GetWidth()  - selection.width);
		selection.y      = min(selection.y, source.GetHeight() - selection.height);
	}
	// fix aspect ratio
	if (aspect_fixed) {
		int diff = selection.width * target_size.GetHeight() - selection.height * target_size.GetWidth();
		if (diff > 0) {
			// too wide
			selection.width  -= int(diff / target_size.GetHeight());
		} else {
			// too high
			selection.height -= int(-diff / target_size.GetWidth());
		}
	}
}

Image ImageSlice::getSlice() const {
	if (selection.width == target_size.GetWidth() && selection.height == target_size.GetHeight() && selection.x == 0 && selection.y == 0) {
		// exactly the right size
		return source;
	}
	Image target(target_size.GetWidth(), target_size.GetHeight(), false);
	if (sharpen && sharpen_amount > 0 && sharpen_amount <= 100) {
		sharp_resample_and_clip(source, target, selection, sharpen_amount);
	} else {
		resample_and_clip(source, target, selection);
	}
	return target;
}

// ----------------------------------------------------------------------------- : Events

DECLARE_EVENT_TYPE(EVENT_SLICE_CHANGED, <not used>);
DEFINE_EVENT_TYPE(EVENT_SLICE_CHANGED);

/// Handle EVENT_SLICE_CHANGED events
#define EVT_SLICE_CHANGED(id, handler) EVT_COMMAND(id, EVENT_SLICE_CHANGED, handler)

// ----------------------------------------------------------------------------- : ImageSliceWindow

ImageSliceWindow::ImageSliceWindow(Window* parent, const Image& source, const wxSize& target_size, const AlphaMaskP& mask)
	: wxDialog(parent,wxID_ANY,_TITLE_("slice image"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxFULL_REPAINT_ON_RESIZE)
	, slice(source, target_size)
{
	// init controls
	const wxPoint defPos = wxDefaultPosition;
	const wxSize spinSize(80,-1);
	selector  = new ImageSliceSelector(this, ID_SELECTOR, slice);
	preview   = new ImageSlicePreview (this, ID_PREVIEW,  slice, mask);
	
	String sizes[] = { _("&Original Size")
	                 , _("Size to &Fit")
	                 , _("F&orce to Fit")
	                 , _("&Custom Size") };
	size      = new wxRadioBox(this, ID_SIZE, _LABEL_("size"), defPos, wxDefaultSize, 4, sizes, 1);
	
	left      = new wxSpinCtrl(this, ID_LEFT,   _(""), defPos, spinSize);
	top       = new wxSpinCtrl(this, ID_TOP,    _(""), defPos, spinSize);
	width     = new wxSpinCtrl(this, ID_WIDTH,  _(""), defPos, spinSize);
	height    = new wxSpinCtrl(this, ID_HEIGHT, _(""), defPos, spinSize);
	top   ->SetRange(-5000,5000);
	left  ->SetRange(-5000,5000);
	width ->SetRange(0,5000);
	height->SetRange(0,5000);
	
	fix_aspect = new wxCheckBox(this, ID_FIX_ASPECT, _("Fix aspect ratio (width/height)"));
	zoom_x     = new wxSpinCtrl(this, ID_ZOOM_X,     _(""), defPos, spinSize);
	zoom_y     = new wxSpinCtrl(this, ID_ZOOM_Y,     _(""), defPos, spinSize);
	zoom       = new wxSpinCtrl(this, ID_ZOOM,       _(""), defPos, spinSize);
	zoom_x->SetRange(1,10000);
	zoom_y->SetRange(1,10000);
	zoom  ->SetRange(1,10000);
	
	sharpen        = new wxCheckBox(this, ID_SHARPEN, _("&Sharpen Filter"));
	sharpen_amount = new wxSlider(this, ID_SHARPEN_AMOUNT, 0, 0, 100);
//	allowOutside= new CheckBox(&this, idSliceAllowOutside, _("Allow selection outside source"))
//	bgColor       = new ColorSelector(&this, wxID_ANY)
	
	// init sizers
	wxSizer* s = new wxBoxSizer(wxVERTICAL);
		// top row: image editors
		wxSizer* s2 = new wxBoxSizer(wxHORIZONTAL);
			wxSizer* s3 = new wxBoxSizer(wxVERTICAL);
				s3->Add(new wxStaticText(this, wxID_ANY, _LABEL_("original")));
				s3->Add(selector, 1, wxEXPAND | wxTOP, 4);
			s2->Add(s3, 1, wxEXPAND | wxALL, 4);
			wxSizer* s4 = new wxBoxSizer(wxVERTICAL);
				s4->Add(new wxStaticText(this, wxID_ANY, _LABEL_("result")));
				s4->Add(preview, 0, wxTOP, 4);
			s2->Add(s4, 0, wxALL, 4);
		s->Add(s2, 1, wxEXPAND);
		// bottom row: controls
		wxSizer* s5 = new wxBoxSizer(wxHORIZONTAL);
			s5->AddStretchSpacer(1);
			s5->Add(size, 0, wxEXPAND | wxALL, 4);
			s5->AddStretchSpacer(1);
			wxSizer* s6 = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("selection"));
				wxSizer* s7 = new wxFlexGridSizer(0, 2, 4, 5);
					s7->Add(new wxStaticText(this, wxID_ANY, _("&Left")),   0, wxALIGN_CENTER_VERTICAL);
					s7->Add(left,   0, wxEXPAND);
					s7->Add(new wxStaticText(this, wxID_ANY, _("&Top")),    0, wxALIGN_CENTER_VERTICAL);
					s7->Add(top,    0, wxEXPAND);
					s7->Add(new wxStaticText(this, wxID_ANY, _("&Width")),  0, wxALIGN_CENTER_VERTICAL);
					s7->Add(width,  0, wxEXPAND);
					s7->Add(new wxStaticText(this, wxID_ANY, _("&Height")), 0, wxALIGN_CENTER_VERTICAL);
					s7->Add(height, 0, wxEXPAND);
				s6->Add(s7, 1, wxEXPAND | wxALL, 4);
			s5->Add(s6, 0, wxEXPAND | wxALL, 4);
			s5->AddStretchSpacer(1);
			wxSizer* s8 = zoom_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("zoom"));
				s8->Add(fix_aspect, 0, wxEXPAND | wxALL & ~wxBOTTOM, 4);
				wxSizer* s9 = zoom_fixed = new wxFlexGridSizer(0, 3, 4, 5);
					s9->Add(new wxStaticText(this, wxID_ANY, _("&Zoom")),   0, wxALIGN_CENTER_VERTICAL);
					s9->Add(zoom, 0, wxEXPAND);
					s9->Add(new wxStaticText(this, wxID_ANY, _("%")),       0, wxALIGN_CENTER_VERTICAL);
				s8->Add(s9, 0, wxEXPAND | wxALL, 4);
				wxSizer* sA = zoom_free = new wxFlexGridSizer(0, 3, 4, 5);
					sA->Add(new wxStaticText(this, wxID_ANY, _("Zoom &X")), 0, wxALIGN_CENTER_VERTICAL);
					sA->Add(zoom_x, 0, wxEXPAND);
					sA->Add(new wxStaticText(this, wxID_ANY, _("%")),       0, wxALIGN_CENTER_VERTICAL);
					sA->Add(new wxStaticText(this, wxID_ANY, _("Zoom &Y")), 0, wxALIGN_CENTER_VERTICAL);
					sA->Add(zoom_y, 0, wxEXPAND);
					sA->Add(new wxStaticText(this, wxID_ANY, _("%")),       0, wxALIGN_CENTER_VERTICAL);
				s8->Add(sA, 0, wxEXPAND | wxALL, 4);
			s5->Add(s8, 0, wxEXPAND | wxALL, 4);
			s5->AddStretchSpacer(1);
			wxSizer* sB = new wxStaticBoxSizer(wxVERTICAL, this, _LABEL_("filter"));
				sB->Add(sharpen,        0, wxEXPAND | wxALL & ~wxBOTTOM, 4);
				sB->Add(sharpen_amount, 0, wxEXPAND | wxALL, 4);
			s5->Add(sB, 0, wxEXPAND | wxALL, 4);
			s5->AddStretchSpacer(1);
		s->Add(s5, 0, wxEXPAND);
		s->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
	s->SetSizeHints(this);
	SetSizer(s);
	updateControls();
}

void ImageSliceWindow::onOk(wxCommandEvent&) {
	EndModal(wxID_OK);
}

Image ImageSliceWindow::getImage() const {
	return slice.getSlice();
}

// ----------------------------------------------------------------------------- : ImageSliceWindow : Controls

void ImageSliceWindow::onChangeSize(wxCommandEvent&) {
	int sel = size->GetSelection();
	if (sel == 0) {
		// original size
		slice.selection.width  = slice.target_size.GetWidth();
		slice.selection.height = slice.target_size.GetHeight();
		slice.aspect_fixed = true;
		onUpdateFromControl();
	} else if (sel == 1) {
		// size to fit
		slice.selection.x = slice.selection.y = 0;
		slice.selection.width  = slice.source.GetWidth();
		slice.selection.height = slice.source.GetHeight();
		slice.aspect_fixed = true;
		onUpdateFromControl();
	} else if (sel == 2) {
		// force to fit
		slice.selection.x = slice.selection.y = 0;
		slice.selection.width  = slice.source.GetWidth();
		slice.selection.height = slice.source.GetHeight();
		slice.aspect_fixed = false;
		onUpdateFromControl();
	}
}

void ImageSliceWindow::onChangeLeft(wxCommandEvent&) {
	slice.selection.x = left->GetValue();
	onUpdateFromControl();
}
void ImageSliceWindow::onChangeTop(wxCommandEvent&) {
	slice.selection.y = top->GetValue();
	onUpdateFromControl();
}
void ImageSliceWindow::onChangeWidth(wxCommandEvent&) {
	slice.selection.width = width->GetValue();
	onUpdateFromControl();
}
void ImageSliceWindow::onChangeHeight(wxCommandEvent&) {
	slice.selection.height = height->GetValue();
	onUpdateFromControl();
}

void ImageSliceWindow::onChangeFixAspect(wxCommandEvent&) {
	slice.aspect_fixed = fix_aspect->GetValue();
	onUpdateFromControl();
}

void ImageSliceWindow::onChangeZoom(wxSpinEvent&) {
	slice.zoomX(zoom->GetValue() / 100.0);
	slice.zoomY(zoom->GetValue() / 100.0);
	onUpdateFromControl();
}
void ImageSliceWindow::onChangeZoomX(wxSpinEvent&) {
	slice.zoomX(zoom_x->GetValue() / 100.0);
	onUpdateFromControl();
}
void ImageSliceWindow::onChangeZoomY(wxSpinEvent&) {
	slice.zoomY(zoom_x->GetValue() / 100.0);
	onUpdateFromControl();
}

void ImageSliceWindow::onChangeSharpen(wxCommandEvent&) {
	slice.sharpen = sharpen->GetValue();
	onUpdateFromControl();
}
void ImageSliceWindow::onChangeSharpenAmount(wxScrollEvent&) {
	slice.sharpen_amount = sharpen_amount->GetValue();
	onUpdateFromControl();
}

// ----------------------------------------------------------------------------- : ImageSliceWindow : Updates

void ImageSliceWindow::onSliceChange(wxCommandEvent&) {
	slice.constrain();
	preview->update();
	selector->update();
	updateControls();
}

void ImageSliceWindow::onUpdateFromControl() {
	slice.constrain();
	preview->update();
	selector->update();
	updateControls();
}

void ImageSliceWindow::updateControls() {
	if (slice.selection.width == slice.target_size.GetWidth() && slice.selection.height == slice.target_size.GetHeight()) {
		size->SetSelection(0); // original size
	} else if (slice.selection.x == 0 && slice.selection.width  == slice.source.GetWidth() &&
	           slice.selection.y == 0 && slice.selection.height == slice.source.GetHeight()) {
		size->SetSelection(2); // force to fit
	} else if (slice.selection.width  <= slice.source.GetWidth()  &&
	           slice.selection.height <= slice.source.GetHeight() &&
		        (  (slice.selection.x == 0 && slice.selection.width  == slice.source.GetWidth())
		         ||(slice.selection.y == 0 && slice.selection.height == slice.source.GetHeight()))) {
		size->SetSelection(1); // size to fit
	} else {
		size->SetSelection(3); // custom size
	}
	left      ->SetValue(slice.selection.x);
	top       ->SetValue(slice.selection.y);
	width     ->SetValue(slice.selection.width);
	height    ->SetValue(slice.selection.height);
	fix_aspect->SetValue(slice.aspect_fixed);
	if (slice.aspect_fixed) {
		zoom->SetValue(int(slice.zoomX() * 100));
		if (zoom_x->IsShown()) {
			zoom_sizer->Show(zoom_fixed, true);
			zoom_sizer->Show(zoom_free,  false);
			Layout();
		}
	} else {
		zoom_x->SetValue(int(slice.zoomX() * 100));
		zoom_y->SetValue(int(slice.zoomY() * 100));
		if (zoom->IsShown()) {
			zoom_sizer->Show(zoom_fixed, false);
			zoom_sizer->Show(zoom_free,  true);
			Layout();
		}
	}
	sharpen       ->SetValue(slice.sharpen);
	sharpen_amount->SetValue(slice.sharpen_amount);
	sharpen_amount->Enable(slice.sharpen);
}

// ----------------------------------------------------------------------------- : ImageSliceWindow : Event table

BEGIN_EVENT_TABLE(ImageSliceWindow, wxDialog)
	EVT_BUTTON			(wxID_OK,			ImageSliceWindow::onOk)
	EVT_RADIOBOX		(ID_SIZE,			ImageSliceWindow::onChangeSize)
	EVT_TEXT			(ID_LEFT,			ImageSliceWindow::onChangeLeft)
	EVT_TEXT			(ID_TOP,			ImageSliceWindow::onChangeTop)
	EVT_TEXT			(ID_WIDTH,			ImageSliceWindow::onChangeWidth)
	EVT_TEXT			(ID_HEIGHT,			ImageSliceWindow::onChangeHeight)
	EVT_CHECKBOX		(ID_FIX_ASPECT,		ImageSliceWindow::onChangeFixAspect)
	EVT_SPINCTRL		(ID_ZOOM,			ImageSliceWindow::onChangeZoom)
	EVT_SPINCTRL		(ID_ZOOM_X,			ImageSliceWindow::onChangeZoomX)
	EVT_SPINCTRL		(ID_ZOOM_Y,			ImageSliceWindow::onChangeZoomY)
	EVT_CHECKBOX		(ID_SHARPEN,		ImageSliceWindow::onChangeSharpen)
	EVT_COMMAND_SCROLL	(ID_SHARPEN_AMOUNT,	ImageSliceWindow::onChangeSharpenAmount)
	EVT_SLICE_CHANGED   (wxID_ANY,			ImageSliceWindow::onSliceChange)
//	EVT_SIZE			(					ImageSliceWindow::onSize)
END_EVENT_TABLE  ()





// ----------------------------------------------------------------------------- : ImageSlicePreview

ImageSlicePreview::ImageSlicePreview(Window* parent, int id, ImageSlice& slice, const AlphaMaskP& mask)
	: wxControl(parent, id)
	, slice(slice)
	, mask(mask)
	, mouse_down(false)
{}

void ImageSlicePreview::update() {
	bitmap = wxNullBitmap;
	Refresh(false);
}

wxSize ImageSlicePreview::DoGetBestSize() const {
	// We know the client size we want, calculate the size that goes with that
	wxSize ws = GetSize(), cs = GetClientSize();
	return slice.target_size + ws - cs;
}

void ImageSlicePreview::onPaint(wxPaintEvent&) {
	wxPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}
void ImageSlicePreview::draw(DC& dc) {
	if (!bitmap.Ok()) {
		Image image = slice.getSlice();
		if (mask && mask->size == slice.target_size) {
			mask->setAlpha(image);
			// create bitmap
			bitmap = Bitmap(image.GetWidth(), image.GetHeight());
			wxMemoryDC mdc; mdc.SelectObject(bitmap);
			// draw checker pattern behind image
			RealRect rect = (RealRect) GetClientSize();
			RotatedDC rdc(mdc, 0, rect, 1, QUALITY_LOW);
			draw_checker(rdc, rect);
			rdc.DrawImage(image, RealPoint(0,0));
			mdc.SelectObject(wxNullBitmap);
		} else {
			bitmap = Bitmap(image);
		}
	}
	if (bitmap.Ok()) {
		dc.DrawBitmap(bitmap, 0, 0);
	}
}

void ImageSlicePreview::onLeftDown(wxMouseEvent& ev) {
	mouseX = ev.GetX();
	mouseY = ev.GetY();
	start_selection = slice.selection;
	mouse_down = true;
	CaptureMouse();
	SetCursor(wxCURSOR_SIZING);
}
void ImageSlicePreview::onLeftUp(wxMouseEvent&v) {
	mouse_down = false;
	if (HasCapture()) ReleaseMouse();
	SetCursor(wxNullCursor);
}
void ImageSlicePreview::onMotion(wxMouseEvent& ev) {
	if (mouse_down) {
		// drag the image
		slice.selection.x = int(start_selection.x + (mouseX - ev.GetX()) / slice.zoomX());
		slice.selection.y = int(start_selection.y + (mouseY - ev.GetY()) / slice.zoomY());
		// Notify parent
		wxCommandEvent ev(EVENT_SLICE_CHANGED, GetId());
		ProcessEvent(ev);
	}
}

BEGIN_EVENT_TABLE(ImageSlicePreview, wxControl)
	EVT_PAINT        (ImageSlicePreview::onPaint)
	EVT_LEFT_DOWN    (ImageSlicePreview::onLeftDown)
	EVT_LEFT_UP      (ImageSlicePreview::onLeftUp)
	EVT_MOTION       (ImageSlicePreview::onMotion)
END_EVENT_TABLE  ()





// ----------------------------------------------------------------------------- : ImageSliceSelector

ImageSliceSelector::ImageSliceSelector(Window* parent, int id, ImageSlice& slice)
	: wxControl(parent, id)
	, slice(slice)
	, mouse_down(false)
{}

void ImageSliceSelector::update() {
	Refresh(false);
}

void ImageSliceSelector::onSize(wxSizeEvent&) {
	bitmap = wxNullBitmap;
	Refresh(false);
}

// ----------------------------------------------------------------------------- : ImageSliceSelector : Drawing

void ImageSliceSelector::onPaint(wxPaintEvent&) {
	wxBufferedPaintDC dc(this);
	dc.BeginDrawing();
	draw(dc);
	dc.EndDrawing();
}
void ImageSliceSelector::draw(DC& dc) {
	if (!bitmap.Ok()) createBitmap();
	if (!bitmap.Ok()) return;
	// Selected region
	wxSize s = GetClientSize();
	int left   = int(slice.selection.x * scaleX + border);
	int top    = int(slice.selection.y * scaleY + border);
	int width  = int(slice.selection.width  * scaleX);
	int height = int(slice.selection.height * scaleY);
	// background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(Color(128,128,128));
	dc.DrawRectangle(0, 0, s.GetWidth(), s.GetHeight());
	// bitmap : unselected
	dc.DrawBitmap(bitmap_no_sel, border, border);
	// draw selected part ungreyed over it
	{
		wxMemoryDC mdc;
		mdc.SelectObject(bitmap);
		dc.Blit(left, top, width, height, &mdc, left - border, top - border);
		mdc.SelectObject(wxNullBitmap);
	}
	// border around source
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawRectangle(left - 1, top - 1, width + 2, height + 2);
	dc.SetPen(Color(64,64,64));
	dc.DrawRectangle(left - 2, top - 2, width + 4, height + 4);
	// Draw handles on all sides
	dc.SetBrush(Color(0,0,128));
	dc.SetPen(*wxWHITE_PEN);
	drawHandle(dc, -1, -1);
	drawHandle(dc, -1, +1);
	drawHandle(dc, +1, -1);
	drawHandle(dc, +1, +1);
	if (!slice.aspect_fixed) {
		drawHandle(dc, -1,  0);
		drawHandle(dc,  0, -1);
		drawHandle(dc,  0, +1);
		drawHandle(dc, +1,  0);
	}
}
void ImageSliceSelector::drawHandle(DC& dc, int dx, int dy) {
	wxPoint p = handlePos(dx, dy);
	dc.DrawRectangle(p.x - 3 + 4 * dx, p.y - 3 + 4 * dy, 6, 6);
}

int blur_pixel(Byte* in, int x, int y, int width, int height) {
	return (2 * (                      in[0])        + // center
	        (x == 0          ? in[0] : in[-3])       + // left
	        (y == 0          ? in[0] : in[-3*width]) + // up
	        (x == width - 1  ? in[0] : in[3])        + // right
	        (y == height - 1 ? in[0] : in[3*width])    // down
	       ) / 6;
}

void blur_image(const Image& img_in, Image& img_out) {
	int width = img_in.GetWidth(), height = img_in.GetHeight();
	assert(img_out.GetWidth() == width && img_out.GetHeight() == height);
	Byte* in = img_in.GetData(), *out = img_out.GetData();
	for (int y = 0 ; y < height ; ++y) {
		for (int x = 0 ; x < width ; ++x) {
			out[0] = blur_pixel(in + 0, x, y, width, height);
			out[1] = blur_pixel(in + 1, x, y, width, height);
			out[2] = blur_pixel(in + 2, x, y, width, height);
			in += 3; out += 3;
		}
	}
}
void desaturate_image(Image& img) {
	int width = img.GetWidth(), height = img.GetHeight();
	Byte* in = img.GetData();
	for (int y = 0 ; y < height ; ++y) {
		for (int x = 0 ; x < width ; ++x) {
			int r = in[0], g = in[1], b = in[2];
			// desaturate
			in[0] = (r + g + b + 5 * r + 255) / 9;
			in[1] = (r + g + b + 5 * g + 255) / 9;
			in[2] = (r + g + b + 5 * b + 255) / 9;
			in += 3;
		}
	}
}

void ImageSliceSelector::createBitmap() {
	// create image, resampled to fit in control
	wxSize s = GetClientSize();
	int width = s.GetWidth() - 2*border, height = s.GetHeight() - 2*border;
	Image img(width, height, false);
	resample(slice.source, img);
	bitmap = Bitmap(img);
	scaleX = (double)width  / slice.source.GetWidth();
	scaleY = (double)height / slice.source.GetHeight();
	// Initialize bitmap_no_sel to be the same bitmap, only with a faded color and blurred
	Image img_no_sel(width, height, false);
	blur_image(img, img_no_sel);
	blur_image(img_no_sel, img_no_sel);
	blur_image(img_no_sel, img_no_sel);
	desaturate_image(img_no_sel);
	bitmap_no_sel = Bitmap(img_no_sel);
}

// ----------------------------------------------------------------------------- : ImageSliceSelector : Mouse

void ImageSliceSelector::onLeftDown(wxMouseEvent& ev) {
	mouseX = ev.GetX();
	mouseY = ev.GetY();
	start_selection = slice.selection;
	mouse_down = true;
	onAnyHandle(ev, &dragX, &dragY);
	if (slice.aspect_fixed && (dragX == 0 || dragY == 0)) {
		dragX = dragY = 0; // only drag corners if aspect fixed
	}
	if (dragX == 0 && dragY == 0) {
		SetCursor(wxCURSOR_SIZING);
	}
	CaptureMouse();
}
void ImageSliceSelector::onLeftUp(wxMouseEvent&) {
	mouse_down = false;
	if (HasCapture()) ReleaseMouse();
	SetCursor(wxNullCursor);
}

void ImageSliceSelector::onMotion(wxMouseEvent& ev) {
	if (mouse_down) {
		double deltaX = (ev.GetX() - mouseX) / scaleX;
		double deltaY = (ev.GetY() - mouseY) / scaleY;
		// are we on a handle?
		if (dragX == 0 && dragY == 0) {
			// dragging entire selection
			slice.selection.x = int(start_selection.x + deltaX);
			slice.selection.y = int(start_selection.y + deltaY);
		} else {
			// fix aspect ratio
			if (slice.aspect_fixed) {
				// fixed == deltaX * height = deltaY * width
				double d = (dragX * deltaX * slice.target_size.GetHeight() + dragY * deltaY * slice.target_size.GetWidth()) / 2;
				deltaX = dragX * d / slice.target_size.GetHeight();
				deltaY = dragY * d / slice.target_size.GetWidth();
				/*if (dragX * deltaX * slice.target_size.GetWidth() <
				    dragY * deltaY * slice.target_size.GetHeight()) {
					deltaY = dragX * dragY * deltaX * slice.target_size.GetWidth() / slice.target_size.GetHeight();
				} else {
					deltaX = dragX * dragY * deltaY * slice.target_size.GetHeight() / slice.target_size.GetWidth();
				}*/
			}
			// move
			if (dragX) {
				slice.selection.x      = int(start_selection.x      + deltaX * (1 - dragX) / 2);
				slice.selection.width  = int(start_selection.width  + deltaX * dragX);
			}
			if (dragY) {
				slice.selection.y      = int(start_selection.y      + deltaY * (1 - dragY) / 2);
				slice.selection.height = int(start_selection.height + deltaY * dragY);
			}
		}
		
		// Notify parent
		wxCommandEvent ev(EVENT_SLICE_CHANGED, GetId());
		ProcessEvent(ev);
	} else {
		int dx, dy;
		if (onAnyHandle(ev, &dx, &dy)) {
			// what cursor to use?
			if      (dx ==  dy) SetCursor(wxCURSOR_SIZENWSE);
			else if (dx == -dy) SetCursor(wxCURSOR_SIZENESW);
			else if (dx == 0)   SetCursor(wxCURSOR_SIZENS);
			else if (dy == 0)   SetCursor(wxCURSOR_SIZEWE);
		} else {
			SetCursor(*wxSTANDARD_CURSOR);
		}
	}
}

// ----------------------------------------------------------------------------- : ImageSliceSelector : handles

bool ImageSliceSelector::onHandle(const wxMouseEvent& ev, int dx, int dy) const {
	wxPoint p = handlePos(dx, dy);
	p.x = p.x - 3 + 4 * dx;
	p.y = p.y - 3 + 4 * dy;
	return ev.GetX() >= p.x && ev.GetX() < p.x + 6 &&
		   ev.GetY() >= p.y && ev.GetY() < p.y + 6;
}
bool ImageSliceSelector::onAnyHandle(const wxMouseEvent& ev, int* dxOut, int* dyOut) const {
	for (int dx = -1 ; dx <= 1 ; ++dx) {
		for (int dy = -1 ; dy <= 1 ; ++dy) {
			if ((dx != 0 || dy != 0) && onHandle(ev, dx, dy)) { // (0,0) == center, not a handle
				*dxOut = dx;
				*dyOut = dy;
				return true;
			}
		}
	}
	*dxOut = *dyOut = 0;
	return false;
}
wxPoint ImageSliceSelector::handlePos(int dx, int dy) const {
	return wxPoint(
		int(scaleX * (slice.selection.x + ((dx + 1) * slice.selection.width)  * 0.5) + border),
		int(scaleY * (slice.selection.y + ((dy + 1) * slice.selection.height) * 0.5) + border)
	);
}

// ----------------------------------------------------------------------------- : ImageSliceSelector : Event table

BEGIN_EVENT_TABLE(ImageSliceSelector, wxControl)
	EVT_PAINT        (ImageSliceSelector::onPaint)
	EVT_LEFT_DOWN    (ImageSliceSelector::onLeftDown)
	EVT_LEFT_UP      (ImageSliceSelector::onLeftUp)
	EVT_MOTION       (ImageSliceSelector::onMotion)
	EVT_SIZE         (ImageSliceSelector::onSize)
END_EVENT_TABLE  ()
