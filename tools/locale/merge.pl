#! /usr/bin/perl

use strict;

# Merge two locale files, by adding to it everythin from the english locale
#
# Usage: perl merge.pl en es

our $dir = "../../data";
my $also_package = 0;  # also DEL in package section?

# Open file
sub load_locale {
	my ($lang) = @_;
	my $filename = "$dir/$lang.mse-locale/locale";
	open F, "< $filename";
	#my @file = <F>;
	my @file;
	# read lines
	my $minimal_indent = "\t\t";
	while (<F>) {
		# previous run of program?
		next if (/^#_ADD ?(.*)/);
		$_ = $1 if (/^#_DEL ?(.*)/s);
		# add to list
		if (/^$minimal_indent(.*)/) {
			my $prev = pop @file;
			push @file, $prev . $_;
		} else {
			push @file, $_;
		}
		if (/^(package|game):/) {
			# The package specific section has indent 3
			$minimal_indent = "\t\t\t";
		}
	}
	# done
	close F;
	return @file;
}

# Write the keys to a temp file
sub write_keys {
	my ($lang,$file) = @_;
	open F, "> $lang.keys.tmp";
	foreach (@$file) {
		my $key = /.*mse version:.*|^[^:#]*:|^\s*#[^:]*/ ? $& : '';
		$key =~ s/TODO.*$//;
		$key =~ s/\s*$//;
		$key =~ s/\s*$//;
		print F "$key\n";
	}
	close F;
}


sub merge_locales {
	my ($lang1,$lang2,$write_back) = @_;
	
	my @file1 = load_locale($lang1);
	my @file2 = load_locale($lang2);
	write_keys($lang1,\@file1);
	write_keys($lang2,\@file2);
	
	my @diff = `diff $lang2.keys.tmp $lang1.keys.tmp -n`;
	
	# write to same file?
	if ($write_back) {
		open OUT, "> $dir/$lang2.mse-locale/locale";
		select OUT;
	}
	
	# Merge files based on diff
	my $pos1 = 0;
	my $pos2 = 0;
	my $pos_out = 0;
	my $allow_del = 1;
	foreach (@diff) {
		if (/^([a|d])([0-9]+) ([0-9]+)/) {
			my $what = $1;
			my $pos  = $what eq 'a' ? $2 : $2-1;
			my $len  = $3;
			# part before
			for(; $pos2 < $pos ; $pos1++, $pos2++, $pos_out++) {
				print $file2[$pos2];
				$allow_del = 0 if !$also_package && $file2[$pos2] =~ /^package:/;
			}
			# change
			if ($what eq 'a') {
				# add
				for(my $k=0 ; $k < $len ; $pos_out++, $pos1++, $k++) {
					my $line = $file1[$pos1];
					$line =~ s/^\xEF\xBB\xBF//; # eat BOM
					$line = "#_ADD" . ($line =~ /^\t/ ? '' : ' ') . $line;
					print $line;
				}
			} else {
				# delete
				for(my $k=0 ; $k < $len ; $pos2++, $k++) {
					my $line = $file2[$pos2];
					if ($allow_del) {
						$line = "#_DEL" . ($line =~ /^\t/ ? '' : ' ') . $line;
						$line =~ s/#_DEL \xEF\xBB\xBF/\xEF\xBB\xBF#_DEL /;
					}
					print $line;
					$allow_del = 0 if !$also_package && $line =~ /^package:/;
				}
			}
		}
	}

	# rest of file2
	for(; $pos2 < scalar(@file2) ; $pos2++) {
		print $file2[$pos2];
	}
	
	if ($write_back) {
		close OUT;
		select STDOUT;
	}
}

# Main thingy
if (scalar(@ARGV) < 2) {
	print "Usage: merge.pl <LANG1> <LANG2> [FLAGS]\n";
	print "    merge two languages to stdout, LANG1 is the master\n\n";
	print "Usage: merge.pl <LANG1> ALL     [FLAGS]\n";
	print "    merge all languages with the master LANG1, modifies files!\n";
	print "Flags:\n";
	print "   --also-package  Also add DEL in the package specific stuff";
	exit;
}

for (my $i = 2 ; $i < scalar(@ARGV) ; ++$i) {
	if ($ARGV[$i] eq '--also-package') {
		$also_package = 1;
	} else {
		die "Unknown flag: ". $ARGV[$i];
	}
}

my $lang1 = $ARGV[0];
my $lang2 = $ARGV[1];

if ($lang2 eq 'ALL') {
	while (<$dir/*.mse-locale/.svn>) {
		if (m|$dir/([a-z]+)|) {
			my $lang2 = $1;
			next if $lang1 eq $lang2;
			print "Converting: $lang2\n";
			merge_locales($lang1, $lang2, 1);
		}
	}
} else {
	merge_locales($lang1, $lang2, 0);
}


