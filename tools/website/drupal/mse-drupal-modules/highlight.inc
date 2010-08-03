<?php

// Syntax highlighting of script and reader code

global $built_in_functions;
$built_in_functions = array(
	// conversion
	'to_string'		=>'',
	'to_int'		=>'',
	'to_real'		=>'',
	'to_number'		=>'',
	'to_boolean'		=>'',
	'to_color'		=>'',
	'to_image'		=>'',
	'to_date'		=>'',
	// numbers
	'abs'			=>'',
	'random_int'		=>'',
	'random_real'		=>'',
	'random_boolean'	=>'',
	// text
	'to_upper'		=>'',
	'to_lower'		=>'',
	'to_title'		=>'',
	'reverse'		=>'',
	'substring'		=>'',
	'format'		=>'',	'format_rule'	=>'format',
	'curly_quotes'		=>'',
	'replace'		=>'',	'replace_rule'	=> 'replace',
	'filter_text'		=>'',	'filter_rule'	=> 'filter_text',
	'break_text'		=>'',	'break_rule'	=> 'break_text',
	'split_text'		=>'',
	'sort_text'		=>'',	'sort_rule'	=> 'sort_text',
	'contains'		=>'',
	'match'			=>'',	'match_rule'	=> 'match',
	'trim'			=>'',
	'regex_escape'		=>'',
	'check_spelling'	=>'',
	// tags
	'tag_contents'		=>'',	'tag_contents_rule'=>'tag_contents',
	'remove_tag'		=>'',	'tag_remove_rule'  =>'remove_tag',
	'remove_tags'		=>'',
	// lists
	'postion'		=>'',
	'length'		=>'',
	'number_of_items'	=>'',
	'sort_list'		=>'',
	'filter_list'		=>'',
	'random_shuffle'	=>'',
	'random_select'		=>'',
	'random_select_many'	=>'',
	// keywords
	'expand_keywords'	=>'',	'expand_keywords_rule'=>'expand_keywords',
	'keyword_usage'		=>'',
	// english
	'english_number'	 =>'',
	'english_number_a'	 =>'english_number',
	'english_number_multiple'=>'english_number',
	'english_number_ordinal' =>'english_number',
	'english_plural'	 =>'',
	'english_singular'	 =>'english_plural',
	'process_english_hints'	=>'',
	// fields and values
	'forward_editor'	=>'combined_editor',
	'combined_editor'	=>'',
	'primary_choice'	=>'',
	'chosen'		=>'',
	'count_chosen'		=>'',
	'require_choice'	=>'',
	'exclusive_choice'	=>'',
	'require_exclusive_choice'=>'',
	'remove_choice'		=>'',
	// images	
	'linear_blend'		=>'',
	'masked_blend'		=>'',
	'combine_blend'		=>'',
	'set_mask'		=>'',
	'set_alpha'		=>'',
	'set_combine'		=>'',
	'saturate'		=>'',
	'invert_image'		=>'',
	'recolor_image'		=>'',
	'enlarge'		=>'',
	'crop'			=>'',
	'flip_horizontal'	=>'',
	'flip_vertical'		=>'',
	'rotate'		=>'',
	'drop_shadow'		=>'',
	'symbol_variation'	=>'',
	'built_in_image'	=>'',
	// cards
	'new_card'		=>'',
	// html export
	'to_html'		=>'',
	'symbols_to_html'	=>'',
	'to_text'		=>'',
	'copy_file'		=>'',
	'write_text_file'	=>'',
	'write_image_file'	=>'',
	'write_set_file'	=>'',
	// other
	'trace'			=>'',
	'assert'		=>'',
);


