#c -*- mode: tm; mode: fold -*-

#c text-macro definitions #%{{{
#i linuxdoc.tm
#i local.tm

#d function#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d variable#1 \sect{<bf>$1</bf>\label{$1}}<descrip>
#d synopsis#1 <tag> Synopsis </tag> $1
#d keywords#1 <tag> Keywords </tag> $1
#d usage#1 <tag> Usage </tag> <tt>$1</tt>
#d description <tag> Description </tag>
#d example <tag> Example </tag>
#d notes <tag> Notes </tag>
#d seealso#1 <tag> See Also </tag> <tt>$1</tt>
#d done </descrip><p>

#d proto#1 \tag{\tt{$1}}
#d documentstyle book

#d SLinterface#1 \bf{$1}

#d SLsmg \SLinterface{SLsmg}
#d SLtt \SLinterface{SLtt}
#d SLsearch \SLinterface{SLsearch}
#%}}}

\linuxdoc

\begin{\documentstyle}

#d DocTitle \slang-library-programmers-guide
\title \DocTitle (\docversion)
\author John E. Davis <www.jedsoft.org>
\date \__today__

\toc

#i preface.tm

\chapter{Introduction} #%{{{

  \slang is a C programmer's library that includes routines for the rapid
  development of sophisticated, user friendly, multi-platform applications.
  The \slang library includes the following:

\begin{itemize}
\item  Low level tty input routines for reading single characters at a time.
\item  Keymap routines for defining keys and manipulating multiple keymaps.
\item  A high-level keyprocessing interface (\verb{SLkp}) for
       handling function and arrow keys.
\item  High level screen management routines for manipulating both
       monochrome and color terminals.  These routines are \em{very}
       efficient. (\tt{SLsmg})
\item  Low level terminal-independent routines for manipulating the display
       of a terminal. (\tt{SLtt})
\item  Routines for reading single line input with line editing and recall
       capabilities. (\tt{SLrline})
\item  Searching functions: both ordinary searches and regular expression
       searches. (\tt{SLsearch})
\item  An embedded stack-based language interpreter with a C-like syntax.
\end{itemize}

  The library is currently available for OS/2, MSDOS, Unix, and VMS
  systems.  For the most part, the interface to library routines has
  been implemented in such a way that it appears to be platform
  independent from the point of view of the application.  In addition,
  care has been taken to ensure that the routines are ``independent''
  of one another as much as possible.  For example, although the
  keymap routines require keyboard input, they are not tied to
  \slang's keyboard input routines--- one can use a different keyboard
  \verb{getkey} routine if one desires.  This also means that linking
  to only part of the \slang library does not pull the whole library
  into the application.  Thus, \slang applications tend to be
  relatively small in comparison to programs that use libraries with
  similar capabilities.

#%}}}

\chapter{Error Handling} #%{{{

  Many of the \slang functions return 0 upon success or -1 to signify
  failure.  Other functions may return \NULL to indicate failure. In
  addition, upon failure, many will set the error state of the library
  to a value that indicates the nature of the error. The value of this
  state may be queried via the \cfun{SLang_get_error} function.  This
  function will return 0 to indicate that there is no error, or a
  non-zero value such as one of the following constants:
#v+
     SL_Any_Error                      SL_Index_Error
     SL_OS_Error                       SL_Parse_Error
     SL_Malloc_Error                   SL_Syntax_Error
     SL_IO_Error                       SL_DuplicateDefinition_Error
     SL_Write_Error                    SL_UndefinedName_Error
     SL_Read_Error                     SL_Usage_Error
     SL_Open_Error                     SL_Application_Error
     SL_RunTime_Error                  SL_Internal_Error
     SL_InvalidParm_Error              SL_NotImplemented_Error
     SL_TypeMismatch_Error             SL_LimitExceeded_Error
     SL_UserBreak_Error                SL_Forbidden_Error
     SL_Stack_Error                    SL_Math_Error
     SL_StackOverflow_Error            SL_DivideByZero_Error
     SL_StackUnderflow_Error           SL_ArithOverflow_Error
     SL_ReadOnly_Error                 SL_ArithUnderflow_Error
     SL_VariableUninitialized_Error    SL_Domain_Error
     SL_NumArgs_Error                  SL_Data_Error
     SL_Unknown_Error                  SL_Unicode_Error
     SL_Import_Error                   SL_InvalidUTF8_Error
#v-
  For example, if a function tries to allocate memory but fails, then
  \cfun{SLang_get_error} will return \var{SL_Malloc_Error}.

  If the application makes use of the interpreter, then it is
  important that application-specific functions called from the
  interpreter set the error state of the library in order for
  exception handling to work.  This may be accomplished using the
  \cfun{SLang_set_error} function, e.g.,
#v+
     if (NULL == (fp = fopen (file, "r")))
       SLang_set_error (SL_Open_Error);
#v-

  Often it is desirable to give error message that contains more
  information about the error.  The \cfun{SLang_verror} function may
  be used for this purpose:
#v+
     if (NULL == (fp = fopen (file, "r")))
       SLang_verror (SL_Open_Error, "Failed to open %s: errno=%d",
                     file, errno);
#v-
  By default, \cfun{SLang_verror} will write the error message to
  \var{stderr}.  For applications that make use of the \SLsmg routines
  it is probably better for the error message to be printed to a
  specific area of the display.  The \var{SLang_Error_Hook} variable
  may be used to redirect error messages to an application defined
  function, e.g.,
#v+
     static void write_error (char *err)
     {
        SLsmg_gotorc (0, 0);
        SLsmg_set_color (ERROR_COLOR);
        SLsmg_write_string (err);
     }
     int main (int argc, char **argv)
     {
        /* Redirect error messages to write_error */
        SLang_Error_Hook = write_error;
              .
              .
     }
#v-

  Under extremely rare circumstances the library will call the C
  \cfun{exit} function causing the application to exit.  This will
  happen if the \cfun{SLtt_get_terminfo} is called but the terminal is
  not sufficiently powerful.  If this behavior is undesirable, then
  another function exists (\cfun{SLtt_initialize}) that returns an
  error code.  The other times the library will exit are when the
  interpreter is called upon to do something but has not been properly
  initialized by the application.  Such a condition is regarded as
  misuse of the libary and should be caught by routine testing of the
  application during development.  In any case, when the library does
  call the exit function, it will call an application-defined exit
  hook specified by the SLang_Exit_Error_Hook variable:
#v+
     static int exit_error_hook (char *fmt, va_list ap)
     {
        fprintf (stderr, "Fatal Error.  Reason:");
        vfprintf (stderr, fmt, va_list);
     }
     int main (int argc, char **argv)
     {
        SLang_Exit_Error_Hook = exit_error_hook;
          .
          .
     }
#v-
  The idea is that the hook can be used to perform some cleanup, free
  resources, and other tasks that the application needs to do for a
  clean exit.

#%}}}

\chapter{Unicode Support} #%{{{

  \slang has native support for the UTF-8 encoding of unicode in a
  number of its interfaces including the the \SLsmg screen mangement
  interface as well as the interpreter.  UTF-8 is a variable length
  multibyte encoding where unicode characters are represented by one
  to six bytes.  A technical description of the UTF-encoding is beyond
  the scope of this document, and as such the reader is advised to
  look elsewhere for a more detailed specification of the encoding.

  By default, the library's handling of UTF-8 is turned off.  It may
  be enabled by a call to the \ifun{SLutf8_enable} function:
#v+
    int SLutf8_enable (int mode)
#v-
  If the value of \exmp{mode} is 1, then the library will be put in
  UTF-8 mode.  If the value of \exmp{mode} is 0, then the library will
  be initialized with UTF-8 support disabled.  If the value is -1,
  then the mode will determined through an OS-dependent manner, e.g.,
  for Unix, the standard locale mechanism will be used.  The return
  value of this function will be 1 if UTF-8 support was activated, or
  0 if not.

  The above function determines the UTF-8 state of the library as a
  whole.  For some purposes it may be desirable to have more
  fine-grained control of the UTF-8 support.  For example, one might
  be using the \jed editor to view a UTF-8 encoded file but
  the terminal associated with the editor may not support UTF-8.  In
  such a case, one would want the \SLsmg interface to be in UTF-8 mode
  but lower-level \SLtt interface to not be in UTF-8 mode.  Hence, the
  following activation functions are also provided:
#v+
    int SLsmg_utf8_enable (int mode);
    int SLtt_utf8_enable (int mode);
    int SLinterp_utf8_enable (int mode);
#v-

  Note that once one of these interface specific functions has been
  called, any further calls to the umbrella function
  \cfun{SLutf8_enable} will have no effect on that interface.  For
  this reason, it is best to call \cfun{SLutf8_enable} first before
  the calling one of the interface-specific functions.

  Until support for Unicode is more widespread among users, it is
  expected that most users will still be using a national character
  set such as ASCII or iso-8869-1.  For example, iso-8869-1 is a very
  widespread character set used on Usenet.  As a result, applications
  will still have to provide support for such character sets.
  Unfortunately there appears to be no best way to do this.

  For the most part, the UTF-8 support should be largely transparent
  to the user.  For example, the interpreter treats all multibyte
  characters as a single character which means that the user does not
  have to be concerned about the internal representation of a
  character.  Rather one must keep in mind the distinction between a
  character and a byte.

#%}}}

\chapter{Interpreter Interface} #%{{{

  The \slang library provides an interpreter that when embedded into
  an application, makes the application extensible.  Examples of
  programs that embed the interpreter include the \jed editor and the
  \slrn newsreader.

  Embedding the interpreter is easy.  The hard part is to decide what
  application specific built-in or intrinsic functions should be
  provided by the application.  The \slang library provides some
  pre-defined intrinsic functions, such as string processing
  functions, and simple file input-output routines.  However, the
  basic philosophy behind the interpreter is that it is not a
  standalone program and it derives much of its power from the
  application that embeds it.

\sect{Embedding the Interpreter} #%{{{

  Only one function needs to be called to embed the \slang interpreter
  into an application: \cfun{SLang_init_slang}.  This function
  initializes the interpreter's data structures and adds some intrinsic
  functions:
#v+
      if (-1 == SLang_init_slang ())
        exit (EXIT_FAILURE);
#v-
  This function does not provide file input output intrinsic nor does
  it provide mathematical functions.  To make these as well as some
  posix system calls available use
