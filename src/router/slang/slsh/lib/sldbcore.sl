%
% This file implements the core a simple debugger.  It needs to be wrapped
% by routines that implement the Debugger_Methods.
%
% Public functions:
%   sldb_methods()
%   sldb_stop ();
%   sldb_start ();
%   sldb_set_breakpoint ();
%
%
%  Notes:
%
%    If a file was not compiled with bos/eos hooks, then debugging of
%    it may be limited due to the lack of line number information.
%
%
require ("print");

private variable Debugger_Methods = struct
{
   list,          % list (file, linemin, linemax)
   vmessage,      % vmessage (fmt, args...)
   read_input,    % input = read_input (prompt, default)
   pprint,        % pprint(obj) % pprint the value of an object
   quit,          % quit (and kill) the program
   exit		  % exit the debugger but not the program
};

private define output ()
{
   variable args = __pop_args (_NARGS);
   (@Debugger_Methods.vmessage)(__push_args(args));
}

private define quit_method ()
{
   output ("Program exiting\n");
   exit (0);
}
Debugger_Methods.quit = &quit_method;

private define exit_method ()
{
   output ("Leaving the debugger\n");
}
Debugger_Methods.exit = &exit_method;

define sldb_methods ()
{
   return Debugger_Methods;
}
define sldb_initialize ();	       % This should be overridden

define sldb_stop();

private variable Depth = 0;
private variable Stop_Depth	= 0;
private variable Debugger_Step	= 0;
private variable STEP_NEXT	= 1;
private variable STEP_STEP	= 2;
private variable STEP_FINISH	= 3;
private variable STEP_EXIT	= 4;
private variable Breakpoints = NULL;
private variable Breakpoint_Number = 1;
private variable Current_Frame;
private variable Max_Current_Frame;
private variable Last_List_Line = 0;
private variable Last_Cmd_Line = NULL;
private variable Last_Cmd = NULL;
private variable Prompt = "(SLdb) ";

private define new_breakpoints ()
{
   Breakpoints = Assoc_Type[Int_Type, 0];
   Breakpoint_Number = 1;
}

private define check_breakpoints ()
{
   if (Breakpoints == NULL)
     new_breakpoints ();
}

define sldb_set_breakpoint (pos)
{
   variable bp;

   check_breakpoints ();
   bp = Breakpoint_Number;
   Breakpoints[pos] = bp;
   Breakpoint_Number++;

   output ("breakpoint #%d set at %s\n", bp, pos);
   return bp;
}

private define make_breakpoint_name (file, line)
{
   return sprintf ("%S:%d", file, line);
}

private define eval_in_frame (frame, expr, num_on_stack, print_fun)
{
   variable boseos = _boseos_info;
   variable bofeof = _bofeof_info;
   expr = sprintf ("_boseos_info=0; _bofeof_info=0; _use_frame_namespace(%d); %s; _bofeof_info=%d; _boseos_info=%d;",
		   frame, expr, bofeof, boseos);
   variable depth = _stkdepth () - num_on_stack;
   eval (expr);

   variable n = _stkdepth () - depth;
   if (print_fun == NULL)
     return n;

   loop (n)
     {
	variable val = ();
	(@print_fun) (val);
     }
   return n;
}

private define break_cmd (cmd, args, file, line)
{
   variable bp;
   if (strlen (args) == 0)
     bp = make_breakpoint_name (file, line);
   else if (_slang_guess_type (args) == Int_Type)
     bp = make_breakpoint_name (file, integer (args));
   else
     {
	bp = args;
	if (0 == is_substr (args, ":"))
	  {
	  }
     }

   () = sldb_set_breakpoint (bp);
   return 0;
}

private define display_file_and_line (file, linemin, linemax)
{
   if (file == "***string***")
     return;

   if (linemin < 1)
     linemin = 1;
   if (linemax < linemin)
     linemax = linemin;

   (@Debugger_Methods.list)(file, linemin, linemax);
}

private define finish_cmd (cmd, args, file, line)
{
   %variable fun = _get_frame_info (Max_Current_Frame).function;
   variable fun = _get_frame_info (Current_Frame).function;
   if (fun == NULL) fun = "<top-level>";
   output ("Run until exit from %s\n", fun);
   Debugger_Step = STEP_FINISH;
   Stop_Depth = Depth-1;
   return 1;
}

