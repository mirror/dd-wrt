\function{add_doc_file}
\synopsis{Make a documentation file known to the help system}
\usage{add_doc_file (String_Type file)}
\description
  The \ifun{add_doc_file} is used to add a documentation file to the
  system.  Such files are searched by the
  \ifun{get_doc_string_from_file} function.  The \exmp{file} must be
  specified using the full path.
\seealso{set_doc_files, get_doc_files, get_doc_string_from_file}
\done

\function{_apropos}
\synopsis{Generate a list of functions and variables}
\usage{Array_Type _apropos (String_Type ns, String_Type s, Integer_Type flags)}
\description
  The \ifun{_apropos} function may be used to get a list of all defined
  objects in the namespace \exmp{ns} whose name matches the regular
  expression \exmp{s} and whose type matches those specified by
  \exmp{flags}.  It returns an array of strings containing the names
  matched.

  The third parameter \exmp{flags} is a bit mapped value whose bits
  are defined according to the following table
#v+
     1          Intrinsic Function
     2          User-defined Function
     4          Intrinsic Variable
     8          User-defined Variable
#v-
\example
#v+
    define apropos (s)
    {
      variable n, name, a;
      a = _apropos ("Global", s, 0xF);

      vmessage ("Found %d matches:", length (a));
      foreach name (a)
        message (name);
    }
#v-
  prints a list of all matches.
\notes
  If the namespace specifier \exmp{ns} is the empty string \exmp{""},
  then the namespace will default to the static namespace of the
  current compilation unit.
\seealso{is_defined, sprintf, _get_namespaces}
\done

\function{_function_name}
\synopsis{Returns the name of the currently executing function}
\usage{String_Type _function_name ()}
\description
  This function returns the name of the currently executing function.
  If called from top-level, it returns the empty string.
\seealso{_trace_function, is_defined}
\done

\function{__get_defined_symbols}
\synopsis{Get the symbols defined by the preprocessor}
\usage{Int_Type __get_defined_symbols ()}
\description
  The \ifun{__get_defined_symbols} functions is used to get the list of
  all the symbols defined by the \slang preprocessor.  It pushes each
  of the symbols on the stack followed by the number of items pushed.
\seealso{is_defined, _apropos, _get_namespaces}
\done

\function{get_doc_files}
\synopsis{Get the list of documentation files}
\usage{String_Type[] = get_doc_files ()}
\description
  The \ifun{get_doc_files} function returns the internal list of
  documentation files as an array of strings.
\seealso{set_doc_files, add_doc_file, get_doc_string_from_file}
\done

\function{get_doc_string_from_file}
\synopsis{Read documentation from a file}
\usage{String_Type get_doc_string_from_file ([String_Type f,] String_Type t)}
\description
  If called with two arguments, \ifun{get_doc_string_from_file} opens
  the documentation file \exmp{f} and searches it for topic \exmp{t}.
  Otherwise, it will search an internal list of documentation files
  looking for the documentation associated with the topic \exmp{t}.  If
  found, the documentation for \exmp{t} will be returned, otherwise the
  function will return \NULL.

  Files may be added to the internal list via the \ifun{add_doc_file}
  or \ifun{set_doc_files} functions.
\seealso{add_doc_file, set_doc_files, get_doc_files, _slang_doc_dir}
\done

\function{_get_namespaces}
\synopsis{Returns a list of namespace names}
\usage{String_Type[] _get_namespaces ()}
\description
  This function returns a string array containing the names of the
  currently defined namespaces.
\seealso{_apropos, use_namespace, implements, __get_defined_symbols}
\done

\function{is_defined}
\synopsis{Determine if a variable or function is defined}
\usage{Integer_Type is_defined (String_Type name)}
\description
   This function is used to determine whether or not a function or
   variable of the given name has been defined.  If the specified name
   has not been defined, the function returns 0.  Otherwise, it
   returns a non-zero value that depends on the type of object
   attached to the name. Specifically, it returns one of the following
   values:
#v+
     +1     intrinsic function
     +2     slang function
     -1     intrinsic variable
     -2     slang variable
      0     undefined
#v-
\example
    Consider the function:
#v+
    define runhooks (hook)
    {
       if (2 == is_defined(hook)) eval(hook);
    }
#v-
    This function could be called from another \slang function to
    allow customization of that function, e.g., if the function
    represents a mode, the hook could be called to setup keybindings
    for the mode.
\seealso{typeof, eval, autoload, __get_reference, __is_initialized}
\done

\function{__is_initialized}
\synopsis{Determine whether or not a variable has a value}
\usage{Integer_Type __is_initialized (Ref_Type r)}
\description
   This function returns non-zero of the object referenced by \exmp{r}
   is initialized, i.e., whether it has a value.  It returns 0 if the
   referenced object has not been initialized.
\example
   The function:
#v+
    define zero ()
    {
       variable f;
       return __is_initialized (&f);
    }
#v-
  will always return zero, but
#v+
    define one ()
    {
       variable f = 0;
       return __is_initialized (&f);
    }
#v-
  will return one.
\seealso{__get_reference, __uninitialize, is_defined, typeof, eval}
\done

\variable{_NARGS}
\synopsis{The number of parameters passed to a function}
\usage{Integer_Type _NARGS}
   The value of the \ivar{_NARGS} variable represents the number of
   arguments passed to the function.  This variable is local to each
   function.
\example
   This example uses the \ivar{_NARGS} variable to print the list of
   values passed to the function:
#v+
     define print_values ()
     {
        variable arg;

        if (_NARGS == 0)
          {
             message ("Nothing to print");
             return;
          }
        foreach arg (__pop_args (_NARGS))
          vmessage ("Argument value is: %S", arg.value);
     }
#v-
\seealso{__pop_args, __push_args, typeof}
\done

\function{set_doc_files}
\synopsis{Set the internal list of documentation files}
\usage{set_doc_files (String_Type[] list)}
\description
  The \ifun{set_doc_files} function may be used to set the internal
  list of documentation files.  It takes a single parameter, which is
  required to be an array of strings.  The internal file list is set
  to the files specified by the elements of the array.
\example
  The following example shows how to add all the files in a specified
  directory to the internal list.  It makes use of the \ifun{glob}
  function that is distributed as part of \slsh.
#v+
     files = glob ("/path/to/doc/files/*.sld");
     set_doc_files ([files, get_doc_files ()]);
#v-
\seealso{get_doc_files, add_doc_file, get_doc_string_from_file}
\done

\variable{_slang_doc_dir}
\synopsis{Installed documentation directory}
\usage{String_Type _slang_doc_dir}
\description
   The \ifun{_slang_doc_dir} variable is a read-only variable that
   specifies the compile-time installation location of the \slang
   documentation.
\seealso{get_doc_string_from_file}
\done

\variable{_slang_version}
\synopsis{The S-Lang library version number}
\usage{Integer_Type _slang_version}
\description
   \ifun{_slang_version} is a read-only variable that gives the version
   number of the \slang library.
\seealso{_slang_version_string}
\done

\variable{_slang_version_string}
\synopsis{The S-Lang library version number as a string}
\usage{String_Type _slang_version_string}
\description
  \ifun{_slang_version_string} is a read-only variable that gives a
  string representation of the version number of the \slang library.
\seealso{_slang_version}
\done

