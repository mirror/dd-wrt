#!/usr/bin/env slsh
private define check_file (file)
{
   variable infun = 0, inexample = 0, innotes = 0, indescr = 0;
   variable fp = fopen (file, "r");

   variable fun_name = "";
   variable line, strs;
   variable lineno = 0;
   while (-1 != fgets (&line, fp))
     {
	lineno++;

	if (line[0] != '\\')
	  {
	     if (inexample) inexample++;
	     if (innotes) innotes++;
	     if (indescr) indescr++;
	     continue;
	  }

	if (0 == strncmp (line, "\function{"R, 10))
	  {
	     strs = string_matches (line, "\\function{\([^}]*\)}"R, 1);
	     if (strs == NULL)
	       {
		  () = fprintf (stderr, "%s:%d:Bad function start after function %s\n",
				file, lineno, fun_name);
		  continue;
	       }
	     if (infun)
	       {
		  () = fprintf (stderr, "%s:%d:Missing \\done for function %s\n",
				file, lineno, fun_name);
		  infun = 0;
	       }
	     fun_name = strs[1];
	     infun++;
	     inexample=0;
	     innotes=0;
	     indescr=0;
	     continue;
	  }

	if (inexample == 1)
	  {
	     () = fprintf (stderr, "%s:%d:Empty Example in %s\n",
			   file, lineno, fun_name);
	     inexample = 0;
	  }
	if (innotes == 1)
	  {
	     () = fprintf (stderr, "%s:%d:Empty Notes in %s\n",
			   file, lineno, fun_name);
	     innotes = 0;
	  }
	if (indescr == 1)
	  {
	     () = fprintf (stderr, "%s:%d:Empty Description in %s\n",
			   file, lineno, fun_name);
	     indescr = 0;
	  }

	if (0 == strncmp (line, "\example"R, 8))
	  {
	     inexample++;
	     continue;
	  }

	if (0 == strncmp (line, "\description"R, 12))
	  {
	     indescr++;
	     continue;
	  }

	if (0 == strncmp (line, "\notes"R, 6))
	  {
	     innotes++;
	     continue;
	  }

	if (strncmp (line, "\done"R, 5))
	  infun = 0;
     }
}

define slsh_main ()
{
   if (__argc < 2)
     {
	() = fprintf (stderr, "Usage: %s files...\n", __argv[0]);
	exit (1);
     }
   variable files = __argv[[1:]];

   foreach (files)
     {
	variable file = ();
	check_file (file);
     }
}
