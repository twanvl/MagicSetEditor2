//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/keyword.hpp>
#include <util/tagged_string.hpp>

class KeywordTrie;
DECLARE_TYPEOF2(map<Char, KeywordTrie*>);

// ----------------------------------------------------------------------------- : Reflection

IMPLEMENT_REFLECTION(KeywordParam) {
	REFLECT(name);
	REFLECT(description);
	REFLECT(match);
	REFLECT(script);
}
IMPLEMENT_REFLECTION(KeywordMode) {
	REFLECT(name);
	REFLECT(description);
}
IMPLEMENT_REFLECTION(KeywordExpansion) {
	REFLECT(match);
	REFLECT(reminder);
}

// backwards compatability
template <typename T> void read_compat(T&, const Keyword*) {}
void read_compat(Reader& tag, Keyword* k) {
	String separator, parameter, reminder;
	REFLECT(separator);
	REFLECT(parameter);
	REFLECT(reminder);
	if (!separator.empty() || !parameter.empty() || !reminder.empty()) {
		// old style keyword declaration, no separate expansion
		KeywordExpansionP e(new KeywordExpansion);
		e->match = k->keyword;
		size_t start = separator.find_first_of('[');
		size_t end   = separator.find_first_of(']');
		if (start != String::npos && end != String::npos) {
			e->match += separator.substr(start + 1, end - start - 1);
		}
		if (!parameter.empty()) {
			e->match += _("<param>") + parameter + _("</param>");
		}
		e->reminder.set(reminder);
		k->expansions.push_back(e);
	}
}

IMPLEMENT_REFLECTION(Keyword) {
	REFLECT(keyword);
	read_compat(tag, this);
	REFLECT(expansions);
	REFLECT(rules);
	REFLECT(mode);
}



// ----------------------------------------------------------------------------- : KeywordTrie

/// A node in a trie to match keywords
class KeywordTrie {
  public:
	KeywordTrie();
	~KeywordTrie();
	
	map<Char, KeywordTrie*> children;    ///< children after a given character (owned)
	KeywordTrie*            on_any_star; ///< children on /.*/ (owned)
	Keyword*		        finished;    ///< keywords that end in this node
	
	/// Insert nodes representing the given string
	/** return the node where the evaluation will be after matching the string */
	KeywordTrie* insert(const String& match);

	/// Insert nodes representing the regex /.*/
	/** return the node where the evaluation will be after matching that regex */
	KeywordTrie* insertAnyStar();
};


KeywordTrie::KeywordTrie()
	: on_any_star(nullptr)
	, finished(nullptr)
{}

KeywordTrie::~KeywordTrie() {
	FOR_EACH(c, children) {
		delete c.second;
	}
	delete on_any_star;
}

KeywordTrie* KeywordTrie::insert(const String& match) {
	KeywordTrie* cur = this;
	FOR_EACH_CONST(c, match) {
		KeywordTrie*& child = cur->children[c];
		if (!child) child = new KeywordTrie;
		cur = child;
	}
	return cur;
}

KeywordTrie* KeywordTrie::insertAnyStar() {
	if (!on_any_star) on_any_star = new KeywordTrie;
	return on_any_star;
}

// ----------------------------------------------------------------------------- : KeywordMatcher

/// State of the matching algorithm
class KeywordMatcher {
  public:
	KeywordMatcher(const String& s);
  private:
	String str;
	size_t pos;
};

// ----------------------------------------------------------------------------- : KeywordDatabase

/// A database of keywords to allow for fast matching
/** NOTE: keywords may not be altered after they are added to the database,
 *  The database should be rebuild.
 */
class KeywordDatabase {
  public:
	/// Add a keyword to be matched
	void addKeyword(const Keyword&);
	
	/// Find the first matching keyword, return its position
	size_t firstMatch(const String& input, Keyword* keyword);
	
  private:
	KeywordTrie root;
};

void KeywordDatabase::addKeyword(const Keyword& kw) {
	// TODO
}

// ----------------------------------------------------------------------------- : Using keywords

KeywordDatabaseP new_keyword_database() {
	return new_shared<KeywordDatabase>();
}
void add_keyword(KeywordDatabase& db, const Keyword& kw) {
	db.addKeyword(kw);
}


String expand_keywords(const KeywordDatabase& db, const String& text) {
	// 1. Remove all old reminder texts
	String s = remove_tag_contents(text, _("<atom-keyword>"));
	// 2. Process keywords
	
	// TODO
	
	return s;
}
