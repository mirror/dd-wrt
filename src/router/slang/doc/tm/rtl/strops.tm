#d n \__newline__
#d notearray_int_f_str2 \
  This function has been vectorized in the sense that if an array of strings\n\
  is  passed for either of the string-valued arguments, then a\n\
  corresponding array of integers will be returned.  If two arrays\n\
  are passed then the arrays must have the same length.
#d notearray_int_f_str \
  This function has been vectorized in the sense that if an array of strings\n\
  is passed to the function, then a corresponding array of integers\n\
  will be returned.
#d notearray_str_f_str \
  This function has been vectorized in the sense that if an array of strings\n\
  is passed to the function, then a corresponding array of strings\n\
  will be returned.
#d notearray_str_f_str_arg1 \
  This function has been vectorized in the sense that if an array of strings\n\
  is passed as the first argument then a corresponding array of strings\n\
  will be returned.  Array values are not supported for the remaining\n\
  arguments.
#d notearray_strtrim \
  This function has been vectorized in the sense that if the first argument\n\
  is an array of strings, then a corresponding array of strings\n\
  will be returned.  An array value for the optional whitespace\n\
  argument is not supported.

\function{count_char_occurrences}
\synopsis{Count the number of occurrences of a character in a string}
\usage{UInt_Type count_char_occurrences (str, ch)}
\description
  This function returns the number of times the specified character
  \exmp{ch} occurs in the string \exmp{str}.
\notes
  If UTF-8 mode is in effect, then the character may correspond to
  more than one byte.  In such a case, the function returns the number
  of such byte-sequences in the string.  To count actual bytes, use
  the \ifun{count_byte_occurrences} function.
\seealso{count_byte_occurrences}
\done

\function{create_delimited_string}
\synopsis{Concatenate strings using a delimiter}
\usage{String_Type create_delimited_string (delim, s_1, s_2, ..., s_n, n)}
#v+
    String_Type delim, s_1, ..., s_n
    Int_Type n
#v-
\description
  \ifun{create_delimited_string} performs a concatenation operation on
  the \exmp{n} strings \exmp{s_1}, ...,\exmp{s_n}, using the string
  \exmp{delim} as a delimiter.  The resulting string is equivalent to
  one obtained via
#v+
      s_1 + delim + s_2 + delim + ... + s_n
#v-
\example
#v+
    create_delimited_string ("/", "user", "local", "bin", 3);
#v-
  will produce \exmp{"usr/local/bin"}.
\notes
  New code should use the \exmp{strjoin} function, which performs a
  similar task.
\seealso{strjoin, is_list_element, extract_element, strchop, strcat}
\done

\function{extract_element}
\synopsis{Extract the nth element of a string with delimiters}
\usage{String_Type extract_element (String_Type list, Int_Type nth, Int_Type delim)}
\description
  The \ifun{extract_element} function may be used to extract the
  \exmp{nth} substring of a string delimited by the character given by
  the \exmp{delim} parameter.  If the string contains fewer than the
  requested substring, the function will return \NULL.  Substring
  elements are numbered from 0.
\example
  The expression
#v+
     extract_element ("element 0, element 1, element 2", 1, ',')
#v-
  returns the string \exmp{" element 1"}, whereas
#v+
     extract_element ("element 0, element 1, element 2", 1, ' ')
#v-
  returns \exmp{"0,"}.

  The following function may be used to compute the number of elements
  in the list:
#v+
     define num_elements (list, delim)
     {
        variable nth = 0;
        while (NULL != extract_element (list, nth, delim))
          nth++;
        return nth;
     }
#v-
  Alternatively, the \ifun{strchop} function may be more useful.  In
  fact, \ifun{extract_element} may be expressed in terms of the
  function \ifun{strchop} as
#v+
    define extract_element (list, nth, delim)
    {
       list = strchop(list, delim, 0);
       if (nth >= length (list))
         return NULL;
       else
         return list[nth];
    }
#v-
   and the \exmp{num_elements} function used above may be recoded more
   simply as:
#v+
    define num_elements (list, delim)
    {
       return length (strchop (length, delim, 0));
    }
#v-
\notes
  New code should make use of the \exmp{List_Type} object for lists.
\seealso{is_list_element, is_substr, strtok, strchop, create_delimited_string}
\done

\function{glob_to_regexp}
\synopsis{Convert a globbing expression to a regular expression}
\usage{String_Type glob_to_regexp (String_Type g)}
\description
  This function may be used to convert a so-called globbing expression
  to a regular expression.  A globbing expression is frequently used
  for matching filenames where '?' represents a single character and
  '*' represents 0 or more characters.
\notes
  The \slsh program that is distributed with the \slang library
  includes a function called \sfun{glob} that is a wrapper around
  \ifun{glob_to_regexp} and \ifun{listdir}.  It returns a list of
  filenames matching a globbing expression.
\seealso{string_match, listdir}
\done

\function{is_list_element}
\synopsis{Test whether a delimited string contains a specific element}
\usage{Int_Type is_list_element (String_Type list, String_Type elem, Int_Type delim)}
\description
  The \ifun{is_list_element} function may be used to determine whether
  or not a delimited list of substring, \exmp{list}, contains the element
  \exmp{elem}.  If \exmp{elem} is not an element of \exmp{list}, the function
  will return zero, otherwise, it returns 1 plus the matching element
  number.
\example
  The expression
#v+
     is_list_element ("element 0, element 1, element 2", "0,", ' ');
#v-
  returns \exmp{2} since \exmp{"0,"} is element number one of the list
  (numbered from zero).
\seealso{extract_element, is_substr, create_delimited_string}
\done

