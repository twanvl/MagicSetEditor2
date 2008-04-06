//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ROTATION
#define HEADER_UTIL_ROTATION

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>
#include <gfx/gfx.hpp>

class Font;

// ----------------------------------------------------------------------------- : Rotation

enum RotationFlags
{	ROTATION_NORMAL
,	ROTATION_ATTACH_TOP_LEFT
};

/// An object that can rotate coordinates inside a specified rectangle
/** This class has lots of tr*** functions, they convert
 *  internal coordinates to external/screen coordinates.
 *  tr***inv do the opposite.
 */
class Rotation {
  public:
	/// Construct a rotation object
	/** with the given rectangle of external coordinates and a given rotation angle and zoom factor.
	 *  if is_internal then the rect gives the internal coordinates, its origin should be (0,0)
	 */
	Rotation(int angle, const RealRect& rect, double zoom = 1.0, double strectch = 1.0, RotationFlags flags = ROTATION_NORMAL);
	
	/// Change the zoom factor
	inline void setZoom(double z) { zoomX = zoomY = z; }
	/// Retrieve the zoom factor
	inline double getZoom() const { return zoomY; }
	/// Change the stretch factor
	void setStretch(double s);
	/// Stretch factor
	inline double getStretch() const { return zoomX / zoomY; }
	/// Get the angle
	inline int getAngle() const { return angle; }
	/// Change the origin
	inline void setOrigin(const RealPoint& o) { origin = o; }
	
	
	/// The internal size
	inline RealSize getInternalSize() const { return size; }
	inline double   getWidth()  const { return size.width; }
	inline double   getHeight() const { return size.height; }
	/// The intarnal rectangle (origin at (0,0))
	inline RealRect getInternalRect() const { return RealRect(RealPoint(0,0), size); }
	/// The size of the external rectangle (as passed to the constructor) == trNoNeg(getInternalSize())
	inline RealSize getExternalSize() const { return trSizeToBB(size); }
	/// The external rectangle (as passed to the constructor) == trNoNeg(getInternalRect())
	inline RealRect getExternalRect() const { return trRectToBB(getInternalRect()); }
	
	/// Translate a size or length
	inline double trS(double s) const { return s * zoomY; }
	inline double trX(double s) const { return s * zoomX; }
	inline double trY(double s) const { return s * zoomY; }
	inline RealSize trS(const RealSize& s) const { return RealSize(s.width * zoomX, s.height * zoomY); }
	
	/// Translate an angle
	inline int trAngle(int a) { return (angle + a) % 360; }
	
	/// Translate a single point
	RealPoint tr(const RealPoint& p) const;
	/// Translate a single point, but don't zoom
	RealPoint trNoZoom(const RealPoint& p) const;
	/// Translate a 'pixel'. A pixel has size 1*1
	RealPoint trPixel(const RealPoint& p) const;
	/// Translate a 'pixel', but don't zoom
	RealPoint trPixelNoZoom(const RealPoint& p) const;
	/// Translate a single size
	RealSize trSize(const RealSize& s) const;
	/// Translate a single size, returns the bounding box size (non-negative)
	RealSize trSizeToBB(const RealSize& s) const;
	/// Translate a rectangle, returns the bounding box
	/* //%%the size of the result may be negative*/
	RealRect trRectToBB(const RealRect& r) const;
	/// Translate a rectangle, can only be used when not rotating
	RealRect trRectStraight(const RealRect& r) const;
	/// Translate a rectangle into a region (supports rotation
	wxRegion trRectToRegion(const RealRect& rect) const;
	
	/// Translate a size or length back to internal 'coordinates'
	inline double   trInvS(double s)           const { return s / zoomY; }
	inline double   trInvX(double s)           const { return s / zoomX; }
	inline double   trInvY(double s)           const { return s / zoomY; }
	/// Translate a size back to internal 'coordinates', doesn't rotate
	inline RealSize trInvS(const RealSize& s) const { return RealSize(s.width / zoomX, s.height / zoomY); }
	
	/// Translate a point back to internal coordinates
	RealPoint trInv(const RealPoint& p) const;
	
  protected:
	int angle;				///< The angle of rotation in degrees (counterclockwise)
	RealSize size;			///< Size of the rectangle, in internal coordinates
	RealPoint origin;		///< tr(0,0)
	double zoomX;			///< Zoom factor, zoom = 2.0 means that 1 internal = 2 external
	double zoomY;
	
	friend class Rotater;
	
