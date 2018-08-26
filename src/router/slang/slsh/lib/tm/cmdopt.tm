\function{cmdopt_new}
\synopsis{Create a cmdopt object for parsing command-line options}
\usage{obj = cmdopt_new (Ref_Type error_routine)}
\description
  This function creates an returns an object that may be used by the
  \sfun{cmdopt_process} function to parse command line arguments.  The
  \sfun{cmdopt_new} function takes a reference to an error handling
  function that will get called upon error.  In most cases, this
  function should print out the error message, display a usage
  message, and the call \ifun{exit}.  If the error handler is \NULL,
  or it returns instead of calling exit, then an exception will be thrown.

  The error hander must be defined to take a single string argument
  (the error message) and must return nothing.
\example
#v+
   require ("cmdopt");
   private define help_callback ()
   {
     () = fputs ("Usage: pgm [options] infile\n", stderr);
     () = fputs ("Options:\n", stderr);
     () = fputs (" -h|--help           Show this help\n", stderr);
     () = fputs (" -v|--verbose        Increase verbosity level\n", stderr);
     () = fputs (" -o|--output         Output filename [stdout]\n", stderr);
     exit (1);
   }
   private define error_handler (text)
   {
      () = fprintf (stderr, "%s\n", text);
      help_callback ();
   }
   define slsh_main ()
   {
      variable verbose = 0;
      outfile = "-";   % stdout
      variable c = cmdopt_new (&error_handler);
      cmdopt_add (c, "v|verbose", &verbose; inc);
      cmdopt_add (c, "h|help", &help_callback);
      cmdopt_add (c, "s:o|output", &outfile; type="str");
      variable iend = cmdopt_process (c, __argv, 1);

      if (verbose) message ("some informative message");
      variable fp = stdout;
      if (outfile != "-") fp = fopen (outfile, "w");
        .
        .
    }
#v-
\seealso{cmdopt_add, cmdopt_process}
\done

\function{cmdopt_process}
\synopsis{Process the command-line options}
\usage{Int_Type cmdopt_process (optobj, argv, istart)}
#v+
   Struct_Type optobj;
   Array_Type argv;
   Int_Type istart
#v-
\description
  This function parses the command line arguments in the string array
  \exmp{argv} according to the rules specified by the \exmp{optobj}
  object, previously allocated by \sfun{cmdopt_new}.  The array of
  strings is processed starting at the index specified by
  \exmp{istart}.  The function returns the index of the array element
  where parsing stopped.  Upon error, the function will call the error
  handler established  by the prior call to \exmp{cmdopt_new}.
\example
#v+
    define slsh_main ()
    {
          .
          .
       optobj = cmdopt_new (...);
       cmdopt_add (optobj, ...);
          .
          .
       variable iend = cmdopt_process (optobj, __argv, 1);
          .
          .
    }
#v-
\notes
  This function may also be called in an object-oriented style using the
  \exmp{process} method:
#v+
       optobj = cmdopt_new (...);
       optobj.add (...)
       iend = optobj.process (__argv, 1);
#v-
\seealso{cmdopt_add, cmdopt_new}
\done

\function{cmdopt_add}
\synopsis{Add support for a command-line option}
\usage{cmdopt_add (optobj, optname, addr [,...] [;qualifiers])}
#v+
   Struct_Type optobj;
   String_Type optname;
   Ref_Type addr;
#v-
\description
  This function adds support for a command-line option to
  \exmp{optobj} and specifies how that option should be handled.
  Handling an option involves setting the value of a variable
  associated with the option, or by calling a function upon its
  behalf.

  For clarity, assume a command-line option can be specified using the
  single character \exmp{f} or by the longer name \exmp{foo}. Then the
  rules for calling \sfun{cmdopt_add} for the various flavors options
  supported by this interface and how the option may be specified on
  the command line are as follows:

  Options that set a variable \exmp{v} to a value \exmp{val}:
