//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_GAME
#define HEADER_DATA_GAME

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>

DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Game);

// ----------------------------------------------------------------------------- : Game

class Game : public Packaged {
  public:
	String fullName;
	String iconFilename;
	vector<FieldP> setFields;
	vector<FieldP> cardFields;
	
	/// Loads the game with a particular name, for example "magic"
	static GameP byName(const String& name);
	
	/// Is this Magic the Gathering?
	bool isMagic() const;
	
  protected:
	String typeName() const;
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
