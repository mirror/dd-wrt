% A simple vi-emulation for SLrline

require ("rline/complete");
require ("rline/editfuns");

private variable Is_Command_Mode = 0;
private variable Last_Command = &rline_beep;

private define vi_enter_command_mode ()
{
   Is_Command_Mode = 1;
}

private define vi_enter_insert_mode ()
{
   Is_Command_Mode = 0;
}

private define self_insert_unless_command_mode ()
{
   Last_Command = &rline_beep;
   if (Is_Command_Mode)
     return 0;

   rline_call ("self_insert");
   return 1;
}

private define vi_insert_mode ()
{
   if (self_insert_unless_command_mode ())
     return;

   vi_enter_insert_mode ();
}

private define vi_append_mode ()
{
   if (self_insert_unless_command_mode ())
     return;

   rline_call("right");
   vi_enter_insert_mode ();
}

private define vi_delete_line ()
{
   if (self_insert_unless_command_mode ())
     return;

   rline_delete_line ();
}

private define vi_left ();
private define vi_left ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_left;
   rline_call ("left");
}

private define vi_right ();
private define vi_right ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_right;
   rline_call ("right");
}

private define vi_up ();
private define vi_up ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_up;
   rline_call ("up");
}

private define vi_down ();
private define vi_down ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_down;
   rline_call ("down");
}

private define vi_bol ();
private define vi_bol ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_bol;
   rline_bol ();
}

private define vi_eol ();
private define vi_eol ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_eol;
   rline_eol ();
   rline_left ();
}

private define vi_del ();
private define vi_del ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_del;
   if (rline_eolp ())
     rline_bdel ();
   else
     rline_del (1);
}

private define vi_replace_char ()
{
   if (self_insert_unless_command_mode ())
     return;

   rline_del (1);
   rline_ins (char (-rline_getkey ()));
}

private define vi_bskip_word ();
private define vi_bskip_word ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_bskip_word;
   rline_bskip_word ();
}

private define vi_skip_word ();
private define vi_skip_word ()
{
   if (self_insert_unless_command_mode ())
     return;

   Last_Command = &vi_skip_word;
   rline_skip_word ();
}

private define vi_self_insert ()
{
   if (self_insert_unless_command_mode ())
     return;

   rline_beep ();
}

private define vi_backspace ();
private define vi_backspace ()
{
   if (Is_Command_Mode)
     {
	rline_beep ();
	return;
     }
   Last_Command = &vi_backspace;
   rline_bdel ();
}

private define vi_enter ()
{
   vi_enter_insert_mode ();
   rline_call ("enter");
}

private define vi_repeat_last_command ()
{
   (@Last_Command)();
}

private variable Command_Map = Ref_Type[256];

private define vi_escape ()
{
   vi_enter_command_mode ();
   if (0 == rline_input_pending (1))
     {
	vi_left ();
	return;
     }
   variable ch = rline_getkey ();
   if ((ch == '[') || (ch == 'O'))
     {
	switch (rline_getkey ())
	  { case 'A': vi_up (); return; }
	  { case 'B': vi_down (); return; }
	  { case 'C': vi_right (); return; }
	  { case 'D': vi_left (); return; }

	while (rline_input_pending (1))
	  () = rline_getkey ();

	return;
     }
   if (Command_Map[ch] != NULL)
     (@Command_Map[ch])();
}

foreach ([32:255])
{
   $1 = ();
   rline_setkey (&vi_self_insert, char(-$1));
}

rline_unsetkey ("\e");
rline_setkey (&vi_escape, "\e");

private define set_command_key (ch, f)
{
   Command_Map[ch] = f;
   rline_setkey (f, char(ch));
}

set_command_key ('\x08', &vi_backspace);
set_command_key ('\x7F', &vi_backspace);
set_command_key ('\r', &vi_enter);
set_command_key ('\n', &vi_enter);
set_command_key ('.', &vi_repeat_last_command);
set_command_key ('a', &vi_append_mode);
set_command_key ('i', &vi_insert_mode);
set_command_key ('d', &vi_delete_line);
set_command_key ('h', &vi_left);
set_command_key ('l', &vi_right);
set_command_key ('k', &vi_up);
set_command_key ('j', &vi_down);
set_command_key ('^', &vi_bol);
set_command_key ('$', &vi_eol);
set_command_key ('r', &vi_replace_char);
set_command_key ('x', &vi_del);
set_command_key ('b', &vi_bskip_word);
set_command_key ('w', &vi_skip_word);

