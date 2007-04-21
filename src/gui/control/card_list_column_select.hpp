//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_CONTROL_CARD_LIST_COLUMN_SELECT
#define HEADER_GUI_CONTROL_CARD_LIST_COLUMN_SELECT

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/settings.hpp>

DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(Field);

// ----------------------------------------------------------------------------- : CardListColumnSelectDialog

/// A dialog for selecting the card list columns to show and their order
/** Layout
 *   | <X> col a |  <^>
 *   | < > col b |  <V>
 *   | <X> col b |
 *
 *       <ok> <cancel>
 */
class CardListColumnSelectDialog : public wxDialog {
  public:
	CardListColumnSelectDialog(Window* parent, const GameP& game);
	
  private:
	DECLARE_EVENT_TABLE();
	
	// gui items
	wxCheckListBox* list;
	// other info
	GameP game;									///< The game we are changing
	public: struct ColumnSettingsF {
		ColumnSettingsF(const FieldP& field, const ColumnSettings& settings)
			: field(field)
			, settings(settings)
		{}
		FieldP         field;
		ColumnSettings settings;
	};
	private: vector<ColumnSettingsF> columns;	///< Settings of the fields, in order
	
	// initialize columns
	void initColumns();
	// intialize the list box
	void initList();
	// refresh list item i
	void refreshItem(int i);
	
	void onSelect  (wxCommandEvent&);
	void onCheck   (wxCommandEvent&);
	void onMove    (wxCommandEvent&);
	void onShowHide(wxCommandEvent&);
	void onOk      (wxCommandEvent&);
	void onUpdateUI(wxUpdateUIEvent&);
	
};

// ----------------------------------------------------------------------------- : EOF
#endif