#v+
     if ((-1 == SLang_init_slang ())    /* basic interpreter functions */
         || (-1 == SLang_init_slmath ()) /* sin, cos, etc... */
         || (-1 == SLang_init_array ()) /* sum, min, max, transpose... */
         || (-1 == SLang_init_stdio ()) /* stdio file I/O */
         || (-1 == SLang_init_ospath ()) /* path_concat, etc... */
         || (-1 == SLang_init_posix_dir ()) /* mkdir, stat, etc. */
         || (-1 == SLang_init_posix_process ()) /* getpid, umask, etc. */
         || (-1 == SLang_init_posix_io ()) /* open, close, read, ... */
         || (-1 == SLang_init_signal ()) /* signal, alarm, ... */
        )
       exit (EXIT_FAILURE);
#v-
  If you intend to enable all intrinsic functions, then it is simpler
  to initialize the interpreter via
#v+
     if (-1 == SLang_init_all ())
       exit (EXIT_FAILURE);
#v-
  See the \bf{\slang-intrinsic-function-reference} for more information about the
  intrinsic functions.

#%}}}

\sect{Calling the Interpreter} #%{{{

  There are several ways of calling the interpreter.  The two most common
  method is to load a file containing \slang code, or to load a
  string.

\sect1{Loading Files}
  The \cfun{SLang_load_file} and \cfun{SLns_load_file} functions may
  be used to interpret a file.  Both these functions return zero if
  successful, or \-1 upon failure.  If either of these functions fail,
  the interpreter will accept no more code unless the error state is
  cleared.  This is done by calling \cfun{SLang_restart} function to
  set the interpreter to its default state:
#v+
     if (-1 == SLang_load_file ("site.sl"))
       {
          /* Clear the error and reset the interpreter */
          SLang_restart (1);
       }
#v-

  When a file is loaded via \cfun{SLang_load_file}, any non-public
  variables and functions defined in the file will be placed into a
  namespace that is local to the file itself.  The
  \cfun{SLns_load_file} function may be used to load a file using a
  specified namespace, e.g.,
#v+
     if (-1 == SLns_load_file ("site.sl", "NS"))
       {
         SLang_restart (1);
         SLang_set_error (0);
       }
#v-
  will load \exfile{site.sl} into a namespace called \exns{NS}.  If such a
  namespace does not exist, then it will be created.

  Both the \cfun{SLang_load_file} and \cfun{SLns_load_file} functions
  search for files along an application-specified search path.  This
  path may be set using the \cfun{SLpath_set_load_path} function, as
  well as from interpeted code via the \ifun{set_slang_load_path}
  function.  By default, no search path is defined.

  \bf{
  NOTE: It is highly recommended that an application embedding the
  interpreter include the \slsh lib directory in the search path.  The
  \exfile{.sl} files that are part of \slsh are both useful and
  and should work with any application embedding the interpreter.
  Moreover, if the application permits dynamically loaded modules,
  then there are a growing number of excellent quality modules for
  \slsh that can be utilized by it.  Applications that follow this
  recommendation are said to be conforming.
  }

  Files are searched as follows: If the name begins with the
  equivalent of \exstr{./} or \exstr{../}, then it is searched for
  with respect to the current directory, and not along the load-path.
  If no such file exists, then an error will be generated. Otherwise,
  the file is searched for in each of the directories of the load-path
  by concatenating the path element with the specified file name.  The
  first such file found to exist by this process will be loaded.  If a
  matching file still has not been found, and the file name lacks an
  extension, then the path is searched with \exstr{.sl} and
  \exstr{.slc} appended to the filename.  If two such files are found
  (one ending with \exstr{.sl} and the other with \exstr{.slc}), then
  the more recent of the two will be used.  If no matching file has
  been found by this process, then the search will cease and an error
  generated.

  The search path is a delimiter separated list of directories that
  specify where the interpreter looks for files.  By default, the
  value of the delimiter is OS-dependent following the convention of
  the underlying OS.  For example, on Unix the delimiter is
  represented by a colon, on DOS/Windows it is a semi-colon, and on
  VMS it is a space.  The \cfun{SLpath_set_delimiter} and
  \cfun{SLpath_get_delimiter} may be used to set and query the
  delimiter's value, respectively.

\sect1{Loading Strings}
  There are several other mechanisms for interacting with the
  interpreter.  For example, the \cfun{SLang_load_string} function
  loads a string into the interpreter and interprets it:
#v+
    if (-1 == SLang_load_string ("message (\"hello\");"))
      return;
#v-
  Similarly, the \cfun{SLns_load_string} function may be used to load
  a string into a specified namespace.

  Typically, an interactive application will load a file via
  \cfun{SLang_load_file} and then go into a loop that consists of
  reading lines of input and sending them to the interpreter, e.g.,
#v+
      while (EOF != fgets (buf, sizeof (buf), stdin))
        {
           if (-1 == SLang_load_string (buf))
             {
                SLang_restart (1);
             }
        }
#v-

  Finally, some applications such as \jed and \slrn use another method of
  interacting with the interpreter.  They read key sequences from the
  keyboard and map those key sequences to interpreter functions via
  the \slang keymap interface.

#%}}}

\sect{Intrinsic Functions} #%{{{

  An intrinsic function is simply a function that is written in C and
  is made available to the interpreter as a built-in function.  For
  this reason, the words `intrinsic' and `built-in' are often used
  interchangeably.

  Applications are expected to add application specific functions to
  the interpreter.  For example, \jed adds nearly 300 editor-specific
  intrinsic functions.  The application designer should think
  carefully about what intrinsic functions to add to the interpreter.

\sect1{Restrictions on Intrinsic Functions} #%{{{

  When implementing intrinsic functions, it is necessary to follow a
  few rules to cooperate with the interpreter.

  The C version of an intrinsic function takes only pointer arguments.
  This is because when the interpreter calls an intrinsic function, it
  passes values to the function by reference and \em{not} by value. For
  example, intrinsic with the declarations:
#v+
     int intrinsic_0 (void);
     int intrinsic_1 (char *s);
     void intrinsic_2 (char *s, int *i);
     void intrinsic_3 (int *i, double *d, double *e);
#v-
  are all valid.  However,
#v+
     int invalid_1 (char *s, int len);
#v-
  is not valid since the \var{len} parameter is not a pointer.

  The return value of an intrinsic function must be one of the
  following types: \var{void}, \var{char}, \var{short}, \var{int},
  \var{long}, \var{double}, \var{char *}, as well as unsigned versions
  of the integer types.  A function such as
#v+
    int *invalid (void);
#v-
  is not permitted since \var{int*} is not a valid return-type for an
  intrinsic function.  Any other type of value can be passed back to
  the interpreter by explicitly pushing the object onto the
  interpreter's stack via the appropriate "push" function.

  The current implementation limits the number of arguments of an
  intrinsic function to \exmp{7}.  The "pop" functions can be used to
  allow the function to take an arbitrary number as seen from an
  interpreter script.

  Another restriction is that the intrinsic function should regard all its
  parameters as pointers to constant objects and make no attempt to
  modify the value to which they point.  For example,
#v+
      void truncate (char *s)
      {
         s[0] = 0;
      }
#v-
  is illegal since the function modifies the string \var{s}.

#%}}}

\sect1{Adding a New Intrinsic} #%{{{

  There are two basic mechanisms for adding an intrinsic function to the
  interpreter: \cfun{SLadd_intrinsic_function} and
  \cfun{SLadd_intrin_fun_table}.  Functions may be added to a specified
  namespace via \cfun{SLns_add_intrinsic_function} and
  \cfun{SLns_add_intrin_fun_table} functions.

  As an specific example, consider a function that will cause the
  program to exit via the \var{exit} C library function.  It is not
  possible to make this function an intrinsic because it does not meet
  the specifications for an intrinsic function that were described
  earlier.  However, one can call \var{exit} from a function that is
  suitable, e.g.,
#v+
     void intrin_exit (int *code)
     {
        exit (*code);
     }
#v-
  This function may be made available to the interpreter as an
  intrinsic via the \cfun{SLadd_intrinsic_function} routine:
#v+
     if (-1 == SLadd_intrinsic_function ("exit", (FVOID_STAR) intrin_exit,
                                         SLANG_VOID_TYPE, 1,
                                         SLANG_INT_TYPE))
       exit (EXIT_FAILURE);
#v-
  This statement basically tells the interpreter that
  \var{intrin_exit} is a function that returns nothing and takes a
  single argument: a pointer to an integer (\var{SLANG_INT_TYPE}).
  A user can call this function from within the interpreter
  via
#v+
     message ("Calling the exit function");
     exit (0);
#v-
  After printing a message, this will cause the \var{intrin_exit}
  function to execute, which in turn calls \var{exit}.

  The most convenient mechanism for adding new intrinsic functions is
  to create a table of \cfun{SLang_Intrin_Fun_Type} objects and add the
  table via the \cfun{SLadd_intrin_fun_table} function.  The table will
  look like:
#v+
    SLang_Intrin_Fun_Type My_Intrinsics [] =
    {
     /* table entries */
      MAKE_INTRINSIC_N(...),
      MAKE_INTRINSIC_N(...),
            .
            .
      MAKE_INTRINSIC_N(...),
      SLANG_END_INTRIN_FUN_TABLE
    };
#v-
  Construction of the table entries may be facilitated using a set of
  \var{MAKE_INTRINSIC} macros defined in \var{slang.h}.  The main
  macro is called \var{MAKE_INTRINSIC_N} and takes 11 arguments:
#v+
    MAKE_INTRINSIC_N(name, funct-ptr, return-type, num-args,
                     arg-1-type, arg-2-type, ... arg-7-type)
#v-
  Here \var{name} is the name of the intrinsic function that the
  interpreter is to give to the function. \var{func-ptr} is a pointer
  to the intrinsic function taking \var{num-args} and returning
  \var{ret-type}.  The final \exmp{7} arguments specify the argument
  types.  For example, the \var{intrin_exit} intrinsic described above
  may be added to the table using
#v+
    MAKE_INTRINSIC_N("exit", intrin_exit, SLANG_VOID_TYPE, 1,
                     SLANG_INT_TYPE, 0,0,0,0,0,0)
#v-

  While \var{MAKE_INTRINSIC_N} is the main macro for constructing
  table entries, \var{slang.h} defines other macros that may prove
  useful.  In particular, an entry for the \var{intrin_exit} function
  may also be created using any of the following forms:
#v+
    MAKE_INTRINSIC_1("exit", intrin_exit, SLANG_VOID_TYPE, SLANG_INT_TYPE)
    MAKE_INTRINSIC_I("exit", intrin_exit, SLANG_VOID_TYPE)
