\variable{_auto_declare}
\synopsis{Set automatic variable declaration mode}
\usage{Integer_Type _auto_declare}
\description
  The \ivar{_auto_declare} variable may be used to have undefined
  variable implicitly declared.  If set to zero, any
  variable must be declared with a \kw{variable} declaration before it
  can be used.  If set to one, then any undeclared variable will be
  declared as a \kw{static} variable.

  The \ivar{_auto_declare} variable is local to each compilation unit and
  setting its value in one unit has no effect upon its value in other
  units.   The value of this variable has no effect upon the variables
  in a function.
\example
  The following code will not compile if \exmp{X} not been
  declared:
#v+
    X = 1;
#v-
  However,
#v+
    _auto_declare = 1;   % declare variables as static.
    X = 1;
#v-
  is equivalent to
#v+
    static variable X = 1;
#v-
\notes
  This variable should be used sparingly and is intended primarily for
  interactive applications where one types \slang commands at a
  prompt.
\done

\function{__class_id}
\synopsis{Return the class-id of a specified type}
\usage{Int_Type __class_id (DataType_Type type)}
\description
  This function returns the internal class-id of a specified data type.
\seealso{typeof, _typeof, __class_type, __datatype}
\done

\function{__class_type}
\synopsis{Return the class-type of a specified type}
\usage{Int_Type __class_type (DataType_Type type))}
\description
  Internally \slang objects are classified according to four types:
  scalar, vector, pointer, and memory managed types.  For example, an
  integer is implemented as a scalar, a complex number as a vector,
  and a string is represented as a pointer.  The \ifun{__class_type}
  function returns an integer representing the class-type associated
  with the specified data type. Specifically, it returns:
#v+
       0    memory-managed
       1    scalar
       2    vector
       3    pointer
#v-
\seealso{typeof, _typeof, __class_id, __datatype}
\done

\function{current_namespace}
\synopsis{Get the name of the current namespace}
\usage{String_Type current_namespace ()}
\description
   The \ifun{current_namespace} function returns the name of the
   static namespace associated with the compilation unit.  If there is
   no such namespace associated with the compilation unit, then the
   empty string \exmp{""} will be returned.
\seealso{implements, use_namespace, import, evalfile}
\done

\function{__datatype}
\synopsis{Get the DataType_Type for a specified internal class-id}
\usage{DataType_Type __datatype (Int_Type id)}
\description
 This function is the inverse of __class_type in the sense that it
 returns the \dtype{DataType_Type} for the specified class-id.  If no
 such class exists, the function will return \NULL.
\notes
 One should not expect distinct interpreter instances to always return
 the same value for a dynamically assigned class-id such as one
 defined by a module or one stemming from a \kw{typedef} statement.
\seealso{__class_id, __class_type, typeof}
\done

\function{_eqs}
\synopsis{Test for equality of two objects}
\usage{Int_Type _eqs (a, b)}
\description
  This function tests its two arguments for equality and returns 1 if
  they are equal or 0 otherwise. What it means to be equal depends
  upon the data types of the objects being compared.  If the types are
  numeric, they are regarded as equal if their numerical values are
  equal.  If they are arrays, then they are equal if they have the
  same shape with equal elements. If they are structures, then they
  are equal if they contain identical fields, and the corresponding
  values are equal.
\example
   _eqs (1, 1)             ===> 1
   _eqs (1, 1.0)           ===> 1
   _eqs ("a", 1)           ===> 0
   _eqs ([1,2], [1.0,2.0]) ===> 1
\seealso{typeof, _eqs, __get_reference, __is_callable}
\notes
   For testing sameness, use \ifun{__is_same}.
\done

\function{getenv}
\synopsis{Get the value of an environment variable}
\usage{String_Type getenv(String_Type var)}
\description
   The \ifun{getenv} function returns a string that represents the
   value of an environment variable \exmp{var}.  It will return
   \NULL if there is no environment variable whose name is given
   by \exmp{var}.
