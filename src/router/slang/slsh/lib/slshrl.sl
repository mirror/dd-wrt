% -*- mode: SLang; mode: fold -*-

% This file was written with the help of Mike Noble and John Houck.
require ("slshhelp");

if (is_defined ("slsh_help"))
  use_namespace ("slsh_interactive");
else
  implements ("slsh_interactive");

#ifnexists quit
public define quit () { exit (0); }
#endif

public define slsh_apropos ()
{
   if (_NARGS == 0)
     {
	vmessage ("apropos what?");
	return;
     }
   variable name = ();
   variable help = _apropos ("Global", "\\C"+name, 0xF);
   variable ns = current_namespace ();
   if (strlen (ns)
       and (ns != "Global"))
     {
	variable help1 = _apropos (ns, "\\C"+name, 0xF);
	help1 = (ns + "->") + help1;
	help = [help, help1];
     }

   if (0 == length (help))
     {
	() = fprintf (stdout, "No matches for %S\n", name);
	return;
     }

   () = fprintf (stdout, " apropos %s ==>\n", name);
   help = help[array_sort(help)];
   foreach (help)
     {
	help = ();
	() = fprintf (stdout, "  %s\n", help);
     }
}

private define generic_help ()
{
   variable help =
     [
      "Most commands must end in a semi-colon.",
      "If a command begins with '!', then the command is passed to the shell.",
      "  Examples: !ls, !pwd, !cd foo, ...",
      "Parenthesis are automatically added if the first word is a function and",
      "is followed by a ','.  For example:",
      "  plot, 1, 2;color=\"red\"  ==>  plot(1,2;color=\"red\");",
      "Special commands:",
      "  help <help-topic>",
      "  apropos <something>",
      "  start_log( <optional-log-file> );",
      "    start logging input to a file (default is slsh.log)",
      "  stop_log();",
      "    stop logging input",
      "  save_input (<optional-file>);",
      "    save all previous input to a file (default: slsh.log)",
      "  quit;"
      ];

   foreach (help)
     {
	variable h = ();
	() = fprintf (stdout, "%s\n", h);
     }
}

public define slsh_help ()
{
   if (_NARGS == 0)
     {
	generic_help ();
	return;
     }

   variable name = ();
   variable help = slsh_get_doc_string (name);
   if (help != NULL)
     {
	() = fprintf (stdout, "%s\n", help);
	return;
     }
   () = fprintf (stdout, "*** No help on %s\n", name);
   slsh_apropos (name);
}

private variable Append_Semicolon = 0;
public define slsh_append_semicolon (val)
{
   Append_Semicolon = val;
}

static define sys_shell_cmd (cmd)
{
   variable status;

   status = system (cmd);
   !if (status)
     return;

   () = fprintf (stdout, "shell command returned %d\n", status);
}

static define sys_chdir_cmd (dir) %{{{
{
   if (-1 == chdir (dir))
     () = fprintf (stderr, "chdir(%s) failed: %s\n", dir, errno_string (errno));
}

%}}}

% Support interactive mode which doesn't require the ';' EOL mark.
% Note that this no-semicolon-required mode causes surprising
% behavior in the '.source' intrinsic.  I'm taking the view
% that that's the price users must pay for the convenience of
% not typing the semicolons.
private define maybe_append_semicolon (input)
{
   !if (Append_Semicolon)
     return input;

   if (input[-1] != ';')
     input = strcat (input, ";");
   return input;
}

