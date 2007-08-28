//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
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
	Rotation(int angle, const RealRect& rect, double zoom = 1.0, double strectch = 1.0, bool is_internal = false);
	
	/// Change the zoom factor
	inline void setZoom(double z) { zoomX = zoomY = z; }
	/// Change the angle
	void setAngle(int a);
	/// Change the origin
	inline void setOrigin(const RealPoint& o) { origin = o; }
	
	/// The internal size
	inline RealSize getInternalSize() const { return trInvNoNeg(size); }
	/// The intarnal rectangle (origin at (0,0))
	inline RealRect getInternalRect() const { return RealRect(RealPoint(0,0), getInternalSize()); }
	/// The size of the external rectangle (as passed to the constructor) == trNoNeg(getInternalSize())
	inline RealSize getExternalSize() const { return size; }
	/// The external rectangle (as passed to the constructor) == trNoNeg(getInternalRect())
	RealRect getExternalRect() const;
	
	/// Translate a size or length
	inline double trS(double s) const { return s * zoomY; }
	inline double trX(double s) const { return s * zoomX; }
	inline double trY(double s) const { return s * zoomY; }
	
	/// Translate an angle
	inline int trAngle(int a) { return (angle + a) % 360; }
	
	/// Translate a single point
	RealPoint tr(const RealPoint& p) const;
	/// Translate a single size, the result may be negative
	RealSize tr(const RealSize& s) const;
	/// Translate a rectangle, the size of the result may be negative
	RealRect tr(const RealRect& r) const;
	
	/// Translate a size, the result will never be negative
	RealSize trNoNeg(const RealSize& s) const;
	/// Translate a rectangle, the result will never have a negative size
	RealRect trNoNeg(const RealRect& r) const;
	/// Translate a rectangle, the result will never have a negative size
	/** The rectangle is also not zoomed */
	RealRect trNoNegNoZoom(const RealRect& r) const;
	
	/// Translate a size or length back to internal 'coordinates'
	inline double   trInvS(double s)           const { return s / zoomY; }
	inline double   trInvX(double s)           const { return s / zoomX; }
	inline double   trInvY(double s)           const { return s / zoomY; }
	/// Translate a size back to internal 'coordinates', doesn't rotate
	inline RealSize trInvS(const RealSize& s) const { return RealSize(s.width / zoomX, s.height / zoomY); }
	
	/// Translate a point back to internal coordinates
	RealPoint trInv(const RealPoint& p) const;
	/// Translate a size back to internal coordinates
	RealSize  trInv(const RealSize&  s) const;
	/// Translate a size back to internal coordinates, that are not negative
	RealSize  trInvNoNeg(const RealSize&  s) const;
	
	/// Stretch factor
	inline double stretch() const { return zoomX / zoomY; }
	
  protected:
	int angle;				///< The angle of rotation in degrees (counterclockwise)
	RealSize size;			///< Size of the rectangle, in external coordinates
	RealPoint origin;		///< tr(0,0)
	double zoomX;			///< Zoom factor, zoom = 2.0 means that 1 internal = 2 external
	double zoomY;
	
	friend class Rotater;
	
  public:
	/// Is the rotation sideways (90 or 270 degrees)?
	inline bool sideways() const { return ::sideways(angle); }
	
  protected:
	/// Is the x axis 'reversed' (after turning sideways)?
	inline bool revX()     const { return  angle >= 180; }
	/// Is the y axis 'reversed' (after turning sideways)?
	inline bool revY()     const { return  angle == 90 || angle == 180; }
	/// Negate if revX
	inline double negX(double d) const { return revX() ? -d : d; }
	/// Negate if revY
	inline double negY(double d) const { return revY() ? -d : d; }
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
	QUALITY_AA,			///< Our own anti aliassing
	QUALITY_SUB_PIXEL,	///< Sub-pixel positioning
	QUALITY_LOW,		///< Normal
};

/// A DC with rotation applied
/** All draw** functions take internal coordinates.
 */
class RotatedDC : public Rotation {
  public:
	RotatedDC(DC& dc, int angle, const RealRect& rect, double zoom, RenderQuality quality, bool is_internal = false);
	RotatedDC(DC& dc, const Rotation& rotation, RenderQuality quality);
  
	// --------------------------------------------------- : Drawing
	
	void DrawText  (const String& text,   const RealPoint& pos, int blur_radius = 0, int boldness = 1);
	/// Draw abitmap, it must already be zoomed!
	void DrawBitmap(const Bitmap& bitmap, const RealPoint& pos);
	/// Draw an image using the given combining mode, the image must already be zoomed!
	void DrawImage (const Image& image,   const RealPoint& pos, ImageCombine combine = COMBINE_DEFAULT, int angle = 0);
	/// Draw a bitmap that is already zoomed and rotated
	void DrawPreRotatedBitmap(const Bitmap& bitmap, const RealPoint& pos);
	/// Draw an image that is already zoomed and rotated
	void DrawPreRotatedImage(const Image& image, const RealPoint& pos, ImageCombine combine = COMBINE_DEFAULT);
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
