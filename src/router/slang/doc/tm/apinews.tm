\chapter{S-Lang 2 API NEWS and UPGRADE information}

 The \slang API underwent a number of changes for version 2.  In
 particular, the following interfaces have been affected:
#v+
    SLsmg
    SLregexp
    SLsearch
    SLrline
    SLprep
    slang interpreter modules
#v-
 Detailed information about these changes is given below.  Other
 changes include:
\begin{itemize}
\item UTF-8 encoded strings are now supported at both the C library level
  and the interpreter.
\item Error handling by the interpreter has been rewritten to support
 try/catch style exception.  Applications may also define
 application-specific error codes.
\item The library may be compiled with large-file-support.
\end{itemize}
See the relevant chapters in this manual for more information.

\sect{SLang_Error}

 The \var{SLang_Error} variable is nolonger part of the API.  Change code
 such as
#v+
      SLang_Error = foo;
      if (SLang_Error == bar) ...
#v-
   to
#v+
      SLang_set_error (foo);
      if (bar == SLang_get_error ()) ...
#v-

\sect{SLsmg/SLtt Functions}

    The changes to these functions were dictated by the new UTF-8
    support.  For the most part, the changes should be transparent but
    some functions and variables have been changed.

\begin{itemize}
\item SLtt_Use_Blink_For_ACS is nolonger supported, nor necessary  I think only
  DOSEMU uses this.
\item
  Prototypes for \cfun{SLsmg_draw_object} and \cfun{SLsmg_write_char}
  were changed to use wide characters (SLwchar_Type).
\item
  \var{SLsmg_Char_Type} was changed from an \exmp{unsigned short} to a
  structure.  This change was necessary in order to support combining
  characters and double width unicode characters.  This change may affect
  the use of the following functions:
#v+
     SLsmg_char_at
     SLsmg_read_raw
     SLsmg_write_raw
     SLsmg_write_color_chars
#v-
\item The \exmp{SLSMG_BUILD_CHAR} macro has been removed.
  The \exmp{SLSMG_EXTRACT_CHAR} macro will continue to work as long as
  combining characters are not present.
\item The prototype for \cfun{SLsmg_char_at} was changed to
#v+
     int SLsmg_char_at (SLsmg_Char_Type *);
#v-
\end{itemize}

\sect{SLsearch Functions}

   \var{SLsearch_Type} is now an opaque type.  Code such as
#v+
      SLsearch_Type st;
      SLsearch_init (string, 1, 0, &st);
         .
         .
      s = SLsearch (buf, bufmax, &st);
#v-
   which searches forward in \exmp{buf} for \exmp{string} must be changed to
#v+
      SLsearch_Type *st = SLsearch_open (string, SLSEARCH_CASELESS);
      if (st == NULL)
        return;
         .
         .
      s = SLsearch_forward (st, buf, bufmax);
         .
         .
      SLsearch_close (st);
#v-

\sect{Regular Expression Functions}

   The slang v1 regular expression API has been redesigned in order to
   facilitate the incorporation of third party regular expression
   engines.

   New functions include:
#v+
     SLregexp_compile
     SLregexp_free
     SLregexp_match
     SLregexp_nth_match
     SLregexp_get_hints
#v-

   The plan is to migrate to the use of the PCRE regular expressions
   for version 2.2.  As such, you may find it convenient to adopt the
   PCRE library now instead of updating to the changed \slang API.

\sect{Readline Functions}

   The readline interface has changed in order to make it easier to
   use.  Using it now is as simple as:
#v+
      SLrline_Type *rli;
      rli = SLrline_open (SLtt_Screen_Cols, flags);
      buf = SLrline_read_line (rli, prompt, &len);
      /* Use buf */
         .
         .
      SLfree (buf);
      SLrline_close (rli);
#v-
  See how it is used in \file{slsh/readline.c}.

\sect{Preprocessor Interface}

   SLPreprocess_Type was renamed to SLprep_Type and made opaque.
   New functions include:
#v+
      SLprep_new
      SLprep_delete
      SLprep_set_flags
      SLprep_set_comment
      SLprep_set_prefix
      SLprep_set_exists_hook
      SLprep_set_eval_hook
#v-
   If you currently use:
#v+
      SLPreprocess_Type pt;
      SLprep_open_prep (&pt);
         .
         .
      SLprep_close_prep (&pt);
#v-
   Then change it to:
#v+
      SLprep_Type *pt;
      pt = SLprep_new ();
         .
         .
      SLprep_delete (pt);
#v-

\sect{Functions dealing with the interpreter C API}

\begin{itemize}
\item \cfun{SLang_pop_double} has been changed to be more like the other
  \cfun{SLang_pop_*} functions.  Now, it may be used as:
#v+
       double x;
       if (-1 == SLang_pop_double (&x))
         .
         .
#v-
\item All the functions that previously took an "unsigned char" to
    specify a slang data type have changed to require an \var{SLtype}.
    Previously, \var{SLtype} was typedefed to be an \var{unsigned
    char}, but now it is an \var{int}.

\item The \var{SLang_Class_Type} object is now an opaque type.  If you were
    previously accessing its fields directly, then you will have to
    change the code to use one of the accessor functions.
\end{itemize}

\sect{Modules}

\begin{itemize}
\item
   In order to support the loading of a module into multiple
   namespaces, a module's init function may be called more than once.
   See modules/README for more information.
\item
   The \var{init_<module>_module} function is no longer supported
    because it did not support namespaces.  Use the
    \var{init_<module>_module_ns} function instead.
\end{itemize}