\example
#v+
    if (NULL != getenv ("USE_COLOR"))
      {
        set_color ("normal", "white", "blue");
        set_color ("status", "black", "gray");
        USE_ANSI_COLORS = 1;
      }
#v-
\seealso{putenv, strlen, is_defined}
\done

\function{__get_reference}
\synopsis{Get a reference to a global object}
\usage{Ref_Type __get_reference (String_Type nm)}
\description
  This function returns a reference to a global variable or function
  whose name is specified by \exmp{nm}.  If no such object exists, it
  returns \NULL, otherwise it returns a reference.
\example
   Consider the function:
#v+
    define runhooks (hook)
    {
       variable f;
       f = __get_reference (hook);
       if (f != NULL)
         @f ();
    }
#v-
   This function could be called from another \slang function to allow
   customization of that function, e.g., if the function represents a
   \jed editor mode, the hook could be called to setup keybindings for
   the mode.
\seealso{is_defined, typeof, eval, autoload, __is_initialized, __uninitialize}
\done

\function{implements}
\synopsis{Create a new static namespace}
\usage{implements (String_Type name)}
\description
  The \ifun{implements} function may be used to create a new static
  namespace and have it associated with the current compilation unit.
  If a namespace with the specified name already exists, a
  \exc{NamespaceError} exception will be thrown.

  In addition to creating a new static namespace and associating it
  with the compilation unit, the function will also create a new
  private namespace.  As a result, any symbols in the previous private
  namespace will be no longer be accessible.  For this reason, it is
  recommended that this function should be used before any private
  symbols have been created.
\example
  Suppose that some file \exmp{t.sl} contains:
#v+
     implements ("My");
     define message (x)
     {
        Global->message ("My's message: $x"$);
     }
     message ("hello");
#v-
  will produce \exmp{"My's message: hello"}.  This \exmp{message}
  function may be accessed from outside the namespace via:
#v+
    My->message ("hi");
#v-
\notes
  Since \ifun{message} is an intrinsic function, it is public and may
  not be redefined in the public namespace.

  The \ifun{implements} function should rarely be used.  It is
  preferable to allow a static namespace to be associated with a
  compilation unit using, e.g., \exmp{evalfile}.
\seealso{use_namespace, current_namespace, import}
\done

\function{__is_callable}
\synopsis{Determine whether or not an object is callable}
\usage{Int_Type __is_callable (obj)}
\description
  This function may be used to determine if an object is callable by
  dereferencing the object.  It returns 1 if the argument is callable,
  or zero otherwise.
\example
   __is_callable (7)      ==> 0
   __is_callable (&sin)   ==> 1
   a = [&sin];
   __is_callable (a[0])   ==> 1
   __is_callable (&a[0])  ==> 0
\seealso{__is_numeric, is_defined}
\done

\function{__is_numeric}
\synopsis{Determine whether or not an object is a numeric type}
\usage{Int_Type __is_numeric (obj)}
\description
  This function may be used to determine if an object represents a
  numeric type.  It returns 0 if the argument is non-numeric, 1 if it
  is an integer, 2 if a floating point number, and 3 if it is complex.
  If the argument is an array, then the array type will be used for
  the test.
\example
   __is_numeric ("foo");  ==> 0
   __is_numeric ("0");    ==> 0
   __is_numeric (0);      ==> 1
   __is_numeric (PI);     ==> 2
   __is_numeric (2j);     ==> 3
   __is_numeric ([1,2]);  ==> 1
   __is_numeric ({1,2});  ==> 0
\seealso{typeof, __is_datatype_numeric}
\done

\function{__is_datatype_numeric}
\synopsis{Determine whether or not a type is a numeric type}
\usage{Int_Type __is_datatype_numeric (DataType_Type type)}
\description
  This function may be used to determine if the specified datatype
  represents a numeric type.  It returns \0 if the datatype does not
  represents a numeric type; otherwise it returns 1 for an
  integer type, 2 for a floating point type, and 3 for a complex type.
