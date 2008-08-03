//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_PACKAGE_LIST
#define HEADER_GUI_CONTROL_PACKAGE_LIST

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/error.hpp>
#include <gui/control/gallery_list.hpp>

DECLARE_POINTER_TYPE(Packaged);

// ----------------------------------------------------------------------------- : PackageList

/// A list of Packages of a specific type
class PackageList : public GalleryList {
  public:
	PackageList(Window* parent, int id, int direction = wxHORIZONTAL, bool always_focused = true);
	
	/// Shows packages that match a specific patern, and that are of the given type
	template <typename T>
	void showData(const String& pattern = _("*")) {
		showData(pattern + _(".mse-") + T::typeNameStatic());
	}
	
	/// Shows packages that match a specific patern
	void showData(const String& pattern = _("*"));
	
	/// Clears this list
	void clear();
		
	/// Get the selected package, T should be the same type used for showData
	/** @pre hasSelection()
	 *  Throws if the selection is not of type T */
	template <typename T>
	intrusive_ptr<T> getSelection(bool load_fully = true) const {
		intrusive_ptr<T> ret = dynamic_pointer_cast<T>(packages.at(columns[0].selection).package);
		if (!ret) throw InternalError(_("PackageList: Selected package has the wrong type"));
		if (load_fully) ret->loadFully();
		return ret;
	}
	
	/// Select the package with the given name, if it is not found, selects nothing
	void select(const String& name, bool send_event = true);
	
  protected:
	/// Return how many items there are in the list
	virtual size_t itemCount() const;
	/// Draw an item
	virtual void drawItem(DC& dc, int x, int y, size_t item);
	
  private:
	// The default icon to use
//	wxIcon default_icon;
	
	// Information about a package
	struct PackageData {
		PackageData() {}
		PackageData(const PackagedP& package, const Bitmap& image) : package(package), image(image) {}
		PackagedP package;
		Bitmap    image;
	};
	struct ComparePackagePosHint;
	/// The displayed packages
	vector<PackageData> packages;
};

// ----------------------------------------------------------------------------- : EOF
#endif
