require("slsmg");

define slsh_main ()
{
   slsmg_init_smg();
   slsmg_cls();
   variable fg, bg;
   _for fg (0, 15, 1)
     _for bg (0, 15, 1)
       {
	  variable c = 16*fg+bg;
	  slsmg_define_color(c, "color$fg"$, "color$bg"$);
	  slsmg_gotorc(fg, 4*bg);
	  slsmg_set_color(c);
	  slsmg_write_string(sprintf(" %02X ", 16*fg+bg));
       }
   slsmg_set_color(0x70);
   slsmg_write_string(" ");
   slsmg_gotorc(16,0);
   slsmg_refresh();
   variable ch;
   () = fgets (&ch, stdin);
}

