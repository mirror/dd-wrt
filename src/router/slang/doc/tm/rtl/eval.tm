\function{_$}
\synopsis{Expand the dollar-escaped variables in a string}
\usage{String_Type _$(String_Type s)}
\description
 This function expands the dollar-escaped variables in a string and
 returns the resulting string.
\example
 Consider the following code fragment:
#v+
     private variable Format = "/tmp/foo-$time.$pid";
     define make_filename ()
     {
        variable pid = getpid ();
        variable time = _time ();
        return _$(Format);
     }
#v-
 Note that the variable \exmp{Format} contains dollar-escaped
 variables, but because the \exmp{$} suffix was omitted from the
 string literal, the variables are not expanded.  Instead expansion is
 deferred until execution of the \exmp{make_filename} function through
 the use of the \exmp{_$} function.
\seealso{eval, getenv}
\done

\function{autoload}
\synopsis{Load a function from a file}
\usage{autoload (String_Type funct, String_Type file)}
\description
  The \ifun{autoload} function is used to declare \exmp{funct} to the
  interpreter and indicate that it should be loaded from \exmp{file}
  when it is actually used.  If \exmp{func} contains a namespace
  prefix, then the file will be loaded into the corresponding
  namespace.  Otherwise, if the \ifun{autoload} function is called
  from an execution namespace that is not the Global namespace nor an
  anonymous namespace, then the file will be loaded into the execution
  namespace.
\example
    Suppose \exmp{bessel_j0} is a function defined in the file
    \exmp{bessel.sl}.  Then the statement
#v+
      autoload ("bessel_j0", "bessel.sl");
#v-
    will cause \exmp{bessel.sl} to be loaded prior to the execution of
    \exmp{bessel_j0}.
\seealso{evalfile, import}
\done

\function{byte_compile_file}
\synopsis{Compile a file to byte-code for faster loading.}
\usage{byte_compile_file (String_Type file, Int_Type method)}
\description
  The \ifun{byte_compile_file} function byte-compiles \exmp{file}
  producing a new file with the same name except a \exmp{'c'} is added
  to the output file name.  For example, \exmp{file} is
  \exmp{"site.sl"}, then this function produces a new file named
  \exmp{site.slc}.
\notes
  The \exmp{method} parameter is not used in the current
  implementation, but may be in the future.  For now, set
  it to \exmp{0}.
\seealso{evalfile}
\done

\function{eval}
\synopsis{Interpret a string as \slang code}
\usage{eval (String_Type expression [,String_Type namespace])}
\description
  The \ifun{eval} function parses a string as S-Lang code and executes the
  result.  If called with the optional namespace argument, then the
  string will be evaluated in the specified namespace.  If that
  namespace does not exist it will be created first.

  This is a useful function in many contexts including those where
  it is necessary to dynamically generate function definitions.
\example
#v+
    if (0 == is_defined ("my_function"))
      eval ("define my_function () { message (\"my_function\"); }");
#v-
\seealso{is_defined, autoload, evalfile}
\done

\function{evalfile}
\synopsis{Interpret a file containing \slang code}
\usage{Int_Type evalfile (String_Type file [,String_Type namespace])}
\description
  The \ifun{evalfile} function loads \exmp{file} into the interpreter
  and executes it.  If called with the optional namespace argument,
  the file will be loaded into the specified namespace, which will be
  created if necessary.  If given no namespace argument and the file
  has already been loaded, then it will be loaded again into an
  anonymous namespace.  A namespace argument given by the empty string
  will also cause the file to be loaded into a new anonymous namespace.

  If no errors were encountered, 1 will be returned; otherwise,
  a \slang exception will be thrown and the function will return zero.
\example
#v+
    define load_file (file)
    {
       try
       {
         () = evalfile (file);
       }
       catch AnyError;
    }
#v-
\notes
  For historical reasons, the return value of this function is not
  really useful.

  The file is searched along an application-defined load-path.  The
  \ifun{get_slang_load_path} and \ifun{set_slang_load_path} functions
  may be used to set and query the path.
\seealso{eval, autoload, set_slang_load_path, get_slang_load_path}
\done

\function{get_slang_load_path}
\synopsis{Get the value of the interpreter's load-path}
\usage{String_Type get_slang_load_path ()}
\description
  This function retrieves the value of the delimiter-separated search
  path used for loading files.  The delimiter is OS-specific and may
  be queried using the \ifun{path_get_delimiter} function.
\notes
  Some applications may not support the built-in load-path searching
  facility provided by the underlying library.
\seealso{set_slang_load_path, path_get_delimiter}
\done

\function{set_slang_load_path}
\synopsis{Set the value of the interpreter's load-path}
\usage{set_slang_load_path (String_Type path)}
\description
  This function may be used to set the value of the
  delimiter-separated search path used by the \ifun{evalfile} and
  \ifun{autoload} functions for locating files.  The delimiter is
  OS-specific and may be queried using the \ifun{path_get_delimiter}
  function.
\example
#v+
    public define prepend_to_slang_load_path (p)
    {
       variable s = stat_file (p);
       if (s == NULL) return;
       if (0 == stat_is ("dir", s.st_mode))
         return;

       p = sprintf ("%s%c%s", p, path_get_delimiter (), get_slang_load_path ());
       set_slang_load_path (p);
    }
#v-
\notes
  Some applications may not support the built-in load-path searching
  facility provided by the underlying library.
\seealso{get_slang_load_path, path_get_delimiter, evalfile, autoload}
\done