% This function is called after input has been read.
% It should be rewritten and made cleaner.
public define slsh_interactive_massage_hook (input)
{
   variable s, n;
   variable s0;
   variable ch;

   input = strtrim (input);
   !if (strlen (input))
     return input;

   ch = input[0];

   if (ch == '!')
     {
	% shell command, unlesss !if
	if (string_match (input, "^!if[ \t]*(", 1))
	  return input;

	input = strtrim (input[[1:]]);
        input = str_quote_string (input, "\"", '\\');

	% cd is special.  On unix systems we do not want to execute it in
	% a shell.
	if (strtok (input)[0] == "cd")
	  {
	     return sprintf ("slsh_interactive->sys_chdir_cmd(\"%s\");",
			     strtrim (input[[2:]]));
	  }

	return sprintf ("slsh_interactive->sys_shell_cmd(\"%s\");", input);
     }

   if (ch != '.')
     {
	% If the first non-word character is a comma, and the first
	% word is callable, then wrap the rest of the arguments in
	% parenthesis.
	s = strtrim (strtrans (input, "\\w", ""));
	if (s[0] == ',')
	  {
	     s0 = strtok (input, ", \t")[0];
	     if (__is_callable(__get_reference(s0)))
	       {
		  (s,) = strreplace (input, ",", "(", 1);
		  return strcat (s, ");");
	       }
	  }

	s = strtok (input, " \t");
	s = strtok (input, " \t()\";");
	if (length (s) == 0)
	  return input;
	s0 = s[0];
	if ((s0 == "exit") and (length (s)==1))
	  usage ("Try 'quit' to exit");

	if ((s0 != "help") and (s0 != "apropos") and (s0 != "quit"))
	  return maybe_append_semicolon (input);
     }
   else
     {
	%  line begins with "."
	variable type = _slang_guess_type (strtok (input, "-+*/<>&|; \t")[0]);
	if (type != String_Type)
	  {
	     % Do not allow the line to be parsed as RPN.  So prefix
	     % with a space.
	     return strcat (" ", maybe_append_semicolon (input));
	  }
	%return sprintf ("eval(\" %s\");", input);

	input = input[[1:]];

	s = strtok (input);
	s0 = s[0];
     }

   if (length (s) < 2)
     {
	if (s0 == "help")
	  return "slsh_help();";

	if (s0 == "quit")
	  return "exit(0);";

	return input;
     }

   if (length (s) == 2)
     {
	% Here the input consists of 2 words such as:
	%  cd foo
	%  load bar
	%  help goo
	%  apropos boo
	variable s1 = s[1];

	if (s1[0] != '(')
	  {
	     if ((s0 == "apropos") or (s0 == "help"))
	       {
		  s0 = strcat ("slsh_", s0);

		  if (0 == is_substr (s1, "\""))
		    return sprintf ("%s(\"%s\");", s0, s1);
		  else
		    return sprintf ("%s(%s);", s0, s1);
	       }

	     if (s0 == "cd")
	       return sprintf ("slsh->interactive->_sys_chdir_cmd(\"%s\");", s1);

	     %  Assume the next arg is a slang script.

	     if ((strlen(path_extname (s1)) == 0)
		 and (NULL != stat_file (s1 + ".sl")))
	       s1 = s1 + ".sl";

	     if (s0 == "load")
	       return sprintf ("()=evalfile(\"%s\");", s1);

#iffalse
	     if (s0 == "source")
	       return sprintf ("%s(\"%s\");", s0, s1);
#endif
	  }
     }

   % Anything else that does not look like a slang command will be converted
   % to a function call
   input = sprintf ("%s(%s);", s0, input[[strlen (s0):]]);

   return input;
}

%}}}

%---------------------------------------------------------------------------
% Logging Functions
%---------------------------------------------------------------------------
private variable Log_File = "slsh.log";

public define slsh_set_log_file (logfile)
{
   Log_File = logfile;
}

private variable Log_File_Fp = NULL;
private variable Input_Line_List = NULL;
private variable Last_Input_Line = NULL;

private define open_log_file (file)
{
   variable fp = fopen (file, "w");
   if (fp == NULL)
     vmessage ("***Warning: Unable to log to %s\n", file);
   return fp;
}

private define log_this ()
{
   variable args = __pop_args (_NARGS);
   if (Log_File_Fp == NULL)
     return;

   if (-1 == fprintf (Log_File_Fp, __push_args(args)))
     {
	vmessage ("Failed to write to log file-- logging stopped\n");
	Log_File_Fp = NULL;
     }
}

public define start_log ()
{
   if (_NARGS)
     Log_File = ();

   Log_File_Fp = open_log_file (Log_File);
   if (Log_File_Fp == NULL)
     return;

   log_this ("%% Logging started on %s\n", time ());
   log_this ("_auto_declare=%d;\n\n", _auto_declare);
   vmessage ("Logging input to %s\n", Log_File);
}

public define stop_log ()
{
   log_this ("%% Logging stopped on %s\n", time ());
   Log_File_Fp = NULL;
}

public define save_input ()
{
   variable file;

   !if (_NARGS)
     Log_File;

   file = ();

   if (Input_Line_List == NULL)
     return;

   variable fp = open_log_file (file);
   if (fp == NULL)
     return;

   variable l = Input_Line_List;
   while (l != NULL)
     {
	() = fprintf (fp, "%s\n", l.line);
	l = l.next;
     }
   vmessage ("Input saved to %s", file);
}

private define log_input (buf)
{
   buf = strtrim (buf);
   !if (strlen (buf))
     return;

   variable l = struct
     {
	line, next
     };
   l.line = buf;
   if (Input_Line_List == NULL)
     {
	Input_Line_List = l;
     }
   else Last_Input_Line.next = l;
   Last_Input_Line = l;

   if (Log_File_Fp != NULL)
     log_this ("%s\n", buf);
}

public define slsh_interactive_after_hook (line)
{
   log_input (line);
}

public define slsh_interactive_before_hook ()
{
   variable n = _stkdepth ();
   _stk_reverse (n);
   loop (n)
     {
	variable v = ();
	() = fprintf (stdout, "%S\n", v);
     }
}
