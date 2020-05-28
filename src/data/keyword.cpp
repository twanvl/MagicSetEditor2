//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/keyword.hpp>
#include <util/tagged_string.hpp>
#include <unordered_map>
#include <unordered_set>

class KeywordTrie;
DECLARE_POINTER_TYPE(KeywordParamValue);
class Value;
DECLARE_DYNAMIC_ARG(Value*, value_being_updated);

#define USE_CASE_INSENSITIVE_KEYWORDS 1

// ----------------------------------------------------------------------------- : Reflection

KeywordParam::KeywordParam()
  : optional(true)
  , eat_separator(true)
{}

IMPLEMENT_REFLECTION(ParamReferenceType) {
  REFLECT(name);
  REFLECT(description);
  REFLECT(script);
}

IMPLEMENT_REFLECTION(KeywordParam) {
  REFLECT(name);
  REFLECT(description);
  REFLECT(placeholder);
  REFLECT(optional);
  REFLECT(match);
  REFLECT(separator_before_is);
  REFLECT(separator_after_is);
  REFLECT(eat_separator);
  REFLECT(script);
  REFLECT(reminder_script);
  REFLECT(separator_script);
  REFLECT(example);
  REFLECT(refer_scripts);
}
IMPLEMENT_REFLECTION(KeywordMode) {
  REFLECT(name);
  REFLECT(description);
  REFLECT(is_default);
}

// backwards compatability
template <typename T> void read_compat(T&, const Keyword*) {}
void read_compat(Reader& handler, Keyword* k) {
  if (!k->match.empty()) return;
  String separator, parameter;
  REFLECT(separator);
  REFLECT(parameter);
  // create a match string from the keyword
  k->match = k->keyword;
  size_t start = separator.find_first_of('[');
  size_t end   = separator.find_first_of(']');
  if (start != String::npos && end != String::npos) {
    k->match += substr(separator, start + 1, end - start - 1);
  }
  if (parameter == _("no parameter")) {
    parameter.clear(); // was used for magic to indicate absence of parameter
  }
  if (!parameter.empty()) {
    k->match += _("<atom-param>") + parameter + _("</atom-param>");
  }
}

bool Keyword::contains(QuickFilterPart const& query) const {
  if (query.match(_("keyword"), keyword)) return true;
  if (query.match(_("rules"), rules)) return true;
  if (query.match(_("match"), match)) return true;
  if (query.match(_("reminder"), reminder.get())) return true;
  return false;
}

IMPLEMENT_REFLECTION(Keyword) {
  REFLECT(keyword);
  if (handler.formatVersion() < 301) read_compat(handler, this);
  REFLECT(match);
  REFLECT(reminder);
  REFLECT(rules);
  REFLECT(mode);
}

void KeywordParam::compile() {
  // compile separator_before
  if (!separator_before_is.empty() && separator_before_re.empty()) {
    separator_before_re.assign(_("^") + separator_before_is);
    if (eat_separator) {
      separator_before_eat.assign(separator_before_is + _("$"));
    }
  }
  // compile separator_after
  if (!separator_after_is.empty() && separator_after_re.empty()) {
    separator_after_re.assign(separator_after_is + _("$"));
    if (eat_separator) {
      separator_after_eat.assign(_("^") + separator_after_is);
    }
  }
}
void KeywordParam::eat_separator_before(String& text) {
  if (separator_before_eat.empty()) return;
  Regex::Results result;
  if (separator_before_eat.matches(result, text)) {
    // keep only stuff before the separator
    assert(result.position() + result.size() == text.size());
    text.resize(result.position());
  }
}
void KeywordParam::eat_separator_after(const String& text, size_t& i) {
  if (separator_after_eat.empty()) return;
  Regex::Results result;
  if (separator_after_eat.matches(result, text.begin() + i, text.end())) {
    // advance past the separator
    assert(result.position() == 0);
    i += result.length();
  }
}

size_t Keyword::findMode(const vector<KeywordModeP>& modes) const {
  // find
  size_t id = 0;
  FOR_EACH_CONST(m, modes) {
    if (mode == m->name) return id;
    ++id;
  }
  // default
  id = 0;
  FOR_EACH_CONST(m, modes) {
    if (m->is_default) return id;
    ++id;
  }
  // not found
  return 0;
}

