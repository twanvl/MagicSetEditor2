<?php

// Automatic formating of text using the same tags as MediaWiki (wikipedia).

require_once('./modules/mse-drupal-modules/highlight.inc');

// quick and dirty aliasses
global $nice_names;
$nice_names = array(
	'type:double'   => 'real number',
	'type:indexmap' => 'map',
);



/**
 * Format $text, recognizes commands at the start of each line
 */
function autoformat($text, $first = true) {
	global $autoformat_lines;
	// Lines in the input
	$autoformat_lines = is_array($text) ? $text : explode("\n",$text);
	// Result text
	$i = 0;
	return autoformat_handle($i, '', $first);
}

function autoformat_handle(&$i, $prefix, $first, $fail_same = false) {
	global $autoformat_lines;
	$text = '';
	$state = '';
	// While not at the end...
	while ($i < count($autoformat_lines)) {
		$line = $autoformat_lines[$i++];
		
		$len = min(strlen($line),strlen($prefix));
		
		// Match prefix
		for ($j = 0 ; $j < $len ; ++$j) {
			if ($prefix{$j} != $line{$j} && $line{$j} != ' ') {
				$i--;
				if ($state == '|') $text .= autoformat_table($table);
				return $text;
			}
		}
		$oldline = $line;
		$line = substr($line, strlen($prefix));
		// trim
		$current_prefix = $prefix;
		while ($line != '' && $line{0} == ' ') {
			$line = substr($line,1);
			$current_prefix .= ' ';
		}
		
		// Determine line type
		if (strlen($oldline) < strlen($prefix)) {
			// empty line => break out of this level
			if ($state == '|') $text .= autoformat_table($table);
			return $text;
		} else if ($line == '') {
			// empty line => paragraph separator
			if ($state == '|') $text .= autoformat_table($table);
			$state = '';
			$text .= "\n";
		
		// Table
		} elseif ($line{0} == '|' || $line{0} == '!') {
			// other code
			if ($state != '|') $table = array();
			$table[]= array(
				'lines'   => array(substr($line,1)),
				'heading' => $line{0} == '!');
			$state = '|';
		} elseif ($line{0} == "\t" && $state=='|') {
			// continue cell
			$table[count($table)-1]['lines'][] = $line;
		} else {
		
		if ($state == '|') $text .= autoformat_table($table);
		
		// Headings
		if ($prefix == '' && preg_match('@^====.*====$@',$line)) {
			// level 4 heading
			$text .= '<h4>' . autoformat_line(substr($line,4,-4)) . "</h4>\n";
			$state = '';
		} elseif ($prefix == '' && preg_match('@^===.*===$@',$line)) {
			// level 3 heading
			$text .= '<h3>' . autoformat_line(substr($line,3,-3)) . "</h3>\n";
			$state = '';
		} elseif ($prefix == '' && (preg_match('@^==.*==$@',$line) || preg_match('@^--.*--$@',$line))) {
			// level 2 heading
			$text .= '<h2>' . autoformat_line(substr($line,2,-2)) . "</h2>\n";
			$state = '';
		
		// Lists
		} elseif ($line{0} == '*') {
			if ($state == '*') $text = substr($text,0,-5);
			else               $text .= '<ul>';
			$i--;
			$text .= '<li>' . autoformat_handle($i, $current_prefix . '*', false, true) . "</li>\n";
			$text .= '</ul>';
			$state = '*';
		} elseif ($line{0} == '#') {
			if ($state == '#') $text = substr($text,0,-5);
			else               $text .= '<ol>';
			$i--;
			$text .= '<li>' . autoformat_handle($i, $current_prefix . '#', false, true) . "</li>\n";
			$text .= '</ol>';
			$state = '#';
		} elseif ($line{0} == ':') {
			if     ($state == ':') $text = substr($text,0,-10);
			elseif ($state == ';') $text = substr($text,0,-5) . '<dt>';
			else                   $text .= '<dl><dt>';
			$i--;
			$text .= "\n" . autoformat_handle($i, $current_prefix . ':', false);
			$text .= '</dt></dl>';
			$state = ':';
		} elseif ($line{0} == ';') {
			if     ($state == ';') $text = substr($text,0,-10);
			elseif ($state == ':') $text = substr($text,0,-5) . '<dd>';
			else                   $text .= '<dl><dd>';
			$i--;
			$text .= "\n" . autoformat_handle($i, $current_prefix . ';', false);
			$text .= '</dd></dl>';
			$state = ';';
		} elseif ($line{0} == '>') {
			// source code
			if ($state == '>') $text = substr($text,0,-6);
			else               $text .= '<pre>';
			$text .= syntax_highlight(htmlspecialchars(substr($line, 1)));
			$text .= "\n</pre>";
			$state = '>';
		} elseif ($line{0} == ']') {
			// other code
			if ($state == ']') $text = substr($text,0,-6);
			else               $text .= '<pre>';
			$text .= htmlspecialchars(substr($line, 1));
			$text .= "\n</pre>";
			$state = ']';
		
		// Just text
		} else if ($fail_same && $autoformat_lines[$i-1]{$len-1} != ' ' && $text != '') {
			// consecutive * and # lines are different items
			$i--;
			return $text;
		} else {
			// text
			if ($first) {
				if ($state == 'P') $text = substr($text,0,-4);
				else               $text .= '<p>';
			}
			$text .= autoformat_line($line) . "\n";
			if ($first) {
				$text .= '</p>';
			}
			$state = 'P';
		}}
		//print_r("\n\n--------------[$prefix]---------------------\n$text\n");
	}
	//print_r("\n\n==================[$prefix]=================\n$text");
	//print_r("\n==================///=================\n");
	if ($state == '|') $text .= autoformat_table($table);
	return $text;
}

