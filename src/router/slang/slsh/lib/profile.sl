% See the slprof script for an example of using the profiler.
%
_boseos_info=0;
_bofeof_info=0;

%public variable __DEBUG_PROFILE_;

% For lines
private variable Profile_Info_Type = struct
{
   name,			       % (bos/eof)
   num_called,			       % (bos/bof) num times line called
   num_s_triggered,		       % (eof/eof) num child statements
   num_f_triggered,		       % (eof/eof) num functions triggered
   num_self_f,			       % (eos/) num functions directly called
   num_self_s,			       % num statements directly called
   self_time,			       % (eos/eof)
   cum_time,			       % (eos/eof)
};
Profile_Info_Type.num_called = 0;
Profile_Info_Type.num_s_triggered = 0;
Profile_Info_Type.num_f_triggered = 0;
Profile_Info_Type.num_self_f = 0;
Profile_Info_Type.num_self_s = 0;
Profile_Info_Type.self_time = 0.0;
Profile_Info_Type.cum_time = 0.0;

private variable L_Info_Table;
private variable F_Info_Table;

private define convert_profile_info_to_array (p)
{
   variable keys = assoc_get_keys (p);
   variable i, n = length (p);

   % Filter out the bad entries
   _for i (0, n-1, 1)
     {
	if (keys[i] == "")	       %  Dummy
	  assoc_delete_key (p, keys[i]);
     }
   n = length (p);
   keys = assoc_get_keys (p);

   variable s = @Profile_Info_Type;
   s.num_called = Int_Type[n];
   s.num_s_triggered = Int_Type[n];
   s.num_f_triggered = Int_Type[n];
   s.num_self_f = Int_Type[n];
   s.num_self_s = Int_Type[n];
   s.self_time = Double_Type[n];
   s.cum_time = Double_Type[n];
   s.name = keys;

   variable vals = assoc_get_values (p);
   _for i (0, n-1, 1)
     {
	variable v = vals[i];
	s.num_called[i] = v.num_called;
	s.num_s_triggered[i] = v.num_s_triggered;
	s.num_f_triggered[i] = v.num_f_triggered;
	s.num_self_f[i] = v.num_self_f;
	s.num_self_s[i] = v.num_self_s;
	s.self_time[i] = v.self_time;
	s.cum_time[i] = v.cum_time;
     }
   return s;
}

private define new_l_info (name)
{
   variable l = @Profile_Info_Type;
   L_Info_Table[name] = l;
   l.name = name;
   return l;
}

private define new_f_info (name)
{
   variable finfo = @Profile_Info_Type;
   F_Info_Table[name] = finfo;
   finfo.name = name;
   return finfo;
}

private variable Function_Stack_Type = struct
{
   f_info,
   s_counter,
   f_counter,
   self_s_counter,
   self_time,
   cum_time,
};
private variable Function_Stack;

private variable Line_Stack_Type = struct
{
   l_info,
   s_counter,
   f_counter,
   self_f_counter,
   self_time,
   cum_time,
};
private variable Line_Stack;

private variable L_Info;
private variable L_S_Counter;	       %  bos/eof
private variable L_F_Counter;	       %  bos/eof
private variable L_Self_Time;	       %  bof/eof
private variable L_Cum_Time;	       %  bof/eof

private variable F_Info;
private variable F_S_Counter;	       %  bof
private variable F_F_Counter;	       %  bof
private variable F_Self_S;	       %  bos
private variable F_Self_Time;	       %  bos/eos
private variable F_Cum_Time;	       %  bos/eos

private variable Num_Statement_Counts;
private variable Num_Fun_Calls;
private variable Tstart;

private variable Null_Struct = struct {dummy};

private variable BOS_Depth;
private variable BOS_Stack_Depth;
private define push_line_info ()
{
   variable s = @Line_Stack_Type;
   s.l_info = L_Info;
   s.s_counter = L_S_Counter;
   s.f_counter = L_F_Counter;
   s.self_time = L_Self_Time;
   s.cum_time = L_Cum_Time;
   list_append (Line_Stack, s);
   BOS_Stack_Depth++;
}