\seealso{typeof, __is_numeric, __is_callable}
\done

\function{__is_same}
\synopsis{Test for sameness of two objects}
\usage{Int_Type __is_same (a, b)}
\description
  This function tests its two arguments for sameness and returns 1
  if they are the same, or 0 otherwise.  To be the same, the data type of
  the arguments must match and the values of the objects must
  reference the same underlying object.
\example
   __is_same (1, 1)         ===> 1
   __is_same (1, 1.0)       ===> 0
   __is_same ("a", 1)       ===> 0
   __is_same ([1,2], [1,2]) ===> 0
\seealso{typeof, _eqs, __get_reference, __is_callable}
\notes
   For testing equality, use \ifun{_eqs}.
\done

\function{putenv}
\synopsis{Add or change an environment variable}
\usage{putenv (String_Type s)}
\description
    This functions adds string \exmp{s} to the environment.  Typically,
    \exmp{s} should of the form \exmp{"name=value"}.  The function
    throws an \exc{OSError} upon failure.
\notes
    This function may not be available on all systems.
\seealso{getenv, sprintf}
\done

\variable{_slang_install_prefix}
\synopsis{S-Lang's installation prefix}
\usage{String_Type _slang_install_prefix}
\description
  The value of this variable is set at the S-Lang library's
  compilation time.  On Unix systems, the value corresponds to the
  value of the \exmp{prefix} variable in the Makefile.  For normal
  installations, the library itself will be located in the \exmp{lib}
  subdirectory of the \exmp{prefix} directory.
\notes
  The value of this variable may or may not have anything to do with
  where the slang library is located.  As such, it should be regarded
  as a hint.  A standard installation will have the \exmp{slsh}
  library files located in the \exmp{share/slsh} subdirectory of the
  installation prefix.
\seealso{_slang_doc_dir}
\done

\variable{_slang_utf8_ok}
\synopsis{Test if the interpreter running in UTF-8 mode}
\usage{Int_Type _slang_utf8_ok}
\description
  If the value of this variable is non-zero, then the interpreter is
  running in UTF-8 mode.  In this mode, characters in strings are
  interpreted as variable length byte sequences according to the
  semantics of the UTF-8 encoding.
\notes
  When running in UTF-8 mode, one must be careful not to confuse a
  character with a byte.  For example, in this mode the \ifun{strlen}
  function returns the number of characters in a string which may be
  different than the number of bytes.  The latter information may be
  obtained by the \ifun{strbytelen} function.
\seealso{strbytelen, strlen, strcharlen}
\done

\function{__uninitialize}
\synopsis{Uninitialize a variable}
\usage{__uninitialize (Ref_Type x)}
\description
  The \ifun{__uninitialize} function may be used to uninitialize the
  variable referenced by the parameter \exmp{x}.
\example
  The following two lines are equivalent:
#v+
     () = __tmp(z);
     __uninitialize (&z);
#v-
\seealso{__tmp, __is_initialized}
\done

\function{__tmp}
\synopsis{Returns the value of a variable and uninitialize the variable}
\usage{__tmp (x)}
\description
  The \ifun{__tmp} function takes a single argument, a variable,
  returns the value of the variable, and then undefines the variable.
  The purpose of this pseudo-function is to free any memory
  associated with a variable if that variable is going to be
  re-assigned.
\example
#v+
     x = 3;
     y = __tmp(x);
#v-
  will result in `y' having a value of `3' and `x' will be undefined.
\notes
  This function is a pseudo-function because a syntax error results if
  used like
#v+
      __tmp(sin(x));
#v-
\seealso{__uninitialize, __is_initialized}
\done

\function{use_namespace}
\synopsis{Change to another namespace}
\usage{use_namespace (String_Type name)}
\description
   The \ifun{use_namespace} function changes the current static namespace to
   the one specified by the parameter.  If the specified namespace
   does not exist, a \exc{NamespaceError} exception will be generated.
\seealso{implements, current_namespace, import}
\done

