//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_IMAGE
#define HEADER_RENDER_VALUE_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/image.hpp>

DECLARE_POINTER_TYPE(AlphaMask);

// ----------------------------------------------------------------------------- : ImageValueViewer

/// Viewer that displays an image value
class ImageValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(Image) : ValueViewer(parent,style) {}
	
	virtual void draw(RotatedDC& dc);
	
	virtual bool containsPoint(const RealPoint& p) const;
	
	virtual void onValueChange();
	virtual void onStyleChange();
			
  private:
	Bitmap bitmap;
	mutable AlphaMaskP alpha_mask;
	
	void loadMask(const Rotation& rot) const;
	
	/// Generate a placeholder image
	static Bitmap imagePlaceholder(const Rotation& rot, UInt w, UInt h, bool editing);
};

// ----------------------------------------------------------------------------- : EOF
#endif
