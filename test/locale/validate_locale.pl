#! /usr/bin/perl

# Validate a .mse-locale file by checking that the keys match those used in the source code, and that they have the right number of arguments.

use strict;
use File::Find;

# ------------------------------------------------------------------------------
# Step 0: arguments
# ------------------------------------------------------------------------------

if (scalar @ARGV < 1) {
  die "Usage: $0 <SRCDIR> <LOCALE> [<LOCALE>...]"
}
my $indir = shift @ARGV;

# ------------------------------------------------------------------------------
# Step 1: find keys used in source code
# ------------------------------------------------------------------------------

# Determine the keys that should be in the locale file,
# and the number of arguments the keys should have

# Array of locale keys: maps type -> key -> info
# where info is {argc:arity, opt:optional(only used in commented code)}

our %locale_keys;
our $i=0;

sub arg_count {
  return scalar split(/,/,$_[0]);
}

sub make_comment {
  my $input = $_[0];
  $input =~ s/(_[A-Z])/_COMMENT$1/g;
  return $input;
}

sub normalize {
  my $key = shift;
  $key =~ s/_/ /g;
  return $key;
}

# for each .cpp/.hpp file, collect locale calls
sub gather_locale_keys {
  my $filename = $_;
  my $full_name = $File::Find::name;

  if (!($filename =~ /\..pp$/)) {
    return;
  }
  
  # Read file
  open(my $fh, "<", $filename) or die "Failed to open source file $full_name";
  my $body = join('',<$fh>);
  close $fh;
  
  # Custom argument expansion
  my $inparen;
  $inparen = qr/[^()]|\((??{$inparen})*\)/; # recursive paren matching
  $body =~ s/(?:String::Format|format_string)\((_[A-Z]+)(_\([^)]+\)),($inparen+)/
              $1 . "_" . arg_count($3) . $2
            /ge;
  $body =~ s/action_name_for[(][^,]*,\s*(_[A-Z]+)(_\([^)]+\))/$1_1$2/g;
  
  # Drop comments, mark found items as 'optional'
  $body =~ s{//[^\n]*} {find_locale_calls($&, 1)}ge;
  $body =~ s{/\*.*?\*/}{find_locale_calls($&, 1)}ge;
  
  find_locale_calls($body, 0);
}

sub find_locale_calls {
  my $body       = shift;
  my $in_comment = shift;
  
  # Find calls to locale functions
  while ($body =~ /_(COMMENT_)?(MENU|HELP|TOOL|TOOLTIP|LABEL|BUTTON|TITLE|TYPE|ACTION|ERROR)_(?:([1-9])_)?\(\s*\"([^\"]+)\"/g) {
    my $type = $2;
    my $argc = $3 ? $3 : 0;
    my $key = normalize($4);
    if (defined($locale_keys{$type}{$key}{'argc'}) && $locale_keys{$type}{$key}{'argc'} != $argc) {
      die "ERROR: locale key _${type}_($key) used with different arities";
    }
    $locale_keys{$type}{$key}{'opt'}  = defined($locale_keys{$type}{$key}{'opt'}) ? ($locale_keys{$type}{$key}{'opt'} && $in_comment) : $in_comment;
    $locale_keys{$type}{$key}{'argc'} = $argc;
  }
  # addPanel/add_menu_tr/add_tool_tr use multiple locale types
  my $ARG = qr{[^,]+};
  my $STRARG = qr{\s*(?:_\()?\"([^\"]+)\"\)?};
  find_multi_locale_calls($body, $in_comment, qr{addPanel\s*\((?:$ARG,){6} $STRARG \)}x, ["MENU","HELP","TOOL","TOOLTIP"]);
  find_multi_locale_calls($body, $in_comment, qr{menu_item_tr\s*\((?:$ARG,){3} $STRARG }x, ["MENU","HELP"]);
  while ($body =~ m{add_tool_tr\s*\((?:$ARG,){3} $STRARG \s*,?\s*(true|false)?}gx) {
    if ($2 eq 'true') {
      add_locale_keys(["TOOL","TOOLTIP","HELP"], normalize($1), $in_comment);
    } else {
      add_locale_keys(["TOOLTIP","HELP"], normalize($1), $in_comment);
    }
  }
}

sub add_locale_keys {
  my $types = shift;
  my $key = shift;
  my $in_comment = shift;
  foreach my $type (@{$types}) {
    $locale_keys{$type}{$key}{'opt'} = $in_comment;
    $locale_keys{$type}{$key}{'argc'} = 0;
  }
}

sub find_multi_locale_calls {
  my $body       = shift;
  my $in_comment = shift;
  my $re         = shift;
  my $types      = shift;
  while ($body =~ m{$re}g) {
    my $key = normalize($1);
    add_locale_keys($types, $key, $in_comment);
  }
}

#my $filename = 'src/code_template.cpp';
#open(my $fh, "<", $filename) or die "WTF Failed to open source file $filename".length($filename);

find(\&gather_locale_keys, $indir);
if (scalar(%locale_keys) == 0) {
  die "Did not find any source files with locale keys"
}
print $locale_keys{'ERROR'}{'successful instalal'} . "\n";

# ------------------------------------------------------------------------------
# Step 2: validate a locale
# ------------------------------------------------------------------------------

sub parse_locale {
  # Load and parse a .mse-locale file
  # Return a mapping  type -> key -> value
  my $locale_file = shift;
  open(my $fh, "<", "$locale_file/locale") or
    die "Error: locale file not found: $locale_file/locale";
  # Get lines from file
  my $type = undef;
  my $key = undef;
  my %locale;
  for my $line (<$fh>) {
    if ($line =~ /^\s*#|^\s*$/) {
      # comment
    } elsif ($line =~ /^([^:\t]+):/) {
      $type = uc $1;
      $key = undef;
    } elsif ($line =~ /^\t([^:\t]+):(.*)/) {
      $key = normalize($1);
      if (defined($locale{$type}{$key})) {
        die "Locale key already defined: $type: $key";
      }
      $locale{$type}{$key} = $2;
    } elsif ($line =~ /^\t\t(.*)/) {
      $locale{$type}{$key} .= $1;
    } else {
      die "Unknown line in locale file: $line\n";
    }
  }
  close $fh;
  return %locale
}

sub validate_locale {
  my $locale_file = shift;
  my %locale = parse_locale($locale_file);
  # Validate locale:
  #  The set of keys should match exactly, so every key expected by the program should be in the locale file, and vice-versa
  my $ok = 1;
  foreach my $type (sort keys %locale_keys) {
    if (!defined($locale{$type})) {
      print "Missing key in locale: $type\n have keys " . "[" . join(', ', keys %locale) . "]\n";
      $ok = 0;
      next;
    }
    foreach my $key (sort keys %{$locale_keys{$type}}) {
      if (!defined($locale{$type}{$key})) {
        if (!$locale_keys{$type}{$key}{'opt'}) {
          print "Missing key in locale: $type: '$key'\n";
          $ok = 0;
        }
        next;
      }
      # Count the number of printf style arguments in the value
      my @args = $locale{$type}{$key} =~ /%[sd]/g;
      my $argc = scalar(@args);
      if ($argc != $locale_keys{$type}{$key}{'argc'}) {
        print "Incorrect number of arguments for $type: $key. Expected $locale_keys{$type}{$key}{'argc'}, got $argc\n";
        $ok = 0;
      }
    }
  }
  foreach my $type (sort keys %locale) {
    next if $type eq 'PACKAGE'; # Ignore package specific locale keys
    if (!defined($locale_keys{$type})) {
      print "Unknown key in locale: $type\n expected keys " . "[" . join(', ', keys %locale_keys) . "]\n";
      $ok = 0;
      next;
    }
    foreach my $key (sort keys %{$locale{$type}}) {
      if (!defined($locale_keys{$type}{$key})) {
        print "Unknown key in locale: $type: '$key'\n";
        $ok = 0;
      }
    }
  }
  return $ok;
}

if (scalar @ARGV < 1) {
  die "No locales found to validate";
}

for my $locale (@ARGV) {
  if (!validate_locale($locale)) {
    exit 1;
  }
}
