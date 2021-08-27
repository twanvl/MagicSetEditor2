//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/version.hpp>
#ifdef __APPLE__
    #include <unordered_map>
#endif

template <typename T> class Defaultable;
template <typename T> class Scriptable;
DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(StyleSheet);
class Packaged;
pair<unique_ptr<wxInputStream>, Packaged*> openFileFromPackage(Packaged* package, const String& name);

// ----------------------------------------------------------------------------- : Reader

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
   *  package is used for looking up included files.
   */
  Reader(wxInputStream& input, Packaged* package = nullptr, const String& filename = wxEmptyString, bool ignore_invalid = false);
  
  ~Reader() { showWarnings(); }
  
  /// Tell the reflection code we are reading
  static constexpr bool isReading = true;
  static constexpr bool isWriting = false;
  static constexpr bool isScripting = false;
  /// Is the thing currently being read 'complex', i.e. does it have children
  inline bool isCompound() const { return indent != expected_indent - 1 || value.empty(); }
  /// Ignore old keys
  void handleIgnore(int, const Char*);
  /// Get the version of the format we are reading
  inline Version formatVersion() const { return file_app_version; }
  
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
    handle_greedy_without_validate(object);
    after_reading(object, file_app_version);
  }
  template <typename T>
  void handle_greedy_without_validate(T& object) {
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
  template <typename V> void handle(map<String, V>&);
  template <typename V> void handle(unordered_map<String, V>&);
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
  
private:
  // --------------------------------------------------- : Data
  /// App version this file was made with
  Version file_app_version;
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
    OUTSIDE,    ///< We have not entered the block of the current key
    ENTERED,    ///< We just entered the block of the current key
    HANDLED,    ///< We have handled a value, and moved to the next line, previous_value is the value we just handled
    UNHANDLED,  ///< Something has been 'unhandle()-ed'
  } state;
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
  wxBufferedInputStream input;
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
    if (key == _("include_file")) {
      auto [stream, include_package] = openFileFromPackage(package, value);
      Reader sub_reader(*stream, include_package, value, ignore_invalid);
      if (sub_reader.file_app_version == 0) {
        // in an included file, use the app version of the parent if there is none
        sub_reader.file_app_version = file_app_version;
      }
      sub_reader.handle_greedy_without_validate(v);
      moveNext();
    } else {
      unknownKey();
    }
  }
  void unknownKey();
};

// ----------------------------------------------------------------------------- : After reading hook

// Overload to perform extra stuff after reading
template <typename T> inline void after_reading(T&, Version) {}

template <typename T>
inline void after_reading(intrusive_ptr<T>& x, Version ver) {
  after_reading(*x, ver);
}
inline void after_reading(GameP&, Version) {}
inline void after_reading(StyleSheetP&, Version) {}

// ----------------------------------------------------------------------------- : Container types

/// Construct a new type, possibly reading something in the process.
/** By default just creates a new object.
 *  This function can be overloaded to provide different behaviour
 */
template <typename T>
intrusive_ptr<T> read_new(Reader& reader) {
  return make_intrusive<T>();
}

/// Update the 'index' member of a value for use by IndexMap
template <typename T> void update_index(T&, size_t index) {}

template <typename T>
void Reader::handle(const Char* name, vector<T>& vector) {
  String vectorKey = singular_form(name);
  while (enterBlock(vectorKey.c_str())) {
    T item;
    handle_greedy(item);
    update_index(item, vector.size()); // update index for IndexMap
    vector.emplace_back(std::move(item));
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

template <typename V>
void Reader::handle(unordered_map<String, V>& m) {
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
#define REFLECT_OBJECT_READER(Cls) \
  template<> void Reader::handle<Cls>(Cls& object) { \
    object.reflect(*this); \
  } \
  void Cls::reflect(Reader& reader) { \
    reflect_impl(reader); \
  }

// ----------------------------------------------------------------------------- : Reflection for enumerations

/// Implement enum reflection as used by Reader
#define REFLECT_ENUM_READER(Enum) \
  template<> void Reader::handle<Enum>(Enum& enum_) { \
    EnumReader reader(getValue()); \
    ReflectEnum<Enum>::reflect(enum_, reader); \
    reader.warnIfNotDone(this); \
  } \
  void parse_enum(const String& value, Enum& out) { \
    EnumReader reader(value); \
    ReflectEnum<Enum>::reflect(out, reader); \
    reader.errorIfNotDone(); \
  }

/// 'Handler' to be used when reflecting enumerations for Reader
class EnumReader {
public:
  inline EnumReader(String const& read)
    : read(read), first(nullptr), done(false) {}
  
  /// Handle a possible value for the enum, if the name matches the name in the input
  template <typename Enum>
  inline void handle(const Char* name, Enum value, Enum& enum_) {
    if (!done) {
      if (read == name) {
        done  = true;
        enum_ = value;
      } else if (!first) {
        first = name;
        enum_ = value;
      }
    }
  }
  
  inline bool isDone() const { return done; }
  void warnIfNotDone(Reader* errors_to);
  void errorIfNotDone();
  
private:
  String      read;  ///< The string to match to a value name
  Char const* first; ///< Has the first (default) value been handled? If so, what is its name.
  bool        done;  ///< Was anything matched?
  
  String notDoneErrorMessage() const;
};

