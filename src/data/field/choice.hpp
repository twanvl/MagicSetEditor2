//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_FIELD_CHOICE
#define HEADER_DATA_FIELD_CHOICE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/defaultable.hpp>
#include <data/field.hpp>
#include <data/font.hpp>
#include <gfx/gfx.hpp> // for ImageCombine
#include <script/scriptable.hpp>
#include <script/image.hpp>

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
	typedef intrusive_ptr<Choice> ChoiceP;
	
	ChoiceP choices;				///< A choice group of possible choices
	OptionalScript script;			///< Script to apply to all values
	OptionalScript default_script;	///< Script that generates the default value
	String initial;					///< Initial choice of a new value, or ""
	String default_name;			///< Name of "default" value
	map<String,Color> choice_colors;			///< Colors for the various choices (when color_cardlist)
	map<String,Color> choice_colors_cardlist;	///< Colors for the various choices, for in the card list
	
	virtual void initDependencies(Context&, const Dependency&) const;
	
  private:
	DECLARE_REFLECTION();
};


enum ChoiceChoiceType {
	CHOICE_TYPE_CHECK,
	CHOICE_TYPE_RADIO
};

/// An item that can be chosen for this field
class ChoiceField::Choice : public IntrusivePtrBase<ChoiceField::Choice> {
  public:
	Choice();
	Choice(const String& name);
	
	String           name;			///< Name/value of the item
	String           default_name;	///< A default item, if this is a group and default_name.empty() there is no default
	vector<ChoiceP>  choices;		///< Choices and sub groups in this group
	bool             line_below;	///< Show a line after this item?
	Scriptable<bool> enabled;		///< Is this item enabled?
	ChoiceChoiceType type;			///< How should this item be shown, only for multiple choice fields
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
{	RENDER_TEXT            = 0x01	// render the name as text
,	RENDER_IMAGE           = 0x10	// render an image
,	RENDER_HIDDEN          = 0x20	// don't render anything, only have a menu
,	RENDER_CHECKLIST       = 0x100	// render as a checklist, intended for multiple choice
,	RENDER_LIST            = 0x200	// render as a list of images/text, intended for multiple choice
,	RENDER_BOTH            = RENDER_TEXT      | RENDER_IMAGE
,	RENDER_HIDDEN_IMAGE    = RENDER_HIDDEN    | RENDER_IMAGE
,	RENDER_TEXT_CHECKLIST  = RENDER_CHECKLIST | RENDER_TEXT
,	RENDER_IMAGE_CHECKLIST = RENDER_CHECKLIST | RENDER_IMAGE
,	RENDER_BOTH_CHECKLIST  = RENDER_CHECKLIST | RENDER_BOTH
,	RENDER_TEXT_LIST       = RENDER_LIST      | RENDER_TEXT
,	RENDER_IMAGE_LIST      = RENDER_LIST      | RENDER_IMAGE
,	RENDER_BOTH_LIST       = RENDER_LIST      | RENDER_BOTH
};

enum ThumbnailStatus
{	THUMB_NOT_MADE
,	THUMB_OK
,	THUMB_CHANGED
};

/// The Style for a ChoiceField
class ChoiceStyle : public Style {
  public:
	ChoiceStyle(const ChoiceFieldP& field);
	DECLARE_STYLE_TYPE(Choice);
	~ChoiceStyle();
	
	ChoicePopupStyle            popup_style;        ///< Style of popups/menus
	ChoiceRenderStyle           render_style;       ///< Style of rendering
	Font                        font;               ///< Font for drawing text (when RENDER_TEXT)
	ScriptableImage             image;				///< Image to draw (when RENDER_IMAGE)
	map<String,ScriptableImage> choice_images;      ///< Images for the various choices (when RENDER_IMAGE)
	bool                        choice_images_initialized;
	Scriptable<String>          mask_filename;      ///< Filename of an additional mask over the images
	ImageCombine                combine;            ///< Combining mode for drawing the images
	Alignment                   alignment;          ///< Alignment of images
	Image                       mask;               ///< The actual mask image
	int                         angle;				///< Angle by which the images are rotated
	wxImageList*                thumbnails;         ///< Thumbnails for the choices
	vector<ThumbnailStatus>     thumbnails_status;	///< Which thumbnails are up to date?
	
	/// Load the mask image, if it's not already done
	void loadMask(Package& pkg);
	/// Initialize image from choice_images
	void initImage();
	
	virtual bool update(Context&);
	virtual void initDependencies(Context&, const Dependency&) const;
	virtual void invalidate(Context&);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ChoiceValue

/// The Value in a ChoiceField
class ChoiceValue : public Value {
  public:
	/// Create a value for the given field
	/** If initial_first_choice then the first choice should be used in the absence of
	    an explicit initial value
	 */
	ChoiceValue(const ChoiceFieldP& field, bool initial_first_choice = true);
	DECLARE_HAS_FIELD(Choice)
	
	typedef Defaultable<String> ValueType;
	ValueType value;	/// The name of the selected choice
	
	virtual ValueP clone() const;
	virtual String toString() const;
	virtual bool update(Context&);
	
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
