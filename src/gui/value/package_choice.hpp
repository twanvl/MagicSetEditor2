//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_PACKAGE_CHOICE
#define HEADER_GUI_VALUE_PACKAGE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/package_choice.hpp>

DECLARE_SHARED_POINTER_TYPE(DropDownList);

// ----------------------------------------------------------------------------- : PackageChoiceValueEditor

/// An editor 'control' for editing PackageChoiceValues
class PackageChoiceValueEditor : public PackageChoiceValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(PackageChoice);
	
	virtual void draw(RotatedDC& dc);
	virtual void determineSize(bool force_fit);
	virtual bool onLeftDown   (const RealPoint& pos, wxMouseEvent& ev);
	virtual bool onChar(wxKeyEvent& ev);
	virtual void onLoseFocus();
	
  private:
	DropDownListP drop_down;
	friend class DropDownPackageChoiceList;
	/// Change the choice
	void change(const String& c);
	/// Initialize the drop down list
	void initDropDown();
};


// ----------------------------------------------------------------------------- : EOF
#endif
