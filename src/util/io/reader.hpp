//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2008 Twan van Laarhoven and "coppro"              |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_READER
#define HEADER_UTIL_IO_READER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/version.hpp>

template <typename T> class Defaultable;
template <typename T> class Scriptable;
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
class Packaged;

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
  private:
	/// Construct a reader that reads a file in a package
	/** Used for "include file" keys.
	 *  package can be nullptr
	 */
	Reader(Reader* parent, Packaged* package, const String& filename, bool ignore_invalid = false);
  public:
	/// Construct a reader that reads from the given input stream
	/** filename is used only for error messages
	 */
	Reader(const InputStreamP& input, Packaged* package = nullptr, const String& filename = wxEmptyString, bool ignore_invalid = false);
	
	~Reader() { showWarnings(); }
	
	/// Tell the reflection code we are reading
	inline bool reading() const { return true; }
	/// Tell the reflection code we are not related to scripting
	inline bool scripting() const { return false; }
	/// Is the thing currently being read 'complex', i.e. does it have children
	inline bool isComplex() const { return value.empty(); }
	/// Add a as an alias for b, all keys a will be replaced with b, only if file_app_version < end_version
	void addAlias(Version end_version, const Char* a, const Char* b);
	/// Ignore old keys
	void handleIgnore(int, const Char*);
	
	/// Read and check the application version
	void handleAppVersion();
	
	/// Add a warning message, but continue reading
	void warning(const String& msg, int line_number_delta = 0, bool warn_on_previous_line = true);
	/// Show all warning messages, but continue reading
	void showWarnings();
	
	// --------------------------------------------------- : Handling objects
	/// Handle an object that can read as much as it can eat
	template <typename T>
	void handle_greedy(T& object) {
		do {
			handle(object);
			if (state != HANDLED) unknownKey(object);
			state = OUTSIDE;
		} while (indent >= expected_indent);
	}
	
	/// Handle an object: read it if it's name matches
	template <typename T>
	void handle(const Char* name, T& object) {
		if (enterBlock(name)) {
			handle_greedy(object);
			exitBlock();
		}
	}
	/// Handle a value
	template <typename T>
	inline void handleNoScript(const Char* name, T& value) { handle(name,value); }
	
	/// Reads a vector from the input stream
	template <typename T>
	void handle(const Char* name, vector<T>& vector);
	
	/// Reads an object of type T from the input stream
	template <typename T> void handle(T&);
	/// Reads a intrusive_ptr from the input stream
	template <typename T> void handle(intrusive_ptr<T>&);
	/// Reads a map from the input stream
	template <typename V> void handle(map<String,V>&);
	/// Reads an IndexMap from the input stream, reads only keys that already exist in the map
	template <typename K, typename V> void handle(IndexMap<K,V>&);
	template <typename K, typename V> void handle(DelayedIndexMaps<K,V>&);
	template <typename K, typename V> void handle(DelayedIndexMapsData<K,V>&);
	/// Reads a Defaultable from the input stream
	template <typename T> void handle(Defaultable<T>&);
	/// Reads a Scriptable from the input stream
	template <typename T> void handle(Scriptable<T>&);
	// special behaviour
	void handle(GameP&);
	void handle(StyleSheetP&);
	
	/// Indicate that the last value from getValue() was not handled, allowing it to be handled again
	void unhandle();
	
	/// The package being read from
	inline Packaged* getPackage() const { return package; }
	
	// --------------------------------------------------- : Data
	/// App version this file was made with
	Version file_app_version;
  private:
	/// The line we read
	String line;
	/// The key and value of the last line we read
	String key, value;
	/// Value of the *previous* line, only valid in state==HANDLED
	String previous_value;
	/// Indentation of the last line we read
	int indent;
	/// Indentation of the block we are in
	int expected_indent;
	/// State of the reader
	enum State {
		OUTSIDE,		///< We have not entered the block of the current key
		ENTERED,		///< We just entered the block of the current key
		HANDLED,		///< We have handled a value, and moved to the next line, previous_value is the value we just handled
		UNHANDLED,		///< Something has been 'unhandled()'
	} state;
	/// Aliasses for compatability
	struct Alias {
		String  new_key;
		Version end_version;
	};
	/// Aliasses for compatability
	map<String, Alias> aliasses;
	/// Should all invalid keys be ignored?
	bool ignore_invalid;
	
	/// Filename for error messages
	String filename;
	/// Package this file is from, if any
	Packaged* package;
	/// Line number of the current line for error messages
	int line_number;
	/// Line number of the previous_line
	int previous_line_number;
	/// Input stream we are reading from
	InputStreamP input;
	/// Accumulated warning messages
	String warnings;
	
	// --------------------------------------------------- : Reading the stream
	
	/// Is there a block with the given key under the current cursor? if so, enter it
	bool enterBlock(const Char* name);
	/// Enter any block, no matter what the key
	bool enterAnyBlock();
	/// Leave the block we are in
	void exitBlock();
	
	/// Move to the next non empty line
	void moveNext();
	/// Reads the next line from the input, and stores it in line/key/value/indent
	void readLine(bool in_string = false);
	
	/// Return the value on the current line
	const String& getValue();
	
	/// No line was read, because nothing mathes the current key
	/** Maybe the key is "include file" */
	template <typename T>
	void unknownKey(T& v) {
		if (key == _("include file")) {
			Reader reader(this, package, value, ignore_invalid);
			reader.handle_greedy(v);
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
intrusive_ptr<T> read_new(Reader& reader) {
	return new_intrusive<T>();
}

/// Update the 'index' member of a value for use by IndexMap
template <typename T> void update_index(T&, size_t index) {}

template <typename T>
void Reader::handle(const Char* name, vector<T>& vector) {
	String vectorKey = singular_form(name);
	while (enterBlock(vectorKey)) {
		vector.resize(vector.size() + 1);
		handle_greedy(vector.back());
		update_index(vector.back(), vector.size() - 1); // update index for IndexMap
		exitBlock();
	}
}

template <typename T>
void Reader::handle(intrusive_ptr<T>& pointer) {
	if (!pointer) pointer = read_new<T>(*this);
	handle(*pointer);
}

template <typename V>
void Reader::handle(map<String, V>& m) {
	while (enterAnyBlock()) {
		handle_greedy(m[key]);
		exitBlock();
	}
}

template <typename K, typename V>
void Reader::handle(IndexMap<K,V>& m) {
	for (typename IndexMap<K,V>::iterator it = m.begin() ; it != m.end() ; ++it) {
		handle(get_key_name(*it).c_str(), *it);
	}
}

// ----------------------------------------------------------------------------- : Reflection

/// Implement reflection as used by Reader
#define REFLECT_OBJECT_READER(Cls)								\
	template<> void Reader::handle<Cls>(Cls& object) {			\
		object.reflect(*this);									\
	}															\
	void Cls::reflect(Reader& reader) {							\
		reflect_impl(reader);									\
	}

// ----------------------------------------------------------------------------- : Reflection for enumerations

/// Implement enum reflection as used by Reader
#define REFLECT_ENUM_READER(Enum)								\
	template<> void Reader::handle<Enum>(Enum& enum_) {			\
		EnumReader reader(getValue());							\
		reflect_ ## Enum(enum_, reader);						\
		if (!reader.isDone()) {									\
			/* warning: unknown value */						\
			warning(_ERROR_1_("unrecognized value", value));	\
		}														\
	}															\
	bool parse_enum(const String& value, Enum& out) {			\
		EnumReader reader(value);								\
		reflect_ ## Enum(out, reader);							\
		return reader.isDone();									\
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
