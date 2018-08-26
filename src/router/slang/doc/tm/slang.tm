#% -*- mode: tm; mode: fold -*-

#% text-macro definitions #%{{{
#i linuxdoc.tm
#i local.tm

#d labeled_sect#1 \sect{$1 \label{$1}}
#d labeled_sect1#1 \sect1{$1 \label{$1}}
#d labeled_chapter#1 \chapter{$1 \label{$1}}
#d sectref#1 the section on \ref{$1}
#d chapterref#1 the chapter on \ref{$1}

#d documentstyle book

#%}}}

\linuxdoc

\begin{\documentstyle}

#d DocTitle A Guide to the S-Lang Language
\title \DocTitle (\docversion)
\author John E. Davis <www.jedsoft.org>
\date \__today__

\toc

#i preface.tm

#%+
   "Slang is a language that rolls up its sleeves, spits on its hands and
    goes to work."
    -- Carl Sandburg (1878-1967), U.S. poet. New York Times (Feb. 13, 1959).
#%-

\labeled_chapter{Introduction} #%{{{

 \slang is a powerful interpreted language that may be embedded into
 an application to make the application extensible.  This enables the
 application to be used in ways not envisioned by the programmer, thus
 providing the application with much more flexibility and power.
 Examples of applications that take advantage of the interpreter in
 this way include the \jed editor and the \slrn newsreader.

\sect{slsh -- The \slang shell}

 The \slang distribution contains a standalone application called
 \slsh that may be used for writing \slang scripts and full-blown
 \slang based applications.  For example, the author has used \slsh to
 create a mediacenter for his home entertainment system that
 integrates internet radio and tv, podcasts, digital pictures and
 video, CDs, and so forth.  The use of \slsh in such non-interactive
 modes is discussed in \chapterref{slsh}.

 \slsh also may be used interactively and has full access to all
 components of the \slang interpreter.  With features such as
 customizable command-line editing, history recall and completion,
 \slsh is a convenient environment for learning and using the
 language.  In fact, as you are reading this manual, it is recommended
 that you use \slsh in its interactive mode as an aid to understanding
 the language.

 While a standard \slang installation includes \slsh,
 some some binary distributions package \slsh separately from the
 \slang library, and as such must be installed separately.  For
 example, on Debian Linux it can be installed via
#v+
    apt-get install slsh
#v-

 When called without arguments, \slsh will start in interactive mode
 by issuing a (customizable) \tt{slsh>} prompt and waits for input.
 While most of the time one would enter \slang statements at the
 prompt, \slsh also accepts some other commands, most notably
 \exmp{help}:
#v+
   slsh> help
   Most commands must end in a semi-colon.
   If a command begins with '!', then the command is passed to the shell.
   Examples: !ls, !pwd, !cd foo, ...
   Special commands:
     help <help-topic>
     apropos <something>
     start_log( <optional-log-file> );
       start logging input to a file (default is slsh.log)
     stop_log();
       stop logging input
     save_input (<optional-file>);
       save all previous input to a file (default: slsh.log)
     quit;
#v-

 Although the language normally requires variables to be declared
 before use, it is not necessary to do so when using \slsh
 interactively.  For example, in this document you will see examples
 such as
#v+
    variable x = [1:10];
    variable y = sin (x^2);
#v-
 At the \slsh command line, the use of the \kw{variable} keyword in such
 statements is optional:
#v+
    slsh> x = [1:10]; y = sin(x^2);
#v-

 As the above example suggests, one use of \slsh is as a sophisticated
 calculator.  For example,
#v+
    slsh> sin (1.24) + 3*cos (1.3*PI);
    -0.817572
#v-
 This is especially true when combined with modules, e.g.,
