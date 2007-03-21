//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_IMAGE
#define HEADER_GUI_VALUE_IMAGE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/image.hpp>

// ----------------------------------------------------------------------------- : ImageValueEditor

/// An editor 'control' for editing ImageValues
class ImageValueEditor : public ImageValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Image);
	
	virtual bool onLeftDClick(const RealPoint&, wxMouseEvent&);
	
	// --------------------------------------------------- : Clipboard
	
	virtual bool canCopy()  const;
	virtual bool canCut()   const { return false; }
	virtual bool canPaste() const;
	virtual bool doCopy();
	virtual bool doPaste();
	
  private:
	// Open the image slice window showing the give image
	void sliceImage(const Image&);
};

// ----------------------------------------------------------------------------- : EOF
#endif