private define pop_line_info ()
{
   variable s = list_pop (Line_Stack, -1);
   L_Info = s.l_info;
   L_Self_Time = s.self_time;
   L_Cum_Time = s.cum_time;

   L_Info.num_self_f++;
   L_Info.num_s_triggered += (Num_Statement_Counts - s.s_counter);
   L_Info.num_f_triggered += (Num_Fun_Calls - s.f_counter);
   L_F_Counter = Num_Fun_Calls;
   L_S_Counter = Num_Statement_Counts;
   BOS_Stack_Depth--;
}

% This variable tracks sequential calls to the BOS and EOS handlers
% This can happen when profiled code calls non-profiled, which calls
% profiled.
private variable Last_Was_BOS_EOS = 0;
private variable Dummy_L_Info;

private define bos_handler (file, line)
{
   variable dt = toc() - Tstart;

   F_Cum_Time += dt;
   F_Self_Time += dt;
   F_Self_S++;
   if (BOS_Depth >= BOS_Stack_Depth)
     {
#ifexists __DEBUG_PROFILE_
	%vmessage ("# bos pushing (%d,%d)", BOS_Depth,BOS_Stack_Depth);
#endif
	L_Cum_Time += dt;
	L_Self_Time += dt;
	push_line_info ();
     }
   BOS_Depth = BOS_Stack_Depth;

   Num_Statement_Counts++;

   file = sprintf ("%s:%d", file, line);
   variable l = L_Info_Table[file];
   if (l == Null_Struct)
     l = new_l_info (file);

   l.num_called++;

   L_Info = l;
   L_S_Counter = Num_Statement_Counts;
   L_F_Counter = Num_Fun_Calls;
   L_Self_Time = 0.0;
   L_Cum_Time = 0.0;

#ifexists __DEBUG_PROFILE_
   vmessage ("BOS: (%d, %d) %S", BOS_Depth, BOS_Stack_Depth, file);
#endif
   Tstart = toc ();
}

private define eos_handler ()
{
   variable dt = toc()-Tstart;

   if (BOS_Depth < BOS_Stack_Depth)
     {
#ifexists __DEBUG_PROFILE_
	%vmessage ("# eos popping (%d,%d)", BOS_Depth,BOS_Stack_Depth);
#endif
	pop_line_info ();
     }
   BOS_Depth--;

   L_Self_Time += dt;
   L_Cum_Time += dt;
   F_Cum_Time += L_Cum_Time;
   F_Self_Time += L_Self_Time;

   L_Info.self_time += L_Self_Time;
   L_Info.cum_time += L_Cum_Time;

#ifexists __DEBUG_PROFILE_
   vmessage ("EOS: (%d,%d) %S", BOS_Depth, BOS_Stack_Depth, L_Info.name);
#endif

   % This is necessary if a BOF/EOF sequence follows to avoid another
   % push/pop of this object since
   if (Last_Was_BOS_EOS == -1)
     L_Info = Dummy_L_Info;

   Last_Was_BOS_EOS = -1;
   Tstart = toc();
}

private define bof_handler (fun, file)
{
   variable dt = toc () - Tstart;
   Num_Fun_Calls++;

#ifexists __DEBUG_PROFILE_
   %vmessage ("# bof pushing (%d,%d)", BOS_Depth,BOS_Stack_Depth);
#endif
   L_Self_Time += dt;
   L_Cum_Time += dt;
   push_line_info ();
   %BOS_Stack_Depth++;

   variable f = @Function_Stack_Type;
   f.f_info = F_Info;
   f.s_counter = F_S_Counter;
   f.f_counter = F_F_Counter;
   f.self_time = F_Self_Time;
   f.self_s_counter = F_Self_S;
   f.cum_time = F_Cum_Time;
   list_append (Function_Stack, f);

   file = sprintf ("%S:%S", fun, file);
   variable f_info = F_Info_Table[file];
   if (f_info == Null_Struct)
     f_info = new_f_info (file);
   f_info.num_called++;

   F_Info = f_info;
   F_S_Counter = Num_Statement_Counts;
   F_F_Counter = Num_Fun_Calls;
   F_Cum_Time = 0.0;
   F_Self_Time = 0.0;
   F_Self_S = 0;

#ifexists __DEBUG_PROFILE_
   vmessage ("BOF: (%d,%d) %S", BOS_Depth, BOS_Stack_Depth, file);
#endif
   Tstart = toc ();
}

