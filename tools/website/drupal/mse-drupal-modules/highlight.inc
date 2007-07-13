<?php

// Syntax highlighting of script and reader code

function syntax_highlight($code) {
	if (preg_match("@^(\s*#.*\n)*\s*[a-zA-Z0-9 _/,]+:[^=]@", $code)) {
		return highlight_reader($code);
	} else {
		return highlight_script($code);
	}
}

function highlight_reader($code) {
	$ret = '';
	$lines = explode("\n",$code);
	$in_script = false;
	for ($i = 0 ; $i < count($lines) ; ++$i) {
		if ($i > 0) $ret .= "\n";
		preg_match("@^(\s*)(.*)@",$lines[$i],$matches);
		$indent = $matches[1];
		$ret .= $indent;
		$rest   = $matches[2];
		if ($in_script !== false) {
			if (strlen($indent) <= strlen($in_script)) $in_script = false;
		}
		if ($in_script !== false) {
			$ret .= highlight_script($rest);
		} else {
			if (preg_match("@^#@",$rest,$matches)) {
				$ret .= "<span class='hl-comment'>$rest</span>";
			} else if (preg_match("@^([a-zA-Z0-9 _/,]+):(.*)@",$rest,$matches)) {
				$key   = $matches[1];
				$value = $matches[2];
				if (preg_match("@script|default@", $key)) $in_script = $indent;
				if (strpos($value,"{")!==false) $in_script = $indent;
				if ($in_script !== false) {
					$value = highlight_script($value);
				} else if (preg_match("@^(\s*)script:(.*)@",$value,$matches)) {
					$value = $matches[1] . "<span class='hl-key'>script:</span>" . highlight_script($matches[2]);
				//} else if (preg_match("@\\s*rgb\\([0-9]+,[0-9]+,[0-9]+\\)\\s*@",$value,$matches)) {
				//	$value = highlight_script($value);
				}
				$ret .= "<span class='hl-key'>$key:</span>$value";
			} else {
				// not valid reader code
				if (strpos($rest,"{")!==false) $in_script = substr($indent,0,-1);
				if ($in_script !== false) $rest = highlight_script($rest);
				$ret .= $rest;
			}
		}
	}
	return $ret;
}

function highlight_script($code) {
	$ret = '';
	while(strlen($code)) {
		if (preg_match("@^(if|then|else|for|in|do|and|or|not|rgb)\b@",$code, $matches)) {
			$ret .= "<span class='hl-kw'>" . $matches[0] . "</span>";
		} else if (preg_match("@^(include file:)(.*)@",$code, $matches)) {
			$ret .= "<span class='hl-key'>" . $matches[1] . "</span>" . $matches[2];
		} else if (preg_match("@^[0-9][0-9.]*@",$code, $matches)) {
			$ret .= "<span class='hl-num'>" . $matches[0] . "</span>";
		} else if (preg_match("@^(\"|&quot;)(\\\\.|[^\\\\])*?(\"|&quot;)@",$code, $matches)) {
			$ret .= "<span class='hl-str'>" . $matches[0] . "</span>";
		} else if (preg_match("@^\\#.*@",$code, $matches)) {
			$ret .= "<span class='hl-comment'>" . $matches[0] . "</span>";
		} else if (preg_match("@^([-+*/=!.]|&lt;|&gt;)+|^:=@",$code, $matches)) {
			$ret .= "<span class='hl-op'>" . $matches[0] . "</span>";
		} else if (preg_match("@^[\\(\\)\\[\\]{},]+@",$code, $matches)) {
			$ret .= "<span class='hl-paren'>" . $matches[0] . "</span>";
		} else if (preg_match("@^[a-zA-Z_][a-zA-Z0-9_]*:@",$code, $matches)) {
			$ret .= "<span class='hl-ckey'>" . $matches[0] . "</span>";
		} else if (preg_match("@^([a-zA-Z0-9_]+\s*|\s+|&#?[a-zA-Z0-9]+;)@",$code, $matches)) {
			//$ret .= '[' . $matches[0] . ']';
			$ret .= $matches[0];
		} else {
			// fallback
			$matches = array($code[0]);
			//$ret .= '{{{' . $matches[0] . '}}}';
			$ret .= $matches[0];
		}
		$code = substr($code, strlen($matches[0]));
	}
	return $ret;
}

?>