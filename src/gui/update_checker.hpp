//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_UPDATE_CHECKER
#define HEADER_UTIL_UPDATE_CHECKER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Update checking

// Checks for updates if the settings say so
void check_updates();

/// Checks if the current version is the latest version
/** If async==true then checking is done in another thread
 */
void check_updates_now(bool async = true);

/// Show a dialog to inform the user that updates are available (if there are any)
/** Call check_updates first.
 *  Call this function from an onIdle loop */
void show_update_dialog(Window* parent);

// ----------------------------------------------------------------------------- : Update window

class PackageUpdateList;
class wxHtmlWindow;

DECLARE_POINTER_TYPE(PackageVersionData);

/// A window that displays the updates and allows the user to select some.
/** NOTE: cannot be called 'UpdateWindow' because there is a Win32 function with that name
 */
class UpdatesWindow : public Frame {
  public:
	UpdatesWindow();
	
	void DrawTitles(wxPaintEvent&);

	enum PackageStatus {
		STATUS_INSTALLED,
		STATUS_NOT_INSTALLED,
		STATUS_UPGRADEABLE
	};
	enum PackageAction {
		ACTION_INSTALL,
		ACTION_UNINSTALL,
		ACTION_UPGRADE,
		ACTION_NOTHING,
		ACTION_NEW_MSE
	};

	typedef pair<PackageStatus, PackageAction> PackageData;

	map<PackageVersionDataP, PackageData> package_data;

  private:
	DECLARE_EVENT_TABLE();
	PackageUpdateList* package_list; ///< List of available packages
	wxHtmlWindow* description_window;

	wxStaticText *package_title, *status_title, *new_title;
	wxButton *install_button, *upgrade_button, *remove_button, *cancel_button, *apply_button;
	
	void onUpdateCheckFinished(wxCommandEvent&);
	void onPackageSelect(wxCommandEvent&);
	void onActionChange(wxCommandEvent&);
	void onApplyChanges(wxCommandEvent&);

	void SelectPackageDependencies   (PackageVersionDataP);
	void RemovePackageDependencies   (PackageVersionDataP);
	void DowngradePackageDependencies(PackageVersionDataP);

	/// Update the buttons to indicate that this is selected.
	void updateButtons(int index);
	
	void setDefaultPackageStatus();
};

/// Was update data found?
bool update_data_found();
/// Is there an update?
bool update_available();

// ----------------------------------------------------------------------------- : EOF
#endif