#v-
  See \var{slang.h} for related macros.  You are also encouraged to
  look at, e.g., \var{slang/src/slstd.c} for a more extensive examples.

  The table may be added via the \cfun{SLadd_intrin_fun_table}
  function, e.g.,
#v+
    if (-1 == SLadd_intrin_fun_table (My_Intrinsics, NULL))
      {
         /* an error occurred */
      }
#v-
  Please note that there is no need to load a given table more than
  once, and it is considered to be an error on the part of the
  application it adds the same table multiple times.  For performance
  reasons, no checking is performed by the library to see if a table
  has already been added.

  Earlier it was mentioned that intrinsics may be added to a specified
  namespace.  To this end, one must first get a pointer to the
  namespace via the \cfun{SLns_create_namespace} function.  The
  following example illustrates how this function is used to add the
  \var{My_Intrinsics} table to a namespace called \exmp{my}:
#v+
   SLang_NameSpace_Type *ns = SLns_create_namespace ("my");
   if (ns == NULL)
     return -1;

   return SLns_add_intrin_fun_table (ns, My_Intrinsics, "__MY__"));
#v-

#%}}}

\sect1{More Complicated Intrinsics} #%{{{
  The intrinsic functions described in the previous example were
  functions that took a fixed number of arguments.  In this section we
  explore more complex intrinsics such as those that take a variable
  number of arguments.

  Consider a function that takes two double precision numbers and
  returns the lesser:
#v+
     double intrin_min (double *a, double *b)
     {
        if (*a < *b) return *a;
        return *b;
     }
#v-
  This function may be added to a table of intrinsics using
#v+
    MAKE_INTRINSIC_2("vmin", intrin_min, SLANG_DOUBLE_TYPE,
                     SLANG_DOUBLE_TYPE, SLANG_DOUBLE_TYPE)
#v-
  It is useful to extend this function to take an arbitray number of
  arguments and return the lesser.  Consider the following variant:
#v+
    double intrin_min_n (int *num_ptr)
    {
       double min_value, x;
       unsigned int num = (unsigned int) *num_ptr;

       if (-1 == SLang_pop_double (&min_value))
         return 0.0;
       num--;

       while (num > 0)
         {
            num--;
            if (-1 == SLang_pop_double (&x))
              return 0.0;
            if (x < min_value) min_value = x;
         }
       return min_value;
    }
#v-
  Here the number to compare is passed to the function and the actual
  numbers are removed from the stack via the \cfun{SLang_pop_double}
  function.  A suitable table entry for it is
#v+
    MAKE_INTRINSIC_I("vmin", intrin_min_n, SLANG_DOUBLE_TYPE)
#v-
  This function would be used in an interpreter script via a statement
  such as
#v+
      variable xmin = vmin (x0, x1, x2, x3, x4, 5);
#v-
  which computes the smallest of \exmp{5} values.

  The problem with this intrinsic function is that the user must
  explicitly specify how many numbers to compare.  It would be more
  convenient to simply use
#v+
      variable xmin = vmin (x0, x1, x2, x3, x4);
#v-
  An intrinsic function can query the value of the variable
  \var{SLang_Num_Function_Args} to obtain the necessary information:
#v+
    double intrin_min (void)
    {
       double min_value, x;

       unsigned int num = SLang_Num_Function_Args;

       if (-1 == SLang_pop_double (&min_value, NULL, NULL))
         return 0.0;
       num--;

       while (num > 0)
         {
            num--;
            if (-1 == SLang_pop_double (&x, NULL, NULL))
              return 0.0;
            if (x < min_value) min_value = x;
         }
       return min_value;
    }
#v-
  This may be declared as an intrinsic using:
#v+
    MAKE_INTRINSIC_0("vmin", intrin_min, SLANG_DOUBLE_TYPE)
#v-

#%}}}

#%}}}

\sect{Intrinsic Variables} #%{{{

  It is possible to access an application's global variables from
  within the interpreter.  The current implementation supports the
  access of variables of type \var{int}, \var{char *}, and
  \var{double}.

  There are two basic methods of making an intrinsic variable
  available to the interpreter.  The most straight forward method is
  to use the function \cfun{SLadd_intrinsic_variable}:
#v+
     int SLadd_intrinsic_variable (char *name, VOID_STAR addr,
                                   SLtype data_type,
                                   int read_only);
#v-
  For example, suppose that \var{I} is an integer variable, e.g.,
#v+
     int I;
#v-
  One can make it known to the interpreter as \var{I_Variable} via a
  statement such as
#v+
     if (-1 == SLadd_intrinsic_variable ("I_Variable", &I,
                                          SLANG_INT_TYPE, 0))
       exit (EXIT_FAILURE);
#v-
  Similarly, if \var{S} is declared as
#v+
    char *S;
#v-
  then
#v+
     if (-1 == SLadd_intrinsic_variable ("S_Variable", &S,
                                          SLANG_STRING_TYPE, 1))
       exit (EXIT_FAILURE);
#v-
  makes \var{S} available as a \em{read-only} variable with the name
  \var{S_Variable}.  Note that if a pointer variable is made available
  to the interpreter, it should be declared as being \em{read-only} to
  prevent the interpreter from changing the pointer's value.

  It is important to note that if \var{S} were declared as an array of
  characters, e.g.,
#v+
     char S[256];
#v-
  then it would not be possible to make it directly available to the
  interpreter.  However, one could create a pointer to it, i.e.,
#v+
     char *S_Ptr = S;
#v-
  and make \var{S_Ptr} available as a read-only variable.

  One should not make the mistake of trying to use the same address
  for different variables as the following example illustrates:
#v+
     int do_not_try_this (void)
     {
        static char *names[3] = {"larry", "curly", "moe"};
        unsigned int i;

        for (i = 0; i < 3; i++)
          {
             int value;
             if (-1 == SLadd_intrinsic_variable (names[i], (VOID_STAR) &value,
                                                 SLANG_INT_TYPE, 1))
               return -1;
          }
        return 0;
     }
#v-
  Not only does this piece of code create intrinsic variables that use
  the same address, it also uses the address of a local variable that
  will go out of scope.

  The most convenient method for adding many intrinsic variables to
  the interpreter is to create an array of \var{SLang_Intrin_Var_Type}
  objects and then add the array via \cfun{SLadd_intrin_var_table}.
  For example, the array
#v+
    static SLang_Intrin_Var_Type Intrin_Vars [] =
    {
       MAKE_VARIABLE("I_Variable", &I, SLANG_INT_TYPE, 0),
       MAKE_VARIABLE("S_Variable", &S_Ptr, SLANG_STRING_TYPE, 1),
       SLANG_END_TABLE
    };
#v-
  may be added via
#v+
    if (-1 == SLadd_intrin_var_table (Intrin_Vars, NULL))
      exit (EXIT_FAILURE);
#v-
  It should be rather obvious that the arguments to the
  \var{MAKE_VARIABLE} macro correspond to the parameters of the
  \cfun{SLadd_intrinsic_variable} function.

  Finally, variables may be added to a specific namespace via the
  SLns_add_intrin_var_table and SLns_add_intrinsic_variable functions.

#%}}}

\sect{Aggregate Data Objects} #%{{{
 An aggregate data object is an object that can contain more than one
 data value.  The \slang interpreter supports several such objects:
 arrays, structure, and associative arrays.  In the following
 sections, information about interacting with these objects is given.

\sect1{Arrays} #%{{{
 An intrinsic function may interact with an array in several different
 ways.  For example, an intrinsic may create an array and return it.
 The basic functions for manipulating arrays include:
#v+
   SLang_create_array
   SLang_pop_array_of_type
   SLang_push_array
   SLang_free_array
   SLang_get_array_element
   SLang_set_array_element
#v-
 The use of these functions will be illustrated via a few simple
 examples.

 The first example shows how to create an return an array of strings
 to the interpreter.  In particular, the names of the four seasons of
 the year will be returned:
#v+
    void months_of_the_year (void)
    {
       static char *seasons[4] =
         {
            "Spring", "Summer", "Autumn", "Winter"
         };
       SLang_Array_Type *at;
       SLindex_Type i, four;

       four = 4;
       at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &four, 1);
       if (at == NULL)
         return;

       /* Now set the elements of the array */
       for (i = 0; i < 4; i++)
         {
           if (-1 == SLang_set_array_element (at, &i, &seasons[i]))
             {
                SLang_free_array (at);
                return;
             }
         }

      (void) SLang_push_array (at, 0);
      SLang_free_array (at);
    }
#v-

 This example illustrates several points:

 First of all, the
 \cfun{SLang_create_array} function was used to create a 1 dimensional
 array of 4 strings.  Since this function could fail, its return value
 was checked.  Also \var{SLindex_Type} was used for the array size and
 index types.  In \slang version 2, \var{SLindex_Type} is typedefed to
 be an \var{int}.  However, as this will change in a future version of
 the library, \var{SLindex_Type} should be used.

 The \cfun{SLang_set_array_element} function was
 used to set the elements of the newly created array.  Note that the
 address containing the value of the array element was passed and not
 the value of the array element itself.  That is,
#v+
    SLang_set_array_element (at, &i, seasons[i])
#v-
 was not used.  The return value from this function was also checked
 because it too could also fail.

 Finally, the array was pushed onto the interpreter's stack and then
 it was freed.  It is important to understand why it was freed.  This
 is because arrays are reference-counted.  When the array was created,
 it was returned with a reference count of \var{1}.  When it was
 pushed, the reference count was bumped up to \var{2}.  Then since it
 was nolonger needed by the function, \cfun{SLang_free_array} was
 called to decrement the reference count back to \var{1}.  For
 convenience, the second argument to \cfun{SLang_push_array}
 determines whether or not it is to also free the array.  So, instead
 of the two function calls:
#v+
   (void) SLang_push_array (at, 0);
   SLang_free_array (at);
#v-
 it is preferable to combine them as
#v+
   (void) SLang_push_array (at, 1);
#v-

 The second example returns a diagonal array of a specified size to
 the stack.  A diagonal array is a 2-d array with all elements zero
 except for those along the diagonal, which have a value of one:
#v+
   void make_diagonal_array (SLindex_Type n)
   {
      SLang_Array_Type *at;
      SLindex_Type dims[2];
      SLindex_Type i, one;

      dims[0] = dims[1] = n;
      at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, dims, 2);
      if (at == NULL)
        return;

      one = 1;
      for (i = 0; i < n; i++)
        {
           dims[0] = dims[1] = i;
           if (-1 == SLang_set_array_element (at, dims, &one))
             {
                SLang_free_array (at);
                return;
             }
        }

      (void) SLang_push_array (at, 1);
   }