	/// Is the x axis 'reversed' (after turning sideways)?
	inline bool revX()     const { return  angle >= 180; }
	/// Is the y axis 'reversed' (after turning sideways)?
	inline bool revY()     const { return  angle == 90 || angle == 180; }
	/// Is the rotation 'simple', i.e. a multiple of 90 degrees?
	inline bool straight() const { return  ::straight(angle); }
	/// Is the rotation sideways (90 or 270 degrees)?
	//  Note: angle & 2 == 0 for angle in {0, 180} and != 0 for angle in {90, 270)
	inline bool sideways() const { return (angle & 2) != 0; }
	
	/// Determine the top-left corner of the bounding box around the rotated box s (in external coordinates)
	RealPoint boundingBoxCorner(const RealSize& s) const;
};

// ----------------------------------------------------------------------------- : Rotater

/// An object that changes a rotation RIIA style
/** Usage:
 *  @code
 *     Rotation a, b;
 *     Rotater(a,b);
 *     a.tr(x) // now acts as a.tr(b.tr(x))
 *  @endcode
 */
class Rotater {
  public:
	/// Compose a rotation by onto the rotation rot
	/** rot is restored when this object is destructed
	 */
	Rotater(Rotation& rot, const Rotation& by);
	~Rotater();
  private:
	Rotation old;
	Rotation& rot;
};

// ----------------------------------------------------------------------------- : RotatedDC

/// Render quality of text
enum RenderQuality {
	QUALITY_LOW,		///< Normal
	QUALITY_SUB_PIXEL,	///< Sub-pixel positioning
	QUALITY_AA,			///< Our own anti aliassing
};

/// A DC with rotation applied
/** All draw** functions take internal coordinates.
 */
class RotatedDC : public Rotation {
  public:
	RotatedDC(DC& dc, int angle, const RealRect& rect, double zoom, RenderQuality quality, RotationFlags flags = ROTATION_NORMAL);
	RotatedDC(DC& dc, const Rotation& rotation, RenderQuality quality);
	
	// --------------------------------------------------- : Drawing
	
	/// Draw text
	void DrawText  (const String& text,   const RealPoint& pos, int blur_radius = 0, int boldness = 1, double stretch = 1.0);
	/// Draw text with the shadow and color settings of the given font
	void DrawTextWithShadow(const String& text, const Font& font, const RealPoint& pos, double scale = 1.0, double stretch = 1.0);
	/// Draw abitmap, it must already be zoomed!
	void DrawBitmap(const Bitmap& bitmap, const RealPoint& pos);
	/// Draw an image using the given combining mode, the image must already be zoomed!
	void DrawImage (const Image& image,   const RealPoint& pos, ImageCombine combine = COMBINE_DEFAULT);
	/// Draw a bitmap that is already zoomed and rotated.
	/** The rectangle the position in internal coordinates, and the size before rotating and zooming */
	void DrawPreRotatedBitmap(const Bitmap& bitmap, const RealRect& rect);
	/// Draw an image that is already zoomed and rotated
	void DrawPreRotatedImage(const Image& image, const RealRect& rect, ImageCombine combine = COMBINE_DEFAULT);
	void DrawLine  (const RealPoint& p1,  const RealPoint& p2);
	void DrawRectangle(const RealRect& r);
	void DrawRoundedRectangle(const RealRect& r, double radius);
	void DrawCircle(const RealPoint& center, double radius);
	/// Draw an arc of an ellipse, angles are in radians
	void DrawEllipticArc(const RealPoint& center, const RealSize& size, double start, double end);
	/// Draw spokes of an ellipse
	void DrawEllipticSpoke(const RealPoint& center, const RealSize& size, double start);
	
	// Fill the dc with the color of the current brush
	void Fill();
	
	// --------------------------------------------------- : Properties
	
	/// Sets the pen for the dc, does not scale the line width
	void SetPen(const wxPen&);
	void SetBrush(const wxBrush&);
	void SetTextForeground(const Color&);
	void SetLogicalFunction(int function);
	
	void SetFont(const wxFont& font);
	/// Set the font, scales for zoom and high_quality
	/** The font size will be multiplied by 'scale' */
	void SetFont(const Font& font, double scale);
	/// Steps to use when decrementing font size
	double getFontSizeStep() const;
	
	RealSize GetTextExtent(const String& text) const;
	double GetCharHeight() const;
	
	void SetClippingRegion(const RealRect& rect);
	void DestroyClippingRegion();
	
	// --------------------------------------------------- : Other
	
	/// Get the current contents of the given ractangle, for later restoring
	Bitmap GetBackground(const RealRect& r);
	
	inline wxDC& getDC() { return dc; }
	
  private:
	wxDC& dc;				///< The actual dc
	RenderQuality quality;	///< Quality of the text
};

// ----------------------------------------------------------------------------- : EOF
#endif
