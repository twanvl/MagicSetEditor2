//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_VALUE_CHOICE
#define HEADER_GUI_VALUE_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <gui/drop_down_list.hpp>
#include <render/value/choice.hpp>

DECLARE_SHARED_POINTER_TYPE(DropDownList);
DECLARE_POINTER_TYPE(ThumbnailRequest);

// ----------------------------------------------------------------------------- : ChoiceValueEditor

/// An editor 'control' for editing ChoiceValues
class ChoiceValueEditor : public ChoiceValueViewer, public ValueEditor {
  public:
	DECLARE_VALUE_EDITOR(Choice);
	~ChoiceValueEditor();
	
	// --------------------------------------------------- : Events
	virtual bool onLeftDown(const RealPoint& pos, wxMouseEvent& ev);
	virtual bool onChar(wxKeyEvent& ev);
	virtual void onLoseFocus();
	
	virtual void draw(RotatedDC& dc);
	virtual void determineSize(bool);
	
  private:
	DropDownListP drop_down;
	friend class DropDownChoiceList;
	/// Change the choice
	void change(const Defaultable<String>& c);
};

// ----------------------------------------------------------------------------- : DropDownChoiceList

/// A drop down list of choices
/** This is a base class, used for single and multiple choice fields */
class DropDownChoiceListBase : public DropDownList {
  public:
	DropDownChoiceListBase(Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group);
	
  protected:
	virtual void          onShow();
	virtual size_t        itemCount() const;
	virtual bool          lineBelow(size_t item) const;
	virtual bool          itemEnabled(size_t item) const;
	virtual String        itemText(size_t item) const;
	virtual void          drawIcon(DC& dc, int x, int y, size_t item, bool selected) const;
	virtual DropDownList* submenu(size_t item) const;
	
  protected:
	virtual DropDownList* createSubMenu(ChoiceField::ChoiceP group) const = 0;
	
  private:
	DECLARE_EVENT_TABLE();
	
	ValueViewer& cve;				///< Editor this list belongs to
	ChoiceField::ChoiceP group;		///< Group this menu shows
	mutable vector<DropDownListP> submenus;
	mutable int default_id; ///< Item id for the default item (if !hasFieldDefault()) this is undefined)
	
	inline ChoiceField& field() const { return static_cast<ChoiceField&>(*cve.getField()); }
	inline ChoiceStyle& style() const { return static_cast<ChoiceStyle&>(*cve.getStyle()); }
	
	inline bool isRoot()          const { return group == field().choices; }
	inline bool hasFieldDefault() const { return isRoot() && field().default_script; }
	inline bool hasGroupDefault() const { return group->hasDefault(); }
	virtual bool hasDefault()     const { return hasFieldDefault() || hasGroupDefault(); }
	inline bool isFieldDefault(size_t item) const { return item == 0 && hasFieldDefault(); }
	inline bool isGroupDefault(size_t item) const { return item == 0 && hasGroupDefault(); }
	inline bool isDefault     (size_t item) const { return item == 0 && hasDefault(); }
	
	// Find an item in the group of choices
	ChoiceField::ChoiceP getChoice(size_t item) const;
	/// Start generating thumbnail images
	void generateThumbnailImages();
	void onIdle(wxIdleEvent&);
};

// ----------------------------------------------------------------------------- : DropDownChoiceList

/// A drop down list of color choices
class DropDownChoiceList : public DropDownChoiceListBase {
  public:
	DropDownChoiceList(Window* parent, bool is_submenu, ValueViewer& cve, ChoiceField::ChoiceP group);
	
  protected:
	virtual void   onShow();
	virtual void   select(size_t item);
	virtual size_t selection() const;
	virtual DropDownList* createSubMenu(ChoiceField::ChoiceP group) const;
};

// ----------------------------------------------------------------------------- : EOF
#endif