#v-
  In this example, only the diagonal elements of the array were set.
  This is bacause when the array was created, all its elements were
  set to zero.

  Now consider an example that acts upon an existing array.  In
  particular, consider one that computes the trace of a 2-d matrix,
  i.e., the sum of the diagonal elements:
#v+
   double compute_trace (void)
   {
      SLang_Array_Type *at;
      double trace;
      SLindex_Type dims[2];

      if (-1 == SLang_pop_array_of_type (&at, SLANG_DOUBLE_TYPE))
        return 0.0;

      /* We want a 2-d square matrix.  If the matrix is 1-d and has only one
         element, then return that element. */
      trace = 0.0;
      if (((at->num_dims == 1) && (at->dims[0] == 1))
          || ((at->num_dims == 2) && (at->dims[0] == at->dims[1])))
        {
           double dtrace;
           SLindex_Type n = at->dims[0];

           for (i = 0; i < n; i++)
             {
                dims[0] = dims[1] = i;
                (void) SLang_get_array_element (at, &dims, &dtrace);
                trace += dtrace;
             }
        }
     else SLang_verror (SL_TYPE_MISMATCH, "Expecting a square matrix");

     SLang_free_array (at);
     return trace;
   }
#v-
 In this example, \cfun{SLang_pop_array_of_type} was used to pop an
 array of doubles from the stack.  This function will make implicit
 typecasts in order to return an array of the requested type.

#%}}}

\sect1{Structures} #%{{{

 For the purposes of this section, we shall differentiate structures
 according to whether or not they correspond to an application defined
 C structure.  Those that do are called intrinsic structures, and
 those do not are called \slang interpreter structures.

\sect2{Interpreter Structures}

 The following simple example shows one method that may be used to
 create and return a structure with a string and integer field to the
 interpreter's stack:
#v+
    int push_struct_example (char *string_value, int int_value)
    {
       char *field_names[2];
       SLtype field_types[2];
       VOID_STAR field_values[2];

       field_names[0] = "string_field";
       field_types[0] = SLANG_STRING_TYPE;
       field_values[0] = &string_value;

       field_names[1] = "int_field";
       field_types[1] = SLANG_INT_TYPE;
       field_values[1] = &int_value;

       if (-1 == SLstruct_create_struct (2, field_names,
                                            field_types, field_values))
         return -1;
       return 0;
    }
#v-
 Here, \cfun{SLstruct_create_struct} is used to push a
 structure with the specified field names and values onto the
 interpreter's stack.

 A simpler mechanism exists provided that one has already defined a C
 structure with a description of how the structure is laid out.  For
 example, consider a C structure defined by
#v+
    typedef struct
    {
       char *s;
       int i;
    }
    SI_Type;
#v-
 Its layout may be specified via a table of
 \var{SLang_CStruct_Field_Type} entries:
#v+
    SLang_CStruct_Field_Type SI_Type_Layout [] =
    {
      MAKE_CSTRUCT_FIELD(SI_Type, s, "string_field", SLANG_STRING_TYPE, 0),
      MAKE_CSTRUCT_FIELD(SI_Type, i, "int_field", SLANG_INT_TYPE, 0),
      SLANG_END_CSTRUCT_TABLE
    };
#v-
   Here, MAKE_CSTRUCT_FIELD is a macro taking 5 arguments:
#v+
    MAKE_CSTRUCT_FIELD(C-structure-type,
                       C-field-name,
                       slang-field-name,
                       slang-data-type,
                       is-read-only)
#v-
   The first argument is the structure type, the second is the name of
   a field of the structure, the third is a string that specifies the
   name of the corresponding field of the \slang structure, the fourth
   argument specifies the field's type, and the last argument
   specifies whether or not the field should be regarded as read-only.

   Once the layout of the structure has been specified, pushing a
   \slang version of the structure is trival:
#v+
    int push_struct_example (char *string_value, int int_value)
    {
       SI_Type si;

       si.s = string_value;
       si.i = int_value;
       return SLang_push_cstruct ((VOID_STAR)&si, SI_Type_Layout);
    }
#v-

   This mechanism of structure creation also permits a \slang
   structure to be passed to an intrinsic function through the use of
   the SLang_pop_cstruct routine, e.g.,
#v+
     void print_si_struct (void)
     {
        SI_Type si;
        if (-1 == SLang_pop_cstruct ((VOID_STAR)&si, SI_Type_Layout))
          return;
        printf ("si.i=%d", si.i);
        printf ("si.s=%s", si.s);
        SLang_free_cstruct ((VOID_STAR)&si, SI_Type_Layout);
     }
#v-
   Assuming \exmp{print_si_struct} exists as an intrinsic function,
   the \slang code
#v+
     variable s = struct {string_field, int_field};
     s.string_field = "hello";
     s.int_field = 20;
     print_si_struct (s);
#v-
   would result in the display of
#v+
     si.i=20;
     si.s=hello
#v-
   Note that the \cfun{SLang_free_cstruct} function was called after
   the contents of \exmp{si} were nolonger needed.  This was necessary
   because \cfun{SLang_pop_cstruct} allocated memory to set the
   \exmp{char *s} field of \exmp{si}.  Calling
   \cfun{SLang_free_cstruct} frees up such memory.

   Now consider the following:
#v+
    typedef struct
    {
       pid_t pid;
       gid_t group;
    }
    X_t;
#v-
  How should the layout of this structure be defined?  One might be
  tempted to use:
#v+
    SLang_CStruct_Field_Type X_t_Layout [] =
    {
      MAKE_CSTRUCT_FIELD(X_t, pid, "pid", SLANG_INT_TYPE, 0),
      MAKE_CSTRUCT_FIELD(X_t, group, "group", SLANG_INT_TYPE, 0),
      SLANG_END_CSTRUCT_TABLE
    };
#v-
  However, this assumes \exmp{pid_t} and \exmp{gid_t} have been
  typedefed as ints.  But what if \exmp{gid_t} is a \exmp{short}?  In
  such a case, using
#v+
      MAKE_CSTRUCT_FIELD(X_t, group, "group", SLANG_SHORT_TYPE, 0),
#v-
  would be the appropriate entry for the \exmp{group} field.  Of
  course, one has no way of knowing how \exmp{gid_t} is declared on
  other systems.  For this reason, it is preferable to use the
  \var{MAKE_CSTRUCT_INT_FIELD} macro in cases involving integer valued
  fields, e.g.,
#v+
    SLang_CStruct_Field_Type X_t_Layout [] =
    {
      MAKE_CSTRUCT_INT_FIELD(X_t, pid, "pid", 0),
      MAKE_CSTRUCT_INT_FIELD(X_t, group, "group", 0),
      SLANG_END_CSTRUCT_TABLE
    };
#v-

 Before leaving this section, it is important to mention that
 access to character array fields is not permitted via this
 interface.  That is, a structure such as
#v+
     typedef struct
     {
        char name[32];
     }
     Name_Type;
#v-
 is not supported since \exmp{char name[32]} is not a
 \var{SLANG_STRING_TYPE} object.  Always keep in mind that a
 \var{SLANG_STRING_TYPE} object is a \exmp{char *}.

\sect2{Intrinsic Structures}

 Here we show how to make intrinsic structures available to
 the interpreter.

 The simplest interface is to structure pointers and not
 to the actual structures themselves.  The latter would require the
 interpreter to be involved with the creation and destruction of the
 structures.  Dealing with the pointers themselves is far simpler.

 As an example, consider an object such as
#v+
    typedef struct _Window_Type
    {
       char *title;
       int row;
       int col;
       int width;
       int height;
    } Window_Type;
#v-
 which defines a window object with a title, size (\var{width},
 \var{height}), and location (\var{row}, \var{col}).

 We can make variables of type \var{Window_Type} available to the
 interpreter via a table as follows:
#v+
   static SLang_IStruct_Field_Type Window_Type_Field_Table [] =
   {
     MAKE_ISTRUCT_FIELD(Window_Type, title, "title", SLANG_STRING_TYPE, 1),
     MAKE_ISTRUCT_FIELD(Window_Type, row, "row", SLANG_INT_TYPE, 0),
     MAKE_ISTRUCT_FIELD(Window_Type, col, "col", SLANG_INT_TYPE, 0),
     MAKE_ISTRUCT_FIELD(Window_Type, width, "width", SLANG_INT_TYPE, 0),
     MAKE_ISTRUCT_FIELD(Window_Type, height, "height", SLANG_INT_TYPE, 0),
     SLANG_END_ISTRUCT_TABLE
   };
#v-
 More precisely, this defines the layout of the \var{Window_Type} structure.
 Here, the \var{title} has been declared as a read-only field.  Using
#v+
     MAKE_ISTRUCT_FIELD(Window_Type, title, "title", SLANG_STRING_TYPE, 0),
#v-
 would allow read-write access.

 Now suppose that \var{My_Window} is a pointer to a \var{Window_Type}
 object, i.e.,
#v+
    Window_Type *My_Window;
#v-
 We can make this variable available to the interpreter via the
 \cfun{SLadd_istruct_table} function:
#v+
    if (-1 == SLadd_istruct_table (Window_Type_Field_Table,
                                   (VOID_STAR) &My_Window,
                                   "My_Win"))
      exit (1);
#v-
 This creates a S-Lang interpreter variable called \var{My_Win} whose value
 corresponds to the \var{My_Win} structure.  This would permit one to
 access the fields of \var{My_Window} via \slang statements such as
#v+
     define set_width_and_height (w,h)
     {
         My_Win.width = w;
         My_Win.height = h;
     }
#v-

 It is extremely important to understand that the interface described in
 this section does not allow the interpreter to create new instances of
 \var{Window_Type} objects.  The interface merely defines an association or
 correspondence between an intrinsic structure pointer and a \slang
 variable.  For example, if the value of \var{My_Window} is \var{NULL}, then
 \var{My_Win} would also be \var{NULL}.

 One should be careful in allowing read/write access to character string
 fields.  If read/write access is allowed, then the application should
 always use the \cfun{SLang_create_slstring} and \cfun{SLang_free_slstring}
 functions to set the character string field of the structure.

#%}}}

#%}}}

\sect{Signals} #%{{{

  If your program that embeds the interpreter processes signals, then
  it may be undesirable to allow access to all signals from the
  interpreter.  For example, if your program has a signal handler for
  \ivar{SIGHUP} then it is possible that an interpreter script could
  specify a different signal handler, which may or may not be desirable.
  If you do not want to allow the interpreter access to some signal,
  then that signal can be made off-limits to the interpreter via the
  \cfun{SLsig_forbid_signal} function:
