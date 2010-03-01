//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GUI_SYMBOL_SELECTION
#define HEADER_GUI_SYMBOL_SELECTION

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

class Vector2D;
DECLARE_POINTER_TYPE(Symbol);
DECLARE_POINTER_TYPE(SymbolPart);
DECLARE_POINTER_TYPE(SymbolShape);
DECLARE_POINTER_TYPE(SymbolSymmetry);
class SymbolGroup;

// ----------------------------------------------------------------------------- : Selection

enum SelectMode
{	SELECT_OVERRIDE		// give a completely new selection
,	SELECT_IF_OUTSIDE	// define a new selection if the affected part is not already selected
,	SELECT_TOGGLE		// toggle selection of affected part
};

/// The selected parts of a symbol, enforcing constraints
class SymbolPartsSelection {
  public:
	inline SymbolPartsSelection() : root(nullptr) {}
	
	void setSymbol(const SymbolP& symbol);
	
	/// Clear selection
	void clear();	
	/// Select a part or toggle its selection
	/** Return true if the selection changed */
	bool select(const SymbolPartP& part, SelectMode mode = SELECT_OVERRIDE);
	/// Toggle the selection of the parts in a rectangle (a,b) or (a,c) but not in both
	/** Return true if the selection changed */
	bool selectRect(const Vector2D& a, const Vector2D& b, const Vector2D& c);
	
	/// Find a part by position (not just in the selection!)
	/** Returns SymbolPartP() if nothing is found.
	 *  Does not select inside groups unless the part in question is already selected.
	 */
	SymbolPartP find(const Vector2D& position) const;
	
	/// Get the selection
	inline const set<SymbolPartP>& get() const { return selection; }
	
	/// Is the selection empty?
	inline bool empty() const { return selection.empty(); }
	/// Number of items selected
	inline size_t size() const { return selection.size(); }
	/// Is a part selected?
	inline bool selected(const SymbolPartP& part) const {
		return selection.find(part) != selection.end();
	}
	
	/// Get any SymbolShape if there is one selected
	SymbolShapeP getAShape() const;
	/// Get any SymbolSymmetry if there is one selected
	SymbolSymmetryP getASymmetry() const;
	/// Get the only selected thing
	inline SymbolPartP getOnlyOne() const {
		assert(selection.size() == 1);
		return *selection.begin();
	}
	
  private:
	Symbol* root;
	set<SymbolPartP> selection;
	
	/// Find a part, in some root
	SymbolPartP find(const SymbolPartP& part, const Vector2D& pos) const;
	/// Select rect for some parent
	bool selectRect(const SymbolGroup& parent, const Vector2D& a, const Vector2D& b, const Vector2D& c);
	
	/// Make sure not both a parent and its child/decendant are selected
	void clearChildren (SymbolPart* part);
};

// ----------------------------------------------------------------------------- : EOF
#endif
