//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
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
	void draw(RotatedDC& dc, const String& text, const TextStyle& style, Context&, DrawWhat);
	/// Draw an indicator for selected text
	void drawSelection(RotatedDC& dc, const TextStyle& style, size_t sel_start, size_t sel_end);
	
	/// Reset the cached data, at a new call to draw it will be recalculated
	void reset();
	
	// --------------------------------------------------- : Positions
	
	/// Find the character index that is before the given index, and which has a nonzero width
	size_t moveLeft(size_t index) const;
	/// Find the character index that is on a line above/below index
	/** If this would move outisde the text, returns the input index */
	size_t moveLine(size_t index, int delta) const;
	/// The character index of the start of the line that character #index is on
	size_t lineStart(size_t index) const;
	/// The character index past the end of the line that character #index is on
	size_t lineEnd  (size_t index) const;
	
  private:
	// --------------------------------------------------- : More drawing
	double scale; /// < Scale when drawing
	
	// --------------------------------------------------- : Elements
	TextElements elements; ///< The elements of the prepared text
	
	/// Find the elements in a string and add them to elements
	void prepareElements(const String&, const TextStyle& style, Context& ctx);
	
	// --------------------------------------------------- : Lines
	vector<Line> lines; ///< The lines in the text box
	
	/// Prepare the lines, layout the text
	void prepareLines(RotatedDC& dc, const String& text, const TextStyle& style);
	/// Prepare the lines, layout the text; at a specific scale
	bool prepareLinesScale(RotatedDC& dc, const String& text, const TextStyle& style, bool stop_if_too_long);
	/// Find the line the given index is on, returns the first line if the index is not found
	const Line& findLine(size_t index) const;
	
	// helper : get the start coordinate of a line, this is 0 unless there is a contour mask
	double lineLeft (RotatedDC& dc, const TextStyle& style, double y);
	// helper : get the end coordinate of a line, this is width unless there is a contour mask
	double lineRight(RotatedDC& dc, const TextStyle& style, double y);
};


// ----------------------------------------------------------------------------- : EOF
#endif
