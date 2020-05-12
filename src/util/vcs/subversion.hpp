//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs.hpp>

// ----------------------------------------------------------------------------- : SubversionVCS

class SubversionVCS : public VCS {
public:
  virtual void addFile (const wxFileName& filename);
  virtual void moveFile (const wxFileName& source, const wxFileName& destination);
  virtual void removeFile (const wxFileName& filename);
  
  DECLARE_REFLECTION();
};

