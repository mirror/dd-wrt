\function{onig_new}
\synopsis{Create an instance of a regular-expression}
\usage{Onig_Type onig_new (pattern [,options [,encoding [,syntax]]])}
#v+
   String_Type pattern;
   Int_Type options;
   String_Type encoding;
   String_Type syntax;
#v-
\description
 The \ifun{onig_new} function compiles the specified regular
 expression (\exmp{pattern}) and returns the result.  The other
 parameters are optional and may be used to specify compilation
 options, the character-set encoding, and the regular expression
 syntax. Upon success, this function returns an \dtype{Onig_Type}
 object representing the compiled pattern.  If compilation fails, a
 \exc{OnigError} exception will be thrown.

 The \exmp{options} parameters is a bit mapped value that may be
 formed from the bitwise-or of zero or more of the following constants:
#v+
   ONIG_OPTION_NONE
   ONIG_OPTION_IGNORECASE
   ONIG_OPTION_EXTEND
   ONIG_OPTION_MULTILINE
   ONIG_OPTION_SINGLELINE
   ONIG_OPTION_FIND_LONGEST
   ONIG_OPTION_FIND_NOT_EMPTY
   ONIG_OPTION_NEGATE_SINGLELINE
   ONIG_OPTION_DONT_CAPTURE_GROUP
   ONIG_OPTION_CAPTURE_GROUP
#v-

 The character-set encoding may be specified using one of the
 following strings:
#v+
  "ascii"         "iso_8859_1"     "iso_8859_2"   "iso_8859_3"
  "iso_8859_4"    "iso_8859_5"     "iso_8859_6"   "iso_8859_7"
  "iso_8859_8",   "iso_8859_9"     "iso_8859_10"  "iso_8859_11"
  "iso_8859_13"   "iso_8859_14"    "iso_8859_15"  "iso_8859_16"
  "utf8"          "utf16_be"       "utf16_le"     "utf32_be"
  "utf32_le"      "euc_jp"         "euc_tw"       "euc_kr"
  "euc_cn"        "sjis"           "koi8_r"       "cp1251"
  "big5"          "gb18030"
#v-
 If not specified, "utf8" will be used if the interpreter is UTF-8
 mode and "iso_8859_1" if not in UTF-8 mode.

 The regular expression syntax of the pattern may be specified by
 setting the \exmp{syntax} parameter to one of the following:
#v+
  "asis"    "posix_basic"  "posix_extended"  "emacs"
  "grep"    "gnu_regex"    "java"            "perl"
  "perl_ng" "ruby"
#v-
 If unspecified, the syntax defaults to \exmp{"perl"}.
\seealso{onig_search, onig_nth_match, onig_nth_substr}
\done

\function{onig_search}
\synopsis{Search a string using an Onig compiled pattern}
\usage{Int_Type onig_search (p, str [,start_pos, end_pos] [,option])}
#v+
   Onig_Type p;
   String_Type str;
   Int_Type start_pos, end_pos;
#v-
\description
 The \ifun{onig_search} function applies a pre-compiled pattern \var{p} to a
 string \ifun{str} and returns the result of the match.  The optional
 third and fourth arguments may be used to constrain the search region
 to the specified byte-offsets in the string.  The \exmp{option}
 parameter is also optional and may be used to control how the search
 is to be performed.  Its value may be specified as a bitwise-or of zero or
 more of the following flags:
#v+
   ONIG_OPTION_NOTBOL
   ONIG_OPTION_NOTEOL
   ONIG_OPTION_POSIX_REGION
#v-
 See the onig library documentation for more information about the meaning
 of these flags.

 Upon success, this function returns a positive integer equal to 1 plus the
 number of so-called captured substrings.  It will return 0 if the pattern
 failed to match the string.
\seealso{onig_new, onig_nth_match, onig_nth_substr}
\done

\function{onig_nth_match}
\synopsis{Return the location of the nth match of an onig regular expression}
\usage{Int_Type[2] onig_nth_match (Onig_Type p, Int_Type nth)}
\description
 The \ifun{onig_nth_match} function returns an integer array whose
 values specify the locations as byte-offsets to the beginning and end
 of the \var{nth} captured substring of the most recent call to
 \ifun{onig_search} with the compiled pattern.  A value of \var{nth}
 equal to 0 represents the substring representing the entire match of
 the pattern.

 If the \var{nth} match did not take place, the function returns \NULL.
\example
 After the execution of:
#v+
    str = "Error in file foo.c, line 127, column 10";
    pattern = "file ([^,]+), line ([0-9]+)";
    p = onig_new (pattern);
    if (onig_search (p, str))
      {
         match_pos = onig_nth_match (p, 0);
         file_pos = onig_nth_match (p, 1);
         line_pos = onig_nth_match (p, 2);
      }
#v-
 \exmp{match_pos} will be set to \exmp{[9,29]}, \exmp{file_pos} to \exmp{[14,19,]}
 and \exmp{line_pos} to \exmp{[26,29]}.  These integer arrays may be used to
 extract the substrings matched by the pattern, e.g.,
#v+
     file = substr (str, file_pos[0]+1, file_pos[1]-file_pos[0]);
     line = str[[line_pos[0]:line_pos[1]-1]];
#v-
 Alternatively, the function \ifun{onig_nth_substr} may be used to get the
 matched substrings:
#v+
     file = onig_nth_substr (p, str, 0);
#v-
\seealso{onig_new, onig_search, onig_nth_substr}
\done

\function{onig_nth_substr}
\synopsis{Extract the nth matched substring from an Onig regular expression search}
\usage{String_Type onig_nth_substr (Onig_Type p, String_Type str, Int_Type nth)}
\description
 This function may be used to extract the \var{nth} captured substring
 resulting from the most recent use of the compiled pattern \var{p} by
 the \ifun{onig_search} function.  Unlike \ifun{onig_nth_match}, this
 function returns the specified captured substring itself and not the
 position of the substring. For this reason, the subject string of the
 pattern is a required argument.
\seealso{onig_new, onig_search, onig_nth_match}
\done