private define next_cmd (cmd, args, file, line)
{
   Debugger_Step = STEP_NEXT;
   Stop_Depth = Depth;
   return 1;
}

private define step_cmd (cmd, args, file, line)
{
   Debugger_Step = STEP_STEP;
   Stop_Depth = Depth + 1;
   return 1;
}

private define delete_cmd (cmd, args, file, line)
{
   variable bp = make_breakpoint_name (file, line);
   variable n = Breakpoints[bp];
   if (n)
     {
	Breakpoints[bp] = 0;
	output ("Deleted breakpoint #%d\n", n);
	return 0;
     }
   if (args == "")
     {
	new_breakpoints ();
	output ("Deleted all breakpoints\n");
	return 0;
     }

   variable keys = assoc_get_keys (Breakpoints);
   variable vals = assoc_get_values (Breakpoints);

   foreach (eval (sprintf ("[%s]", args)))
     {
	bp = ();
	variable i = wherefirst (vals == bp);
	if (i == NULL)
	  continue;
	assoc_delete_key (Breakpoints, keys[i]);
	output ("Deleted breakpoint %d\n", bp);
     }
   return 0;
}

private define continue_cmd (cmd, args, file, line)
{
   Debugger_Step = 0;
   return 1;
}

private define watch_cmd (cmd, args, file, line)
{
   output ("%s is not implemented\n", cmd);
   return 0;
}

private define exit_cmd (cmd, args, file, line)
{
   sldb_stop ();
   (@Debugger_Methods.exit) ();
   return 1;
}

private define quit_cmd (cmd, args, file, line)
{
   variable prompt = "Are you sure you want to quit (and kill) the program? (y/n) ";
   variable y = (@Debugger_Methods.read_input)(prompt, NULL);
   y = strup (y);
   !if (strlen (y))
     return 0;
   if (y[0] != 'Y')
     {
	output ("Try using 'exit' to leave the debugger");
	return 0;
     }
   sldb_stop ();
   (@Debugger_Methods.quit)();
   return 1;
}

private define simple_print (v)
{
   if (length (v) <= 1)
     print (v, &v);
   output ("%S\n", v);
}

private define pretty_print (v)
{
   variable p = Debugger_Methods.pprint;
   if (p == NULL)
     {
	simple_print (v);
	return;
     }
   (@p)(v);
}

private define print_expr (print_fun, expr)
{
   variable info = _get_frame_info (Current_Frame);
   variable localvars = info.locals;

   if (localvars == NULL)
     {
	() = eval_in_frame (Current_Frame, expr, 0, print_fun);
	return;
     }

   % Create a dummy function and call it with the values of the local-vars
   % The idea is that variables that are initialized will be arguments, and
   % others will just be locals
   variable a = Assoc_Type[];
   foreach (localvars)
     {
	variable lvar = ();
	try
	  {
	     a[lvar] = _get_frame_variable (Current_Frame, lvar);
	  }
	catch VariableUninitializedError;
     }
   variable inited_vars = assoc_get_keys (a);
   variable uninited_vars = String_Type[0];
   foreach (localvars)
     {
	lvar = ();
	if (assoc_key_exists (a, lvar))
	  continue;
	uninited_vars = [uninited_vars, lvar];
     }
   if (length (uninited_vars))
     uninited_vars = strcat ("variable ", strjoin (uninited_vars, ","), ";");
   else
     uninited_vars = "";

   variable fmt = "private define %s (%s) { %s %s; }";
   variable dummy = "__debugger_print_function";
   variable fun = sprintf (fmt, dummy, strjoin (inited_vars, ","),
			   uninited_vars, expr);
   () = eval_in_frame (Current_Frame, fun, 0, print_fun);

   % push values to the stack and call the dummy function
   foreach lvar (inited_vars)
     {
	a[lvar];
     }
   () = eval_in_frame (Current_Frame, dummy, length (inited_vars), print_fun);
}

private define print_cmd (cmd, args, file, line)
{
   print_expr (&simple_print, args);
   return 0;
}

private define pprint_cmd (cmd, args, file, line)
{
   print_expr (&pretty_print, args);
   return 0;
}

private define list_cmd (cmd, args, file, line)
{
   variable dline = 5;
   line = int (line);

   if (Last_Cmd == cmd)
     line = Last_List_Line + 1 + dline;

   variable line_min = line - dline;
   variable line_max = line + dline;

   if (strlen (args))
     {
	line_min = integer (args);
	line_max = line_min + 10;
     }

   display_file_and_line (file, line_min, line_max);
   Last_List_Line = line_max;
   return 0;
}

