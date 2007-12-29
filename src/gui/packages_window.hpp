//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_PACKAGES_WINDOW
#define HEADER_GUI_PACKAGES_WINDOW

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/installer.hpp>

class PackageUpdateList;
class PackageInfoPanel;

// ----------------------------------------------------------------------------- : Packages window

/// A window that displays the installed packages and updates to them
class PackagesWindow : public wxDialog {
  public:
	PackagesWindow(Window* parent, bool download_package_list = true);
	~PackagesWindow();
		
	InstallablePackages installable_packages;
	
  private:
	PackageUpdateList* package_list; ///< List of available packages
	PackageInfoPanel*  package_info; ///< Description of the selected package
	
	InstallablePackageP package; ///< Selected package
	PackageAction       where;   ///< Where to install? (PACKAGE_LOCAL or PACKAGE_GLOBAL)
	
	bool waiting_for_list; ///< waiting for the list of installers?
	
	DECLARE_EVENT_TABLE();
	
	void onOk(wxCommandEvent&);
	void onActionChange(wxCommandEvent&);
	void onPackageSelect(wxCommandEvent&);
	void onUpdateUI(wxUpdateUIEvent&);
	void onIdle(wxIdleEvent&);
	
	bool checkInstallerList(bool refresh = true);
};

// ----------------------------------------------------------------------------- : EOF
#endif
