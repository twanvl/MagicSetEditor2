#! /usr/bin/perl

@html = <../../doc/doxygen/*.html>;

# for each html file
foreach $file (@html) {
	# read
	open F, "< $file";
	$_ = join('',<F>);
	close F;
	
	# cleanup : globals heading
	
	# in globals?
	$in_globals = m{<li id="current"><a href="globals.html"><span>Globals</span></a></li>};
	$glob_cur = $in_globals ? ' id="current"' : '';
	
	# in files
	$in_files = !$in_globals && m{<li id="current"><a href="files.html"><span>Files</span></a></li>};
	$files_cur = $in_files ? ' id="current"' : '';
	
	# add link to top level
	s {<li( id="current")?><a href="files.html"><span>Files</span></a></li>}
		{<li$glob_cur><a href="globals.html"><span>Globals</span></a></li>
		 <li$files_cur><a href="files.html"><span>Files</span></a></li>
		};
	
	# remove second level bar
	if ($in_globals  || $in_files) {
	  s {(<div class="tabs">.*?</div>)(.*?<div class="tabs">.*?</div>)}
	    {$1}s;
	}
	
	# write
	open F, "> $file";
	print F $_;
	close F;
}