// ----------------------------------------------------------------------------- : Regex stuff

void Keyword::prepare(const vector<KeywordParamP>& param_types, bool force) {
  if (!force && !match_re.empty()) return;
  parameters.clear();
  // Prepare regex
  String regex;
  String text; // normal, non-regex, text
  vector<KeywordParamP>::const_iterator param = parameters.begin();
  #if USE_CASE_INSENSITIVE_KEYWORDS
    regex = _("(?i)"); // case insensitive matching
  #endif
  // Parse the 'match' string
  for (size_t i = 0 ; i < match.size() ;) {
    Char c = match.GetChar(i);
    if (is_substr(match, i, _("<atom-param"))) {
      // parameter, determine type...
      size_t start = skip_tag(match, i), end = match_close_tag(match, i);
      String type = match.substr(start, end-start);
      // find parameter type 'type'
      KeywordParamP param;
      FOR_EACH_CONST(pt, param_types) {
        if (pt->name == type) {
          param = pt;
          break;
        }
      }
      if (!param) {
        // throwing an error can mean a set will not be loaded!
        // instead, simply disable the keyword
        //throw InternalError(_("Unknown keyword parameter type: ") + type);
        handle_error(_("Unknown keyword parameter type: ") + type);
        valid = false;
        return;
      }
      parameters.push_back(param);
      // modify regex : match text before
      param->compile();
      // remove the separator from the text to prevent duplicates
      param->eat_separator_before(text);
      regex += _("(") + regex_escape(text) + _(")");
      text.clear();
      // modify regex : match parameter
      regex += _("(") + make_non_capturing(param->match) + (param->optional ? _(")?") : _(")"));
      i = skip_tag(match, end);
      // eat separator_after?
      param->eat_separator_after(match, i);
    } else {
      text += c;
      i++;
    }
  }
  regex += _("(") + regex_escape(text) + _(")");
  #if USE_BOOST_REGEX
    regex = _("\\<")
  #else
    regex = _("\\y")
  #endif
        + regex + _("(?=$|[^a-zA-Z0-9\\(])"); // only match whole words
  match_re.assign(regex);
  // not valid if it matches "", that would make MSE hang
  valid = !match_re.matches(_(""));
}

// ----------------------------------------------------------------------------- : KeywordTrie

namespace std {
  template <> struct hash<wxUniChar> {
    inline size_t operator () (wxUniChar x) const {
      return std::hash<wxUniChar::value_type>()(x.GetValue());
    }
  };
}

/// A node in a trie to match keywords
/* The trie is used to speed up matching, by quickly finding candidate keywords.
*/
class KeywordTrie {
public:
  KeywordTrie();
  ~KeywordTrie();
  
  unordered_map<wxUniChar, unique_ptr<KeywordTrie>> children; ///< children after a given character
  KeywordTrie* on_any_star; ///< children on /.*/ (owned or this)
  vector<const Keyword*> finished; ///< keywords that end in this node
  
  /// Insert nodes representing the given character
  /** return the node where the evaluation will be after matching the character */
  KeywordTrie* insert(wxUniChar match);
  /// Insert nodes representing the given string
  /** return the node where the evaluation will be after matching the string */
  KeywordTrie* insert(const String& match);

  /// Insert nodes representing the regex /.*/
  /** return the node where the evaluation will be after matching that regex */
  KeywordTrie* insertAnyStar();
};


KeywordTrie::KeywordTrie()
  : on_any_star(nullptr)
{}
KeywordTrie::~KeywordTrie() {
  if (on_any_star != this) delete on_any_star;
}

KeywordTrie* KeywordTrie::insert(wxUniChar c) {
  #if USE_CASE_INSENSITIVE_KEYWORDS
    c = toLower(c); // case insensitive matching
  #endif
  unique_ptr<KeywordTrie>& child = children[c];
  if (!child) child.reset(new KeywordTrie);
  return child.get();
}
KeywordTrie* KeywordTrie::insert(const String& match) {
  KeywordTrie* cur = this;
  for (wxUniChar c : match) {
    cur = cur->insert(c);
  }
  return cur;
}

