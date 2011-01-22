;//+----------------------------------------------------------------------------+
;//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
;//| Copyright:    (C) 2001 - 2011 Twan van Laarhoven and "coppro"              |
;//| License:      GNU General Public License 2 or later (see file COPYING)     |
;//+----------------------------------------------------------------------------+

; Note: This installer requires the Inno Setup Preprocessor add on

#ifndef INSTALL_ALL
  #error installer.iss must not be used directly, use either installer-large or installer-small
#endif

#define Debug

[setup]
AppName                 = Magic Set Editor 2
AppVerName              = Magic Set Editor 2.0.0
AppCopyright            = Copyright © 2001-2011 Twan van Laarhoven and "coppro"
DefaultDirName          = {pf}\Magic Set Editor 2
DisableStartupPrompt    = 1
DisableProgramGroupPage = 1
AllowNoIcons            = 0
ChangesAssociations     = 1
WizardImageBackColor    = $FFF7F0
BackColor               = $FFF7F0
BackColor2              = $FFF7F0
;LicenseFile             = COPYING
OutputDir               = tools/msw-installer/
WizardImageFile         = tools/msw-installer/WizModernImage.bmp
WizardSmallImageFile    = tools/msw-installer/WizModernSmallImage.bmp
SourceDir               = ../..
PrivilegesRequired      = none

; Filename of the installer
#define INSTALLER_DATE GetDateTimeString('yyyy-mm-dd','-',':')
#if INSTALL_ALL
  #define INSTALLER_SUFFIX '-full'
#else
  #define INSTALLER_SUFFIX '-reduced'
#endif
#emit 'OutputBaseFilename = mse2-' + INSTALLER_DATE + INSTALLER_SUFFIX


[Types]
Name: "full";       Description: "Full installation"
Name: "magic";      Description: "Only Magic"
Name: "vs";         Description: "Only VS System"
Name: "yugioh";     Description: "Only Yu-Gi-Oh!"
Name: "custom";     Description: "Custom installation"; Flags: iscustom


[Components]
; Note: The following line does nothing other than provide a visual cue
; to the user that the program files are installed no matter what.
Name: "prog";                      Description: "MSE Program Files";   Flags: fixed; Types: full custom magic vs yugioh
Name: "prog/en";                   Description: "English translation"; Flags: fixed; Types: full custom magic vs yugioh
#if INSTALL_ALL && 0
  ; there are no good translations
  Name: "prog/de";                   Description: "German translation";                Types: full
  Name: "prog/fr";                   Description: "French translation";                Types: full
  Name: "prog/es";                   Description: "Spanish translation";               Types: full
  Name: "prog/it";                   Description: "Italian translation";               Types: full
  Name: "prog/cht";                  Description: "Chinese traditional translation";   Types: full
  Name: "prog/chs";                  Description: "Chinese simplified translation";    Types: full
#endif
Name: "style";                     Description: "Templates";                         Types: full custom magic vs yugioh
Name: "style/mtg";                 Description: "Magic the Gathering";               Types: full custom magic
Name: "style/mtg/new";             Description: "Modern style, after 8th edition";   Types: full custom magic
Name: "style/mtg/new/base";        Description: "Normal cards";                      Types: full custom magic
Name: "style/mtg/new/flip";        Description: "Flip cards";                        Types: full custom magic
Name: "style/mtg/new/split";       Description: "Split cards";                       Types: full custom magic
#if INSTALL_ALL
  Name: "style/mtg/new/promo";       Description: "Promotional cards";                 Types: full custom magic
  Name: "style/mtg/new/extart";      Description: "Extended art style";                Types: full custom magic
#endif
Name: "style/mtg/new/textless";    Description: "Textless cards";                    Types: full custom magic
#if INSTALL_ALL
  Name: "style/mtg/new/token";       Description: "Tokens";                            Types: full custom magic
  Name: "style/mtg/new/planeshift";  Description: "Planeshifted";                      Types: full custom magic
  Name: "style/mtg/new/walker";      Description: "Planeswalkers";                     Types: full custom magic
  Name: "style/mtg/new/leveler";     Description: "Levelers";                          Types: full custom magic
