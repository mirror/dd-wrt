\variable{errno}
\synopsis{Error code set by system functions}
\usage{Int_Type errno}
\description
  A system function can fail for a variety of reasons.  For example, a
  file operation may fail because lack of disk space, or the process
  does not have permission to perform the operation.  Such functions
  will return -1 and set the variable \ivar{errno} to an error
  code describing the reason for failure.

  Particular values of \ivar{errno} may be specified by the following
  symbolic constants (read-only variables) and the corresponding
  \ifun{errno_string} value:
#v+
     E2BIG            "Arg list too long"
     EACCES           "Permission denied"
     EBADF            "Bad file number"
     EBUSY            "Mount device busy"
     ECHILD           "No children"
     EEXIST           "File exists"
     EFAULT           "Bad address"
     EFBIG            "File too large"
     EINTR            "Interrupted system call"
     EINVAL           "Invalid argument"
     EIO              "I/O error"
     EISDIR           "Is a directory"
     ELOOP            "Too many levels of symbolic links"
     EMFILE           "Too many open files"
     EMLINK           "Too many links"
     ENAMETOOLONG     "File name too long"
     ENFILE           "File table overflow"
     ENODEV           "No such device"
     ENOENT           "No such file or directory"
     ENOEXEC          "Exec format error"
     ENOMEM           "Not enough core"
     ENOSPC           "No space left on device"
     ENOTBLK          "Block device required"
     ENOTDIR          "Not a directory"
     ENOTEMPTY        "Directory not empty"
     ENOTTY           "Not a typewriter"
     ENXIO            "No such device or address"
     EPERM            "Operation not permitted"
     EPIPE            "Broken pipe"
     EROFS            "Read-only file system"
     ESPIPE           "Illegal seek"
     ESRCH            "No such process"
     ETXTBSY          "Text file busy"
     EXDEV            "Cross-device link"
#v-
\example
  The \ifun{mkdir} function will attempt to create a directory.  If it
  fails, the function will throw an IOError exception with a message
  containing the string representation of the \ivar{errno} value.
#v+
    if (-1 == mkdir (dir))
       throw IOError, sprintf ("mkdir %s failed: %s",
                               dir, errno_string (errno));
#v-
\seealso{errno_string, error, mkdir}
\done

\function{errno_string}
\synopsis{Return a string describing an errno.}
\usage{String_Type errno_string ( [Int_Type err ])}
\description
  The \ifun{errno_string} function returns a string describing the
  integer errno code \exmp{err}.  If the \exmp{err} parameter is
  omitted, the current value of \ivar{errno} will be used. See the
  description for \ivar{errno} for more information.
\example
  The \ifun{errno_string} function may be used as follows:
#v+
    define sizeof_file (file)
    {
       variable st = stat_file (file);
       if (st == NULL)
         throw IOError, sprintf ("%s: %s", file, errno_string (errno));
       return st.st_size;
    }
#v-
\seealso{errno, stat_file}
\done

\function{error}
\synopsis{Generate an error condition (deprecated)}
\usage{error (String_Type msg)}
\description
  This function has been deprecated in favor of \kw{throw}.

  The \ifun{error} function generates a \slang \exc{RunTimeError}
  exception. It takes a single string parameter which is displayed on
  the stderr output device.
\example
#v+
    define add_txt_extension (file)
    {
       if (typeof (file) != String_Type)
         error ("add_extension: parameter must be a string");
       file += ".txt";
       return file;
    }
#v-
\seealso{verror, message}
\done

\function{__get_exception_info}
\synopsis{Get information about the current exception}
\usage{Struct_Type __get_exception_info ()}
\description
 This function returns information about the currently active
 exception in the form as a structure with the
 following fields:
#v+
    error            The current exception, e.g., RunTimeError
    descr            A description of the exception
    file             Name of the file generating the exception
    line             Line number where the exception originated
    function         Function where the exception originated
    object           A user-defined object thrown by the exception
    message          A user-defined message
    traceback        Traceback messages
#v-
 If no exception is active, \NULL will be returned.

 This same information may also be obtained via the optional argument
 to the \kw{try} statement:
#v+
     variable e = NULL;
     try (e)
       {
          do_something ();
       }
     finally
       {
          if (e != NULL)
            vmessage ("An error occurred: %s", e.message);
       }
#v-
\seealso{error}
\done

\function{message}
\synopsis{Print a string onto the message device}
\usage{message (String_Type s)}
\description
  The \ifun{message} function will print the string specified by
  \exmp{s} onto the message device.
\example
#v+
     define print_current_time ()
     {
       message (time ());
     }
#v-
\notes
  The message device will depend upon the application.  For example,
  the output message device for the \jed editor corresponds to the
  line at the bottom of the display window.  The default message
  device is the standard output device.
\seealso{vmessage, sprintf, error}
\done

\function{new_exception}
\synopsis{Create a new exception}
\usage{new_exception (String_Type name, Int_Type baseclass, String_Type descr)}
\description
  This function creates a new exception called \exmp{name} subclassed
  upon \exmp{baseclass}.  The description of the exception is
  specified by \exmp{descr}.
\example
#v+
  new_exception ("MyError", RunTimeError, "My very own error");
  try
    {
       if (something_is_wrong ())
         throw MyError;
    }
  catch RunTimeError;
#v-
  In this case, catching \exc{RunTimeError} will also catch
  \exmp{MyError} since it is a subclass of \exc{RunTimeError}.
\seealso{error, verror}
\done

\function{usage}
\synopsis{Generate a usage error}
\usage{usage (String_Type msg)}
\description
  The \ifun{usage} function generates a \exc{UsageError} exception and
  displays \exmp{msg} to the message device.
\example
  Suppose that a function called \exmp{plot} plots an array of \exmp{x} and
  \exmp{y} values.  Then such a function could be written to issue a
  usage message if the wrong number of arguments was passed:
#v+
    define plot ()
    {
       variable x, y;

       if (_NARGS != 2)
         usage ("plot (x, y)");

       (x, y) = ();
       % Now do the hard part
          .
          .
    }
#v-
\seealso{error, message}
\done

\function{verror}
\synopsis{Generate an error condition (deprecated)}
\usage{verror (String_Type fmt, ...)}
\description
  This function has been deprecated in favor or \kw{throw}.

  The \ifun{verror} function performs the same role as the \ifun{error}
  function.  The only difference is that instead of a single string
  argument, \ifun{verror} takes a sprintf style argument list.
\example
#v+
    define open_file (file)
    {
       variable fp;

       fp = fopen (file, "r");
       if (fp == NULL) verror ("Unable to open %s", file);
       return fp;
    }
#v-
\notes
  In the current implementation, the \ifun{verror} function is not an
  intrinsic function.  Rather it is a predefined \slang function using
  a combination of \ifun{sprintf} and \ifun{error}.

  To generate a specific exception, a \kw{throw} statement should be
  used.  In fact, a \kw{throw} statement such as:
#v+
     if (fp == NULL)
       throw OpenError, "Unable to open $file"$;
#v-
  is preferable to the use of \ifun{verror} in the above example.
\seealso{error, Sprintf, vmessage}
\done

\function{vmessage}
\synopsis{Print a formatted string onto the message device}
\usage{vmessage (String_Type fmt, ...)}
\description
  The \ifun{vmessage} function formats a sprintf style argument list
  and displays the resulting string onto the message device.
\notes
  In the current implementation, the \ifun{vmessage} function is not an
  intrinsic function.  Rather it is a predefined \slang function using
  a combination of \ifun{Sprintf} and \ifun{message}.
\seealso{message, sprintf, Sprintf, verror}
\done