KeywordTrie* KeywordTrie::insertAnyStar() {
  if (!on_any_star) on_any_star = new KeywordTrie();
  on_any_star->on_any_star = on_any_star; // circular reference to itself
  return on_any_star;
}


// ----------------------------------------------------------------------------- : KeywordDatabase

IMPLEMENT_DYNAMIC_ARG(KeywordUsageStatistics*, keyword_usage_statistics, nullptr);

KeywordDatabase::KeywordDatabase()
  : root(nullptr)
{}
// Note: has to be here because in the header KeywordTrie is not defined
KeywordDatabase::~KeywordDatabase() {}

void KeywordDatabase::clear() {
  root.reset();
}

void KeywordDatabase::add(const vector<KeywordP>& kws) {
  FOR_EACH_CONST(kw, kws) {
    add(*kw);
  }
}

void KeywordDatabase::add(const Keyword& kw) {
  if (kw.match.empty() || !kw.valid) return; // can't handle empty keywords
  // Create root
  if (!root) {
    root = make_unique<KeywordTrie>();
    root->on_any_star = root.get();
  }
  KeywordTrie* cur = root->insertAnyStar();
  // Add to trie
  String text; // normal text
  size_t param = 0;
  bool only_star = true;
  for (size_t i = 0 ; i < kw.match.size() ;) {
    Char c = kw.match.GetChar(i);
    if (is_substr(kw.match, i, _("<atom-param"))) {
      i = match_close_tag_end(kw.match, i);
      // parameter, is there a separator we should eat?
      if (param < kw.parameters.size()) {
        kw.parameters[param]->eat_separator_before(text);
        kw.parameters[param]->eat_separator_after(kw.match, i);
      }
      ++param;
      // match anything
      cur = cur->insert(text);
      text.clear();
      cur = cur->insertAnyStar();
      // enough?
      if (!only_star) {
        // If we have matched anything specific, this is a good time to stop
        // it doesn't really matter how long we go on, since the trie is only used
        // as an optimization to not have to match lots of regexes.
        // As an added bonus, we get a better behaviour of matching earlier keywords first.
        break;
      }
    } else {
      text += c;
      i++;
      only_star = false;
    }
  }
  cur = cur->insert(text);
  // now cur is the trie after matching the keyword anywhere in the input text
  cur->finished.push_back(&kw);
}

void KeywordDatabase::prepare_parameters(const vector<KeywordParamP>& ps, const vector<KeywordP>& kws) {
  FOR_EACH_CONST(kw, kws) {
    kw->prepare(ps);
  }
}

#ifdef _DEBUG
void dump(int i, const KeywordTrie* t) {
  FOR_EACH(c, t->children) {
    wxLogDebug(String(i, _(' ')) + c.first + _("     ") + String::Format(_("%p"), c.second.get()));
    dump(i + 2, c.second.get());
  }
  if (t->on_any_star) {
    wxLogDebug(String(i, _(' ')) + _(".*") + _("     ") + String::Format(_("%p"), t->on_any_star));
    if (t->on_any_star != t) dump(i + 2, t->on_any_star);
  }
}
#endif

// ----------------------------------------------------------------------------- : KeywordDatabase : matching

// transitive closure of a state, follow all on_any_star links
void closure(vector<const KeywordTrie*>& state) {
  for (size_t j = 0 ; j < state.size() ; ++j) {
    if (state[j]->on_any_star && state[j]->on_any_star != state[j]) {
      state.push_back(state[j]->on_any_star);
    }
  }
}

void step_state(vector<const KeywordTrie*>& state, wxUniChar c) {
  vector<const KeywordTrie*> next;
  for(auto kt : state) {
    auto it = kt->children.find(c);
    if (it != kt->children.end()) {
      next.push_back(it->second.get());
    }
    // TODO: on any star first or last?
    if (kt->on_any_star) {
      next.push_back(kt->on_any_star);
    }
  }
  swap(state,next);
}

// Collect possible matching keywords
/* First step in matching is to run over the string, and use the trie to find keywords that *potentially* appear in it.
 */
