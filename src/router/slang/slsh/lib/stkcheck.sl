% This file adds BOS/EOS hooks to the interpreter to help debug stack problems.
% To use it, put the following at the top of a problematic file:
%
%   require ("stkcheck");
%   enable_stack_check ();
%
% To check only portions of a file, surround the suspect functions by:
%
%   enable_stack_check ();
%      .
%      .
%   disable_stack_check ();
%
_boseos_info = 0;
private variable Stack = Struct_Type[512];
private variable Stack_Depth = 0;

private variable Output_Hook = NULL;
private variable Output_Hook_Args = NULL;

define stkcheck_set_output_hook ()
{
   if (_NARGS == 0)
     {
	Output_Hook = NULL;
	Output_Hook_Args = NULL;
	return;
     }

   Output_Hook_Args = __pop_list (_NARGS-1);
   Output_Hook = ();
}

private define output ()
{
   variable args = __pop_list (_NARGS);
   variable str = sprintf (__push_list (args));

   if (Output_Hook != NULL)
     return (@Output_Hook)(__push_list (Output_Hook_Args), str);

   () = fprintf (stderr, __push_list (args));
}

private define bos_handler (file, line)
{
   if (Stack_Depth >= length (Stack))
     throw StackOverflowError, "BOS stack overflow";

   variable s = struct
     {
	file, line, depth
     };
   s.file = file;
   s.line = line;
   s.depth = _stkdepth ();
   Stack[Stack_Depth] = s;
   Stack_Depth++;
}

private define eos_handler ()
{
   if (Stack_Depth <= 0)
     return;
     %throw StackUnderflowError, "BOS stack depth underflow";
   Stack_Depth--;
   variable s = Stack[Stack_Depth];
   variable depth = _stkdepth ();
   if (depth != s.depth)
     {
	if (depth < s.depth)
	  ;
	else
	  {
	     output ("%s:%d: %d object(s) left on the stack\n", s.file, s.line, depth-s.depth);
	     if ((s.file == "***string***") && (Stack_Depth > 0))
	       {
		  s = Stack[Stack_Depth-1];
		  output (" called from %s:%d\n", s.file, s.line);
	       }
	  }
     }
}

define enable_stack_check ()
{
   ()=_set_bos_handler (&bos_handler);
   ()=_set_eos_handler (&eos_handler);
   Stack_Depth = 0;
   _boseos_info = 1;
}

define disable_stack_check ()
{
   ()=_set_bos_handler (NULL);
   ()=_set_eos_handler (NULL);
   Stack_Depth = 0;
   _boseos_info = 0;
}

#ifexists _jed_version
private define output_to_buffer (buf, str)
{
   variable cbuf = whatbuf ();
   setbuf (buf);
   insert (str);
   setbuf (cbuf);
}
if (BATCH == 0) stkcheck_set_output_hook (&output_to_buffer, "*traceback*");
#endif

provide ("stkcheck");