#v+
    /* forbid a signal handler for SIGHUP */
    SLsig_forbid_signal (SIGHUP, 1);

    /* Allow a signal handler for SIGTERM */
    SLsig_forbid_signal (SIGTERM, 0);
#v-

  By default, all signals are allowed access from the interpreter.

#%}}}

\sect{Exceptions}
#%}}}

\chapter{Keyboard Interface} #%{{{

#%{{{ Overview

  \slang's keyboard interface has been designed to allow an
  application to read keyboard input from the user in a
  system-independent manner.  The interface consists of a set of low
  routines for reading single character data as well as a higher
  level interface (\grp{SLkp}) which utilize \slang's keymap facility
  for reading multi-character sequences.

  To initialize the interface, one must first call the function
  \verb{SLang_init_tty}. Before exiting the program, the function
  \verb{SLang_reset_tty} must be called to restore the keyboard
  interface to its original state.  Once initialized, the low-level
  \verb{SLang_getkey} function may be used to read \em{simgle}
  keyboard characters from the terminal.  An application using the
  higher-level \grp{SLkp} interface will read charcters using the
  \verb{SLkp_getkey} function.

  In addition to these basic functions, there are also functions to
  ``unget'' keyboard characters, flush the input, detect pending-input
  with a timeout, etc. These functions are defined below.

#%}}}

\sect{Initializing the Keyboard Interface} #%{{{

  The function \verb{SLang_init_tty} must be called to initialize the
  terminal for single character input.  This puts the terminal in a mode
  usually referred to as ``raw'' mode.

  The prototype for the function is:
#v+
      int SLang_init_tty (int abort_char, int flow_ctrl, int opost);
#v-
  It takes three parameters that are used to specify how the terminal is to
  be initialized.
#%+
  Although the \slang keyboard interface has been
  %designed to be as system independent as possible, there are semantic
  % differences.
#%-

  The first parameter, \verb{abort_char}, is used to specify the interrupt
  character (\tt{SIGINT}).  Under MSDOS, this value corresponds to the scan
  code of the character that will be used to generate the interrupt.  For
  example, under MSDOS, \verb{34} should be used to make \key{Ctrl-G} generate an
  interrupt signal since 34 is the scan code for \key{G}.  On other
  systems, the value of \verb{abort_char} will simply be the ascii value of
  the control character that will be used to generate the interrupt signal,
  e.g., \tt{7} for \key{Ctrl-G}.  If \verb{-1} is passed, the interrupt
  character will not be changed.

  Pressing the interrupt character specified by the first argument will
  generate a signal (\tt{SIGINT}) that may or not be caught by the
  application.  It is up to the application to catch this signal.  \slang
  provides the function \verb{Slang_set_abort_signal} to make it easy to
  facilitate this task.

  The second parameter is used to specify whether or not flow control should
  be used.  If this parameter is zero, flow control is enabled otherwise
  it is disabled.  Disabling flow control is necessary to pass certain
  characters to the application (e.g., \key{Ctrl-S} and \key{Ctrl-Q}).
  For some systems such as MSDOS, this parameter is meaningless.

  The third parameter, \verb{opost}, is used to turn output processing on or
  off.  If \verb{opost} is zero, output processing is \em{not} turned on
  otherwise, output processing is turned on.

  The \verb{SLang_init_tty} function returns -1 upon failure.  In addition,
  after it returns, the \slang global variable \verb{SLang_TT_Baud_Rate}
  will be set to the baud rate of the terminal if this value can be
  determined.

  Example:
#v+
      if (-1 == SLang_init_tty (7, 0, 0))  /* For MSDOS, use 34 as scan code */
        {
          fprintf (stderr, "Unable to initialize the terminal.\n");
          exit (1);
        }
      SLang_set_abort_signal (NULL);
#v-
   Here the terminal is initialized such that flow control and output
   processing are turned off.  In addition, the character
   \key{Ctrl-G}\footnote{For MSDOS systems, use the \em{scan code} 34
   instead of 7 for \key{Ctrl-G}} has been specified to be the interrupt
   character.  The function \verb{SLang_set_abort_signal} is used to
   install the default \slang interrupt signal handler.

#%}}}

\sect{Resetting the Keyboard Interface} #%{{{

  The function \verb{SLang_reset_tty} must be called to reset the terminal
  to the state it was in before the call to \verb{SLang_init_tty}.  The
  prototype for this function is:
#v+
      void SLang_reset_tty (void);
#v-
  Usually this function is only called before the program exits.  However,
  if the program is suspended it should also be called just before suspension.

#%}}}

\sect{Initializing the \grp{SLkp} Routines} #%{{{

  Extra initialization of the higher-level \grp{SLkp} functions are
  required because they are layered on top of the lower level
  routines.  Since the \verb{SLkp_getkey} function is able to process
  function and arrow keys in a terminal independent manner, it is
  necessary to call the \verb{SLtt_get_terminfo} function to get
  information about the escape character sequences that the terminal's
  function keys send.  Once that information is available, the
  \verb{SLkp_init} function can construct the proper keymaps to
  process the escape sequences.

  This part of the initialization process for an application using
  this interface will look something like:

#v+
      SLtt_get_terminfo ();
      if (-1 == SLkp_init ())
        {
           SLang_doerror ("SLkp_init failed.");
           exit (1);
        }
      if (-1 == SLang_init_tty (-1, 0, 1))
        {
           SLang_doerror ("SLang_init_tty failed.");
           exit (1);
        }
#v-

  It is important to check the return status of the \verb{SLkp_init}
  function which can failed if it cannot allocate enough memory for
  the keymap.

#%}}}

\sect{Setting the Interrupt Handler} #%{{{

  The function \verb{SLang_set_abort_signal} may be used to associate an
  interrupt handler with the interrupt character that was previously
  specified by the \verb{SLang_init_tty} function call.  The prototype for
  this function is:
#v+
      void SLang_set_abort_signal (void (*)(int));
#v-
  This function returns nothing and takes a single parameter which is a
  pointer to a function taking an integer value and returning
  \verb{void}.  If a \verb{NULL} pointer is passed, the default \slang
  interrupt handler will be used. The \slang default interrupt handler
  under Unix looks like:
#v+
      static void default_sigint (int sig)
      {
        SLsignal_intr (SIGINT, default_sigint);
        SLKeyBoard_Quit = 1;
        if (SLang_Ignore_User_Abort == 0)
          SLang_set_error (SL_UserBreak_Error);
      }
#v-
  It simply sets the global variable \verb{SLKeyBoard_Quit} to one and
  if the variable \verb{SLang_Ignore_User_Abort} is non-zero,
  the error state is set to indicate a user break condition.  (The
  function \verb{SLsignal_intr} is similar to the standard C
  \verb{signal} function \em{except that it will interrupt system
  calls}.  Some may not like this behavior and may wish to call
  this \verb{SLang_set_abort_signal} with a different handler.)

  Although the function expressed above is specific to Unix, the
  analogous routines for other operating systems are equivalent in
  functionality even though the details of the implementation may vary
  drastically (e.g., under MSDOS, the hardware keyboard interrupt
  \verb{int 9h} is hooked).

#%}}}

\sect{Reading Keyboard Input with SLang_getkey} #%{{{

  After initializing the keyboard via \verb{SLang_init_tty},
  the \slang function \verb{SLang_getkey} may be used to read
  characters from the terminal interface.  In addition, the function
  \verb{SLang_input_pending} may be used to determine whether or not
  keyboard input is available to be read.

  These functions have prototypes:
#v+
      unsigned int SLang_getkey (void);
      int SLang_input_pending (int tsecs);
#v-
  The \verb{SLang_getkey} function returns a single character from the
  terminal.  Upon failure, it returns \verb{0xFFFF}.  If the interrupt
  character specified by the \verb{SLang_init_tty} function is pressed
  while this function is called, the function will return the value of
  the interrupt character and set the \slang global variable
  \verb{SLKeyBoard_Quit} to a non-zero value.  In addition, if the
  default \slang interrupt handler has been specified by a \verb{NULL}
  argument to the \verb{SLang_set_abort_signal} function, the error
  state of the library will be set to \verb{SL_UserBreak_Error}
  \em{unless} the variable \verb{SLang_Ignore_User_Abort} is non-zero.

  The \verb{SLang_getkey} function waits until input is available to be
  read.  The \verb{SLang_input_pending} function may be used to determine
  whether or not input is ready.  It takes a single parameter that indicates
  the amount of time to wait for input before returning with information
  regarding the availability of input.  This parameter has units of one
  tenth (1/10) of a second, i.e., to wait one second, the value of the
  parameter should be \tt{10}.  Passing a value of zero causes the function
  to return right away.  \verb{SLang_input_pending} returns a positive
  integer if input is available or zero if input is not available.  It will
  return -1 if an error occurs.

  Here is a simple example that reads keys from the terminal until one
  presses \key{Ctrl-G} or until 5 seconds have gone by with no input:
#v+
      #include <stdio.h>
      #include <slang.h>
      int main ()
      {
         int abort_char = 7;  /* For MSDOS, use 34 as scan code */
         unsigned int ch;

         if (-1 == SLang_init_tty (abort_char, 0, 1))
           {
              fprintf (stderr, "Unable to initialize the terminal.\n");
              exit (-1);
           }
         SLang_set_abort_signal (NULL);
         while (1)
           {
              fputs ("\nPress any key.  To quit, press Ctrl-G: ", stdout);
              fflush (stdout);
              if (SLang_input_pending (50) == 0)  /* 50/10 seconds */
                {
                   fputs ("Waited too long! Bye\n", stdout);
                   break;
                }

              ch = SLang_getkey ();
              if (SLang_get_error () == SL_UserBreak_Error)
                {
                   fputs ("Ctrl-G pressed!  Bye\n", stdout);
                   break;
                }
              putc ((int) ch, stdout);
           }
         SLang_reset_tty ();
         return 0;
      }
#v-

#%}}}

\sect{Reading Keyboard Input with SLkp_getkey} #%{{{

  Unlike the low-level function \verb{SLang_getkey}, the
  \verb{SLkp_getkey} function can read a multi-character sequence
  associated with function keys.  The \verb{SLkp_getkey} function uses
  \verb{SLang_getkey} and \slang's keymap facility to process escape
  sequences.  It returns a single integer which describes the key that
  was pressed:
#v+
      int SLkp_getkey (void);