unordered_set<Keyword const*> possible_matches(String const& tagged_str, KeywordTrie const* trie_root) {
  unordered_set<const Keyword*> possible_matches;
  if (!trie_root) return possible_matches;

  vector<const KeywordTrie*> state;
  state.push_back(trie_root);

  for (String::const_iterator it = tagged_str.begin(); it != tagged_str.end();) {
    wxUniChar c = *it;
    // tag?
    if (c == '<') {
      it = skip_tag(it, tagged_str.end());
    } else {
      ++it;
      c = toLower(c); // case insensitive matching
      // find 'next' trie node set matching c
      step_state(state, c);
      closure(state);
      // matches
      for (auto kt : state) {
        for (auto kw : kt->finished) {
          possible_matches.insert(kw);
        }
      }
    }
  }
  return possible_matches;
}

struct KeywordMatch {
  Keyword const* keyword;
  // match in (substring of) the untagged string
  Regex::Results match;
  // position of match in the untagged string
  size_t pos;
  KeywordMatch(Keyword const& keyword, Regex::Results match, size_t pos) : keyword(&keyword), match(match), pos(pos) {}
};

// Collect exact matching keywords
/* Second step in matching is to match regexes
 */
void keyword_matches(const String& untagged_str, const Keyword& keyword, vector<KeywordMatch>& out) {
  Regex::Results match;
  size_t i = 0;
  String::const_iterator it = untagged_str.begin();
  while (keyword.match_re.matches(match, it, untagged_str.end())) {
    size_t pos = match[0].first - untagged_str.begin();
    out.emplace_back(keyword, match, pos);
    it = max(it+1, match[0].end());
  }
}
void keyword_matches(const String& untagged_str, unordered_set<Keyword const*> keywords, vector<KeywordMatch>& out) {
  for (auto keyword : keywords) {
    keyword_matches(untagged_str, *keyword, out);
  }
}
void sort_keyword_matches(vector<KeywordMatch>& matches) {
  // sort matches by their start position
  sort(matches.begin(), matches.end(), [](KeywordMatch const& a, KeywordMatch const& b) {
    if (a.pos < b.pos) return true;
    if (a.pos > b.pos) return false;
    // otherwise sort by matching set keywords (non-fixed) first
    if (a.keyword->fixed < b.keyword->fixed) return true;
    if (a.keyword->fixed > b.keyword->fixed) return false;
    // otherwise sort by name
    return a.keyword->keyword < b.keyword->keyword;
  });
}
vector<KeywordMatch> keyword_matches(const String& untagged_str, unordered_set<Keyword const*> keywords) {
  vector<KeywordMatch> out;
  keyword_matches(untagged_str, keywords, out);
  sort_keyword_matches(out);
  return out;
}



tuple<bool,String::const_iterator> expand_keyword(String::const_iterator it, String::const_iterator end, KeywordMatch const& match, char expand_type, String& out, KeywordExpandOptions const& options);

/* Last step in matching is to go over the string, and expand each of the matches, as long as they don't overlap
 * Note that matches are already sorted, so we can try them in order.
 * But as a complication, positions and lengths in matches refer to the untagged string.
 */
