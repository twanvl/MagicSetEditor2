# Generate a header and source file

$file  = $ARGV[0];
$macro = uc $file;
$macro =~ s@/@_@g;

# read templates

open F, "../src/code_template.hpp";
$hpp = join('',<F>);
close F;

open F, "../src/code_template.cpp";
$cpp = join('',<F>);
close F;

# insert stuff

$hpp =~ s/_\r?\n/_$macro\n/g;

$cpp =~ s@<util/prec.hpp>@$&\n#include <$file.hpp>@g;

# write files

if (-e "../src/$file.hpp" or -e "../src/$file.cpp") {
	die "The output files already exist!";
}

open F, "> ../src/$file.hpp";
print F $hpp;
close F;

open F, "> ../src/$file.cpp";
print F $cpp;
close F;