private define print_frame_info (f, print_line)
{
   variable info = _get_frame_info (f);
   variable file = info.file;
   variable function = info.function;
   variable line = info.line;

   if (function == NULL)
     function = "<top-level frame>";

   output("#%d %S:%d:%s\n", Max_Current_Frame-f, file, line, function);
   if (print_line)
     display_file_and_line (file, line, line);
}

private define up_cmd (cmd, args, file, line)
{
   if (Current_Frame == 1)
     {
	output ("Can't go up\n");
	return 0;
     }
   Current_Frame--;
   print_frame_info (Current_Frame, 1);
   return 0;
}

private define down_cmd (cmd, args, file, line)
{
   if (Current_Frame == Max_Current_Frame)
     {
	output ("At inner-most frame\n");
	return 0;
     }
   Current_Frame++;
   print_frame_info (Current_Frame, 1);
   return 0;
}

private define where_cmd (cmd, args, file, line)
{
   variable i = Current_Frame;
   while (i > 0)
     {
	print_frame_info (i, 0);
	i--;
     }
   return 0;
}

#ifexists fpu_clear_except_bits
private variable WatchFPU_Flags = 0;
#endif

private define watchfpu_cmd (cmd, args, file, line)
{
#ifexists fpu_clear_except_bits
   fpu_clear_except_bits ();
   if (args == "")
     {
	WatchFPU_Flags = FE_ALL_EXCEPT;
	output ("Watching all FPU exceptions:\n");
	output (" FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW\n");
	return 0;
     }
   WatchFPU_Flags = eval (args);
   if (WatchFPU_Flags == 0)
     {
	output ("Watching FPU exceptions disabled\n");
     }
   return 0;
#else
   output ("watchfpu is not supported on this OS\n");
   return 0;
#endif
}

private variable Cmd_Table = Assoc_Type [Ref_Type];
Cmd_Table["finish"] = &finish_cmd;
Cmd_Table["next"] = &next_cmd;
Cmd_Table["step"] = &step_cmd;
Cmd_Table["break"] = &break_cmd;
Cmd_Table["delete"] = &delete_cmd;
Cmd_Table["cont"] = &continue_cmd;
Cmd_Table["continue"] = &continue_cmd;
Cmd_Table["watch"] = &watch_cmd;
Cmd_Table["list"] = &list_cmd;
Cmd_Table["pprint"] = &pprint_cmd;
Cmd_Table["print"] = &print_cmd;
Cmd_Table["exit"] = &exit_cmd;
Cmd_Table["quit"] = &quit_cmd;
Cmd_Table["up"] = &up_cmd;
Cmd_Table["down"] = &down_cmd;
Cmd_Table["where"] = &where_cmd;
Cmd_Table["watchfpu"] = &watchfpu_cmd;

% Aliases
define sldb_add_alias (alias, cmd)
{
   if (0 == assoc_key_exists (Cmd_Table, cmd))
     return;
   Cmd_Table[alias] = Cmd_Table[cmd];
}
sldb_add_alias ("b", "break");
sldb_add_alias ("c", "continue");
sldb_add_alias ("d", "delete");
sldb_add_alias ("h", "help");
sldb_add_alias ("l", "list");
sldb_add_alias ("n", "next");
sldb_add_alias ("p", "print");
sldb_add_alias ("pp", "pprint");
sldb_add_alias ("q", "quit");
sldb_add_alias ("s", "step");

private define help_cmd (cmd, args, file, line)
{
   output ("Commands:\n");
   variable cmds = assoc_get_keys (Cmd_Table);
   cmds = cmds[array_sort(cmds)];
   foreach cmd (cmds)
     output (" %s\n", cmd);
   return 0;
}
Cmd_Table["help"] = &help_cmd;

private define sigint_handler (sig)
{
   Debugger_Step = STEP_STEP;
   Stop_Depth = INT_MAX;
   if (Depth == 0)
     throw UserBreakError;
}

private variable Old_Sigint_Handler;
private define deinit_sigint_handler ()
{
#ifexists SIGINT
   signal (SIGINT, Old_Sigint_Handler);
#endif
}

private define init_sigint_handler ()
{
#ifexists SIGINT
   variable old;
   signal (SIGINT, &sigint_handler, &old);
   ifnot (_eqs(old, &sigint_handler))
     Old_Sigint_Handler = old;
#endif
}

