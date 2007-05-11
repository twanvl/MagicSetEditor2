;//+----------------------------------------------------------------------------+
;//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
;//| Copyright:    (C) 2001 - 2005 Twan van Laarhoven                           |
;//| License:      GNU General Public License 2 or later (see file COPYING)     |
;//+----------------------------------------------------------------------------+

[setup]
AppName                 = Magic Set Editor 2
AppVerName              = Magic Set Editor 2 - 0.3.2 beta
AppCopyright            = Copyright © 2001-2006 Twan van Laarhoven
DefaultDirName          = {pf}\Magic Set Editor 2
DisableStartupPrompt    = 1
DisableProgramGroupPage = 1
AllowNoIcons            = 0
ChangesAssociations     = 1
WizardImageBackColor    = $FFF7F0
BackColor               = $FFF7F0
BackColor2              = $FFF7F0
InfoBeforeFile          = tools/msw-installer/beta-readme.rtf
;LicenseFile             = COPYING
OutputBaseFilename      = mse2-
OutputDir               = tools/msw-installer/
WizardImageFile         = tools/msw-installer/WizModernImage.bmp
WizardSmallImageFile    = tools/msw-installer/WizModernSmallImage.bmp
SourceDir               = ../..


[Types]
Name: "full";       Description: "Full installation"
Name: "magic";      Description: "Only Magic"
Name: "vs";         Description: "Only VS System"
Name: "yugioh";     Description: "Only Yu-Gi-Oh!"
Name: "custom";     Description: "Custom installation"; Flags: iscustom


[Components]
; Note: The following line does nothing other than provide a visual cue
; to the user that the program files are installed no matter what.
Name: "prog";                    Description: "MSE Program Files";   Flags: fixed; Types: full custom magic vs yugioh
Name: "style";                   Description: "Templates";                         Types: full custom magic vs yugioh
Name: "style/mtg";               Description: "Magic the Gathering";               Types: full custom magic
Name: "style/mtg/new";           Description: "Modern style, after 8th edition";   Types: full custom magic
Name: "style/mtg/new/base";      Description: "Normal cards";                      Types: full custom magic
Name: "style/mtg/new/flip";      Description: "Flip cards";                        Types: full custom magic
Name: "style/mtg/new/split";     Description: "Split cards";                       Types: full custom magic
Name: "style/mtg/new/promo";     Description: "Promotional cards";                 Types: full custom magic
Name: "style/mtg/new/textless";  Description: "Texttless cards";                   Types: full custom magic
Name: "style/mtg/new/extart";    Description: "Extended art style";                Types: full custom magic
Name: "style/mtg/new/planeshift";Description: "Planeshifted";                      Types: full custom magic
Name: "style/mtg/old";           Description: "Old style, before 8th edition";     Types: full custom magic
Name: "style/mtg/old/base";      Description: "Normal cards";                      Types: full custom magic
Name: "style/mtg/old/split";     Description: "Split cards";                       Types: full custom magic
Name: "style/mtg/old/token";     Description: "Tokens";                            Types: full custom magic
Name: "style/mtg/vanguard";      Description: "Vanguard";                          Types: full custom magic
Name: "style/vs";                Description: "VS System";                         Types: full custom vs
Name: "style/vs/std";            Description: "Standard style";                    Types: full custom vs
Name: "style/vs/ext";            Description: "Extended art promo";                Types: full custom vs
Name: "style/yugioh";            Description: "Yu-Gi-Oh!";                         Types: full custom yugioh

[Files]

; ------------------------------ : Program

; program
Source: "build/Release Unicode/mse.exe";                        DestDir: "{app}";                                           Components: prog; Flags: replacesameversion
Source: "tools/msw-installer/msvcr71.dll";                      DestDir: "{sys}";                                           Components: prog; Flags: restartreplace sharedfile uninsneveruninstall onlyifdoesntexist

; locale : en
Source: "data/en.mse-locale/*";                                 DestDir:    "{app}/data/en.mse-locale/";                    Components: prog; Flags: recursesubdirs

