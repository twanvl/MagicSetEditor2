//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_IO_WRITER
#define HEADER_UTIL_IO_WRITER

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <wx/txtstrm.h>

// ----------------------------------------------------------------------------- : Writer

typedef wxOutputStream  OutputStream;
typedef shared_ptr<wxOutputStream> OutputStreamP;

/// The Writer can be used for writing (serializing) objects
class Writer {
  public:
	/// Construct a writer that writes to the given output stream
	Writer(const OutputStreamP& output);
	
	/// Tell the reflection code we are not reading
	inline bool reading() const { return false; }
	
	// --------------------------------------------------- : Handling objects
	/// Handle an object: write it under the given name
	template <typename T>
	void handle(const Char* name, const T& object) {
		enterBlock(name);
		handle(object);
		exitBlock();
	}
	
	/// Write a string to the output stream
	void handle(const String& str);
	void handle(const Char* str) { handle(String(str)); }
	
	/// Write an object of type T to the output stream
	template <typename T> void handle(const T& object);
	/// Write a vector to the output stream
	template <typename T> void handle(const vector<T>& vector);
	/// Write a shared_ptr to the output stream
	template <typename T> void handle(const shared_ptr<T>& pointer);
	/// Write a map to the output stream
	//template <typename K, typename V> void handle(map<K,V>& map);
	
  private:
	// --------------------------------------------------- : Data
	/// Indentation of the current block
	int indentation;
	/// Did we just open a block (i.e. not written any lines of it)?
	bool justOpened;
	/// Last key opened
	String openedKey;
	
	/// Output stream we are writing to
	OutputStreamP output;
	/// Text stream wrapping the output stream
	wxTextOutputStream stream;
	
	// --------------------------------------------------- : Writing to the stream
	
	/// Start a new block with the given name
	void enterBlock(const Char* name);
	/// Leave the block we are in
	void exitBlock();
	
	/// Write the openedKey and the required indentation
	void writeKey();
	/// Output some taps to represent the indentation level
	void writeIndentation();
};

// ----------------------------------------------------------------------------- : Container types

template <typename T>
void Writer::handle(const vector<T>& vector) {
	/*String vectorKey = key;
	while (key == vectorKey) { // TODO : check indent
		moveNext(); // skip key
		vector.resize(vector.size() + 1);
		handle(vector.back());
	}*/
}

template <typename T>
void Writer::handle(const shared_ptr<T>& pointer) {
	if (pointer) handle(*pointer);
}

// ----------------------------------------------------------------------------- : Reflection

/// Implement reflection as used by Writer
#define REFLECT_OBJECT_WRITER(Cls)								\
	template<> void Writer::handle<Cls>(const Cls& object) {	\
		const_cast<Cls&>(object).reflect(*this);				\
	}

// ----------------------------------------------------------------------------- : Reflection for enumerations

/// Implement enum reflection as used by Writer
#define REFLECT_ENUM_WRITER(Enum)								\
	template<> void Writer::handle<Enum>(const Enum& enum_) {	\
		EnumWriter writer(*this);								\
		reflect_ ## Enum(const_cast<Enum&>(enum_), writer);		\
	}

/// 'Tag' to be used when reflecting enumerations for Writer
class EnumWriter {
  public:
	inline EnumWriter(Writer& writer)
		: writer(writer) {}
	
	/// Handle a possible value for the enum, if the name matches the name in the input
	template <typename Enum>
	inline void handle(const Char* name, Enum value, Enum enum_) {
		if (enum_ == value) {
			writer.handle(name);
		}
	}
	
  private:
	Writer& writer;  ///< The writer to write output to
};

// ----------------------------------------------------------------------------- : EOF
#endif
