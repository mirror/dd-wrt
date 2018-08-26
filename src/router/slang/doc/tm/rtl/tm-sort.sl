#! /usr/bin/env slsh

private variable Data;

private define init ()
{
   Data = Assoc_Type[String_Type];
}

private define warning ()
{
   variable args = __pop_args (_NARGS);
   () = fprintf (stderr, "***WARNING: %s\n", sprintf (__push_args (args)));
}

private define process_function (line, fp)
{
   variable fname;
   variable lines;

   fname = strtrim (strtok (line, "{}")[1]);

   lines = line;
#iftrue
   foreach (fp)
     {
	line = ();
	lines = strcat (lines, line);
	if (0 == strncmp ("\\done", line, 5))
	  break;
     }
#else
   while (-1 != fgets (&line, fp))
     {
	lines += line;
	if (0 == strncmp ("\\done", line, 5))
	  break;
     }
#endif
   if (assoc_key_exists (Data, fname))
     {
	warning ("Key %s already exists", fname);
	return -1;
     }

   Data[fname] = lines;
   return 0;
}

private define process_variable (line, fp)
{
   process_function (line, fp);
}
private define process_datatype (line, fp)
{
   process_function (line, fp);
}

private define read_file_contents (file)
{
   variable fp = fopen (file, "r");
   variable n = 0;
   variable line;

   if (fp == NULL)
     {
	() = fprintf (stderr, "Unable to open %s\n", file);
	return -1;
     }

   %while (-1 != fgets (&line, fp))
   foreach (fp)
     {
	line = ();
	if (0 == strncmp (line, "\\function{", 10))
	  {
	     if (-1 == process_function (line, fp))
	       return -1;

	     continue;
	  }

	if (0 == strncmp (line, "\\variable{", 10))
	  {
	     if (-1 == process_variable (line, fp))
	       return -1;

	     continue;
	  }

	if (0 == strncmp (line, "\\datatype{", 10))
	  {
	     if (-1 == process_datatype (line, fp))
	       return -1;

	     continue;
	  }
     }

   () = fclose (fp);
   return 0;
}

private define sort_keys (a, b)
{
   variable a1 = strup (strtrim_beg (a, "_"));
   variable b1 = strup (strtrim_beg (b, "_"));
   variable ret = strcmp (a1, b1);
   if (ret == 0)
     ret = strcmp (a, b);
   return ret;
}

private define sort_and_write_file_elements (file)
{
   variable fp;
   variable i, keys;
   variable backup_file;

   backup_file = file + ".BAK";
   () = remove (backup_file);
   () = rename (file, backup_file);

   fp = fopen (file, "w");
   if (fp == NULL)
     return -1;

   keys = assoc_get_keys (Data);
   i = array_sort (keys, &sort_keys);

   foreach (keys[i])
     {
	variable k = ();

	() = fputs (Data[k], fp);
	() = fputs ("\n", fp);
     }

   () = fclose (fp);

   return 0;
}

private define process_file (file)
{
   init ();

   () = fprintf (stdout, "Processing %s ...", file);
   () = fflush (stdout);

   if (-1 == read_file_contents (file))
     return -1;

   if (-1 == sort_and_write_file_elements (file))
     return -1;

   () = fputs ("done.\n", stdout);
   return 0;
}

define slsh_main ()
{
   if (__argc < 2)
     {
	() = fprintf (stderr, "Usage: %s files....\n", __argv[0]);
	exit (1);
     }

   foreach (__argv[[1:]])
     process_file ();
}