private variable Last_Frame = -1;
private variable Last_Function = NULL;

private define debugger_input_loop ()
{
   variable max_frame = Max_Current_Frame;
   %Last_Cmd_Line = NULL;
   %Last_Cmd = NULL;
   forever
     {
	variable e;
	try (e)
	  {
	     deinit_sigint_handler ();
	     Debugger_Step = 0;

	     if (Current_Frame > max_frame)
	       {
		  Max_Current_Frame = max_frame;
		  Current_Frame = max_frame;
	       }
	     variable info = _get_frame_info (Current_Frame);
	     variable file = info.file;
	     variable line = info.line;

	     variable cmdline, cmd, cmd_parm;
	     forever
	       {
		  variable prompt = Prompt;
#iffalse
		  prompt = "Depth=${Depth},Stop_Depth=${Stop_Depth} $prompt"$;
#endif
		  cmdline = (@Debugger_Methods.read_input)(prompt, Last_Cmd_Line);
		  if (cmdline == NULL)
		    throw ReadError, "NULL input returned";

		  cmdline = strtrim (cmdline);
		  variable tokens = strtok (cmdline, " \t");
		  if (length (tokens))
		    {
		       cmd = tokens[0];
		       break;
		    }
	       }
	     cmd_parm = substr (cmdline, 1+strlen(cmd), -1);
	     cmd_parm = strtrim (cmd_parm, "\t ");

	     if (0 == assoc_key_exists (Cmd_Table, cmd))
	       {
		  output("%s is unknown.  Try help.\n", cmd);
		  Last_Cmd_Line = NULL;
		  Last_Cmd = NULL;
		  continue;
	       }
	     variable ret = (@Cmd_Table[cmd])(cmd, cmd_parm, file, line);
	     Last_Cmd_Line = cmdline;
	     Last_Cmd = cmd;
	     if (ret) return;
	  }
	catch IOError:
	  {
	     sldb_stop ();
	     vmessage ("Caught IOError exception -- stopping the debugger: %S",e.message);
	     return;
	  }
	catch AnyError:
	  {
	     output("Caught exception:%S:%S:%S:%S\n", e.file, e.line, e.function, e.message);
	  }
     }
}

private define do_debug (file, line, bp_num)
{
   %output ("do_debug: file=%S, line=%S, fun=%S\n", file, line, bp_num);
   Current_Frame = _get_frame_depth ()-2;
   Max_Current_Frame = Current_Frame;
   %vmessage ("Current_Frame=%d\n", Current_Frame);
   % We do not want the debug_hook catching errors here
   variable debug_hook = _set_debug_hook (NULL);
   EXIT_BLOCK
     {
	if (Debugger_Step != STEP_EXIT)
	  {
	     () = _set_debug_hook (debug_hook);
	     init_sigint_handler ();
	  }
     }

   variable info = _get_frame_info (Current_Frame);
   if (file == NULL)
     {
	file = info.file;
	if (file == NULL)
	  file = "???";
     }
   if (line == NULL)
     line = info.line;

   variable fun = info.function;
   if (fun == NULL) fun = "<top-level>";

   if ((file == "<stdin>"))% or (file == "***string***"))
     {
	Last_Frame = Current_Frame;
	Last_Function = fun;
	Debugger_Step = STEP_NEXT;
	Stop_Depth = Depth-1;
	return;
     }
   if (bp_num)
     {
	output ("Breakpoint %d, %s\n    at %s:%d\n", abs(bp_num), fun, file, line);
     }
   else if ((Last_Frame != Current_Frame) or (Last_Function != fun))
     {
	output ("%s at %s:%d\n", fun, file, line);
     }
   display_file_and_line (file, line, line);
   Last_Frame = Current_Frame;
   Last_Function = fun;

   debugger_input_loop ();
}

private define bos_handler (file, line)
{
   %output ("bos: depth=%d, stop_depth=%d, fun=%S\n", Depth,Stop_Depth,_get_frame_info(-1).function);
   variable pos = make_breakpoint_name (file, line);
   variable bp = Breakpoints[pos];

   if (bp)
     {
	if (bp < 0) Breakpoints[pos] = 0;   %  clear temporary breakpoint
	do_debug (file, line, bp);
	return;
     }

   if (Depth > Stop_Depth)
     return;

   if (Debugger_Step == 0)
     return;

#iffalse
   if (Debugger_Step == STEP_FINISH)
     return;

   if (Debugger_Step == STEP_NEXT)
     {
	if (Depth > Stop_Depth)
	  return;
     }
#endif
   do_debug (file, line, bp);
}

