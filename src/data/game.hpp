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
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Game);

// ----------------------------------------------------------------------------- : Game

class Game : public Packaged {
  public:
	String full_name;				///< Name of this game for menus etc.
	String icon_filename;			///< Filename of icon to use in NewWindow
	OptionalScript init_script;		///< Script of variables available to other scripts in this game
	vector<FieldP> set_fields;		///< Fields for set information
	vector<FieldP> card_fields;		///< Fields on each card
	
	/// Loads the game with a particular name, for example "magic"
	static GameP byName(const String& name);
	
	/// Is this Magic the Gathering?
	bool isMagic() const;
	
	static String typeNameStatic();
	virtual String typeName() const;
	virtual String fullName() const;
	virtual InputStreamP openIconFile();
	
  protected:
	void validate();
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
