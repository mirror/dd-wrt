\function{SLmake_string}
\synopsis{Duplicate a string}
\usage{char *SLmake_string (char *s)}
\description
  The \var{SLmake_string} function creates a new copy of the string
  \var{s}, via \var{malloc}, and returns it.  Upon failure it returns
  \var{NULL}.  Since the resulting string is malloced, it should be
  freed when nolonger needed via a call to either \var{free} or
  \var{SLfree}.
\notes
  \var{SLmake_string} should not be confused with the function
  \var{SLang_create_slstring}, which performs a similar function.
\seealso{SLmake_nstring, SLfree, SLmalloc, SLang_create_slstring}
\done

\function{SLmake_nstring}
\synopsis{Duplicate a substring}
\usage{char *SLmake_nstring (char *s, unsigned int n)}
\description
  This function is like \var{SLmake_string} except that it creates a
  null terminated string formed from the first \var{n} characters of
  \var{s}.  Upon failure, it returns \var{NULL}, otherwise it returns
  the new string.  When nolonger needed, the returned string should be
  freed with \var{SLfree}.
\seealso{SLmake_string, SLfree, SLang_create_nslstring}
\done

\function{SLang_create_nslstring}
\synopsis{Created a hashed substring}
\usage{char *SLang_create_nslstring (char *s, unsigned int n)}
\description
  \var{SLang_create_nslstring} is like \var{SLang_create_slstring}
  except that only the first \var{n} characters of \var{s} are used to
  create the hashed string.  Upon error, it returns \var{NULL}, otherwise it
  returns the hashed substring.  Such a string must be freed by the
  function \var{SLang_free_slstring}.
\notes
  Do not use \var{free} or \var{SLfree} to free the string returned by
  \var{SLang_create_slstring} or \var{SLang_create_nslstring}.  Also
  it is important that no attempt is made to modify the hashed string
  returned by either of these functions.  If one needs to modify a
  string, the functions \var{SLmake_string} or \var{SLmake_nstring}
  should be used instead.
\seealso{SLang_free_slstring, SLang_create_slstring, SLmake_nstring}
\done

\function{SLang_create_slstring}
\synopsis{Create a hashed string}
\usage{char *SLang_create_slstring (char *s)}
\description
  The \var{SLang_create_slstring} creates a copy of \var{s} and
  returns it as a hashed string.  Upon error, the function returns
  \var{NULL}, otherwise it returns the hashed string.  Such a string
  must only be freed via the \var{SLang_free_slstring} function.
\notes
  Do not use \var{free} or \var{SLfree} to free the string returned by
  \var{SLang_create_slstring} or \var{SLang_create_nslstring}.  Also
  it is important that no attempt is made to modify the hashed string
  returned by either of these functions.  If one needs to modify a
  string, the functions \var{SLmake_string} or \var{SLmake_nstring}
  should be used instead.
\seealso{SLang_free_slstring, SLang_create_nslstring, SLmake_string}
\done

\function{SLang_free_slstring}
\synopsis{Free a hashed string}
\usage{void SLang_free_slstring (char *s)}
\description
  The \var{SLang_free_slstring} function is used to free a hashed
  string such as one returned by \var{SLang_create_slstring},
  \var{SLang_create_nslstring}, or \var{SLang_create_static_slstring}.
  If \var{s} is \var{NULL}, the routine does nothing.
\seealso{SLang_create_slstring, SLang_create_nslstring, SLang_create_static_slstring}
\done

\function{SLang_concat_slstrings}
\synopsis{Concatenate two strings to produce a hashed string}
\usage{char *SLang_concat_slstrings (char *a, char *b)}
\description
  The \var{SLang_concat_slstrings} function concatenates two strings,
  \var{a} and \var{b}, and returns the result as a hashed string.
  Upon failure, \var{NULL} is returned.
\notes
  A hashed string can only be freed using \var{SLang_free_slstring}.
  Never use \var{free} or \var{SLfree} to free a hashed string,
  otherwise memory corruption will result.
\seealso{SLang_free_slstring, SLang_create_slstring}
\done

\function{SLang_create_static_slstring}
\synopsis{Create a hashed string}
\usage{char *SLang_create_static_slstring (char *s_literal)}
\description
  The \var{SLang_create_static_slstring} creates a hashed string from
  the string literal \var{s_literal} and returns the result.  Upon
  failure it returns \var{NULL}.
\example
#v+
     char *create_hello (void)
     {
        return SLang_create_static_slstring ("hello");
     }
#v-
\notes
  This function should only be used with string literals.
\seealso{SLang_create_slstring, SLang_create_nslstring}
\done

\function{SLmalloc}
\synopsis{Allocate some memory}
\usage{char *SLmalloc (unsigned int nbytes)}
\description
  This function uses \var{malloc} to allocate \var{nbytes} of memory.
  Upon error it returns \var{NULL}; otherwise it returns a pointer to
  the allocated memory.  One should use \var{SLfree} to free the
  memory after use.
\seealso{SLfree, SLrealloc, SLcalloc}
\done

\function{SLcalloc}
\synopsis{Allocate some memory}
\usage{char *SLcalloc (unsigned int num_elem, unsigned int elem_size)}
\description
  This function uses \var{calloc} to allocate memory for
  \var{num_elem} objects with each of size \var{elem_size} and returns
  the result.  In addition, the newly allocated memory is zeroed.
  Upon error it returns \var{NULL}; otherwise it returns a pointer to
  the allocated memory.  One should use \var{SLfree} to free the
  memory after use.
\seealso{SLmalloc, SLrealloc, SLfree}
\done

\function{SLfree}
\synopsis{Free some allocated memory}
\usage{void SLfree (char *ptr)}
\description
  The \var{SLfree} function deallocates the memory specified by
  \var{ptr}, which may be \var{NULL} in which case the function does
  nothing.
\notes
  Never use this function to free a hashed string returned by one of
  the family of \var{slstring} functions, e.g.,
  \var{SLang_pop_slstring}.
\seealso{SLmalloc, SLcalloc, SLrealloc, SLmake_string}
\done

\function{SLrealloc}
\synopsis{Resize a dynamic memory block}
\usage{char *SLrealloc (char *ptr, unsigned int new_size)}
\description
  The \var{SLrealloc} uses the \var{realloc} function to resize the
  memory block specified by \var{ptr} to the new size \var{new_size}.
  If \var{ptr} is \var{NULL}, the function call is equivalent to
  \exmp{SLmalloc(new_size)}.  Similarly, if \var{new_size} is zero,
  the function call is equivalent to \var{SLfree(ptr)}.

  If the function fails, or if \var{new_size} is zero, \var{NULL} is
  returned.  Otherwise a pointer is returned to the (possibly moved)
  new block of memory.
\seealso{SLfree, SLmalloc, SLcalloc}
\done

