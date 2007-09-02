//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_RENDER_TEXT_VIEWER
#define HEADER_RENDER_TEXT_VIEWER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/rotation.hpp>
#include <data/field/text.hpp>
#include <render/text/element.hpp>

// ----------------------------------------------------------------------------- : TextViewer

/// Class for viewing and determining positions in formated text
/** It can:
 *  - Draw text to a DC
 *  - Draw selection to a DC
 *  - Convert between screen coordinates and cursor position
 *
 * To refer to positions in the text this class uses several concepts:
 *  - index     An index in the input string.
 *              For cursor positions char_id = 0 means the cursor is BEFORE character 0
 *  - pos       The position of a character in real world x,y coordinates (in pixels)
 *              The position is the top-left of the character.
 *              A char_pos is often only the x coordinate.
 *              A line_pos is often only the y coordinate.
 *  - Line      A line on the screen, this does not neccessarly correspond to explicit linebreaks,
 *              since textwrapping also leads to a new line.
 *  - line_id   The index of a line, 0 is the first line.
 */
class TextViewer {
  public:
	/// Information on a line in the textbox
	struct Line;
	
	TextViewer();
	~TextViewer();
	
	// --------------------------------------------------- : Drawing
	
	/// Draw the given text with the given text style
	/** The drawing information is cached,
	 *  before calling draw again with different text/style reset() should be called
	 */
	void draw(RotatedDC& dc, const TextStyle& style, DrawWhat what);
	/// Draw an indicator for selected text
	void drawSelection(RotatedDC& dc, const TextStyle& style, size_t sel_start, size_t sel_end);
	/// Draw separators for <line> tags
	void drawSeparators(RotatedDC& dc);
	
	/// Prepare the text for drawing, if it is not already prepared
	/** Returns true if something has been done */
	bool prepare(RotatedDC& dc, const String& text, TextStyle& style, Context&);
	/// Reset the cached data, at a new call to draw it will be recalculated
	/** If related, the new value is related to the old one, and layout information should be reused where possible
	 */
	void reset(bool related);
	/// Is the viewer prepare()d?
	bool prepared() const;
	
	// --------------------------------------------------- : Positions
	
	/// Find the character index that is on a line above/below index
	/** If this would move outisde the text, returns the input index */
	size_t moveLine(size_t index, int delta) const;
	
	/// The character index of the start of the line that character #index is on
	size_t lineStart(size_t index) const;
	/// The character index past the end of the line that character #index is on
	size_t lineEnd  (size_t index) const;
	
	/// Find the index of the character at the given position
	/** If the position is before everything returns 0,
	 *  if it is after everything returns text.size().
	 *  The position is in internal coordinates */
	size_t indexAt(const RealPoint& pos) const;
	/// Find the position of the character at the given index
	/** The position is in internal coordinates */
	RealPoint posOf(size_t index) const;
	
	/// Return the rectangle around a single character
	RealRect charRect(size_t index) const;
	/// Is the character at the given index visible?
	bool isVisible(size_t index) const;
	/// Find the first character index that is at/before/after the given index, and which has a nonzero width
	/** More precisely: it returns a position so that the character after it in the direction delta has nonzero width
	 */
	size_t firstVisibleChar(size_t index, int delta) const;
	
	/// Return the height of the last line
	double heightOfLastLine() const;
	
	// --------------------------------------------------- : Lines/scrolling
	
	/// The total number of lines
	size_t lineCount() const;
	/// number of fully visible lines, height gives the height of the box
	size_t visibleLineCount(double height) const;
	/// the index of the first visible line
	size_t firstVisibleLine() const;
	// scroll so line_id becomes the first visible line
	void scrollTo(size_t line_id);
	/// Ensure the specified character is fully visible
	/*  Always scrolls by a whole line.
	 *  Returns true if the editor has scrolled.
	 */
	bool ensureVisible(double height, size_t char_id);
	
	/// Get exact scroll position
	double getExactScrollPosition() const;
	/// Set exact scroll position
	void setExactScrollPosition(double pos);
	
  private:
	/// Scroll all lines a given amount
	void scrollBy(double delta);
	
  private:
	// --------------------------------------------------- : More drawing
	double scale;    ///< Scale when drawing
	bool justifying; ///< Is text justified?
	
	// --------------------------------------------------- : Elements
	TextElements elements; ///< The elements of the prepared text
	
	/// Find the elements in a string and add them to elements
	void prepareElements(const String&, const TextStyle& style, Context& ctx);
	
	// --------------------------------------------------- : Lines
	vector<Line> lines; ///< The lines in the text box
	
	/// Prepare the lines, layout the text
	void prepareLines(RotatedDC& dc, const String& text, TextStyle& style, Context& ctx);
	/// Find the scale to use for the text
	void prepareLinesTryScales(RotatedDC& dc, const String& text, const TextStyle& style, vector<CharInfo>& chars_out);
	/// Prepare the lines, layout the text; at a specific scale
	/** Stores output in lines_out */
	bool prepareLinesScale(RotatedDC& dc, const vector<CharInfo>& chars, const TextStyle& style, bool stop_if_too_long, vector<Line>& lines_out) const;
	/// Align the lines within the textbox
	void alignLines(RotatedDC& dc, const vector<CharInfo>& chars, const TextStyle& style);
	/// Align the lines of a single paragraph (a set of lines)
	void alignParagraph(size_t start_line, size_t end_line, const vector<CharInfo>& chars, const TextStyle& style, const RealRect& box);
	
	/// Find the line the given index is on, returns the first line if the index is not found
	const Line& findLine(size_t index) const;
	
	// helper : get the start coordinate of a line, this is 0 unless there is a contour mask
	double lineLeft (RotatedDC& dc, const TextStyle& style, double y) const;
	// helper : get the end coordinate of a line, this is width unless there is a contour mask
	double lineRight(RotatedDC& dc, const TextStyle& style, double y) const;
};


// ----------------------------------------------------------------------------- : EOF
#endif
