\function{SLang_init_tty}
\synopsis{Initialize the terminal keyboard interface}
\usage{int SLang_init_tty (int intr_ch, int no_flow_ctrl, int opost)}
\description
  \var{SLang_init_tty} initializes the terminal for single character
  input.  If the first parameter \var{intr_ch} is in the range 0-255,
  it will be used as the interrupt character, e.g., under Unix this
  character will generate a \var{SIGINT} signal.  Otherwise, if it is
  \exmp{-1}, the interrupt character will be left unchanged.

  If the second parameter \var{no_flow_ctrl} is non-zero, flow control
  (\var{XON}/\var{XOFF}) processing will be
  enabled.

  If the last parmeter \var{opost} is non-zero, output processing by the
  terminal will be enabled.  If one intends to use this function in
  conjunction with the \slang screen management routines
  (\var{SLsmg}), this paramete shold be set to zero.

  \var{SLang_init_tty} returns zero upon success, or \-1 upon error.
\notes
  Terminal I/O is a complex subject.  The \slang interface presents a
  simplification that the author has found useful in practice.  For
  example, the only special character processing that
  \var{SLang_init_tty} enables is that of the \var{SIGINT} character,
  and the generation of other signals via the keyboard is disabled.
  However, generation of the job control signal \var{SIGTSTP} is possible
  via the \var{SLtty_set_suspend_state} function.

  Under Unix, the integer variable \var{SLang_TT_Read_FD} is used to
  specify the input descriptor for the terminal.  If
  \var{SLang_TT_Read_FD} represents a terminal device as determined
  via the \var{isatty} system call, then it will be used as the
  terminal file descriptor.  Otherwise, the terminal device
  \exmp{/dev/tty} will used as the input device.  The default value of
  \var{SLang_TT_Read_FD} is \-1 which causes \exmp{/dev/tty} to be
  used.  So, if you prefer to use \var{stdin} for input, then set
  \var{SLang_TT_Read_FD} to \exmp{fileno(stdin)} \em{before} calling
  \var{SLang_init_tty}.

  If the variable \var{SLang_TT_Baud_Rate} is zero when this function
  is called, the function will attempt to determine the baud rate by
  querying the terminal driver and set \var{SLang_TT_Baud_Rate} to
  that value.
\seealso{SLang_reset_tty, SLang_getkey, SLtty_set_suspend_state}
\done

\function{SLang_reset_tty}
\synopsis{Reset the terminal}
\usage{void SLang_reset_tty (void)}
\description
  \var{SLang_reset_tty} resets the terminal interface back to the
  state it was in before \var{SLang_init_tty} was called.
\seealso{SLang_init_tty}
\done

\function{SLtty_set_suspend_state}
\synopsis{Enable or disable keyboard suspension}
\usage{void SLtty_set_suspend_state (int s)}
\description
  The \var{SLtty_set_suspend_state} function may be used to enable or
  disable keyboard generation of the \var{SIGTSTP} job control signal.
  If \var{s} is non-zero, generation of this signal via the terminal
  interface will be enabled, otherwise it will be disabled.

  This function should only be called after the terminal driver has be
  initialized via \var{SLang_init_tty}.  The \var{SLang_init_tty}
  always disables the generation of \var{SIGTSTP} via the keyboard.
\seealso{SLang_init_tty}
\done

\function{SLang_getkey}
\synopsis{Read a character from the keyboard}
\usage{unsigned int SLang_getkey (void);}
\description
  The \var{SLang_getkey} reads a single character from the terminal
  and returns it.  The terminal must first be initialized via a call
  to \var{SLang_init_tty} before this function can be called.  Upon
  success, \var{SLang_getkey} returns the character read from the
  terminal, otherwise it returns \var{SLANG_GETKEY_ERROR}.
\seealso{SLang_init_tty, SLang_input_pending, SLang_ungetkey}
\done