private define eof_handler ()
{
   variable dt = toc() - Tstart;

#ifexists __DEBUG_PROFILE_
   %vmessage ("# eof popping (%d,%d)", BOS_Depth,BOS_Stack_Depth);
#endif
   F_Cum_Time += dt;
   F_Self_Time += dt;
   F_Info.cum_time += F_Cum_Time;
   F_Info.self_time += F_Self_Time;
   F_Info.num_s_triggered += (Num_Statement_Counts - F_S_Counter);
   F_Info.num_f_triggered += (Num_Fun_Calls - F_F_Counter);
   F_Info.num_self_s += F_Self_S;

   pop_line_info ();
   % Add on time spent in the function to this line's value
   L_Cum_Time += F_Cum_Time;

#ifexists __DEBUG_PROFILE_
   vmessage ("EOF: (%d,%d) %S", BOS_Depth, BOS_Stack_Depth, F_Info.name);
#endif

   variable f = list_pop (Function_Stack, -1);
   F_Info = f.f_info;
   F_Info.num_self_f++;
   F_S_Counter = f.s_counter;
   F_F_Counter = f.f_counter;
   F_Self_Time = f.self_time;
   F_Self_S = f.self_s_counter;
   F_Cum_Time = f.cum_time;

   %F_Cum_Time += dt;
   %F_Self_Time += dt;

   Tstart = toc();
}

% In function-only mode, lines are not profiled.  The self-time of the
% function is its cumulative time minus the cumulative times of the
% functions that it directly calls.  The F_Self_Time variable will
% track those.
private define f_bof_handler (fun, file)
{
   variable dt = toc () - Tstart;
   Num_Fun_Calls++;

   variable f = @Function_Stack_Type;
   f.f_info = F_Info;
   f.f_counter = F_F_Counter;
   f.self_time = F_Self_Time;
   f.cum_time = F_Cum_Time + dt;
   list_append (Function_Stack, f);

   file = sprintf ("%S:%S", fun, file);
   variable f_info = F_Info_Table[file];
   if (f_info == Null_Struct)
     f_info = new_f_info (file);
   f_info.num_called++;

   F_Info = f_info;
   F_F_Counter = Num_Fun_Calls;
   F_Cum_Time = 0.0;
   F_Self_Time = 0.0;

   Tstart = toc ();
}

private define f_eof_handler ()
{
   variable dt = toc() - Tstart;

   F_Cum_Time += dt;
   F_Info.cum_time += F_Cum_Time;
   F_Info.self_time += (F_Cum_Time - F_Self_Time);
   F_Info.num_f_triggered += (Num_Fun_Calls - F_F_Counter);

   variable f = list_pop (Function_Stack, -1);
   F_Info = f.f_info;
   F_Info.num_self_f++;
   F_F_Counter = f.f_counter;
   F_Self_Time = f.self_time + F_Cum_Time;
   F_Cum_Time += f.cum_time;

   Tstart = toc();
}

% Usage: profile_on (do_line_profile)
define profile_on ()
{
   _boseos_info = 0;
   _bofeof_info = 1;
   if (_NARGS)
     {
	variable arg = ();
	if (arg)
	  _boseos_info = 3;
     }
}

define profile_off ()
{
   _boseos_info = 0;
   _bofeof_info = 0;
}