\function{is_substr}
\synopsis{Test for a specified substring within a string}
\usage{Int_Type is_substr (String_Type a, String_Type b)}
\description
  This function may be used to determine if \exmp{a} contains the
  string \exmp{b}.  If it does not, the function returns 0; otherwise it
  returns the position of the first occurrence of \exmp{b} in \exmp{a}
  expressed in terms of characters, not bytes.
\notes
  This function regards the first character of a string to be given by
  a position value of 1.

  The distinction between characters and bytes is significant in UTF-8
  mode.

  \notearray_int_f_str2
\seealso{substr, string_match, strreplace}
\done

\function{make_printable_string}
\synopsis{Format a string suitable for parsing}
\usage{String_Type make_printable_string(String_Type str)}
\description
  This function formats a string in such a way that it may be used as
  an argument to the \ifun{eval} function.  The resulting string is
  identical to \exmp{str} except that it is enclosed in double quotes
  and the backslash, newline, control, and double quote characters are
  expanded.
\seealso{eval, str_quote_string}
\done

\function{Sprintf}
\synopsis{Format objects into a string (deprecated)}
\usage{String_Type Sprintf (String_Type format, ..., Int_Type n)}
\description
  This function performs a similar task as the \exmp{sprintf}
  function but requires an additional argument that specifies the
  number of items to format.  For this reason, the \exmp{sprintf}
  function should be used.
\seealso{sprintf, string, sscanf, vmessage}
\done

