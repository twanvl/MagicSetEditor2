//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_READER
#define HEADER_UTIL_IO_READER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/version.hpp>
#include <wx/txtstrm.h>

template <typename T> class Defaultable;
template <typename T> class Scriptable;
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);

// ----------------------------------------------------------------------------- : Reader

typedef wxInputStream  InputStream;
typedef shared_ptr<wxInputStream> InputStreamP;

/// The Reader can be used for reading (deserializing) objects
/** This class makes use of the reflection functionality, in effect
 *  an object tells the Reader what fields it would like to read.
 *  The reader then sees if the requested field is currently available.
 *
 *  The handle functions ensure that afterwards the reader is at the line after the
 *  object that was just read.
 */
class Reader {
  public:
	/// Construct a reader that reads from the given input stream
	/** filename is used only for error messages
	 */
	Reader(const InputStreamP& input, const String& filename = wxEmptyString);
	
	/// Construct a reader that reads a file in a package
	/** Used for "include file" keys. */
	Reader(const String& filename);
	
	~Reader() { showWarnings(); }
	
	/// Tell the reflection code we are reading
	inline bool reading() const { return true; }
	/// Is the thing currently being read 'complex', i.e. does it have children
	inline bool isComplex() const { return value.empty(); }
	/// Add a as an alias for b, all keys a will be replaced with b, only if app_version < end_version
	void addAlias(Version end_version, const Char* a, const Char* b);
	
	/// Read and check the application version
	void handleAppVersion();
	
	/// Add a warning message, but continue reading
	void warning(const String& msg);
	/// Show all warning messages, but continue reading
	void showWarnings();
	
	// --------------------------------------------------- : Handling objects
	/// Handle an object: read it if it's name matches
	template <typename T>
	void handle(const Char* name, T& object) {
		if (enterBlock(name)) {
			handle(object);
			exitBlock();
		}
	}
	/// Reads a vector from the input stream
	template <typename T>
	void handle(const Char* name, vector<T>& vector);
	
	/// Reads an object of type T from the input stream
	template <typename T> void handle(T& object);
	/// Reads a shared_ptr from the input stream
	template <typename T> void handle(shared_ptr<T>& pointer);
	/// Reads a map from the input stream
	template <typename V> void handle(map<String,V>& m);
	/// Reads an IndexMap from the input stream, reads only keys that already exist in the map
	template <typename K, typename V> void handle(IndexMap<K,V>& m);
	/// Reads a Defaultable from the input stream
	template <typename T> void handle(Defaultable<T>&);
	/// Reads a Scriptable from the input stream
	template <typename T> void handle(Scriptable<T>&);
	// special behaviour
	void handle(GameP&);
	void handle(StyleSheetP&);
	
	// --------------------------------------------------- : Data
	/// App version this file was made with
	Version app_version;
  private:
	/// The line we read
	String line;
	/// The key and value of the last line we read
	String key, value;
	/// A string spanning multiple lines
	String multi_line_str;
	/// Indentation of the last line we read
	int indent;
	/// Indentation of the block we are in
	int expected_indent;
	/// Did we just open a block (i.e. not read any more lines of it)?
	bool just_opened;
	/// Aliasses for compatability
	map<String, String> aliasses;
	
	/// Filename for error messages
	String filename;
	/// Line number for error messages
	UInt line_number;
	/// Input stream we are reading from
	InputStreamP input;
	/// Text stream wrapping the input stream
	wxTextInputStream stream;
	/// Accumulated warning messages
	String warnings;
	
	// --------------------------------------------------- : Reading the stream
	
	/// Is there a block with the given key under the current cursor?
	bool enterBlock(const Char* name);
	/// Leave the block we are in
	void exitBlock();
	
	/// Move to the next non empty line
	void moveNext();
	/// Reads the next line from the input, and stores it in line/key/value/indent
	void readLine();
	
	/// No line was read, because nothing mathes the current key
	/** Maybe the key is "include file" */
	template <typename T>
	void unknownKey(T& v) {
		if (key == _("include_file")) {
			Reader reader(value);
			reader.handle(v);
			moveNext();
		} else {
			unknownKey();
		}
	}
	void unknownKey();
};

// ----------------------------------------------------------------------------- : Container types

/// Construct a new type, possibly reading something in the process.
/** By default just creates a new object.
 *  This function can be overloaded to provide different behaviour
 */
template <typename T>
shared_ptr<T> read_new(Reader& reader) {
	return new_shared<T>();
}

/// Update the 'index' member of a value for use by IndexMap
template <typename T> void update_index(T&, size_t index) {}

template <typename T>
void Reader::handle(const Char* name, vector<T>& vector) {
	String vectorKey = singular_form(name);
	while (enterBlock(vectorKey)) {
		vector.resize(vector.size() + 1);
		handle(vector.back());
		update_index(vector.back(), vector.size() - 1); // update index for IndexMap
		exitBlock();
	}
}

template <typename T>
void Reader::handle(shared_ptr<T>& pointer) {
	if (!pointer) pointer = read_new<T>(*this);
	handle(*pointer);
}

template <typename V>
void Reader::handle(map<String, V>& m) {
	while (true) {
		// same as enterBlock
		if (just_opened) moveNext(); // on the key of the parent block, first move inside it
		if (indent != expected_indent) return; // not enough indentation
		just_opened = true;
		expected_indent += 1;
		// now read the value
		handle(m[key]);
		exitBlock();
	}
}

template <typename K, typename V>
void Reader::handle(IndexMap<K,V>& m) {
	do {
		UInt l = line_number;
		for (typename IndexMap<K,V>::iterator it = m.begin() ; it != m.end() ; ++it) {
			handle(get_key_name(*it).c_str(), *it);
		}
		if (l == line_number) unknownKey(m);
	} while (indent >= expected_indent);
}

// ----------------------------------------------------------------------------- : Reflection

/// Implement reflection as used by Reader
#define REFLECT_OBJECT_READER(Cls)								\
	template<> void Reader::handle<Cls>(Cls& object) {			\
		do {													\
			UInt l = line_number;								\
			object.reflect(*this);								\
			if (l == line_number) unknownKey(object);			\
		} while (indent >= expected_indent);					\
	}															\
	void Cls::reflect(Reader& reader) {							\
		reflect_impl(reader);									\
	}

// ----------------------------------------------------------------------------- : Reflection for enumerations

/// Implement enum reflection as used by Reader
#define REFLECT_ENUM_READER(Enum)								\
	template<> void Reader::handle<Enum>(Enum& enum_) {			\
		EnumReader reader(value);								\
		reflect_ ## Enum(enum_, reader);						\
		if (!reader.isDone()) {									\
			/* warning: unknown value */						\
			warning(_("Unrecognized value: ") + value);			\
		}														\
	}

/// 'Tag' to be used when reflecting enumerations for Reader
class EnumReader {
  public:
	inline EnumReader(String read)
		: read(read), first(true), done(false) {}
	
	/// Handle a possible value for the enum, if the name matches the name in the input
	template <typename Enum>
	inline void handle(const Char* name, Enum value, Enum& enum_) {
		if (!done && read == name) {
			done  = true;
			first = false;
			enum_ = value;
		} else if (first) {
			first = false;
			enum_ = value;
		}
	}
	
	inline bool isDone() const { return done; }
	
  private:
	String read;  ///< The string to match to a value name
	bool   first; ///< Has the first (default) value been matched?
	bool   done;  ///< Was anything matched?
};

// ----------------------------------------------------------------------------- : EOF
#endif
