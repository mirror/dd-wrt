\chapter{Regular Expressions}

 The S-Lang library includes a regular expression (RE) package that
 may be used by an application embedding the library.  The RE syntax
 should be familiar to anyone acquainted with regular expressions.  In
 this section the syntax of the \slang regular expressions is
 discussed.

 NOTE: At the moment, the \slang regular expressions do not support
 UTF-8 encoded strings.  The \slang library will most likely migrate
 to the use of the PCRE regular expression library, deprecating the
 use of the \slang REs in the process.  For these reasons, the user is
 encouraged to make use of the \module{pcre} module if possible.

\sect{\slang RE Syntax}

 A regular expression specifies a pattern to be matched against a
 string, and has the property that the contcatenation of two REs is
 also a RE.

 The \slang library supports the following standard regular
 expressions:
#v+
   .                 match any character except newline
   *                 matches zero or more occurrences of previous RE
   +                 matches one or more occurrences of previous RE
   ?                 matches zero or one occurrence of previous RE
   ^                 matches beginning of a line
   $                 matches end of line
   [ ... ]           matches any single character between brackets.
                     For example, [-02468] matches `-' or any even digit.
                     and [-0-9a-z] matches `-' and any digit between 0 and 9
                     as well as letters a through z.
   \<                Match the beginning of a word.
   \>                Match the end of a word.
   \( ... \)
   \1, \2, ..., \9   Matches the match specified by nth \( ... \)
                     expression.
#v-
 In addition the following extensions are also supported:
#v+
   \c                turn on case-sensitivity (default)
   \C                turn off case-sensitivity
   \d                match any digit
   \e                match ESC char
#v-
Here are some simple examples:

  \exmp{"^int "} matches the \exmp{"int "} at the beginning of a line.

  \exmp{"\\<money\\>"} matches \exmp{"money"} but only if it appears
  as a separate word.

  \exmp{"^$"} matches an empty line.

 A more complex pattern is
#v+
  "\(\<[a-zA-Z]+\>\)[ ]+\1\>"
#v-
 which matches any word repeated consecutively.  Note how the grouping
 operators \exmp{\\(} and \exmp{\\)} are used to define the text
 matched by the enclosed regular expression, and then subsequently
 referred to \exmp{\\1}.

 Finally, remember that when used in string literals either in the
 \slang language or in the C language, care must be taken to
 "double-up" the \exmp{'\\'} character since both languages treat it
 as an escape character.

\sect{Differences between \slang and egrep REs}

 There are several differences between \slang regular expressions and,
 e.g., \bf{egrep} regular expressions.

 The most notable difference is that the \slang regular expressions do
 not support the \bf{OR} operator \exmp{|} in expressions.  This means
 that \exmp{"a|b"} or \exmp{"a\\|b"} do not have the meaning that they
 have in regular expression packages that support egrep-style
 expressions.

 The other main difference is that while \slang regular expressions
 support the grouping operators \exmp{\\(} and \exmp{\\)}, they are
 only used as a means of specifying the text that is matched.  That
 is, the expression
#v+
     "@\([a-z]*\)@.*@\1@"
#v-
 matches \exmp{"xxx@abc@silly@abc@yyy"}, where the pattern \exmp{\\1}
 matches the text enclosed by the \exmp{\\(} and \exmp{\\)}
 expressions. However, in the current implementation, the grouping
 operators are not used to group regular expressions to form a single
 regular expression.  Thus expression such as \exmp{"\\(hello\\)*"} is
 \em{not} a pattern to match zero or more occurrences of \exmp{"hello"}
 as it is in e.g., \bf{egrep}.

 One question that comes up from time to time is why doesn't \slang
 simply employ some posix-compatible regular expression library.  The
 simple answer is that, at the time of this writing, none exists that
 is available across all the platforms that the \slang library
 supports (Unix, VMS, OS/2, win32, win16, BEOS, MSDOS, and QNX) and
 can be distributed under both the GNU licenses.  It is particularly
 important that the library and the interpreter support a common set
 of regular expressions in a platform independent manner.

