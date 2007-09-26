#!/usr/bin/perl
#
#Perl script to create updates.
#Just run with first argument being the folder all the installers are in, and the others being the packages
# e.g. ./create-package-list.pl http://magicseteditor.sf.net/packages/ ../../data/*

$url = shift;

while ($ARGV = shift) {
	$f = $ARGV =~ /((([a-z]+)[-a-z]*).mse-(game|style|symbol-font|include|export-template|locale))/;
	if (!$f) {
		warn "$ARGV not an appropriate package.";
		next;
	}

	$fullname = $1;
	$name = $2;
	$prefix = $3;
	$type = $4;

	open(FILE, "$ARGV/$type");

	$version = $msever = $dependencies = $shortname = "";

	while (<FILE>) {
		while (/^(?:\xef\xbb\xbf)?depends[ _]on:\s*$/) {
			$dep = $depver = "";
			while (<FILE>) {
				last unless /^\t/;
				$dep = $1 if /^\tpackage: (.*)$/;
				$depver = $1 if /^\tversion: (.*)$/;
			}
			if (!$dep || !$depver) {
				warn "$ARGV has an invalid dependency!";
				next;
			}
			$dependencies .= "\tdepends on:\n\t\tpackage: $dep\n\t\tversion: $depver\n";
		}
		$version = $1 if /^(?:\xef\xbb\xbf)?version: (.*)$/;
		$msever = $1 if /^(?:\xef\xbb\xbf)?mse[ _]version: (.*)$/;
		$shortname = $1 if /^(?:\xef\xbb\xbf)?short[ _]name: (.*)$/;
	}

	close(FILE);

	if (!$version || !$msever) {
		warn "$ARGV does not have a version" unless $version;
		warn "$ARGV does not have an application version" unless $msever;
		next;
	}

	$shortname = $name unless $shortname;

	if ($type ne "locale" && $type ne "game") {
		$packagetype = "$prefix $type";
	} else {
		$packagetype = $type;
	}

	print "package:\n\tname: $fullname\n\ttype: $packagetype\n\turl: $url$name.mse-installer\n\tversion: $version\n\tapp version: $msever\n$dependencies\tdisplay name: $shortname\n\tdescription:\n\n";
}