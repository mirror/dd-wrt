\function{SLang_verror}
\synopsis{Signal an error with a message}
\usage{void SLang_verror (int code, char *fmt, ...);}
\description
   The \var{SLang_verror} function sets \var{SLang_Error} to
   \var{code} if \var{SLang_Error} is 0.  It also displays the error
   message implied by the \var{printf} variable argument list using
   \var{fmt} as the format.
\example
#v+
      FILE *open_file (char *file)
      {
         char *file = "my_file.dat";
	 if (NULL == (fp = fopen (file, "w")))
	   SLang_verror (SL_INTRINSIC_ERROR, "Unable to open %s", file);
	 return fp;
      }
#v-
\seealso{SLang_vmessage, SLang_exit_error}
\done

\function{SLang_doerror}
\synopsis{Signal an error}
\usage{void SLang_doerror (char *err_str)}
\description
  The \var{SLang_doerror} function displays the string \var{err_str}
  to the error device and signals a \slang error.
\notes
  \var{SLang_doerror} is considered to obsolete.  Applications should
  use the \var{SLang_verror} function instead.
\seealso{SLang_verror, SLang_exit_error}
\done

\function{SLang_vmessage}
\synopsis{Display a message to the message device}
\usage{void SLang_vmessage (char *fmt, ...)}
\description
  This function prints a \var{printf} style formatted variable
  argument list to the message device.  The default message device is
  \var{stdout}.
\seealso{SLang_verror}
\done

\function{SLang_exit_error}
\synopsis{Exit the program and display an error message}
\usage{void SLang_exit_error (char *fmt, ...)}
\description
   The \var{SLang_exit_error} function terminates the program and
   displays an error message using a \var{printf} type variable
   argument list.  The default behavior to this function is to write
   the message to \var{stderr} and exit with the \var{exit} system
   call.

   If the function pointer \var{SLang_Exit_Error_Hook} is
   non-NULL, the function to which it points will be called.  This
   permits an application to perform whatever cleanup is necessary.
   This hook has the prototype:
#v+
     void (*SLang_Exit_Error_Hook)(char *, va_list);
#v-
\seealso{SLang_verror, exit}
\done
