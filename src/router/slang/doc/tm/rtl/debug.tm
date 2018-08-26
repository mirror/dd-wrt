\variable{_bofeof_info}
\synopsis{Control the generation of function callback code}
\usage{Int_Type _bofeof_info}
\description
 This value of this variable dictates whether or not the \slang
 interpreter will generate code to call the beginning and end of
 function callback handlers.  The value of this variable is local to
 the compilation unit, but is inherited by other units loaded by the
 current unit.

 If the value of this variable is 1 when a function is defined, then
 when the function is executed, the callback handlers defined via
 \ifun{_set_bof_handler} and \ifun{_set_eof_handler} will be called.
\seealso{_set_bof_handler, _set_eof_handler, _boseos_info}
\done

\variable{_boseos_info}
\synopsis{Control the generation of BOS/EOS callback code}
\usage{Int_Type _boseos_info}
\description
 This value of this variable dictates whether or not the \slang
 interpreter will generate code to call the beginning and end of
 statement callback handlers.  The value of this variable is local to
 the compilation unit, but is inherited by other units loaded by the
 current unit.

 The lower 8 bits of \ivar{_boseos_info} controls the generation of code for
 callbacks as follows:
#v+
   Value      Description
   -----------------------------------------------------------------
     0        No code for making callbacks will be produced.
     1        Callback generation will take place for all non-branching
              and looping statements.
     2        Same as for 1 with the addition that code will also be
              generated for branching statements (if, !if, loop, ...)
     3        Same as 2, but also including break and continue
              statements.
#v-
 A non-branching statement is one that does not effect chain of
 execution.  Branching statements include all looping statements,
 conditional statement, \exmp{break}, \exmp{continue}, and \exmp{return}.

 If bit 0x100 is set, callbacks will be generated for preprocessor
 statements.
\example
 Consider the following:
#v+
   _boseos_info = 1;
   define foo ()
   {
      if (some_expression)
        some_statement;
   }
   _boseos_info = 2;
   define bar ()
   {
      if (some_expression)
        some_statement;
   }
#v-
 The function \exmp{foo} will be compiled with code generated to call the
 BOS and EOS handlers when \exmp{some_statement} is executed.  The
 function \exmp{bar} will be compiled with code to call the handlers
 for both \exmp{some_expression} and \exmp{some_statement}.
\notes
 The \sldb debugger and \slsh's \exmp{stkcheck.sl} make use of this
 facility.
\seealso{_set_bos_handler, _set_eos_handler, _bofeof_info}
\done

\function{_clear_error}
\synopsis{Clear an error condition (deprecated)}
\usage{_clear_error ()}
\description
  This function has been deprecated.  New code should make use of
  try-catch exception handling.

  This function may be used in error-blocks to clear the error that
  triggered execution of the error block.  Execution resumes following
  the statement, in the scope of the error-block, that triggered the
  error.
\example
  Consider the following wrapper around the \ifun{putenv} function:
