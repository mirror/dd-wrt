#i docbook_man.tm

#d arg#1 <arg>$1</arg>\__newline__

#d slsh \command{slsh}
#d sldb \command{sldb}
#d slang \literal{S-Lang}
#d ifun#1 \literal{$1}
#d sfun#1 \literal{$1}

\manpage{slsh}{1}{Interpreter for S-Lang scripts}
\mansynopsis{slsh}{
  \arg{\option{--help}}
  \arg{\option{--version}}
  \arg{\option{-g}}
  \arg{\option{-n}}
  \arg{\option{--init}{file}}
  \arg{\option{--no-readline}}
  \arg{\option{-e}{string}}
  \arg{\option{-i}}
  \arg{\option{-q, --quiet}}
  \arg{\option{-t}}
  \arg{\option{-v}}
  \arg{\option{-|\replaceable{script-file args...}}}
}

\refsect1{DESCRIPTION}
  \p
  \slsh is a simple program for interpreting \slang scripts.  It
  supports dynamic loading of \slang modules and includes a readline
  interface for interactive use.
  \p-end

\refsect1-end

#d man_options_entry#2 \varlistentry{\term{$1}}{\p $2 \p-end}\__newline__

\refsect1{OPTIONS}
    \variablelist
      \man_options_entry{\option{--help}}{
        Show a summary of options
      }
      \man_options_entry{\option{--version}}{
        Show \slsh version information
      }
      \man_options_entry{\option{-g}}{
        Compile with debugging code, tracebacks, etc
      }
      \man_options_entry{\option{-n}}{
        Don't load the personal initialization file
      }
      \man_options_entry{\option{--init}{file}}{
        Use this file instead of ~/.slshrc
      }
      \man_options_entry{\option{--no-readline}}{
        Do not use a readline interface for the interactive mode
      }
      \man_options_entry{\option{-e}{string}}{
        Execute ``string'' as \slang code.
      }
      \man_options_entry{\option{-i}}{
        Force interactive mode.  Normally \slsh will go into
        interactive mode if both stdin and stdout are attached to a
        terminal.
      }
      \man_options_entry{\option{-q, --quiet}}{
        Startup quietly by not printing the version and copyright
        information.
      }
      \man_options_entry{\option{-t}}{
        Normally, \slsh will call \sfun{slsh_main} if it is defined.  This
        option prevents that from happening making it useful for
        checking for syntax error.
      }
      \man_options_entry{\option{-v}}{
        Show verbose loading messages.  This is useful for seeing what
        files are being loaded.
      }
    \variablelist-end
\refsect1-end

\refsect1{INITIALIZATION}
\p
   Upon startup, the program will try to load \filename{slsh.rc} as
   follows. If either \literal{SLSH_CONF_DIR} or
   \literal{SLSH_LIB_DIR} environment variables exist, then \slsh will
   look look in the corresponding directories for \filename{slsh.rc}.
   Otherwise it will look in:
\simplelist
    \member{\filename{$(prefix)/etc/}   (as specified in the Makefile)}
    \member{\filename{/usr/local/etc/}}
    \member{\filename{/usr/local/etc/slsh/}}
    \member{\filename{/etc/}}
    \member{\filename{/etc/slsh/}}
\simplelist-end
\pp
  The \filename{slsh.rc} file may load other files from slsh's library
  directory in the manner described below.
\pp
  Once \filename{slsh.rc} has been loaded, \slsh will load
  \literal{$HOME/.slshrc} if present.  Finally, it will load the
  script specified on the command line.  If the name of the script is
  \literal{-}, then it will be read from stdin.  If the script name is
  not present, or a string to execute was not specified using the -e
  option, then \slsh will go into interactive mode and read input from
  the terminal.  If the script is present and defines a function
  called \sfun{slsh_main}, that function will be called.
\p-end
\refsect1-end

\refsect1{LOADING FILES}
\p
  When a script loads a file via the built-in \ifun{evalfile} function
  or the \sfun{require} function (autoloaded by slsh.rc), the file is
  searched for along the \literal{SLSH_PATH} as specified in the Makefile.  An
  alternate path may be specified by the \literal{SLSH_PATH} environment
  variable.
\pp
  The search path may be queried and set during run time via the
  \sfun{get_slang_load_path} and \sfun{set_slang_load_path} functions, e.g.,
#v+
   set_slang_load_path ("/home/bill/lib/slsh:/usr/share/slsh");
#v-
\p-end
\refsect1-end

\refsect1{INTERACTIVE MODE}
\p
  When \slsh is invoked without a script or is given the \option{-i}
  command line argument, it will go into into interactive mode.  In
  this mode, the user will be prompted for input.  The program will
  leave this mode and exit if it sees an EOF (Ctrl-D) or the user
  exits by issuing the \literal{quit} command.
\pp
  If an uncaught exception occurs during execution of a command, the
  error message will be shown and the user will be prompted for more
  input.
\pp
  Any objects left on the stack after a command will be printed and
  the stack cleared.  This makes interactive mode useful as a
  calculator, e.g.,
#v+
     slsh> 3*10;
     30
     slsh> x = [1:20];
     slsh> sum (sin(x)-cos(x));
     0.458613
     slsh> quit;
#v-
  Note that in this mode, variables are automatically declared.
\pp
  The interactive mode also supports command logging.  Logging is
  enabled by the \literal{start_log} function.  The \literal{stop_log}
  function will turn off logging.  The default file where logging
  information will be written is \filename{slsh.log}.  An alternative
  may be specified as an optional argument to the \literal{start_log}
  function:
