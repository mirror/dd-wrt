\function{alarm}
\synopsis{Schedule an alarm signal}
\usage{alarm (UInt_Type secs [, Ref_Type secs_remaining])}
\description
  The \ifun{alarm} function schedules the delivery of a \icon{SIGALRM}
  signal in \exmp{secs} seconds.  Any previously scheduled alarm will
  be canceled.  If \exmp{secs} is zero, then no new alarm will be
  scheduled.  If the second argument is present, then it must be a
  reference to a variable whose value will be set upon return to the
  number of seconds remaining for a previously scheduled alarm to take
  place.
\example
  This example shows demonstrates how the \ifun{alarm} function may be
  used to read from a file within a specified amount of time:
#v+
    define sigalrm_handler (sig)
    {
       throw ReadError, "Read timed out";
    }
    define read_or_timeout (secs)
    {
       variable line, e;
       variable fp = fopen ("/dev/tty", "r");
       signal (SIGALRM, &sigalrm_handler);
       alarm (secs);
       try (e)
         {
            () = fputs ("Enter some text> ", stdout); () = fflush (stdout);
            if (-1 == fgets (&line, fp))
              line = NULL;
         }
       catch IOError: { message (e.message); line = NULL; }
       return line;
    }
#v-
\notes
  Some operating systems may implement the \ifun{sleep} function using
  \ifun{alarm}.  As a result, it is not a good idea to mix calls to
  \ifun{alarm} and \ifun{sleep}.

  The default action for \icon{SIGALRM} is to terminate the process.
  Hence, if \ifun{alarm} is called it is wise to establish a signal
  handler for \ifun{SIGALRM}.
\seealso{signal, sleep}
\done

\function{signal}
\synopsis{Establish a signal handler}
\usage{signal (Int_Type sig, Ref_Type func [,Ref_Type old_func])}
\description
  The \ifun{signal} function assigns the signal handler represented by
  \exmp{func} to the signal \exmp{sig}.  Here \exmp{func} is usually
  reference to a function that takes an integer argument (the signal)
  and returns nothing, e.g.,
#v+
    define signal_handler (sig)
    {
       return;
    }
#v-
  Alternatively, \exmp{func} may be given by one of the symbolic
  constants \icon{SIG_IGN} or \icon{SIG_DFL} to indicate that the
  signal is to be ignored or given its default action, respectively.

  The first parameter, \exmp{sig}, specifies the signal to be handled.
  The actual supported values vary with the OS.  Common values on Unix
  include \exmp{SIGHUP}, \exmp{SIGINT}, and \exmp{SIGTERM}.

  If a third argument is present, then it must be a reference to a
  variable whose value will be set to the value of the previously
  installed handler.
\example
  This example establishes a handler for \exmp{SIGTSTP}.
#v+
    static define sig_suspend ();  % forward declaration
    static define sig_suspend (sig)
    {
       message ("SIGTSTP received-- stopping");
       signal (sig, SIG_DFL);
       () = kill (getpid(), SIGSTOP);
       message ("Resuming");
       signal (sig, &sig_suspend);
    }
    signal (SIGTSTP, &sig_suspend);
#v-
\notes
  Currently the signal interface is supported only on systems that
  implement signals according to the POSIX standard.

  Once a signal has been received, it will remain blocked until after
  the signal handler has completed.  This is the reason \icon{SIGSTOP}
  was used in the above signal handler instead of \icon{SIGTSTP}.
\seealso{alarm, sigsuspend, sigprocmask}
\done

\function{sigprocmask}
\synopsis{Change the list of currently blocked signals}
\usage{sigprocmask (Int_Type how, Array_Type mask [,Ref_Type old_mask])}
\description
  The \ifun{sigprocmask} function may be used to change the list of
  signals that are currently blocked.  The first parameter indicates
  how this is accomplished.  Specifically, \exmp{how} must be one of
  the following values: \icon{SIG_BLOCK}, \icon{SIG_UNBLOCK}, or
  \icon{SIG_SETMASK}.

  If \exmp{how} is \icon{SIG_BLOCK}, then the set of blocked signals
  will be the union the current set with the values specified in the
  \exmp{mask} argument.

  If \exmp{how} is \icon{SIG_UNBLOCK}, then the signals specified by
  the \exmp{mask} parameter will be removed from the currently blocked
  set.

  If \exmp{how} is \icon{SIG_SETMASK}, then the set of blocked signals
  will be set to those given by the \exmp{mask}.

  If a third argument is present, then it must be a reference to a
  variable whose value will be set to the previous signal mask.
\seealso{signal, sigsuspend, alarm}
\done

\function{sigsuspend}
\synopsis{Suspend the process until a signal is delivered}
\usage{sigsuspend ([Array_Type signal_mask])}
\description
  The \variable{sigsuspend} function suspends the current process
  until a signal is received.  An optional array argument may be
  passed to the function to specify a list of signals that should be
  temporarily blocked while waiting for a signal.
\example
  The following example pauses the current process for 10 seconds
  while blocking the \icon{SIGHUP} and \icon{SIGINT} signals.
#v+
     static variable Tripped;
     define sigalrm_handler (sig)
     {
        Tripped = 1;
     }
     signal (SIGALRM, &sigalrm_handler);
     Tripped = 0;
     alarm (10);
     while (Tripped == 0) sigsuspend ([SIGHUP, SIGINT]);
#v-
  Note that in this example the call to \ifun{sigsuspend} was wrapped in
  a while-loop.  This was necessary because there is no guarantee that
  another signal would not cause \ifun{sigsuspend} to return.
\seealso{signal, alarm, sigprocmask}
\done

