//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_CARD_VIEWER
#define HEADER_RENDER_CARD_VIEWER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <data/set.hpp>

DECLARE_POINTER_TYPE(Style);
DECLARE_POINTER_TYPE(ValueViewer);
class Context;

// ----------------------------------------------------------------------------- : DataViewer

/// A viewer can generate an image of some values, usually a card.
class DataViewer : public SetView {
  public:
	/// Rotation and zoom to use when drawing
//	Rotation rotation;
	
	// --------------------------------------------------- : Drawing
	
	/// Draw the current (card/data) to the given dc
	void draw(DC& dc);
	/// Draw the current (card/data) to the given dc
	virtual void draw(RotatedDC& dc);
	
	// --------------------------------------------------- : Utility for ValueViewers
	
	/// Should the ValueViewers use a platform native look and feel?
	/** false by default, can be overloaded */
	virtual bool nativeLook() const;
	/// Should field borders be drawn?
	/** false by default, can be overloaded */
	virtual bool drawBorders() const;
	/// Should editing specific things be drawn?
	/** false by default, can be overloaded */
	virtual bool drawEditing() const;
	/// Pens for drawing field borders (only called if drawBorders())
	virtual wxPen borderPen(bool active) const;
	/// The viewer that is currently focused, may be null
	/** null by default, can be overloaded */
	virtual ValueViewer* focusedViewer() const;
	/// Get a script context to use for scripts in the viewers
	Context& getContext() const;
	
	// --------------------------------------------------- : Setting data
	
	/// Display a card in this viewer
	void setCard(const CardP& card);
	
	// --------------------------------------------------- : The viewers
  protected:
	/// Set the styles for the data to be shown, recreating the viewers
	void setStyles(IndexMap<FieldP,StyleP>& styles);
	/// Set the data to be shown in the viewers, refresh them
	void setData(IndexMap<FieldP,ValueP>& values);
	
	/// Create a viewer for the given style.
	/** Can be overloaded to create a ValueEditor instead */
	virtual ValueViewerP makeViewer(const StyleP&);
	
	/// Update the viewers and forward actions
	virtual void onAction(const Action&, bool undone);
	
	/// Notification that the total image has changed
	virtual void onChange() {}
	
	/// Notification that the viewers are initialized
	virtual void onInit() {}
	
	vector<ValueViewerP> viewers;	///< The viewers for the different values in the data
	CardP card;						///< The card that is currently displayed, if any
};

// ----------------------------------------------------------------------------- : EOF
#endif
