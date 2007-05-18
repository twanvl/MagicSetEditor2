//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_IMAGE_SLICE_WINDOW
#define HEADER_GUI_IMAGE_SLICE_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class ImageSlicePreview;
class ImageSliceSelector;
class wxSpinEvent;
class wxSpinCtrl;
DECLARE_POINTER_TYPE(AlphaMask);

// ----------------------------------------------------------------------------- : ImageSlice

/// A slice of an image, i.e. a selected rectangle
class ImageSlice {
  public:
	ImageSlice(const Image& source, const wxSize& target_size);
	
	Image  source;		///< The source image
	wxSize target_size;	///< Size of the target image
	Color  background;	///< Color for areas outside the source image
	wxRect selection;	///< Area to slect from source
	bool allow_outside;
	bool aspect_fixed;	///< Aspect ratio lock?
	// Filters
	bool sharpen;
	int sharpen_amount;
	
	/// Enforce relations between values
	void constrain();
	/// Get the sliced image
	Image getSlice() const;
	
	// Zoom factor
	inline double zoomX() const { return target_size.GetWidth()  / (double)selection.width;  }
	inline double zoomY() const { return target_size.GetHeight() / (double)selection.height; }
	inline void zoomX(double zoom) { selection.width  = int(target_size.GetWidth()  / zoom); }
	inline void zoomY(double zoom) { selection.height = int(target_size.GetHeight() / zoom); }
};


// ----------------------------------------------------------------------------- : ImageSliceWindow

/// Dialog for selecting a slice of an image
class ImageSliceWindow : public wxDialog {
  public:
	ImageSliceWindow(Window* parent, const Image& source, const wxSize& target_size, const AlphaMaskP& target_mask);
	
	/// Return the sliced image
	Image getImage() const;
	
	// --------------------------------------------------- : Data
  private:
	// The slice we are extracting
	ImageSlice slice;
	// Gui items
	ImageSlicePreview*   preview;
	ImageSliceSelector*  selector;
	wxRadioBox*          size;
	wxSpinCtrl*          top, *left, *width, *height;
	wxCheckBox*          fix_aspect;
	wxSpinCtrl*          zoom, *zoom_x, *zoom_y;
	wxSizer*             zoom_sizer, *zoom_fixed, *zoom_free;
	wxCheckBox*          sharpen;
	wxSlider*            sharpen_amount;
	
	// --------------------------------------------------- : Events
	DECLARE_EVENT_TABLE();
	
	void onOk                 (wxCommandEvent&);
	
	void onSize               (wxSizeEvent&);
	
	void onChangeSize         (wxCommandEvent&);
	void onChangeLeft         (wxCommandEvent&);
	void onChangeTop          (wxCommandEvent&);
	void onChangeWidth        (wxCommandEvent&);
	void onChangeHeight       (wxCommandEvent&);
	void onChangeFixAspect    (wxCommandEvent&);
	void onChangeZoom         (wxSpinEvent&);
	void onChangeZoomX        (wxSpinEvent&);
	void onChangeZoomY        (wxSpinEvent&);
	void onChangeSharpen      (wxCommandEvent&);
	void onChangeSharpenAmount(wxScrollEvent&);
	
	// Something changed in the selector control, update controls and selection displays
	void onSliceChange(wxCommandEvent&);
	
	// --------------------------------------------------- : Updating
	
	// The manual controls were changed
	void onUpdateFromControl();
	// Update the values in the controls
	void updateControls();
};


// ----------------------------------------------------------------------------- : ImageSlicePreview

/// A preview of the sliced image
class ImageSlicePreview : public wxControl {
  public:
	ImageSlicePreview(Window* parent, int id, ImageSlice& slice, const AlphaMaskP& mask);
	
	/// Notify that the slice was updated
	void update();
	
	// --------------------------------------------------- : Data
  private:
	Bitmap bitmap;
	ImageSlice& slice;
	AlphaMaskP mask;
	
	bool mouse_down;
	int mouseX, mouseY;		///< starting mouse position
	wxRect start_selection; ///< selection in slice at start of dragging
	
	// --------------------------------------------------- : Events
	DECLARE_EVENT_TABLE();
	
	wxSize DoGetBestSize() const;
	
	void onLeftDown(wxMouseEvent&);
	void onLeftUp  (wxMouseEvent&);
	void onMotion  (wxMouseEvent&);
	
	void onPaint(wxPaintEvent&);
	void draw(DC& dc);
};

// ----------------------------------------------------------------------------- : ImageSliceSelector

// A overview of the slicing of the image, allows to select the sliced area
class ImageSliceSelector : public wxControl {
  public:
	ImageSliceSelector(Window* parent, int id, ImageSlice& slice);
	
	/// Notify that the slice was updated
	void update();
	
	// --------------------------------------------------- : Data
  private:
	ImageSlice& slice;
	Bitmap bitmap, bitmap_no_sel;	///< Bitmaps showing selection
	
	bool mouse_down;
	int mouseX, mouseY;			///< starting mouse position
	int dragX,  dragY;			///< corner that is being dragged
	wxRect start_selection;		///< selection in slice at start of dragging
	double scaleX, scaleY;		///< Amount the source image is scaled to fit in this control
	static const int border = 8;
	
	// --------------------------------------------------- : Events
	DECLARE_EVENT_TABLE();
	
	void onLeftDown(wxMouseEvent&);
	void onLeftUp  (wxMouseEvent&);
	void onMotion  (wxMouseEvent&);
	
	void onPaint(wxPaintEvent&);
	void onSize(wxSizeEvent&);
	
	// Is the mouse on a (scale) handle?
	bool onHandle(const wxMouseEvent& ev, int dx, int dy) const;
	// Is the mouse on any handle?
	bool onAnyHandle(const wxMouseEvent& ev, int* dxOut, int* dyOut) const;
	// Return the position of a handle, dx,dy in {-1,0,1}
	wxPoint handlePos(int dx, int dy) const;
	
	void draw(DC& dc);
	// Draw a handle, dx and dy indicate the side, can be {-1,0,1}
	void drawHandle(DC& dc, int dx, int dy);
	void createBitmap();
	
	
};

// ----------------------------------------------------------------------------- : EOF
#endif