#v+
     slsh> start_log;
     Logging input to slsh.log
        .
        .
     slsh> stop_log;
     slsh> start_log("foo.log");
     Logging input to foo.log
        .
        .
     slsh> stop_log;
     slsh> start_log;
     Logging input to foo.log
#v-
\pp
  Similarly, the \literal{save_input} function may be used to save the
  previous input to a specified file:
#v+
     slsh> save_input;
     Input saved to slsh.log
     slsh> save_input ("foo.log");
     Input saved to foo.log
#v-
\pp
  As the above examples indicate, lines must end in a semicolon.  This
  is a basic feature of the language and permits commands to span
  multiple lines, e.g.,
#v+
     slsh> x = [
            1,2,3,
            4,5,6];
     slsh> sum(x);
#v-
  For convenience some users prefer that commands be automatically
  terminated with a semicolon.  To have a semicolon silently appended
  to the end of an input line, put the following in
  \filename{$HOME/.slshrc} file:
#v+
    #ifdef __INTERACTIVE__
    slsh_append_semicolon (1);
    #endif
#v-
\pp
  The interactive mode also supports shell escapes.  To pass a command
  to the shell, prefix it with \literal{!}, e.g.,
#v+
    slsh> !pwd
    /grandpa/d1/src/slang2/slsh
    slsh> !cd doc/tm
    slsh> !pwd
    /grandpa/d1/src/slang2/slsh/doc/tm
#v-
\pp
  Finally, the interactive mode supports a \literal{help} and
  \literal{apropos} function:
#v+
    slsh> apropos list
    apropos list ==>
    List_Type
    list_append
    list_delete
       .
       .
    slsh> help list_append
    list_append

     SYNOPSIS
       Append an object to a list

     USAGE
       list_append (List_Type, object, Int_Type nth)
       .
       .
#v-
  For convenience, the \literal{help} and \literal{apropos} functions
  do not require the syntactic constraints of the other functions.
\p-end
\refsect1-end

\refsect1{READLINE HISTORY MECHANISM}
\p
  By default, \slsh is built to use the \slang readline interface,
  which includes a customizable command completion and a history mechanism.
  When \slsh (or any \slang application that makes use of this
  feature) starts in interactive mode, it will look for a file in the
  user's home directory called \filename{.slrlinerc} and load it if
  present.  This file allows the user to customize the readline
  interface and enable the history to be saved between sessions.  As
  an example, here is a version of the author's
  \filename{.slrlinerc} file:
#v+
     % Load some basic functions that implement the history mechanism
     () = evalfile ("rline/slrline.rc");
     % The name of the history file -- expands to .slsh_hist for slsh
     RLine_History_File = "$HOME/.${name}_hist";

     % Some addition keybindings.  Some of these functions are defined
     % in rline/editfuns.sl, loaded by rline/slrline.rc
     rline_unsetkey ("^K");
     rline_setkey ("bol",   "^B");
     rline_setkey ("eol",   "^E");
     rline_setkey (&rline_kill_eol,  "^L");
     rline_setkey (&rline_set_mark,  "^K^B");
     rline_setkey (&rline_copy_region, "^Kk");
     rline_setkey (&rline_kill_region, "^K^V");
     rline_setkey (&rline_yank,  "^K^P");
     rline_setkey ("redraw",   "^R");

     #ifexists rline_up_hist_search
     % Map the up/down arrow to the history search mechanism
     rline_setkey (&rline_up_hist_search, "\e[A");
     rline_setkey (&rline_down_hist_search, "\e[B");
     #endif

     #ifexists rline_edit_history
     rline_setkey (&rline_edit_history, "^Kj");
     #endif

     % Add a new function
     private define double_line ()
     {
        variable p = rline_get_point ();
        variable line = rline_get_line ();
        rline_eol ();
        variable pend = rline_get_point ();
        rline_ins (line);
        rline_set_point (pend + p);
     }
    rline_setkey (&double_line,  "^K^L");
#v-
\p-end
\refsect1-end

\refsect1{MISCELLANEOUS SCRIPTS}
\p
  Several useful example scripts are located in
  \filename{$prefix/share/slsh/scripts/}, where $prefix represents the
  \slsh installation prefix (\filename{/usr},
  \filename{/usr/local},...).  These scripts include:
\variablelist
    \man_options_entry{\sldb}{A script that runs the \slang debugger.}
    \man_options_entry{\command{jpegsize}}{Reports the size of a jpeg file.}
    \man_options_entry{\command{svnsh}}{A shell for browsing an SVN repository.}
\variablelist-end
\p-end
\refsect1{AUTHOR}
\p
  The principal author of \slsh is John E. Davis <www.jedsoft.org>.
  The interactive mode was provided by Mike Noble.  The \slang library
  upon which \slsh is based is primarily the work of John E. Davis
  with help from many others.
\pp
  This manual page was originally written by Rafael Laboissiere for
  the Debian system (but may be used by others).
\pp
  Permission is granted to copy, distribute and/or modify
  this document under the terms of the GNU General Public License,
  Version 2 any later version published by the Free Software
  Foundation.
\pp
  On Debian systems, the complete text of the GNU General Public
  License can be found in \filename{/usr/share/common-licenses/GPL}
\p-end
\refsect1-end

\manpage-end
