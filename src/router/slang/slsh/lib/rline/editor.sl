% Public functions:
%   rline_edit_line
%      Binding this will allow the current line to be
%      edited in an external editor.
%   rline_call_editor
%      A utility function to call the editor
%
autoload ("new_process", "process");

variable RLine_Tmp_Dir;
private define open_tmp_file (prefix, ext)
{
   variable dir, dirs = ["/tmp", "$HOME"$];
   if (__is_initialized (&RLine_Tmp_Dir))
     dirs = [RLine_Tmp_Dir, dirs];

   foreach dir (dirs)
     {
	variable st = stat_file (dir);
	if (st == NULL)
	  continue;
	if (stat_is ("dir", st.st_mode))
	  break;
     }
   then dir = "";

   variable fmt = path_concat (dir, "%s%X%d.%s");
   variable pid = getpid ();
   variable n = 0;
   variable file, fp;

   loop (100)
     {
	n++;
	file = sprintf (fmt, prefix, pid*_time(), n, ext);

	variable fd = open (file, O_WRONLY|O_CREAT|O_TRUNC|O_TEXT, S_IRUSR|S_IWUSR);
	if (fd == NULL)
	  return;

	fp = fdopen (fd, "w");
	if (fp != NULL)
	  return fp, fd, file;
     }
   throw OpenError, "Unable to open a temporary file";
}

private define get_editor ()
{
   variable editor = getenv("VISUAL");
   if (editor == NULL) editor = getenv ("EDITOR");
   if (editor == NULL) editor = "vi";
   return editor;
}

define rline_call_editor (lines, prefix, ext)
{
   variable editor = get_editor ();
   variable file, fp, fd;
   (fp, fd, file) = open_tmp_file (prefix, ext);

   EXIT_BLOCK
     {
	() = remove (file);
     }

   () = array_map (Int_Type, &fputs, lines+"\n", fp);
   () = fclose (fp);

   variable st = stat_file (file);
   if (st == NULL)
     return NULL;

   variable mtime = st.st_mtime;

#ifexists __rline_reset_tty
   __rline_reset_tty ();
#endif
   variable p = new_process ([editor, file]).wait();
#ifexists __rline_init_tty
   __rline_init_tty ();
#endif
   rline_call ("redraw");

   if ((p.exited == 0) || (p.exit_status != 0))
     return NULL;

   st = stat_file (file);
   if ((st == NULL) || (st.st_mtime == mtime))
     return NULL;

   fp = fopen (file, "r");
   if (fp == NULL)
     return NULL;

   lines = fgetslines (fp);
   () = fclose (fp);

   return lines;
}

define rline_edit_line ()
{
   variable lines = rline_get_line ();
   lines = rline_call_editor (lines, "rline", "sl");
   if ((lines == NULL) || (length (lines) == 0))
     return;
   lines = strtrim_end (lines, "\n");
   rline_set_line (strjoin (lines, ""));
}

