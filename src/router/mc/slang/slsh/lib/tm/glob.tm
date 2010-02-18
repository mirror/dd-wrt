\function{glob}
\synopsis{Find files using wildcards}
\usage{files = glob (pattern1, ..., patternN)};
\description
  This function returns a list of files whose names match the specified
  globbing patterns.  A globbing pattern is one in which '?' matches a
  single character, and '*' matches 0 or more characters.
\example
#v+
   files = glob ("*.c", "*.h");
#v-
\seealso{glob_to_regexp}
\done