#v+
    slsh> require ("fits");
    slsh> require ("histogram");
    slsh> tbl = fit_read_table ("evt1a.fits");
    slsh> engrid = [min(tbl.energy):max(energy):#1024];
    slsh> spectrum = hist1d (tbl.energy[where(tbl.status==0)], engrid);
#v-
 In this example, the \module{fits} module was used to read data
 from a binary file called \exmp{evt1a.fits}, and the
 \module{histogram} module was used to bin the data in the energy
 column into a histogram to create a spectrum.  The expression
 involving \exmp{where} filters the data by accepting only those
 energy values whose status is set to 0.  The \module{fits} and
 \module{histogram} modules are not distributed with \slang but may be
 obtained separately-- see \url{http://www.jedsoft.org/slang/modules/}
 for links to them.  For more information about modules, see the
 \ref{Modules} chapter in this document.

 For more information about using \slsh, see \chapterref{slsh}.

\sect{Language Features}

   The language features both global and local variables, branching
   and looping constructs, user-defined functions, structures,
   datatypes, and arrays.  In addition, there is limited support for
   pointer types.  The concise array syntax rivals that of commercial
   array-based numerical computing environments.

\sect{Data Types and Operators} #%{{{

   The language provides built-in support for string, integer (signed
   and unsigned long and short), double precision floating point, and
   double precision complex numbers.  In addition, it supports user
   defined structure types, multi-dimensional array types, lists, and
   associative arrays.  To facilitate the construction of
   sophisticated data structures such as linked lists and trees, the
   language also includes a ``reference'' type.  The reference type
   provides much of the same flexibility as pointers in other
   languages.  Finally, applications embedding the interpreter may
   also provide special application specific types, such as the
   \var{Mark_Type} that the \jed editor provides.

   The language provides standard arithmetic operations such as
   addition, subtraction, multiplication, and division.  It also
   provides support for modulo arithmetic as well as operations at the
   bit level, e.g., exclusive-or.  Any binary or unary operator may be
   extended to work with any data type, including user-defined types.
   For example, the addition operator (\var{+}) has been extended to
   work between string types to permit string concatenation.

   The binary and unary operators work transparently with array types.
   For example, if \var{a} and \var{b} are arrays, then \exmp{a + b}
   produces an array whose elements are the result of element by
   element addition of \var{a} and \var{b}.  This permits one to do
   vector operations without explicitly looping over the array
   indices.

#%}}}

\sect{Statements and Functions} #%{{{

   The \slang language supports several types of looping constructs and
   conditional statements.  The looping constructs include \kw{while},
   \kw{do...while}, \kw{for}, \kw{forever}, \kw{loop}, \kw{foreach},
   and \kw{_for}. The conditional statements include \kw{if},
   \kw{if-then-else}, and \kw{ifnot}.

   User defined functions may be defined to return zero, one, or more
   values.  Functions that return zero values are similar to
   ``procedures'' in languages such as PASCAL.  The local variables of a
   function are always created on a stack allowing one to create
   recursive functions.  Parameters to a function are always passed by
   value and never by reference. However, the language supports a
   \em{reference} data type that allows one to simulate pass by
   reference.

   Unlike many interpreted languages, \slang allows functions to be
   dynamically loaded (function autoloading).  It also provides
   constructs specifically designed for error handling and recovery as
   well as debugging aids (e.g., function tracebacks).

   Functions and variables may be declared as private belonging to a
   namespace associated with the compilation unit that defines the
   function or variable.  The ideas behind the namespace implementation
   stem from the C language and should be quite familiar to any one
   familiar with C.

#%}}}

\sect{Error Handling} #%{{{

   The \slang language has a try/throw/catch/finally exception model
   whose semantics are similar to that of other languages.  Users may
   also extend the exception class hierarchy with user-defined
   exceptions.  The \exmp{ERROR_BLOCK} based exception model of \slang
   1.x is still supported but deprecated.

#%}}}

\sect{Run-Time Library} #%{{{

   Functions that compose the \slang run-time library are called
   \em{intrinsics}.  Examples of \slang intrinsic functions available
   to every \slang application include string manipulation functions
   such as \var{strcat}, \var{strchop}, and \var{strcmp}.  The \slang
   library also provides mathematical functions such as \var{sin},
   \var{cos}, and \var{tan}; however, not all applications enable the
   use of these intrinsics.  For example, to conserve memory, the 16
   bit version of the \jed editor does not provide support for any
   mathematics other than simple integer arithmetic, whereas other
   versions of the editor do support these functions.

   Most applications embedding the languages will also provide a set of
   application specific intrinsic functions.  For example, the \jed
   editor adds over 100 application specific intrinsic functions to
   the language.  Consult your application specific documentation to
   see what additional intrinsics are supported.

   Operating systems that support dynamic linking allow a slang
   interpreter to dynamically link additional libraries of intrinsic
   functions and variables into the interpreter.  Such loadable
   objects are called \bf{modules}.  A separate chapter of this manual
   is devoted to this important feature.

#%}}}

\sect{Input/Output}

   The language supports C-like stdio input/output functions such as
   \var{fopen}, \var{fgets}, \var{fputs}, and \var{fclose}.  In
   addition it provides two functions, \var{message} and \var{error},
   for writing to the standard output device and standard error.
   Specific applications may provide other I/O mechanisms, e.g.,
   the \jed editor supports I/O to files via the editor's
   buffers.

\sect{Obtaining more information about \slang} #%{{{

  Comprehensive information about the library may be obtained via the
  World Wide Web from \url{http://www.jedsoft.org/slang/}.  In
  particular see \url{http://www.jedsoft.org/slang/download.html} for
  downloading the latest version of the library.

  Users with generic questions about the interpreter are encouraged to
  post questions to the Usenet newsgroup \var{alt.lang.s-lang}.  More
  specific questions relating to the use of \slang within some
  application may be better answered in an application-specific forum.
  For example, users with questions about using \slang as embedded in
  the \jed editor are more likely to be answered in the
  \var{comp.editors} newsgroup or on the \jed mailing list.  Similarly
  users with questions concerning \slrn will find
  \var{news.software.readers} to be a valuable source of information.

  Developers who have embedded the interpreter are encouraged to join
  the \slang mailing list.  To subscribe to the list or just browse
  the archives, visit
  \url{http://www.jedsoft.org/slang/mailinglists.html}.

#%}}}

#%}}}

\chapter{Overview of the Language} #%{{{

 This purpose of this section is to give the reader a feel for the
 \slang language, its syntax, and its capabilities.  The information
 and examples presented in this section should be sufficient to
 provide the reader with the necessary background to understand the
 rest of the document.

\sect{Variables and Functions} #%{{{

   \slang is different from many other interpreted languages in the
   sense that all variables and functions must be declared before they
   can be used.

   Variables are declared using the \kw{variable} keyword, e.g.,
#v+
     variable x, y, z;
#v-
   declares three variables, \exmp{x}, \exmp{y}, and \exmp{z}.  Note the
   semicolon at the end of the statement.  \em{All \slang statements must
   end in a semicolon.}

   Unlike compiled languages such as C, it is not necessary to specify
   the data type of a \slang variable.  The data type of a \slang
   variable is determined upon assignment.  For example, after
   execution of the statements
#v+
     x = 3;
     y = sin (5.6);
     z = "I think, therefore I am.";
#v-
   \exmp{x} will be an integer, \exmp{y} will be a
   double, and \exmp{z} will be a string.  In fact, it is even possible
   to re-assign \exmp{x} to a string:
#v+
     x = "x was an integer, but now is a string";
#v-
   Finally, one can combine variable declarations and assignments in
   the same statement:
#v+
     variable x = 3, y = sin(5.6), z = "I think, therefore I am.";
#v-

   Most functions are declared using the \kw{define} keyword.  A
   simple example is
#v+
      define compute_average (x, y)
      {
         variable s = x + y;
         return s / 2.0;
      }
#v-
   which defines a function that simply computes the average of two
   numbers and returns the result.  This example shows that a function
   consists of three parts: the function name, a parameter list, and
   the function body.

   The parameter list consists of a comma separated list of variable
   names.  It is not necessary to declare variables within a parameter
   list; they are implicitly declared.  However, all other \em{local}
   variables used in the function must be declared.  If the function
   takes no parameters, then the parameter list must still be present,
   but empty:
#v+
      define go_left_5 ()
      {
         go_left (5);
      }
#v-
   The last example is a function that takes no arguments and returns
   no value.  Some languages such as PASCAL distinguish such objects
   from functions that return values by calling these objects
   \em{procedures}.  However, \slang, like C, does not make such a
   distinction.

   The language permits \em{recursive} functions, i.e., functions that
   call themselves.  The way to do this in \slang is to first declare
   the function using the form:
\begin{tscreen}
     define \em{function-name} ();
\end{tscreen}
   It is not necessary to declare a list of parameters when declaring a
   function in this way.

   Perhaps the most famous example of a recursive function is the factorial
   function.  Here is how to implement it using \slang:
#v+
     define factorial ();   % declare it for recursion

     define factorial (n)
     {
        if (n < 2) return 1;
        return n * factorial (n - 1);
     }
#v-
   This example also shows how to mix comments with code.  \slang uses
   the `\exmp{%}' character to start a comment and all characters from
   the comment character to the end of the line are ignored.

#%}}}

\sect{Qualifiers}
  \slang 2.1 introduced support for function qualifiers as a mechanism
  for passing additional information to a function.  For example,
  consider a plotting application with a function
#v+
      define plot (x, y)
      {
         variable linestyle = qualifier ("linestyle", "solid");
         variable color = qualifier ("color", "black");

         sys_set_color (color);
         sys_set_linestyle (linestyle);
         sys_plot (x,y);
      }
#v-
  Here the functions \exmp{sys_set_linestyle}, \exmp{sys_set_color},
  and \exmp{sys_plot} are hypothetical low-level functions that
  perform the actual work.  This function may be called simply as
#v+
     x = [0:10:0.1];
     plot (x, sin(x));
#v-
  to produce a solid black line connecting the points.  Through the
  use of qualifiers, the color or linestyle may be specified, e.g,,
#v+
     plot (x, sin(x); linestyle="dashed");
#v-
  would produce a ``dashed'' black curve, whereas
#v+
     plot (x, sin(x); linestyle="dotted", color="blue");
#v-
  would produce a blue ``dotted'' one.

\sect{Strings} #%{{{

   Perhaps the most appealing feature of any interpreted language is
   that it frees the user from the responsibility of memory
   management. This is particularly evident when contrasting how
   \slang handles string variables with a lower level language such as
   C.  Consider a function that concatenates three strings.  An
   example in \slang is:
#v+
     define concat_3_strings (a, b, c)
     {
        return strcat (a, b, c);
     }
#v-
   This function uses the built-in
   \ifun{strcat} function for concatenating two or more strings.  In C, the
   simplest such function would look like:
#v+
     char *concat_3_strings (char *a, char *b, char *c)
     {
        unsigned int len;
        char *result;
        len = strlen (a) + strlen (b) + strlen (c);
        if (NULL == (result = (char *) malloc (len + 1)))
          exit (1);
        strcpy (result, a);
        strcat (result, b);
        strcat (result, c);
        return result;
     }
#v-
   Even this C example is misleading since none of the issues of memory
   management of the strings has been dealt with.  The \slang language
   hides all these issues from the user.

   Binary operators have been defined to work with the string data
   type.  In particular the \var{+} operator may be used to perform
   string concatenation.  That is, one can use the
   \var{+} operator as an alternative to \ifun{strcat}:
#v+
      define concat_3_strings (a, b, c)
      {
         return a + b + c;
      }
#v-
   See \sectref{Strings} for more information about string variables.

#%}}}

\sect{Referencing and Dereferencing} #%{{{
   The unary prefix operator, \var{&}, may be used to create a
   \em{reference} to an object, which is similar to a pointer
   in other languages.  References are commonly used as a mechanism to
   pass a function as an argument to another function as the following
   example illustrates:
#v+
       define compute_functional_sum (funct)
       {
          variable i, s;

          s = 0;
          for (i = 0; i < 10; i++)
           {
              s += (@funct)(i);
           }
          return s;
       }

       variable sin_sum = compute_functional_sum (&sin);
       variable cos_sum = compute_functional_sum (&cos);
#v-
   Here, the function \exmp{compute_functional_sum} applies the
   function specified by the parameter \exmp{funct} to the first
   10 integers and returns the sum.  The two statements
   following the function definition show how the \var{sin} and
   \var{cos} functions may be used.

   Note the \var{@} operator in the definition of
   \exmp{compute_functional_sum}.  It is known as the \em{dereference}
   operator and is the inverse of the reference operator.

   Another use of the reference operator is in the context of the
   \var{fgets} function.  For example,
#v+
      define read_nth_line (file, n)
      {
         variable fp, line;
         fp = fopen (file, "r");

         while (n > 0)
           {
              if (-1 == fgets (&line, fp))
                return NULL;
              n--;
           }
         return line;
      }
#v-
   uses the \var{fgets} function to read the nth line of a file.
   In particular, a reference to the local variable \exmp{line} is
   passed to \var{fgets}, and upon return \exmp{line} will be set to
   the character string read by \var{fgets}.

   Finally, references may be used as an alternative to multiple
   return values by passing information back via the parameter list.
   The example involving \var{fgets} presented above provided an
   illustration of this.  Another example is
#v+
       define set_xyz (x, y, z)
       {
          @x = 1;
          @y = 2;
          @z = 3;
       }
       variable X, Y, Z;
       set_xyz (&X, &Y, &Z);
#v-
   which, after execution, results in \exmp{X} set to 1, \exmp{Y}
   set to 2, and \exmp{Z} set to 3.  A C programmer will
   note the similarity of \exmp{set_xyz} to the following C
   implementation:
#v+
      void set_xyz (int *x, int *y, int *z)
      {
         *x = 1;
         *y = 2;
         *z = 3;
      }
#v-
#%}}}

\sect{Arrays} #%{{{
   The \slang language supports multi-dimensional arrays of all
   datatypes.  For example, one can define arrays of references to
   functions as well as arrays of arrays.  Here are a few examples of
   creating arrays:
#v+
       variable A = Int_Type [10];
       variable B = Int_Type [10, 3];
       variable C = [1, 3, 5, 7, 9];
#v-
   The first example creates an array of 10 integers and assigns
   it to the variable \exmp{A}.  The second example creates a 2-d array
   of 30 integers arranged in 10 rows and 3 columns
   and assigns the result to \exmp{B}.  In the last example, an array
   of 5 integers is assigned to the variable \exmp{C}.  However,
   in this case the elements of the array are initialized to the
   values specified.  This is known as an \em{inline-array}.

   \slang also supports something called a \em{range-array}.  An
   example of such an array is
#v+
      variable C = [1:9:2];
#v-
   This will produce an array of 5 integers running from 1 through 9
   in increments of 2.  Similarly \exmp{[0:1:#1000]} represents a 1000
   element floating point array of numbers running from 0 to 1
   (inclusive).

   Arrays are passed by reference to functions and never by value.
   This permits one to write functions that can initialize arrays.
   For example,
#v+
      define init_array (a)
      {
         variable i, imax;

         imax = length (a);
         for (i = 0; i < imax; i++)
           {
              a[i] = 7;
           }
      }

      variable A = Int_Type [10];
      init_array (A);
#v-
   creates an array of 10 integers and initializes all its
   elements to 7.

   There are more concise ways of accomplishing the result of the
   previous example.  These include:
#v+
      A = [7, 7, 7, 7, 7, 7, 7, 7, 7, 7];
      A = Int_Type [10];  A[[0:9]] = 7;
      A = Int_Type [10];  A[*] = 7;
#v-
   The second and third methods use an array of indices to index the array
   \exmp{A}.  In the second, the range of indices has been explicitly
   specified, whereas the third example uses a wildcard form.  See
   chapter \ref{Arrays} for more information about array indexing.

   Although the examples have pertained to integer arrays, the fact is
   that \slang arrays can be of any type, e.g.,
#v+
      A = Double_Type [10];
      B = Complex_Type [10];
      C = String_Type [10];
      D = Ref_Type [10];
#v-
   create 10 element arrays of double, complex, string, and
   reference types, respectively.  The last example may be used to
   create an array of functions, e.g.,
#v+
      D[0] = &sin;
      D[1] = &cos;
#v-

   The language also defines unary, binary, and mathematical
   operations on arrays.  For example, if \exmp{A} and \exmp{B} are
   integer arrays, then \exmp{A + B} is an array whose elements are
   the sum of the elements of \exmp{A} and \exmp{B}.  A trivial example
   that illustrates the power of this capability is
#v+
        variable X, Y;
        X = [0:2*PI:0.01];
        Y = 20 * sin (X);
#v-
   which is equivalent to the highly simplified C code:
#v+
        double *X, *Y;
        unsigned int i, n;

        n = (2 * PI) / 0.01 + 1;
        X = (double *) malloc (n * sizeof (double));
        Y = (double *) malloc (n * sizeof (double));
        for (i = 0; i < n; i++)
          {
            X[i] = i * 0.01;
            Y[i] = 20 * sin (X[i]);
          }
#v-

#%}}}

\sect{Lists}

   A \slang list is like an array except that it may contain a
   heterogeneous collection of data, e.g.,
#v+
     my_list = { 3, 2.9, "foo", &sin };
#v-
   is a list of four objects, each with a different type.  Like an
   array, the elements of a list may be accessed via an index, e.g.,
   \exmp{x=my_list[2]} will result in the assignment of \exmp{"foo"}
   to \exmp{x}.  The most important difference between an array and a
   list is that an array's size is fixed whereas a list may grow or
   shrink.  Algorithms that require such a data structure may execute
   many times faster when a list is used instead of an array.

\sect{Structures and User-Defined Types} #%{{{

   A \em{structure} is similar to an array in the sense that it is a
   container object.  However, the elements of an array must all be of
   the same type (or of \dtype{Any_Type}), whereas a structure is
   heterogeneous.  As an example, consider
#v+
      variable person = struct
      {
         first_name, last_name, age
      };
      variable bill = @person;
      bill.first_name = "Bill";
      bill.last_name = "Clinton";
      bill.age = 51;
#v-
   In this example a structure consisting of the three fields has been
   created and assigned to the variable \exmp{person}.  Then an
   \em{instance} of this structure has been created using the
   dereference operator and assigned to \exmp{bill}.  Finally, the
   individual fields of \exmp{bill} were initialized.  This is an
   example of an \em{anonymous} structure.

   Note: \slang versions 2.1 and higher permit assignment statements
   within the structure definition, e.g.,
#v+
      variable bill = struct
      {
         first_name = "Bill",
         last_name = "Clinton",
         age = 51
      };
#v-

   A \em{named} structure is really a new data type and may be created
   using the \kw{typedef} keyword:
#v+
      typedef struct
      {
         first_name, last_name, age
      }
      Person_Type;

      variable bill = @Person_Type;
      bill.first_name = "Bill";
      bill.last_name = "Clinton";
      bill.age = 51;
#v-
   One advantage of creating a new type is that array elements of such
   types are automatically initialized to instances of the type.  For
   example,
#v+
      People = Person_Type [100];
      People[0].first_name = "Bill";
      People[1].first_name = "Hillary";
#v-
   may be used to create an array of 100 such objects and initialize
   the \exmp{first_name} fields of the first two elements.  In
   contrast, the form using an anonymous would require a separate step
   to instantiate the array elements:
#v+
      People = Struct_Type [100];
      People[0] = @person;
      People[0].first_name = "Bill";
      People[1] = @person;
      People[1].first_name = "Hillary";
#v-

   Another big advantage of a user-defined type is that the binary and
   unary operators may be overloaded onto such types.  This is
   explained in more detail below.

   The creation and initialization of a structure may be facilitated
   by a function such as
#v+
      define create_person (first, last, age)
      {
          variable person = @Person_Type;
          person.first_name = first;
          person.last_name = last;
          person.age = age;
          return person;
      }
      variable Bill = create_person ("Bill", "Clinton", 51);
#v-

   Other common uses of structures is the creation of linked lists,
   binary trees, etc.  For more information about these and other
   features of structures, see \sectref{Linked Lists}.

#%}}}

\sect{Namespaces}

  The language supports namespaces that may be used to control the
  scope and visibility of variables and functions.  In addition to the
  global or public namespace, each \slang source file or compilation
  unit has a private or anonymous namespace associated with it.  The
  private namespace may be used to define symbols that are local to
  the compilation unit and inaccessible from the outside.  The
  language also allows the creation of named (non-anonymous or static)
  namespaces that permit access via the namespace operator.  See
  \chapterref{Namespaces} for more information.

#%}}}

\chapter{Data Types and Literal Constants} #%{{{

   The current implementation of the \slang language permits up to 65535
   distinct data types, including predefined data types such as integer and
   floating point, as well as specialized application-specific data
   types.  It is also possible to create new data types in the
   language using the \kw{typedef} mechanism.

   Literal constants are objects such as the integer 3 or the
   string \exmp{"hello"}.  The actual data type given to a literal
   constant depends upon the syntax of the constant.  The following
   sections describe the syntax of literals of specific data types.

\sect{Predefined Data Types} #%{{{

   The current version of \slang defines integer, floating point,
   complex, and string types. It also defines special purpose data
   types such as \dtype{Null_Type}, \dtype{DataType_Type}, and
   \dtype{Ref_Type}.  These types are discussed below.

\sect1{Integers} #%{{{

   The \slang language supports both signed and unsigned characters,
   short integer, long integer, and long long integer types. On most 32
   bit systems, there is no difference between an integer and a long
   integer; however, they may differ on 16 and 64 bit systems.
   Generally speaking, on a 16 bit system, plain integers are 16 bit
   quantities with a range of -32767 to 32767.  On a 32 bit system,
   plain integers range from -2147483648 to 2147483647.

   An plain integer \em{literal} can be specified in one of several ways:
\begin{itemize}
\item As a decimal (base 10) integer consisting of the characters
      0 through 9, e.g., 127.  An integer specified
      this way cannot begin with a leading 0.  That is,
      0127 is \em{not} the same as 127.

\item Using hexadecimal (base 16) notation consisting of the characters
      0 to 9 and \exmp{A} through \exmp{F}.  The hexadecimal
      number must be preceded by the characters \exmp{0x}.  For example,
      \exmp{0x7F} specifies an integer using hexadecimal notation and has
      the same value as decimal 127.

\item In Octal notation using characters 0 through 7.  The Octal
      number must begin with a leading 0.  For example,
      0177 and 127 represent the same integer.

\item In Binary notation using characters 0 and 1 with the \exmp{0b}
      prefix.  For example, 21 may be expressed in binary using
      \exmp{0b10101}.
\end{itemize}

   Short, long, long long, and unsigned types may be specified by
   using the proper suffixes: \exmp{L} indicates that the integer is a
   long integer, \exmp{LL} indicates a long long integer, \exmp{h}
   indicates that the integer is a short integer, and \exmp{U}
   indicates that it is unsigned.  For example, \exmp{1UL} specifies
   an unsigned long integer.

   Finally, a character literal may be specified using a notation
   containing a character enclosed in single quotes as \exmp{'a'}.
   The value of the character specified this way will lie in the
   range 0 to 256 and will be determined by the ASCII value of the
   character in quotes.  For example,
#v+
              i = '0';
#v-
   assigns to \exmp{i} the character 48 since the \exmp{'0'} character
   has an ASCII value of 48.

   A ``wide'' character (unicode) may be specified using the form
   '\\x{y...y}' where \exmp{y...y} are hexadecimal digits.  For example,
#v+
     '\x{12F}'         % Latin Small Letter I With Ogonek;
     '\x{1D7BC}'       % Mathematical Sans-Serif Bold Italic Small Sigma
#v-

    Any integer may be preceded by a minus sign to indicate that it is a
    negative integer.

#%}}}

\sect1{Floating Point Numbers} #%{{{

    Single and double precision floating point literals must contain either a
    decimal point or an exponent (or both). Here are examples of
    specifying the same double precision point number:
#v+
         12.    12.0    12e0   1.2e1   120e-1   .12e2   0.12e2
#v-
    Note that 12 is \em{not} a floating point number since it
    contains neither a decimal point nor an exponent.  In fact,
    12 is an integer.

    One may append the \exmp{f} character to the end of the number to
    indicate that the number is a single precision literal.  The
    following are all single precision values:
#v+
         12.f    12.0f    12e0f   1.2e1f   120e-1f   .12e2f   0.12e2f
#v-

#%}}}

\sect1{Complex Numbers} #%{{{

    The language implements complex numbers as a pair of double
    precision floating point numbers.  The first number in the pair
    forms the \em{real} part, while the second number forms the
    \em{imaginary} part.  That is, a complex number may be regarded as the
    sum of a real number and an imaginary number.

    Strictly speaking, the current implementation of the \slang does
    not support generic complex literals.  However, it does support
    imaginary literals permitting a more generic complex number with a
    non-zero real part to be constructed from the imaginary literal
    via addition of a real number.

    An imaginary literal is specified in the same way as a floating
    point literal except that \exmp{i} or \exmp{j} is appended.  For
    example,
#v+
         12i    12.0i   12e0j
#v-
    all represent the same imaginary number.

    A more generic complex number may be constructed from an imaginary
    literal via addition, e.g.,
#v+
        3.0 + 4.0i
#v-
    produces a complex number whose real part is \exmp{3.0} and whose
    imaginary part is \exmp{4.0}.

    The intrinsic functions \var{Real} and \var{Imag} may be used to
    retrieve the real and imaginary parts of a complex number,
    respectively.

#%}}}

\labeled_sect1{Strings} #%{{{

    A string literal must be enclosed in double quotes as in:
#v+
      "This is a string".
#v-
    As described below, the string literal may contain a suffix that
    specifies how the string is to be interpreted, e.g., a string
    literal such as
#v+
      "$HOME/.jedrc"$
#v-
    with the '$' suffix will be subject to variable name expansion.

    Although there is no imposed limit on the length of a string,
    single-line string literals must be less than 256 characters in
    length.  It is possible to construct strings longer than this by
    string concatenation, e.g.,
#v+
      "This is the first part of a long string"
       + " and this is the second part"
#v-

    \slang version 2.2 introduced support for multi-line string
    literals.  There are basic variants supported.  The first makes
    use of the backslash at the end of a line to indicate that the
    string is continued onto the next line:
#v+
      "This is a \
      multi-line string. \
      Note the presence of the \
      backslash character at the end \
      of each of the lines."
#v-
    The second form of multiline string is delimited by the backquote
    character (`) and does not require backslashes:
#v+
       `This form does not
       require backslash characters.
       In fact, here the backslash
       character \ has no special
       meaning (unless given the ``Q' suffix`
#v-
    Note that if a backquote is to appear in such a string, then it
    must be doubled, as illustrated in the above example.

    Any character except a newline (ASCII 10) or the null character
    (ASCII 0) may appear explicitly in a string literal.  However,
    these characters may embedded implicitly using the mechanism
    described below.

    The backslash character is a special character and is used to
    include other special characters (such as a newline character) in
    the string. The special characters recognized are:
#v+
       \"        --  double quote
       \'        --  single quote
       \\        --  backslash
       \a        --  bell character (ASCII 7)
       \t        --  tab character (ASCII 9)
       \n        --  newline character (ASCII 10)
       \e        --  escape character (ASCII 27)
       \xhh      --  byte expressed in HEXADECIMAL notation
       \ooo      --  byte expressed in OCTAL notation
       \dnnn     --  byte expressed in DECIMAL
       \u{h..h}  --  the Unicode character U+h..h
       \x{h..h}  --  the Unicode character U+h..h  [modal]
#v-
    In the above table, \tt{h} represents one of the HEXADECIMAL
    characters from the set \em{[0-9A-Fa-f]}.  It is important to
    understand the distinction between the \exmp{\\x\{h..h\}} and
    \exmp{\\u\{h..h\}} forms.  When using in a string, the \exmp{\\u}
    form always expands to the corresponding UTF-8 sequence regardless
    of the UTF-8 mode.  In contrast, when in non-UTF-8 mode, the
    \exmp{\\x} form expands to a byte when given two hex characters,
    or to the corresponding UTF-8 sequence when used with three or
    more hex characters.

    For example, to include the double quote character as part of the
    string, it must be preceded by a backslash character, e.g.,
#v+
       "This is a \"quote\"."
#v-
    Similarly, the next example illustrates how a newline character
    may be included:
#v+
       "This is the first line\nand this is the second."
#v-
    Alternatively, slang-2.2 or newer permits
#v+
       `This is a "quote".`
       `This is the first line
       and this is the second.`
#v-

\sect2{Suffixes}
    A string literal may be contain a suffix that specifies how the
    string is to be interpreted.  The suffix may consist of one or
    more of the following characters:
\begin{descrip}
     \tag{R}
       Backslash substitution will not be performed on the string.
       This is the default when using back-quoted strings.
     \tag{Q}
       Backslash substitution will be performed on the string.  This
       is the default when using strings using the double-quote
       character.
     \tag{B}
       If this suffix is present, the string will be interpreted as a
       binary string (BString_Type).
     \tag{$}
       Variable name substitution will be performed on the string.
\end{descrip}

   Not all combinations of the above controls characters are
   supported, nor make sense.  For example, a string with the suffix
   \exmp{QR} will cause a parse-error because \exmp{Q} and \exmp{R}
   have opposing meanings.

\sect3{The Q and R suffixes}
   These suffixes turn on and off backslash expansion.  Unless the
   \exmp{R} suffix is present, all double-quoted string literals will
   have backslash substitution performed.  By default, backslash
   expansion is turned off for backquoted strings.

   Sometimes it is desirable to turn off backslash expansion for
   double-quoted strings.  For example, pathnames on an MSDOS or
   Windows system use the backslash character as a path separator. The
   \exmp{R} prefix turns off backslash expansion, and as a result the
   following statements are equivalent:
#v+
      file = "C:\\windows\\apps\\slrn.rc";
      file = "C:\\windows\\apps\\slrn.rc"Q;
      file = "C:\windows\apps\slrn.rc"R;
      file = `C:\windows\apps\slrn.rc`;        % slang-2.2 and above
#v-
   The only exception is that a backslash character is not permitted
   as the last character of a string with the \exmp{R} suffix.  That is,
#v+
     string = "This is illegal\"R;
#v-
   is not permitted.  Without this exception, a string such as
#v+
     string = "Some characters: \"R, S, T\"";
#v-
   would not be parsed properly.

\sect3{The $ suffix}
   If the string contains the \exmp{$} suffix, then variable name
   expansion will be performed upon names prefixed by a \exmp{$}
   character occurring within the string, e.g.,
#v+
     "The value of X is $X and the value of Y is $Y"$.
#v-
   with variable name substitution to be performed on the
   names \exmp{X} and \exmp{Y}.  Such strings may be used as a
   convenient alternative to the \ifun{sprintf} function.

   Name expansion is carried out according to the following rules: If
   the string literal occurs in a function, and the name corresponds
   to a variable local to the function, then the string representation
   of the value of that variable will be substituted.  Otherwise, if
   the name corresponds to a variable that is local to the compilation
   unit (i.e., is declared as static or private), then its value's
   string representation will be used.  Otherwise, if the name
   corresponds to a variable that exists as a global (public) then its
   value's string representation will be substituted.  If the above
   searches fail and the name exists in the environment, then the
   value of the corresponding environment variable will be used.
   Otherwise, the variable will expand to the empty string.

   Consider the following example:
#v+
     private variable bar = "two";
     putenv ("MYHOME=/home/baz");
     define funct (foo)
     {
       variable bar = 1;
       message ("file: $MYHOME/foo: garage=$MYGARAGE,bar=$bar"$);
     }
#v-
   When executed, this will produce the message:
#v+
     file: /home/baz/foo: garage=,bar=1
#v-
   assuming that \exmp{MYGARAGE} is not defined anywhere.

   A name may be enclosed in braces.  For example,
#v+
      "${MYHOME}/foo: bar=${bar}"$
#v-
   This is useful in cases when the name is followed immediately by
   other characters that may be interpreted as part of the name, e.g.,
#v+
      variable HELLO="Hello ";
      message ("${HELLO}World"$);
#v-
   will produce the message "Hello World".

#%}}}

\sect1{Null_Type}

   Objects of type \dtype{Null_Type} can have only one value:
   \var{NULL}.  About the only thing that you can do with this data
   type is to assign it to variables and test for equality with
   other objects.  Nevertheless, \var{Null_Type} is an important and
   extremely useful data type.  Its main use stems from the fact that
   since it can be compared for equality with any other data type, it
   is ideal to represent the value of an object which does not yet
   have a value, or has an illegal value.

   As a trivial example of its use, consider
#v+
      define add_numbers (a, b)
      {
         if (a == NULL) a = 0;
         if (b == NULL) b = 0;
         return a + b;
      }
      variable c = add_numbers (1, 2);
      variable d = add_numbers (1, NULL);
      variable e = add_numbers (1,);
      variable f = add_numbers (,);
#v-
   It should be clear that after these statements have been executed,
   \exmp{c} will have a value of 3.  It should also be clear
   that \exmp{d} will have a value of 1 because \var{NULL} has
   been passed as the second parameter.  One feature of the language
   is that if a parameter has been omitted from a function call, the
   variable associated with that parameter will be set to \var{NULL}.
   Hence, \exmp{e} and \exmp{f} will be set to 1 and 0,
   respectively.

   The \dtype{Null_Type} data type also plays an important role in the
   context of \em{structures}.

\sect1{Ref_Type}
   Objects of \dtype{Ref_Type} are created using the unary
   \em{reference} operator \var{&}.  Such objects may be
   \em{dereferenced} using the dereference operator \var{@}.  For
   example,
#v+
      sin_ref = &sin;
      y = (@sin_ref) (1.0);
#v-
   creates a reference to the \ifun{sin} function and assigns it to
   \exmp{sin_ref}.  The second statement uses the dereference operator
   to call the function that \exmp{sin_ref} references.

   The \dtype{Ref_Type} is useful for passing functions as arguments to
   other functions, or for returning information from a function via
   its parameter list.  The dereference operator may also used to create
   an instance of a structure.  For these reasons, further discussion
   of this important type can be found in \sectref{Referencing Variables}.

\sect1{Array_Type, Assoc_Type, List_Type, and Struct_Type}

   Variables of type \dtype{Array_Type}, \dtype{Assoc_Type},
   \dtype{List_Type}, and \dtype{Struct_Type} are known as
   \em{container objects}.  They are more complicated than the
   simple data types discussed so far and each obeys a special syntax.
   For these reasons they are discussed in a separate chapters.

\sect1{DataType_Type Type} #%{{{

   \slang defines a type called \dtype{DataType_Type}.  Objects of
   this type have values that are type names.  For example, an integer
   is an object of type \dtype{Integer_Type}.  The literals of
   \dtype{DataType_Type} include:
#v+
     Char_Type            (signed character)
     UChar_Type           (unsigned character)
     Short_Type           (short integer)
     UShort_Type          (unsigned short integer)
     Integer_Type         (plain integer)
     UInteger_Type        (plain unsigned integer)
     Long_Type            (long integer)
     ULong_Type           (unsigned long integer)
     LLong_Type           (long long integer)
     ULLong_Type          (unsigned long long integer)
     Float_Type           (single precision real)
     Double_Type          (double precision real)
     Complex_Type         (complex numbers)
     String_Type          (strings, C strings)
     BString_Type         (binary strings)
     Struct_Type          (structures)
     Ref_Type             (references)
     Null_Type            (NULL)
     Array_Type           (arrays)
     Assoc_Type           (associative arrays/hashes)
     List_Type            (lists)
     DataType_Type        (data types)
#v-
   as well as the names of any other types that an application
   defines.

   The built-in function \var{typeof} returns the data type of
   its argument, i.e., a \dtype{DataType_Type}.  For instance
   \exmp{typeof(7)} returns \dtype{Integer_Type} and
   \var{typeof(Integer_Type)} returns \dtype{DataType_Type}.  One can use this
   function as in the following example:
#v+
     if (Integer_Type == typeof (x)) message ("x is an integer");
#v-
   The literals of \dtype{DataType_Type} have other uses as well.  One
   of the most common uses of these literals is to create arrays, e.g.,
#v+
     x = Complex_Type [100];
#v-
   creates an array of 100 complex numbers and assigns it to
   \exmp{x}.
#%}}}

\sect1{Boolean Type}

   Strictly speaking, \slang has no separate boolean type; rather it
   represents boolean values as \dtype{Char_Type} objects.  In
   particular, boolean FALSE is equivalent to \dtype{Char_Type} 0,
   and TRUE as any non-zero \dtype{Char_Type} value.  Since the
   exact value of TRUE is unspecified, it is unnecessary and even
   pointless to define TRUE and FALSE literals in \slang.

#%}}}

\sect{Typecasting: Converting from one Type to Another}

   Occasionally, it is necessary to convert from one data type to
   another.  For example, if you need to print an object as a string,
   it may be necessary to convert it to a \dtype{String_Type}.  The
   \var{typecast} function may be used to perform such conversions.
   For example, consider
#v+
      variable x = 10, y;
      y = typecast (x, Double_Type);
#v-
   After execution of these statements, \exmp{x} will have the integer
   value 10 and \exmp{y} will have the double precision floating
   point value \exmp{10.0}.  If the object to be converted is an
   array, the \var{typecast} function will act upon all elements of
   the array.  For example,
#v+
      x = [1:10];       % Array of integers
      y = typecast (x, Double_Type);
#v-
   will create an array of 10 double precision values and assign it to
   \exmp{y}.  One should also realize that it is not always possible
   to perform a typecast.  For example, any attempt to convert an
   \dtype{Integer_Type} to a \dtype{Null_Type} will result in a
   run-time error.  Typecasting works only when datatypes are similar.

   Often the interpreter will perform implicit type conversions as necessary
   to complete calculations.  For example, when multiplying an
   \var{Integer_Type} with a \var{Double_Type}, it will convert the
   \var{Integer_Type} to a \var{Double_Type} for the purpose of the
   calculation.  Thus, the example involving the conversion of an
   array of integers to an array of doubles could have been performed
   by multiplication by \exmp{1.0}, i.e.,
#v+
      x = [1:10];       % Array of integers
      y = 1.0 * x;
#v-

   The \ifun{string} intrinsic function should be used whenever a
   string representation is needed. Using the \ifun{typecast} function
   for this purpose will usually fail unless the object to be
   converted is similar to a string--- most are not.  Moreover, when
   typecasting an array to \dtype{String_Type}, the \ifun{typecast}
   function acts on each element of the array to produce another
   array, whereas the \ifun{string} function will produce a string.

   One use of \ifun{string} function is to print the value of an
   object.  This use is illustrated in the following simple example:
#v+
      define print_object (x)
      {
         message (string (x));
      }
#v-
   Here, the \ifun{message} function has been used because it writes a
   string to the display.  If the \ifun{string} function was not used
   and the \ifun{message} function was passed an integer, a
   type-mismatch error would have resulted.

#%}}}

\labeled_chapter{Identifiers} #%{{{

   The names given to variables, functions, and data types are called
   \em{identifiers}.  There are some restrictions upon the actual
   characters that make up an identifier.  An identifier name must
   start with an alphabetic character (\exmp{[A-Za-z]}), an underscore
   character, or a dollar sign.  The rest of the characters in the
   name can be any combination of letters, digits, dollar signs, or
   underscore characters.  However, all identifiers whose name begins
   with two underscore characters are reserved for internal use by the
   interpreter and declarations of objects with such names should be
   avoided.

   Examples of valid identifiers include:
#v+
      mary    _3    _this_is_ok
      a7e1    $44   _44$_Three
#v-
   However, the following are not legal:
#v+
      7abc   2e0    #xx
#v-
   In fact, \exmp{2e0} actually specifies the double precision number
   \exmp{2.0}.

   There is no limit to the maximum length of an identifier.  For
   practical usage it is wise to limit the length of identifiers to a
   reasonable value.

   The following identifiers are reserved by the language for use as
   keywords:
#v+
   and          andelse      break         case          catch
   continue     define       do            else          ERROR_BLOCK
   exch         EXIT_BLOCK   finally       _for          for
   foreach      forever      !if           if            ifnot
   loop         mod          not           or            orelse
   pop          private      public        return        shl
   shr          static       struct        switch        __tmp
   then         throw        try           typedef       USER_BLOCK0
   USER_BLOCK1  USER_BLOCK2  USER_BLOCK3   USER_BLOCK4   using
   variable     while        xor
#v-

#%}}}

\labeled_chapter{Variables} #%{{{

   As many of the preceding examples have shown, a variable must be
   declared before it can be used, otherwise an undefined name error
   will be generated.  A variable is declared using the \kw{variable}
   keyword, e.g,
#v+
      variable x, y, z;
#v-
   declares three variables, \exmp{x}, \exmp{y}, and \exmp{z}.  This
   is an example of a variable declaration statement, and like all
   statements, it must end in a semicolon.

   Variables declared this way are untyped and inherit a type upon
   assignment.  As such, type-checking of function arguments, etc is
   performed at run-time.  For example,
#v+
      x = "This is a string";
      x = 1.2;
      x = 3;
      x = 2i;
#v-
   results in x being set successively to a string, a float, an
   integer, and to a complex number (\exmp{0+2i}).  Any attempt to use
   a variable before it has acquired a type will result in an
   uninitialized variable error.

   It is legal to put executable code in a variable declaration list.
   That is,
#v+
      variable x = 1, y = sin (x);
#v-
   are legal variable declarations.  This also provides a convenient way
   of initializing a variable.

   Variables are classified as either \em{global} or \em{local}. A
   variable declared inside a function is said to be local and has no
   meaning outside the function.  A variable is said to be global if
   it was declared outside a function.  Global variables are further
   classified as being \var{public}, \var{static}, or \var{private},
   according to the namespace where they were defined. See
   \chapterref{Namespaces} for more information about namespaces.

   The following global variables are predefined by the language and
   live in the \var{public} namespace.  They are mainly used as
   convenience variables:
#v+
      $0 $1 $2 $3 $4 $5 $6 $7 $8 $9
#v-

   An \em{intrinsic} variable is another type of global variable.
   Such variables have a definite type which cannot be altered.
   Variables of this type may also be defined to be read-only, or
   constant variables.  An example of an intrinsic variable is
   \var{PI} which is a read-only double precision variable with a value
   of approximately \exmp{3.14159265358979323846}.

#%}}}

\chapter{Operators} #%{{{

   \slang supports a variety of operators that are grouped into three
   classes: assignment operators, binary operators, and unary operators.

   An assignment operator is used to assign a value to a variable.
   They will be discussed more fully in the context of the assignment
   statement in \sectref{Assignment Statements}.

   An unary operator acts only upon a single quantity while a binary
   operation is an operation between two quantities.  The boolean
   operator \kw{not} is an example of an unary operator.  Examples of
   binary operators include the usual arithmetic operators
   \var{+}, \var{-}, \var{*}, and \var{/}.  The operator given by
   \var{-} can be either an unary operator (negation) or a binary operator
   (subtraction); the actual operation is determined from the context
   in which it is used.

   Binary operators are used in algebraic forms, e.g., \exmp{a + b}.
   Unary operators fall into one of two classes: postfix-unary or
   prefix-unary.  For example, in the expression \exmp{-x}, the minus
   sign is a prefix-unary operator.

   All binary and unary operators may be defined for any supported
   data type.  For example, the arithmetic plus operator has been
   extended to the \dtype{String_Type} data type to permit
   concatenation between strings.  But just because it is possible to
   define the action of an operator upon a data type, it does not mean
   that all data types support all the binary and unary operators.
   For example, while \dtype{String_Type} supports the \var{+}
   operator, it does not admit the \var{*} operator.

\sect{Unary Operators} #%{{{

   The \bf{unary} operators operate only upon a single operand.  They
   include: \kw{not}, \var{~}, \var{-}, \var{@}, \var{&}, as well as the
   increment and decrement operators \var{++} and \var{--},
   respectively.

   The boolean operator \kw{not} acts only upon integers and produces
   \var{0} if its operand is non-zero, otherwise it produces 1.

   The bit-level not operator \var{~} performs a similar function,
   except that it operates on the individual bits of its integer
   operand.

   The arithmetic negation operator \var{-} is perhaps the most
   well-known unary operator.  It simply reverses the sign of its
   operand.

   The reference (\var{&}) and dereference (\var{@}) operators will be
   discussed in greater detail in \sectref{Referencing Variables}.
   Similarly, the increment (\var{++}) and decrement (\var{--})
   operators will be discussed in the context of the assignment
   operator.

#%}}}

\sect{Binary Operators} #%{{{

   The binary operators may be grouped according to several classes:
   arithmetic operators, relational operators, boolean operators, and
   bitwise operators.

\sect1{Arithmetic Operators} #%{{{

   The arithmetic operators include \var{+}, \var{-}, \var{*}, and \var{/},
   which perform addition, subtraction, multiplication, and division,
   respectively.  In addition to these, \slang supports the \var{mod}
   operator, which divides two numbers and produces the remainder, as
   as well as the power operator \var{^}.

   The data type of the result produced by the use of one of these
   operators depends upon the data types of the binary participants.
   If they are both integers, the result will be an integer.  However,
   if the operands are not of the same type, they will be converted to
   a common type before the operation is performed.  For example, if
   one is a floating point type and the other is an integer, the
   integer will be converted to a float. In general, the promotion
   from one type to another is such that no information is lost, if
   possible.  As an example, consider the expression \exmp{8/5} which
   indicates division of the integer 8 by the integer 5.
   The result will be the integer 1 and \em{not} the floating
   point value \exmp{1.6}.  However, \exmp{8/5.0} will produce
   \exmp{1.6} because \exmp{5.0} is a floating point number.

#%  TODO: Add something about precedence.  Also explain that some
#% types are always promoted to ints for arithmetic, e.g., Char+Char
#% != Char

#%}}}

\sect1{Relational Operators} #%{{{

   The relational operators are \var{>}, \var{>=}, \var{<}, \var{<=},
   \var{==}, and \var{!=}.  These perform the comparisons greater
   than, greater than or equal, less than, less than or equal, equal,
   and not equal, respectively.  For most data types, the result of
   the comparison will be a boolean value; however, for arrays the
   result will be an array of boolean values.  The section on arrays
   will explain this is greater detail.

   Note: For \slang versions 2.1 and higher, relational expressions
   such as \exmp{a<b<=c} are defined in the mathematical sense, i.e.,
#v+
      ((a < b) and (b <= c))
#v-
   Simarily, \exmp{(a < b <= c < d)} is the same as
#v+
      ((a < b) and (b <= c) and (c < d))
#v-
   and so on.  In previous versions of \slang, \exmp{(a<b<c)} meant
   \exmp{(a<b)<c}; however this interpretation was not very useful.

#%}}}

\sect1{Boolean Operators} #%{{{

   \slang supports four boolean binary operators: \exmp{or},
   \exmp{and}, \exmp{||}, and \exmp{&&}, which for most data types,
   return a boolean result.  In particular, the \exmp{or} and
   \exmp{||} operators return a non-zero value (boolean TRUE) if
   either of their operands are non-zero, otherwise they produce zero
   (boolean FALSE).  The \exmp{and} and \exmp{&&} operators produce a
   non-zero value if and only if both their operands are non-zero,
   otherwise they produce zero.

   Unlike the operators \exmp{&&} and \exmp{||}, the \exmp{and} and
   \exmp{or} operators do not perform the so-called boolean
   short-circuit evaluation.  For example, consider the expression:
#v+
      (x != 0) and (1/x > 10)
#v-
   Here, if \exmp{x} were to have a value of zero, a division by zero error
   would occur because even though \exmp{x!=0} evaluates to zero, the
   \var{and} operator is not short-circuited and the \exmp{1/x} expression
   would still be evaluated.  This problem can be avoided using the
   short-circuiting \exmp{&&} operator:
#v+
     (x != 0) && (1/x > 10)
#v-
   Another difference between the short-circuiting (\exmp{&&,||}) and
   the non-short-circuiting operators (\exmp{and,or}) is that the
   short-circuiting forms work only with integer or boolean types.  In
   contrast, if either of the operands of the \exmp{and} or \exmp{or}
   operators is an array then a corresponding array of boolean values
   will result.  This is explained in more detail in the section on
   arrays.

   Note: the short-circuiting operators \exmp{&&} and \exmp{||} were
   first introduced in \slang 2.1; they are not available in older
   versions.

#%}}}

\sect1{Bitwise Operators} #%{{{

   The bitwise binary operators are currently defined for integer operands
   and are used for bit-level operations.  Operators that fall in this
   class include \var{&}, \var{|}, \var{shl}, \var{shr}, and
   \var{xor}.  The \var{&} operator performs a boolean AND operation
   between the corresponding bits of the operands.  Similarly, the
   \var{|} operator performs the boolean OR operation on the bits.
   The bit-shifting operators \var{shl} and \var{shr} shift the bits
   of the first operand by the number given by the second operand to
   the left or right, respectively.  Finally, the \var{xor} performs
   an EXCLUSIVE-OR operation.

   These operators are commonly used to manipulate variables whose
   individual bits have distinct meanings.  In particular, \var{&} is
   usually used to test bits, \var{|} can be used to set bits, and
   \var{xor} may be used to flip a bit.

   As an example of using \var{&} to perform tests on bits, consider
   the following: The \jed text editor stores some of the information
   about a buffer in a bitmapped integer variable.  The value of this
   variable may be retrieved using the \jed intrinsic function
   \exmp{getbuf_info}, which actually returns four quantities: the
   buffer flags, the name of the buffer, directory name, and file
   name.  For the purposes of this section, only the buffer flags are
   of interest and can be retrieved via a function such as
#v+
      define get_buffer_flags ()
      {
         variable flags;
         (,,,flags) = getbuf_info ();
         return flags;
      }
#v-
   The buffer flags object is a bitmapped quantity where the 0th bit
   indicates whether or not the buffer has been modified, the first
   bit indicates whether or not autosave has been enabled for the
   buffer, and so on.  Consider for the moment the task of determining
   if the buffer has been modified.  This can be determined by looking
   at the zeroth bit: if it is 0 the buffer has not been
   modified, otherwise it has been modified.  Thus we can create the
   function,
#v+
     define is_buffer_modified ()
     {
        variable flags = get_buffer_flags ();
        return (flags & 1);
     }
#v-
   where the integer 1 has been used since it is represented as
   an object with all bits unset, except for the zeroth one, which is
   set.   (At this point, it should also be apparent that bits are
   numbered from zero, thus an 8 bit integer consists of bits
   0 to 7, where 0 is the least significant bit
   and 7 is the most significant one.) Similarly, we can create
   another function
#v+
     define is_autosave_on ()
     {
        variable flags = get_buffer_flags ();
        return (flags & 2);
     }
#v-
   to determine whether or not autosave has been turned on for the
   buffer.

   The \var{shl} operator may be used to form the integer with only
   the \em{nth} bit set.  For example, \exmp{1 shl 6} produces an
   integer with all bits set to zero except the sixth bit, which is
   set to one.  The following example exploits this fact:
#v+
     define test_nth_bit (flags, nth)
     {
        return flags & (1 shl nth);
     }
#v-

#%}}}

\sect1{The Namespace Operator}

   The operator \var{->} is used to in conjunction with a
   namespace to access an object within the namespace.  For example,
   if \exmp{A} is the name of a namespace containing the variable
   \exmp{v}, then \exmp{A->v} refers to that variable.  Namespaces are
   discussed more fully in \chapterref{Namespaces}.

\sect1{Operator Precedence}

\sect1{Binary Operators and Functions Returning Multiple Values} #%{{{

   Care must be exercised when using binary operators with an operand
   that returns multiple values.  In fact, the current implementation
   of the \slang language will produce incorrect results if both
   operands of a binary expression return multiple values.  \em{At
   most, only one of operands of a binary expression can return
   multiple values, and that operand must be the first one, not the
   second.}  For example,
#v+
    define read_line (fp)
    {
       variable line, status;

       status = fgets (&line, fp);
       if (status == -1)
         return -1;
       return (line, status);
    }
#v-
   defines a function, \exmp{read_line} that takes a single argument
   specifying a handle to an open file, and returns one or two values,
   depending upon the return value of \var{fgets}.  Now consider
#v+
        while (read_line (fp) > 0)
          {
             text = ();
             % Do something with text
             .
             .
          }
#v-
   Here the relational binary operator \var{>} forms a comparison
   between one of the return values (the one at the top of the stack)
   and 0.  In accordance with the above rule, since \exmp{read_line}
   returns multiple values, it must occur as the left binary operand.
   Putting it on the right as in
#v+
        while (0 < read_line (fp))    % Incorrect
          {
             text = ();
             % Do something with text
             .
             .
          }
#v-
   violates the rule and will result in the wrong answer.  For this
   reason, one should avoid using a function that returns muliple
   return values as a binary operand.

#%}}}

#%}}}

\sect{Mixing Integer and Floating Point Arithmetic}

   If a binary operation (\var{+}, \var{-}, \var{*} , \var{/}) is
   performed on two integers, the result is an integer.  If at least
   one of the operands is a floating point value, the other will be
   converted to a floating point value, and a floating point result
   be produced.  For example:
#v+
      11 / 2           --> 5   (integer)
      11 / 2.0         --> 5.5 (double)
      11.0 / 2         --> 5.5 (double)
      11.0 / 2.0       --> 5.5 (double)
#v-

   Sometimes to achive the desired result, it is necessary to
   explicitly convert from one data type to another.  For example,
   suppose that \exmp{a} and \exmp{b} are integers, and that one wants
   to compute \exmp{a/b} using floating point arithmetic.  In such a
   case, it is necessary to convert at least one of the operands to a
   floating point value using, e.g., the \ifun{double} function:
#v+
      x = a/double(b);
#v-

\sect{Short Circuit Boolean Evaluation}

  \bf{
       As of \slang version 2.1, use of the \exmp{andelse} and
       \exmp{orelse} have been deprecated in favor of the \exmp{&&} and
       \exmp{||} short-circuiting operators.
     }

   The boolean operators \var{or} and \var{and} \em{are not short
   circuited} as they are in some languages.  \slang uses \var{orelse}
   and \var{andelse} expressions for short circuit boolean evaluation.
   However, these are not binary operators. Expressions of the form:
\begin{tscreen}
        \em{expr-1} and \em{expr-2} and ... \em{expr-n}
\end{tscreen}
   can be replaced by the short circuited version using \var{andelse}:
\begin{tscreen}
        andelse {\em{expr-1}} {\em{expr-2}} ... {\em{expr-n}}
\end{tscreen}
   A similar syntax holds for the \var{orelse} operator.  For example, consider
   the statement:
#v+
      if ((x != 0) and (1/x > 10)) do_something ();
#v-
   Here, if \exmp{x} were to have a value of zero, a division by zero error
   would occur because even though \exmp{x!=0} evaluates to zero, the
   \var{and} operator is not short circuited and the \exmp{1/x} expression
   would be evaluated causing division by zero. For this case, the
   \var{andelse} expression could be used to avoid the problem:
#v+
      if (andelse
          {x != 0}
          {1 / x > 10})  do_something ();
#v-

#%}}}

\chapter{Statements} #%{{{

   Loosely speaking, a \em{statement} is composed of \em{expressions}
   that are grouped according to the syntax or grammar of the language
   to express a complete computation.  A semicolon is used to denote
   the end of a statement.

   A statement that occurs within a function is executed only during
   execution of the function.  However, statements that occur outside
   the context of a function are evaluated immediately.

   The language supports several different types of statements such as
   assignment statements, conditional statements, and so forth.  These
   are described in detail in the following sections.

\sect{Variable Declaration Statements}
   Variable declarations were already discussed in
   \chapterref{Variables}.  For the sake of completeness, a variable
   declaration is a statement of the form
\begin{tscreen}
     variable \em{variable-declaration-list} ;
\end{tscreen}
   where the \em{variable-declaration-list} is a comma separated list
   of one or more variable names with optional initializations, e.g.,
#v+
     variable x, y = 2, z;
#v-
\labeled_sect{Assignment Statements} #%{{{

   Perhaps the most well known form of statement is the \em{assignment
   statement}.  Statements of this type consist of a left-hand side,
   an assignment operator, and a right-hand side.  The left-hand side
   must be something to which an assignment can be performed.  Such
   an object is called an \em{lvalue}.

   The most common assignment operator is the simple assignment
   operator \var{=}.  Examples of its use include
#v+
      x = 3;
      x = some_function (10);
      x = 34 + 27/y + some_function (z);
      x = x + 3;
#v-
   In addition to the simple assignment operator, \slang
   also supports the binary assignment operators:
#v+
     +=   -=   *=    /=   &=   |=
#v-
   Internally, \slang transforms
#v+
       a += b;
#v-
   to
#v+
       a = a + b;
#v-
   Likewise \exmp{a-=b} is transformed to \exmp{a=a-b}, \exmp{a*=b} is
   transformed to \exmp{a=a*b}, and so on.

   It is extremely important to realize that, in general, \exmp{a+b}
   is not equal to \exmp{b+a}.  For example if \exmp{a} and \exmp{b}
   are strings, then \exmp{a+b} will be the string resulting from the
   concatenation of \exmp{a} and \exmp{b}, which generally is not he
   same as the concatenation of \exmp{b} with \exmp{a}.  This means
   that \exmp{a+=b} may not be the same as \exmp{a=b+a}, as the
   following example illustrates:
#v+
      a = "hello"; b = "world";
      a += b;                      % a will become "helloworld"
      c = b + a;                   % c will become "worldhelloworld"
#v-

   Since adding or subtracting 1 from a variable is quite
   common, \slang also supports the unary increment and decrement
   operators \exmp{++}, and \exmp{--}, respectively.  That is, for
   numeric data types,
#v+
       x = x + 1;
       x += 1;
       x++;
#v-
   are all equivalent.  Similarly,
#v+
       x = x - 1;
       x -= 1;
       x--;
#v-
   are also equivalent.

   Strictly speaking, \var{++} and \var{--} are unary operators.  When
   used as \var{x++}, the \var{++} operator is said to be a
   \em{postfix-unary} operator.  However, when used as \var{++x} it is
   said to be a \em{prefix-unary} operator.  The current
   implementation does not distinguish between the two forms, thus
   \exmp{x++} and \exmp{++x} are equivalent.  The reason for this
   equivalence is \em{that assignment expressions do not return a value in
   the \slang language} as they do in C.  Thus one should exercise care
   and not try to write C-like code such as
#v+
      x = 10;
      while (--x) do_something (x);     % Ok in C, but not in S-Lang
#v-
   The closest valid \slang form involves a \em{comma-expression}:
#v+
      x = 10;
      while (x--, x) do_something (x);  % Ok in S-Lang and in C
#v-

   \slang also supports a \em{multiple-assignment} statement.  It is
   discussed in detail in \sectref{Multiple Assignment Statement}.

#%}}}

\sect{Conditional and Looping Statements} #%{{{

  \slang supports a wide variety of conditional and looping
  statements.  These constructs operate on statements grouped together
  in \em{blocks}.  A block is a sequence of \slang statements enclosed
  in braces and may contain other blocks. However, a block cannot
  include function declarations.  In the following,
  \em{statement-or-block} refers to either a single \slang statement
  or to a block of statements, and \em{integer-expression} is an
  integer-valued or boolean expression. \em{next-statement} represents
  the statement following the form under discussion.

\sect1{Conditional Forms} #%{{{
\sect2{if}
   The simplest condition statement is the \kw{if} statement.  It
   follows the syntax
\begin{tscreen}
        if (\em{integer-expression}) \em{statement-or-block}
        \em{next-statement}
\end{tscreen}
   If \em{integer-expression} evaluates to a non-zero (boolean TRUE)
   result, then the statement or group of statements implied
   \em{statement-or-block} will get executed.  Otherwise, control will
   proceed to \em{next-statement}.

   An example of the use of this type of conditional statement is
#v+
       if (x != 0)
         {
            y = 1.0 / x;
            if (x > 0) z = log (x);
         }
#v-
   This example illustrates two \kw{if} statements where the second
   \kw{if} statement is part of the block of statements that belong to
   the first.

\sect2{if-else}
   Another form of \kw{if} statement is the \em{if-else} statement.
   It follows the syntax:
\begin{tscreen}
      if (\em{integer-expression}) \em{statement-or-block-1}
      else \em{statement-or-block-2}
      \em{next-statement}
\end{tscreen}
   Here, if \em{expression} evaluates to a non-zero integer,
   \em{statement-or-block-1} will get executed and control will pass
   on to \em{next-statement}. However, if \em{expression} evaluates to zero,
   \em{statement-or-block-2} will get executed before continuing on to
   \em{next-statement}.  A simple example of this form is
#v+
     if (x > 0)
       z = log (x);
     else
       throw DomainError, "x must be positive";
#v-
   Consider the more complex example:
#v+
     if (city == "Boston")
       if (street == "Beacon") found = 1;
     else if (city == "Madrid")
       if (street == "Calle Mayor") found = 1;
     else found = 0;
#v-
   This example illustrates a problem that beginners have with
   \em{if-else} statements.  Syntactically, this example is equivalent to
#v+
     if (city == "Boston")
       {
         if (street == "Beacon") found = 1;
         else if (city == "Madrid")
           {
             if (street == "Calle Mayor") found = 1;
             else found = 0;
           }
       }
#v-
   although the indentation indicates otherwise.  It is important to
   understand the grammar and not be seduced by the indentation!

\sect2{ifnot}

   One often encounters \kw{if} statements similar to
\begin{tscreen}
     if (\em{integer-expression} == 0) \em{statement-or-block}
\end{tscreen}
   or equivalently,
\begin{tscreen}
     if (not(\em{integer-expression})) \em{statement-or-block}
\end{tscreen}
   The \kw{ifnot} statement was added to the language to simplify the
   handling of such statements.  It obeys the syntax
\begin{tscreen}
     ifnot (\em{integer-expression}) \em{statement-or-block}
\end{tscreen}
   and is functionally equivalent to
\begin{tscreen}
     if (not (\em{expression})) \em{statement-or-block}
\end{tscreen}

  Note: The \kw{ifnot} keyword was added in version 2.1 and is not
  supported by earlier versions.  For compatibility with older code,
  the \kw{!if} keyword can be used, although its use is deprecated in
  favor of \kw{ifnot}.

\sect2{orelse, andelse}

  \bf{
  As of \slang version 2.1, use of the \exmp{andelse} and
  \exmp{orelse} have been deprecated in favor of the \exmp{&&} and
  \exmp{||} short-circuiting operators.
  }

  The syntax for the \kw{orelse} statement is:
\begin{tscreen}
     orelse {\em{integer-expression-1}} ... {\em{integer-expression-n}}
\end{tscreen}
  This causes each of the blocks to be executed in turn until one of
  them returns a non-zero integer value.  The result of this statement
  is the integer value returned by the last block executed.  For
  example,
#v+
     orelse { 0 } { 6 } { 2 } { 3 }
#v-
  returns 6 since the second block is the first to return a
  non-zero result.  The last two block will not get executed.

  The syntax for the \kw{andelse} statement is:
\begin{tscreen}
     andelse {\em{integer-expression-1}} ... {\em{integer-expression-n}}
\end{tscreen}
  Each of the blocks will be executed in turn until one of
  them returns a zero value.  The result of this statement is the
  integer value returned by the last block executed.  For example,
#v+
     andelse { 6 } { 2 } { 0 } { 4 }
#v-
  evaluates to 0 since the third block will be the last to execute.

\sect2{switch}
  The switch statement deviates from its C counterpart.  The syntax
  is:
#v+
          switch (x)
            { ...  :  ...}
              .
              .
            { ...  :  ...}
#v-
   The `\var{:}' operator is a special symbol that in the context of
   the switch statement, causes the top item on the stack to be
   tested, and if it is non-zero, the rest of the block
   will get executed and control will pass out of the switch statement.
   Otherwise, the execution of the block will be terminated and the process
   will be repeated for the next block.  If a block contains no
   \var{:} operator, the entire block is executed and control will
   pass onto the next statement following the \kw{switch} statement.
   Such a block is known as the \em{default} case.

   As a simple example, consider the following:
#v+
      switch (x)
        { x == 1 : message("Number is one.");}
        { x == 2 : message("Number is two.");}
        { x == 3 : message("Number is three.");}
        { x == 4 : message("Number is four.");}
        { x == 5 : message("Number is five.");}
        { message ("Number is greater than five.");}
#v-
   Suppose \exmp{x} has an integer value of 3.  The first two
   blocks will terminate at the `\var{:}' character because each of the
   comparisons with \exmp{x} will produce zero.  However, the third
   block will execute to completion.  Similarly, if \exmp{x} is
   7, only the last block will execute in full.

   A more familiar way to write the previous example is to make use of
   the \kw{case} keyword:
#v+
      switch (x)
        { case 1 : message("Number is one.");}
        { case 2 : message("Number is two.");}
        { case 3 : message("Number is three.");}
        { case 4 : message("Number is four.");}
        { case 5 : message("Number is five.");}
        { message ("Number is greater than five.");}
#v-
   The \var{case} keyword is a more useful comparison operator because
   it can perform a comparison between different data types while
   using \var{==} may result in a type-mismatch error.  For example,
#v+
      switch (x)
        { (x == 1) or (x == "one") : message("Number is one.");}
        { (x == 2) or (x == "two") : message("Number is two.");}
        { (x == 3) or (x == "three") : message("Number is three.");}
        { (x == 4) or (x == "four") : message("Number is four.");}
        { (x == 5) or (x == "five") : message("Number is five.");}
        { message ("Number is greater than five.");}
#v-
  will fail because the \var{==} operation is not defined between
  strings and integers.  The correct way to write this is to use the
  \var{case} keyword:
#v+
      switch (x)
        { case 1 or case "one" : message("Number is one.");}
        { case 2 or case "two" : message("Number is two.");}
        { case 3 or case "three" : message("Number is three.");}
        { case 4 or case "four" : message("Number is four.");}
        { case 5 or case "five" : message("Number is five.");}
        { message ("Number is greater than five.");}
#v-

#%}}}

\sect1{Looping Forms} #%{{{

  In this section, the various looping statements are discussed.  Each
  of these statements support an optional \kw{then} clause, which is
  discussed in a separate section below.

\sect2{while}
   The \kw{while} statement follows the syntax
\begin{tscreen}
      while (\em{integer-expression}) \em{statement-or-block}
      [ then \em{statement-or-block} ]
      \em{next-statement}
\end{tscreen}
   It simply causes \em{statement-or-block} to get executed as long as
   \em{integer-expression} evaluates to a non-zero result.  For
   example,
#v+
      i = 10;
      while (i)
        {
          i--;
          newline ();
        }
#v-
   will cause the \exmp{newline} function to get called 10 times.
   However,
#v+
      i = -10;
      while (i)
        {
          i--;
          newline ();
        }
#v-
   would loop forever (or until \exmp{i} wraps from the most negative
   integer value to the most positive and then decrements to zero).

   If you are a C programmer, do not let the syntax of the language
   seduce you into writing this example as you would in C:
#v+
      i = 10;
      while (i--) newline ();
#v-
   Keep in mind that expressions such as \exmp{i--} do not return a
   value in \slang as they do in C.  The same effect can be achieved
   to use a comma to separate the expressions as as in
#v+
      i = 10;
      while (i, i--) newline ();
#v-

\sect2{do...while}
   The \kw{do...while} statement follows the syntax
\begin{tscreen}
      do
         \em{statement-or-block}
      while (\em{integer-expression});
      [ then \em{statement-or-block} ]
\end{tscreen}
   The main difference between this statement and the \var{while}
   statement is that the \kw{do...while} form performs the test
   involving \em{integer-expression} after each execution
   of \em{statement-or-block} rather than before.  This guarantees that
   \em{statement-or-block} will get executed at least once.

   A simple example from the \jed editor follows:
#v+
     bob ();      % Move to beginning of buffer
     do
       {
          indent_line ();
       }
     while (down (1));
#v-
   This will cause all lines in the buffer to get indented via the
   \jed intrinsic function \exmp{indent_line}.

\sect2{for}
   Perhaps the most complex looping statement is the \kw{for}
   statement; nevertheless, it is a favorite of many C programmers.
   This statement obeys the syntax
\begin{tscreen}
    for (\em{init-expression}; \em{integer-expression}; \em{end-expression})
      \em{statement-or-block}
    [ then \em{statement-or-block} ]
    \em{next-statement}
\end{tscreen}
   In addition to \em{statement-or-block}, its specification requires
   three other expressions.  When executed, the \kw{for} statement
   evaluates \em{init-expression}, then it tests
   \em{integer-expression}.  If \em{integer-expression} evaluates to zero,
   control passes to \em{next-statement}.  Otherwise, it executes
   \em{statement-or-block} as long as \em{integer-expression}
   evaluates to a non-zero result.  After every execution of
   \em{statement-or-block}, \em{end-expression} will get evaluated.

   This statement is \em{almost} equivalent to
\begin{tscreen}
    \em{init-expression};
    while (\em{integer-expression})
      {
         \em{statement-or-block}
         \em{end-expression};
      }
\end{tscreen}
   The reason that they are not fully equivalent involves what happens
   when \em{statement-or-block} contains a \kw{continue} statement.

   Despite the apparent complexity of the \kw{for} statement, it is
   very easy to use.  As an example, consider
#v+
     s = 0;
     for (i = 1; i <= 10; i++) s += i;
#v-
   which computes the sum of the first 10 integers.

\sect2{loop}
   The \kw{loop} statement simply executes a block of code a fixed
   number of times.  It follows the syntax
\begin{tscreen}
      loop (\em{integer-expression}) \em{statement-or-block}
      [ then \em{statement-or-block} ]
      \em{next-statement}
\end{tscreen}
   If the \em{integer-expression} evaluates to a positive integer,
   \em{statement-or-block} will get executed that many times.
   Otherwise, control will pass to \em{next-statement}.

   For example,
#v+
      loop (10) newline ();
#v-
   will execute the \exmp{newline} function 10 times.

\sect2{_for}
   Like \kw{loop}, the \kw{_for} statement simply executes a block of
   code a fixed number times.  Unlike the \kw{loop} statement, the
   \kw{_for} loop is useful in situations where the loop index is
   needed.  It obeys the syntax
\begin{tscreen}
      _for \em{loop-variable} (\em{first-value}, \em{last-value}, \em{increment})
         \em{block}
      [ then \em{statement-or-block} ]
      \em{next-statement}
\end{tscreen}
   Each time through the loop, the loop-variable will take on the
   successive values dictated by the other parameters.  The first time
   through, the loop-variable will have the value of \em{first-value}.
   The second time its value will be \em{first-value} +
   \em{increment}, and so on.  The loop will terminate when the value
   of the loop index exceeds \em{last-value}.  The current
   implementation requires the control parameters \em{first-value},
   \em{last-value}, and \em{increment} to be integer-valued
   expressions.

   For example, the \kw{_for} statement may be used to compute the sum
   of the first ten integers:
#v+
     s = 0;
     _for i (1, 10, 1)
       s += i;
#v-

   The execution speed of the \kw{_for} loop is more than twice as fast as
   the more powerful \kw{for} loop making it a better choice for many
   situations.

\sect2{forever}
   The \kw{forever} statement is similar to the \kw{loop} statement
   except that it loops forever, or until a \kw{break} or a
   \kw{return} statement is executed.  It obeys the syntax
\begin{tscreen}
     forever \em{statement-or-block}
     [ then \em{statement-or-block} ]
\end{tscreen}
   A trivial example of this statement is
#v+
     n = 10;
     forever
       {
          if (n == 0) break;
          newline ();
          n--;
       }
#v-

\sect2{foreach}
   The \kw{foreach} statement is used to loop over one or more
   statements for every element of an object.  Most often the object
   will be a container object such as an array, structure, or
   associative arrays, but it need not be.

   The simple type of \kw{foreach} statement obeys the syntax
\begin{tscreen}
     foreach \em{var} (\em{object}) \em{statement-or-block}
     [ then \em{statement-or-block} ]
\end{tscreen}
   Here \em{object} can be an expression that evaluates to a value.
   Each time through the loop the variable \em{var} will take on a
   value that depends upon the data type of the object being
   processed.  For container objects, \em{var} will take on values of
   successive members of the object.

   A simple example is
#v+
     foreach fruit (["apple", "peach", "pear"])
       process_fruit (fruit);
#v-
   This example shows that if the container object is an array, then
   successive elements of the array are assigned to \exmp{fruit} prior to
   each execution cycle.  If the container object is a string, then
   successive characters of the string are assigned to the variable.

   What actually gets assigned to the variable may be controlled via the
   \kw{using} form of the \kw{foreach} statement.  This more complex
   type of \kw{foreach} statement follows the syntax
\begin{tscreen}
     foreach \em{var} ( \em{container-object} ) using ( \em{control-list} )
       \em{statement-or-block}
\end{tscreen}
   The allowed values of \em{control-list} will depend upon the type
   of container object.  For associative arrays (\var{Assoc_Type}),
   \em{control-list} specifies whether \em{keys}, \em{values}, or both
   are used.  For example,
#v+
     foreach k (a) using ("keys")
       {
           .
           .
       }
#v-
   results in the keys of the associative array \exmp{a} being
   successively assigned to \exmp{k}.  Similarly,
#v+
     foreach v (a) using ("values")
       {
           .
           .
       }
#v-
   will cause the values to be used.  The form
#v+
     foreach k,v (a) using ("keys", "values")
       {
           .
           .
       }
#v-
  may be used when both keys and values are desired.

  Similarly, for linked-lists of structures, one may walk the list via
  code like
#v+
     foreach s (linked_list) using ("next")
       {
            .
            .
       }
#v-
  This \kw{foreach} statement is equivalent
#v+
     s = linked_list;
     while (s != NULL)
       {
          .
          .
         s = s.next;
       }
#v-
  Consult the type-specific documentation for a discussion of the
  \kw{using} control words, if any, appropriate for a given type.

#%}}}

\sect1{break, return, and continue} #%{{{

   \slang also includes the non-local transfer statements
   \var{return}, \var{break}, and \var{continue}.  The \var{return}
   statement causes control to return to the calling function while
   the \var{break} and \var{continue} statements are used in the
   context of loop structures.  Consider:
#v+
       define fun ()
       {
          forever
            {
               s1;
               s2;
               ..
               if (condition_1) break;
               if (condition_2) return;
               if (condition_3) continue;
               ..
               s3;
            }
          s4;
          ..
       }
#v-
   Here, a function \exmp{fun} has been defined that contains a \kw{forever}
   loop consisting of statements \exmp{s1}, \exmp{s2},\ldots,\exmp{s3}, and
   three \kw{if} statements.  As long as the expressions \exmp{condition_1},
   \exmp{condition_2}, and \exmp{condition_3} evaluate to zero, the statements
   \exmp{s1}, \exmp{s2},\ldots,\exmp{s3} will be repeatedly executed.  However,
   if \exmp{condition_1} returns a non-zero value, the \kw{break} statement
   will get executed, and control will pass out of the \kw{forever} loop to
   the statement immediately following the loop, which in this case is
   \exmp{s4}. Similarly, if \exmp{condition_2} returns a non-zero number,
   the \kw{return} statement will cause control to pass back to the
   caller of \exmp{fun}.  Finally, the \kw{continue} statement will
   cause control to pass back to the start of the loop, skipping the
   statement \exmp{s3} altogether.

#%}}}

\sect1{The looping then clause} #%{{{

  As mentioned above, all the looping statements support an optional
  \kw{then} clause.  The statements that comprise this clause get
  executed only when the loop has run to completion and was not
  prematurely terminated via a \kw{break} statement.  As an example,
  consider the following:
#v+
    count = 0;
    max_tries = 20;
    while (count < max_tries)
      {
         if (try_something ())
           break;

         count++;
         % Failed -- try again
      }
    if (count == 20)
      throw RunTimeError, "try_something failed 20 times";
#v-
  Here, the code makes 20 attempts to perform some task (via the
  \exmp{try_something} function) and if not successful it will throw
  an exception.  Compare the above to an equivalent form that makes
  use of a \kw{then}-clause for the \kw{loop} statement:
#v+
    max_tries = 20;
    loop (max_tries)
      {
         if (try_something ())
           break;
         % Failed -- try again
      }
    then throw RunTimeError, "try_something failed 20 times";
#v-
  Here, the \kw{then} statement would get executed only if the loop
  statement has run to completion, i.e., loops 20 times in this case.
  This only happens if the \exmp{try_something} function fails each
  time through the loop.  However, if the \exmp{try_something}
  function succeeds, then the \kw{break} statement will get executed
  causing the loop to abort prematurely, which would result in the
  \kw{then} clause \em{not} getting executed.

  The use of such a construct can also simplify code such as:
#v+
   if (some_condition)
     {
        foo_statements;
        if (another_condition)
          bar_statements;
        else
          fizzle_statements;
     }
   else fizzle_statements;
#v-
  In this case the \exmp{fizzle_statements} are duplicated making the
  code ugly and less maintainable.  Ideally one would wrap the
  \exmp{fizzle_statements} in a separate function and call it twice.
  However, this is not always possible or convenient.  The duplication
  can be eliminated by using the \key{then} form of the \kw{loop}
  statement:
#v+
   loop (some_condition != 0)
     {
        foo_statements;
        if (another_condition)
          {
            bar_statements;
            break;
          }
     }
   then fizzle_statements;
#v-
  Here, the expression \exmp{some_condition != 0} is going to result
  in either 0 or 1, causing the code to execute 0 or 1 loops.  Since
  the \exmp{fizzle_statements} are contained in the \kw{then} clause,
  they will get executed only when the requested number of loops
  executes to completion.  Executing 0 loops is regarded as successful
  completion of the loop statement.  Hence, when \exmp{some_condition}
  is 0, the \exmp{fizzle_statements} will get executed.  The
  \exmp{fizzle_statements} will not get executed only when the loop is
  prematurely terminated, and that will occur when both
  \exmp{some_condition} and \exmp{another_condition} are non-zero.

#%}}}

#%}}}

#%}}}

\chapter{Functions} #%{{{

   There are essentially two classes of functions that may be called
   from the interpreter: intrinsic functions and slang functions.

   An intrinsic function is one that is implemented in C or some other
   compiled language and is callable from the interpreter.  Nearly all
   of the built-in functions are of this variety.  At the moment the
   basic interpreter provides nearly 300 intrinsic functions. Examples
   include the trigonometric functions \ifun{sin} and \ifun{cos}, string
   functions such as \ifun{strcat}, etc. Dynamically loaded modules
   such as the \module{png} and \module{pcre} modules add additional
   intrinsic functions.

   The other type of function is written in \slang and is known simply
   as a ``\slang function''.  Such a function may be thought of as a
   group of statements that work together to perform a computation.
   The specification of such functions is the main subject of this
   chapter.

\sect{Declaring Functions} #%{{{

   Like variables, functions must be declared before they can be used. The
   \kw{define} keyword is used for this purpose.  For example,
#v+
      define factorial ();
#v-
   is sufficient to declare a function named \exmp{factorial}.  Unlike
   the \kw{variable} keyword used for declaring variables, the
   \kw{define} keyword does not accept a list of names.

   Usually, the above form is used only for recursive functions.  In
   most cases, the function name is almost always followed by a
   parameter list and the body of the function:
\begin{tscreen}
      define \em{function-name} (\em{parameter-list})
      {
         \em{statement-list}
      }
\end{tscreen}
   The \em{function-name} is an identifier and must conform to the
   naming scheme for identifiers discussed in \chapterref{Identifiers}. The
   \em{parameter-list} is a comma-separated list of variable names
   that represent parameters passed to the function, and may be empty
   if no parameters are to be passed.  The variables in the
   \em{parameter-list} are implicitly declared, thus, there is no need
   to declare them via a variable declaration statement.  In fact any
   attempt to do so will result in a syntax error.

   The body of the function is enclosed in braces and consists of zero
   or more statements (\em{statement-list}).  While there are no
   imposed limits upon the number statements that may occur within a
   \slang function, it is considered poor programming practice if a
   function contains many statements. This notion stems from the
   belief that a function should have a simple, well-defined purpose.

#%}}}

\sect{Parameter Passing Mechanism} #%{{{

   Parameters to a function are always passed by value and never by
   reference.  To see what this means, consider
#v+
     define add_10 (a)
     {
        a = a + 10;
     }
     variable b = 0;
     add_10 (b);
#v-
   Here a function \exmp{add_10} has been defined, which when
   executed, adds 10 to its parameter.  A variable \exmp{b} has
   also been declared and initialized to zero before being passed to
   \exmp{add_10}.  What will be the value of \exmp{b} after the call
   to \exmp{add_10}?  If \slang were a language that passed parameters
   by reference, the value of \exmp{b} would be changed to 10.
   However, \slang always passes by value, which means that \exmp{b}
   will retain its value during and after after the function call.

   \slang does provide a mechanism for simulating pass by reference
   via the reference operator.  This is described in greater detail in
   the next section.

   If a function is called with a parameter in the parameter list
   omitted, the corresponding variable in the function will be set to
   \NULL.  To make this clear, consider the function
#v+
     define add_two_numbers (a, b)
     {
        if (a == NULL) a = 0;
        if (b == NULL) b = 0;
        return a + b;
     }
#v-
   This function must be called with two parameters.  However, either
   of them may omitted by calling the function in one of the following
   ways:
#v+
     variable s = add_two_numbers (2,3);
     variable s = add_two_numbers (2,);
     variable s = add_two_numbers (,3);
     variable s = add_two_numbers (,);
#v-
   The first example calls the function using both parameters, but
   at least one of the parameters was omitted in the other
   examples.  If the parser recognizes that a parameter has been
   omitted by finding a comma or right-parenthesis where a value is
   expected, it will substitute \NULL for missing value.  This means
   that the parser will convert the latter three statements in the
   above example to:
#v+
     variable s = add_two_numbers (2, NULL);
     variable s = add_two_numbers (NULL, 3);
     variable s = add_two_numbers (NULL, NULL);
#v-
   It is important to note that this mechanism is available only for
   function calls that specify more than one parameter.  That is,
#v+
     variable s = add_10 ();
#v-
  is \em{not} equivalent to \exmp{add_10(NULL)}.  The reason for this
  is simple: the parser can only tell whether or not \var{NULL} should
  be substituted by looking at the position of the comma character in
  the parameter list, and only function calls that indicate more than
  one parameter will use a comma.  A mechanism for handling single
  parameter function calls is described later in this chapter.

#%}}}

\sect{Returning Values} #%{{{

   The usual way to return values from a function is via the
   \kw{return} statement.  This statement has the simple syntax
\begin{tscreen}
      return \em{expression-list} ;
\end{tscreen}
   where \em{expression-list} is a comma separated list of expressions.
   If a function does not return any values, the expression list
   will be empty.  A simple example of a function that can return
   multiple values (two in this case) is:
#v+
        define sum_and_diff (x, y)
        {
            variable sum, diff;

            sum = x + y;  diff = x - y;
            return sum, diff;
        }
#v-

#%}}}

\labeled_sect{Multiple Assignment Statement} #%{{{

   In the previous section an example of a function returning two
   values was given.  That function can also be written somewhat
   simpler as:
#v+
       define sum_and_diff (x, y)
       {
          return x + y, x - y;
       }
#v-
   This function may be called using
#v+
      (s, d) = sum_and_diff (12, 5);
#v-
   After the above line is executed, \exmp{s} will have a value of 17
   and the value of \exmp{d} will be 7.

   The most general form of the multiple assignment statement is
#v+
     ( var_1, var_2, ..., var_n ) = expression;
#v-
   Here \exmp{expression} is an arbitrary expression that leaves
   \exmp{n} items on the stack, and \exmp{var_k} represents an l-value
   object (permits assignment). The assignment statement removes
   those values and assigns them to the specified variables.
   Usually, \exmp{expression} is a call to a function that returns
   multiple values, but it need not be.  For example,
#v+
     (s,d) = (x+y, x-y);
#v-
   produces results that are equivalent to the call to the
   \exmp{sum_and_diff} function.  Another common use of the multiple
   assignment statement is to swap values:
#v+
     (x,y) = (y,x);
     (a[i], a[j], a[k]) = (a[j], a[k], a[i]);
#v-

   If an l-value is omitted from the list, then the corresponding
   value will be removed fro the stack.  For example,
#v+
     (s, ) = sum_and_diff (9, 4);
#v-
   assigns the sum of 9 and 4 to \exmp{s} and the
   difference (\exmp{9-4}) is removed from the stack.  Similarly,
#v+
     () = fputs ("good luck", fp);
#v-
   causes the return value of the \ifun{fputs} function to be discarded.

   It is possible to create functions that return a \em{variable
   number} of values instead of a \em{fixed number}.  Although such
   functions are discouraged, it is easy to cope with them.  Usually,
   the value at the top of the stack will indicate the actual number
   of return values.  For such functions, the multiple assignment
   statement cannot directly be used.  To see how such functions can
   be dealt with, consider the following function:
#v+
     define read_line (fp)
     {
        variable line;
        if (-1 == fgets (&line, fp))
          return -1;
        return (line, 0);
     }
#v-
   This function returns either one or two values, depending upon the
   return value of \ifun{fgets}.  Such a function may be handled using:
#v+
      status = read_line (fp);
      if (status != -1)
        {
           s = ();
           .
           .
        }
#v-
   In this example, the \em{last} value returned by \exmp{read_line} is
   assigned to \exmp{status} and then tested.  If it is non-zero, the
   second return value is assigned to \exmp{s}.  In particular note the
   empty set of parenthesis in the assignment to \exmp{s}.  This simply
   indicates that whatever is on the top of the stack when the
   statement is executed will be assigned to \exmp{s}.

#%}}}

\labeled_sect{Referencing Variables} #%{{{

   One can achieve the effect of passing by reference by using the
   reference (\var{&}) and dereference (\var{@}) operators. Consider
   again the \exmp{add_10} function presented in the previous section.
   This time it is written as:
#v+
     define add_10 (a)
     {
        @a = @a + 10;
     }
     variable b = 0;
     add_10 (&b);
#v-
   The expression \exmp{&b} creates a \em{reference} to the variable
   \exmp{b} and it is the reference that gets passed to \exmp{add_10}.
   When the function \exmp{add_10} is called, the value of the local
   variable \exmp{a} will be a reference to the variable \exmp{b}.  It
   is only by \em{dereferencing} this value that \exmp{b} can be
   accessed and changed.  So, the statement \exmp{@a=@a+10} should be
   read as ``add 10 to the value of the object that \exmp{a}
   references and assign the result to the object that \exmp{a}
   references''.

   The reader familiar with C will note the similarity between
   \em{references} in \slang and \em{pointers} in C.

   References are not limited to variables.  A reference to a function
   may also be created and passed to other functions.  As a simple
   example from elementary calculus, consider the following function
   which returns an approximation to the derivative of another
   function at a specified point:
#v+
     define derivative (f, x)
     {
        variable h = 1e-6;
        return ((@f)(x+h) - (@f)(x)) / h;
     }
     define x_squared (x)
     {
        return x^2;
     }
     dydx = derivative (&x_squared, 3);
#v-
   When the \exmp{derivative} function is called, the local variable
   \exmp{f} will be a reference to the \exmp{x_squared} function. The
   \exmp{x_squared} function is called is called with the specified
   parameters by dereferencing \exmp{f} with the dereference operator.

#%}}}

\sect{Functions with a Variable Number of Arguments} #%{{{

  \slang functions may be called with a variable number of arguments.
  A natural example of such functions is the \ifun{strcat} function,
  which takes one or more string arguments and returns the
  concatenated result.  An example of different sort is the
  \ifun{strtrim} function which moves both leading and trailing
  whitespace from a string.  In this case, when called with one
  argument (the string to be ``trimmed''), the characters that are
  considered to be whitespace are those in the character-set that have
  the whitespace property (space, tab, newline, ...).  However, when
  called with two arguments, the second argument may be used to
  specify the characters that are to be considered as whitespace.  The
  \ifun{strtrim} function exemplifies a class of variadic functions
  where the additional arguments are used to pass optional information to
  the function.  Another more flexible and powerful way of passing
  optional information is through the use of \em{qualifiers}, which is
  the subject of the next section.

  When a \slang function is called with parameters, those parameters
  are placed on the run-time stack.  The function accesses those
  parameters by removing them from the stack and assigning them to the
  variables in its parameter list.  This details of this operation
  are for the most part hidden from the programmer.  But what happens
  when the number of parameters in the parameter list is not equal to
  the number of parameters passed to the function?  If the number
  passed to the function is less than what the function expects, a
  \var{StackUnderflow} error could result as the function tries to
  remove items from the stack.  If the number passed is greater than
  the number in the parameter list, then the extras will remain on the
  stack.  The latter feature makes it possible to write functions that
  take a variable number of arguments.

  Consider the \exmp{add_10} example presented earlier.  This time it
  is written
#v+
     define add_10 ()
     {
        variable x;
        x = ();
        return x + 10;
     }
     variable s = add_10 (12);  % ==> s = 22;
#v-
  For the uninitiated, this example looks as if it is destined for
  disaster.  The \exmp{add_10} function appears to accept zero
  arguments, yet it was called with a single argument.  On top of
  that, the assignment to \exmp{x} might look a bit strange.  The
  truth is, the code presented in this example makes perfect sense,
  once you realize what is happening.

  First, consider what happens when \exmp{add_10} is called with the
  parameter 12.  Internally, 12 is pushed onto the stack
  and then the function called.  Now, consider the function
  \exmp{add_10} itself.  In it, \exmp{x} is a local variable.
  The strange looking assignment `\exmp{x=()}' causes whatever is on
  the top of the stack to be assigned to \exmp{x}.  In other words, after
  this statement, the value of \exmp{x} will be 12, since
  12 is at the top of the stack.

  A generic function of the form
#v+
    define function_name (x, y, ..., z)
    {
       .
       .
    }
#v-
  is transformed internally by the parser to something akin to
#v+
    define function_name ()
    {
       variable x, y, ..., z;
       z = ();
       .
       .
       y = ();
       x = ();
       .
       .
    }
#v-
  before further parsing.  (The \exmp{add_10} function, as defined above, is
  already in this form.)  With this knowledge in hand, one can write a
  function that accepts a variable number of arguments.  Consider the
  function:
#v+
    define average_n (n)
    {
       variable x, y;
       variable s;

       if (n == 1)
         {
            x = ();
            s = x;
         }
       else if (n == 2)
         {
            y = ();
            x = ();
            s = x + y;
         }
       else throw NotImplementedError;

       return s / n;
   }
   variable ave1 = average_n (3.0, 1);        % ==> 3.0
   variable ave2 = average_n (3.0, 5.0, 2);   % ==> 4.0
#v-
  Here, the last argument passed to \exmp{average_n} is an integer
  reflecting the number of quantities to be averaged.  Although this
  example works fine, its principal limitation is obvious: it only
  supports one or two values.  Extending it to three or more values
  by adding more \exmp{else if} constructs is rather straightforward but
  hardly worth the effort.  There must be a better way, and there is:
#v+
   define average_n (n)
   {
      variable s, x;
      s = 0;
      loop (n)
        {
           x = ();    % get next value from stack
           s += x;
        }
      return s / n;
   }
#v-
  The principal limitation of this approach is that one must still
  pass an integer that specifies how many values are to be averaged.
  Fortunately, a special variable exists that is local to every function
  and contains the number of values that were passed to the function.
  That variable has the name \var{_NARGS} and may be used as follows:
#v+
   define average_n ()
   {
      variable x, s = 0;

      if (_NARGS == 0)
        usage ("ave = average_n (x, ...);");

      loop (_NARGS)
        {
           x = ();
           s += x;
        }
      return s / _NARGS;
   }
#v-
  Here, if no arguments are passed to the function, the \ifun{usage}
  function will generate a \var{UsageError} exception along with a
  simple message indicating how to use the function.

#%}}}

\sect{Qualifiers} #%{{{

  One way to pass optional information to a function is to do so using
  the variable arguments mechanism described in the previous section.
  However, a much more powerful mechanism is through the use of
  \em{qualifiers}, which were added in version 2.1.

  To illustrate the use of qualifiers, consider a graphics application
  that defines a function called \exmp{plot} that plots a set of (x,y)
  values specified as 1-d arrays:
#v+
     plot(x,y);
#v-
  Suppose that when called in the above manner, the application will
  plot the data as black points.  But instead of black points, one
  might want to plot the data using a red diamond as the plot symbol.
  It would be silly to have a separate function such as
  \exmp{plot_red_diamond} for this purpose.  A much better way to
  achieve this functionality is through the use of qualifiers:
#v+
    plot(x,y ; color="red", symbol="diamond");
#v-
  Here, a single semicolon is used to separate the argument-list
  proper (\exmp{x,y}) from the list of qualifiers.  In this case, the
  qualifiers are ``color'' and ``symbol''.  The order of the
  qualifiers in unimportant; the function could just as well have been
  called with the symbol qualifier listed first.

  Now consider the implementation of the \exmp{plot} function:
#v+
    define plot (x, y)
    {
       variable color = qualifier ("color", "black");
       variable symbol = qualifier ("symbol", "point");
       variable symbol_size = qualifier ("size", 1.0);
          .
          .
    }
#v-
  Note that the qualifiers are not handled in the parameter list;
  rather they are handled in the function body using the
  \ifun{qualifier} function, which is used to obtain the value of the
  qualifier. The second argument to the \ifun{qualifier} function
  specifies the default value to be used if the function was not
  called with the specified qualifier.  Also note that the variable
  associated with the qualifier need not have the same name as the
  qualifier.

  A qualifier need not have a value--- its mere presence may be used
  to enable or disable a feature or trigger some action.  For example,
#v+
     plot (x, y; connect_points);
#v-
  specifies a qualifier called \exmp{connect_points} that indicates
  that a line should be drawn between the data points.  The presence
  of such a qualifier can be detected using the
  \ifun{qualifier_exists} function:
#v+
     define plot (x,y)
     {
         .
         .
       variable connect_points = qualifier_exists ("connect_points");
         .
         .
     }
#v-

  Sometimes it is useful for a function to pass the qualifiers that it
  has received to other functions.  Suppose that the \exmp{plot}
  function calls \exmp{draw_symbol} to plot the specified symbol at a
  particular location and that it requires the symbol attibutes to be
  specified using qualifiers.  Then the plot function might look like:
#v+
    define plot (x, y)
    {
       variable color = qualifier ("color", "black");
       variable symbol = qualifier ("symbol", "point");
       variable symbol_size = qualifier ("size", 1.0);
          .
          .
       _for i (0, length(x)-1, 1)
         draw_symbol (x[i],y[i]
                      ;color=color, size=symbol_size, symbol=symbol);
          .
          .
    }
#v-
  The problem with this approach is that it does not scale well: the
  \exmp{plot} function has to be aware of all the qualifiers that the
  \exmp{draw_symbol} function takes and explicitly pass them.  In
  many cases this can be quite cumbersome and error prone.  Rather it
  is better to simply pass the qualifiers that were passed to the plot
  function on to the \exmp{draw_symbol} function.  This may be achieved
  using the \ifun{__qualifiers} function.  The \ifun{__qualifiers}
  function returns the list of qualifiers in the form of a structure
  whose field names are the same as the qualifier names.  In fact, the
  use of this function can simplify the implementation of the
  \exmp{plot} function, which may be coded more simply as
#v+
    define plot (x, y)
    {
       variable i;
       _for i (0, length(x)-1, 1)
         draw_symbol (x[i],y[i] ;; __qualifiers());
    }
#v-
  Note the syntax is slightly different.  The two semicolons indicate
  that the qualifiers are specified not as name-value pairs, but as a
  structure.  Using a single semicolon would have created a qualifier
  called \exmp{__qualifiers}, which is not what was desired.

  As alluded to above an added benefit of this approach is that the
  \exmp{plot} function does not need to know nor care about the
  qualifiers supported by \exmp{draw_symbol}.  When called as
#v+
    plot (x, y; symbol="square", size=2.0, fill=0.8);
#v-
  the \exmp{fill} qualifier would get passed to the \exmp{draw_symbol}
  function to specify the ``fill'' value to be used when creating
  the symbol.

#%}}}

\sect{Exit-Blocks} #%{{{

   An \em{exit-block} is a set of statements that get executed when a
   functions returns.  They are very useful for cleaning up when a
   function returns via an explicit call to \var{return} from deep
   within a function.

   An exit-block is created by using the \kw{EXIT_BLOCK} keyword
   according to the syntax
\begin{tscreen}
      EXIT_BLOCK { \em{statement-list} }
\end{tscreen}
   where \em{statement-list} represents the list of statements that
   comprise the exit-block.  The following example illustrates the use
   of an exit-block:
#v+
      define simple_demo ()
      {
         variable n = 0;

         EXIT_BLOCK { message ("Exit block called."); }

         forever
          {
            if (n == 10) return;
            n++;
          }
      }
#v-
   Here, the function contains an exit-block and a \var{forever} loop.
   The loop will terminate via the \kw{return} statement when \exmp{n}
   is 10.  Before it returns, the exit-block will get executed.

   A function can contain multiple exit-blocks, but only the last
   one encountered during execution will actually get used.  For
   example,
#v+
      define simple_demo (n)
      {
         EXIT_BLOCK { return 1; }

         if (n != 1)
           {
              EXIT_BLOCK { return 2; }
           }
         return;
      }
#v-
   If 1 is passed to this function, the first exit-block will
   get executed because the second one would not have been encountered
   during the execution.  However, if some other value is passed, the
   second exit-block would get executed.  This example also
   illustrates that it is possible to explicitly return from an
   exit-block, but nested exit-blocks are illegal.

#%}}}

\sect{Handling Return Values from a Function} #%{{{

   The most important rule to remember in calling a function is that
   \em{if the function returns a value, the caller must do something
   with it}. While this might sound like a trivial statement it is the
   number one issue that trips-up novice users of the language.

   To elaborate on this point further, consider the \em{fputs}
   function, which writes a string to a file descriptor.  This
   function can fail when, e.g., a disk is full, or the file is
   located on a network share and the network goes down, etc.

   \slang supports two mechanisms that a function may use to report a
   failure: raising an exception, returning a status code.  The latter
   mechanism is used by the \slang fputs function. i.e., it returns a
   value to indicate whether or not is was successful.  Many users
   familiar with this function either seem to forget this fact, or
   assume that the function will succeed and not bother handling the
   return value.  While some languages silently remove such values
   from the stack, \slang regards the stack as a dynamic data
   structure that programs can utilize.  As a result, the value will
   be left on the \slang stack and can cause problems later on.

   There are a number of correct ways of ``doing something'' with the
   return value from a function.  Of course the recommended procedure
   is to use the return value as it was meant to be used.  In the case
   of \ifun{fputs}, the proper thing to do is to check the return
   value, e.g.,
#v+
     if (-1 == fputs ("good luck", fp))
       {
          % Handle the error
       }
#v-
   Other acceptable ways to ``do something'' with the return value
   include assigning it to a dummy variable,
#v+
     dummy = fputs ("good luck", fp);
#v-
   or simply ``popping'' it from the stack:
#v+
     fputs ("good luck", fp);  pop();
#v-
   The latter mechanism can also be written as
#v+
     () = fputs ("good luck", fp);
#v-
   The last form is a special case of the \em{multiple assignment
   statement}, which was discussed earlier.  Since this
   form is simpler than assigning the value to a dummy variable or
   explicitly calling the \ifun{pop} function, it is recommended over
   the other two mechanisms.  Finally, this form has the
   redeeming feature that it presents a visual reminder that the
   function is returning a value that is not being used.

#%}}}

#%}}}

\labeled_chapter{Namespaces} #%{{{

#% FIXME: Add some guidelines for the usage of namespaces
#% FIXME: This chapter needs rewritten.

  By default, all global variables and functions are defined in the
  global or public namespace.  In addition to the global namespace,
  every compilation unit (e.g., a file containing \slang code) has a
  private, or anonymous namespace.  The private namespace is used when
  one wants to restrict the usage of one or more functions or
  variables to the compilation unit that defines them without worrying
  about objects with the same names defined elsewhere.

  Objects are declared as belonging to the private namespace using
  the \kw{private} declaration keyword.  Similarly if a variable is
  declared using the \kw{public} qualifier, it will be placed in the
  public namespace. For example,
#v+
    private variable i;
    public variable j;
#v-
 defines a variable called \exmp{i} in the private namespace and one
 called \exmp{j} in the public namespace.

 The \ifun{implements} function may be used to create a new namespace
 of a specified name and have it associated with the compilation unit.
 Objects may be placed into this namespace space using the
 \exmp{static} keyword, e.g.,
#v+
    static variable X;
    static define foo () {...}
#v-
 For this reason, such a namespace will be called the \em{static
 namespace} associated with the compilation unit. Such objects may be
 accessed from outside the local compilation unit using the namespace
 operator \exmp{->} in conjunction with the name of the namespace.

 Since it is possible for three namespaces (private, static, public)
 to be associated with a compilation unit, it is important to
 understand how names are resolved by the parser.  During the
 compilation stage, symbols are looked up according to the current
 scope.  If in a function, the local variables of the function are
 searched first.  Then the search proceeds with symbols in the private
 namespace, followed by those in the \kw{static} namespace associated
 with the compilation unit (if any), and finally with the public
 namespace.  If after searching the public namespace the symbol has
 not been resolved, an \exc{UndefinedNameError} exception will result.

 In addition to using the \ifun{implements} function, there are other
 ways to associate a namespace with a compilation unit.  One is via
 the optional namespace argument of the \exmp{evalfile} function.  For
 example,
#v+
    () = evalfile ("foo.sl", "bar");
#v-
 will cause \exmp{foo.sl} to be loaded and associated with a namespace
 called \exmp{bar}.  Then any static symbols of \exmp{foo.sl} may
 accessed using the \exmp{bar->} prefix.

 It is important to note that if a static namespace has been
 associated with the compilation unit, then any symbols in that unit
 declared without an namespace qualifier will be placed in the static
 namespace.  Otherwise such symbols will be placed in the public
 namespace, and any symbols declared as \kw{static} will be placed in
 the private namespace.

 To illustrate these concepts, consider the following example:
#v+
   % foo.sl
   variable X = 1;
   static variable Y;
   private variable Z;
   public define set_Y (y) { Y = y; }
   static define set_z (z) { Z = z; }
#v-
 If \exmp{foo.sl} is loaded via
#v+
    () = evalfile ("foo.sl");
#v-
 then no static namespace will be associated with it.  As a result,
 \exmp{X} will be placed in the public namespace since it was declared
 with no namespace qualifier.  Also \exmp{Y} and \exmp{set_z} will be
 placed in the private namespace since no static namespace has been
 associated with the file.  In this scenario there will be no way to get at
 the \exmp{Z} variable from outside of \exmp{foo.sl} since both it and
 the function that accesses it (\exmp{set_z}) are placed in the
 private namespace.

 On the other hand, suppose that the file is loaded using a namespace
 argument:
#v+
    () = evalfile ("foo.sl", "foo");
#v-
 In this case \exmp{X}, \exmp{Y}, and \exmp{get_z} will be placed in the
 \exmp{foo} namespace.  These objects may be accessed from outside
 \exmp{foo.sl} using the \exmp{foo->} prefix, e.g.,
#v+
    foo->set_z (3.0);
    if (foo->X == 2) foo->Y = 1;
#v-

  Because a file may be loaded with or without a namespace attached to
  it, it is a good idea to avoid using the \exmp{static} qualifier. To
  see this, consider again the above example but this time without the
  use of the \exmp{static} qualifier:
#v+
    % foo.sl
    variable X = 1;
    variable Y;
    private variable Z;
    public define set_Y (y) { Y = y; }
    define set_z (z) { Z = z; }
#v-
  When loaded without a namespace argument, the variable
  \exmp{Z} will remain in the private namespace, but the \exmp{set_z}
  function will be put in the public namespace.  Previously
  \exmp{set_z} was put in the private namespace making both it and
  \exmp{Z} inaccessible.

#%}}}

\labeled_chapter{Arrays} #%{{{

   An array is a container object that can contain many values of one
   data type.  Arrays are very useful objects and are indispensable
   for certain types of programming.  The purpose of this chapter is
   to describe how arrays are defined and used in the \slang language.

\sect{Creating Arrays} #%{{{

   The \slang language supports multi-dimensional arrays of all data
   types.  Since the \dtype{Array_Type} is a data type, one can even
   have arrays of arrays.  To create a multi-dimensional array of
   \em{SomeType} and assign to some variable, use:
#v+
      a = SomeType [dim0, dim1, ..., dimN];
#v-
   Here \em{dim0}, \em{dim1}, ... \em{dimN} specify the size of
   the individual dimensions of the array.  The current implementation
   permits arrays to contain as many as 7 dimensions.  When a
   numeric array is created, all its elements are initialized to zero.
   The initialization of other array types depend upon the data type,
   e.g., the elements in \var{String_Type} and \var{Struct_Type} arrays are
   initialized to \var{NULL}.

   As a concrete example, consider
#v+
     a = Integer_Type [10];
#v-
   which creates a one-dimensional array of 10 integers and
   assigns it to \var{a}.
   Similarly,
#v+
     b = Double_Type [10, 3];
#v-
   creates a \var{30} element array of double precision numbers
   arranged in \var{10} rows and \var{3} columns, and assigns it to
   \var{b}.

\sect1{Range Arrays}

   There is a more convenient syntax for creating and initializing 1-d
   arrays.  For example, to create an array of ten integers whose
   elements run from 1 through 10, one may simply use:
#v+
     a = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
#v-
   Similarly,
#v+
     b = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];
#v-
   specifies an array of ten doubles.

   An even more compact way of specifying a numeric array is to use a
   \em{range-array}.  For example,
#v+
     a = [0:9];
#v-
   specifies an array of 10 integers whose elements range from 0
   through 9.  The syntax for the most general form of range array is
   given by
#v+
     [first-value : last-value : increment]
#v-
   where the \em{increment} is optional and defaults to 1. This
   creates an array whose first element is \em{first-value} and whose
   successive values differ by \em{increment}.  \em{last-value} sets
   an upper limit upon the last value of the array as described below.

   If the range array \exmp{[a:b:c]} is integer valued, then the
   interval specified by \exmp{a} and \exmp{b} is closed.  That is, the
   kth element of the array \math{x_k} is given by \math{x_k=a+kc} and
   satisfies \math{a<=x_k<=b}.  Hence, the number of elements in an
   integer range array is given by the expression \math{1 + (b-a)/c}.

   The situation is somewhat more complicated for floating point range
   arrays.  The interval specified by a floating point range array
   \exmp{[a:b:c]} is semi-open such that \exmp{b} is not contained in
   the interval.  In particular, the kth element of \exmp{[a:b:c]} is
   given by \math{x_k=a+kc} such that \math{a<=x_k<b} when
   \math{c>=0}, and \math{b<x_k<=a} otherwise.  The number of elements
   in the array is one greater than the largest \math{k} that
   satisfies the open interval constraint.

   In contrast, a range-array expressed in the form \exmp{[a:b:#n]}
   represents an array of exactly n elements running from \exmp{a} to
   \exmp{b} inclusive.  It is equivalent to
   \exmp{a+[0:n-1]*(b-a)/(n-1)}.

   Here are a few examples that illustrate the above comments:
#v+
       [1:5:1]         ==> [1,2,3,4,5]
       [1.0:5.0:1.0]   ==> [1.0, 2.0, 3.0, 4.0]
       [5:1:-1]        ==> [5,4,3,2,1]
       [5.0:1.0:-1.0]  ==> [5.0, 4.0, 3.0, 2.0];
       [1:1]           ==> [1]
       [1.0:1.0]       ==> []
       [1.0:1.0001]    ==> [1.0]
       [1:-3]          ==> []
       [0:1:#5]        ==> [0.0, 0.25, 0.50, 0.75, 1.0]
       [0:-1:#3]       ==> [0.0, -0.5, -1.0]
#v-

   Currently \dtype{Int_Type} is the only integer type supported
   by range arrays--- arbitrary integer types will be supported in a
   future version.  This means that \exmp{[1h:5h]} will not produce
   an array of \dtype{Short_Type}, rather it will produce an
   \dtype{Int_Type} array.  However, \exmp{[1h,2h,3h,4h,5h]} will
   produce an array of \dtype{Short_Type} integers.

\sect1{Creating arrays via the dereference operator}

   Another way to create an array is to apply the dereference operator
   \var{@} to the \dtype{DataType_Type} literal \dtype{Array_Type}.  The
   actual syntax for this operation resembles a function call
\begin{tscreen}
     variable a = @Array_Type (\em{data-type}, \em{integer-array});
\end{tscreen}
  where \em{data-type} is of type \dtype{DataType_Type} and
  \em{integer-array} is a 1-d array of integers that specify the size
  of each dimension.  For example,
#v+
     variable a = @Array_Type (Double_Type, [10, 20]);
#v-
  will create a 10 by 20 array of doubles and assign it
  to \exmp{a}.  This method of creating arrays derives its power from
  the fact that it is more flexible than the methods discussed in this
  section.  It is particularly useful for creating arrays during
  run-time in situations where the data-type can vary.

#%}}}

\sect{Reshaping Arrays} #%{{{
   It is sometimes useful to change the `shape' of an array using
   the \ifun{reshape} function.  For example, a 1-d 10 element array
   may be reshaped into a 2-d array consisting of 5 rows and 2
   columns.  The only restriction on the operation is that the arrays
   must be commensurate.  The \ifun{reshape} function follows the
   syntax
\begin{tscreen}
       reshape (\em{array-name}, \em{integer-array});
\end{tscreen}
   where \em{array-name} specifies the array to be reshaped to
   the dimensions given by \var{integer-array}, a 1-dimensional array of
   integers.  It is important to note that this does \em{not} create a
   new array, it simply reshapes the existing array.  Thus,
#v+
     variable a = Double_Type [100];
     reshape (a, [10, 10]);
#v-
   turns \var{a} into a 10 by 10 array, as well as any
   other variables attached to the array.

   The \ifun{_reshape} function works like \ifun{reshape} except that
   it creates a new array instead of changing the shape of an existing
   array:
#v+
     new_a = _reshape (a, [10,10]);
#v-

#%}}}

\sect{Simple Array Indexing} #%{{{

   An individual element of an array may be referred to by its
   \em{index}.  For example, \exmp{a[0]} specifies the zeroth element
   of the one dimensional array \exmp{a}, and \exmp{b[3,2]} specifies
   the element in the third row and second column of the two
   dimensional array \exmp{b}.  As in C, array indices are numbered
   from 0.  Thus if \exmp{a} is a one-dimensional array of ten
   integers, the last element of the array is given by \exmp{a[9]}.
   Using \exmp{a[10]} would result in an \var{IndexError} exception.

   A negative index may be used to index from the end of the array,
   with \exmp{a[-1]} referring to the last element of \exmp{a}.
   Similarly, \exmp{a[-2]} refers to the next to the last element, and
   so on.

   One may use the indexed value like any other variable.  For
   example, to set the third element of an integer array to 6, use
#v+
     a[2] = 6;
#v-
   Similarly, that element may be used in an expression, such as
#v+
     y = a[2] + 7;
#v-
   Unlike other \slang variables which inherit a type upon assignment,
   array elements already have a type and any attempt to assign a
   value with an incompatible type will result in a
   \var{TypeMismatchError} exception.  For example, it is illegal to
   assign a string value to an integer array.

   One may use any integer expression to index an array.  A simple
   example that computes the sum of the elements of a 10 element 1-d
   array is
#v+
      variable i, s;
      s = 0;
      for (i = 0; i < 10; i++) s += a[i];
#v-
   (In practice, do not carry out sums this way--- use the
   \ifun{sum} function instead, which is much simpler and faster, i.e.,
   \exmp{s=sum(a)}).

\sect{Indexing Multiple Elements with Ranges}

   Unlike many other languages, \slang permits arrays to be indexed by
   other integer arrays.   Suppose that \exmp{a} is a 1-d array of 10
   doubles.  Now consider:
#v+
      i = [6:8];
      b = a[i];
#v-
   Here, \exmp{i} is a 1-dimensional range array of three integers with
   \exmp{i[0]} equal to 6, \exmp{i[1]} equal to 7,
   and \exmp{i[2]} equal to 8.  The statement \var{b = a[i];}
   will create a 1-d array of three doubles and assign it to \var{b}.
   The zeroth element of \var{b}, \exmp{b[0]} will be set to the sixth
   element of \var{a}, or \exmp{a[6]}, and so on.  In fact, these two simple
   statements are equivalent to
#v+
     b = Double_Type [3];
     b[0] = a[6];
     b[1] = a[7];
     b[2] = a[8];
#v-
   except that using an array of indices is not only much more
   convenient, but executes much faster.

   More generally, one may use an index array to specify which
   elements are to participate in a calculation.  For example, consider
#v+
     a = Double_Type [1000];
     i = [0:499];
     j = [500:999];
     a[i] = -1.0;
     a[j] = 1.0;
#v-
   This creates an array of 1000 doubles and sets the first
   500 elements to \exmp{-1.0} and the last 500 to
   \exmp{1.0}.  Actually, one may do away with the \exmp{i} and \exmp{j}
   variables altogether and use
#v+
     a = Double_Type [1000];
     a[[0:499]] = -1.0;
     a[[500:999]] = 1.0;
#v-
   It is important to note that the syntax requires the use of the
   double square brackets, and in particular that \exmp{a[[0:499]]} is
   \em{not} the same as \exmp{a[0:499]}.  In fact, the latter will
   generate a syntax error.

   Index-arrays are not contrained to be one-dimensional arrays.  Suppose
   that \exmp{I} represents a multidimensional index array, and that \exmp{A}
   is the array to be indexed.  Then what does \exmp{A[I]} represent?
   Its value will be an array of the same type as \exmp{A}, but with
   the dimensionality of \em{I}.  For example,
#v+
    a = 1.0*[1:10];
    i = _reshape ([4,5,6,7,8,9], [2,3]);
#v-
   defines \exmp{a} to be a 10 element array of doubles, and \exmp{i}
   to be \exmp{2x3} array of integers.  Then \exmp{a[i]} will be a
   \exmp{2x3} array of doubles with elements:
#v+
    a[4]   a[5]   a[6]
    a[7]   a[8]   a[9]
#v-

   Often, it is convenient to use a ``rubber'' range to specify
   indices.  For example, \exmp{a[[500:]]} specifies all elements of
   \var{a} whose index is greater than or equal to \var{500}.  Similarly,
   \exmp{a[[:499]]} specifies the first 500 elements of \var{a}.
   Finally, \exmp{a[[:]]} specifies all the elements of \var{a}.  The
   latter form may also be written as \exmp{a[*]}.

   One should be careful when using index arrays with negative
   elements.  As pointed out above, a negative index is used to index
   from the end of the array.  That is, \exmp{a[-1]} refers to the
   last element of \exmp{a}.  How should \exmp{a[[[0:-1]]} be
   interpreted?

   In version 1 of the interpreter, when used in an
   array indexing context, a construct such as \exmp{[0:-1]} was taken
   to mean from the first element through the last.  While this might
   seem like a convenient shorthand, in retrospect it was a bad idea.
   For this reason, the meaning of a ranges over negative valued
   indices was changed in version 2 of the interpreter as follows:
   First the index-range gets expanded to an array of indices
   according to the rules for range arrays described above.  Then if
   any of the resulting indices are negative, they are interpreted as
   indices from the end of the array.  For example, if \exmp{a}
   is an array of 10 elements, then \exmp{a[[-2:3]]} is first expanded
   to \exmp{a[[-2,-1,0,1,2,3]]}, and then to the 6 element array
#v+
    [ a[8], a[9], a[0], a[1], a[2], a[3] ]
#v-
   So, what does \var{a[[0:-1]]} represent in the new interpretation?
   Since \exmp{[0:-1]} expands to an empty array, \var{a[[0:-1]]} will
   also produce an empty array.

   Indexing of multidimensional arrays using ranges works similarly.
   Suppose \var{a} is a 100 by 100 array of doubles.  Then
   the expression \var{a[0, *]} specifies all elements in the zeroth
   row.  Similarly, \var{a[*, 7]} specifies all elements in the
   seventh column.  Finally, \var{a[[3:5],[6:12]]} specifies the
   3 by 7 region consisting of rows 3, 4,
   and 5, and columns 6 through 12 of \var{a}.

   Before leaving this section, a few examples are presented to
   illustrate some of these points.

   The ``trace'' of a matrix is an important concept that occurs
   frequently in linear algebra.  The trace of a 2d matrix is given by
   the sum of its diagonal elements.  Consider the creation of a
   function that computes the trace of such a matrix.

   The most straightforward implementation of such a function uses an
   explicit loop:
#v+
      define array_trace (a, n)
      {
         variable s = 0, i;
         for (i = 0; i < n; i++) s += a[i, i];
         return s;
      }
#v-
   Better yet is to recognize that the diagonal elements of an
   \exmp{n} by \exmp{n} array are given by an index array \exmp{I}
   with elements 0, \exmp{n+1}, \exmp{2*n+2}, ..., \exmp{n*n-1}, or
   more precisely as
#v+
     [0:n*n-1:n+1]
#v-
   Hence the above may be written more simply as
#v+
     define array_trace (a, n)
     {
        return sum (a[[0:n*n-1:n+1]]);
     }
#v-

   The following example creates a 10 by 10 integer array, sets
   its diagonal elements to 5, and then computes the trace of
   the array:
#v+
      a = Integer_Type [10, 10];
      a[[0:99:11]] = 5;
      the_trace = array_trace(a, 10);
#v-

   In the previous examples, the size of the array was passed as an
   additional argument.  This is unnecessary because the size may be
   obtained from array itself by using the \ifun{array_shape}
   function.  For example, the following function may be used to
   obtain the indices of the diagonal element of an array:
#v+
     define diag_indices (a)
     {
        variable dims = array_shape (a);
        if (length (dims) != 2)
          throw InvalidParmError, "Expecting a 2d array";
        if (dims[0] != dims[1])
          throw InvalidParmError, "Expecting a square array";
        variable n = dims[0];
        return [0:n*(n-1):n+1];
     }
#v-
   Using this function, the trace function may be written more simply as
#v+
     define array_trace (a)
     {
        return sum (a[diag_indices(a)]);
     }
#v-
   Another example of this technique is a function that creates an
   \exmp{n} by \exmp{n} unit matrix:
#v+
      define unit_matrix (n)
      {
         variable a = Int_Type[n, n];
         a[diag_indices(a)] = 1;
         return a;
      }
#v-

#%}}}

\sect{Arrays and Variables}

   When an array is created and assigned to a variable, the
   interpreter allocates the proper amount of space for the array,
   initializes it, and then assigns to the variable a \em{reference}
   to the array.   So, a variable that represents an array has a value
   that is really a reference to the array.  This has several
   consequences, most good and some bad.  It is believed that the
   advantages of this representation outweigh the disadvantages.
   First, we shall look at the positive aspects.

   When a variable is passed to a function, it is always the value of
   the variable that gets passed.  Since the value of a variable
   representing an array is a reference, a reference to the array gets
   passed.  One major advantage of this is rather obvious: it is a
   fast and efficient way to pass the array.  This also has another
   consequence that is illustrated by the function
#v+
      define init_array (a)
      {
         variable i;
         variable n = length(a);
         _for i (0, n-1, 1)
           a[i] = some_function (i);
      }
#v-
   where \exmp{some_function} is a function that generates a scalar
   value to initialize the \em{ith} element.  This function can be
   used in the following way:
#v+
      variable X = Double_Type [100000];
      init_array (X);
#v-
   Since the array is passed to the function by reference, there is no
   need to make a separate copy of the \var{100000} element array. As
   pointed out above, this saves both execution time and memory. The
   other salient feature to note is that any changes made to the
   elements of the array within the function will be manifested in the
   array outside the function.  Of course, in this case this is a
   desirable side-effect.

   To see the downside of this representation, consider:
#v+
      a = Double_Type [10];
      b = a;
      a[0] = 7;
#v-
   What will be the value of \exmp{b[0]}?  Since the value of \exmp{a}
   is really a reference to the array of ten doubles, and that
   reference was assigned to \exmp{b}, \exmp{b} also refers to the same
   array.  Thus any changes made to the elements of \exmp{a}, will also
   be made implicitly to \exmp{b}.

   This begs the question: If the assignment of a variable attached to
   an an array to another variable results in the assignment of the
   same array, then how does one make separate copies
   of the array?  There are several answers including using an index
   array, e.g., \exmp{b = a[*]}; however, the most natural method is
   to use the dereference operator:
#v+
      a = Double_Type [10];
      b = @a;
      a[0] = 7;
#v-
   In this example, a separate copy of \exmp{a} will be created and
   assigned to \exmp{b}.  It is very important to note that \slang
   never implicitly dereferences an object.  So, one must explicitly use
   the dereference operator.  This means that the elements of a
   dereferenced array are not themselves dereferenced.  For example,
   consider dereferencing an array of arrays, e.g.,
#v+
      a = Array_Type [2];
      a[0] = Double_Type [10];
      a[1] = Double_Type [10];
      b = @a;
#v-
   In this example, \exmp{b[0]} will be a reference to the array that
   \exmp{a[0]} references because \exmp{a[0]} was not explicitly
   dereferenced.

\sect{Using Arrays in Computations} #%{{{

   Many functions and operations work transparently with arrays.
   For example, if \exmp{a} and \exmp{b} are arrays, then the sum
   \exmp{a + b} is an array whose elements are formed from the sum of
   the corresponding elements of \exmp{a} and \exmp{b}.  A similar
   statement holds for all other binary and unary operations.

   Let's consider a simple example.  Suppose, that we wish to solve a
   set of \exmp{n} quadratic equations whose coefficients are given by
   the 1-d arrays \exmp{a}, \exmp{b}, and \exmp{c}.  In general, the
   solution of a quadratic equation will be two complex numbers.  For
   simplicity, suppose that all we really want is to know what subset of
   the coefficients, \exmp{a}, \exmp{b}, \exmp{c}, correspond to
   real-valued solutions.  In terms of \kw{for} loops, we can write:
#v+
     index_array = Char_Type [n];
     _for i (0, n-1, 1)
       {
          d = b[i]^2 - 4 * a[i] * c[i];
          index_array [i] = (d >= 0.0);
       }
#v-
   In this example, the array \exmp{index_array} will contain a
   non-zero value if the corresponding set of coefficients has a
   real-valued solution.  This code may be written much more compactly
   and with more clarity as follows:
#v+
     index_array = ((b^2 - 4 * a * c) >= 0.0);
#v-
   Moreover, it executes about 20 times faster than the version using
   an explicit loop.

   \slang has a powerful built-in function called \ifun{where}.  This
   function takes an array of boolean values and returns an array of
   indices that correspond to where the elements of the input array
   are non-zero.  The utility of this simple operation cannot be
   overstated.  For example, suppose \exmp{a} is a 1-d array of \exmp{n}
   doubles, and it is desired to set all elements of the array whose
   value is less than zero to zero.  One way is to use a \kw{for}
   loop:
#v+
     _for i (0, n-1, 1)
       if (a[i] < 0.0) a[i] = 0.0;
#v-
   If \exmp{n} is a large number, this statement can take some time to
   execute.  The optimal way to achieve the same result is to use the
   \ifun{where} function:
#v+
     a[where (a < 0.0)] = 0;
#v-
   Here, the expression \exmp{(a < 0.0)} returns a boolean array whose
   dimensions are the same size as \exmp{a} but whose elements are
   either 1 or 0, according to whether or not the corresponding
   element of \exmp{a} is less than zero.  This array of zeros and ones
   is then passed to the \ifun{where} function, which returns a 1-d
   integer array of indices that indicate where the elements of
   \exmp{a} are less than zero.  Finally, those elements of \exmp{a} are
   set to zero.

   Consider once more the example involving the set of \exmp{n}
   quadratic equations presented above.  Suppose that we wish to get
   rid of the coefficients of the previous example that generated
   non-real solutions.  Using an explicit \kw{for} loop requires code
   such as:
#v+
     nn = 0;
     _for i (0, n-1, 1)
       if (index_array [i]) nn++;

     tmp_a = Double_Type [nn];
     tmp_b = Double_Type [nn];
     tmp_c = Double_Type [nn];

     j = 0;
     _for i (0, n-1, 1)
       {
          if (index_array [i])
            {
               tmp_a [j] = a[i];
               tmp_b [j] = b[i];
               tmp_c [j] = c[i];
               j++;
            }
       }
     a = tmp_a;
     b = tmp_b;
     c = tmp_c;
#v-
   Not only is this a lot of code, making it hard to digest, but it is
   also clumsy and error-prone.  Using the \ifun{where} function, this
   task is trivial and executes in a fraction of the time:
#v+
     i = where (index_array != 0);
     a = a[i];
     b = b[i];
     c = c[i];
#v-

   Most of the examples up till now assumed that the dimensions of the
   array were known.  Although the intrinsic function \ifun{length}
   may be used to get the total number of elements of an array, it
   cannot be used to get the individual dimensions of a
   multi-dimensional array.  The \ifun{array_shape} function may
   be used to determine the dimensionality of an array.  It may be used
   to determine the number of rows of an array as follows:
#v+
     define num_rows (a)
     {
        return array_shape (a)[0];
     }
#v-
   The number of columns may be obtained in a similar manner:
#v+
     define num_cols (a)
     {
        variable dims = array_shape (a);
        if (length(dims) > 1) return dims[1];
        return 1;
     }
#v-
   The \ifun{array_shape} function may also be used to create an array
   that has the same number of dimensions as another array:
#v+
     define make_int_array (a)
     {
        return @Array_Type (Int_Type, array_shape (a));
     }
#v-

   Finally, the \ifun{array_info} function may be used to get
   additional information about an array, such as its data type and
   size.

#%}}}

\sect{Arrays of Arrays: A Cautionary Note}
  Sometimes it is desirable to create an array of arrays.  For example,
#v+
     a = Array_Type[3];
     a[0] = [1:10];
     a[1] = [1:100];
     a[2] = [1:1000];
#v-
  will produce an array of the 3 arrays \exmp{[1:10]}, \exmp{[1:100]},
  and \exmp{[1:1000]}.  Index arrays may be used to access elements of
  an array of arrays: a[[1,2]] will produce an array of arrays that
  consists of the elements a[1] and a[2].  However, it is important to
  note that setting the elements of an array of arrays via an index
  array does not work as one might naively expect.  Consider the
  following:
#v+
     b = Array_Type[3];
     b[*] = a[[2,1,0]];
#v-
  where \exmp{a} is the array of arrays given in the previous example.
  The reader might expect \exmp{b} to have elements
  \exmp{b[0]=a[2]}, \exmp{b[1]=a[1]}, and \exmp{b[2]=a[0]}, and be
  surprised to learn that \exmp{b[0]=b[1]=b[2]=a[[2,1,0]]}.  The reason
  for this is that, by definition, \exmp{b} is an array of arrays, and
  even though \exmp{a[[2,1,0]]} is an array of arrays, it is
  first and foremost an array, and it is that array that is assigned
  to the elements of \exmp{b}.

#%}}}

\chapter{Associative Arrays} #%{{{

   An associative array differs from an ordinary array in the sense
   that its size is not fixed and that it is indexed by a string, called
   the \em{key}. For example, consider:
#v+
       A = Assoc_Type [Int_Type];
       A["alpha"] = 1;
       A["beta"] = 2;
       A["gamma"] = 3;
#v-
   Here, \exmp{A} has been assigned to an associative array of integers
   (\dtype{Int_Type}) and then three keys were been added to the array.

   As the example suggests, an associative array may be created using
   one of the following forms:
\begin{tscreen}
      Assoc_Type [\em{type}]
      Assoc_Type [\em{type}, \em{default-value}]
      Assoc_Type []
\end{tscreen}
   The last form returns an \em{un-typed} associative array capable of
   storing values of any type.

   The form involving a \em{default-value} is useful for associating a
   default value with non-existent array members.  This feature is
   explained in more detail below.

   There are several functions that are specially designed to work
   with associative arrays.  These include:
\begin{itemize}
\item \var{assoc_get_keys}, which returns an ordinary array of strings
      containing the keys of the array.

\item \var{assoc_get_values}, which returns an ordinary array of the
      values of the associative array.  If the associative array is
      un-typed, then an array of \dtype{Any_Type} objects will be
      returned.

\item \var{assoc_key_exists}, which can be used to determine whether
      or not a key exists in the array.

\item \var{assoc_delete_key}, which may be used to remove a key (and
      its value) from the array.
\end{itemize}

   To illustrate the use of an associative array, consider the problem
   of counting the number of repeated occurrences of words in a list.
   Let the word list be represented as an array of strings given by
   \exmp{word_list}.  The number of occurrences of each word may be
   stored in an associative array as follows:
#v+
     a = Assoc_Type [Int_Type];
     foreach word (word_list)
       {
          if (0 == assoc_key_exists (a, word))
            a[word] = 0;
          a[word]++;  % same as a[word] = a[word] + 1;
       }
#v-
   Note that \var{assoc_key_exists} was necessary to determine whether
   or not a word was already added to the array in order to properly
   initialize it.  However, by creating the associative array with a
   default value of 0, the above code may be simplified to
#v+
     variable a, word;
     a = Assoc_Type [Int_Type, 0];
     foreach word (word_list)
       a[word]++;
#v-

   Associative arrays are extremely useful and have may other
   applications.  Whenever there is a one to one mapping between a
   string and some object, one should always consider using an
   associative array to represent the mapping.  To illustrate this
   point, consider the following code fragment:
#v+
      define call_function (name, arg)
      {
         if (name == "foo") return foo (arg);
         if (name == "bar") return bar (arg);
           .
           .
         if (name == "baz") return baz (arg);
         throw InvalidParmError;
      }
#v-
   This represents a mapping between names and functions.  Such a
   mapping may be written in terms of an associative array as follows:
#v+
      private define invalid_fun (arg) { throw InvalidParmError; }
      Fun_Map = Assoc_Type[Ref_Type, &invalid_fun];
      define add_function (name, fun)
      {
         Fun_Map[name] = fun;
      }
      add_function ("foo", &foo);
      add_function ("bar", &bar);
         .
         .
      add_function ("baz", &baz);
      define call_function (name, arg)
      {
         return (@Fun_Map[name])(arg);
      }
#v-
   The most redeeming feature of the version involving the series of
   \kw{if} statements is that it is easy to understand.  However, the
   version involving the associative array has two significant
   advantages over the former.  Namely, the function lookup will be
   much faster with a time that is independent of the item being
   searched, and it is extensible in the sense that additional
   functions may be added at run-time, e.g.,
#v+
      add_function ("bing", &bing);
#v-

#%}}}

\chapter{Structures and User-Defined Types} #%{{{

   A \em{structure} is a heterogeneous container object, i.e., it is
   an object with elements whose values do not have to be of the same
   data type.  The elements or fields of a structure are named, and
   one accesses a particular field of the structure via the field
   name. This should be contrasted with an array whose values are of
   the same type, and whose elements are accessed via array indices.

   A \em{user-defined} data type is a structure with a fixed set of
   fields defined by the user.

\sect{Defining a Structure} #%{{{

   The \kw{struct} keyword is used to define a structure.  The syntax
   for this operation is:
\begin{tscreen}
     struct {\em{field-name-1}, \em{field-name-2}, ... \em{field-name-N}};
\end{tscreen}
   This creates and returns a structure with \em{N} fields whose names
   are specified by \em{field-name-1}, \em{field-name-2}, ...,
   \em{field-name-N}.  When a structure is created, the values of its
   fields are initialized to \NULL.

   For example,
#v+
     variable t = struct { city_name, population, next };
#v-
   creates a structure with three fields and assigns it to the
   variable \exmp{t}.

   Alternatively, a structure may be created by dereferencing
   \dtype{Struct_Type}.  Using this technique, the above structure may
   be created using one of the two forms:
#v+
      t = @Struct_Type ("city_name", "population", "next");
      t = @Struct_Type (["city_name", "population", "next"]);
#v-
   This approach is useful when creating structures dynamically where
   one does not know the name of the fields until run-time.

   Like arrays, structures are passed around by reference.  Thus,
   in the above example, the value of \exmp{t} is a reference to the
   structure.  This means that after execution of
#v+
     u = t;
#v-
   \em{both} \var{t} and \var{u} refer to the \em{same} underlying
   structure, since only the reference was copied by the assignment.  To
   actually create a new copy of the structure, use the
   \em{dereference} operator, e.g.,
#v+
     variable u = @t;
#v-
   It create new structure whose field names are identical to the old
   and copies the field values to the new structure.  If any of the
   values are objects that are passed by reference, then only the
   references will be copied.  In other words,
#v+
      t = struct{a};
      t.a = [1:10];
      u = @t;
#v-
   will produce a structure \exmp{u} that references the same array as
   \exmp{t}.

#%}}}

\sect{Accessing the Fields of a Structure} #%{{{

   The dot (\var{.}) operator is used to specify the particular
   field of structure.  If \exmp{s} is a structure and \exmp{field_name}
   is a field of the structure, then \exmp{s.field_name} specifies
   that field of \exmp{s}.  This specification can be used in
   expressions just like ordinary variables.  Again, consider
#v+
     t = struct { city_name, population, next };
#v-
   described in the last section.  Then,
#v+
     t.city_name = "New York";
     t.population = 13000000;
     if (t.population > 200) t = t.next;
#v-
   are all valid statements involving the fields of \exmp{t}.

#%}}}

\labeled_sect{Linked Lists} #%{{{

  One of the most important uses of structures is the creation of
  \em{dynamic} data structures such as \em{linked-lists}.
  A linked-list is simply a chain of structures that are linked
  together such that one structure in the chain is the value of a
  field of the previous structure in the chain.  To be concrete,
  consider the structure discussed earlier:
#v+
     t = struct { city_name, population, next };
#v-
  and suppose that it is desired to create a linked-list of such
  objects to store population data.
  The purpose of the \exmp{next} field is to provide the link to the
  next structure in the chain.  Suppose that there exists a function,
  \exmp{read_next_city}, that reads city names and populations from a
  file.  Then the list may be created using:
#v+
     define create_population_list ()
     {
        variable city_name, population, list_root, list_tail;
        variable next;

        list_root = NULL;
        while (read_next_city (&city_name, &population))
          {
             next = struct {city_name, population, next };

             next.city_name = city_name;
             next.population = population;
             next.next = NULL;

             if (list_root == NULL)
               list_root = next;
             else
               list_tail.next = next;

             list_tail = next;
          }
        return list_root;
     }
#v-
  In this function, the variables \exmp{list_root} and \exmp{list_tail}
  represent the beginning and end of the list, respectively.  As long
  as \exmp{read_next_city} returns a non-zero value, a new structure is
  created, initialized, and then appended to the list via the
  \exmp{next} field of the \exmp{list_tail} structure.  On the first
  time through the loop, the list is created via the assignment to the
  \exmp{list_root} variable.

  This function may be used as follows:
#v+
    Population_List = create_population_list ();
    if (Population_List == NULL)
      throw RunTimeError, "List is empty";
#v-
  Other functions may be created that manipulate the list.  Here is one
  that finds the city with the largest population:
#v+
    define get_largest_city (list)
    {
       variable largest;

       largest = list;
       while (list != NULL)
         {
            if (list.population > largest.population)
              largest = list;
            list = list.next;
         }
       return largest.city_name;
    }

    vmessage ("%s is the largest city in the list",
               get_largest_city (Population_List));
#v-
  The \exmp{get_largest_city} is a typical example of how one traverses
  a linear linked-list by starting at the head of the list and
  successively moves to the next element of the list via the
  \exmp{next} field.

  In the previous example, a \kw{while} loop was used to traverse the
  linked list.  It is also possible to use a \kw{foreach} loop for this:
#v+
    define get_largest_city (list)
    {
       variable largest, elem;

       largest = list;
       foreach elem (list)
         {
            if (elem.population > largest.population)
              largest = elem;
         }
       return largest.city_name;
    }
#v-
  Here a \kw{foreach} loop has been used to walk the list via its
  \exmp{next} field.  If the field name linking the elements was not
  called \exmp{next}, then it would have been necessary to use the
  \kw{using} form of the \kw{foreach} statement.  For example, if the
  field name implementing the linked list was \exmp{next_item}, then
#v+
     foreach item (list) using ("next_item")
       {
          .
          .
       }
#v-
  would have been used.  In other words, unless otherwise indicated
  via the \kw{using} clause, \kw{foreach} walks the list using a field
  named \exmp{next} by default.

  Now consider a function that sorts the list according to population.
  To illustrate the technique, a \em{bubble-sort} will be used, not
  because it is efficient (it is not), but because it is simple,
  intuitive, and provides another example of structure manipulation:
#v+
    define sort_population_list (list)
    {
       variable changed;
       variable node, next_node, last_node;
       do
         {
            changed = 0;
            node = list;
            next_node = node.next;
            last_node = NULL;
            while (next_node != NULL)
              {
                 if (node.population < next_node.population)
                   {
                      % swap node and next_node
                      node.next = next_node.next;
                      next_node.next = node;
                      if (last_node != NULL)
                        last_node.next = next_node;

                      if (list == node) list = next_node;
                      node = next_node;
                      next_node = node.next;
                      changed++;
                   }
                 last_node = node;
                 node = next_node;
                 next_node = next_node.next;
              }
         }
       while (changed);

       return list;
    }
#v-
   Note the test for equality between \exmp{list} and \exmp{node}, i.e.,
#v+
                      if (list == node) list = next_node;
#v-
   It is important to appreciate the fact that the values of these
   variables are references to structures, and that the
   comparison only compares the references and \em{not} the actual
   structures they reference.  If it were not for this, the algorithm
   would fail.

#%}}}

\sect{Defining New Types} #%{{{

   A user-defined data type may be defined using the \kw{typedef}
   keyword.  In the current implementation, a user-defined data type
   is essentially a structure with a user-defined set of fields. For
   example, in the previous section a structure was used to represent
   a city/population pair.  We can define a data type called
   \var{Population_Type} to represent the same information:
#v+
      typedef struct
      {
         city_name,
         population
      } Population_Type;
#v-
   This data type can be used like all other data types.  For example,
   an array of Population_Type types can be created,
#v+
      variable a = Population_Type[10];
#v-
   and `populated' via expressions such as
#v+
      a[0].city_name = "Boston";
      a[0].population = 2500000;
#v-
   The new type \var{Population_Type} may also be used with the
   \var{typeof} function:
#v+
      if (Population_Type == typeof (a))
        city = a.city_name;
#v-
   The dereference \var{@} may be used to create an instance of the
   new type:
#v+
     a = @Population_Type;
     a.city_name = "Calcutta";
     a.population = 13000000;
#v-

   Another feature that user-defined types possess is that the action
   of the binary and unary operations may be defined for them.
   This idea is discussed in more detail below.

#%}}}

\sect{Operator Overloading}

 The binary and unary operators may be extended to user-defined types.
 To illustrate how this works, consider a data type that represents a
 vector in 3-space:
#v+
    typedef struct { x, y, z } Vector_Type;
#v-
 and a function that instantiates such an object:
#v+
    define vector_new (x, y, z)
    {
       variable v = @Vector_Type;
       v.x = double(x); v.y = double(y); v.z = double(z);
       return v;
    }
#v-
 This function may be used to define a function that adds two vectors
 together:
#v+
    define vector_add (v1, v2)
    {
       return vector_new (v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
    }
#v-
 Using these functions, three vectors representing the points
 \exmp{(2,3,4)}, \exmp{(6,2,1)}, and \exmp{(-3,1,-6)} may be created using
#v+
   V1 = vector_new (2,3,4);
   V2 = vector_new (6,2,1);
   V3 = vector_new (-3,1,-6);
#v-
 and then added together via
#v+
   V4 = vector_add (V1, vector_add (V2, V3));
#v-
 The problem with the last statement is that it is not a very natural
 way to express the addition of three vectors.  It would be far better
 to extend the action of the binary \exmp{+} operator to the
 \exmp{Vector_Type} objects and then write the above sum more simply as
#v+
   V4 = V1 + V2 + V3;
#v-

 The \ifun{__add_binary} function defines the result of a binary
 operation between two data types:
\begin{tscreen}
   __add_binary (\em{op}, \em{result-type}, \em{funct}, \em{typeA},\em{typeB});
\end{tscreen}
 Here, \em{op} is a string representing any one of the binary operators
 (\exmp{"+"}, \exmp{"-"}, \exmp{"*"}, \exmp{"/"}, \exmp{"=="},...),
 and \em{funct} is reference to a function that carries out the binary
 operation between objects of types \em{typeA} and \em{typeB} to
 produce an object of type \em{result-type}.

 This function may be used to extend the \exmp{+} operator to
 \em{Vector_Type} objects:
#v+
    __add_binary ("+", Vector_Type, &vector_add, Vector_Type, Vector_Type);
#v-
 Similarly the subtraction and equality operators may be extended to
 \exmp{Vector_Type} via
#v+
    define vector_minus (v1, v2)
    {
       return vector_new (v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
    }
    __add_binary ("-", Vector_Type, &vector_minus, Vector_Type, Vector_Type);

    define vector_eqs (v1, v2)
    {
       return (v1.x==v2.x) and (v1.y==v2.y) and (v1.z==v2.z);
    }
    __add_binary ("==", Char_Type, &vector_eqs, Vector_Type, Vector_Type);
#v-
 permitting a statement such as
#v+
    if (V2 != V1) V3 = V2 - V1;
#v-
 The \exmp{-} operator is also an unary operator that is customarily
 used to change the sign of an object.  Unary operations may be
 extended to \exmp{Vector_Type} objects using the \ifun{__add_unary}
 function:
#v+
   define vector_chs (v)
   {
      return vector_new (-v.x, -v.y, -v.z);
   }
   __add_unary ("-", Vector_Type, &vector_chs, Vector_Type);
#v-
 A trivial example of the use of the unary minus is \exmp{V4 = -V2}.

 It is interesting to consider the extension of the multiplication
 operator \exmp{*} to \exmp{Vector_Type}.  A vector may be multiplied
 by a scalar to produce another vector.  This can happen in two ways as
 reflected by the following functions:
#v+
   define vector_scalar_mul (v, a)
   {
      return vector_new (a*v.x, a*v.y, a*v.z);
   }
   define scalar_vector_mul (a, v)
   {
      return vector_new (a*v.x, a*v.y, a*v.z);
   }
#v-
 Here \exmp{a} represents the scalar, which can be any object that may
 be multiplied by a \dtype{Double_Type}, e.g., \dtype{Int_Type},
 \dtype{Float_Type}, etc.  Instead of using multiple statements
 involving \ifun{__add_binary} to define the action of
 \exmp{Int_Type+Vector_Type}, \exmp{Float_Type+Vector_Type}, etc, a
 single statement using \var{Any_Type} to represent a ``wildcard''
 type may be used:
#v+
   __add_binary ("*", Vector_Type, &vector_scalar_mul, Vector_Type, Any_Type);
   __add_binary ("*", Vector_Type, &scalar_vector_mul, Any_Type, Vector_Type);
#v-
 There are a couple of natural possibilities for
 \exmp{Vector_Type*Vector_Type}: The cross-product defined by
#v+
   define crossprod (v1, v2)
   {
      return vector_new (v1.y*v2.z-v1.z*v2.y,
                         v1.z*v2.x-v1.x*v2.z,
                         v1.x*v2.y-v1.y*v2.x);
   }
#v-
 and the dot-product:
#v+
   define dotprod (v1, v2)
   {
      return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
   }
#v-
 The binary \exmp{*} operator between two vector types may be defined
 to be just one of these functions--- it cannot be extended to both.
 If the dot-product is chosen then one would use
#v+
   __add_binary ("*", Double_Type, &dotprod, Vector_Type_Type, Vector_Type);
#v-

 Just because it is possible to define the action of a binary or unary
 operator on an user-defined type, it is not always wise to do so.  A
 useful rule of thumb is to ask whether defining a particular
 operation leads to more readable and maintainable code.  For example,
 simply looking at
#v+
   c = a + b;
#v-
 in isolation one can easily overlook the fact that a function such as
 \exmp{vector_add} may be getting executed.  Moreover, in cases where
 the action is ambiguous such as \exmp{Vector_Type*Vector_Type} it may
 not be clear what
#v+
   c = a*b;
#v-
 means unless one knows exactly what choice was made when extending
 the \exmp{*} operator to the types.  For this reason it may
 be wise to leave \exmp{Vector_Type*Vector_Type} undefined and use
 ``old-fashioned'' function calls such as
#v+
   c = dotprod (a, b);
   d = crossprod (a, b);
#v-
 to avoid the ambiguity altogether.

 Finally, the \ifun{__add_string} function may be used to define the
 string representation of an object.  Examples involving the string
 representation include:
#v+
    message ("The value is " + string (V));
    vmessage ("The result of %S+%S is %S", V1, V1, V1+V2);
    str = "The value of V is $V"$;
#v-
 For the \exmp{Vector_Type} one might want to use the string
 represention generated by
#v+
   define vector_string (v)
   {
      return sprintf ("(%S,%S,%S)", v.x, v.y, v.z);
   }
   __add_string (Vector_Type, &vector_string);
#v-

#%}}}

\chapter{Lists} #%{{{

 Sometimes it is desirable to utilize an object that has many of the
 properties of an array, but can also easily grow or shrink upon
 demand.  The \dtype{List_Type} object has such properties.

 An empty list may be created either by the \ifun{list_new} function
 or more simply using curly braces, e.g.,
#v+
    list = {};
#v-
 More generally a list of objects may be created by simply enclosing
 them in braces.  For example,
#v+
   list = { "hello", 7, 3.14, {&sin, &cos}}
#v-
 specifies a list of 4 elements, where the last element is also a list.
 The number of items in a list may be obtained using the \ifun{length}
 function.  For the above list, \exmp{length(list)} will return 4.

 One may examine the contents of the list using an array index
 notation.  For the above example, \exmp{list[0]} refers to the zeroth
 element of the list (\exmp{"hello"} in this case).  Similarly,
#v+
    list[1] = [1,2,3];
#v-
 changes the first element of the list (7) to the array \exmp{[1,2,3]}.
 Also as the case for arrays one may index from the end of the list
 using negative indices, e.g., \exmp{list[-1]} refers to the last
 element of the list.

 The functions \ifun{list_insert} and \ifun{list_append} may be used
 to add items to a list.  In particular,
 \exmp{list_insert(list,obj,nth)} will insert the object \exmp{obj}
 into the list at the \exmp{nth} position.  Similarly,
 \exmp{list_append(list,obj,nth)} will insert the object \exmp{obj}
 into the list right after \exmp{nth} position.  If
#v+
   list = { "hello", 7, 3.14, {&sin, &cos}}
#v-
 then
#v+
   list_insert (list, 0, "hi");
   list_append (list, 0, "there");
   list_insert (list, -1, "before");
   list_append (list, -1, "after");
#v-
 will result in the list
#v+
   {"hi", "there", "hello", 7, 3.14, "before", {&sin,&cos}, "after"}
#v-

 One might be tempted to use
#v+
   list = {"hi", list};
#v-
 to insert \exmp{"hi"} at the head of the list.  However, this simply
 creates a new list of two items: \exmp{hi} and the original list.

 Items may be removed from a list via the \exmp{list_delete} function,
 which deletes the item from the specified position and shrinks the
 list.  In the context of the above example,
#v+
   list_delete (list, 2);
#v-
 will shrink the list to
#v+
   {"hi", "there", 7, 3.14, "before", {&sin,&cos}, "after"}
#v-

 Another way of removing items from the list is to use the
 \ifun{list_pop} function.  The main difference between it and
 \ifun{list_delete} is that \ifun{list_pop} returns the deleted item.
 For example,
#v+
   item = list_pop (list, -2);
#v-
 would reduce the list to
#v+
   {"hi", "there", 7, 3.14, "before", "after"}
#v-
 and assign \exmp{{&sin,&cos}} to \exmp{item}.  If the position
 parameter to \ifun{list_pop} is left unspecified, then the position
 will default to the zeroth, i.e., \exmp{list_pop(list)} is
 equaivalent to \exmp{list_pop(list,0)}.

 To copy a list, use the dereference operator \var{@}:
#v+
   new_list = @list;
#v-
 Keep in mind that this does not perform a so-called deep copy.  If
 any of the elements of the list are objects that are assigned by
 reference, only the references will be copied.

 The \ifun{list_reverse} function may be used to reverse the
 elements of a list.  Note that this does not create a new list.  To
 create new list that is the reverse of another, copy the original
 using the dereference operator (@) and reverse that, i.e.,
#v+
    new_list = list_reverse (@list);
#v-

#%}}}

\chapter{Error Handling} #%{{{

   All non-trivial programs or scripts must be deal with the
   possibility of run-time errors.  In fact, one sign of a seasoned
   programmer is that such a person pays particular attention to error
   handling.  This chapter presents some techniques for handling
   errors using \slang.  First the traditional method of using return
   values to indicate errors will be discussed.  Then attention will
   turn to \slang's more powerful exception handling mechanisms.

\sect{Traditional Error Handling} #%{{{
   The simplist and perhaps most common mechanism for signaling a
   failure or error in a function is for the function to return an
   error code, e.g.,
#v+
    define write_to_file (file, str)
    {
       variable fp = fopen (file, "w");
       if (fp == NULL)
         return -1;
       if (-1 == fputs (str, fp))
         return -1;
       if (-1 == fclose (fp))
         return -1;
       return 0;
    }
#v-
  Here, the \exmp{write_to_file} function returns 0 if successful, or
  -1 upon failure.  It is up to the calling routine to check the
  return value of \exmp{write_to_file} and act accordingly.  For
  instance:
#v+
     if (-1 == write_to_file ("/tmp/foo", "bar"))
       {
          () = fprintf (stderr, "Write failed\n");
          exit (1);
       }
#v-

  The main advantage of this technique is in its simplicity.  The
  weakness in this approach is that the return value must be checked
  for every function that returns information in this way.  A more
  subtle problem is that even minor changes to large programs can
  become unwieldy.  To illustrate the latter aspect, consider the
  following function which is supposed to be so simple that it cannot
  fail:
#v+
     define simple_function ()
     {
         do_something_simple ();
         more_simple_stuff ();
     }
#v-
  Since the functions called by \exmp{simple_function} are not
  supposed to fail, \exmp{simple_function} itself cannot fail and
  there is no return value for its callers to check:
#v+
     define simple ()
     {
         simple_function ();
         another_simple_function ();
     }
#v-
  Now suppose that the function \exmp{do_something_simple} is changed
  in some way that could cause it to fail from time to time.  Such a
  change could be the result of a bug-fix or some feature enhancement.
  In the traditional error handling approach, the function would need
  to be modified to return an error code.  That error code would have
  to be checked by the calling routine \exmp{simple_function} and as a
  result, it can now fail and must return an error code.  The obvious
  effect is that a tiny change in one function can be felt up the
  entire call chain.  While making the appropriate changes for a small
  program can be a trivial task, for a large program this could be a
  major undertaking opening the possibility of introducing additional
  errors along the way.  In a nutshell, this is a code maintenance
  issue.  For this reason, a veteran programmer using this approach to
  error handling will consider such possibilities from the outset and
  allow for error codes the first time regardless of whether the
  functions can fail or not, e.g.,
#v+
     define simple_function ()
     {
         if (-1 == do_something_simple ())
           return -1;
         if (-1 == more_simple_stuff ())
           return -1;
         return 0;
     }
     define simple ()
     {
         if (-1 == simple_function ())
           return -1;
         if (-1 == another_simple_function ())
           return -1;
         return 0;
     }
#v-

  Although latter code containing explicit checks for failure is more
  robust and more easily maintainable than the former, it is also less
  readable.  Moreover, since return values are now checked the code
  will execute somewhat slower than the code that lacks such checks.
  There is also no clean separation of the error handling code from
  the other code.  This can make it difficult to maintain if the error
  handling semantics of a function change. The next section discusses
  another approach to error handling that tries to address these
  issues.

#%}}}

\sect{Error Handling through Exceptions}

  This section describes \slang's exception model.
  The idea is that when a function encounters an error,
  instead of returning an error code, it simply gives up and
  \em{throws} an exception.  This idea will be fleshed out in
  what follows.

\sect1{Introduction to Exceptions} #%{{{

  Consider the \exmp{write_to_file} function used in the previous
  section but adapted to throw an exception:
#v+
    define write_to_file (file, str)
    {
       variable fp = fopen (file, "w");
       if (fp == NULL)
         throw OpenError;
       if (-1 == fputs (str, fp))
         throw WriteError;
       if (-1 == fclose (fp))
         throw WriteError;
    }
#v-
  Here the \kw{throw} statement has been used to generate the
  appropriate exception, which in this case is either an
  \exmp{OpenError} exception or a \var{WriteError} exception.  Since
  the function now returns nothing (no error code), it may be called as
#v+
     write_to_file ("/tmp/foo", "bar");
     next_statement;
#v-
  As long as the \exmp{write_to_file} function encounters no errors,
  control passes from \exmp{write_to_file} to \exmp{next_statement}.

  Now consider what happens when the function encounters an error. For
  concreteness assume that the \ifun{fopen} function failed causing
  \exmp{write_to_file} to throw the \var{OpenError} exception. The
  \exmp{write_to_file} function will stop execution after executing
  the \exmp{throw} statement and return to its caller.  Since no
  provision has been made to handle the exception,
  \exmp{next_statement} will not execute and control will pass to the
  previous caller on the call stack.  This process will continue until
  the exception is either handled or until control reaches the
  top-level at which point the interpreter will terminate. This
  process is known as \em{unwinding} of the call stack.

  An simple exception handler may be created through the use of a
  \em{try-catch} statement, such as
#v+
     try
      {
        write_to_file ("/tmp/foo", "bar");
      }
     catch OpenError:
      {
         message ("*** Warning: failed to open /tmp/foo.");
      }
     next_statement;
#v-
  The above code works as follows: First the statement (or statements)
  inside the try-block are executed.  As long as no exception occurs,
  once they have executed, control will pass on to \exmp{next_statement},
  skipping the catch statement(s).

  If an exception occurs while executing the statements in the
  try-block, any remaining statements in the block will be skipped and
  control will pass to the ``catch'' portion of the exception handler.
  This may consist of one or more \kw{catch} statements and an optional
  \em{finally} statement.  Each \kw{catch} statement specifies a list
  of exceptions it will handle as well as the code that is to be
  excecuted when a matching exception is caught.  If a matching \kw{catch}
  statement is found for the exception, the exception will be cleared
  and the code associated with the catch statement will get executed.
  Control will then pass to \exmp{next_statement} (or first to the
  code in an optional \kw{finally} block).

  Catch-statements are tested against the exception in the order that
  they appear.  Once a matching \kw{catch} statement is found, the
  search will terminate.  If no matching \kw{catch}-statement is
  found, an optional \kw{finally} block will be processed, and the
  call-stack will continue to unwind until either a matching exception
  handler is found or the interpreter terminates.

  In the above example, an exception handler was established for the
  \exmp{OpenError} exception.  The error handling code for this exception will
  cause a warning message to be displayed.  Execution will resume at
  \exmp{next_statement}.

  Now suppose that \exmp{write_to_file} successfully opened the file,
  but that for some reason, e.g., a full disk, the actual write
  operation failed.  In such a case, \exmp{write_to_file} will throw a
  \exmp{WriteError} exception passing control to the caller.  The file
  will remain on the disk but not fully written.  An exception handler can
  be added for \exmp{WriteError} that removes the file:

#v+
     try
      {
        write_to_file ("/tmp/foo", "bar");
      }
     catch OpenError:
      {
         message ("*** Warning: failed to open /tmp/foo.");
      }
     catch WriteError:
      {
         () = remove ("/tmp/foo");
         message ("*** Warning: failed to write to /tmp/foo");
      }
     next_statement;
#v-
  Here the exception handler for \exmp{WriteError} uses the
  \ifun{remove} intrinsic function to delete the file and then issues a warning
  message.  Note that the \ifun{remove} intrinsic uses the traditional
  error handling mechanism--- in the above example its return status
  has been discarded.

  Above it was assumed that failure to write to the file was not
  critical allowing a warning message to suffice upon failure.  Now
  suppose that it is important for the file to be written but that it
  is still desirable for the file to be removed upon failure.  In this
  scenario, \exmp{next_statement} should not get executed upon
  failure.  This can be achieved as follows:
#v+
     try
      {
        write_to_file ("/tmp/foo", "bar");
      }
     catch WriteError:
      {
         () = remove ("/tmp/foo");
         throw WriteError;
      }
     next_statement;
#v-
  Here the exception handler for \exmp{WriteError} removes the file
  and then re-throws the exception.

#%}}}

\sect1{Obtaining information about the exception}
  When an exception is generated, an exception object is thrown.  The
  object is a structure containing the following fields:
\begin{descrip}
  \tag{error}
     The exception error code (\dtype{Int_Type}).
  \tag{descr}
     A brief description of the error (\dtype{String_Type}).
  \tag{file}
     The filename containing the code that generated the exception
     (\dtype{String_Type}).
  \tag{line}
     The line number where the exception was thrown
     (\dtype{Int_Type}).
  \tag{function}
     The name of the currently executing function, or \NULL if at top-level
     (\dtype{String_Type}).
  \tag{message}
     A text message that may provide more information about the exception
     (\dtype{String_Type}).
  \tag{object}
     A user-defined object.
\end{descrip}

  If it is desired to have information about the exception, then
  an alternative form of the \var{try} statement must be used:
#v+
     try (e)
     {
        % try-block code
     }
     catch SomeException: { code ... }
#v-
  If an exception occurs while executing the code in the try-block,
  then the variable \exmp{e} will be assigned the value of the
  exception object.  As a simple example, suppose that the file
  \tt{foo.sl} consists of:
#v+
     define invert_x (x)
     {
        if (x == 0)
          throw DivideByZeroError;
        return 1/x;
     }
#v-
  and that the code is called using
#v+
     try (e)
     {
        y = invert_x (0);
     }
     catch DivideByZeroError:
     {
        vmessage ("Caught %s, generated by %s:%d\n",
                  e.descr, e.file, e.line);
        vmessage ("message: %s\nobject: %S\n",
                  e.message, e.object);
        y = 0;
     }
#v-
  When this code is executed, it will generate the message:
#v+
     Caught Divide by Zero, generated by foo.sl:5
     message: Divide by Zero
     object: NULL
#v-
  In this case, the value of the \exmp{message} field was assigned a
  default value.  The reason that the \exmp{object} field is \NULL is
  that no object was specified when the exception was generated.
  In order to throw an object, a more complex form of \kw{throw}
  statement must be used:
\begin{tscreen}
    \kw{throw} \em{exception-name} [, \em{message} [, \em{object} ] ]
\end{tscreen}
  where the square brackets indicate optional parameters

  To illustrate this form, suppose that \exmp{invert_x} is modified to
  accept an array object:
#v+
    private define invert_x(x)
    {
       variable i = where (x == 0);
       if (length (i))
         throw DivideByZeroError,
               "Array contains elements that are zero", i;
       return 1/x;
    }
#v-
  In this case, the message field of the exception object will contain
  the string \exmp{"Array contains elements that are zero"} and the
  object field will be set to the indices of the zero elements.

\sect1{The finally block}
  The full form of the try-catch statement obeys the following syntax:
\begin{tscreen}
    try \em{[(opt-e)]}
     \{
       \em{try-block-statements}
     }
    catch \em{Exception-List-1}: { \em{catch-block-1-statements} }
      .
      .
    catch \em{Exception-List-N}: { \em{catch-block-N-statements} }
    \em{[} finally { \em{finally-block-statements} } \em{]}
\end{tscreen}
  Here an exception-list is simply a list of exceptions such as:
#v+
    catch OSError, RunTimeError:
#v-
  The last clause of a try-statement is the \em{finally-block}, which is
  optional and is introduced using the \kw{finally} keyword.  If the
  try-statement contains no catch-clauses, then it must specify a
  finally-clause, otherwise a syntax error will result.

  If the finally-clause is present, then its corresponding statements
  will be executed \bf{regardless of whether an exception occurs}.  If
  an exception occurs while executing the statements in the try-block,
  then the finally-block will execute after the code in any of the
  catch-blocks.  The finally-clause is useful for freeing any
  resources (file handles, etc) allocated by the try-block regardless
  of whether an exception has occurred.

\sect1{Creating new exceptions: the Exception Hierarchy} #%{{{

   The following table gives the class hierarchy for the built-in
   exceptions.
#v+
   AnyError
      OSError
         MallocError
         ImportError
      ParseError
         SyntaxError
         DuplicateDefinitionError
         UndefinedNameError
      RunTimeError
         InvalidParmError
         TypeMismatchError
         UserBreakError
         StackError
            StackOverflowError
            StackUnderflowError
         ReadOnlyError
         VariableUnitializedError
         NumArgsError
         IndexError
         UsageError
         ApplicationError
         InternalError
         NotImplementedError
         LimitExceededError
         MathError
            DivideByZeroError
            ArithOverflowError
            ArithUnderflowError
            DomainError
         IOError
            WriteError
            ReadError
            OpenError
         DataError
         UnicodeError
         InvalidUTF8Error
         UnknownError
#v-

 The above table shows that the root class of all exceptions is
 \var{AnyError}.  This means that a catch block for \var{AnyError}
 will catch any exception.  The \var{OSError}, \var{ParseError}, and
 \var{RunTimeError} exceptions are subclasses of the \var{AnyError}
 class.  Subclasses of \var{OSError} include \var{MallocError},
 and \var{ImportError}.  Hence a handler for the
 \var{OSError} exception will catch \var{MallocError} but not
 \var{ParseError} since the latter is not a subclass of
 \var{OSError}.

 The user may extend this tree with new exceptions using the
 \ifun{new_exception} function.  This function takes three arguments:
\begin{tscreen}
   new_exception (\em{exception-name}, \em{baseclass}, \em{description});
\end{tscreen}
 The \em{exception-name} is the name of the exception, \em{baseclass}
 represents the node in the exception hierarchy where it is to be
 placed, and \em{description} is a string that provides a brief
 description of the exception.

 For example, suppose that you are writing some code that processes
 numbers stored in a binary format.  In particular, assume that the
 format specifies that data be stored in a specific byte-order, e.g.,
 in big-endian form.  Then it might be useful to extend the
 \var{DataError} exception with \var{EndianError}.  This is easily
 accomplished via
#v+
   new_exception ("EndianError", DataError, "Invalid byte-ordering");
#v-
 This will create a new exception object called \var{EndianError}
 subclassed on \var{DataError}, and code that catches the \var{DataError}
 exception will additionally catch the \var{EndianError} exception.

#%}}}

#%}}}

\chapter{Loading Files: evalfile, autoload, and require}

\labeled_chapter{Modules} #%{{{

\sect{Introduction}
  A module is a shared object that may be dynamically linked into the
  interpreter at run-time to provide the interpreter with additional
  intrinsic functions and variables.  Several modules are distributed
  with the stock version of the \slang library, including a
  \module{pcre} module that allows the interpreter to make use of the
  \em{Perl Compatible Regular Expression library}, a \module{png}
  module that allows the interpreter to easily read and write PNG
  files, and a \module{rand} module for producing random numbers.
  There are also a number of modules for the interpreter that are not
  distributed with the library.  See
  \url{http://www.jedsoft.org/slang/modules/} for links to some of
  those.

\sect{Using Modules}

  In order to make use of a module, it must first be ``imported'' into
  the interpreter.  There are two ways to go about this.  One is to
  use the \ifun{import} function to dynamically link-in the specified
  module, e.g.,
#v+
    import ("pcre");
#v-
  will dynamically link to the \module{pcre} module and make its
  symbols available to the interpreter using the active namespace.
  However, this is not the preferred method for loading a module.

  Module writers are encouraged to distribute a module with a file of
  \slang code that performs the actual import of the module. Rather
  than a user making direct use of the \ifun{import} function, the
  preferred method of loading the modules is to load that file
  instead.  For example the \module{pcre} module is distributed with a
  file called \file{pcre.sl} that contains little more than the
  \exmp{import("pcre")} statement.  To use the \module{pcre} module,
  load \file{pcre.sl}, e.g.,
#v+
    require ("pcre");
#v-

  The main advantage of this approach to loading a module is that the
  functionality provided by the module may be split between intrinsic
  functions in the module proper, and interpreted functions contained
  in the \exmp{.sl} file.  In such a case, loading the module via
  \ifun{import} would only provide partial functionality.  The
  \module{png} module provides a simple example of this concept.  The
  current version of the \exmp{png} module consists of a couple intrinsic
  functions (\ifun{png_read} and \ifun{png_write}) contained in the
  shared object (\file{png-module.so}), and a number of other
  interpreted \slang functions defined in \file{png.sl}.  Using the
  \ifun{import} statement to load the module would miss the latter set
  of functions.

  In some cases, the symbols in a module may conflict with symbols
  that are currently defined by the interpreter.  In order to avoid
  the conflict, it may be necessary to load the module into its own
  namespace and access its symbols via the namespace prefix.  For
  example, the GNU Scientific Library Special Function module,
  \module{gslsf}, defines a couple hundred functions, some with common
  names such as \exmp{zeta}.  In order to avoid any conflict, it is
  recommended that the symbols from such a module be imported into a
  separate namespace.  This may be accomplished by specifying the
  namespace as a second argument to the \exmp{require} function, e.g.,
#v+
    require ("gslsf", "gsl");
       .
       .
    y = gsl->zeta(x);
#v-
  This form requires that the module's symbols be accessed via the
  namespace qualifier \exmp{"gsl->"}.

#%}}}

\chapter{File Input/Output} #%{{{

 \slang provides built-in support for two different I/O facilities.
 The simplest interface is modeled upon the C language \em{stdio}
 interface and consists of functions such as \ifun{fopen},
 \ifun{fgets}, etc.  The other interface is modeled on a lower level
 POSIX interface consisting of functions such as \ifun{open},
 \ifun{read}, etc.  In addition to permitting more control, the lower
 level interface permits one to access network objects as well as disk
 files.

 For reading data formatted in text files, e.g., columns of numbers,
 then do not overlook the high-level routines in the \slsh library. In
 particular, the \sfun{readascii} function is quite flexible and can
 read data from text files that are formatted in a variety of ways.
 For data stored in a standard binary format such as HDF or FITS, then
 the corresponding modules should be used.

\sect{Input/Output via stdio}
\sect1{Stdio Overview}
 The \em{stdio} interface consists of the following functions:
\begin{itemize}
\item \ifun{fopen}: opens a file for reading or writing.

\item \ifun{fclose}: closes a file opened by \ifun{fopen}.

\item \ifun{fgets}: reads a line from a file.

\item \ifun{fputs}: writes text to a file.

\item \var{fprintf}: writes formatted text to a file.

\item \ifun{fwrite}: writes one of more objects to a file.

\item \ifun{fread}: reads a specified number of objects from
       a file.

\item \ifun{fread_bytes}: reads a specified number of bytes from a
 file and returns them as a string.

\item \ifun{feof}: tests if a file pointer is at the
       end of the file.

\item \ifun{ferror}: tests whether or not the stream
       associated with a file has an error.

\item \ifun{clearerr}: clears the end-of-file and error
       indicators for a stream.

\item \ifun{fflush}, forces all buffered data associated with
       a stream to be written out.

\item \ifun{ftell}: queries the file position indicator
       a the stream.

\item \ifun{fseek}: sets the position of a file
      position indicator of the stream.

\item \ifun{fgetslines}: reads all the lines from a text file and
      returns them as an array of strings.

\end{itemize}

 In addition, the interface supports the \ifun{popen} and \ifun{pclose}
 functions on systems where the corresponding C functions are available.

 Before reading or writing to a file, it must first be opened using
 the \ifun{fopen} function.  The only exceptions to this rule involve
 use of the pre-opened streams: \ivar{stdin}, \ivar{stdout}, and
 \ivar{stderr}.  \ifun{fopen} accepts two arguments: a file name and a
 string argument that indicates how the file is to be opened, e.g.,
 for reading, writing, update, etc.  It returns a \var{File_Type}
 stream object that is used as an argument to all other functions of
 the \em{stdio} interface.  Upon failure, it returns \NULL.  See the
 reference manual for more information about \ifun{fopen}.

\sect1{Stdio Examples}

 In this section, some simple examples of the use of the \em{stdio}
 interface is presented.  It is important to realize that all the
 functions of the interface return something, and that return value
 must be handled in some way by the caller.

 The first example involves writing a function to count the number of
 lines in a text file.  To do this, we shall read in the lines, one by
 one, and count them:
#v+
    define count_lines_in_file (file)
    {
       variable fp, line, count;

       fp = fopen (file, "r");    % Open the file for reading
       if (fp == NULL)
         throw OpenError, "$file failed to open"$;

       count = 0;
       while (-1 != fgets (&line, fp))
         count++;

       () = fclose (fp);
       return count;
    }
#v-
 Note that \exmp{&line} was passed to the \ifun{fgets} function.  When
 \ifun{fgets} returns, \var{line} will contain the line of text read in
 from the file.  Also note how the return value from \ifun{fclose} was
 handled (discarded in this case).

 Although the preceding example closed the file via \ifun{fclose},
 there is no need to explicitly close a file because the interpreter will
 automatically close a file when it is no longer referenced.  Since
 the only variable to reference the file is \var{fp}, it would have
 automatically been closed when the function returned.

 Suppose that it is desired to count the number of characters in the
 file instead of the number of lines.  To do this, the \var{while}
 loop could be modified to count the characters as follows:
#v+
      while (-1 != fgets (&line, fp))
        count += strlen (line);
#v-
 The main difficulty with this approach is that it will not work for
 binary files, i.e., files that contain null characters.  For such
 files, the file should be opened in \em{binary} mode via
#v+
      fp = fopen (file, "rb");
#v-
 and then the data read using the \ifun{fread} function:
#v+
      while (-1 != fread (&line, Char_Type, 1024, fp))
           count += length (line);
#v-
 The \ifun{fread} function requires two additional arguments: the type
 of object to read (\var{Char_Type} in the case), and the number of
 such objects to be read.  The function returns the number of objects
 actually read in the form of an array of the specified type, or -1
 upon failure.

 Sometimes it is more convenient to obtain the data from a file in the form
 of a character string instead of an array of characters.  The
 \exmp{fread_bytes} function may be used in such situations.  Using
 this function, the equivalent of the above loop is
#v+
      while (-1 != fread_bytes (&line, 1024, fp))
           count += bstrlen (line);
#v-

 The \kw{foreach} construct also works with \var{File_Type} objects.
 For example, the number of characters in a file may be counted via
#v+
     foreach ch (fp) using ("char")
       count++;
#v-
 Similarly, one can count the number of lines using:
#v+
     foreach line (fp) using ("line")
      {
         num_lines++;
         count += strlen (line);
      }
#v-
 Often one is not interested in trailing whitespace in the lines of a
 file.   To have trailing whitespace automatically stripped from the
 lines as they are read in, use the \exmp{"wsline"} form, e.g.,
#v+
     foreach line (fp) using ("wsline")
      {
          .
          .
      }
#v-

 Finally, it should be mentioned that none of these examples should
 be used to count the number of bytes in a file when that
 information is more readily accessible by another means.  For
 example, it is preferable to get this information via the
 \ifun{stat_file} function:
#v+
     define count_chars_in_file (file)
     {
        variable st;

        st = stat_file (file);
        if (st == NULL)
          throw IOError, "stat_file failed";
        return st.st_size;
     }
#v-

\sect{POSIX I/O}

\sect{Advanced I/O techniques}

  The previous examples illustrate how to read and write objects of a
  single data-type from a file, e.g.,
#v+
      num = fread (&a, Double_Type, 20, fp);
#v-
  would result in a \exmp{Double_Type[num]} array being assigned to
  \exmp{a} if successful.  However, suppose that the binary data file
  consists of numbers in a specified byte-order.  How can one read
  such objects with the proper byte swapping?  The answer is to use
  the \ifun{fread_bytes} function to read the objects as a (binary)
  character string and then \em{unpack} the resulting string into the
  specified data type, or types.  This process is facilitated using
  the \ifun{pack} and \ifun{unpack} functions.

  The \ifun{pack} function follows the syntax
\begin{tscreen}
    BString_Type pack (\em{format-string}, \em{item-list});
\end{tscreen}
  and combines the objects in the \em{item-list} according to
  \em{format-string} into a binary string and returns the result.
  Likewise, the \ifun{unpack} function may be used to convert a binary
  string into separate data objects:
\begin{tscreen}
   (\em{variable-list}) = unpack (\em{format-string}, \em{binary-string});
\end{tscreen}

  The format string consists of one or more data-type specification
  characters, and each may be followed by an optional decimal length
  specifier. Specifically, the data-types are specified according to
  the following table:
#v+
     c     char
     C     unsigned char
     h     short
     H     unsigned short
     i     int
     I     unsigned int
     l     long
     L     unsigned long
     j     16 bit int
     J     16 unsigned int
     k     32 bit int
     K     32 bit unsigned int
     f     float
     d     double
     F     32 bit float
     D     64 bit float
     s     character string, null padded
     S     character string, space padded
     z     character string, null padded
     x     a null pad character
#v-
  A decimal length specifier may follow the data-type specifier. With
  the exception of the \exmp{s} and \exmp{S} specifiers, the length
  specifier indicates how many objects of that data type are to be
  packed or unpacked from the string.  When used with the \exmp{s} or
  \exmp{S} specifiers, it indicates the field width to be used.  If the
  length specifier is not present, the length defaults to one.

  With the exception of \exmp{c}, \exmp{C}, \exmp{s}, \exmp{S}, \exmp{z}, and
  \exmp{x}, each of these may be prefixed by a character that indicates
  the byte-order of the object:
#v+
     >    big-endian order (network order)
     <    little-endian order
     =    native byte-order
#v-
  The default is to use the native byte order.

  Here are a few examples that should make this more clear:
#v+
     a = pack ("cc", 'A', 'B');         % ==> a = "AB";
     a = pack ("c2", 'A', 'B');         % ==> a = "AB";
     a = pack ("xxcxxc", 'A', 'B');     % ==> a = "\0\0A\0\0B";
     a = pack ("h2", 'A', 'B');         % ==> a = "\0A\0B" or "\0B\0A"
     a = pack (">h2", 'A', 'B');        % ==> a = "\0\xA\0\xB"
     a = pack ("<h2", 'A', 'B');        % ==> a = "\0B\0A"
     a = pack ("s4", "AB", "CD");       % ==> a = "AB\0\0"
     a = pack ("s4s2", "AB", "CD");     % ==> a = "AB\0\0CD"
     a = pack ("S4", "AB", "CD");       % ==> a = "AB  "
     a = pack ("S4S2", "AB", "CD");     % ==> a = "AB  CD"
#v-

  When unpacking, if the length specifier is greater than one, then an
  array of that length will be returned.  In addition, trailing
  whitespace and null characters are stripped when unpacking an object
  given by the \exmp{S} specifier.  Here are a few examples:
#v+
    (x,y) = unpack ("cc", "AB");         % ==> x = 'A', y = 'B'
    x = unpack ("c2", "AB");             % ==> x = ['A', 'B']
    x = unpack ("x<H", "\0\xAB\xCD");    % ==> x = 0xCDABuh
    x = unpack ("xxs4", "a b c\0d e f");  % ==> x = "b c\0"
    x = unpack ("xxS4", "a b c\0d e f");  % ==> x = "b c"
#v-

\sect1{Example: Reading /var/log/wtmp}

  Consider the task of reading the Unix system file
  \file{/var/log/utmp}, which contains login records about who logged
  onto the system.  This file format is documented in section 5 of the
  online Unix man pages, and consists of a sequence of entries
  formatted according to the C structure \exmp{utmp} defined in the
  \file{utmp.h} C header file.  The actual details of the structure
  may vary from one version of Unix to the other.  For the purposes of
  this example, consider its definition under the Linux operating
  system running on an Intel 32 bit processor:
#v+
    struct utmp {
       short ut_type;              /* type of login */
       pid_t ut_pid;               /* pid of process */
       char ut_line[12];           /* device name of tty - "/dev/" */
       char ut_id[2];              /* init id or abbrev. ttyname */
       time_t ut_time;             /* login time */
       char ut_user[8];            /* user name */
       char ut_host[16];           /* host name for remote login */
       long ut_addr;               /* IP addr of remote host */
    };
#v-
  On this system, \exmp{pid_t} is defined to be an \exmp{int} and
  \exmp{time_t} is a \exmp{long}.  Hence, a format specifier for the
  \var{pack} and \var{unpack} functions is easily constructed to be:
#v+
     "h i S12 S2 l S8 S16 l"
#v-
  However, this particular definition is naive because it does not
  allow for structure padding performed by the C compiler in order to
  align the data types on suitable word boundaries.  Fortunately, the
  intrinsic function \var{pad_pack_format} may be used to modify a
  format by adding the correct amount of padding in the right places.
  In fact, \var{pad_pack_format} applied to the above format on an
  Intel-based Linux system produces the result:
#v+
     "h x2 i S12 S2 x2 l S8 S16 l"
#v-
  Here we see that 4 bytes of padding were added.

  The other missing piece of information is the size of the structure.
  This is useful because we would like to read in one structure at a
  time using the \ifun{fread} function.  Knowing the size of the
  various data types makes this easy; however it is even easier to use
  the \ifun{sizeof_pack} intrinsic function, which returns the size (in
  bytes) of the structure described by the pack format.

  So, with all the pieces in place, it is rather straightforward to
  write the code:
#v+
    variable format, size, fp, buf;

    typedef struct
    {
       ut_type, ut_pid, ut_line, ut_id,
       ut_time, ut_user, ut_host, ut_addr
    } UTMP_Type;

    format = pad_pack_format ("h i S12 S2 l S8 S16 l");
    size = sizeof_pack (format);

    define print_utmp (u)
    {

      () = fprintf (stdout, "%-16s %-12s %-16s %s\n",
                    u.ut_user, u.ut_line, u.ut_host, ctime (u.ut_time));
    }

   fp = fopen ("/var/log/utmp", "rb");
   if (fp == NULL)
     throw OpenError, "Unable to open utmp file";

   () = fprintf (stdout, "%-16s %-12s %-16s %s\n",
                          "USER", "TTY", "FROM", "LOGIN@");

   variable U = @UTMP_Type;

   while (-1 != fread (&buf, Char_Type, size, fp))
     {
       set_struct_fields (U, unpack (format, buf));
       print_utmp (U);
     }

   () = fclose (fp);
#v-
  A few comments about this example are in order.  First of all, note
  that a new data type called \exmp{UTMP_Type} was created, although
  this was not really necessary.  The file was opened in binary mode,
  but this too was optional because under a Unix system where there is
  no distinction between binary and text modes.  The \exmp{print_utmp}
  function does not print all of the structure fields.  Finally, last
  but not least, the return values from \sfun{fprintf} and \ifun{fclose}
  were handled by discarding them.

#%}}}

\labeled_chapter{slsh}

 \slsh, also known as the S-Lang shell, is an application that is
 included in the stock \slang distribution.  As some binary
 distributions include \slsh  as a separate package it must be
 installed separately, e.g.,
#v+
    apt-get install slsh
#v-
 on Debian Linux systems.  The use of \slsh in its interactive mode
 was discussed briefly in the \ref{Introduction}.  This chapter
 concentrates on the use of \slsh for writing executable \slang
 scripts.

\sect{Running slsh}
 When run the \exmp{--help} command-line argument, \slsh displays a
 brief usage message:
#v+
   # slsh --help
   Usage: slsh [OPTIONS] [-|file [args...]]
    --help           Print this help
    --version        Show slsh version information
    -e string        Execute 'string' as S-Lang code
    -g               Compile with debugging code, tracebacks, etc
    -n               Don't load personal init file
    --init file      Use this file instead of ~/.slshrc
    --no-readline    Do not use readline
    -i               Force interactive input
    -t               Test mode.  If slsh_main exists, do not call it
    -v               Show verbose loading messages
    -Dname           Define "name" as a preprocessor symbol

     Note: - and -i are mutually exclusive

   Default search path: /usr/local/share/slsh
#v-

 When started with no arguments, \slsh will start in interactive mode
 and take input from the terminal.  As the usage message indicates
 \slsh loads a personal initialization file called \file{.slshrc} (on
 non-Unix systems, this file is called \file{slsh.rc}).  The contents
 of this file must be valid \slang code, but are otherwise arbitrary.
 One use of this file is to define commonly used functions and to
 setup personal search paths.

 \slsh will run in non-interactive mode when started with a file (also
 known as a ``script'') as its first (non-option) command-line
 argument.  The rest of the arguments on the command line serve as
 arguments for the script.  The next section deals with the use of the
 \exmp{cmdopt} routines for parsing those arguments.

 \slsh will read the script and feed it to the \slang interpreter for
 execution.  If the script defines a public function called
 \sfun{slsh_main}, then \slsh will call it after the script has been
 loaded.  In this sense, \exmp{slsh_main} is analogous to \exmp{main}
 in \bf{C} or \bf{C++}.

 A typical \slsh script is be structured as
#v+
   #!/usr/bin/env slsh
      .
      .
   define slsh_main ()
   {
      .
      .
   }
#v-
 The first line of the script Unix-specific and should be familiar to
 Unix users.   Typically, the code before \sfun{slsh_main} will load
 any required modules or packages, and define other functions to be
 used by the script.

 Although the use of \sfun{slsh_main} is not required, its use is
 strongly urged for several reasons.  In addition to lending
 uniformity to \slang scripts, \sfun{slsh_main} is well supported by
 the \slang debugger (\sldb) and the \slang profiler (\slprof), which
 look for \sfun{slsh_main} as a starting point for script execution.
 Also as scripts necessarily do something (otherwise they have no
 use), \slsh's \exmp{-t} command-line option may be used to turn off
 the automatic execution of \exmp{slsh_main}.  This allows the syntax
 of the entire script to be checked for errors instead of running it.

\sect{Command line processing}

 The script's command-line arguments are availble to it via the
 \ivar{__argc} and \ivar{__argv} intrinsic variables.  Any optional
 arguments represented by these variables may be parsed using \slsh's
 \bf{cmdopt} routines.

 As a useful illustration, consider the script that the author uses to
 rip tracks from CDs to OGG encoded files.  The name of the script
 is \exmp{cd2ogg.sl}.  Running the script without arguments causes it
 to issue a usage message:
#v+
   Usage: cd2ogg.sl [options] device
   Options:
    --help                 This help
    --work DIR             Use DIR as working dir [/tmp/29848]
    --root DIR             Use DIR/GENRE as root for final output [/data/CDs]
    --genre GENRE          Use GENRE for output dir
    --no-rip               Skip rip stage
    --no-normalize         Skip normalizing stage
    --no-encode            Don't encode to ogg
    --albuminfo PERFORMER/TITLE
                           Use PERFORMER/TITLE if audio.cddb is absent
#v-
 As the message shows, some of the options require an argument while
 others do not.  The cd2ogg.sl script looks like:
#v+
   #!/usr/bin/env slsh
   require ("cmdopt");
      .
      .
   private define exit_usage ()
   {
      () = fprintf (stderr, "Usage: %s [options] device\n",
                    path_basename (__argv[0]));
      () = fprintf (stderr, "Options:\n");
         .
         .
      exit (1);
   }

   private define parse_album_info (albuminfo)
   {
      ...
   }

   define slsh_main ()
   {
      variable genre = NULL;
      variable no_rip = 0;
      variable no_normalize = 0;
      variable no_encode = 0;

      variable opts = cmdopt_new ();
      opts.add ("help", &exit_usage);
      opts.add ("device", &CD_Device; type="str");
      opts.add ("work", &Work_Dir; type="str");
      opts.add ("root", &Root_Dir; type="str");
      opts.add ("genre", &genre; type="str");
      opts.add ("albuminfo", &parse_album_info; type="str");
      opts.add ("no-normalize", &no_normalize);
      opts.add ("no-encode", &no_encode);
      variable i = opts.process (__argv, 1);
      if (i + 1 != __argc)
        exit_usage ();
      CD_Device = __argv[i];
         .
         .
   }
#v-

 There are several points that one should take from the above example.
 First, to use the \exmp{cmdopt} interface it is necessary to load it.
 This is accomplished using the \sfun{require} statement. Second, the
 above example uses \exmp{cmdopt}'s object-oriented style interface
 through the use of the \exmp{add} and \exmp{process} methods of the
 \exmp{cmdopt} object created by the call to \exmp{cmdopt_new}. Third,
 two of the command line options make use of callback functions: the
 \exmp{exit_usage} function will get called when \exmp{--help} appears
 on the command line, and the \exmp{parse_album_info} function will
 get called to handle the \exmp{--albuminfo} option.  Options such as
 \exmp{--no-encode} do not take a value and the presence of such an
 option on the command line causes the variable associated with the
 option to be set to 1.  Other options such as \exmp{--genre} will
 cause the variable associated with them to be set of the value
 specified on the command-line.  Finally, the \exmp{process} method
 returns the index of \ivar{__argv} that corresponds to ``non-option''
 argument.  In this case, for proper usage of the script, that
 argument would correspond to device representing the CD drive.

 For more information about the \exmp{cmdopt} interface, see the
 documentation for \exmp{cmdopt_add}:
#v+
    slsh> help cmdopt_add
#v-

\chapter{Debugging} #%{{{

 There are several ways to debug a \slang script.  When the
 interpreter encounters an uncaught exception, it can generate a
 traceback report showing where the error occurred and the values of
 local variables in the function call stack frames at the time of the
 error.  Often just knowing where the error occurs is all that is
 required to correct the problem.  More subtle bugs may require a
 deeper analysis to diagnose the problem.  While one can insert the
 appropriate print statements in the code to get some idea about what
 is going on, it may be simpler to use the interactive debugger.

\sect{Tracebacks}

 When the value of the \ivar{_traceback} variable is non-zero, the
 interpreter will generate a traceback report when it encounters an
 error.  This variable may be set by putting the line
#v+
    _traceback = 1;
#v-
 at the top of the suspect file.  If the script is running in \slsh,
 then invoking \slsh using the \exmp{-g} option will enable tracebacks:
#v+
    slsh -g myscript.sl
#v-

 If \ivar{_traceback} is set to a positive value, the values of local
 variables will be printed in the traceback report.  If set to a
 negative integer, the values of the local variables will be absent.

 Here is an example of a traceback report:
#v+
    Traceback: error
    ***string***:1:verror:Run-Time Error
    /grandpa/d1/src/jed/lib/search.sl:78:search_generic_search:Run-Time Error
    Local Variables:
          String_Type prompt = "Search forward:"
          Integer_Type dir = 1
          Ref_Type line_ok_fun = &_function_return_1
          String_Type str = "ascascascasc"
          Char_Type not_found = 1
          Integer_Type cs = 0
    /grandpa/d1/src/jed/lib/search.sl:85:search_forward:Run-Time Error
#v-
 There are several ways to read this report; perhaps the simplest is
 to read it from the bottom.  This report says that on line 85 in
 \exmp{search.sl} the \exmp{search_forward} function called the
 \exmp{search_generic_search} function.  On line 78 it called the
 \sfun{verror} function, which in turn called \ifun{error}.  The
 \exmp{search_generic_search} function contains 6 local variables
 whose values at the time of the error are given by the traceback
 output.  The above example shows that a local variable called
 \exmp{"not_found"} had a \dtype{Char_Type} value of 1 at the time
 of the error.

\labeled_sect{Using the sldb debugger}

 The interpreter contains a number of hooks that support a debugger.
 \sldb consists of a set of functions that use these hooks to implement
 a simple debugger.  Although written for \slsh, the debugger may be
 used by other \slang interpreters that permit the loading of \slsh
 library files.  The examples presented here are given in the context
 of \slsh.

 In order to use the debugger, the code to to be debugged must be
 loaded with debugging info enabled.  This can be in done several
 ways, depending upon the application embedding the interpreter.

 For applications that support a command line, the simplest way to
 access the debugger is to use the \sfun{sldb} function with the name
 of the file to be debugged:
#v+
   require ("sldb");
   sldb ("foo.sl");
#v-
 When called without an argument, \sfun{sldb} will prompt for input.
 This can be useful for setting or removing breakpoints.

 Another mechanism to access the debugger is to put
#v+
   require ("sldb");
   sldb_enable ();
#v-
 at the top of the suspect file.  Any files loaded by the file will
 also be compiled with debugging support, making it unnecessary to
 add this to all files.

 If the file contains any top-level executable statements, the
 debugger will display the line to be executed and prompt for input.
 If the file does not contain any executable statements, the debugger
 will not be activated until one of the functions in the file is
 executed.

 As a concrete example, consider the following contrived \slsh script
 called \exmp{buggy.sl}:
#v+
    define divide (a, b, i)
    {
       return a[i] / b;
    }
    define slsh_main ()
    {
       variable x = [1:5];
       variable y = x*x;
       variable i;
       _for i (0, length(x), 1)
         {
            variable z = divide (x, y, i);
            () = fprintf (stdout, "%g/%g = %g", x[i], y[i], z);
         }
    }
#v-
 Running this via
#v+
    slsh buggy.sl
#v-
 yields
#v+
    Expecting Double_Type, found Array_Type
    ./buggy.sl:13:slsh_main:Type Mismatch
#v-
 More information may be obtained by using \slsh's \exmp{-g} option to
 cause a traceback report to be printed:
#v+
    slsh -g buggy.sl
    Expecting Double_Type, found Array_Type
    Traceback: fprintf
    ./buggy.sl:13:slsh_main:Type Mismatch
    Local variables for slsh_main:
         Array_Type x = Integer_Type[5]
         Array_Type y = Integer_Type[5]
         Integer_Type i = 0
         Array_Type z = Integer_Type[5]
    Error encountered while executing slsh_main
#v-
 From this one can see that the problem is that \exmp{z} is an array
 and not a scalar as expected.

 To run the program under debugger control, startup \slsh and load the
 file using the \sfun{sldb} function:
#v+
    slsh> sldb ("./buggy.sl");
#v-
 Note the use of \exmp{"./"} in the filename.  This may be necessary
 if the file is not in the \slsh search path.

 The above command causes execution to stop with the following displayed:
#v+
    slsh_main at ./buggy.sl:9
    9    variable x = [1:5];
    (sldb)
#v-
 This shows that the debugger has stopped the script at line 9 of
 buggy.sl and is waiting for input.  The \exmp{print} function may be
 used to print the value of an expression or variable.  Using it to
 display the value of \exmp{x} yields
#v+
    (sldb) print x
    Caught exception:Variable Uninitialized Error
    (sldb)
#v-
 This is because \exmp{x} has not yet been assigned a value and will
 not be until line 9 has been executed.  The \exmp{next} command may
 be used to execute the current line and stop at the next one:
#v+
    (sldb) next
    10    variable y = x*x;
    (sldb)
#v-
 The \exmp{step} command functions almost the same as \exmp{next},
 except when a function call is involved.  In such a case, the
 \exmp{next} command will step over the function call but \exmp{step}
 will cause the debugger to enter the function and stop there.

 Now the value of \exmp{x} may be displayed using the \exmp{print}
 command:
#v+
    (sldb) print x
    Integer_Type[5]
    (sldb) print x[0]
    1
    (sldb) print x[-1]
    5
    (sldb)
#v-

  The \var{list} command may be used to get a list of the source code
  around the current line:
#v+
    (sldb) list
    5     return a[i] / b;
    6  }
    7  define slsh_main ()
    8  {
    9     variable x = [1:5];
    10    variable y = x*x;
    11    variable i;
    12    _for i (0, length(x), 1)
    13      {
    14      variable z = divide (x, y, i);
    15      () = fprintf (stdout, "%g/%g = %g", x[i], y[i], z);
#v-

  The \exmp{break} function may be used to set a breakpoint.  For
  example,
#v+
    (sldb) break 15
    breakpoint #1 set at ./buggy.sl:15
#v-
  will set a break point at the line 15 of the current file.

  The \exmp{cont} command may be used to continue execution until the
  next break point:
#v+
    (sldb) cont
    Breakpoint 1, slsh_main
        at ./buggy.sl:15
    15      () = fprintf (stdout, "%g/%g = %g", x[i], y[i], z);
    (sldb)
#v-
  Using the \exmp{next} command produces:
#v+
    Received Type Mismatch error.  Entering the debugger
    15      () = fprintf (stdout, "%g/%g = %g", x[i], y[i], z);
#v-
  This shows that during the execution of line 15, a
  \exmp{TypeMismatchError} was generated.  Let's see what caused it:
#v+
    (sldb) print x[i]
    1
    (sldb) print y[i]
    1
    (sldb) print z
    Integer_Type[5]
#v-
  This shows that the problem was caused by \exmp{z} being an array and not a
  scalar--- something that was already known from the traceback
  report.  Now let's see why it is not a scalar.  Start the program
  again and set a breakpoint in the \exmp{divide} function:
#v+
    slsh_main at ./buggy.sl:9
    9    variable x = [1:5];
    (sldb) break divide
    breakpoint #1 set at divide
    (sldb) cont
    Breakpoint 1, divide
    at ./buggy.sl:5
    5    return a[i] / b;
    (sldb)
#v-
  The values of \exmp{a[i]/b} and \exmp{b} may be printed:
#v+
    (sldb) print a[i]/b
    Integer_Type[5]
    (sldb) print b
    Integer_Type[5]
#v-
  From this it is easy to see that \exmp{z} is an array because
  \exmp{b} is an array.  The fix for this is to change line 5 to
#v+
    z = a[i]/b[i];
#v-

  The debugger supports several other commands.  For example, the
  \exmp{up} and \exmp{down} commands may be used to move up and down
  the stack-frames, and \exmp{where} command may be used to display
  the stack-frames.  These commands are useful for examining the
  variables in the other frames:
#v+
    (sldb) where
    #0 ./buggy.sl:5:divide
    #1 ./buggy.sl:14:slsh_main
    (sldb) up
    #1 ./buggy.sl:14:slsh_main
    14      variable z = divide (x, y, i);
    (sldb) print x
    Integer_Type[5]
    (sldb) down
    #0 ./buggy.sl:5:divide
    5    return a[i] / b;
    (sldb) print z
    Integer_Type[5]
#v-

  On some operating systems, the debugger's \exmp{watchfpu} command
  may be used to help isolate floating point exceptions.  Consider the
  following example:
#v+
     define solve_quadratic (a, b, c)
     {
        variable d = b^2 - 4.0*a*c;
        variable x = -b + sqrt (d);
        return x / (2.0*a);
     }
     define print_root (a, b, c)
     {
        vmessage ("%f %f %f %f\n", a, b, c, solve_quadratic (a,b,c));
     }
     print_root (1,2,3);
#v-
  Running it via \slsh produces:
#v+
    1.000000 2.000000 3.000000 nan
#v-
  Now run it in the debugger:
#v+
     <top-level> at ./example.sl:12
     11 print_root (1,2,3);
     (sldb) watchfpu FE_INVALID
     (sldb) cont
     *** FPU exception bits set: FE_INVALID
     Entering the debugger.
     solve_quadratic at ./t.sl:4
     4    variable x = -b + sqrt (d);
#v-
  This shows the the \exmp{NaN} was produced on line 4.

  The \exmp{watchfpu} command may be used to watch for the occurrence
  of any combination of the following exceptions
#v+
     FE_DIVBYZERO
     FE_INEXACT
     FE_INVALID
     FE_OVERFLOW
     FE_UNDERFLOW
#v-
  by the bitwise-or operation of the desired combination. For
  instance, to track both \exmp{FE_INVALID} and \exmp{FE_OVERFLOW},
  use:
#v+
   (sldb) watchfpu FE_INVALID | FE_OVERFLOW
#v-

#%}}}

\chapter{Profiling}
\sect{Introduction}

This chapter deals with the subject of writing efficient \slang code,
and using the \slang profiler to isolate places in the code that could
benefit from optimization.

The most important consideration in writing efficient code is the
choice of algorithm.  A poorly optimized good algorithm will almost
always execute faster than a highly optimized poor algorithm.  In
choosing an algorithm, it is also important to choose the right data
structures for its implementation.  As a simple example, consider the
task of counting words.  Any algorithm would involve a some sort of
table with word/number pairs.  Such a table could be implemented using
a variety of data structures, e.g., as a pair of arrays or lists
representing the words and corresponding numbers, as an array of
structures, etc. But in this case, the associative array is ideally
suited to the task:
#v+
   a = Assoc_Type[Int_Type, 0];
   while (get_word (&word))
     a[word]++;
#v-

Note the conciseness of the above code.  It is important to appreciate
the fact that \slang is a byte-compiled interpreter that executes
statements much slower than that of a language that compiles to
machine code.  The overhead of the processing of byte-codes by the
interpreter may be used to roughly justify the rule of thumb that the
smaller the code is, the faster it will run.

When possible, always take advantage of \slang's powerful array
facilities.  For example, consider the act of clipping an array by
setting all values greater than 10 to 10.  Rather than coding this as
#v+
    n = length(a);
    for (i = 0; i < n; i++)
      if (a[i] > 10) a[i] = 10;
#v-
it should be written as
#v+
    a[where(a>10)] = 10;
#v-

Finally, do not overlook the specialized modules that are available
for \slang.

\sect{Using the profiler}

\slprof is an executable \slsh script that implements a standalone
profiler for \slsh scripts.  The script is essentially a front-end for
a set of interpreter hooks defined in a file called \file{profile.sl},
which may be used by any application embedding \slang.  The use of the
profiler will first be demonstrated in the context of \slprof, and
after that follows a discussion of how to use \file{profile.sl} for
other \slang applications.

(To be completed...)

#i regexp.tm

#%}}}

\appendix

#i intnews.tm

#i copyright.tm

\end{\documentstyle}