#v+
    cmdopt_add (optobj, "f|foo", &v; default=val);
    cmdline: pgm -f ...
    cmdline: pgm --foo ...
#v-

  Options that increment an integer variable \exmp{v}:
#v+
    cmdopt_add (optobj, "f|foo", &v; inc);
    cmdline: pgm -f -f ...       % In these examples, v
    cmdline: pgm --foo --foo ... % gets incremented twice
#v-

  Options that bitwise-or an integer variable \exmp{v} with \exmp{FLAG}:
#v+
    cmdopt_add (optobj, "f|foo", &v; bor=FLAG);
    cmdline: pgm -f ...       % v = v | FLAG
    cmdline: pgm --foo ...    % v = v | FLAG
#v-
  Options that bitwise-and an integer variable \exmp{v} with \exmp{MASK}:
#v+
    cmdopt_add (optobj, "f|foo", &v; band=MASK);
    cmdline: pgm -f ...       % v = v & MASK;
    cmdline: pgm --foo ...    % v = v & MASK;
#v-
  The above two options may be combined:
#v+
    cmdopt_add (optobj, "f|foo", &v; bor=FLAG1, band=~FLAG2);
    cmdline: pgm -f ...       % v &= ~FLAG2; v |= FLAG1;
#v-

  Options that require a value and set \exmp{v} to the value VAL.
#v+
    cmdopt_add (optobj, "f|foo", &v; type="int");
    cmdline: pgm -f VAL ...
    cmdline: pgm -fVAL ...
    cmdline: pgm --foo VAL ...
    cmdline: pgm --foo=VAL ...
#v-

  Options whose value is optional:
#v+
    cmdopt_add (optobj, "f|foo", &v; type="string", optional=DLFT);
    cmdline: pgm -f ...            % set v to DFLT
    cmdline: pgm -fVAL ...         % set v to VAL
    cmdline: pgm --foo ...         % set v to DFLT
    cmdline: pgm --foo=VAL ...     % set v to VAL
#v-

  For the latter two cases, if the \exmp{append} qualifier is used,
  then instead of assigning the value to the specified variable, the
  value will be appended to a list assigned to the variable, e.g.,
#v+
    cmdopt_add (optobj, "f|foo", &v; type="float", append);
#v-
  Then the command line \exmp{pgm --foo=VAL1 -fVAL2 -f VAL3 ...} will
  result in the assignment to \exmp{v} or the 3 element list
  \exmp{\{VAL1, VAL2, VAL3\}}.

  An option can also be associated with a callback function that get
  called when the option is handled.

  Options that cause a function to be called with arguments
  \exmp{a0,...}:
#v+
    cmdopt_add (optobj, "f|foo", &func, a0...);
    cmdline: pgm --foo
    cmdline: pgm -f
#v-
  Here \exmp{func} should be written with the signature:
#v+
    define func (a0, ...) {...}
#v-
  Options that take a value and cause a function to be called with
  additional arguments \exmp{a0,...}:
#v+
    cmdopt_add (optobj, "f|foo", &func, a0,...; type="int");
    cmdline: pgm --foo=VAL
    cmdline: pgm -f VAL
    cmdline: pgm -fVAL
#v-
  In this case, \exmp{func} should be written as
#v+
    define func (value, a0, ...) {...}
#v-

  As the above examples illustrate, the data-type of the value assigned
  to a variable must be specified using the \exmp{type} qualifier.
  Currently the \exmp{type} must be set to one of the following values:
#v+
     "str"          (String_Type)
     "int"          (Int_Type)
     "float"        (Double_Type)
#v-
\notes
  This function may also be called in an object-oriented style using the
  \exmp{add} method:
#v+
       optobj = cmdopt_new (...);
       optobj.add ("f|foo", &func, a0,...; type="int");
#v-
\seealso{cmdopt_new, cmdopt_process}
\done