\function{SLang_ungetkey_string}
\synopsis{Unget a key string}
\usage{int SLang_ungetkey_string (unsigned char *buf, unsigned int n)}
\description
  The \var{SLang_ungetkey_string} function may be used to push the
  \var{n} characters pointed to by \var{buf} onto the buffered input
  stream that \var{SLgetkey} uses.  If there is not enough room for
  the characters, \-1 is returned and none are buffered.  Otherwise,
  it returns zero.
\notes
  The difference between \var{SLang_buffer_keystring} and
  \var{SLang_ungetkey_string} is that the \var{SLang_buffer_keystring}
  appends the characters to the end of the getkey buffer, whereas
  \var{SLang_ungetkey_string} inserts the characters at the beginning
  of the input buffer.
\seealso{SLang_ungetkey, SLang_getkey}
\done

\function{SLang_buffer_keystring}
\synopsis{Append a keystring to the input buffer}
\usage{int SLang_buffer_keystring (unsigned char *b, unsigned int len)}
\description
  \var{SLang_buffer_keystring} places the \var{len} characters
  specified by \var{b} at the \em{end} of the buffer that
  \var{SLang_getkey} uses.  Upon success it returns 0; otherwise, no
  characters are buffered and it returns \-1.
\notes
  The difference between \var{SLang_buffer_keystring} and
  \var{SLang_ungetkey_string} is that the \var{SLang_buffer_keystring}
  appends the characters to the end of the getkey buffer, whereas
  \var{SLang_ungetkey_string} inserts the characters at the beginning
  of the input buffer.
\seealso{SLang_getkey, SLang_ungetkey, SLang_ungetkey_string}
\done

\function{SLang_ungetkey}
\synopsis{Push a character back onto the input buffer}
\usage{int SLang_ungetkey (unsigned char ch)}
\description
  \var{SLang_ungetkey} pushes the character \var{ch} back onto the
  \var{SLgetkey} input stream.  Upon success, it returns zero,
  otherwise it returns \1.
\example
  This function is implemented as:
#v+
    int SLang_ungetkey (unsigned char ch)
    {
       return SLang_ungetkey_string(&ch, 1);
    }
#v-
\seealso{SLang_getkey, SLang_ungetkey_string}
\done

\function{SLang_flush_input}
\synopsis{Discard all keyboard input waiting to be read}
\usage{void SLang_flush_input (void)}
\description
  \var{SLang_flush_input} discards all input characters waiting to be
  read by the \var{SLang_getkey} function.
\seealso{SLang_getkey}
\done

\function{SLang_input_pending}
\synopsis{Check to see if input is pending}
\usage{int SLang_input_pending (int tsecs)}
\description
  \var{SLang_input_pending} may be used to see if an input character
  is available to be read without causing \var{SLang_getkey} to block.
  It will wait up to \var{tsecs} tenths of a second if no characters
  are immediately available for reading.  If \var{tsecs} is less than
  zero, then \var{SLang_input_pending} will wait \exmp{-tsecs}
  milliseconds for input, otherwise \var{tsecs} represents \var{1/10}
  of a second intervals.
\notes
  Not all systems support millisecond resolution.
\seealso{SLang_getkey}
\done

\function{SLang_set_abort_signal}
\synopsis{Set the signal to trap SIGINT}
\usage{void SLang_set_abort_signal (void (*f)(int));}
\description
  \var{SLang_set_abort_signal} sets the function that gets
  triggered when the user presses the interrupt key (\var{SIGINT}) to
  the function \var{f}.  If \var{f} is \var{NULL} the default handler
  will get installed.
\example
  The default interrupt handler on a Unix system is:
#v+
     static void default_sigint (int sig)
     {
        SLKeyBoard_Quit = 1;
	if (SLang_Ignore_User_Abort == 0) SLang_Error = SL_USER_BREAK;
	SLsignal_intr (SIGINT, default_sigint);
   }
#v-
\notes
  For Unix programmers, the name of this function may appear
  misleading since it is associated with \var{SIGINT} and not
  \var{SIGABRT}.  The origin of the name stems from the original intent
  of the function: to allow the user to abort the running of a \slang
  interpreter function.
\seealso{SLang_init_tty, SLsignal_intr}
\done