private define setup_profiler_variables ()
{
   L_Info_Table = Assoc_Type[Struct_Type, Null_Struct];
   F_Info_Table = Assoc_Type[Struct_Type, Null_Struct];
   Line_Stack = {};
   Function_Stack = {};

   Dummy_L_Info = new_l_info ("");
   L_Info = Dummy_L_Info;
   L_S_Counter = 0;
   L_F_Counter = 0;
   L_Self_Time = 0;
   L_Cum_Time = 0;

   F_Info = new_f_info ("");
   F_S_Counter = 0;
   F_F_Counter = 0;
   F_Self_Time = 0;
   F_Self_S = 0;
   F_Cum_Time = 0;

   Tstart = 0;
   Num_Statement_Counts = 0;
   Num_Fun_Calls = 0;
   BOS_Stack_Depth=0;
   BOS_Depth=0;
}

define profile_begin ()
{
   variable arg = 0;
   if (_NARGS)
     arg = ();
   if (arg)
     {
	()=_set_bos_handler (&bos_handler);
	()=_set_eos_handler (&eos_handler);
	()=_set_bof_handler (&bof_handler);
	()=_set_eof_handler (&eof_handler);
     }
   else
     {
	()=_set_bos_handler (NULL);
	()=_set_eos_handler (NULL);
	()=_set_bof_handler (&f_bof_handler);
	()=_set_eof_handler (&f_eof_handler);
     }
   setup_profiler_variables ();
   profile_on (arg);
   tic ();
}

define profile_end ()
{
   ()=_set_bos_handler (NULL);
   ()=_set_eos_handler (NULL);
   ()=_set_bof_handler (NULL);
   ()=_set_eof_handler (NULL);
   profile_off ();
}

% Calibration Notes:
%
%  If the execution of a statememt causes M other statements to
%  execute and N function calls, then the amount of profiler overhead
%  is given by
%
%     N*dF + M*dS
%
%  where dF is the amount of overhead per function call for running
%  the bof/eof-handlers, and dS is the overhead per statement in
%  running the bos/eos-handlers.
%
%  The timers used to measure the amount of time spent in a function
%  or statement are stopped when a handler is called, and started
%  again when the handler returns.  This introduces systematic error
%  in the measurements.  Call dt_S the systematic error introduced by
%  bos/eos handlers and dt_F systematic error from the bof/eof handlers.
%  Then the observed time as given by the interval times is related to
%  the "true" time t via
%
%     t_obs = t + N*dt_F + M*dt_S + dt_S
%
%  The self-time of the statement t_self is determined through an
%  interval timer that stops when a function is called, and starts
%  again after the function returns.  If the statement makes N_self
%  direct function calls, then t_self is related to its observed
%  self-time t_self_obs by
%
%     t_self_obs = t_self + N_self*dt_F + dt_S
%
%  The self-time of a function is the sum of the self-times of the
%  individual statements that were executed directly by the function.
%  Suppose the function executes M_self statements that in turn
%  directly call a total of N_self functions.  Then, for functions it
%  follows that:
%
%     t_self_obs_f = t_self_f + N_self*dt_F + M_self*dt_S + dt_F
%
%  where the last term accounts for the overhead of the bof/eof
%  handler for the function itself.
%
%  ----------------------------------------------------------------
%
%  The values dF, dS, dt_S, and dt_F can be determined as follows:
%
%  With no handlers in place, t can be accurately measured and can be
%  considered t be a known quantity.  Suppose the statement has no
%  function calls and let that statement be executed a very large
%  number of times B with handlers in place.  Then the elapsed time
%  t_elapsed as given by the wallclock will be given by
%
%     t_elapsed = B*(t+dS)   ===> dS = (t_elapsed - B*t)/B
%
%  The total measured or observed time t_obs, as given by the
%  profiler's starting and stopping of the clock upon each execution
%  will be given by
%
%     t_obs = B*(t+dt_S)      ===> dt_S = (t_obs - B*t)/B
%
%  If the statement is just a function call to a function that does
%  nothing, then
%
%     t_elapsed = B*(t + dS + dF)
%
%  Using the prior determination of dS, we obtain
%
%     dF = t_elapsed/B - t - dS
%
%  For just a single function call with no arguments, the self-time is
%  defined to be 0.  So:
%
%     t_self_obs = 0 + 1*dt_F + dt_S  ==> dt_F = t_self_obs-dt_S
%
private variable Overhead_Per_Statement = 0.0;   %  dS
private variable Error_Per_Statement = 0.0;   %  dt_S
private variable Overhead_Per_Function = 0.0;   %  dF
private variable Error_Per_Function = 0.0;   %  dt_F