String expand_keywords(const String& tagged_str, vector<KeywordMatch> const& matches, KeywordExpandOptions const& options) {
  vector<KeywordMatch>::const_iterator match_it = matches.begin();
  size_t untagged_pos = 0;

  // tags to skip
  int atom = 0;
  // Possible values are:
  //  - '0' = reminder text explicitly hidden
  //  - '1' = reminder text explicitly shown
  //  - 'a' = reminder text in default state, hidden
  //  - 'A' = reminder text in default state, shown
  const char default_expand_type = 'a';
  char expand_type = default_expand_type;

  String out;
  String::const_iterator it = tagged_str.begin();
  const String::const_iterator end = tagged_str.end();

  // in the loop below, skip past tags
  auto skip_tags_for_keyword = [&](bool open, bool close) {
    while (it != end && *it == '<') {
      if (is_substr(it, end, "<kw-")) {
        if (it + 4 != end) expand_type = *(it + 4); // <kw-?>
        it = skip_tag(it, end);
      } else if (is_substr(it, end, "</kw-")) {
        expand_type = default_expand_type;
        it = skip_tag(it, end);
      } else {
        bool is_close = (it+1) != end && *(it+1) == '/';
        if ((is_close && !close) || (!is_close && !open)) return;
        if (is_tag(it, end, "<atom")) {
          atom++;
        } else if (is_tag(it, end, "</atom")) {
          atom++;
        }
        // keep tag in output
        auto after = skip_tag(it, end);
        out.append(it, after);
        it = after;
      }
    }
  };

  while (true) {
    // prefer to match 'outside' tags, so before open tags and after close tags
    // that way we avoid breaking up atoms
    // so here match only close tags
    skip_tags_for_keyword(false, true);
    if (it == end) break;
    // is there a match here?
    if (atom == 0) // don't expand keywords that are inside <atom> tags
    while (match_it != matches.end() && match_it->pos <= untagged_pos) {
      if (match_it->pos > untagged_pos) {
        ++match_it;
        continue;
      }
      // try to expand
      auto [match,new_it] = expand_keyword(it, end, *match_it, expand_type, out, options);
      if (match) {
        untagged_pos += untagged_length(it,new_it);
        it = new_it;
        ++match_it;
        goto after_match;
      } else {
        ++match_it;
      }
    }
    // No match, so there is at least one character not part of a keyword
    // and possibly some tags before it that we missed
    skip_tags_for_keyword(true, true);
    out += *it;
    ++it;
    ++untagged_pos;
    // after matching or skipping, go past close tags, to remain as much oustide tags as possible
    after_match:
    skip_tags_for_keyword(true, false);
  }
  return out;
}