; ------------------------------ : Magic

; Basic
Source: "data/magic.mse-game/*";                                DestDir:    "{app}/data/magic.mse-game/";                   Components: style/mtg;
Source: "data/magic.mse-game/stats/*";                          DestDir:    "{app}/data/magic.mse-game/stats/";             Components: style/mtg;
Source: "data/magic-blends.mse-include/*";                      DestDir:    "{app}/data/magic-blends.mse-include/";         Components: style/mtg;
Source: "data/magic-default-image.mse-include/*";               DestDir:    "{app}/data/magic-default-image.mse-include/";  Components: style/mtg;
Source: "data/magic-watermarks.mse-include/*";                  DestDir:    "{app}/data/magic-watermarks.mse-include/";     Components: style/mtg;
Source: "data/magic-mana-small.mse-symbol-font/*";              DestDir:    "{app}/data/magic-mana-small.mse-symbol-font/"; Components: style/mtg;
Source: "data/magic-mana-large.mse-symbol-font/*";              DestDir:    "{app}/data/magic-mana-large.mse-symbol-font/"; Components: style/mtg;


; style : magic-new
Source: "data/magic-new.mse-style/*";                           DestDir:    "{app}/data/magic-new.mse-style/";              Components: style/mtg/new/base;
; style : magic-new-flip
Source: "data/magic-new-flip.mse-style/*";                      DestDir:    "{app}/data/magic-new-flip.mse-style/";         Components: style/mtg/new/flip;
; style : magic-new-split
Source: "data/magic-new-split.mse-style/*";                     DestDir:    "{app}/data/magic-new-split.mse-style/";        Components: style/mtg/new/split;
; style : magic-new-promo
Source: "data/magic-new-promo.mse-style/*";                     DestDir:    "{app}/data/magic-new-promo.mse-style/";        Components: style/mtg/new/promo;

; style : magic-textless
Source: "data/magic-textless.mse-style/*";                      DestDir:    "{app}/data/magic-textless.mse-style/";         Components: style/mtg/new/textless;
; style : magic-extended-art
Source: "data/magic-extended-art.mse-style/*";                  DestDir:    "{app}/data/magic-extended-art.mse-style/";     Components: style/mtg/new/extart;

; style : magic-planeshifted
Source: "data/magic-planeshifted.mse-style/*";                  DestDir:    "{app}/data/magic-planeshifted.mse-style/";     Components: style/mtg/new/planeshift;

; style : magic-old
Source: "data/magic-old.mse-style/*";                           DestDir:    "{app}/data/magic-old.mse-style/";              Components: style/mtg/old/base;
; style : magic-old-split
Source: "data/magic-old-split.mse-style/*";                     DestDir:    "{app}/data/magic-old-split.mse-style/";        Components: style/mtg/old/split;
; style : magic-old-token
Source: "data/magic-old-token.mse-style/*";                     DestDir:    "{app}/data/magic-old-token.mse-style/";        Components: style/mtg/old/token;

; export : magic-test
;Source: "data/magic-test.mse-export-template/export-template";  DestDir:   "{app}/data/magic-test.mse-export-template/";    Components: style/mtg;

; fonts
Source: "tools/msw-installer/font/matrixb.ttf";    DestDir: "{fonts}"; FontInstall: "Matrix";              Components: style/mtg/new;  Flags: onlyifdoesntexist uninsneveruninstall
Source: "tools/msw-installer/font/magmed.ttf";     DestDir: "{fonts}"; FontInstall: "MagicMedieval";       Components: style/mtg/old;  Flags: onlyifdoesntexist uninsneveruninstall
Source: "tools/msw-installer/font/mplantin.ttf";   DestDir: "{fonts}"; FontInstall: "MPlantin";            Components: style/mtg;      Flags: onlyifdoesntexist uninsneveruninstall
Source: "tools/msw-installer/font/mplantinit.ttf"; DestDir: "{fonts}"; FontInstall: "MPlantin-Italic";     Components: style/mtg;      Flags: onlyifdoesntexist uninsneveruninstall

