\function{SLwchar_toupper}
\synopsis{Uppercase a Unicode character}
\usage{SLwchar_Type SLwchar_toupper (SLwchar_Type wc)}
\description
 \cfun{SLwchar_toupper} returns the uppercase equivalent of the
 specified character.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_tolower, SLwchar_isupper, SLutf8_strup}
\done

\function{SLwchar_tolower}
\synopsis{Lowercase a Unicode character}
\usage{SLwchar_Type SLwchar_tolower (SLwchar_Type wc)}
\description
 \cfun{SLwchar_tolower} returns the lowercase equivalent of the
 specified character.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_toupper, SLwchar_islower, SLutf8_strlow}
\done

\function{SLwchar_wcwidth}
\synopsis{Determine the displayable width of a wide character}
\usage{int SLwchar_wcwidth (SLwchar_Type wc)}
\description
 This function returns the number of columns necessary to display the
 specified Unicode character.  Combining characters are meant to be
 combined with other characters and, as such, have 0 width.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isspace, SLwchar_iscntrl}
\done

\function{SLwchar_isalnum}
\synopsis{Determine if a Unicode character is alphanumeric}
\usage{int SLwchar_isalnum (SLwchar_Type wc)}
\description
  \cfun{SLwchar_isalnum} returns a non-zero value if the Unicode
  character is alphanumeric, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isalpha, SLwchar_isdigit, SLwchar_iscntrl}
\done

\function{SLwchar_isalpha}
\synopsis{Determine if a Unicode character is an alphabetic character}
\usage{int SLwchar_isalpha (SLwchar_Type wc)}
\description
  \cfun{SLwchar_isalpha} returns a non-zero value if the Unicode
  character is alphabetic, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isalnum, SLwchar_isalpha, SLwchar_isdigit, SLwchar_iscntrl}
\done

\function{SLwchar_isblank}
\synopsis{Determine if a Unicode character is a blank}
\usage{int SLwchar_isblank (SLwchar_Type wc)}
\description
  \cfun{SLwchar_isblank} returns a non-zero value if the Unicode
  character is a blank one (space or tab), otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit, SLwchar_iscntrl}
\done

\function{SLwchar_iscntrl}
\synopsis{Determine if a Unicode character is a control character}
\usage{int SLwchar_iscntrl (SLwchar_Type wc)}
\description
  \cfun{SLwchar_isblank} returns a non-zero value if the Unicode
  character is a control character, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit, SLwchar_isprint}
\done

\function{SLwchar_isdigit}
\synopsis{Determine if a Unicode character is a digit}
\usage{int SLwchar_isdigit (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a digit, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isspace, SLwchar_isalpha, SLwchar_isxdigit, SLwchar_isprint}
\done

\function{SLwchar_isgraph}
\synopsis{Determine if a non-space Unicode character is printable}
\usage{int SLwchar_isgraph (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a non-space printable character, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit, SLwchar_isprint}
\done

\function{SLwchar_islower}
\synopsis{Determine if a Unicode character is alphanumeric}
\usage{int SLwchar_islower (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a lowercase one, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isupper, SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit, SLwchar_iscntrl}
\done

\function{SLwchar_isprint}
\synopsis{Determine if a Unicode character is printable}
\usage{int SLwchar_isprint (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a printable one (includes space), otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isgraph, SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit}
\done

\function{SLwchar_ispunct}
\synopsis{Determine if a Unicode character is a punctuation character}
\usage{int SLwchar_ispunct (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a punctuation character, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit, SLwchar_isprint}
\done

\function{SLwchar_isspace}
\synopsis{Determine if a Unicode character is a whitespace character}
\usage{int SLwchar_isspace (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a whitespace character, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isblank, SLwchar_isalpha, SLwchar_isdigit, SLwchar_ispunct}
\done

\function{SLwchar_isupper}
\synopsis{Determine if a Unicode character is uppercase}
\usage{int SLwchar_isupper (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is an uppercase character, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_islower, SLwchar_isspace, SLwchar_isalpha, SLwchar_isdigit}
\done

\function{SLwchar_isxdigit}
\synopsis{Determine if a Unicode character is a hexidecimal digit}
\usage{int SLwchar_isxdigit (SLwchar_Type wc)}
\description
  This function returns a non-zero value if the specified Unicode
  character is a hexadecimal digit character, otherwise it returns 0.
\notes
  If the library is not in UTF-8 mode, then the current locale will be
  used.
\seealso{SLwchar_isdigit, SLwchar_isspace, SLwchar_isalpha, SLwchar_ispunct}
\done