// Get detailed information on a keyword match:
//  * The value of each of the parameters
//  * Whether the case matches
// Add these things to the context
// Return iterator after the whole match
tuple<bool,String::const_iterator> keyword_match_detail(String::const_iterator it, String::const_iterator end, KeywordMatch const& kw_match, Context& ctx) {
  Keyword const& keyword = *kw_match.keyword;
  Regex::Results const& match = kw_match.match;

  // used placeholders?
  bool used_placeholders = false;
  // case errors? For finding these we will loop over the keyword.match string
  bool correct_case = true;
  String::const_iterator match_str_it = keyword.match.begin();

  // in tags?
  int atom = 0;

  // Combined tagged match string
  String total;

  // Split the keyword, set parameters in context
  // The even captures are parameter values, the odd ones are the plain text in between
  // submatch 0 is the whole match
  assert(match.size() - 1 == 1 + 2 * keyword.parameters.size());
  for (int sub = 1; sub < (int)match.size(); ++sub) {
    // The matched part, indices in untagged string. We only need the length
    size_t part_len_untagged = match.length(sub);
    // Translate back to tagged position
    // Note: when part_len_untagged==0, the positions are invalid
    String::const_iterator part_end = advance_untagged(it, end, part_len_untagged, false,true);
    String part(it,part_end);
    // strip left over </kw tags
    part = remove_tag(part, _("</kw-"));

    // we start counting at 1, so
    // sub = 1 mod 2 -> text
    // sub = 0 mod 2 -> parameter
    bool is_parameter = (sub % 2) == 0;
    if (is_parameter) {
      // parameter
      KeywordParam& kwp = *keyword.parameters[(sub - 2) / 2];
      String param = match.str(sub); // untagged version
      // strip separator_before
      String separator_before, separator_after;
      Regex::Results sep_match;
      if (!kwp.separator_before_re.empty() && kwp.separator_before_re.matches(sep_match, param)) {
        size_t sep_end = sep_match.length();
        assert(sep_match.position() == 0); // should only match at start of param
        separator_before.assign(param, 0, sep_end);
        param.erase(0, sep_end);
        // strip from tagged version
        size_t sep_end_t = untagged_to_index(part, sep_end, false);
        part = get_tags(part, 0, sep_end_t, true, true) + part.substr(sep_end_t);
        // transform?
        if (kwp.separator_script) {
          ctx.setVariable(_("input"), to_script(separator_before));
          separator_before = kwp.separator_script.invoke(ctx)->toString();
        }
      }
      // strip separator_after
      if (!kwp.separator_after_re.empty() && kwp.separator_after_re.matches(sep_match, param)) {
        size_t sep_start = sep_match.position();
        assert(sep_match[0].second == param.end()); // should only match at end of param
        separator_after.assign(param, sep_start, String::npos);
        param.resize(sep_start);
        // strip from tagged version
        size_t sep_start_t = untagged_to_index(part, sep_start, false);
        part = part.substr(0, sep_start_t) + get_tags(part, sep_start_t, part.size(), true, true);
        // transform?
        if (kwp.separator_script) {
          ctx.setVariable(_("input"), to_script(separator_after));
          separator_after = kwp.separator_script.invoke(ctx)->toString();
        }
      }
      // to script
      KeywordParamValueP script_param = make_intrusive<KeywordParamValue>(kwp.name, separator_before, separator_after, param);
      KeywordParamValueP script_part  = make_intrusive<KeywordParamValue>(kwp.name, separator_before, separator_after, part);
      // process param
      if (param.empty()) {
        // placeholder
        used_placeholders = true;
        script_param->value = _("<atom-kwpph>") + (kwp.placeholder.empty() ? kwp.name : kwp.placeholder) + _("</atom-kwpph>");
        script_part->value = part + script_param->value; // keep tags
      } else {
        // apply parameter script
        if (kwp.script) {
          ctx.setVariable(_("input"), script_part);
          script_part->value = kwp.script.invoke(ctx)->toString();
        }
        if (kwp.reminder_script) {
          ctx.setVariable(_("input"), script_param);
          script_param->value = kwp.reminder_script.invoke(ctx)->toString();
        }
      }
      part = separator_before + script_part->toString() + separator_after;
      ctx.setVariable(String(_("param")) << (int)(sub / 2), script_param);

    } else {
      // Plain text with exact match
      // check if the case matches
      if (correct_case) {
        while (it != part_end) {
          it = skip_all_tags(it, part_end);
          if (it == part_end) break;
          while (match_str_it != keyword.match.end() && is_substr(match_str_it, keyword.match.end(), "<param")) {
            match_str_it = skip_tag(match_str_it, keyword.match.end());
            while (match_str_it != keyword.match.end() && !is_substr(match_str_it, keyword.match.end(), "</param")) ++match_str_it;
            match_str_it = skip_tag(match_str_it, keyword.match.end());
          }
          if (match_str_it == keyword.match.end()) break;
          // does the text match the keyword match string exactly?
          if (*it != *match_str_it) {
            correct_case = false;
            break;
          }
          ++it;
          ++match_str_it;
        }
      }
    }
    // count <atom> tags
    for (String::const_iterator pit = part.begin(); pit != part.end();) {
      if (*pit == '<') {
        if (is_tag(pit, part.end(), "<atom")) atom++;
        else if(is_tag(pit, part.end(), "</atom")) atom--;
        pit = skip_tag(pit, part.end());
      } else {
        if (atom > 0 && !is_parameter) {
          // the fixed parts of a keyword should not be in atom tags
          return {false,it};
        }
        ++pit;
      }
    }
    // build total match
    total += part;
    // next part starts after this
    it = part_end;
  }
  assert_tagged(total, false); // note: tags might not be entirely balanced
  ctx.setVariable(_("keyword"), to_script(total));
  ctx.setVariable(_("mode"), to_script(keyword.mode));
  ctx.setVariable(_("correct_case"), to_script(correct_case));
  ctx.setVariable(_("used_placeholders"), to_script(used_placeholders));
  return {true, it};
};