% end of statement handler: tracks the recursion depth,
% to be able to step over function calls (using 'Next' Command)
private define eos_handler()
{
#ifexists fpu_clear_except_bits
   if (WatchFPU_Flags)
     {
	variable bits = fpu_test_except_bits (WatchFPU_Flags);
	if (bits)
	  {
	     variable info = _get_frame_info (-1);
	     variable str = String_Type[0];
	     if (bits & FE_DIVBYZERO) str = [str,"FE_DIVBYZERO"];
	     if (bits & FE_INEXACT) str = [str,"FE_INEXACT"];
	     if (bits & FE_INVALID) str = [str,"FE_INVALID"];
	     if (bits & FE_OVERFLOW) str = [str,"FE_OVERFLOW"];
	     if (bits & FE_UNDERFLOW) str = [str,"FE_UNDERFLOW"];
	     output ("*** FPU exception bits set: %s\n", strjoin(str, ","));
	     output ("Entering the debugger.\n");
	     fpu_clear_except_bits ();
	     do_debug (info.file, info.line, 0);
	  }
     }
#endif
   %output ("eos: depth=%d\n", Depth);
}

private define bof_handler (fun, file)
{
   %output ("Entering BOF: %S, %S, %S", fun, file, line);
   Depth++;

   variable bp = Breakpoints[fun];
   if (bp)
     {
	output ("Breakpoint %d, %s\n", abs(bp), fun);
	if (bp < 0) Breakpoints[fun] = 0;   %  clear temporary breakpoint
	Debugger_Step = STEP_NEXT;
	Stop_Depth = Depth;
     }
}

private define eof_handler ()
{
   %output ("Leaving EOF");
   Depth--;
   if (Debugger_Step)
     {
	if (Debugger_Step == STEP_FINISH)
	  {
	     if (Depth == Stop_Depth)
	       {
		  Debugger_Step = 0;
		  %variable info = _get_frame_info (_get_frame_depth ()-2);
		  %do_debug (info.file, info.line, 0);
		  do_debug (NULL, NULL, 0);
	       }
	  }
	if ((Debugger_Step == STEP_NEXT) and (Stop_Depth > Depth))
	  Stop_Depth = Depth;
     }
}

private define debug_hook (file, line)
{
   %variable file = e.file, line = e.line;
   variable e = __get_exception_info ();
   output ("Received %s error.  Entering the debugger\n", e.descr);
   check_breakpoints ();
   do_debug (file, line, 0);
}

define sldb_enable ()
{
   ()=_set_bos_handler (&bos_handler);
   ()=_set_eos_handler (&eos_handler);
   ()=_set_bof_handler (&bof_handler);
   ()=_set_eof_handler (&eof_handler);
   ()=_set_debug_hook (&debug_hook);

   check_breakpoints ();
   Depth = 0;
   Debugger_Step = STEP_STEP;
   init_sigint_handler ();
   _traceback = 1;
   _bofeof_info = 1;
   _boseos_info = 3;
}

% Usage Forms:
%   sldb ();
%   sldb (file);
%   sldb (file, ns);
% The namespace semantics are the same as that of require.
define sldb ()
{
   sldb_initialize ();

   sldb_enable ();
   if (_NARGS == 0)
     {
	Current_Frame = _get_frame_depth ()-1;
	Max_Current_Frame = Current_Frame;
	debugger_input_loop ();
	return;
     }
   variable args = __pop_args (_NARGS);
   require (__push_args (args));
#iffalse
   variable ns = current_namespace ();
   if (_NARGS == 2)

     ns = ();
   variable file = ();

   if (ns == NULL)
     () = evalfile (file);
   else
     () = evalfile (file, ns);
#endif
}

% remove bos and eos handlers.
define sldb_stop ()
{
   ()=_set_bos_handler (NULL);
   ()=_set_eos_handler (NULL);
   ()=_set_bof_handler (NULL);
   ()=_set_eof_handler (NULL);
   ()=_set_debug_hook (NULL);
   deinit_sigint_handler ();
   _bofeof_info = 0;
   _boseos_info = 0;
   Debugger_Step = STEP_EXIT;
}

provide ("sldbcore");