#v+
    define try_putenv (name, value)
    {
       variable status;
       ERROR_BLOCK
        {
          _clear_error ();
          status = -1;
        }
       status = 0;
       putenv (sprintf ("%s=%s", name, value);
       return status;
    }
#v-
  If \ifun{putenv} fails, it generates an error condition, which the
  \exmp{try_putenv} function catches and clears.  Thus \exmp{try_putenv}
  is a function that returns -1 upon failure and 0 upon
  success.
\seealso{_trace_function, _slangtrace, _traceback}
\done

\function{_set_bof_handler}
\synopsis{Set the beginning of function callback handler}
\usage{_set_bof_handler (Ref_Type func)}
\description
 This function is used to set the function to be called prior to the
 execution of the body \slang function but after its arguments have
 been evaluated, provided that function was defined
 with \ivar{_bofeof_info} set appropriately.  The callback function
 must be defined to take a single parameter representing the name of
 the function and must return nothing.
\example
#v+
    private define bof_handler (fun)
    {
      () = fputs ("About to execute $fun"$, stdout);
    }
    _set_bos_handler (&bof_handler);
#v-
\seealso{_set_eof_handler, _boseos_info, _set_bos_handler}
\done

\function{_set_bos_handler}
\synopsis{Set the beginning of statement callback handler}
\usage{_set_bos_handler (Ref_Type func)}
\description
 This function is used to set the function to be called prior to the
 beginning of a statement.  The function will be passed two
 parameters: the name of the file and the line number of the statement
 to be executed.  It should return nothing.
\example
#v+
    private define bos_handler (file, line)
    {
      () = fputs ("About to execute $file:$line\n"$, stdout);
    }
    _set_bos_handler (&bos_handler);
#v-
\notes
 The beginning and end of statement handlers will be called for
 statements in a file only if that file was compiled with the variable
 \ivar{_boseos_info} set to a non-zero value.
\seealso{_set_eos_handler, _boseos_info, _bofeof_info}
\done

\function{_set_eof_handler}
\synopsis{Set the beginning of function callback handler}
\usage{_set_eof_handler (Ref_Type func)}
\description
 This function is used to set the function to be called at the end of
 execution of a \slang function, provided that function was compiled with
 \ivar{_bofeof_info} set accordingly.

 The callback function will be passed no parameters and it must return
 nothing.
\example
#v+
   private define eof_handler ()
   {
     () = fputs ("Done executing the function\n", stdout);
   }
   _set_eof_handler (&eof_handler);
#v-
\seealso{_set_bof_handler, _bofeof_info, _boseos_info}
\done

\function{_set_eos_handler}
\synopsis{Set the end of statement callback handler}
\usage{_set_eos_handler (Ref_Type func)}
\description
 This function is used to set the function to be called at the end of
 a statement.  The function will be passed no parameters and it should
 return nothing.
\example
#v+
   private define eos_handler ()
   {
     () = fputs ("Done executing the statement\n", stdout);
   }
   _set_eos_handler (&eos_handler);
#v-
\notes
 The beginning and end of statement handlers will be called for
 statements in a file only if that file was compiled with the variable
 \ivar{_boseos_info} set to a non-zero value.
\seealso{_set_bos_handler, _boseos_info, _bofeof_info}
\done

\variable{_slangtrace}
\synopsis{Turn function tracing on or off}
\usage{Integer_Type _slangtrace}
\description
  The \ivar{_slangtrace} variable is a debugging aid that when set to a
  non-zero value enables tracing when function declared by
  \ifun{_trace_function} is entered.  If the value is greater than
  zero, both intrinsic and user defined functions will get traced.
  However, if set to a value less than zero, intrinsic functions will
  not get traced.
\seealso{_trace_function, _traceback, _print_stack}
\done

\variable{_traceback}
\synopsis{Generate a traceback upon error}
\usage{Integer_Type _traceback}
\description
  \ivar{_traceback} is an intrinsic integer variable whose bitmapped value
  controls the generation of the call-stack traceback upon error.
  When set to 0, no traceback will be generated.  Otherwise its value
  is the bitwise-or of the following integers:
#v+
       1        Create a full traceback
       2        Omit local variable information
       4        Generate just one line of traceback
#v-
  The default value of this variable is 4.
\notes
  Running \slsh with the \exmp{-g} option causes this variable to be
  set to 1.
\seealso{_boseos_info}
\done

\function{_trace_function}
\synopsis{Set the function to trace}
\usage{_trace_function (String_Type f)}
\description
  \ifun{_trace_function} declares that the \slang function with name
  \exmp{f} is to be traced when it is called.  Calling
  \ifun{_trace_function} does not in itself turn tracing on.  Tracing
  is turned on only when the variable \ivar{_slangtrace} is non-zero.
\seealso{_slangtrace, _traceback}
\done

\function{_get_frame_info}
\synopsis{Get information about a stack frame}
\usage{Struct_Type _get_frame_info (Integer_Type depth)}
\description
  \ifun{_get_frame_info} returns a structure with information about
  the function call stack from of depth \svar{depth}. The structure
  contains the following fields:
#v+
    file: The file that contains the code of the stack frame.
    line: The line number the file the stack frame is in.
    function: the name of the function containing the code of the stack
      frame; it might be NULL if the code isn't inside a function.
    locals: Array of String_Type containing the names of variables local
      to the stack frame; it might be NULL if the stack frame doesn't
      belong to a function.
    namespace: The namespace the code of this stack frame is in.
#v-
\seealso{_get_frame_variable, _use_frame_namespace}
\done

\function{_get_frame_variable}
\synopsis{Get the value of a variable local to a stack frame}
\usage{Any_Type _get_frame_variable (Integer_Type depth, String_Type name)}
\description
  This function returns value of the variable \exmp{name} in the stack
  frame at depth \exmp{depth}.  This might not only be a local variable but
  also variables from outer scopes, e.g., a variable private to the
  namespace.

  If no variable with this name is found an \exc{UndefinedNameError}
  will be thrown.  An \exc{VariableUninitializedError} will be
  generated if the variable has no value.
\seealso{_get_frame_info, _use_frame_namespace}
\done

\function{_use_frame_namespace}
\synopsis{Selects the namespace of a stack frame}
\usage{_use_frame_namespace (Integer_Type depth)}
\description
  This function sets the current namespace to the one belonging to the
  call stack frame at depth \svar{depth}.
\seealso{_get_frame_info, _get_frame_variable}
\done
