//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_EXPORT_TEMPLATE
#define HEADER_DATA_EXPORT_TEMPLATE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <script/scriptable.hpp>

// ----------------------------------------------------------------------------- : ExportTemplate

/// A template for exporting sets to HTML or text format
class ExportTemplate : public Packaged {
  public:
	
	OptionalScript script;				///< Export script
	String         file_type;			///< Type of the created file, in "name|*.ext" format
	bool           create_directory;	///< The export creates an entire directory
	
	
	static String typeNameStatic();
	virtual String typeName() const;
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif
