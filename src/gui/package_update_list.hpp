//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_PACKAGE_UPDATE_LIST
#define HEADER_GUI_PACKAGE_UPDATE_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/tree_list.hpp>
#include <data/installer.hpp>

// ----------------------------------------------------------------------------- : PackageUpdateList

/// A list of installed and downloadable packages
class PackageUpdateList : public TreeList {
  public:
	PackageUpdateList(Window* parent, const InstallablePackages& packages, int id = wxID_ANY);
	~PackageUpdateList();
	
	inline InstallablePackageP getSelection() const {
		return selection == NOTHING ? InstallablePackageP() : get(selection);
	}
	
	inline InstallablePackageP get(size_t item) const {
		return static_pointer_cast<TreeItem>(items[item])->package;
	}
		
  protected:
	// overridden methods from TreeList
	virtual void initItems();
	virtual void drawItem(DC& dc, size_t index, size_t column, int x, int y, bool selected) const;
	
	virtual size_t columnCount() const { return 3; }
	virtual String columnText(size_t column) const;
	virtual int    columnWidth(size_t column) const;
	
  private:
	const InstallablePackages& packages;
	
	class TreeItem;
  public:
	typedef intrusive_ptr<TreeItem> TreeItemP;
  private:
	class TreeItem : public Item {
	  public:
		TreeItem() : position_type(TYPE_OTHER), position_hint(1000000) {}
		String label;
		vector<TreeItemP> children;
		InstallablePackageP package;
		Bitmap icon, icon_grey;
		// positioning
		enum PackageType {
			TYPE_PROG,
			TYPE_LOCALE,
			TYPE_GAME,
			TYPE_STYLESHEET,
			TYPE_EXPORT_TEMPLATE,
			TYPE_SYMBOL_FONT,
			TYPE_INCLUDE,
			TYPE_FONT,
			TYPE_OTHER,
		}   position_type;
		int position_hint;
		
		void add(const InstallablePackageP& package, const String& path, int level = -1);
		void toItems(vector<TreeList::ItemP>& items);
		void setIcon(const Image& image);
		bool highlight() const;
		
		static PackageType package_type(const PackageDescription& desc);
	};
	friend class PackageIconRequest;
};

// ----------------------------------------------------------------------------- : EOF
#endif
