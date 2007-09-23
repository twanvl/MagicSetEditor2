#!/usr/bin/perl
#
#Perl script to create updates.
#Just run with first argument being the folder all the installers are in, and the others being the packages
# e.g. ./create-package-list.pl http://magicseteditor.sf.net/packages/ ../../data/*

$url = shift;

while ($ARGV = shift) {
	$f = $ARGV =~ /(([-a-z]+).mse-(game|style|symbol-font|include|export-template|locale))/;
	if (!$f) {
		warn "$ARGV not an appropriate package.";
		next;
	}

	$fullname = $1;
	$name = $2;

	open(FILE, "$ARGV/$3");

	$version = $msever = $dependencies = "";

	while (<FILE>) {
		$version = $1 if /^(?:\xef\xbb\xbf)?version: (.*)$/;
		$msever = $1 if /^(?:\xef\xbb\xbf)?mse[ _]version: (.*)$/;
		while (/^(?:\xef\xbb\xbf)?depends[ _]on:\s*$/) {
			$dep = $depver = "";
			while (<FILE>) {
				$version = $1 if /^(?:\xef\xbb\xbf)?version: (.*)$/;
				$msever = $1 if /^(?:\xef\xbb\xbf)?mse[ _]version: (.*)$/;
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
	}

	close(FILE);

	if (!$version || !$msever) {
		warn "$ARGV does not have a version" unless $version;
		warn "$ARGV does not have an application version" unless $msever;
		next;
	}

	print "package:\n\tname: $fullname\n\turl: $url$name.mse-installer\n\tversion: $version\n\tapp version: $msever\n$dependencies\tdescription:\n\n";
}