profile_off ();

private define cal_nop_0 ();
private define cal_f_0 (n)
{
   loop (n)
     {
	cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0();
	cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0();
	cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0();
	cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0(); cal_nop_0();
     }
}

private define cal_s_0 (n)
{
   loop (n)
     {
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
     }
}

profile_on (1);

private define cal_nop_1 ();
private define cal_f_1 (n)
{
   loop (n)
     {
	cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1();
	cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1();
	cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1();
	cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1(); cal_nop_1();
     }
}
private define cal_s_1 (n)
{
   loop (n)
     {
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
	() = 1;	() = 1;	() = 1;	() = 1; () = 1;
     }
}
profile_off ();

#ifnexists sum
private define sum (x)
{
   variable s = 0.0;
   foreach (x) s += ();
   return s;
}
#endif

define profile_calibrate ()
{
#ifexists __DEBUG_PROFILE_
   return;
#endif
   if (_NARGS == 0) 1000;
   variable n1 = ();
   variable n0 = 100*n1;

   tic();
   cal_s_0 (n0);
   variable t0 = toc;
   variable t_expected = ((t0*n1)/n0); %  when done n1 times

   profile_begin (1);
   tic ();
   cal_s_1 (n1);
   variable t_elapsed = toc ();
   profile_end ();

   variable s = convert_profile_info_to_array (L_Info_Table);
   variable num = sum (s.num_called);
   variable t_obs = sum(s.cum_time);
   Error_Per_Statement = (t_obs - t_expected)/num;
   Overhead_Per_Statement = (t_elapsed-t_expected)/num;

   % Now calibrate the function call overhead
   tic();
   cal_f_0(n0);
   t0 = toc;
   t_expected = ((t0*n1)/n0); %  when done n1 times

   profile_begin (1);
   tic ();
   cal_f_1 (n1);
   t_elapsed = toc ();
   profile_end ();

   %variable f = convert_func_info_to_array ();
   s = convert_profile_info_to_array (L_Info_Table);
   num = sum (s.num_called);
   Overhead_Per_Function = (t_elapsed - t_expected)/num - Overhead_Per_Statement;
   t_obs = sum(s.self_time);
   Error_Per_Function = t_obs/num - Error_Per_Statement;
   if (Error_Per_Function < 0)
     Error_Per_Function = 0.0;
}

