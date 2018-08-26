require ("slsmg", "Global");	       %  Global namespace

private variable Button_Color = 3;
private variable Box_Color = 2;
private variable Normal_Color = 1;

slsmg_define_color (Button_Color, "black", "brown");
slsmg_define_color (Box_Color, "yellow", "cyan");
slsmg_define_color (Normal_Color, "lightgray", "blue");

private define display_button (name, r, c)
{
   slsmg_gotorc (r, c);
   slsmg_set_color (Button_Color);
   slsmg_write_string (" " + name + " ");
   slsmg_set_color (Normal_Color);
}

private define draw_centered_string (s, r, c, dc)
{
   variable len;

   len = strlen (s);
   slsmg_gotorc (r, c + (dc - len)/2);
   slsmg_write_string (s);
}

private define get_yes_no_cancel (question)
{
   variable r, c, dr, dc;

   dc = strlen (question) + 5;
   dr = 7;

   % We also need room for the yes-no-cancel buttons
   if (dc < 32) dc = 36;

   r = (SLsmg_Screen_Rows - dr)/2;
   c = (SLsmg_Screen_Cols - dc)/2;

   slsmg_set_color (Box_Color);
   slsmg_draw_box (r, c, dr, dc);
   slsmg_set_color (Normal_Color);

   r += 2;

   draw_centered_string (question + "?", r, c, dc);

   r += 2;
   display_button ("Yes", r, c + 4);
   display_button ("No", r, c + 14);
   display_button ("Cancel", r, c + 24);
}

slsmg_write_to_status_line ("smg-module demo");
slsmg_init_smg ();
slsmg_set_color(Normal_Color);
slsmg_erase_eos ();
get_yes_no_cancel ("This demo will exit in 5 seconds");

slsmg_refresh ();
sleep (5);

slsmg_write_to_status_line ("");

%slsmg_reset_smg ();

exit(0);
