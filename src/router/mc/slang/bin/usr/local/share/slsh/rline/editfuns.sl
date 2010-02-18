
define rline_beep ()
{
   () = fputs ("\007", stdout);
   () = fflush (stdout);
}

define rline_bol ()
{
   rline_call ("bol");
}

define rline_eol ()
{
   rline_call ("eol");
}

define rline_delete_line ()
{
   rline_set_line ("");
}

define rline_deleol ()
{
   rline_call ("deleol");
}

define rline_left ()
{
   rline_call ("left");
}

define rline_right ()
{
   rline_call ("right");
}

define rline_bdel ()
{
   rline_call ("bdel");
}


define rline_bskip_word ()
{
}

define rline_skip_word ()
{
}


private variable Mark = NULL;
private variable Paste_Buffer = "";

define rline_set_mark ()
{
   Mark = rline_get_point ();
}

private define get_mark (mp)
{
   if (Mark == NULL)
     {
	rline_beep ();
	return -1;
     }
   @mp = Mark;
   Mark = NULL;
   return 0;
}

private define get_region (mp, pp)
{
   variable m;
   if (-1 == get_mark (&m))
     return -1;

   variable p = rline_get_point ();
   if (p < m)
     (p, m) = (m, p);
   @mp = m;
   @pp = p;
   return 0;
}

private define copy_region_to_pastebuffer (m, p)
{
   variable line = rline_get_line ();
   Paste_Buffer = substrbytes (line, m+1, (p-m));
}

define rline_copy_region ()
{
   variable m, p;
   if (-1 == get_region (&m, &p))
     return;
   copy_region_to_pastebuffer (m, p);
}

define rline_kill_region ()
{
   variable m, p;
   if (-1 == get_region (&m, &p))
     return;
   
   copy_region_to_pastebuffer (m, p);
   rline_set_point (m);
   rline_del (p-m);
}

define rline_kill_eol ()
{
   rline_set_mark ();
   rline_eol ();
   rline_kill_region ();
}

define rline_kill_bol ()
{
   rline_set_mark ();
   rline_bol ();
   rline_kill_region ();
}

define rline_yank ()
{
   rline_ins (Paste_Buffer);
}