#endif
Name: "style/mtg/old";             Description: "Old style, before 8th edition";     Types: full custom magic
Name: "style/mtg/old/base";        Description: "Normal cards";                      Types: full custom magic
Name: "style/mtg/old/split";       Description: "Split cards";                       Types: full custom magic
#if INSTALL_ALL
  Name: "style/mtg/old/token";       Description: "Tokens";                            Types: full custom magic
  Name: "style/mtg/fpm";             Description: "Fire Penguin Master (FPM)";         Types: full custom magic
  Name: "style/mtg/fpm/base";        Description: "Normal cards";                      Types: full custom magic
  Name: "style/mtg/fpm/flip";        Description: "Flip cards";                        Types: full custom magic
  Name: "style/mtg/fpm/split";       Description: "Split cards";                       Types: full custom magic
  Name: "style/mtg/fpm/promo";       Description: "Promotional cards";                 Types: full custom magic
  Name: "style/mtg/fpm/token";       Description: "Tokens";                            Types: full custom magic
  Name: "style/mtg/future";          Description: "Future sight";                      Types: full custom magic
  Name: "style/mtg/future/base";     Description: "Normal cards";                      Types: full custom magic
  Name: "style/mtg/future/split";    Description: "Split cards";                       Types: full custom magic
  Name: "style/mtg/future/textless"; Description: "Textless cards";                    Types: full custom magic
  Name: "style/mtg/counter";         Description: "Counters";                          Types: full custom magic
  Name: "style/mtg/vanguard";        Description: "Vanguard";                          Types: full custom magic
#endif
Name: "style/vs";                  Description: "VS System";                         Types: full custom vs
Name: "style/vs/std";              Description: "Standard style";                    Types: full custom vs
#if INSTALL_ALL
  Name: "style/vs/ext";              Description: "Extended art promo";                Types: full custom vs
  Name: "style/vs/hstd";             Description: "Hellboy style";                     Types: full custom vs
  Name: "style/vs/hext";             Description: "Hellboy extended art";              Types: full custom vs
  Name: "style/vs/alter";            Description: "Alter Ego";                         Types: full custom vs
  Name: "style/vs/new";              Description: "New style";                         Types: full custom vs
#endif
Name: "style/yugioh";              Description: "Yu-Gi-Oh!";                         Types: full custom yugioh

; (indented lines are full installer only)

[Files]

; ------------------------------------------------------------------------- : Program files

; program
Source: "build/Release Unicode/mse.exe";  DestDir: "{app}";                      Components: prog; Flags: replacesameversion
Source: "build/Release Unicode/mse.com";  DestDir: "{app}";                      Components: prog; Flags: replacesameversion

; locales: en
Source: "data/en.mse-locale/*";           DestDir: "{app}/data/en.mse-locale/";  Components: prog/en; Flags: recursesubdirs
#if INSTALL_ALL && 0
  Source: "data/de.mse-locale/*";           DestDir: "{app}/data/de.mse-locale/";  Components: prog/de; Flags: recursesubdirs
  Source: "data/fr.mse-locale/*";           DestDir: "{app}/data/fr.mse-locale/";  Components: prog/fr; Flags: recursesubdirs
  Source: "data/it.mse-locale/*";           DestDir: "{app}/data/it.mse-locale/";  Components: prog/it; Flags: recursesubdirs
  Source: "data/es.mse-locale/*";           DestDir: "{app}/data/es.mse-locale/";  Components: prog/es; Flags: recursesubdirs
  Source: "data/cht.mse-locale/*";          DestDir: "{app}/data/cht.mse-locale/"; Components: prog/cht; Flags: recursesubdirs
  Source: "data/chs.mse-locale/*";          DestDir: "{app}/data/chs.mse-locale/"; Components: prog/chs; Flags: recursesubdirs
  ;Source: "data/jp.mse-locale/*";           DestDir: "{app}/data/jp.mse-locale/";  Components: prog/jp; Flags: recursesubdirs
#endif

; dictionaries: en
Source: "data/dictionaries/en_US.dic";      DestDir: "{app}/data/dictionaries/";  Components: prog/en
Source: "data/dictionaries/en_US.aff";      DestDir: "{app}/data/dictionaries/";  Components: prog/en


; ------------------------------------------------------------------------- : Style packages

; ----------------------------- : Utilities

; Declare a (style) package that must be installed
#define Package(large_only, base,name,type,component) \
          INSTALL_ALL || !large_only ? \
            'Source:        "data/' + base + (name == '' ? '' : '-'+name) + '.mse-' + type + '/*"; ' + \
            'DestDir: "{app}/data/' + base + (name == '' ? '' : '-'+name) + '.mse-' + type + '/";  ' + \
            'Components: style/' + component + ';' + \
            'Flags: recursesubdirs' \
          : ''

