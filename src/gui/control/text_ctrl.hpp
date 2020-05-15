//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/control/card_editor.hpp>

class TextStyle;
DECLARE_POINTER_TYPE(TextField);
DECLARE_POINTER_TYPE(FakeTextValue);

// ----------------------------------------------------------------------------- : TextCtrl

/// A control for editing a String
/** Implemented using a DataEditor. A fake Field and Value are created for the item.
 *
 *  TODO:
 *  Possible problem: If multiple TextCtrls are editing the same value they will both
 *                    create a Value, actions modifying one of them will not match with actions modifying the other
 *  Possible solution: 1. Global map of Values
 *                     2. Ignore the problem, it will not happen in practice
 */
class TextCtrl : public DataEditor {
public:
  TextCtrl(Window* parent, int id, bool multi_line, long style = wxBORDER_THEME);
  ~TextCtrl();
  
  /// Set the value that is being edited
  /** value can be a nullptr*/
  void setValue(String* value, bool untagged = false);
  /// Set the value that is being edited
  void setValue(const FakeTextValueP& value);
  
  /// Update the size, for example after changing the style
  void updateSize();
  
  /// Get access to the field used by the control
  TextField& getField();
  /// Get access to the field used by the control
  TextFieldP getFieldP();
  /// Get access to the style used by the control
  TextStyle& getStyle();
  
  /// Uses a native look
  bool nativeLook() const override { return true; }
  Rotation getRotation() const override;
  
  void draw(DC& dc) override;
  
  bool AcceptsFocus() const override;
  
  void onChangeSet() override;
  
protected:
  void onInit() override;
  wxSize DoGetBestSize() const override;
  
private:
  bool multi_line; ///< Multi line text control?
  
  DECLARE_EVENT_TABLE();
  
  void onSize(wxSizeEvent&);
};


