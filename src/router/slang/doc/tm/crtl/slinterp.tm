\function{SLallocate_load_type}
\synopsis{Allocate a SLang_Load_Type object}
\usage{SLang_Load_Type *SLallocate_load_type (char *name)}
\description
  The \var{SLallocate_load_type} function allocates and initializes
  space for a \var{SLang_Load_Type} object and returns it.  Upon
  failure, the function returns \var{NULL}.  The parameter \var{name}
  must uniquely identify the object.  For example, if the object
  represents a file, then \var{name} could be the absolute path name
  of the file.
\seealso{SLdeallocate_load_type, SLang_load_object}
\done

\function{SLdeallocate_load_type}
\synopsis{Free a SLang_Load_Type object}
\usage{void SLdeallocate_load_type (SLang_Load_Type *slt)}
\description
  This function frees the memory associated with a
  \var{SLang_Load_Type} object that was acquired from a call to the
  \var{SLallocate_load_type} function.
\seealso{SLallocate_load_type, SLang_load_object}
\done

\function{SLang_load_object}
\synopsis{Load an object into the interpreter}
\usage{int SLang_load_object (SLang_Load_Type *obj)}
\description
  The function \var{SLang_load_object} is a generic function that may
  be used to loaded an object of type \var{SLang_Load_Type} into the
  interpreter.  For example, the functions \var{SLang_load_file} and
  \var{SLang_load_string} are wrappers around this function to load a
  file and a string, respectively.
\seealso{SLang_load_file, SLang_load_string, SLallocate_load_type}
\done

\function{SLclass_allocate_class}
\synopsis{Allocate a class for a new data type}
\usage{SLang_Class_Type *SLclass_allocate_class (char *name)}
\description
  The purpose of this function is to allocate and initialize space
  that defines a new data type or class called \var{name}.  If
  successful, a pointer to the class is returned, or upon failure the
  function returns \var{NULL}.

  This function does not automatically create the new data type.
  Callback functions must first be associated with the data type via
  functions such as \var{SLclass_set_push_function}, and the data
  type must be registered with the interpreter via
  \var{SLclass_register_class}.  See the \slang library programmer's
  guide for more information.
\seealso{SLclass_register_class, SLclass_set_push_function}
\done

\function{SLclass_register_class}
\synopsis{Register a new data type with the interpreter}
\usage{int SLclass_register_class (cl, type, sizeof_type, class_type)}
#v+
    SLang_Class_Type *cl
    SLtype type
    unsigned int sizeof_type
    SLclass_Type class_type
#v-
\description
  The \var{SLclass_register_class} function is used to register a new
  class or data type with the interpreter.  If successful, the
  function returns \exmp{0}, or upon failure, it returns \var{-1}.

  The first parameter, \var{cl}, must have been previously obtained
  via the \var{SLclass_allocate_class} function.

  The second parameter, \var{type} specifies the data type of the new
  class.  If set to \var{SLANG_VOID_TYPE} then the library will
  automatically allocate an unused value for the class (the allocated
  value can then be found using the \var{SLclass_get_class_id}
  function), otherwise a value greater than \exmp{255} should be
  used.  The values in the range \exmp{0-255} are reserved for
  internal use by the library.

  The size that the data type represents in bytes is specified by the
  third parameter, \var{sizeof_type}.   This value should not be
  confused with the sizeof the structure that represents the data
  type, unless the data type is of class \var{SLANG_CLASS_TYPE_VECTOR}
  or \var{SLANG_CLASS_TYPE_SCALAR}.  For pointer objects, the value
  of this parameter is just \var{sizeof(void *)}.

  The final parameter specifies the class type of the data type.  It must
  be one of the values:
#v+
     SLANG_CLASS_TYPE_SCALAR
     SLANG_CLASS_TYPE_VECTOR
     SLANG_CLASS_TYPE_PTR
     SLANG_CLASS_TYPE_MMT
#v-
  The \var{SLANG_CLASS_TYPE_SCALAR} indicates that the new data type
  is a scalar.  Examples of scalars in \var{SLANG_INT_TYPE} and
  \var{SLANG_DOUBLE_TYPE}.

  Setting \var{class_type} to SLANG_CLASS_TYPE_VECTOR implies that the
  new data type is a vector, or a 1-d array of scalar types.  An
  example of a data type of this class is the
  \var{SLANG_COMPLEX_TYPE}, which represents complex numbers.

  \var{SLANG_CLASS_TYPE_PTR} specifies the data type is of a pointer
  type.  Examples of data types of this class include
  \var{SLANG_STRING_TYPE} and \var{SLANG_ARRAY_TYPE}.  Such types must
  provide for their own memory management.

  Data types of class \var{SLANG_CLASS_TYPE_MMT} are pointer types
  except that the memory management, i.e., creation and destruction of
  the type, is handled by the interpreter.  Such a type is called a
  \em{memory managed type}.  An example of this data type is the
  \var{SLANG_FILEPTR_TYPE}.
