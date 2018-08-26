\function{SLsearch_new}
\synopsis{Create an SLsearch_Type object}
\usage{SLsearch_Type *SLsearch_new (SLuchar_Type *key, int search_flags)}
\description
 The \cfun{SLsearch_new} function instantiates  an \ctype{SLsearch_Type}
 object for use in ordinary searches (non-regular expression) by the
 functions in the SLsearch interface.  The first argument \exmp{key}
 is a pointer to a null terminated string that specifies the character
 string to be searched.  This character string may not contain any
 embedded null characters.

 The second argument \exmp{search_flags} is used to specify how the
 search is to be performed.  It is a bit-mapped integer whose value is
 constructed by the bitwise-or of zero or more of the following:
#v+
   SLSEARCH_CASELESS
     The search shall be performed in a case-insensitive manner.

   SLSEARCH_UTF8
     Both the search string and the text to be searched is UTF-8
     encoded.
#v-

 Upon sucess, the function returns the newly created object, and \NULL
 otherwise.  When the search object is nolonger needed, it should be
 freed via the \cfun{SLsearch_delete} function.
\seealso{SLsearch_delete, SLsearch_forward, SLsearch_backward}
\done

\function{SLsearch_delete}
\synopsis{Free the memory associated with a SLsearch_Type object}
\usage{SLsearch_delete (SLsearch_Type *)}
\description
  This function should be called to free the memory associated with a
  \ctype{SLsearch_Type} object created by the \cfun{SLsearch_new}
  function.  Failure to do so will result in a memory leak.
\seealso{SLsearch_new, SLsearch_forward, SLsearch_backward}
\done

\function{SLsearch_forward}
\synopsis{Search forward in a buffer}
\usage{SLuchar_Type SLsearch_forward (st, pmin, pmax)}
#v+
   SLsearch_Type *st;
   SLuchar_Type *pmin, *pmax;
#v-
\description
  The \cfun{SLsearch_forward} function searches forward in the buffer
  defined by the pointers \exmp{pmin} and \exmp{pmax}.  The
  starting point for the search is at the beginning of the buffer at
  \exmp{pmin}.  At no point will the bytes at \exmp{pmax} and beyond
  be examined.  The first parameter \exmp{st}, obtained by a prior call to
  \exmp{SLsearch_new}, specifies the object to found.
  be found from a previous call to \exmp{SLsearch_new}.

  If the object was found, the pointer to the beginning of it will be
  returned.  Otherwise, \cfun{SLsearch_forward} will return \NULL.
  The length of the object may be obtained via the
  \cfun{SLsearch_match_len} function.
\notes
  This function uses the Boyer-Moore search algorithm when possible.
\seealso{SLsearch_new, SLsearch_backward, SLsearch_delete, SLsearch_match_len}
\done

\function{SLsearch_backward}
\synopsis{Search backward in a buffer}
\usage{SLuchar_Type SLsearch_forward (st, pmin, pstart, pmax)}
#v+
   SLsearch_Type *st;
   SLuchar_Type *pmin, *pstart, *pmax;
#v-
\description
  The \cfun{SLsearch_forward} function searches backward in the buffer
  defined by the pointers \exmp{pmin} and \exmp{pmax}.  The starting
  point for the search is at the position \exmp{pstart}. At no point
  will the bytes at \exmp{pmax} and beyond be examined.  The first
  parameter \exmp{st}, obtained by a prior call to
  \exmp{SLsearch_new}, specifies the object to found.

  If the object was found, the pointer to the beginning of it will be
  returned.  Otherwise, \cfun{SLsearch_forward} will return \NULL.
  The length of the object may be obtained via the
  \cfun{SLsearch_match_len} function.
\notes
  This function uses the Boyer-Moore search algorithm when possible.

  It is possible for the end of match to appear after the point where
  the search began (\exmp{pstart}).
\seealso{SLsearch_new, SLsearch_forward, SLsearch_delete, SLsearch_match_len}
\done

\function{SLsearch_match_len}
\synopsis{Get the length of the previous match}
\usage{unsigned int SLsearch_match_len (SLsearch_Type *st)}
\description
  The \cfun{SLsearch_match_len} function returns the length of the
  match from the most recent search involving the specified
  \ctype{SLsearch_Type} object.  If the most recent search was
  unsuccessful, the function will return 0.
\seealso{SLsearch_forward, SLsearch_backward, SLsearch_new, SLsearch_delete}
\done
