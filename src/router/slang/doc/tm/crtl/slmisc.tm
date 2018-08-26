\function{SLcurrent_time_string}
\synopsis{Get the current time as a string}
\usage{char *SLcurrent_time_string (void)}
\description
  The \var{SLcurrent_time_string} function uses the C library function
  \var{ctime} to obtain a string representation of the
  current date and time in the form
#v+
     "Wed Dec 10 12:50:28 1997"
#v-
  However, unlike the \var{ctime} function, a newline character is not
  present in the string.

  The returned value points to a statically allocated memory block
  which may get overwritten on subsequent function calls.
\seealso{SLmake_string}
\done

\function{SLatoi}
\synopsis{Convert a text string to an integer}
\usage{int SLatoi(unsigned char *str}
\description
  \var{SLatoi} parses the string \var{str} to interpret it as an
  integer value.  Unlike \var{atoi}, \var{SLatoi} can also parse
  strings containing integers expressed in
  hexidecimal (e.g., \exmp{"0x7F"}) and octal (e.g., \exmp{"012"}.)
  notation.
\seealso{SLang_guess_type}
\done

\function{SLextract_list_element}
\synopsis{Extract a substring of a delimited string}
\usage{int SLextract_list_element (dlist, nth, delim, buf, buflen)}
#v+
    char *dlist;
    unsigned int nth;
    char delim;
    char *buf;
    unsigned int buflen;
#v-
\description
  \var{SLextract_list_element} may be used to obtain the \var{nth}
  element of a list of strings, \var{dlist}, that are delimited by the
  character \var{delim}.  The routine copies the \var{nth} element of
  \var{dlist} to the buffer \var{buf} whose size is \var{buflen}
  characters.  It returns zero upon success, or \-1 if \var{dlist}
  does not contain an \var{nth} element.
\example
  A delimited list of strings may be turned into an array of strings
  as follows.  For conciseness, all malloc error checking has been
  omitted.
#v+
    int list_to_array (char *list, char delim, char ***ap)
    {
       unsigned int nth;
       char **a;
       char buf[1024];

       /* Determine the size of the array */
       nth = 0;
       while (0 == SLextract_list_element (list, nth, delim, buf, sizeof(buf)))
         nth++;

       ap = (char **) SLmalloc ((nth + 1) * sizeof (char **));
       nth = 0;
       while (0 == SLextract_list_element (list, nth, delim, buf, sizeof(buf)))
         {
	     a[nth] = SLmake_string (buf);
	     nth++;
	 }
       a[nth] = NULL;
       *ap = a;
       return 0;
    }
#v-
\seealso{SLmalloc, SLmake_string}
\done

