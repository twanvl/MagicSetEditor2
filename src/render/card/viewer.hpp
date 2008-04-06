//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
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
	DataViewer();
	~DataViewer();
	
	// --------------------------------------------------- : Drawing
	
	/// Draw the current (card/data) to the given dc
	virtual void draw(DC& dc);
	/// Draw the current (card/data) to the given dc
	virtual void draw(RotatedDC& dc, const Color& background);
	/// Draw a single viewer
	virtual void drawViewer(RotatedDC& dc, ValueViewer& v);
	
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
	/// Should focus only editing specific things be drawn?
	/** false by default, can be overloaded */
	virtual bool drawFocus() const;
	/// Pens for drawing field borders (only called if drawBorders())
	virtual wxPen borderPen(bool active) const;
	/// The viewer that is currently focused, may be null
	/** null by default, can be overloaded */
	virtual ValueViewer* focusedViewer() const;
	/// Get a script context to use for scripts in the viewers
	Context& getContext() const;
	/// The rotation to use
	virtual Rotation getRotation() const;
	/// The card we are viewing, can be null
	inline const CardP& getCard() const { return card; }
	/// Invalidate and redraw (the area of) a single value viewer
	virtual void redraw(const ValueViewer&) {}
	
	// --------------------------------------------------- : Setting data
	
	/// Display a card in this viewer
	/** \param refresh: Always refresh, even if this card is already shown */
	void setCard(const CardP& card, bool refresh = false);
	
	/// Clear data
	virtual void onChangeSet();
	
	// --------------------------------------------------- : The viewers
  private:
	/// Create some viewers for the given styles
	void addStyles(IndexMap<FieldP,StyleP>& styles);
	/// Update style scripts
	void updateStyles(bool only_content_dependent);
  protected:
	/// Set the styles for the data to be shown, recreating the viewers
	void setStyles(const StyleSheetP& stylesheet, IndexMap<FieldP,StyleP>& styles, IndexMap<FieldP,StyleP>* extra_styles = nullptr);
	/// Set the data to be shown in the viewers, refresh them
	void setData(IndexMap<FieldP,ValueP>& values, IndexMap<FieldP,ValueP>* extra_values = nullptr);
	
	/// Create a viewer for the given style.
	/** Can be overloaded to create a ValueEditor instead */
	virtual ValueViewerP makeViewer(const StyleP&);
	
	/// Update the viewers and forward actions
	virtual void onAction(const Action&, bool undone);
	
	/// Notification that the total image has changed
	virtual void onChange() {}
	/// Notification that the viewers are initialized
	virtual void onInit() {}
	/// Notification that the size of the viewer may have changed
	virtual void onChangeSize() {}
	
	vector<ValueViewerP> viewers;	///< The viewers for the different values in the data
	CardP card;						///< The card that is currently displayed, if any
	bool drawing;					///< Are we currently drawing?
  public:
	mutable StyleSheetP stylesheet;	///< Stylesheet being used
};

// ----------------------------------------------------------------------------- : EOF
#endif
