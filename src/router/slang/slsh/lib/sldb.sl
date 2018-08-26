require ("sldbcore");

private define vmessage_method ()
{
   variable args = __pop_args (_NARGS);
   () = fprintf (stdout, __push_args (args));
   () = fflush (stdout);
}

private define open_file_at_linenum (file, linenum)
{
   if (path_extname (file) == ".slc")
     file = path_sans_extname (file) + ".sl";

   variable fp = fopen (file, "r");
   if (fp == NULL)
     {
	vmessage_method ("Unable to open %s\n", file);
	return NULL;
     }
   if (linenum == 1)
     return fp;

   foreach (fp) using ("line")
     {
	variable line = ();
	linenum--;
	if (linenum == 1)
	  break;
     }
   return fp;
}

private define list_method (file, linemin, linemax)
{
   variable n = linemax - linemin + 1;
   foreach (open_file_at_linenum (file, linemin))
     {
	variable line = ();
	vmessage_method ("%d\t%s", linemin, line);
	%vmessage_method ("> %s:%d %s", file, linemin, line);
	linemin++;
	if (linemin > linemax)
	  break;
     }
}

#ifexists slsh_readline
slsh_readline_init ("SLDB");
private variable Rline = NULL;
private define close_readline ()
{
   Rline = NULL;
}
Rline = slsh_readline_new ("sldb");
atexit (&close_readline);
#endif

private define read_input_method (prompt, default_cmd)
{
   variable line;
   forever
     {
	try
	  {
#ifexists slsh_readline
	     line = slsh_readline (Rline, prompt);
#else
	     () = fputs (prompt, stdout); () = fflush(stdout);
	     if (-1 == fgets (&line, stdin))
	       line = NULL;
#endif
	  }
	catch UserBreakError: continue;
	if (line == NULL)
	  break;

	line = strtrim (line, "\t \n");
	if (line == "")
	  {
	     if (default_cmd != NULL)
	       return default_cmd;
	     continue;
	  }
	break;
     }
   return line;
}

define sldb_initialize ()
{
   variable m = sldb_methods ();
   m.list = &list_method;
   m.vmessage = &vmessage_method;
   m.read_input = &read_input_method;
   m.pprint = &print;
}

provide ("sldb");
