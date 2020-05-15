//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/value/editor.hpp>
#include <render/value/image.hpp>

// ----------------------------------------------------------------------------- : ImageValueEditor

/// An editor 'control' for editing ImageValues
class ImageValueEditor : public ImageValueViewer, public ValueEditor {
public:
  DECLARE_VALUE_EDITOR(Image);
  
  bool onLeftDClick(const RealPoint&, wxMouseEvent&) override;
  
  // --------------------------------------------------- : Clipboard
  
  bool canCopy() const override;
  bool canPaste() const override;
  bool doCopy() override;
  bool doPaste() override;
  bool doDelete() override;
  
  bool onChar(wxKeyEvent&) override;
  
private:
  // Open the image slice window showing the give image
  void sliceImage(const Image&);
};

