//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_EXPORT_TEMPLATE
#define HEADER_DATA_EXPORT_TEMPLATE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package.hpp>
#include <script/scriptable.hpp>

DECLARE_POINTER_TYPE(Game);
DECLARE_POINTER_TYPE(Field);
DECLARE_POINTER_TYPE(Style);

// ----------------------------------------------------------------------------- : ExportTemplate

/// A template for exporting sets to HTML or text format
class ExportTemplate : public Packaged {
  public:
	ExportTemplate();
	
	GameP                   game;				///< Game this template is for
	String                  file_type;			///< Type of the created file, in "name|*.ext" format
	bool                    create_directory;	///< The export creates an entire directory
	vector<FieldP>          option_fields;		///< Options for exporting
	IndexMap<FieldP,StyleP> option_style;		///< Style of the options
	OptionalScript          script;				///< Export script, for multi file templates and initialization
	
	static String typeNameStatic();
	virtual String typeName() const;
  private:
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : ExportPackage

/// A package that is being written to when exporting
class ExportingPackage : public Package {
};

// ----------------------------------------------------------------------------- : EOF
#endif
