import ("smg", "Global");	       %  Global namespace

static variable Button_Color = 3;
static variable Box_Color = 2;
static variable Normal_Color = 1;

smg_define_color (Button_Color, "white", "green");
smg_define_color (Box_Color, "yellow", "blue");
smg_define_color (Normal_Color, "green", "red");

static define display_button (name, r, c)
{
   smg_gotorc (r, c);
   smg_set_color (Button_Color);
   smg_write_string (" " + name + " ");
   smg_set_color (Normal_Color);
}

static define draw_centered_string (s, r, c, dc)
{
   variable len;
   
   len = strlen (s);
   smg_gotorc (r, c + (dc - len)/2);
   smg_write_string (s);
}

static define get_yes_no_cancel (question)
{
   variable r, c, dr, dc;
   
   dc = strlen (question) + 5;
   dr = 7;

   % We also need room for the yes-no-cancel buttons 
   if (dc < 32) dc = 36;

   r = (Smg_Screen_Rows - dr)/2;
   c = (Smg_Screen_Cols - dc)/2;
   
   smg_set_color (Box_Color);
   smg_draw_box (r, c, dr, dc);
   smg_set_color (Normal_Color);

   r += 2;
   
   draw_centered_string (question + "?", r, c, dc);
   
   r += 2;
   display_button ("Yes", r, c + 4);
   display_button ("No", r, c + 14);
   display_button ("Cancel", r, c + 24);
}

   
smg_write_to_status_line ("smg-module demo");
smg_init_smg ();
smg_set_color(Normal_Color);
smg_erase_eos ();
get_yes_no_cancel ("This demo will exit in 5 seconds");


smg_refresh ();
sleep (5);

smg_write_to_status_line ("");

%smg_reset_smg ();

exit(0);
