//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_CHOICE
#define HEADER_GUI_VALUE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <gui/drop_down_list.hpp>
#include <render/value/choice.hpp>

DECLARE_POINTER_TYPE(DropDownList);

// ----------------------------------------------------------------------------- : ChoiceValueEditor

/// An editor 'control' for editing ChoiceValues
class ChoiceValueEditor : public ChoiceValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Choice);
	
	// --------------------------------------------------- : Events
	virtual void onLeftDown(const RealPoint& pos, wxMouseEvent& ev);
	virtual void onChar(wxKeyEvent& ev);
	virtual void onLoseFocus();
	
	virtual void drawSelection(RotatedDC& dc);
	virtual void determineSize();
	
  private:
	DropDownListP drop_down;
	friend class DropDownChoiceList;
	/// Change the choice
	void change(const Defaultable<String>& c);
};

// ----------------------------------------------------------------------------- : DropDownChoiceList

// A drop down list of color choices
class DropDownChoiceList : public DropDownList {
  public:
	DropDownChoiceList(Window* parent, bool is_submenu, ChoiceValueEditor& cve, ChoiceField::ChoiceP group);
	
  protected:		
	virtual size_t        itemCount() const;
	virtual bool          lineBelow(size_t item) const;
	virtual String        itemText(size_t item) const;
	virtual void          drawIcon(DC& dc, int x, int y, size_t item, bool selected) const;
	virtual DropDownList* submenu(size_t item);
	
	virtual void   select(size_t item);
	virtual size_t selection() const;
	
  private:
	ChoiceValueEditor& cve;
	ChoiceField::ChoiceP group;		///< Group this menu shows
	vector<DropDownListP> submenus;
	
	inline const ChoiceField& field() const { return cve.field(); }
	
	inline bool hasFieldDefault() const { return group == field().choices && field().default_script; }
	inline bool hasGroupDefault() const { return group->hasDefault(); }
	inline bool hasDefault()      const { return hasFieldDefault() || hasGroupDefault(); }
	inline bool isFieldDefault(size_t item) const { return item == 0 && hasFieldDefault(); }
	inline bool isGroupDefault(size_t item) const { return item == 0 && hasGroupDefault(); }
	inline bool isDefault     (size_t item) const { return item == 0 && hasDefault(); }
	
	// Find an item in the group of choices
	ChoiceField::ChoiceP getChoice(size_t item) const;
};

// ----------------------------------------------------------------------------- : EOF
#endif
