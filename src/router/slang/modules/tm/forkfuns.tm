\function{fork}
\synopsis{Create a new process via the fork system function}
\usage{Int_Type fork ()}
\description
 The \ifun{fork} function creates a new child process via the
 \exmp{fork} system function.  See the approriate documentation for
 the actual semantics involved such as what is preserved in the child
 process.  Upon sucess, the function returns a value that is greater
 than 0 to the parent process that represents the child process's id.
 It will return 0 to the child process.  If the fork function fails, a
 value of -1 will be returned and \ivar{errno} set accordingly.
\example
 The following example creates a child process to invoke the ``ls''
 command on a Unix system.  It also illustrates the low-level nature
 of the \ifun{fork} and related system calls.
#v+
    define ls ()
    {
       variable pid = fork ();
       if (pid == -1)
         throw OSError, "fork failed: " + errno_string(errno);

       if ((pid == 0)
            && (-1 == execvp ("ls", ["ls", "-l"])))
         {
           () = fprintf (stderr, "execvp failed: " + errno_string(errno));
           _exit (1);
         }

       forever
         {
           variable status = waitpid (pid, 0);
           if (status == NULL)
             {
               if (errno == EINTR) continue;
               throw OSError, "waitpid failed: " + errno_string(errno);
             }
           return status.exit_status;
         }
     }
#v-
\seealso{waitpid, execv, execvp, execve, _exit, system}
\done

\function{execv}
\synopsis{Execute a new process}
\usage{Int_Type execv(path, argv[])}
\description
  The \ifun{execv} function may be used to overlay the current process
  with a new process by invoking to the program specified by the
  \exmp{path} argument.  If for some reason the function fails a
  value of -1 will be returned with \ivar{errno} set accordingly.
  Normally the function will not return.

  The \exmp{argv} parameter is an array of strings that will
  be used to construct the argument list for the program.  For
  example, if the invoked program is a C program, the \exmp{argv}
  parameter will be correspond to the C program's argv-list.
\notes
  The \exmp{path} parameter must specify the exact location of the
  executable.  The related \ifun{execvp} function may be used to
  search for the executable along a path.

  This function is a wrapper around the corresponding system library
  function.  See the system-specific documentation for more
  information.
\seealso{execvp, execve, system, fork, _exit}
\done

\function{execve}
\synopsis{Execute a new process}
\usage{Int_Type execve(path, argv[], envp[])}
\description
  The \ifun{execve} function may be used to overlay the current process
  with a new process by invoking to the program specified by the
  \exmp{path} argument.  If for some reason the function fails a
  value of -1 will be returned with \ivar{errno} set accordingly.
  Normally the function will not return.

  The \exmp{argv} parameter is an array of strings that will
  be used to construct the argument list for the program.  For
  example, if the invoked program is a C program, the \exmp{argv}
  parameter will be correspond to the C program's argv-list.

  The \exmp{envp} parameter is an array of strings that are used to
  initialize the environment of the overlayed program.
\notes
  This function is a wrapper around the corresponding system library
  function.  See the system-specific documentation for more information.
\seealso{execv, execvp, system, fork, _exit}
\done

\function{execvp}
\synopsis{Execute a new process}
\usage{Int_Type execvp(pgm, argv[])}
\description
  The \ifun{execvp} function may be used to overlay the current process
  with a new process by invoking to the program specified by the
  \exmp{pgm} argument.  If for some reason the function fails a
  value of -1 will be returned with \ivar{errno} set accordingly.
  Normally the function will not return.

  If the \exmp{pgm} argument does specify the directory containing the
  program, then a search will be performed for the program using,
  e.g., the \exmp{PATH} environment variable.

  The \exmp{argv} parameter is an array of strings that will
  be used to construct the argument list for the program.  For
  example, if the invoked program is a C program, the \exmp{argv}
  parameter will be correspond to the C program's argv-list.
\notes
  This function is a wrapper around the corresponding system library
  function.  See the system-specific documentation for more information.
\seealso{execv, execve, system, fork, _exit}
\done

\function{execve, execv, execvp}
\synopsis{Execute a new process}
\usage{Int_Type execve(path, argv[], envp[])}
#v+
  Int_Type execvp(path, argv[])}
  Int_Type execv(path, argv[])}
#v-
\description
  The \ifun{execv} family of functions overlay the current process
  with a new process that corresponds to the program specified by the
  \exmp{path} argument.  If for some reason the function fails a
  value of -1 will be returned with \ivar{errno} set accordingly.
  Normally the function will not return.

  The \exmp{argv} parameter is an array of strings that will
  correspond to the argument list used when invoking the program.  For
  example, if the invoked program is a C program, the \exmp{argv}
  parameter will be correspond to the C program's argv-list.

  The \ifun{execve} function takes an array of strings that will be
  used to initialize the environment of the overlayed program.

  The \ifun{execvp} function will mimick the actions /bin/sh when
  searching for the executable file.
\notes
  These function are wrappers around the corresponding system library
  functions.  See the system documentation for more information.
\seealso{execve, execvp, system, fork, _exit}
\done

\function{_exit}
\synopsis{Exit the current processes}
\usage{_exit(Int_Type status)}
\description
  Like the related \ifun{exit} function, \ifun{_exit} may be used to
  terminate the current process.  One of the differences between the two
  functions is that the \ifun{_exit} will not invoke various ``atexit''
  handlers that are normally run when a program is terminated.  See
  the system-specific C runtime library documentation for more
  information about this function.

  The main use of the \ifun{_exit} function is after the failure of
  one of the \ifun{execv} functions in a child process.
\seealso{fork, execv, execvp, execve, waitpid, exit}
\done

\function{waitpid}
\synopsis{Wait for a specified process}
\usage{Struct_Type waitpid (pid, options)}
\description
  The \exmp{waitpid} function will cause the calling process to wait
  for the child process whose process id is \exmp{pid} to change its
  state, e.g., exit.  It returns information about the process in the
  form of a structure with the following fields:
#v+
    pid           The process-id of the child process, 0 if the process
                    has not changed state.
    exited        Non-zero if the child exited normally.
    exit_status   The exit status of the child.  This field is
                    meaninful only if the exited field is non-zero.
    signal        Non-zero of the child was terminated by a signal.
                    The value of this field is that of the signal.
    coredump      Non-zero if the child produced a core-dump as the
                    result of termination by a signal.
    stopped       Non-zero if the child was stopped via a signal.  The
                    value is that of the signal that stopped it.
    continued     Non-zero if the process was continued.
#v-

  Upon error, the function will return \NULL and set \ifun{errno}
  accordingly.

  The value of the \exmp{options} parameter is the bitwise-or of one
  of the following values:
#v+
    WNOHANG      causes waitpid to return immediately if the child has
                  not terminated.  In this case, the value of the pid
                  field will be 0 if the child is still running.

    WUNTRACED    (see semantics in the OS documentation)
    WCONTINUED   (see semantics in the OS documentation)
#v-
\example
#v+
   s = waitpid (pid, WNOHANG);
   if (s == NULL)
     throw OSError, "waitpid failed: " + errno_string ();
   if (s.pid == 0)
     message ("The child is still running");
   else if (s.exited)
     vmessage ("child exited with status %d", s.exit_status);
   else if (s.signal)
     {
        vmessage ("child terminated by signal %d %s",
                  s.signal, (s.coredump ? "(coredump)" : ""));
     }
#v-
\seealso{fork, new_process}
\done