#v-
  That is, the \verb{SLkp_getkey} function simple provides a mapping
  between keys and integers.  In this context the integers are called
  \em{keysyms}.

  For single character input such as generated by the \key{a} key on
  the keyboard, the function returns the character that was generated,
  e.g., \verb{'a'}.  For single characters, \verb{SLkp_getkey} will
  always return an keysym whose value ranges from 0 to 256. For
  keys that generate multiple character sequences, e.g., a function or
  arrow key, the function returns an keysym whose value is greater
  that 256.  The actual values of these keysyms are represented as
  macros defined in the \file{slang.h} include file.  For example, the
  up arrow key corresponds to the keysym whose value is
  \verb{SL_KEY_UP}.

  Since it is possible for the user to enter a character sequence that
  does not correspond to any key.  If this happens, the special keysym
  \verb{SL_KEY_ERR} will be returned.

  Here is an example of how \verb{SLkp_getkey} may be used by a file
  viewer:
#v+
      switch (SLkp_getkey ())
        {
           case ' ':
           case SL_KEY_NPAGE:
              next_page ();
              break;
           case 'b':
           case SL_KEY_PPAGE:
              previous_page ();
              break;
           case '\r':
           case SL_KEY_DOWN:
              next_line ();
              break;
               .
               .
           case SL_KEY_ERR:
           default:
              SLtt_beep ();
        }
#v-

   Unlike its lower-level counterpart, \verb{SLang_getkey}, there do
   not yet exist any functions in the library that are capable of
   ``ungetting'' keysyms.  In particular, the \verb{SLang_ungetkey}
   function will not work.

#%}}}

\sect{Buffering Input} #%{{{

  \slang has several functions pushing characters back onto the
  input stream to be read again later by \verb{SLang_getkey}.  It
  should be noted that none of the above functions are designed to
  push back keysyms read by the \verb{SLkp_getkey} function.  These
  functions are declared as follows:
#v+
      void SLang_ungetkey (unsigned char ch);
      void SLang_ungetkey_string (unsigned char *buf, int buflen);
      void SLang_buffer_keystring (unsigned char *buf, int buflen);
#v-

  \verb{SLang_ungetkey} is the most simple of the three functions.  It takes
  a single character a pushes it back on to the input stream.  The next call to
  \verb{SLang_getkey} will return this character.  This function may be used
  to \em{peek} at the character to be read by first reading it and then
  putting it back.

  \verb{SLang_ungetkey_string} has the same function as
  \verb{SLang_ungetkey} except that it is able to push more than one
  character back onto the input stream.  Since this function can push back
  null (ascii 0) characters, the number of characters to push is required as
  one of the parameters.

  The last of these three functions, \verb{SLang_buffer_keystring} can
  handle more than one charater but unlike the other two, it places the
  characters at the \em{end} of the keyboard buffer instead of at the
  beginning.

  Note that the use of each of these three functions will cause
  \verb{SLang_input_pending} to return right away with a non-zero value.

  Finally, the \slang keyboard interface includes the function
  \verb{SLang_flush_input} with prototype
#v+
      void SLang_flush_input (void);
#v-
  It may be used to discard \em{all} input.

  Here is a simple example that looks to see what the next key to be read is
  if one is available:
#v+
      int peek_key ()
      {
         int ch;
         if (SLang_input_pending (0) == 0) return -1;
         ch = SLang_getkey ();
         SLang_ungetkey (ch);
         return ch;
      }
#v-

#%}}}

\sect{Global Variables} #%{{{
  Although the following \slang global variables have already been
  mentioned earlier, they are gathered together here for completeness.

  \verb{int SLang_Ignore_User_Abort;}
  If non-zero, pressing the interrupt character will not result in
  the libraries error state set to \verb{SL_UserBreak_Error}.

  \verb{volatile int SLKeyBoard_Quit;}
  This variable is set to a non-zero value when the interrupt
  character is pressed. If the interrupt character is pressed when
  \verb{SLang_getkey} is called, the interrupt character will be
  returned from \verb{SLang_getkey}.

  \verb{int SLang_TT_Baud_Rate;}
  On systems which support it, this variable is set to the value of the
  terminal's baud rate after the call to \verb{SLang_init_tty}.

#%}}}

#%}}}

\chapter{Readline Interface}

  The \slang library includes simple but capable readline
  functionality in its \verb{SLrline} layer.  The \verb{SLrline}
  routines provide a simple mechanism for an application to get
  prompted input from a user with command line editing, completions,
  and history recall.

  The use of the \verb{SLrline} routines will be illustrated with a
  few simple examples.  All of the examples given in this section may
  be found in the file \verb{demo/rline.c} in the \slang source code
  distribution.  For clarity, the code shown below omits most error
  checking.

\sect{Introduction}

  The first example simply reads input from the user until the user
  enters \exmp{quit}:
#v+
   SLrline_Type *rl;
   SLang_init_tty (-1, 0, 1);
   rl = SLrline_open (80, SL_RLINE_BLINK_MATCH);
   while (1)
     {
       char *line;
       unsigned int len;

       line = SLrline_read_line (rl, "prompt>", &len);
       if (line == NULL) break;
       if (0 == strcmp (line, "quit"))
         {
            SLfree (line);
            break;
         }
       (void) fprintf (stdout, "\nRead %d bytes: %s\n", strlen(line), line);
       SLfree (line);
     }
   SLrline_close (rl);
   SLang_reset_tty ();
#v-
  In this example, the \verb{SLtt} interface functions
  \cfun{SLang_init_tty} and \cfun{SLang_reset_tty} functions have been
  used to open and close the terminal for reading input.  By default,
  the \verb{SLrline} functions use the \verb{SLang_getkey} function to
  read characters and assume that the terminal has been properly
  initialized before use.

  The \cfun{SLrline_open} function was used to create an instance of
  an \verb{SLrline_Type} object.  The function takes two arguments:
  and edit window display width (80 above), and a set of flags.  In
  this case, the \verb{SL_RLINE_BLINK_MATCH} flags has been used to
  turn on parenthesis blinking.  Once finished, the
  \exmp{SLrline_Type} object must be freed using the
  \exmp{SLrline_close} function.

  The actual reading of the line occurs in the
  \cfun{SLrline_read_line} function, which takes an
  \verb{SLrline_Type} instance and a string representing the prompt to
  be used.  The line itself is returned as a malloced \exmp{char *}
  and must be freed using the \cfun{SLfree} function after used.  The
  length (in bytes) of the line is returned via the parameter list.

  If an end-of-file character (\exmp{^D} on Unix) was entered at the
  beginning of a line, the \cfun{SLrline_read_line} function will
  return \NULL.  However, it also return \NULL if an error of
  some sort was encountered.  The only way to tell the difference
  between these two conditions is to call \cfun{SLang_get_error}.

  The above code fragment did not provide for any sort of
  \exmp{SIGINT} handling.  Without such a provision, pressing
  \exmp{^C} at the prompt could be enough to kill the application.
  This is especially undesirable if one wants to press \exmp{^C} to
  abort the call to \exmp{SLrline_read_line}.  The function
  \exmp{example_2} in \exmp{demo/rline.c} shows code to handle this
  situation as well as distinguish between EOF and other errors.

\sect{Interpreter Interface}

  \verb{SLrline} features such as command-line completion,
  vi-emulation, and so on are implemented through callbacks or hooks from
  the \verb{SLrline} functions to the \slang interpreter.  Hence, this
  functionality is only available to applications that make use of the
  interpreter.

  TBD...

\chapter{Screen Management} #%{{{

  The \slang library provides two interfaces to terminal independent
  routines for manipulating the display on a terminal.  The highest level
  interface, known as the \verb{SLsmg} interface is discussed in this
  section.  It provides high level screen management functions for
  manipulating the display in an optimal manner and is similar in spirit to
  the \verb{curses} library.  The lowest level interface, or the
  \verb{SLtt}
  interface, is used by the \verb{SLsmg} routines to actually perform the
  task of writing to the display.  This interface is discussed in another
  section.  Like the keyboard routines, the \verb{SLsmg} routines are
  \em{platform independent} and work the same on MSDOS, OS/2, Unix, and VMS.

  The screen management, or \verb{SLsmg}, routines are initialized by
  function \verb{SLsmg_init_smg}.  Once initialized, the application uses
  various \verb{SLsmg} functions to write to a \em{virtual} display.  This does
  not cause the \em{physical} terminal display to be updated immediately.
  The physical display is updated to look like the virtual display only
  after a call to the function \verb{SLsmg_refresh}.  Before exiting, the
  application using these routines is required to call
  \verb{SLsmg_reset_smg} to reset the display system.

  The following subsections explore \slang's screen management system in
  greater detail.

\sect{Initialization}

  The function \verb{SLsmg_init_smg} must be called before any other
  \verb{SLsmg} function can be used.  It has the simple prototype:
#v+
      int SLsmg_init_smg (void);
#v-
  It returns zero if successful or -1 if it cannot allocate space for
  the virtual display.

  For this routine to properly initialize the virtual display, the
  capabilities of the terminal must be known as well as the size of
  the \em{physical} display.  For these reasons, the lower level \verb{SLtt} routines
  come into play.  In particular, before the first call to
  \verb{SLsmg_init_smg}, the application is required to call the function
  \verb{SLtt_get_terminfo} before calling \verb{SLsmg_init_smg}.

  The \verb{SLtt_get_terminfo} function sets the global variables
  \verb{SLtt_Screen_Rows} and \verb{SLtt_Screen_Cols} to the values
  appropriate for the terminal.  It does this by calling the
  \verb{SLtt_get_screen_size} function to query the terminal driver
  for the appropriate values for these variables.  From this point on,
  it is up to the application to maintain the correct values for these
  variables by calling the \verb{SLtt_get_screen_size} function
  whenever the display size changes, e.g., in response to a
  \verb{SIGWINCH} signal. Finally, if the application is going to read
  characters from the keyboard, it is also a good idea to initialize
  the keyboard routines at this point as well.

\sect{Resetting SLsmg}

  Before the program exits or suspends, the function
  \verb{SLsmg_reset_smg}
  should be called to shutdown the display system.  This function has the
  prototype
#v+
      void SLsmg_reset_smg (void);
#v-
  This will deallocate any memory allocated for the virtual screen and
  reset the terminal's display.

  Basically, a program that uses the \verb{SLsmg} screen management functions
  and \slang's keyboard interface will look something like:
#v+
      #include <slang.h>
      int main ()
      {
         SLtt_get_terminfo ();
         SLang_init_tty (-1, 0, 0);
         SLsmg_init_smg ();

         /* do stuff .... */

         SLsmg_reset_smg ();
         SLang_reset_tty ();
         return 0;
      }