; Declare a font that must be installed
#define Font(large_only, file,name,component)                 \
          INSTALL_ALL || !large_only ?                        \
            'Source:  "tools/msw-installer/font/'+file+'";' + \
            'DestDir: "{fonts}";'                           + \
            'FontInstall: "'+name+'";'                      + \
            'Components: style/'+component+';'              + \
            'Flags: onlyifdoesntexist uninsneveruninstall'    \
          : ''
; Declare a font that must be installed, but not registered
#define FontNoReg(large_only, file,name,component)            \
          INSTALL_ALL || !large_only ?                        \
            'Source:  "tools/msw-installer/font/'+file+'";' + \
            'DestDir: "{fonts}";'                           + \
            'Components: style/'+component+';'              + \
            'Flags: onlyifdoesntexist uninsneveruninstall'    \
          : ''

; ----------------------------- : Magic

#emit Package(0, 'magic', '',                'game',            'mtg')
#emit Package(0, 'magic', 'blends',          'include',         'mtg')
#emit Package(0, 'magic', 'default-image',   'include',         'mtg')
#emit Package(0, 'magic', 'watermarks',      'include',         'mtg')
#emit Package(0, 'magic', 'future-common',   'include',         'mtg')
#emit Package(0, 'magic', 'spoiler',         'export-template', 'mtg')
#emit Package(0, 'magic', 'forum',           'export-template', 'mtg')
#emit Package(0, 'magic', 'mana-small',      'symbol-font',     'mtg')
#emit Package(0, 'magic', 'mana-large',      'symbol-font',     'mtg')
#emit Package(1, 'magic', 'mana-beveled',    'symbol-font',     'mtg/fpm')
#emit Package(1, 'magic', 'mana-future',     'symbol-font',     'mtg/future')
#emit Package(0, 'magic', 'new',             'style',           'mtg/new/base')
#emit Package(0, 'magic', 'new-flip',        'style',           'mtg/new/flip')
#emit Package(0, 'magic', 'new-split',       'style',           'mtg/new/split')
#emit Package(1, 'magic', 'new-promo',       'style',           'mtg/new/promo')
#emit Package(1, 'magic', 'extended-art',    'style',           'mtg/new/extart')
#emit Package(0, 'magic', 'textless',        'style',           'mtg/new/textless')
#emit Package(1, 'magic', 'new-token',       'style',           'mtg/new/token')
#emit Package(1, 'magic', 'embossedletters', 'symbol-font',     'mtg/new/token')
#emit Package(1, 'magic', 'planeshifted',    'style',           'mtg/new/planeshift')
#emit Package(1, 'magic', 'new-planeswalker','style',           'mtg/new/walker')
#emit Package(1, 'magic', 'new-leveler',     'style',           'mtg/new/leveler')
#emit Package(0, 'magic', 'old',             'style',           'mtg/old/base')
;#emit Package(0, 'magic', 'old-flip',        'style',           'mtg/old/flip')
#emit Package(0, 'magic', 'old-split',       'style',           'mtg/old/split')
#emit Package(1, 'magic', 'old-token',       'style',           'mtg/old/token')
#define fpm 'firepenguinmaster'
#emit Package(1, 'magic', fpm,               'style',           'mtg/fpm/base')
#emit Package(1, 'magic', fpm+'-flip',       'style',           'mtg/fpm/flip')
#emit Package(1, 'magic', fpm+'split',       'style',           'mtg/fpm/split')
#emit Package(1, 'magic', fpm+'promo',       'style',           'mtg/fpm/promo')
#emit Package(1, 'magic', fpm+'tokens',      'style',           'mtg/fpm/token')
#emit Package(1, 'magic', 'future',          'style',           'mtg/future/base')
#emit Package(1, 'magic', 'future-split',    'style',           'mtg/future/split')
#emit Package(1, 'magic', 'future-textless', 'style',           'mtg/future/textless')
#emit Package(1, 'magic', 'counter',         'style',           'mtg/counter')

#emit Font   (0, 'ModMatrix.ttf',   'ModMatrix',                'mtg')
#emit Font   (0, 'matrixb.ttf',     'Matrix',                   'mtg style/yugioh')
#emit Font   (0, 'matrixbsc.ttf',   'MatrixBoldSmallCaps',      'mtg')
#emit Font   (0, 'magmed.ttf',      'MagicMedieval',            'mtg/old')
#emit Font   (0, 'mplantin.ttf',    'MPlantin',                 'mtg')
#emit Font   (0, 'mplantinit.ttf',  'MPlantin-Italic',          'mtg')