/**
 * Format a table, given the rows
 */
function autoformat_table($rows) {
	foreach ($rows as $k=>$r) {
		// split lines into columns
		$cols = array();
		foreach($r['lines'] as $l) {
			// split into columns
			$lcols = preg_split("/\t+/",$l);
			for ($i = 0 ; $i < count($lcols) ; ++$i) {
				$cols[$i] .= $lcols[$i] . "\n";
			}
		}
		$rows2[$k] = $cols;
	}
	$newrows = array();
	foreach ($rows2 as $y=>$r) {
		// colspan
		foreach($r as $x=>$c) {
			$data = trim($c);
			if ($data == "<<<" && $x > 0) {
				$skip = $newrows[$y][$x-1]['skip'];
				if ($skip == false) $skip = array($y,$x-1);
				$newrows[$y][$x]['skip'] = $skip;
				if ($y == $skip[0]) {
					$newrows[$skip[0]][$skip[1]]['cols'] += 1;
				}
			} elseif ($data == "^^^" && $y > 0) {
				$skip = $newrows[$y-1][$x]['skip'];
				if ($skip == false) $skip = array($y-1,$x);
				$newrows[$y][$x]['skip'] = $skip;
				if ($x == $skip[1]) {
					$newrows[$skip[0]][$skip[1]]['rows'] += 1;
				}
			} else {
				$newrows[$y][$x]['skip'] = false;
				$newrows[$y][$x]['data'] = $data;
				$newrows[$y][$x]['rows'] = 1;
				$newrows[$y][$x]['cols'] = 1;
			}
		}
	}
	
	global $autoformat_lines;
	$l = $autoformat_lines;
	$text = '<table>';
	foreach ($newrows as $k=>$r) {
		$text .= $k %2 == 0 ? '<tr class="even">' : '<tr class="odd">';
		$td = $rows[$k]['heading'] ? 'th' : 'td';
		foreach($r as $c) {
			if (!$c['skip']) {
				$text .= "<$td"
				      . ($c['cols'] > 1 ? ' colspan="'.$c['cols'].'"' : "")
				      . ($c['rows'] > 1 ? ' rowspan="'.$c['rows'].'"' : "")
				      . ">";
				$text .= autoformat($c['data'], false);
				$text .= "</$td>";
			}
		}
		$text .= '</tr>';
	}
	$text .= '</table>';
	$autoformat_lines = $l;
	return $text;
}


/**
 * Expand formting tags inside a single line,
 */
function autoformat_line($line) {
	$line = preg_replace("/'''(.*?)'''/",  "<strong>\\1</strong>", $line);
	$line = preg_replace("/''(.*?)''/",    "<em>\\1</em>", $line);
	$line = preg_replace_callback("/@(.*?)@/", "autoformat_code", $line);
	$line = preg_replace_callback("/\[\[(.*?)\|(.*?)]](s?)/", "autoformat_link_s", $line);
	$line = preg_replace_callback("/\[\[(.*?)]](s?)/", "autoformat_link", $line);
	return $line;
}

function autoformat_code($matches) {
	return '<tt>' . syntax_highlight(htmlspecialchars($matches[1])) . '</tt>';
}

function autoformat_link($matches) {
	return '<a href="' . autoformat_url($matches[1]) . '">' . autoformat_title($matches[1]) . $matches[2] . '</a>';
}
function autoformat_link_s($matches) {
	return '<a href="' . autoformat_url($matches[1]) . '">' . $matches[2] . $matches[3] . '</a>';
}

function autoformat_url($url) {
	if (preg_match("/^type:/",$url)) {
		return url("doc/type/" . substr($url,5));
	} elseif (preg_match("/^fun:/",$url)) {
		return url("doc/function/" . substr($url,4));
	} else {
		return url($url);
	}
}
function autoformat_title($url) {
	global $nice_names;
	if (isset($nice_names[$url])) {
		return $nice_names[$url];
	} else {
		return preg_replace("/.*:/","",$url);
	}
}

?>