#v-
  If this program is compiled and run, all it will do is clear the screen
  and position the cursor at the bottom of the display.  In the following
  sections, other \verb{SLsmg} functions will be introduced which may be used
  to make this simple program do much more.

\sect{Handling Screen Resize Events}
  The function \verb{SLsmg_reinit_smg} is designed to be used in
  conjunction with resize events.

  Under Unix-like operating systems, when the size of the display
  changes, the application will be sent a \verb{SIGWINCH} signal.  To
  properly handle this signal, the \verb{SLsmg} routines must be
  reinitialized to use the new display size.  This may be accomplished
  by calling \verb{SLtt_get_screen_size} to get the new size, followed by
  \verb{SLsmg_reinit_smg} to reinitialize the \verb{SLsmg} interface
  to use the new size.  Keep in mind that these routines should
  not be called from within the signal handler.  The following code
  illustrates the main ideas involved in handling such events:
#v+
     static volatile int Screen_Size_Changed;
     static sigwinch_handler (int sig)
     {
        Screen_Size_Changed = 1;
        SLsignal (SIGWINCH, sigwinch_handler);
     }

     int main (int argc, char **argv)
     {
        SLsignal (SIGWINCH, sigwinch_handler);
        SLsmg_init_smg ();
          .
          .
        /* Now enter main loop */
        while (not_done)
          {
             if (Screen_Size_Changed)
               {
                  SLtt_get_screen_size ();
                  SLsmg_reinit_smg ();
                  redraw_display ();
               }
             .
             .
          }
       return 0;
     }
#v-

\sect{SLsmg Functions} #%{{{

  In the previous sections, functions for initializing and shutting down the
  \verb{SLsmg} routines were discussed.  In this section, the rest of the
  \verb{SLsmg} functions are presented.  These functions act only on the
  \em{virtual} display.  The \em{physical} display is updated when the
  \verb{SLsmg_refresh} function is called and \em{not until that time}.
  This function has the simple prototype:
#v+
     void SLsmg_refresh (void);
#v-

\sect1{Positioning the cursor}

  The \verb{SLsmg_gotorc} function is used to position the cursor at a given
  row and column.  The prototype for this function is:
#v+
      void SLsmg_gotorc (int row, int col);
#v-
  The origin of the screen is at the top left corner and is given the
  coordinate (0, 0), i.e., the top row of the screen corresponds to
  \verb{row = 0} and the first column corresponds to \verb{col = 0}.  The last
  row of the screen is given by \verb{row = SLtt_Screen_Rows - 1}.

  It is possible to change the origin of the coordinate system by using the
  function \verb{SLsmg_set_screen_start} with prototype:
#v+
     void SLsmg_set_screen_start (int *r, int *c);
#v-
  This function takes pointers to the new values of the first row and first
  column.  It returns the previous values by modifying the values of the
  integers at the addresses specified by the parameter list.  A
  \verb{NULL}
  pointer may be passed to indicate that the origin is to be set to its
  initial value of 0.  For example,
#v+
      int r = 10;
      SLsmg_set_screen_start (&r, NULL);
#v-
  sets the origin to (10, 0) and after the function returns, the variable
  \verb{r} will have the value of the previous row origin.

\sect1{Writing to the Display}

  \verb{SLsmg} has several routines for outputting text to the virtual
  display.  The following points should be understood:
\begin{itemize}
\item The text is output at the position of the cursor of the virtual
      display and the cursor is advanced to the position that corresponds to
      the end of the text.

\item Text does \em{not} wrap at the boundary of the
      display--- it is trucated.  This behavior seems to be more useful in
      practice since most programs that would use screen management tend to
      be line oriented.

\item Control characters are displayed in a two character sequence
      representation with \verb{^} as the first character.  That is,
      \key{Ctrl-X} is output as \verb{^X}.

\item The behavior of the newline character depends upon the value of
      the \verb{SLsmg_Newline_Behavior} variable.  It may be set to
      any one of the following values:

      \p\verb{SLSMG_NEWLINE_IGNORED} : If a newline character is
      encountered when writing a string to the virtual display, the
      characters in the string following the newline character will not
      be written.  In other words, the newline character will act like
      a string termination character.  This is the default setting for
      the \verb{SLsmg_Newline_Behavior}.

      \p\verb{SLSMG_NEWLINE_MOVES} : If a newline character is when
      writing to the virtual display, the following characters will be
      written to the beginning of the next row.

      \p\verb{SLSMG_NEWLINE_SCROLLS} : When set to this value and a
      newline character is output at the bottom of the virtual
      display, the display will scroll up.  Otherwise the behavior
      will be the same as that of \verb{SLSMG_NEWLINE_MOVES}.

      \p\verb{SLSMG_NEWLINE_PRINTABLE} : When set to this value, a
      newline character will be printed as the two characters sequence
      \verb{^J}.

\end{itemize}

  Although the some of the above items might appear to be too restrictive, in
  practice this is not seem to be the case.  In fact, the design of the
  output routines was influenced by their actual use and modified to
  simplify the code of the application utilizing them.
\begin{descrip}
\proto{void SLsmg_write_char (char ch);}
     Write a single character to the virtual display.

\proto{void SLsmg_write_nchars (char *str, int len);}
     Write \verb{len} characters pointed to by \verb{str} to the
     virtual display.

\proto{void SLsmg_write_string (char *str);}
  Write the null terminated string given by pointer \verb{str} to the
  virtual display.  This function is a wrapper around
  \verb{SLsmg_write_nchars}.

\proto{void SLsmg_write_nstring (char *str, int n);}
  The purpose of this function is to write a null terminated string to
  a field that is at most \verb{n} cells wide.  Each double-wide
  character in the string will use two cells.  If the string is not
  big enough to fill the \verb{n} cells, the rest of the cells will be
  filled with space characters. This function is a wrapper around
  \verb{SLsmg_write_wrapped_string}.

\proto{void SLsmg_write_wrapped_string(SLuchar_Type *str, int r, int c, unsigned int dr, unsigned int dc, int fill)}
  The purpose of this function is two write a string \verb{str} to a
  box defined by rows and columns satisfying \verb{r<=row<r+dc} and
  \verb{c<=column<c+dc}. The string will be wrapped at the column
  boundaries of the box and truncated if its size exceeds to size of
  the box.  If the total size of the string is less than that of the
  box, and the \verb{fill} parameter is non-zero, then the rest of the
  cells in the box will be filled with space characters. Currently the
  wrapping algorithm is very simple and knows nothing about word
  boundaries.

\proto{void SLsmg_printf (char *fmt, ...);}
  This function is similar to \verb{printf} except that it writes to
  the \verb{SLsmg} virtual display.

  \proto{void SLsmg_vprintf (char *, va_list);}
  Like \verb{SLsmg_printf} but uses a variable argument list.
\end{descrip}
\sect1{Erasing the Display}

  The following functions may be used to fill portions of the display with
  blank characters.  The attributes of blank character are the current
  attributes.  (See below for a discussion of character attributes)
\begin{descrip}
  \proto{void SLsmg_erase_eol (void);}
  Erase line from current position to the end of the line.

  \proto{void SLsmg_erase_eos (void);}
  Erase from the current position to the end of the screen.

  \proto{void SLsmg_cls (void);}
  Clear the entire virtual display.
\end{descrip}
\sect1{Setting Character Attributes}

  Character attributes define the visual characteristics the character
  possesses when it is displayed.  Visual characteristics include the
  foreground and background colors as well as other attributes such as
  blinking, bold, and so on.  Since \verb{SLsmg} takes a different approach
  to this problem than other screen management libraries an explanation of
  this approach is given here.  This approach has been motivated by
  experience with programs that require some sort of screen management.

  Most programs that use \verb{SLsmg} are composed of specific textual
  objects or objects made up of line drawing characters. For example,
  consider an application with a menu bar with drop down menus.  The menus
  might be enclosed by some sort of frame or perhaps a shadow.  The basic
  idea is to associate an integer to each of the objects (e.g., menu bar,
  shadow, current menu item, etc.) and create a mapping from the integer to
  the set of attributes.  In the terminology of \verb{SLsmg}, the integer is
  simply called an \em{object}.

  For example, the menu bar might be associated with the object \verb{1}, the
  drop down menu could be object \verb{2}, the shadow could be object
  \verb{3},
  and so on.

  The range of values for the object integer is restricted from 0 up to
  and including 255 on all systems except MSDOS where the maximum allowed
  integer is 15\footnote{This difference is due to memory constraints
  imposed by MSDOS.  This restriction might be removed in a future version of
  the library.}. The object numbered zero should not be regarding as an
  object at all.  Rather it should be regarded as all \em{other} objects
  that have not explicitly been given an object number.  \verb{SLsmg}, or
  more precisely \verb{SLtt}, refers to the attributes of this special object
  as the \em{default} or \em{normal} attributes.

  The \verb{SLsmg} routines know nothing about the mapping of the color to the
  attributes associated with the color.  The actual mapping takes place at a
  lower level in the \verb{SLtt} routines.  Hence, to map an object to the
  actual set of attributes requires a call to any of the following
  \verb{SLtt}
  routines:
#v+
     void SLtt_set_color (int obj, char *name, char *fg, char *bg);
     void SLtt_set_color_object (int obj, SLtt_Char_Type attr);
     void SLtt_set_mono (int obj, char *, SLtt_Char_Type attr);
#v-
  Only the first of these routines will be discussed briefly here.  The
  latter two functions allow more fine control over the object to attribute
  mapping (such as assigning a ``blink'' attribute to the object).  For a
  more full explanation on all of these routines see the section about the
  \verb{SLtt} interface.

  The \verb{SLtt_set_color} function takes four parameters.  The first
  parameter, \verb{obj}, is simply the integer of the object for which
  attributes are to be assigned.  The second parameter is currently
  unused by these routines.  The third and forth parameters, \verb{fg}
  and \verb{bg}, are the names of the foreground and background color
  to be used associated with the object.  The strings that one can use
  for the third and fourth parameters can be any one of the 16 colors:
#v+
     "black"                "gray"
     "red"                  "brightred"
     "green"                "brightgreen"
     "brown"                "yellow"
     "blue"                 "brightblue"
     "magenta"              "brightmagenta"
     "cyan"                 "brightcyan"
     "lightgray"            "white"