#emit Package(1, 'vanguard', '',         'game',   'mtg/vanguard')
#emit Package(1, 'vanguard', 'standard', 'style',  'mtg/vanguard')

; ----------------------------- : VS System

#emit Package(0, 'vs', '',                 'game',            'vs')
#emit Package(0, 'vs', 'common',           'include',         'vs')
#emit Package(0, 'vs', 'standard-arrow',   'symbol-font',     'vs')
#emit Package(0, 'vs', 'standard-official','symbol-font',     'vs')
#emit Package(0, 'vs', 'spoiler',          'export-template', 'vs')
#emit Package(0, 'vs', 'standard',         'style',           'vs/std')
#emit Package(1, 'vs', 'extended-art',     'style',           'vs/ext')
#emit Package(1, 'vs', 'hellboy',          'style',           'vs/hstd')
#emit Package(1, 'vs', 'extended-hellboy', 'style',           'vs/hext')
#emit Package(1, 'vs', 'alter',            'style',           'vs/alter')
#emit Package(1, 'vs', 'new',              'style',           'vs/new')
#emit Package(1, 'vs', 'standard-new',     'symbol-font',     'vs/new')

#emit Font   (0, 'BadhouseBoldNumbers.ttf', 'BadhouseBoldNumbers',    'vs')
#emit Font   (0, 'eurosti.ttf',             'Eurostile',              'vs')
#emit Font   (0, 'eurostile.oblique.ttf',   'EurostileObl-Normal',    'vs')
#emit Font   (0, 'percexptm.ttf',           'Percolator Expert TM',   'vs')
#emit Font   (1, 'GILC____.TTF',            'Gill Sans MT Condensed', 'vs/new')

; ----------------------------- : YuGiOh

#emit Package(0, 'yugioh', '',                  'game',        'yugioh')
#emit Package(0, 'yugioh', 'standard-levels',   'symbol-font', 'yugioh')
#emit Package(0, 'yugioh', 'text-replacements', 'symbol-font', 'yugioh')
#emit Package(0, 'yugioh', 'standard',          'style',       'yugioh')

#emit FontNoReg (0, 'MatrixRegularSmallCaps.pfm',   'MatrixRegularSmallCaps',  'yugioh')
#emit FontNoReg (0, 'MatrixRegularSmallCaps.pfb',   'MatrixRegularSmallCaps',  'yugioh')
#emit Font      (0, 'pala.ttf',                     'Palatino Linotype',       'yugioh')
#emit Font      (0, 'palab.ttf',                    'Palatino Linotype Bold',  'yugioh')
#emit Font      (0, 'MatriBoo.ttf',                 'MatrixBook',              'yugioh')

; ------------------------------------------------------------------------- : Rest of installer

[Icons]
Name: "{commonprograms}\Magic Set Editor"; Filename: "{app}\mse.exe"; WorkingDir: "{app}"

[Registry]
#pragma parseroption -p-
#define Association(ext, name, icon) \
    'Root: HKCR; Subkey: "'+ext+'";                                       ValueType: string; ValueName: ""; ValueData: "MagicSetEditor2'+name+'";   Flags: uninsdeletevalue;\n' + \
    'Root: HKCR; Subkey: "MagicSetEditor2'+name+'";                       ValueType: string; ValueName: ""; ValueData: "Magic Set Editor '+name+'"; Flags: uninsdeletekey;\n' + \
    'Root: HKCR; Subkey: "MagicSetEditor2'+name+'\\DefaultIcon";          ValueType: string; ValueName: ""; ValueData: "{app}\\mse.exe,'+icon+'";\n' + \
    'Root: HKCR; Subkey: "MagicSetEditor2'+name+'\\shell\\open\\command"; ValueType: string; ValueName: ""; ValueData: """{app}\\mse.exe"" ""%1""";'

#emit Association('.mse-set',       'Set',       '2')
#emit Association('.mse-symbol',    'Symbol',    '3')
#emit Association('.mse-installer', 'Installer', '1')

[Run]
Filename: "{app}\mse.exe"; Description: "Start Magic Set Editor"; Flags: postinstall nowait skipifsilent unchecked

; ------------------------------ : Uninstaller

[UninstallDelete]
Type: filesandordirs; Name: "{userappdata}\Magic Set Editor"

; Debugging
#ifdef Debug
  #expr SaveToFile(AddBackslash(SourcePath) + "Preprocessed.iss")
#endif