\function{strbskipchar}
\synopsis{Get an index to the previous character in a UTF-8 encoded string}
\usage{(p1, wch) = strbskipchar (str, p0 [,skip_combining])}
\description
  This function moves backward from the 0-based byte-offset \exmp{p0}
  in the string \exmp{str} to the previous character in the string.
  It returns the byte-offset (\exmp{p1} of the previous character and
  the decoded character value at that byte-offset.

  The optional third argument specifies the handling of
  combining characters.  If it is non-zero, combining characters will
  be ignored, otherwise a combining character will not be treated
  differently from other characters.  The default is to ignore such
  characters.

  If the byte-offset \exmp{p0} corresponds to the end of the string
  (\exmp{p0=0}), then \exmp{(p0,0)} will be returned.  Otherwise if
  the byte-offset specifies a value that lies outside the string, an
  \exc{IndexError} exception will be thrown.  Finally, if the
  byte-offset corresponds to an illegally coded character, the
  character returned will be the negative byte-value at the position.
\seealso{strskipchar, strskipbytes}
\done

\function{sprintf}
\synopsis{Format objects into a string}
\usage{String_Type sprintf (String fmt, ...)}
\description
  The \ifun{sprintf} function formats a string from a variable number
  of arguments according to according to the format specification
  string \exmp{fmt}.

  The format string is a C library \cfun{sprintf} style format
  descriptor.  Briefly, the format string may consist of ordinary
  characters (not including the \exmp{%} character), which are copied
  into the output string as-is, and conversion specification sequences
  introduced by the \exmp{%} character.  The number of additional
  arguments passed to the \ifun{sprintf} function must be consistent
  with the number required by the format string.

  The \exmp{%} character in the format string starts a conversion
  specification that indicates how an object is to be formatted.
  Usually the percent character is followed immediately by a
  conversion specification character.  However, it may optionally be
  followed by flag characters, field width characters, and precision
  modifiers, as described below.

  The character immediately following the \exmp{%} character may be
  one or more of the following flag characters:
#v+
    -         Use left-justification
    #         Use alternate form for formatting.
    0         Use 0 padding
    +         Preceed a number by a plus or minus sign.
    (space)   Use a blank instead of a plus sign.
#v-

  The flag characters (if any) may be followed by an optional field
  width specification string represented by one or more digit
  characters.  If the size of the formatted object is less than the
  field width, it will be right-justified in the specified field
  width, unless the \exmp{-} flag was given, in which case it will be
  left justified.

  If the next character in the control sequence is a period, then it
  introduces a precision specification sequence.  The precision is
  given by the digit characters following the period.  If none are
  given the precision is taken to be 0.  The meaning of the precision
  specifier depends upon the type of conversion:  For integer
  conversions, it gives the minimum number digits to appear in the
  output.  For \exmp{e} and \exmp{f} floating point conversions, it
  gives the number of digits to appear after the decimal point.  For
  the \exmp{g} floating point conversion, it gives the maximum number
  of significant digits to appear.  Finally for the \exmp{s} and
  \exmp{S} conversions it specifies the maximum number of characters
  to be copied to the output string.

  The next character in the sequence may be a modifier that controls
  the size of object to be formatted. It may consist of the following
  characters:
#v+
     h    This character is ignored in the current implementation.
     l    The integer is be formatted as a long integer, or a
          character as a wide character.
#v-

  Finally the conversion specification sequence ends with the
  conversion specification character that describes how the object is
  to be
  formatted:
#v+
     s    as a string
     f    as a floating point number
     e    as a float using exponential form, e.g., 2.345e08
     g    format as e or f, depending upon its value
     c    as a character
     b    as a byte
     %    a literal percent character
     d    as a signed decimal integer
     u    as an unsigned decimal integer
     o    as an octal integer
     X,x  as hexadecimal
     B    as a binary integer
     S    convert object to a string and format accordingly
#v-
  The \exmp{S} conversion specifier is a \slang extension which will
  cause the corresponding object to be converted to a string using the
  \ifun{string} function, and then converted as \exmp{s}. formatted as
  string.  In fact, \exmp{sprintf("%S",x)} is equivalent to
  \exmp{sprintf("%s",string(x))}.
\example
#v+
    sprintf("%s","hello")               ===> "hello"
    sprintf("%s %s","hello", "world")   ===> "hello world"
    sprintf("Agent %.3d",7)             ===> "Agent 007"
    sprintf("%S",PI)                    ===> "3.141592653589793"
    sprintf("%g",PI)                    ===> "3.14159"
    sprintf("%.2g",PI)                  ===> "3.1"
    sprintf("%.2e",PI)                  ===> "3.14e+00"
    sprintf("%.2f",PI)                  ===> "3.14"
    sprintf("|% 8.2f|",PI)              ===> "|    3.14|"
    sprintf("|%-8.2f|",PI)              ===> "|3.14    |"
    sprintf("|%+8.2f|",PI)              ===> "|   +3.14|"
    sprintf("|%8B|", 21)                ===> "|   10101|"
    sprintf("|%.8B|", 21)               ===> "|00010101|"
    sprintf("|%#.8B|", 21)              ===> "|0b00010101|"
    sprintf("%S",{1,2,3})               ===> "List_Type with 3 elements"
    sprintf("%S",1+2i)                  ===> "(1 + 2i)"
#v-
\notes
  The \ifun{set_float_format} function controls the format for the
  \exmp{S} conversion of floating point numbers.
\seealso{string, sscanf, message, pack}
\done

\function{sscanf}
\synopsis{Parse a formatted string}
\usage{Int_Type sscanf (s, fmt, r1, ... rN)}
#v+
    String_Type s, fmt;
    Ref_Type r1, ..., rN
#v-
\description
 The \ifun{sscanf} function parses the string \exmp{s} according to the
 format \exmp{fmt} and sets the variables whose references are given by
 \exmp{r1}, ..., \exmp{rN}.  The function returns the number of
 references assigned, or throws an exception upon error.

 The format string \exmp{fmt} consists of ordinary characters and
 conversion specifiers.  A conversion specifier begins with the
 special character \exmp{%} and is described more fully below.  A white
 space character in the format string matches any amount of whitespace
 in the input string.  Parsing of the format string stops whenever a
 match fails.

 The \exmp{%} character is used to denote a conversion specifier whose
 general form is given by \exmp{%[*][width][type]format} where the
 brackets indicate optional items.  If \exmp{*} is present, then the
 conversion will be performed but no assignment to a reference will be
 made.  The \exmp{width} specifier specifies the maximum field width to
 use for the conversion.  The \exmp{type} modifier is used to indicate
 the size of the object, e.g., a short integer, as follows.

 If \em{type} is given as the character \exmp{h}, then if the format
 conversion is for an integer (\exmp{dioux}), the object assigned will
 be a short integer.  If \em{type} is \exmp{l}, then the conversion
 will be to a long integer for integer conversions, or to a double
 precision floating point number for floating point conversions.

 The format specifier is a character that specifies the conversion:
#v+
       %     Matches a literal percent character.  No assignment is
             performed.
       d     Matches a signed decimal integer.
       D     Matches a long decimal integer (equiv to `ld')
       u     Matches an unsigned decimal integer
       U     Matches an unsigned long decimal integer (equiv to `lu')
       i     Matches either a hexadecimal integer, decimal integer, or
             octal integer.
       I     Equivalent to `li'.
       x     Matches a hexadecimal integer.
       X     Matches a long hexadecimal integer (same as `lx').
       e,f,g Matches a decimal floating point number (Float_Type).
       E,F,G Matches a double precision floating point number, same as `lf'.
       s     Matches a string of non-whitespace characters (String_Type).
       c     Matches one character.  If width is given, width
             characters are matched.
       n     Assigns the number of characters scanned so far.
       [...] Matches zero or more characters from the set of characters
             enclosed by the square brackets.  If '^' is given as the
             first character, then the complement set is matched.
#v-
\example
 Suppose that \exmp{s} is \exmp{"Coffee: (3,4,12.4)"}.  Then
#v+
    n = sscanf (s, "%[a-zA-Z]: (%d,%d,%lf)", &item, &x, &y, &z);
#v-
 will set \exmp{n} to \4, \exmp{item} to \exmp{"Coffee"}, \exmp{x} to \3,
 \exmp{y} to \4, and \exmp{z} to the double precision number
 \exmp{12.4}.  However,
#v+
    n = sscanf (s, "%s: (%d,%d,%lf)", &item, &x, &y, &z);
#v-
 will set \exmp{n} to \1, \exmp{item} to \exmp{"Coffee:"} and the
 remaining variables will not be assigned.
\seealso{sprintf, unpack, string, atof, int, integer, string_matches}
\done

\function{strbytelen}
\synopsis{Get the number of bytes in a string}
\usage{Int_Type strbytelen (String_Type s)}
\description
  This function returns the number of bytes in a string.  In UTF-8
  mode, this value is generally different from the number of
  characters in a string.  For the latter information, the
  \ifun{strlen} or \ifun{strcharlen} functions should be used.
\notes
  \notearray_int_f_str
\seealso{strlen, strcharlen, length}
\done

\function{strbytesub}
\synopsis{Replace a byte with another in a string.}
\usage{String_Type strsub (String_Type s, Int_Type pos, UChar_Type b)}
\description
  The \ifun{strbytesub} function may be be used to substitute the byte
  \exmp{b} for the byte at byte position \exmp{pos} of the string
  \exmp{s}.  The resulting string is returned.
\notes
  The first byte in the string \exmp{s} is specified by \exmp{pos}
  equal to 1.  This function uses byte semantics, not character
  semantics.
\seealso{strsub, is_substr, strreplace, strbytelen}
\done

\function{strcat}
\synopsis{Concatenate strings}
\usage{String_Type strcat (String_Type a_1, ...,  String_Type a_N)}
\description
   The \ifun{strcat} function concatenates its N string
   arguments \exmp{a_1}, ... \exmp{a_N} together and returns the result.
\example
#v+
    strcat ("Hello", " ", "World");
#v-
   produces the string \exmp{"Hello World"}.
\notes
   This function is equivalent to the binary operation \exmp{a_1+...+a_N}.
   However, \ifun{strcat} is much faster making it the preferred method
   to concatenate strings.
\seealso{sprintf, strjoin}
\done

\function{strcharlen}
\synopsis{Get the number of characters in a string including combining characters}
\usage{Int_Type strcharlen (String_Type s)}
\description
  The \ifun{strcharlen} function returns the number of characters in a
  string.  If the string contains combining characters, then they are
  also counted.  Use the \ifun{strlen} function to obtain the
  character count ignoring combining characters.
\notes
  \notearray_int_f_str
\seealso{strlen, strbytelen}
\done

\function{strchop}
\synopsis{Chop or split a string into substrings.}
\usage{String_Type[] strchop (String_Type str, Int_Type delim, Int_Type quote)}
\description
   The \ifun{strchop} function may be used to split-up a string
   \exmp{str} that consists of substrings delimited by the character
   specified by \exmp{delim}.  If the integer \exmp{quote} is non-zero,
   it will be taken as a quote character for the delimiter.  The
   function returns the substrings as an array.
\example
   The following function illustrates how to sort a comma separated
   list of strings:
#v+
     define sort_string_list (a)
     {
        variable i, b, c;
        b = strchop (a, ',', 0);

        i = array_sort (b);
        b = b[i];   % rearrange

        % Convert array back into comma separated form
        return strjoin (b, ",");
     }
#v-
\seealso{strchopr, strjoin, strtok}
\done

\function{strchopr}
\synopsis{Chop or split a string into substrings.}
\usage{String_Type[] strchopr (String_Type str, String_Type delim, String_Type quote)}
\description
  This routine performs exactly the same function as \ifun{strchop} except
  that it returns the substrings in the reverse order.  See the
  documentation for \ifun{strchop} for more information.
\seealso{strchop, strtok, strjoin}
\done

\function{strcmp}
\synopsis{Compare two strings}
\usage{Int_Type strcmp (String_Type a, String_Type b)}
\description
  The \ifun{strcmp} function may be used to perform a case-sensitive
  string comparison, in the lexicographic sense, on strings \exmp{a}
  and \exmp{b}.  It returns 0 if the strings are identical, a negative
  integer if \exmp{a} is less than \exmp{b}, or a positive integer if
  \exmp{a} is greater than \exmp{b}.
\example
  The \ifun{strup} function may be used to perform a case-insensitive
  string comparison:
#v+
    define case_insensitive_strcmp (a, b)
    {
      return strcmp (strup(a), strup(b));
    }
#v-
\notes
  One may also use one of the binary comparison operators, e.g.,
  \exmp{a > b}.

  \notearray_int_f_str
\seealso{strup, strncmp}
\done

\function{strcompress}
\synopsis{Remove excess whitespace characters from a string}
\usage{String_Type strcompress (String_Type s, String_Type white)}
\description
  The \ifun{strcompress} function compresses the string \exmp{s} by
  replacing a sequence of one or more characters from the set
  \exmp{white} by the first character of \exmp{white}. In addition, it
  also removes all leading and trailing characters from \exmp{s} that
  are part of \exmp{white}.
\example
  The expression
#v+
    strcompress (",;apple,,cherry;,banana", ",;");
#v-
  returns the string \exmp{"apple,cherry,banana"}.
\notes
  \notearray_str_f_str_arg1
\seealso{strtrim, strtrans, str_delete_chars}
\done

\function{string_match}
\synopsis{Match a string against a regular expression}
\usage{Int_Type string_match(String_Type str, String_Type pat [,Int_Type pos])}
\description
  The \ifun{string_match} function returns zero if \exmp{str} does not
  match regular expression specified by \exmp{pat}.  This function
  performs the match starting at the first byte of the string.  The
  optional \exmp{pos} argument may be used to specify a different byte
  offse (numbered from 1).  This function returns the position in
  bytes (numbered from 1) of the start of the match in \exmp{str}.
  The exact substring matched may be found using
  \ifun{string_match_nth}.
\notes
  Positions in the string are specified using byte-offsets not
  character offsets. The value returned by this function is measured
  from the beginning of the string \exmp{str}.

  The function is not yet UTF-8 aware.  If possible, consider using
  the \module{pcre} module for better, more sophisticated regular
  expressions.

  The \exmp{pos} argument was made optional in version 2.2.3.
\seealso{string_matches, string_match_nth, strcmp, strncmp}
\done

\function{string_match_nth}
\synopsis{Get the result of the last call to string_match}
\usage{(Int_Type pos, Int_Type len) = string_match_nth(Int_Type nth)}
\description
  The \ifun{string_match_nth} function returns two integers describing
  the result of the last call to \ifun{string_match}.  It returns both
  the zero-based byte-position of the \exmp{nth} submatch and
  the length of the match.

  By convention, \exmp{nth} equal to zero means the entire match.
  Otherwise, \exmp{nth} must be an integer with a value 1 through 9,
  and refers to the set of characters matched by the \exmp{nth} regular
  expression enclosed by the pairs \exmp{\\(, \\)}.
\example
  Consider:
#v+
     variable matched, pos, len;
     matched = string_match("hello world", "\([a-z]+\) \([a-z]+\)"R, 1);
     if (matched)
       (pos, len) = string_match_nth(2);
#v-
  This will set \exmp{matched} to 1 since a match will be found at the
  first byte position, \exmp{pos} to 6 since \exmp{w} is offset 6 bytes
  from the beginning of the string, and \exmp{len} to 5 since
  \exmp{"world"} is 5 bytes long.
\notes
  The position offset is \em{not} affected by the value of the offset
  parameter to the \ifun{string_match} function. For example, if the
  value of the last parameter to the \ifun{string_match} function had
  been 3, \exmp{pos} would still have been set to 6.

  The \sfun{string_matches} function may be used as an alternative to
  \sfun{string_match_nth}.
\seealso{string_match, string_matches}
\done

\function{string_matches}
\synopsis{Match a string against a regular expression and return the matches}
\usage{String_Type[] string_matches(String_Type str, String_Type pat [,Int_Type pos])}
\description
  The \ifun{string_matches} function combines the functionality of
  \ifun{string_match} and \ifun{string_match_nth}.  Like
  \ifun{string_match}, it matches the test string \exmp{str} against
  the regular expression \exmp{pat}.  If the string does not match the
  pattern the function will return \NULL.  Otherwise, the function
  will return an array of strings whose \exmp{ith} element is the string that
  corresponds to the return value of the \ifun{string_match_nth}
  function.
\example
#v+
    strs = string_matches ("p0.5keV_27deg.dat",
                           "p\([0-9.]+\)keV_\([0-9.]+\)deg\.dat"R, 1);
    % ==> strs[0] = "p0.5keV_27deg.dat"
    %     strs[1] = "0.5"
    %     strs[2] = "27"

    strs = string_matches ("q0.5keV_27deg.dat",
                           "p\([0-9.]+\)keV_\([0-9.]+\)deg\.dat"R);
    % ==> strs = NULL
#v-
\notes
  The function is not yet UTF-8 aware.  If possible, consider using
  the \module{pcre} module for better, more sophisticated regular
  expressions.

  The \exmp{pos} argument was made optional in version 2.2.3.
\seealso{string_match, string_match_nth, strcmp, strncmp}
\done

\function{strjoin}
\synopsis{Concatenate elements of a string array}
\usage{String_Type strjoin (Array_Type a [, String_Type delim])}
\description
   The \ifun{strjoin} function operates on an array of strings by joining
   successive elements together separated with the optional delimiter
   \exmp{delim}.  If \exmp{delim} is not specified, then empty string
   \exmp{""} will be used resulting in a concatenation of the elements.
\example
   Suppose that
#v+
      days = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat","Sun"];
#v-
   Then \exmp{strjoin (days,"+")} will produce
   \exmp{"Sun+Mon+Tue+Wed+Thu+Fri+Sat+Sun"}.  Similarly,
   \exmp{strjoin (["","",""], "X")} will produce \exmp{"XX"}.
\seealso{strchop, strcat}
\done

\function{strlen}
\synopsis{Compute the length of a string}
\usage{Int_Type strlen (String_Type a)}
\description
  The \ifun{strlen} function may be used to compute the character
  length of a string ignoring the presence of combining characters.
  The \ifun{strcharlen} function may be used to count combining
  characters as distinct characters.  For byte-semantics, use the
  \ifun{strbytelen} function.
\example
  After execution of
#v+
   variable len = strlen ("hello");
#v-
  \exmp{len} will have a value of \exmp{5}.
\notes
  \notearray_int_f_str
\seealso{strbytelen, strcharlen, bstrlen, length, substr}
\done

\function{strlow}
\synopsis{Convert a string to lowercase}
\usage{String_Type strlow (String_Type s)}
\description
  The \ifun{strlow} function takes a string \exmp{s} and returns another
  string identical to \exmp{s} except that all upper case characters
  that are contained in \exmp{s} are converted converted to lower case.
\example
  The function
#v+
    define Strcmp (a, b)
    {
      return strcmp (strlow (a), strlow (b));
    }
#v-
  performs a case-insensitive comparison operation of two strings by
  converting them to lower case first.
\notes
  \notearray_str_f_str
\seealso{strup, tolower, strcmp, strtrim, define_case}
\done

\function{strnbytecmp}
\synopsis{Compare the first n bytes of two strings}
\usage{Int_Type strnbytecmp (String_Type a, String_Type b, Int_Type n)}
\description
  This function compares the first \exmp{n} bytes of the strings
  \exmp{a} and \exmp{b}.  See the documentation for \ifun{strcmp} for
  information about the return value.
\notes
  \notearray_int_f_str2
\seealso{strncmp, strncharcmp, strcmp}
\done

\function{strncharcmp}
\synopsis{Compare the first n characters of two strings}
\usage{Int_Type strncharcmp (String_Type a, String_Type b, Int_Type n)}
\description
  This function compares the first \exmp{n} characters of the strings
  \exmp{a} and \exmp{b} counting combining characters as distinct
  characters.  See the documentation for \ifun{strcmp} for information
  about the return value.
\notes
  \notearray_int_f_str2
\seealso{strncmp, strnbytecmp, strcmp}
\done

\function{strncmp}
\synopsis{Compare the first few characters of two strings}
\usage{Int_Type strncmp (String_Type a, String_Type b, Int_Type n)}
\description
  This function behaves like \ifun{strcmp} except that it compares only the
  first \exmp{n} characters in the strings \exmp{a} and \exmp{b}.
  See the documentation for \ifun{strcmp} for information about the return
  value.

  In counting characters, combining characters are not counted,
  although they are used in the comparison.  Use the
  \ifun{strncharcmp} function if you want combining characters to be
  included in the character count.  The \ifun{strnbytecmp} function
  should be used to compare bytes.
\example
  The expression
#v+
     strncmp ("apple", "appliance", 3);
#v-
  will return zero since the first three characters match.
\notes
  This function uses character semantics.

  \notearray_int_f_str2
\seealso{strcmp, strlen, strncharcmp, strnbytecmp}
\done

\function{strreplace}
\synopsis{Replace one or more substrings}
\usage{(new,n) = strreplace(a, b, c, max_n)}
\altusage{new = strreplace(a, b, c)}
\description
  The \ifun{strreplace} function may be used to replace one or more
  occurrences of \exmp{b} in \exmp{a} with \exmp{c}.  This function
  supports two calling interfaces.

  The first form may be used to replace a specified number of
  substrings.  If \exmp{max_n} is positive, then the first
  \exmp{max_n} occurrences of \exmp{b} in \exmp{a} will be replaced.
  Otherwise, if \exmp{max_n} is negative, then the last
  \exmp{abs(max_n)} occurrences will be replaced. The function returns
  the resulting string and an integer indicating how many replacements
  were made.

  The second calling form may be used to replace all occurrences of
  \exmp{b} in \exmp{a} with \exmp{c}.  In this case, only the
  resulting string will be returned.

\example
  The following function illustrates how \ifun{strreplace} may be used
  to remove all occurrences of a specified substring:
#v+
     define delete_substrings (a, b)
     {
        return strreplace (a, b, "");
     }
#v-
\seealso{is_substr, strsub, strtrim, strtrans, str_delete_chars}
\done

\function{strskipbytes}
\synopsis{Skip a range of bytes in a byte string}
\usage{Int_Type strskipbytes (str, range [n0 [,nmax]])}
#v+
   String_Type s;
   String_Type range;
   Int_Type n0, nmax;
#v-
\description
  This function skips over a range of bytes in a string \exmp{str}.
  The byte range to be skipped is specified by the \exmp{range}
  parameter.  Optional start (\exmp{n0}) and stop (\exmp{nmax})
  (0-based) parameters may be used to specifiy the part of the input
  string to be processed.  The function returns a 0-based offset from
  the beginning of the string where processing stopped.

  See the documentation for the \ifun{strtrans} function for the
  format of the range parameter.
\seealso{strskipchar, strbskipchar, strtrans}
\done

\function{strskipchar}
\synopsis{Get an index to the next character in a UTF-8 encoded string}
\usage{(p1, wch) = strskipchar (str, p0 [,skip_combining])}
\description
  This function decodes the character at the 0-based byte-offset \exmp{p0} in
  the string \exmp{str}.  It returns the byte-offset (\exmp{p1} of the next
  character in the string and the decoded character at byte-offset
  \exmp{p0}.

  The optional third argument specifies the handling of
  combining characters.  If it is non-zero, combining characters will
  be ignored, otherwise a combining character will not be treated
  differently from other characters.  The default is to ignore such
  characters.

  If the byte-offset \exmp{p0} corresponds to the end of the string,
  then \exmp{(p0,0)} will be returned.  Otherwise if the byte-offset
  specifies a value that lies outside the string, an \exc{IndexError}
  exception will be thrown.  Finally, if the byte-offset corresponds
  to an illegally coded character, the character returned will be the
  negative byte-value at the position.
\example
  The following is an example of a function that skips alphanumeric
  characters and returns the new byte-offset.
#v+
    private define skip_word_chars (line, p)
    {
       variable p1 = p, ch;
       do
         {
            p = p1;
            (p1, ch) = strskipchar (line, p, 1);
          }
       while (isalnum(ch));
       return p;
    }
#v-
\notes
  In non-UTF-8 mode (\exmp{_slang_utf8_ok=0}), this function is
  equivalent to:
#v+
     define strskipchar (s, p)
     {
        if ((p < 0) || (p > strlen(s)))
          throw IndexError;
        if (p == strlen(s))
          return (p, s[p])
        return (p+1, s[p]);
     }
#v-
  It is important to understand that the above code relies upon
  byte-semantics, which are invalid for multi-byte characters.
\seealso{strbskipchar, strskipbytes}
\done

\function{strsub}
\synopsis{Replace a character with another in a string.}
\usage{String_Type strsub (String_Type s, Int_Type pos, Int_Type ch)}
\description
  The \ifun{strsub} function may be used to substitute the character
  \exmp{ch} for the character at character position \exmp{pos} of the string
  \exmp{s}.  The resulting string is returned.
\example
#v+
    define replace_spaces_with_comma (s)
    {
      variable n;
      while (n = is_substr (s, " "), n) s = strsub (s, n, ',');
      return s;
    }
#v-
  For uses such as this, the \ifun{strtrans} function is a better choice.
\notes
  The first character in the string \exmp{s} is specified by \exmp{pos}
  equal to 1.  This function uses character semantics, not byte
  semantics.
\seealso{is_substr, strreplace, strlen}
\done

\function{strtok}
\synopsis{Extract tokens from a string}
\usage{String_Type[] strtok (String_Type str [,String_Type white])}
\description
  \ifun{strtok} breaks the string \exmp{str} into a series of tokens
  and returns them as an array of strings.  If the second parameter
  \exmp{white} is present, then it specifies the set of characters
  that are to be regarded as whitespace when extracting the tokens,
  and may consist of the whitespace characters or a range of such
  characters. If the first character of \exmp{white} is \exmp{'^'},
  then the whitespace characters consist of all characters except
  those in \exmp{white}.  For example, if \exmp{white} is \exmp{"
  \\t\\n,;."}, then those characters specify the whitespace
  characters.  However, if \exmp{white} is given by
  \exmp{"^a-zA-Z0-9_"}, then any character is a whitespace character
  except those in the ranges \exmp{a-z}, \exmp{A-Z}, \exmp{0-9}, and
  the underscore character.  To specify the hyphen character as a
  whitespace character, then it should be the first character of the
  whitespace string.  In addition to ranges, the whitespace specifier
  may also include character classes:
#v+
    \w matches a unicode "word" character, taken to be alphanumeric.
    \a alphabetic character, excluding digits
    \s matches whitespace
    \l matches lowercase
    \u matches uppercase
    \d matches a digit
    \\ matches a backslash
    \^ matches a ^ character
#v-

  If the second parameter is not present, then it defaults to
  \exmp{"\\s"}.
\example
  The following example may be used to count the words in a text file:
#v+
    define count_words (file)
    {
       variable fp, line, count;

       fp = fopen (file, "r");
       if (fp == NULL) return -1;

       count = 0;
       while (-1 != fgets (&line, fp))
         {
           line = strtok (line, "^\\a");
           count += length (line);
         }
       () = fclose (fp);
       return count;
    }
#v-
  Here a word was assumed to consist only of alphabetic characters.
\seealso{strchop, strcompress, strjoin}
\done

\function{strtrans}
\synopsis{Replace characters in a string}
\usage{String_Type strtrans (str, old_set, new_set)}
#v+
   String_Type str, old_set, new_set;
#v-
\description
  The \ifun{strtrans} function may be used to replace all the characters
  from the set \exmp{old_set} with the corresponding characters from
  \exmp{new_set} in the string \exmp{str}.  If \exmp{new_set} is empty,
  then the characters in \exmp{old_set} will be removed from \exmp{str}.

  If \exmp{new_set} is not empty, then \exmp{old_set} and
  \exmp{new_set} must be commensurate.  Each set may consist of
  character ranges such as \exmp{A-Z} and character classes:
#v+
    \, matches a punctuation character
    \7 matches any 7bit ascii character
    \\ matches a backslash
    \^ matches the ^ character
    \a matches an alphabetic character, excluding digits
    \c matches a control character
    \d matches a digit
    \g matches a graphic character
    \l matches lowercase
    \p matches a printable character
    \s matches whitespace
    \u matches uppercase
    \w matches a unicode "word" character, taken to be alphanumeric.
    \x matches hex digit (a-fA-F0-9)
#v-
  If the first character of a set is \exmp{^} then the set is taken to
  be the complement set.
\example
#v+
    str = strtrans (str, "\\u", "\\l");   % lower-case str
    str = strtrans (str, "^0-9", " ");    % Replace anything but 0-9 by space
    str = strtrans (str, "\\^0-9", " ");  % Replace '^' and 0-9 by a space
#v-
\notes
  \notearray_str_f_str_arg1
\seealso{strreplace, strtrim, strup, strlow}
\done

\function{strtrim}
\synopsis{Remove whitespace from the ends of a string}
\usage{String_Type strtrim (String_Type s [,String_Type w])}
\description
  The \ifun{strtrim} function removes all leading and trailing whitespace
  characters from the string \exmp{s} and returns the result.  The
  optional second parameter specifies the set of whitespace
  characters.  If the argument is not present, then the set defaults
  to \exmp{"\\s"}.  The whitespace specification may consist of
  character ranges such as \exmp{A-Z} and character classes:
#v+
    \w matches a unicode "word" character, taken to be alphanumeric.
    \a alphabetic character, excluding digits
    \s matches whitespace
    \l matches lowercase
    \u matches uppercase
    \d matches a digit
    \\ matches a backslash
    \^ matches a ^ character
#v-
  If the first character of a set is \exmp{^} then the set is taken to
  be the complement set.
\notes
  \notearray_strtrim
\seealso{strtrim_beg, strtrim_end, strcompress}
\done

\function{strtrim_beg}
\synopsis{Remove leading whitespace from a string}
\usage{String_Type strtrim_beg (String_Type s [,String_Type w])}
\description
  The \ifun{strtrim_beg} function removes all leading whitespace
  characters from the string \exmp{s} and returns the result.
  The optional second parameter specifies the set of whitespace
  characters.  See the documentation for the \ifun{strtrim} function
  form more information about the whitespace parameter.
\notes
  \notearray_strtrim
\seealso{strtrim, strtrim_end, strcompress}
\done

\function{strtrim_end}
\synopsis{Remove trailing whitespace from a string}
\usage{String_Type strtrim_end (String_Type s [,String_Type w])}
\description
  The \ifun{strtrim_end} function removes all trailing whitespace
  characters from the string \exmp{s} and returns the result.  The
  optional second parameter specifies the set of whitespace
  characters.  See the documentation for the \ifun{strtrim} function
  form more information about the whitespace parameter.
\notes
  \notearray_strtrim
\seealso{strtrim, strtrim_beg, strcompress}
\done

\function{strup}
\synopsis{Convert a string to uppercase}
\usage{String_Type strup (String_Type s)}
\description
  The \ifun{strup} function takes a string \exmp{s} and returns another
  string identical to \exmp{s} except that all lower case characters
  that contained in \exmp{s} are converted to upper case.
\example
  The function
#v+
    define Strcmp (a, b)
    {
      return strcmp (strup (a), strup (b));
    }
#v-
  performs a case-insensitive comparison operation of two strings by
  converting them to upper case first.
\notes
  \notearray_str_f_str
\seealso{strlow, toupper, strcmp, strtrim, define_case, strtrans}
\done

\function{str_delete_chars}
\synopsis{Delete characters from a string}
\usage{String_Type str_delete_chars (String_Type str [, String_Type del_set])}
\description
  This function may be used to delete the set of characters specified
  by the optional argument \exmp{del_set} from the string \exmp{str}.
  If \exmp{del_set} is not given, \exmp{"\\s"} will be used.
  The modified string is returned.

  The set of characters to be deleted may include ranges such as
  \exmp{A-Z} and characters classes:
#v+
    \w matches a unicode "word" character, taken to be alphanumeric.
    \a alphabetic character, excluding digits
    \s matches whitespace
    \l matches lowercase
    \u matches uppercase
    \d matches a digit
    \\ matches a backslash
    \^ matches a ^ character
#v-
  If the first character of \exmp{del_set} is \exmp{^}, then the set
  is taken to be the complement of the remaining string.
\example
#v+
    str = str_delete_chars (str, "^A-Za-z");
#v-
  will remove all characters except \exmp{A-Z} and \exmp{a-z} from
  \exmp{str}.  Similarly,
#v+
    str = str_delete_chars (str, "^\\a");
#v-
  will remove all but the alphabetic characters.
\notes
  \notearray_str_f_str_arg1
\seealso{strtrans, strreplace, strcompress}
\done

\function{str_quote_string}
\synopsis{Escape characters in a string.}
\usage{String_Type str_quote_string(String_Type str, String_Type qlis, Int_Type quote)}
\description
  The \ifun{str_quote_string} returns a string identical to \exmp{str}
  except that all characters contained in the string \exmp{qlis} are
  escaped with the \exmp{quote} character, including the quote
  character itself.  This function is useful for making a string that
  can be used in a regular expression.
\example
  Execution of the statements
#v+
   node = "Is it [the coat] really worth $100?";
   tag = str_quote_string (node, "\\^$[]*.+?", '\\');
#v-
  will result in \exmp{tag} having the value:
#v+
    Is it \[the coat\] really worth \$100\?
#v-
\seealso{str_uncomment_string, make_printable_string}
\done

\function{str_replace}
\synopsis{Replace a substring of a string (deprecated)}
\usage{Int_Type str_replace (String_Type a, String_Type b, String_Type c)}
\description
  The \ifun{str_replace} function replaces the first occurrence of \exmp{b} in
  \exmp{a} with \exmp{c} and returns an integer that indicates whether a
  replacement was made.  If \exmp{b} does not occur in \exmp{a}, zero is
  returned.  However, if \exmp{b} occurs in \exmp{a}, a non-zero integer is
  returned as well as the new string resulting from the replacement.
\notes
  This function has been superceded by \ifun{strreplace}.  It should no
  longer be used.
\seealso{strreplace}
\done

\function{str_uncomment_string}
\synopsis{Remove comments from a string}
\usage{String_Type str_uncomment_string(String_Type s, String_Type beg, String_Type end)}
\description
  This function may be used to remove simple forms of comments from a
  string \exmp{s}. The parameters, \exmp{beg} and \exmp{end}, are strings
  of equal length whose corresponding characters specify the begin and
  end comment characters, respectively.  It returns the uncommented
  string.
\example
  The expression
#v+
     str_uncomment_string ("Hello (testing) 'example' World", "'(", "')")
#v-
  returns the string \exmp{"Hello   World"}.
\notes
  This routine does not handle multi-character comment delimiters and it
  assumes that comments are not nested.
\seealso{str_quote_string, str_delete_chars, strtrans}
\done

\function{substr}
\synopsis{Extract a substring from a string}
\usage{String_Type substr (String_Type s, Int_Type n, Int_Type len)}
\description
  The \ifun{substr} function returns a substring with character length
  \exmp{len} of the string \exmp{s} beginning at the character position
  \exmp{n}.  If \exmp{len} is \exmp{-1}, the entire length of the string
  \exmp{s} will be used for \exmp{len}.  The first character of \exmp{s}
  is given by \exmp{n} equal to 1.
\example
#v+
     substr ("To be or not to be", 7, 5);
#v-
  returns \exmp{"or no"}
\notes
  In many cases it is more convenient to use array indexing rather
  than the \ifun{substr} function.  In fact, if UTF-8 mode is not in
  effect, \exmp{substr(s,i+1,strlen(s))} is equivalent to
  \exmp{s[[i:]]}.  Array indexing uses byte-semantics, not character
  semantics assumed by the \ifun{substr} function.
\seealso{is_substr, substrbytes, strlen}
\done

\function{substrbytes}
\synopsis{Extract a byte sequence from a string}
\usage{String_Type substrbytes (String_Type s, Int_Type n, Int_Type len)}
\description
  The \ifun{substrbytes} function returns a substring with byte length
  \exmp{len} of the string \exmp{s} beginning at the byte position
  \exmp{n}, counting from 1.  If \exmp{len} is \exmp{-1}, the entire
  byte-length of the string \exmp{s} will be used for \exmp{len}.  The first
  byte of \exmp{s} is given by \exmp{n} equal to 1.
\example
#v+
     substrbytes ("To be or not to be", 7, 5);
#v-
  returns \exmp{"or no"}
\notes
  In many cases it is more convenient to use array indexing rather
  than the \ifun{substr} function.  In fact
  \exmp{substrbytes(s,i+1,-1)} is equivalent to
  \exmp{s[[i:]]}.

  The function \ifun{substr} may be used if character semantics are
  desired.
\seealso{substr, strbytelen}
\done

