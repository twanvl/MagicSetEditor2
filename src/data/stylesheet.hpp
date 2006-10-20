//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_STYLESHEET
#define HEADER_DATA_STYLESHEET

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>

DECLARE_POINTER_TYPE(Game);

// ----------------------------------------------------------------------------- : StyleSheet

/// A collection of style information for card and set fields
class StyleSheet : public Packaged {
  public:
	GameP game;
	
	static String typeNameStatic();
	virtual String typeName() const;
	virtual String fullName() const;
	virtual InputStreamP openIconFile();
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
