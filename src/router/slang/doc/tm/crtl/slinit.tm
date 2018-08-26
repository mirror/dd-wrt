\function{SLang_init_slang}
\synopsis{Initialize the interpreter}
\usage{int SLang_init_slang (void)}
\description
  The \var{SLang_init_slang} function must be called by all
  applications that use the \slang interpreter.  It initializes the
  interpreter, defines the built-in data types, and adds a set of core
  intrinsic functions.

  The function returns \var{0} upon success, or \var{-1} upon failure.
\seealso{SLang_init_slfile, SLang_init_slmath, SLang_init_slunix}
\done

\function{SLang_init_slfile}
\synopsis{Initialize the interpreter file I/O intrinsics}
\usage{int SLang_init_slfile (void)}
\description
  This function initializes the interpreters file I/O intrinsic
  functions.  This function adds intrinsic functions such as
  \var{fopen}, \var{fclose}, and \var{fputs} to the interpreter.
  It returns \exmp{0} if successful, or \exmp{-1} upon error.
\notes
  Before this function can be called, it is first necessary to call
  \var{SLang_init_slang}.  It also adds
  the preprocessor symbol \var{__SLFILE__} to the interpreter.
\seealso{SLang_init_slang, SLang_init_slunix, SLang_init_slmath}
\done

\function{SLang_init_slmath}
\synopsis{Initialize the interpreter math intrinsics}
\usage{int SLang_init_slmath (void)}
\description
  The \var{SLang_init_slmath} function initializes the interpreter's
  mathematical intrinsic functions and makes them available to the
  language.  The intrinsic functions include \var{sin}, \var{cos},
  \var{tan}, etc...  It returns \exmp{0} if successful, or \exmp{-1}
  upon failure.
\notes
  This function must be called after \var{SLang_init_slang}.  It adds
  the preprocessor symbol \var{__SLMATH__} to the interpreter.
\seealso{SLang_init_slang, SLang_init_slfile, SLang_init_slunix}
\done

\function{SLang_init_slunix}
\synopsis{Make available some unix system calls to the interpreter}
\usage{int SLang_init_slunix (void)}
\description
  The \var{SLang_init_slunix} function initializes the interpreter's
  unix system call intrinsic functions and makes them available to the
  language.  Examples of functions made available by
  \var{SLang_init_slunix} include \var{chmod}, \var{chown}, and
  \var{stat_file}.  It returns \exmp{0} if successful, or \exmp{-1}
  upon failure.
\notes
  This function must be called after \var{SLang_init_slang}.  It adds
  the preprocessor symbol \var{__SLUNIX__} to the interpreter.
\seealso{SLang_init_slang, SLang_init_slfile, SLang_init_slmath}
\done