\notes
   See the \slang-library-programmers-guide for more information.
\seealso{SLclass_allocate_class, SLclass_get_class_id}
\done

\function{SLclass_set_string_function}
\synopsis{Set a data type's string representation callback}
\usage{int SLclass_set_string_function (cl, sfun)}
#v+
   SLang_Class_Type *cl
   char *(*sfun) (SLtype, VOID_STAR);
#v-
\description
  The \var{SLclass_set_string_function} routine is used to define a
  callback function, \var{sfun}, that will be used when a string
  representation of an object of the data type represented by \var{cl}
  is needed.  \var{cl} must have already been obtained via a call to
  \var{SLclass_allocate_class}.  When called, \var{sfun} will be
  passed two arguments: an SLtype which represents the data
  type, and the address of the object for which a string represetation
  is required.  The callback function must return a \em{malloced}
  string.

  Upon success, \var{SLclass_set_string_function} returns zero, or
  upon error it returns \-1.
\example
  A callback function that handles both \var{SLANG_STRING_TYPE} and
  \var{SLANG_INT_TYPE} variables looks like:
#v+
     char *string_and_int_callback (SLtype type, VOID_STAR addr)
     {
        char buf[64];

        switch (type)
          {
           case SLANG_STRING_TYPE:
             return SLmake_string (*(char **)addr);

           case SLANG_INTEGER_TYPE:
             sprintf (buf, "%d", *(int *)addr);
             return SLmake_string (buf);
          }
        return NULL;
     }
#v-
\notes
  The default string callback simply returns the name of the data type.
\seealso{SLclass_allocate_class, SLclass_register_class}
\done

\function{SLclass_set_destroy_function}
\synopsis{Set the destroy method callback for a data type}
\usage{int SLclass_set_destroy_function (cl, destroy_fun)}
#v+
    SLang_Class_Type *cl
    void (*destroy_fun) (SLtype, VOID_STAR);
#v-
\description
  \var{SLclass_set_destroy_function} is used to set the destroy
  callback for a data type.  The data type's class \var{cl} must have
  been previously obtained via a call to \var{SLclass_allocate_class}.
  When called, \var{destroy_fun} will be passed two arguments: an
  SLtype which represents the data type, and the address of the
  object to be destroyed.

  \var{SLclass_set_destroy_function} returns zero upon success, and
  \-1 upon failure.
\example
  The destroy method for \var{SLANG_STRING_TYPE} looks like:
#v+
    static void string_destroy (SLtype type, VOID_STAR ptr)
    {
       char *s = *(char **) ptr;
       if (s != NULL) SLang_free_slstring (*(char **) s);
    }
#v-
\notes
  Data types of class SLANG_CLASS_TYPE_SCALAR do not require a destroy
  callback.  However, other classes do.
\seealso{SLclass_allocate_class, SLclass_register_class}
\done

\function{SLclass_set_push_function}
\synopsis{Set the push callback for a new data type}
\usage{int SLclass_set_push_function (cl, push_fun)}
#v+
    SLang_Class_Type *cl
    int (*push_fun) (SLtype, VOID_STAR);
#v-
\description
   \var{SLclass_set_push_function} is used to set the push callback
   for a new data type specified by \var{cl}, which must have been
   previously obtained via \var{SLclass_allocate_class}.

   The parameter \var{push_fun} is a pointer to the push callback.  It
   is required to take two arguments: an SLtype
   representing the data type, and the address of the object to be
   pushed.  It must return zero upon success, or \-1 upon failure.

   \var{SLclass_set_push_function} returns zero upon success, or \-1
   upon failure.
\example
   The push callback for \var{SLANG_COMPLEX_TYPE} looks like:
#v+
      static int complex_push (SLtype type, VOID_STAR ptr)
      {
         double *z = *(double **) ptr;
         return SLang_push_complex (z[0], z[1]);
      }
#v-
\seealso{SLclass_allocate_class, SLclass_register_class}
\done

\function{SLclass_set_pop_function}
\synopsis{Set the pop callback for a new data type}
\usage{int SLclass_set_pop_function (cl, pop_fun)}
#v+
    SLang_Class_Type *cl
    int (*pop_fun) (SLtype, VOID_STAR);
#v-
\description
   \var{SLclass_set_pop_function} is used to set the callback for
   popping an object from the stack for a new data type specified by
   \var{cl}, which must have been previously obtained via
   \var{SLclass_allocate_class}.

   The parameter \var{pop_fun} is a pointer to the pop callback
   function, which is required to take two arguments: an unsigned
   character representing the data type, and the address of the object
   to be popped.  It must return zero upon success, or \-1 upon
   failure.

   \var{SLclass_set_pop_function} returns zero upon success, or \-1
   upon failure.
\example
   The pop callback for \var{SLANG_COMPLEX_TYPE} looks like:
#v+
      static int complex_push (SLtype type, VOID_STAR ptr)
      {
         double *z = *(double **) ptr;
         return SLang_pop_complex (&z[0], &z[1]);
      }
#v-
\seealso{SLclass_allocate_class, SLclass_register_class}
\done

\function{SLclass_get_datatype_name}
\synopsis{Get the name of a data type}
\usage{char *SLclass_get_datatype_name (SLtype type)}
\description
  The \var{SLclass_get_datatype_name} function returns the name of the
  data type specified by \var{type}.  For example, if \var{type} is
  \var{SLANG_INT_TYPE}, the string \exmp{"Integer_Type"} will be
  returned.

  This function returns a pointer that should not be modified or freed.
\seealso{SLclass_allocate_class, SLclass_register_class}
\done

\function{SLang_free_mmt}
\synopsis{Free a memory managed type}
\usage{void SLang_free_mmt (SLang_MMT_Type *mmt)}
\description
  The \var{SLang_MMT_Type} function is used to free a memory managed
  data type.
\seealso{SLang_object_from_mmt, SLang_create_mmt}
\done

\function{SLang_object_from_mmt}
\synopsis{Get a pointer to the value of a memory managed type}
\usage{VOID_STAR SLang_object_from_mmt (SLang_MMT_Type *mmt)}
\description
  The \var{SLang_object_from_mmt} function returns a pointer to the
  actual object whose memory is being managed by the interpreter.
\seealso{SLang_free_mmt, SLang_create_mmt}
\done

\function{SLang_create_mmt}
\synopsis{Create a memory managed data type}
\usage{SLang_MMT_Type *SLang_create_mmt (SLtype t, VOID_STAR ptr)}
\description
  The \var{SLang_create_mmt} function returns a pointer to a new
  memory managed object.  This object contains information necessary
  to manage the memory associated with the pointer \var{ptr} which
  represents the application defined data type of type \var{t}.
\seealso{SLang_object_from_mmt, SLang_push_mmt, SLang_free_mmt}
\done

\function{SLang_push_mmt}
\synopsis{Push a memory managed type}
\usage{int SLang_push_mmt (SLang_MMT_Type *mmt)}
\description
   This function is used to push a memory managed type onto the
   interpreter stack.  It returns zero upon success, or \exmp{-1} upon
   failure.
\seealso{SLang_create_mmt, SLang_pop_mmt}
\done

\function{SLang_pop_mmt}
\synopsis{Pop a memory managed data type}
\usage{SLang_MMT_Type *SLang_pop_mmt (SLtype t)}
\description
  The \var{SLang_pop_mmt} function may be used to pop a memory managed
  type of type \var{t} from the stack.  It returns a pointer to the
  memory managed object upon success, or \var{NULL} upon failure.  The
  function \var{SLang_object_from_mmt} should be used to access the
  actual pointer to the data type.
\seealso{SLang_object_from_mmt, SLang_push_mmt}
\done

\function{SLang_inc_mmt}
\synopsis{Increment a memory managed type reference count}
\usage{void SLang_inc_mmt (SLang_MMT_Type *mmt);}
\description
  The \var{SLang_inc_mmt} function may be used to increment the
  reference count associated with the memory managed data type given
  by \var{mmt}.
\seealso{SLang_free_mmt, SLang_create_mmt, SLang_pop_mmt, SLang_pop_mmt}
\done

\function{SLadd_intrin_fun_table}
\synopsis{Add a table of intrinsic functions to the interpreter}
\usage{int SLadd_intrin_fun_table(SLang_Intrin_Fun_Type *tbl, char *pp_name);}
\description
  The \var{SLadd_intrin_fun_table} function adds an array, or table, of
  \var{SLang_Intrin_Fun_Type} objects to the interpreter.  The first
  parameter, \var{tbl} specifies the table to be added.  The second
  parameter \var{pp_name}, if non-NULL will be added to the list of
  preprocessor symbols.

  This function returns \-1 upon failure or zero upon success.
\notes
  A table should only be loaded one time and it is considered to be an
  error on the part of the application if it loads a table more than
  once.
\seealso{SLadd_intrin_var_table, SLadd_intrinsic_function, SLdefine_for_ifdef}
\done

\function{SLadd_intrin_var_table}
\synopsis{Add a table of intrinsic variables to the interpreter}
\usage{int SLadd_intrin_var_table (SLang_Intrin_Var_Type *tbl, char *pp_name);}
\description
  The \var{SLadd_intrin_var_table} function adds an array, or table, of
  \var{SLang_Intrin_Var_Type} objects to the interpreter.  The first
  parameter, \var{tbl} specifies the table to be added.  The second
  parameter \var{pp_name}, if non-NULL will be added to the list of
  preprocessor symbols.

  This function returns \-1 upon failure or zero upon success.
\notes
  A table should only be loaded one time and it is considered to be an
  error on the part of the application if it loads a table more than
  once.
\seealso{SLadd_intrin_var_table, SLadd_intrinsic_function, SLdefine_for_ifdef}
\done

\function{SLang_load_file}
\synopsis{Load a file into the interpreter}
\usage{int SLang_load_file (char *fn)}
\description
  The \var{SLang_load_file} function opens the file whose name is
  specified by \var{fn} and feeds it to the interpreter, line by line,
  for execution.  If \var{fn} is \var{NULL}, the function will take
  input from \var{stdin}.

  If no error occurs, it returns \exmp{0}; otherwise,
  it returns \exmp{-1}, and sets \var{SLang_Error} accordingly.  For
  example, if it fails to open the file, it will return \exmp{-1} with
  \var{SLang_Error} set to \var{SL_OBJ_NOPEN}.
\notes
   If the hook \var{SLang_Load_File_Hook} declared as
#v+
      int (*SLang_Load_File_Hook)(char *);
#v-
   is non-NULL, the function point to by it will be used to load the
   file.  For example, the \jed editor uses this hook to load files
   via its own routines.
\seealso{SLang_load_object, SLang_load_string}
\done

\function{SLang_restart}
\synopsis{Reset the interpreter after an error}
\usage{void SLang_restart (int full)}
\description
   The \var{SLang_restart} function should be called by the
   application at top level if an error occurs.  If the parameter
   \var{full} is non-zero, any objects on the \slang run time stack
   will be removed from the stack; otherwise, the stack will be left
   intact.  Any time the stack is believed to be trashed, this routine
   should be called with a non-zero argument (e.g., if
   \var{setjmp}/\var{longjmp} is called).

   Calling \var{SLang_restart} does not reset the global variable
   \var{SLang_Error} to zero.  It is up to the application to reset
   that variable to zero after calling \var{SLang_restart}.
\example
#v+
      while (1)
        {
           if (SLang_Error)
             {
                SLang_restart (1);
                SLang_Error = 0;
             }
           (void) SLang_load_file (NULL);
        }
#v-
\seealso{SLang_init_slang, SLang_load_file}
\done

\function{SLang_byte_compile_file}
\synopsis{Byte-compile a file for faster loading}
\usage{int SLang_byte_compile_file(char *fn, int reserved)}
\description
  The \var{SLang_byte_compile_file} function ``byte-compiles'' the
  file \var{fn} for faster loading by the interpreter.  This produces
  a new file whose filename is equivalent to the one specified by
  \var{fn}, except that a \var{'c'} is appended to the name.  For
  example, if \var{fn} is set to \exmp{init.sl}, then the new file
  will have the name exmp{init.slc}.  The meaning of the second
  parameter, \var{reserved}, is reserved for future use.  For now, set
  it to \var{0}.

  The function returns zero upon success, or \exmp{-1} upon error and
  sets SLang_Error accordingly.
\seealso{SLang_load_file, SLang_init_slang}
\done

\function{SLang_autoload}
\synopsis{Autoload a function from a file}
\usage{int SLang_autoload(char *funct, char *filename)}
\description
  The \var{SLang_autoload} function may be used to associate a
  \var{slang} function name \var{funct} with the file \var{filename}
  such that if \var{funct} has not already been defined when needed,
  it will be loaded from \var{filename}.

  \var{SLang_autoload} has no effect if \var{funct} has already been
  defined.  Otherwise it declares \var{funct} as a user-defined \slang
  function.  It returns \exmp{0} upon success, or \exmp{-1} upon error.
\seealso{SLang_load_file, SLang_is_defined}
\done

\function{SLang_load_string}
\synopsis{Interpret a string}
\usage{int SLang_load_string(char *str)}
\description
  The \var{SLang_load_string} function feeds the string specified by
  \var{str} to the interpreter for execution.  It returns zero upon
  success, or \exmp{-1} upon failure.
\seealso{SLang_load_file, SLang_load_object}
\done

\function{SLdo_pop}
\synopsis{Delete an object from the stack}
\usage{int SLdo_pop(void)}
\description
   This function removes an object from the top of the interpeter's
   run-time stack and frees any memory associated with it.  It returns
   zero upon success, or \var{-1} upon error (most likely due to a
   stack-underflow).
\seealso{SLdo_pop_n, SLang_pop_integer, SLang_pop_string}
\done

\function{SLdo_pop_n}
\synopsis{Delete n objects from the stack}
\usage{int SLdo_pop_n (unsigned int n)}
\description
   The \var{SLdo_pop_n} function removes the top \var{n} objects from
   the interpreter's run-time stack and frees all memory associated
   with the objects.  It returns zero upon success, or \var{-1} upon
   error (most likely due to a stack-underflow).
\seealso{SLdo_pop, SLang_pop_integer, SLang_pop_string}
\done

\function{SLang_pop_integer}
\synopsis{Pop an integer off the stack}
\usage{int SLang_pop_integer (int *i)}
\description
   The \var{SLang_pop_integer} function removes an integer from the
   top of the interpreter's run-time stack and returns its value via
   the pointer \var{i}.  If successful, it returns zero.  However, if
   the top stack item is not of type \var{SLANG_INT_TYPE}, or the
   stack is empty, the function will return \exmp{-1} and set
   \var{SLang_Error} accordingly.
\seealso{SLang_push_integer, SLang_pop_double}
\done

\function{SLpop_string}
\synopsis{Pop a string from the stack}
\usage{int SLpop_string (char **strptr);}
\description
   The \var{SLpop_string} function pops a string from the stack and
   returns it as a malloced pointer.  It is up to the calling routine
   to free this string via a call to \var{free} or \var{SLfree}.  If
   successful, \var{SLpop_string} returns zero.  However, if the top
   stack item is not of type \var{SLANG_STRING_TYPE}, or the stack is
   empty, the function will return \exmp{-1} and set
   \var{SLang_Error} accordingly.
\example
#v+
      define print_string (void)
      {
         char *s;
         if (-1 == SLpop_string (&s))
           return;
         fputs (s, stdout);
         SLfree (s);
      }
#v-
\notes
   This function should not be confused with \var{SLang_pop_slstring},
   which pops a \em{hashed} string from the stack.
\seealso{SLang_pop_slstring. SLfree}
\done

\function{SLang_pop_string}
\synopsis{Pop a string from the stack}
\usage{int SLang_pop_string(char **strptr, int *do_free)}
\description
   The \var{SLpop_string} function pops a string from the stack and
   returns it as a malloced pointer via \var{strptr}.  After the
   function returns, the integer pointed to by the second parameter
   will be set to a non-zero value if \var{*strptr} should be freed via
   \var{free} or \var{SLfree}.  If successful, \var{SLpop_string}
   returns zero.  However, if the top stack item is not of type
   \var{SLANG_STRING_TYPE}, or the stack is empty, the function will
   return \exmp{-1} and set \var{SLang_Error} accordingly.
\notes
   This function is considered obsolete and should not be used by
   applications.  If one requires a malloced string for modification,
   \var{SLpop_string} should be used.  If one requires a constant
   string that will not be modifed by the application,
   \var{SLang_pop_slstring} should be used.
\seealso{SLang_pop_slstring, SLpop_string}
\done

\function{SLang_pop_slstring}
\synopsis{Pop a hashed string from the stack}
\usage{int SLang_pop_slstring (char **s_ptr)}
\description
   The \var{SLang_pop_slstring} function pops a hashed string from the
   \slang run-time stack and returns it via \var{s_ptr}.  It returns
   zero if successful, or \-1 upon failure.  The resulting string
   should be freed via a call to \var{SLang_free_slstring} after use.
\example
#v+
   void print_string (void)
   {
      char *s;
      if (-1 == SLang_pop_slstring (&s))
        return;
      fprintf (stdout, "%s\n", s);
      SLang_free_slstring (s);
   }
#v-
\notes
   \var{SLang_free_slstring} is the preferred function for popping
   strings.  This is a result of the fact that the interpreter uses
   hashed strings as the native representation for string data.

   One must \em{never} free a hashed string using \var{free} or
   \var{SLfree}.  In addition, one must never make any attempt to
   modify a hashed string and doing so will result in memory
   corruption.
\seealso{SLang_free_slstring, SLpop_string}
\done

\function{SLang_pop_double}
\synopsis{Pop a double from the stack}
\usage{int SLang_pop_double (double *dptr)}
\description
   The \var{SLang_pop_double} function pops a double precision number
   from the stack and returns it via \var{dptr}.  This
   function returns \0 upon success, otherwise it returns \-1 and sets
   \var{SLang_Error} accordingly.
\seealso{SLang_pop_integer, SLang_push_double}
\done

\function{SLang_pop_complex}
\synopsis{Pop a complex number from the stack}
\usage{int SLang_pop_complex (double *re, double *im)}
\description
   \var{SLang_pop_complex} pops a complex number from the stack and
   returns it via the parameters \var{re} and \var{im} as the real and
   imaginary parts of the complex number, respectively.  This function
   automatically converts objects of type \var{SLANG_DOUBLE_TYPE} and
   \var{SLANG_INT_TYPE} to \var{SLANG_COMPLEX_TYPE}, if necessary.
   It returns zero upon success, or \-1 upon error setting
   \var{SLang_Error} accordingly.
\seealso{SLang_pop_integer, SLang_pop_double, SLang_push_complex}
\done

\function{SLang_push_complex}
\synopsis{Push a complex number onto the stack}
\usage{int SLang_push_complex (double re, double im)}
\description
   \var{SLang_push_complex} may be used to push the complex number
   whose real and imaginary parts are given by \var{re} and \var{im},
   respectively.  It returns zero upon success, or \-1 upon error
   setting \var{SLang_Error} accordingly.
\seealso{SLang_pop_complex, SLang_push_double}
\done

\function{SLang_push_double}
\synopsis{Push a double onto the stack}
\usage{int SLang_push_double(double d)}
\description
   \var{SLang_push_double} may be used to push the double precision
   floating point number \var{d} onto the interpreter's run-time
   stack.  It returns zero upon success, or \-1 upon error setting
   \var{SLang_Error} accordingly.
\seealso{SLang_pop_double, SLang_push_integer}
\done

\function{SLang_push_string}
\synopsis{Push a string onto the stack}
\usage{int SLang_push_string (char *s)}
\description
   \var{SLang_push_string} pushes a copy of the string specified by
   \var{s} onto the interpreter's run-time stack.  It returns zero
   upon success, or \-1 upon error setting \var{SLang_Error}
   accordingly.
\notes
   If \var{s} is \var{NULL}, this function pushes \var{NULL}
   (\var{SLANG_NULL_TYPE}) onto the stack.
\seealso{SLang_push_malloced_string}
\done

\function{SLang_push_integer}
\synopsis{Push an integer onto the stack}
\usage{int SLang_push_integer (int i)}
\description
   \var{SLang_push_integer} the integer \var{i} onto the interpreter's
   run-time stack.  It returns zero upon success, or \-1 upon error
   setting \var{SLang_Error} accordingly.
\seealso{SLang_pop_integer, SLang_push_double, SLang_push_string}
\done

\function{SLang_push_malloced_string}
\synopsis{Push a malloced string onto the stack}
\usage{int SLang_push_malloced_string (char *s);}
\description
   \var{SLang_push_malloced_string} may be used to push a malloced
   string onto the interpreter's run-time stack.  It returns zero upon
   success, or \-1 upon error setting \var{SLang_Error} accordingly.
\example
   The following example illustrates that it is up to the calling
   routine to free the string if \var{SLang_push_malloced_string} fails:
#v+
      int push_hello (void)
      {
         char *s = malloc (6);
         if (s == NULL) return -1;
         strcpy (s, "hello");
         if (-1 == SLang_push_malloced_string (s))
           {
              free (s);
              return -1;
           }
         return 0;
      }
#v-
\example
   The function \var{SLang_create_slstring} returns a hashed string.
   Such a string may not be malloced and should not be passed to
   \var{SLang_push_malloced_string}.
\notes
   If \var{s} is \var{NULL}, this function pushes \var{NULL}
   (\var{SLANG_NULL_TYPE}) onto the stack.
\seealso{SLang_push_string, SLmake_string}
\done

\function{SLang_is_defined}
\synopsis{Check to see if the interpreter defines an object}
\usage{int SLang_is_defined (char *nm)}
\description
   The \var{SLang_is_defined} function may be used to determine
   whether or not a variable or function whose name is given by
   \var{em} has been defined.  It returns zero if no such object has
   been defined.  Otherwise it returns a non-zero value according to
   the following table:
#v+
      1    intrinsic function
      2    user-defined slang function
     -1    intrinsic variable
     -2    user-defined global variable
#v-
   Note that variables correspond to negative numbers and functions
   are represented by positive numbers.
\seealso{SLadd_intrinsic_function, SLang_run_hooks, SLang_execute_function}
\done

\function{SLang_run_hooks}
\synopsis{Run a user-defined hook with arguments}
\usage{int SLang_run_hooks (char *fname, unsigned int n, ...)}
\description
   The \var{SLang_run_hooks} function may be used to execute a
   user-defined function named \var{fname}.  Before execution of the
   function, the \var{n} string arguments specified by the variable
   parameter list are pushed onto the stack.  If the function
   \var{fname} does not exist, \var{SLang_run_hooks} returns zero;
   otherwise, it returns \exmp{1} upon successful execution of the
   function, or \-1 if an error occurred.
\example
   The \jed editor uses \var{SLang_run_hooks} to setup the mode of a
   buffer based on the filename extension of the file associated with
   the buffer:
#v+
      char *ext = get_filename_extension (filename);
      if (ext == NULL) return -1;
      if (-1 == SLang_run_hooks ("mode_hook", 1, ext))
        return -1;
      return 0;
#v-
\seealso{SLang_is_defined, SLang_execute_function}
\done

\function{SLang_execute_function}
\synopsis{Execute a user or intrinsic function}
\usage{int SLang_execute_function (char *fname)}
\description
   This function may be used to execute either a user-defined function
   or an intrinisic function.  The name of the function is specified
   by \var{fname}.  It returns zero if \var{fname} is not defined, or
   \exmp{1} if the function was successfully executed, or \-1 upon
   error.
\notes
   The function \var{SLexecute_function} may be a better alternative
   for some uses.
\seealso{SLang_run_hooks, SLexecute_function, SLang_is_defined}
\done

\function{SLang_get_function}
\synopsis{Get a pointer to a \slang function}
\usage{SLang_Name_Type *SLang_get_function (char *fname)}
\description
  This function returns a pointer to the internal \slang table entry
  of a function whose name is given by \var{fname}.  It returns
  \var{NULL} upon failure.  The value returned by this function can be
  used \var{SLexecute_function} to call the function directly
  from C.
\seealso{SLexecute_function}
\done

\function{SLexecute_function}
\synopsis{Execute a \slang or intrinsic function}
\usage{int SLexecute_function (SLang_Name_Type *nt)}
\description
  The \var{SLexecute_function} allows an application to call the
  \slang function specified by the \var{SLang_Name_Type} pointer
  \var{nt}.  This parameter must be non \var{NULL} and must have been
   previously obtained by a call to \var{SLang_get_function}.
\example
   Consider the \slang function:
#v+
     define my_fun (x)
     {
        return x^2 - 2;
     }
#v-
   Suppose that it is desired to call this function many times with
   different values of x.  There are at least two ways to do this.
   The easiest way is to use \var{SLang_execute_function} by passing
   the string \exmp{"my_fun"}.  A better way that is much faster is to
   use \var{SLexecute_function}:
#v+
      int sum_a_function (char *fname, double *result)
      {
         double sum, x, y;
         SLang_Name_Type *nt;

         if (NULL == (nt = SLang_get_function (fname)))
           return -1;

         sum = 0;
         for (x = 0; x < 10.0; x += 0.1)
           {
              SLang_start_arg_list ();
              if (-1 == SLang_push_double (x))
                return -1;
              SLang_end_arg_list ();
              if (-1 == SLexecute_function (nt))
                return -1;
              if (-1 == SLang_pop_double (&y))
                return -1;

              sum += y;
           }
         return sum;
      }
#v-
   Although not necessary in this case, \var{SLang_start_arg_list} and
   \var{SLang_end_arg_list} were used to provide the function with
   information about the number of parameters passed to it.
\seealso{SLang_get_function, SLang_start_arg_list, SLang_end_arg_list}
\done

\function{SLang_peek_at_stack}
\synopsis{Find the type of object on the top of the stack}
\usage{int SLang_peek_at_stack (void)}
\description
  The \var{SLang_peek_at_stack} function is useful for determining the
  data type of the object at the top of the stack.  It returns the
  data type, or -1 upon a stack-underflow error.  It does not remove
  anything from the stack.
\seealso{SLang_pop_string, SLang_pop_integer}
\done

\function{SLang_pop_fileptr}
\synopsis{Pop a file pointer}
\usage{int SLang_pop_fileptr (SLang_MMT_Type **mmt, FILE **fp)}
\description
  \var{SLang_pop_fileptr} pops a file pointer from the \slang
  run-time stack.  It returns zero upon success, or \-1 upon failure.

  A \slang file pointer (SLANG_FILEPTR_TYPE) is actually a memory
  managed object.  For this reason, \var{SLang_pop_fileptr} also
  returns the memory managed object via the argument list.  It is up
  to the calling routine to call \var{SLang_free_mmt} to free the
  object.
\example
  The following example illustrates an application defined intrinsic
  function that writes a user defined double precision number to a
  file.  Note the use of \var{SLang_free_mmt}:
#v+
     int write_double (void)
     {
        double t;
        SLang_MMT_Type *mmt;
        FILE *fp;
        int status;

        if (-1 == SLang_pop_double (&d, NULL, NULL))
          return -1;
        if (-1 == SLang_pop_fileptr (&mmt, &fp))
          return -1;

        status = fwrite (&d, sizeof (double), 1, fp);
        SLang_free_mmt (mmt);
        return status;
     }
#v-
  This function can be used by a \slang function as follows:
#v+
     define write_some_values ()
     {
        variable fp, d;

        fp = fopen ("myfile.dat", "wb");
        if (fp == NULL)
          error ("file failed to open");
        for (d = 0; d < 10.0; d += 0.1)
          {
             if (-1 == write_double (fp, d))
               error ("write failed");
          }
        if (-1 == fclose (fp))
          error ("fclose failed");
     }
#v-
\seealso{SLang_free_mmt, SLang_pop_double}
\done

\function{SLadd_intrinsic_function}
\synopsis{Add a new intrinsic function to the interpreter}
\usage{int SLadd_intrinsic_function (name, f, type, nargs, ...)}
#v+
    char *name
    FVOID_STAR f
    SLtype type
    unsigned int nargs
#v-
\description
  The \var{SLadd_intrinsic_function} function may be used to add a new
  intrinsic function.  The \slang name of the function is specified by
  \var{name} and the actual function pointer is given by \var{f}, cast
  to \var{FVOID_STAR}.  The third parameter, \var{type} specifies the
  return type of the function and must be one of the following values:
#v+
    SLANG_VOID_TYPE   (returns nothing)
    SLANG_INT_TYPE    (returns int)
    SLANG_DOUBLE_TYPE (returns double)
    SLANG_STRING_TYPE (returns char *)
#v-
  The \var{nargs} parameter specifies the number of parameters to pass
  to the function.  The variable argument list following \var{nargs}
  must consists of \var{nargs} integers which specify the data type of
  each argument.

  The function returns zero upon success or \-1 upon failure.
\example
  The \jed editor uses this function to change the \var{system}
  intrinsic function to the following:
#v+
     static int jed_system (char *cmd)
     {
        if (Jed_Secure_Mode)
          {
             msg_error ("Access denied.");
             return -1;
          }
        return SLsystem (cmd);
     }
#v-
  After initializing the interpreter with \var{SLang_init_slang},
  \jed calls \var{SLadd_intrinsic_function} to substitute the above
  definition for the default \slang definition:
#v+
     if (-1 == SLadd_intrinsic_function ("system", (FVOID_STAR)jed_system,
                                         SLANG_INT_TYPE, 1,
                                         SLANG_STRING_TYPE))
       return -1;
#v-
\seealso{SLadd_intrinsic_variable, SLadd_intrinsic_array}
\done

\function{SLadd_intrinsic_variable}
\synopsis{Add an intrinsic variable to the interpreter}
\usage{int SLadd_intrinsic_variable (name, addr, type, rdonly)}
#v+
    char *name
    VOID_STAR addr
    SLtype type
    int rdonly
#v-
\description
  The \var{SLadd_intrinsic_variable} function adds an intrinsic
  variable called \var{name} to the interpeter.  The second parameter
  \var{addr} specifies the address of the variable (cast to
  \var{VOID_STAR}).  The third parameter, \var{type}, specifies the
  data type of the variable.  If the fourth parameter, \var{rdonly},
  is non-zero, the variable will interpreted by the interpreter as
  read-only.

  If successful, \var{SLadd_intrinsic_variable} returns zero,
  otherwise it returns \-1.
\example
  Suppose that \var{My_Global_Int} is a global variable (at least not
  a local one):
#v+
    int My_Global_Int;
#v-
  It can be added to the interpreter via the function call
#v+
    if (-1 == SLadd_intrinsic_variable ("MyGlobalInt",
                                        (VOID_STAR)&My_Global_Int,
                                        SLANG_INT_TYPE, 0))
      exit (1);
#v-
\notes
  The current implementation requires all pointer type intrinsic
  variables to be read-only.  For example,
#v+
    char *My_Global_String;
#v-
  is of type \var{SLANG_STRING_TYPE}, and must be declared as
  read-only.  Finally, not that
#v+
   char My_Global_Char_Buf[256];
#v-
  is \em{not} a \var{SLANG_STRING_TYPE} object.  This difference is
  very important because internally the interpreter dereferences the
  address passed to it to get to the value of the variable.
\seealso{SLadd_intrinsic_function, SLadd_intrinsic_array}
\done

\function{SLclass_add_unary_op}
\synopsis{??}
\usage{int SLclass_add_unary_op (SLtype,int (*) (int, SLtype, VOID_STAR, unsigned int, VOID_STAR), int (*) (int, SLtype, SLtype *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_app_unary_op}
\synopsis{??}
\usage{int SLclass_add_app_unary_op (SLtype, int (*) (int,SLtype, VOID_STAR, unsigned int,VOID_STAR),int (*) (int, SLtype, SLtype *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_binary_op}
\synopsis{??}
\usage{int SLclass_add_binary_op (SLtype, SLtype,int (*)(int, SLtype, VOID_STAR, unsigned int,SLtype, VOID_STAR, unsigned int,VOID_STAR),int (*) (int, SLtype, SLtype, SLtype *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_math_op}
\synopsis{??}
\usage{int SLclass_add_math_op (SLtype,int (*)(int,SLtype, VOID_STAR, unsigned int,VOID_STAR),int (*)(int, SLtype, SLtype *));}
\description
??
\seealso{??}
\done

\function{SLclass_add_typecast}
\synopsis{??}
\usage{int SLclass_add_typecast (SLtype, SLtype int (*)_PROTO((SLtype, VOID_STAR, unsigned int,SLtype, VOID_STAR)),int);}
\description
??
\seealso{??}
\done
