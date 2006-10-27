//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_CHOICE
#define HEADER_DATA_FIELD_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <data/field.hpp>
#include <gfx/gfx.hpp> // for ImageCombine
#include <script/scriptable.hpp>

// ----------------------------------------------------------------------------- : ChoiceField

DECLARE_POINTER_TYPE(ChoiceField);
DECLARE_POINTER_TYPE(ChoiceStyle);
DECLARE_POINTER_TYPE(ChoiceValue);

/// A field that contains a list of choices
class ChoiceField : public Field {
  public:
	ChoiceField();
	DECLARE_FIELD_TYPE(Choice);
	
	class Choice;
	typedef shared_ptr<Choice> ChoiceP;
	
	ChoiceP choices;				///< A choice group of possible choices
	OptionalScript script;			///< Script to apply to all values
	OptionalScript default_script;	///< Script that generates the default value
	String initial;					///< Initial choice of a new value, or ""
	String default_name;			///< Name of "default" value
		
  private:
	DECLARE_REFLECTION();
};

/// An item that can be chosen for this field
class ChoiceField::Choice {
  public:
	Choice();
	Choice(const String& name);
	
	String          name;			///< Name/value of the item
	String          default_name;	///< A default item, if this is a group and default_name.empty() there is no default
	vector<ChoiceP> choices;		///< Choices and sub groups in this group
	/// First item-id in this group (can be the default item)
	/** Item-ids are consecutive integers, a group uses all ids [first_id..lastId()).
	 *  The top level group has first_id 0.
	 */
	int             first_id;
		
	/// Is this a group?
	bool isGroup() const;
	/// Can this Choice itself be chosen?
	/** For a single choice this is always true, for a group only if it has a default choice */
	bool hasDefault() const;
	
	/// Initialize the first_id of children
	/** @pre first_id is set
	 *  Returns lastId()
	 */
	int initIds();
	/// Number of choices in this group (and subgroups), 1 if it is not a group
	/** The default choice also counts */
	int choiceCount() const;
	/// item-id just beyond the end of this group
	int lastId() const;
	
	/// item-id of a choice, given the internal name
	/** If the id is not in this group, returns -1 */
	int choiceId(const String& name) const;
	/// Internal name of a choice
	/** The internal name is formed by concatenating the names of all parents, separated by spaces.
	 *  Returns "" if id is not in this group
	 */
	String choiceName(int id) const;
	/// Formated name of a choice.
	/** Intended for use in menu structures, so it doesn't include the group name for children.
	 *  Returns "" if id is not in this group.
	 */
	String choiceNameNice(int id) const;
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ChoiceStyle

// How should the menu for a choice look?
enum ChoicePopupStyle
{	POPUP_MENU
,	POPUP_DROPDOWN
,	POPUP_DROPDOWN_IN_PLACE
};
// How should a choice value be rendered?
enum ChoiceRenderStyle
{	RENDER_TEXT   = 0x01		// render the name as text
,	RENDER_IMAGE  = 0x10		// render an image
,	RENDER_BOTH   = RENDER_TEXT | RENDER_IMAGE
,	RENDER_HIDDEN = 0x20		// don't render anything, only have a menu
,	RENDER_HIDDEN_IMAGE = RENDER_HIDDEN | RENDER_IMAGE
};

/// The Style for a ChoiceField
class ChoiceStyle : public Style {
  public:
	ChoiceStyle(const ChoiceFieldP& field);
	DECLARE_STYLE_TYPE(Choice);
	
	ChoicePopupStyle			popup_style;	///< Style of popups/menus
	ChoiceRenderStyle			render_style;	///< Style of rendering
//	FontInfo					font;			///< Font for drawing text (when RENDER_TEXT)
//	map<String,ScriptableImage>	choice_images;	///< Images for the various choices (when RENDER_IMAGE)
	map<String,Color>			choice_colors;	///< Colors for the various choices (when color_cardlist)
	bool						colors_card_list;///< Does this field determine colors of the rows in the card list?
	String						mask_filename;	///< Filename of an additional mask over the images
	ImageCombine				combine;		///< Combining mode for drawing the images
	Alignment					alignment;		///< Alignment of images
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ChoiceValue

/// The Value in a ChoiceField
class ChoiceValue : public Value {
  public:
	inline ChoiceValue(const ChoiceFieldP& field) : Value(field) {}
	DECLARE_HAS_FIELD(Choice)
	
	Defaultable<String> value;	/// The name of the selected choice
	
	virtual String toString() const;
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
