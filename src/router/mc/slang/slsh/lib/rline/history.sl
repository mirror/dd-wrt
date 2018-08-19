% Public functions:
%   
%   rline_load_history (file);
%   rline_save_history (file);
%

variable RLine_Hist_Max_Lines = 50;

variable RLine_History_File = NULL;

define rline_load_history ()
{
   variable file = RLine_History_File;
   if (_NARGS == 1)
     file = ();
   if (file == NULL)
     return;

   variable fp = fopen (file, "r");
   if (fp == NULL)
     return;
   
   variable lines = fgetslines (fp);
   () = fclose (fp);
   lines = array_map (String_Type, &strtrim_end, lines, "\n");
   rline_set_history (lines);
}

define rline_save_history ()
{
   variable file = RLine_History_File;
   if (_NARGS == 1)
     file = ();
   if (file == NULL)
     return;

   variable max_lines = qualifier ("max", RLine_Hist_Max_Lines);
   variable h = rline_get_history ();
   variable n = length (h);
   if (n > max_lines)
     h = h[[n-max_lines:]];
   
   variable fd = open (file, O_WRONLY|O_CREAT|O_TRUNC|O_TEXT, S_IRUSR|S_IWUSR);
   if (fd == NULL)
     return;

   variable fp = fdopen (fd, "w");
   if (fp == NULL)
     return;

   foreach (h)
     {
	variable line = ();
	() = fputs (line, fp);
	() = fputs ("\n", fp);
     }
   () = fflush (fp);
   () = close (fd);
}