#v-
  The value of the foreground parameter \verb{fg} can be anyone of these
  sixteen colors.   However, on most terminals, the background color will
  can only be one of the colors listed in the first column\footnote{This is
  also true on the Linux console.  However, it need not be the case and
  hopefully the designers of Linux will someday remove this restriction.}.

  Of course not all terminals are color terminals.  If the \slang global
  variable \verb{SLtt_Use_Ansi_Colors} is non-zero, the terminal is
  assumed to be a color terminal.  The \verb{SLtt_get_terminfo} will
  try to determine whether or not the terminal supports colors and set
  this variable accordingly.  It does this by looking for the
  capability in the terminfo/termcap database.  Unfortunately many Unix
  databases lack this information and so the \verb{SLtt_get_terminfo}
  routine will check whether or not the environment variable
  \verb{COLORTERM} exists.  If it exists, the terminal will be assumed
  to support ANSI colors and \verb{SLtt_Use_Ansi_Colors} will be set to one.
  Nevertheless, the application should provide some other mechanism to set
  this variable, e.g., via a command line parameter.

  When the \verb{SLtt_Use_Ansi_Colors} variable is zero, all objects
  with numbers greater than one will be displayed in inverse
  video\footnote{This behavior can be modified by using the
  \tt{SLtt_set_mono} function call.}.

  With this background, the \verb{SLsmg} functions for setting the character
  attributes can now be defined.  These functions simply set the object
  attributes that are to be assigned to \em{subsequent} characters written
  to the virtual display.  For this reason, the new attribute is called the
  \em{current} attribute.
\begin{descrip}
  \proto{void SLsmg_set_color (int obj);}
  Set the current attribute to those of object \verb{obj}.

  \proto{void SLsmg_normal_video (void);}
  This function is equivalent to \verb{SLsmg_set_color (0)}.

  \proto{void SLsmg_reverse_video (void);}
  This function is equivalent to \verb{SLsmg_set_color (1)}.  On monochrome
  terminals, it is equivalent to setting the subsequent character attributes
  to inverse video.
\end{descrip}

  Unfortunately there does not seem to be a standard way for the
  application or, in particular, the library to determine which color
  will be used by the terminal for the default background.  Such
  information would be useful in initializing the foreground and
  background colors associated with the default color object (0).  For
  this reason, it is up to the application to provide some means for
  the user to indicate what these colors are for the particular
  terminal setup. To facilitate this, the \verb{SLtt_get_terminfo}
  function checks for the existence of the \verb{COLORFGBG}
  environment variable.  If this variable exists, its value will be
  used to initialize the colors associated with the default color
  object.  Specifically, the value is assumed to consist of a
  foreground color name and a background color name separated by a
  semicolon.  For example, if the value of \verb{COLORFGBG} is
  \verb{lightgray;blue}, the default color object will be initialized
  to represent a \verb{lightgray} foreground upon a \verb{blue}
  background.

\sect1{Lines and Alternate Character Sets}
  The \slang screen management library also includes routines for turning
  on and turning off alternate character sets.  This is especially useful
  for drawing horizontal and vertical lines.
\begin{descrip}
  \proto{void SLsmg_set_char_set (int flag);}
  If \verb{flag} is non-zero, subsequent write functions will use characters
  from the alternate character set.  If \verb{flag} is zero, the default, or,
  ordinary character set will be used.

  \proto{void SLsmg_draw_hline (int len);}
  Draw a horizontal line from the current position to the column that is
  \verb{len} characters to the right.

  \proto{void SLsmg_draw_vline (int len);}
  Draw a horizontal line from the current position to the row that is
  \verb{len} rows below.

  \proto{void SLsmg_draw_box (int r, int c, int dr, int dc);}
  Draw a box whose upper right corner is at row \verb{r} and column
  \verb{c}.
  The box spans \verb{dr} rows and \verb{dc} columns.  The current position
  will be left at row \verb{r} and column \verb{c}.
\end{descrip}

\sect1{Miscellaneous Functions}

\begin{descrip}
  \proto{void SLsmg_touch_lines (int r, int n);}
  Mark screen rows numbered \verb{r}, \verb{r + 1}, \ldots \verb{r +
  (n - 1)} as
  modified.  When \verb{SLsmg_refresh} is called, these rows will be
  completely redrawn.

  \proto{int SLsmg_char_at(SLsmg_Char_Type *ch);}
  Returns the character and its attributes at the current position.
  The SLsmg_Char_Type object is a structure representing the character
  cell:
#v+
    #define SLSMG_MAX_CHARS_PER_CELL 5
    typedef struct
     {
        unsigned int nchars;
        SLwchar_Type wchars[SLSMG_MAX_CHARS_PER_CELL];
        SLsmg_Color_Type color;
     }
     SLsmg_Char_Type;
#v-
  Normally the value of the \verb{nchars} field will be 1 to indicate
  that the character contains precisely one character whose value is
  given by the zeroth element of the wchars array of the structure.
  The value of \verb{nchars} will be greater than one if the character
  cell also contains so-called Unicode combining characters.  The
  combining characters are given by the elements 1 through
  \verb{nchars-1} of the \verb{wchars} array.  If \verb{nchars} is 0,
  then the character cell represents the second half of a double-wide
  character.

  The \verb{color} field repesents both the color of the character
  cell and the alternate character set setting of the cell.  This
  value may be bitwise-anded with \verb{SLSMG_COLOR_MASK} to obtain
  the cell's color, and bitwise-anded with \verb{SLSMG_ACS_MASK} to
  determine whether or not the alternate-character set setting is in
  effect for the cell (zero or non-zero).
\end{descrip}

#%}}}

\sect{Variables} #%{{{

  The following \slang global variables are used by the \verb{SLsmg}
  interface.  Some of these have been previously discussed.

  \verb{int SLtt_Screen_Rows;}
  \verb{int SLtt_Screen_Cols;}
  The number of rows and columns of the \em{physical} display.  If either of
  these numbers changes, the functions \verb{SLsmg_reset_smg} and
  \verb{SLsmg_init_smg} should be called again so that the \verb{SLsmg}
  routines can re-adjust to the new size.

  \verb{int SLsmg_Tab_Width;}
  Set this variable to the tab width that will be used when expanding tab
  characters.  The default is 8.

  \verb{int SLsmg_Display_Eight_Bit}
  This variable determines how characters with the high bit set are to be
  output.  Specifically, a character with the high bit set with a value
  greater than or equal to this value is output as is; otherwise, it will be
  output in a 7-bit representation.  The default value for this variable is
  \verb{128} for MSDOS and \verb{160} for other systems (ISO-Latin).
  In UTF-8 mode, the value of this variable is 0.

  \verb{int SLtt_Use_Ansi_Colors;}
  If this value is non-zero, the terminal is assumed to support ANSI colors
  otherwise it is assumed to be monochrome.  The default is 0.

  \verb{int SLtt_Term_Cannot_Scroll;}
  If this value is zero, the \verb{SLsmg} will attempt to scroll the physical
  display to optimize the update.  If it is non-zero, the screen management
  routines will not perform this optimization.  For some applications, this
  variable should be set to zero.  The default value is set by the
  \verb{SLtt_get_terminfo} function.

#%}}}

\sect{Hints for using SLsmg}

  This section discusses some general design issues that one must face when
  writing an application that requires some sort of screen management.

#%}}}

\chapter{Signal Functions} #%{{{

 Almost all non-trivial programs must worry about signals.  This is
 especially true for programs that use the \slang terminal
 input/output and screen management routines.  Unfortunately, there is
 no fixed way to handle signals; otherwise, the Unix kernel would take
 care of all issues regarding signals and the application programmer
 would never have to worry about them.  For this reason, none of the
 routines in the \slang library catch signals; however, some of the
 routines block the delivery of signals during crucial moments.  It is
 up to the application programmer to install handlers for the various
 signals of interest.

 If the application makes use of the interpreter, then a signal
 handler for \var{SIGINT} should be installed to allow the user to
 break out of the interpreter via, e.g., \key{Ctrl-C}.  In order for
 this to work, the signal handler should call \cfun{SLang_set_error}
 to generate a \var{SL_UserBreak_Error} exception, i.e.,
#v+
    void sigint_handler (int sig)
    {
       if (SLang_Ignore_User_Abort == 0)
         SLang_set_error (SL_UserBreak_Error);
    }
#v-

 Applications that use the \grp{tty} \var{getkey} routines or the screen
 management routines must worry about signals such as:
#v+
     SIGINT                interrupt
     SIGTSTP               stop
     SIGQUIT               quit
     SIGTTOU               background write
     SIGTTIN               background read
     SIGWINCH              window resize
#v-
 It is important that handlers be established for these signals while
 the either the \var{SLsmg} routines or the \var{getkey} routines are
 initialized.  The \cfun{SLang_init_tty}, \cfun{SLang_reset_tty},
 \cfun{SLsmg_init_smg}, and \cfun{SLsmg_reset_smg} functions block these
 signals from occurring while they are being called.

 Since a signal can be delivered at any time, it is important for the
 signal handler to call only functions that can be called from a
 signal handler.  This usually means that such function must be
 re-entrant. In particular, the \var{SLsmg} routines are \em{not}
 re-entrant; hence, they should not be called when a signal is being
 processed unless the application can ensure that the signal was not
 delivered while an \var{SLsmg} function was called.  This statement
 applies to many other functions such as \var{malloc}, or, more
 generally, any function that calls \var{malloc}.  The upshot is that
 the signal handler should not attempt to do too much except set a
 global variable for the application to look at while not in a signal
 handler.

 The \slang library provides two functions for blocking and unblocking the
 above signals:
#v+
    int SLsig_block_signals (void);
    int SLsig_unblock_signals (void);
#v-
 It should be noted that for every call to \cfun{SLsig_block_signals}, a
 corresponding call should be made to \cfun{SLsig_unblock_signals}, e.g.,
#v+
    void update_screen ()
    {
       SLsig_block_signals ();

       /* Call SLsmg functions */
           .
           .
       SLsig_unblock_signals ();
    }
#v-
 See \file{demo/pager.c} for examples.

#%}}}

\chapter{Searching Functions} #%{{{

 The S-Lang library incorporates two types of searches: Regular expression
 pattern matching and ordinary searching.

\sect{Simple Searches} #%{{{

 \slang's \SLsearch interface functions a convenient interface to the
 famous Boyer-Moore fast searching algrothim.  The searches can go in
 either a forward or backward direction and and may be performed with
 or without regard to case.  Moreover, UTF-8 encoded strings are fully
 supported by the interface.

#%}}}

\sect{Regular Expressions} #%{{{

             !!! No documentation available yet !!!

#%}}}

#%}}}

\appendix

#i apinews.tm

#i copyright.tm

\end{\documentstyle}
