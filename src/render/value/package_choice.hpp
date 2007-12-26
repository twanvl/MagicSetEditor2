//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_VALUE_PACKAGE_CHOICE
#define HEADER_RENDER_VALUE_PACKAGE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <render/value/viewer.hpp>
#include <data/field/package_choice.hpp>

// ----------------------------------------------------------------------------- : PackageChoiceValueViewer

/// Viewer that displays a package choice value
class PackageChoiceValueViewer : public ValueViewer {
  public:
	DECLARE_VALUE_VIEWER(PackageChoice) : ValueViewer(parent,style) { initItems(); }
	
	virtual void draw(RotatedDC& dc);
	
	struct Item{
		String package_name;
		String name;
		Bitmap image;
	};
  protected:
	vector<Item> items;
  private:
	void initItems();
	struct ComparePackagePosHint;
};

// ----------------------------------------------------------------------------- : EOF
#endif