function syntax_highlight($code) {
	if (preg_match("@^(\s*#.*\n)*\s*[-a-zA-Z0-9 _/,]+:([^=]|$)@", $code)) {
		return highlight_reader($code);
	} else if (preg_match("@^&lt;[^&>]*?(&gt;|>)$@", $code)) {
		return "<span class='hl-tag'>$code</span>";
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
	global $built_in_functions;
	global $current_doc_file;
	$ret = '';
	$string = ''; // inside "{..}" braces? b for braces, s for string
	while(strlen($code)) {
		if (preg_match("@^<[^>]+>@",$code, $matches)) {
			$ret .= $matches[0]; // plain tag
		} else if (preg_match("@^(rgb)[(]([0-9]+),([0-9]+),([0-9]+)[)]@",$code, $matches)) {
			$ret .= "<span class='hl-rgb'><span class='popup' style='background:" . $matches[0] . "'></span>"
			      . "<span class='hl-kw'>rgb</span>("
			      . "<span class='hl-num'>" . $matches[2] . "</span>,"
			      . "<span class='hl-num'>" . $matches[3] . "</span>,"
			      . "<span class='hl-num'>" . $matches[4] . "</span>)</span>";
		} else if (preg_match("@^(if|then|else|for( each)?|in(?= )|do|div|mod|and|or|xor|not|rgb|rgba|from|to|min|max)\b(?!:)@",$code, $matches)) {
			$ret .= "<span class='hl-kw'>" . $matches[0] . "</span>";
		} else if (preg_match("@^(include file:)(.*)@",$code, $matches)) {
			$ret .= "<span class='hl-key'>" . $matches[1] . "</span>" . $matches[2];
		} else if (preg_match("@^([0-9][0-9.]*|true|false)@",$code, $matches)) {
			$ret .= "<span class='hl-num'>" . $matches[0] . "</span>";
//		} else if (preg_match("@^(\"|&quot;)(\\\\.|[^\\\\])*?(\"|&quot;)@",$code, $matches)) {
//			$ret .= "<span class='hl-str'>" . $matches[0] . "</span>";
		} else if (preg_match("@^(\"|&quot;)(\\\\.|[^\\\\{])*?(\"|&quot;|{)@",$code, $matches)) {
			$ret .= "<span class='hl-str'>" . highlight_script_string($matches[0]) . "</span>";
			if ($matches[3] == '{') $string .= 's';
		} else if ($string != '' && $string{strlen($string)-1}=='s' &&
		           preg_match("@^}(\\\\.|[^\\\\{])*?(\"|&quot;|{)@",$code, $matches)) {
			// return from string quote
			$ret .= "<span class='hl-str'>" . highlight_script_string($matches[0]) . "</span>";
			$string = substr($string,0,-1);
			if ($matches[2] == '{') $string .= 's';
		} else if (preg_match("@^\\#.*@",$code, $matches)) {
			$ret .= "<span class='hl-comment'>" . $matches[0] . "</span>";
		} else if (preg_match("@^([-+*/=!.\@^]|&lt;|&gt;)+|^:=@",$code, $matches)) {
			$ret .= "<span class='hl-op'>" . $matches[0] . "</span>";
		} else if (preg_match("@^([\\(\\)\\[\\],]+)@",$code, $matches)) {
			$ret .= "<span class='hl-paren'>" . $matches[0] . "</span>";
		} else if (preg_match("@^[{}]@",$code, $matches)) {
			$ret .= "<span class='hl-fun-paren'>" . $matches[0] . "</span>";
			if     ($matches[0] == '{') $string .= 'b';
			elseif ($matches[0] == '}') $string = substr($string,0,-1);
		} else if (preg_match("@^[a-zA-Z_][a-zA-Z0-9_]*:@",$code, $matches)) {
			$ret .= "<span class='hl-ckey'>" . $matches[0] . "</span>";
		} else if (preg_match("@^([a-zA-Z0-9_]+|\s+|&#?[a-zA-Z0-9]+;)@",$code, $matches)) {
			$m = $matches[0];
			// function call?
			if (isset($built_in_functions[$m]) && // to built in function
			    preg_match("@\s*[+(){}]@",substr($code, strlen($matches[0]))) // in a function context
			    ) { 
				$f = $built_in_functions[$m];
				if ($f == '') $f = $m;
				$f = 'function/' . $f;
				if (!isset($current_doc_file) || $current_doc_file != $f) {
					$ret .= "<a class='hl-fun' href='" . url('doc/' . $f) . "'>" . $m . "</a>";
				} else {
					$ret .= "<span class='hl-this'>$m</span>"; // current function
				}
			} else {
				$ret .= $m;
				//$ret .= '[' . $m . ']';
			}
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
function highlight_script_string($code) {
	$code = preg_replace("@&lt;.*?(&gt;|>)@", "<span class='hl-tag'>\\0</span>", $code);
	return $code;
}

?>