; ------------------------------ : Vanguard

; game : vanguard
Source: "data/vanguard.mse-game/*";                             DestDir:    "{app}/data/vanguard.mse-game/";                Components: style/mtg/vanguard;
; style : vanguard-standard
Source: "data/vanguard-standard.mse-style/*";                   DestDir:    "{app}/data/vanguard-standard.mse-style/";      Components: style/mtg/vanguard;

; ------------------------------ : VS System

; Basic : vs
Source: "data/vs.mse-game/*";                                   DestDir:    "{app}/data/vs.mse-game/";                      Components: style/vs;
Source: "data/vs-standard-arrow.mse-symbol-font/*";             DestDir:    "{app}/data/vs-standard-arrow.mse-symbol-font/";Components: style/vs

; style : vs-standard
Source: "data/vs-standard.mse-style/*";                         DestDir:    "{app}/data/vs-standard.mse-style/";            Components: style/vs/std;

; style : vs-extended
Source: "data/vs-extended-art.mse-style/*";                     DestDir:    "{app}/data/vs-extended-art.mse-style/";        Components: style/vs/ext;

; fonts
Source: "tools/msw-installer/font/dirtyheadline.ttf"; DestDir: "{fonts}"; FontInstall: "Dirty Headline";      Components: style/vs;  Flags: onlyifdoesntexist uninsneveruninstall
Source: "tools/msw-installer/font/eurosti.ttf";       DestDir: "{fonts}"; FontInstall: "Eurostile";           Components: style/vs;  Flags: onlyifdoesntexist uninsneveruninstall
Source: "tools/msw-installer/font/percexp.ttf";       DestDir: "{fonts}"; FontInstall: "Percolator Expert";   Components: style/vs;  Flags: onlyifdoesntexist uninsneveruninstall


; ------------------------------ : Yu-Gi-Oh

; game : yugioh
Source: "data/yugioh.mse-game/*";                               DestDir:    "{app}/data/yugioh.mse-game/";                       Components: style/yugioh

; style : yugioh-standard
Source: "data/yugioh-standard.mse-style/*";                     DestDir:    "{app}/data/yugioh-standard.mse-style/";             Components: style/yugioh

; font : yugioh-standard-levels
Source: "data/yugioh-standard-levels.mse-symbol-font/*";        DestDir:    "{app}/data/yugioh-standard-levels.mse-symbol-font/";Components: style/yugioh


; ------------------------------ : Rest of installer

[Icons]
Name: "{commonprograms}\Magic Set Editor"; Filename: "{app}\mse.exe"; WorkingDir: "{app}"


[Registry]
; .mse-set file association
Root: HKCR; Subkey: ".mse-set";                                 ValueType: string; ValueName: ""; ValueData: "MagicSetEditor2Set";   Flags: uninsdeletevalue
Root: HKCR; Subkey: "MagicSetEditor2Set";                       ValueType: string; ValueName: ""; ValueData: "Magic Set Editor Set"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MagicSetEditor2Set\DefaultIcon";           ValueType: string; ValueName: ""; ValueData: "{app}\mse.exe,3"
Root: HKCR; Subkey: "MagicSetEditor2Set\shell\open\command";    ValueType: string; ValueName: ""; ValueData: """{app}\mse.exe"" ""%1"""
; .mse-symbol file association
Root: HKCR; Subkey: ".mse-symbol";                              ValueType: string; ValueName: ""; ValueData: "MagicSetEditor2Symbol";   Flags: uninsdeletevalue
Root: HKCR; Subkey: "MagicSetEditor2Symbol";                    ValueType: string; ValueName: ""; ValueData: "Magic Set Editor Symbol"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MagicSetEditor2Symbol\DefaultIcon";        ValueType: string; ValueName: ""; ValueData: "{app}\mse.exe,5"
Root: HKCR; Subkey: "MagicSetEditor2Symbol\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\mse.exe"" ""%1"""



[Run]
Filename: "{app}\mse.exe"; Description: "Start Magic Set Editor"; Flags: postinstall nowait skipifsilent unchecked

