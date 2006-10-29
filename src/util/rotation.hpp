//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ROTATION
#define HEADER_UTIL_ROTATION

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/real_point.hpp>
#include <gfx/gfx.hpp>

// ----------------------------------------------------------------------------- : Rotation

/// An object that can rotate coordinates inside a specified rectangle
/** This class has lots of tr*** functions, they convert
 *  internal coordinates to external/screen coordinates.
 *  tr***inv do the opposite.
 */
class Rotation {
  public:
	/// Construct a rotation object with the given rectangle of external coordinates
	/// and a given rotation angle and zoom factor
	Rotation(int angle, const RealRect& rect, double zoom = 1.0);
	
	/// Change the zoom factor
	inline void setZoom(double z) { zoom = z; }
	/// Change the angle
	void setAngle(int a);
	/// The internal size
	inline RealSize getInternalSize() const { return trInv(size); }
	/// The intarnal rectangle (origin at (0,0))
	inline RealRect getInternalRect() const { return RealRect(RealPoint(0,0), getInternalSize()); }
	
	/// Translate a size or length
	inline double trS(double s) const { return  s * zoom; }
	
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
	inline double trInvS(double s) const { return  s / zoom; }
	
	/// Translate a point back to internal coordinates
	RealPoint trInv(const RealPoint& p) const;
	/// Translate a size back to internal coordinates
	RealSize  trInv(const RealSize&  s) const;
	
  protected:
	int angle;				///< The angle of rotation in degrees (counterclockwise)
	RealSize size;			///< Size of the rectangle, in external coordinates
	RealPoint origin;		///< tr(0,0)
	double zoom;			///< Zoom factor, zoom = 2.0 means that 1 internal = 2 external
	
	/// Is the rotation sideways (90 or 270 degrees)?
	// Note: angle & 2 == 0 for angle in {0, 180} and != 0 for angle in {90, 270)
	inline bool sideways() const { return  (angle & 2) != 0; }
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
	/// Compose a rotation by onto the rotation rot
	/** rot is restored when this object is destructed
	 */
	Rotater(Rotation& rot, const Rotation& by);
	~Rotater();
};

// ----------------------------------------------------------------------------- : RotatedDC

/// A DC with rotation applied
/** All draw** functions take internal coordinates.
 */
class RotatedDC : public Rotation {
  public:
	RotatedDC(DC& dc, int angle, const RealRect& rect, double zoom, bool high_quality);
	RotatedDC(DC& dc, const Rotation& rotation, bool high_quality);
  
	// --------------------------------------------------- : Drawing
	
	void DrawText  (const String& text,   const RealPoint& pos);
	/// Draw abitmap, it must already be zoomed!
	void DrawBitmap(const Bitmap& bitmap, const RealPoint& pos);
	/// Draw an image using the given combining mode, the image must already be zoomed!
	void DrawImage (const Image& image,   const RealPoint& pos, ImageCombine combine = COMBINE_NORMAL);
	void DrawLine  (const RealPoint& p1,  const RealPoint& p2);
	void DrawRectangle(const RealRect& r);
	void DrawRoundedRectangle(const RealRect& r, double radius);
	
	// Fill the dc with the color of the current brush
	void Fill();
	
	// --------------------------------------------------- : Forwarded properties
	
	/// Sets the pen for the dc, does not scale the line width
	void SetPen(const wxPen&);
	void SetBrush(const wxBrush&);
	void SetTextForeground(const Color&);
	void SetLogicalFunction(int function);
	void SetFont(const wxFont& font);
	/// Set the font, scales for zoom and high_quality
	/** The font will get the given (internal) point size */
	void SetFont(wxFont font, double size);
	
	RealSize GetTextExtent(const String& text) const;
	double GetCharHeight() const;
	
	inline wxDC& getDC() { return dc; }
	
  private:
	wxDC& dc;			///< The actual dc
	bool high_quality;	///< Drawing using our own anti aliassing?
};

// ----------------------------------------------------------------------------- : EOF
#endif