// expand a keyword that matches at it
tuple<bool, String::const_iterator> expand_keyword(String::const_iterator it, String::const_iterator end, KeywordMatch const& kw_match, char expand_type, String& out, KeywordExpandOptions const& options) {
  Keyword const& keyword = *kw_match.keyword;

  // Perform script stuff in a local scope to not leave a mess
  Context& ctx = options.ctx;
  LocalScope scope(ctx);

  // Get details of the match
  auto [ok, after] = keyword_match_detail(it, end, kw_match, ctx);
  if (!ok) return {false,it};

  // Final check whether the keyword matches
  if (options.match_condition && options.match_condition->eval(ctx)->toBool() == false) {
    return {false,it};
  }

  // Show reminder text?
  bool expand = expand_type == _('1');
  if (!expand && expand_type != _('0')) {
    // default expand, determined by script
    expand = options.expand_default ? options.expand_default->eval(ctx)->toBool() : true;
    expand_type = expand ? _('A') : _('a');
  }
  ctx.setVariable(_("expand"), to_script(expand));

  // Reminder text
  String reminder;
  try {
    reminder = keyword.reminder.invoke(ctx)->toString();
  } catch (const Error& e) {
    handle_error(_ERROR_2_("in keyword reminder", e.what(), keyword.keyword));
  }
  ctx.setVariable(_("reminder"), to_script(reminder));

  // Combine, add to output
  out += _("<kw-");
  out += expand_type;
  out += _(">");
  out += options.combine_script->eval(ctx)->toString();
  out += _("</kw-");
  out += expand_type;
  out += _(">");

  // Add to usage statistics
  if (options.stat && options.stat_key) {
    options.stat->emplace_back(options.stat_key, &keyword);
  }

  return {true,after};
}

String remove_keyword_tags(String const& tagged_str) {
  // Remove all old reminder texts
  String s = remove_tag_contents(tagged_str, _("<atom-reminder"));
  s = remove_tag_contents(s, _("<atom-keyword")); // OLD, TODO: REMOVEME
  s = remove_tag_contents(s, _("<atom-kwpph>"));
  s = remove_tag(s, _("<keyword-param"));
  s = remove_tag(s, _("<param-"));
  return s;
}

void remove_from_stats(KeywordUsageStatistics* stat, const Value* stat_key) {
  if (stat && stat_key) {
    auto condition = [stat_key](KeywordUsageStatistics::value_type const& it) {
      return it.first == stat_key;
    };
    stat->erase(std::remove_if(stat->begin(), stat->end(), condition), stat->end());
  }
}

String KeywordDatabase::expand(const String& text, KeywordExpandOptions const& options) const {
  assert(options.combine_script);
  assert_tagged(text);

  // Clean up usage statistics
  remove_from_stats(options.stat, options.stat_key);
  
  // Remove all old reminder texts
  String tagged = remove_keyword_tags(text);

  // any keywords in database?
  if (!root) return tagged;

  // Find potential matches
  auto possible_matches = ::possible_matches(tagged, root.get());

  // Refine
  String untagged = untag_no_escape(tagged);
  auto matches = keyword_matches(untagged, possible_matches);
  
  // Expand
  String result = expand_keywords(tagged, matches, options);
  assert_tagged(result);
  return result;
}

// ----------------------------------------------------------------------------- : KeywordParamValue

ScriptType KeywordParamValue::type() const { return SCRIPT_STRING; }
String KeywordParamValue::typeName() const { return _("keyword parameter"); }

String KeywordParamValue::toString() const {
  String safe_type = replace_all(replace_all(replace_all(type_name,
              _("("),_("-")),
              _(")"),_("-")),
              _(" "),_("-"));
  return _("<param-") + safe_type + _(">") + value  + _("</param-") + safe_type + _(">");
}

// a bit of a hack: use the ScriptString implementation
int KeywordParamValue::toInt()       const { return to_script(value)->toInt(); }
double KeywordParamValue::toDouble() const { return to_script(value)->toDouble(); }
bool KeywordParamValue::toBool()     const { return to_script(value)->toBool(); }
Color KeywordParamValue::toColor()   const { return to_script(value)->toColor(); }
int KeywordParamValue::itemCount()   const { return to_script(value)->itemCount(); }

ScriptValueP KeywordParamValue::getMember(const String& name) const {
  if (name == _("type"))             return to_script(type_name);
  if (name == _("separator_before")) return to_script(separator_before);
  if (name == _("separator_after"))  return to_script(separator_after);
  if (name == _("value"))            return to_script(value);
  if (name == _("param"))            return to_script(value);
  return ScriptValue::getMember(name);
}