private define profile_report_lines (fp, s)
{
   s.cum_time -= (Error_Per_Statement * (s.num_called + s.num_s_triggered)
		  + s.num_f_triggered * Error_Per_Function);
   s.self_time -= (Error_Per_Statement * s.num_called
		   + s.num_self_f * Error_Per_Function);

   variable rates = s.cum_time/s.num_called;
   variable i = array_sort (s.self_time);
   %variable i = array_sort (s.cum_time);
   array_reverse (i);
   rates = rates[i];
   variable num_called = s.num_called[i];
   variable self_time = s.self_time[i];
   variable cum_time = s.cum_time[i];
   variable num_self_f = s.num_self_f[i];
   variable num_s_triggered = s.num_s_triggered[i];
   variable num_f_triggered = s.num_f_triggered[i];
   variable name = s.name[i];

#iffalse
   variable total_counts = sum(num_called);
   variable total_calls = sum(num_self_f);
   () = fprintf (fp, "#Number of profiled statements executed: %g\n", total_counts);
   () = fprintf (fp, "#Number of profiled function calls: %g\n", total_calls);
   () = fprintf (fp, "#Profiler Overhead per statement: %g ms\n", Overhead_Per_Statement*1e3);
   () = fprintf (fp, "#Profiler Overhead per function : %g ms\n", Overhead_Per_Function*1e3);
   () = fprintf (fp, "#Profiler Error per statement   : %g ms\n", Error_Per_Statement*1e3);
   () = fprintf (fp, "#Profiler Error per function    : %g ms\n", Error_Per_Function*1e3);
   () = fprintf (fp, "#Total Profiler Overhead: %g secs\n",
		 total_counts * Overhead_Per_Statement + total_calls*Overhead_Per_Function);
   () = fprintf (fp, "\n\n");
#endif

   () = fprintf (fp, "#ncalls      ms/call totalselfms     totalsecs  Fcalls   Scalls File:line\n");
   %                  1234567 1234567890AB 1234567890AB 1234567890AB 1234567  1234567
   _for i (0, length(rates)-1, 1)
     {
	() = fprintf (fp, "%7d %12.5f %12.5f %12.5f %7d %7d %s\n",
		      num_called[i], rates[i]*1e3, self_time[i]*1e3,
		      cum_time[i], num_f_triggered[i], num_s_triggered[i], name[i]);
     }
}

private define profile_report_funcs (fp, s)
{
#ifnfalse
   s.cum_time -= (Error_Per_Statement * s.num_s_triggered
		  + s.num_f_triggered * Error_Per_Function);
   s.self_time -= (Error_Per_Statement * s.num_self_s
		   + s.num_self_f * Error_Per_Function);
#endif
   variable rates = s.cum_time/s.num_called;
   variable i = array_sort (s.self_time);
   %variable i = array_sort (s.cum_time);
   array_reverse (i);
   rates = rates[i];
   variable num_called = s.num_called[i];
   variable self_time = s.self_time[i];
   variable cum_time = s.cum_time[i];
   variable num_self_f = s.num_self_f[i];
   variable num_self_s = s.num_self_s[i];
   variable num_s_triggered = s.num_s_triggered[i];
   variable num_f_triggered = s.num_f_triggered[i];
   variable name = s.name[i];

   () = fprintf (fp, "#function                 ncalls      ms/call  totalselfms    totalsecs Function File\n");
   %                  123456789012345678901234 1234567 1234567890AB 1234567890AB 1234567890AB
   _for i (0, length(rates)-1, 1)
     {
	variable func_file = name[i];
	variable j = is_substr (func_file, ":");
	variable func = substr (func_file, 1, j-1);
	variable file = substr (func_file, j+1, -1);
	() = fprintf (fp, "%-24s %7d %12.4f %12.4f %12.4f %7d %7d %s\n",
		      func,
		      num_called[i], rates[i]*1e3, self_time[i]*1e3,
		      cum_time[i], num_self_f[i], num_self_s[i],
		      file
		     );
     }
}

private define output_title (fp, title)
{
   variable s = "----------------------------------------------------------------";
   variable spaces = "";
   loop ((strlen(s) - strlen(title))/2)
     spaces = strcat (spaces, " ");
   () = fprintf (fp, "\n#%s\n#%s%s\n#%s\n\n", s, spaces, title, s);;
}

define profile_report (file)
{
   if (0 == __is_initialized (&L_Info_Table))
     return;

   variable fp = file;
   if (typeof (file) == String_Type)
     fp = fopen (fp, "w");

   variable f = convert_profile_info_to_array (F_Info_Table);
   if (length (f.cum_time))
     {
	output_title (fp, "Function Call Profile Report");
	profile_report_funcs (fp, f);
     }

   variable s = convert_profile_info_to_array (L_Info_Table);
   if (length (s.cum_time))
     {
	output_title (fp, "Line by Line Profile Report");
	profile_report_lines (fp, s);
     }

   ifnot (_eqs (fp,file))
     () = fclose (fp);
}

provide ("profile");
