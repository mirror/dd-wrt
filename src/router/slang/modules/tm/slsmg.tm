\function{slsmg_suspend_smg}
\usage{slsmg_suspend_smg ()}
\done

\function{slsmg_resume_smg}
\usage{slsmg_resume_smg ()}
\done

\function{slsmg_erase_eol}
\usage{slsmg_erase_eol ()}
\done

\function{slsmg_gotorc}
\usage{slsmg_gotorc (Integer_Type r, c)}
\done

\function{slsmg_erase_eos}
\usage{slsmg_erase_eos ()}
\done

\function{slsmg_reverse_video}
\usage{slsmg_reverse_video ()}
\done

\function{slsmg_set_color}
\usage{slsmg_set_color (Integer_Type c)}
\done

\function{slsmg_normal_video}
\usage{slsmg_normal_video ()}
\done

\function{slsmg_write_string}
\usage{slsmg_write_string (String_Type s)}
\done

\function{slsmg_cls}
\usage{slsmg_cls ()}
\done

\function{slsmg_refresh}
\usage{slsmg_refresh ()}
\done

\function{slsmg_reset_smg}
\usage{slsmg_reset_smg ()}
\done

\function{slsmg_init_smg}
\usage{slsmg_init_smg ()}
\done

\function{slsmg_write_nstring}
\usage{slsmg_write_nstring (String_Type s, Integer_Type len)}
\done

\function{slsmg_write_wrapped_string}
\usage{slsmg_write_wrapped_string (String_Type s, Integer_Type r, c, dr, dc, fill)}
\done

\function{slsmg_char_at}
\usage{Integer_Type slsmg_char_at ()}
\done

\function{slsmg_set_screen_start}
\usage{slsmg_set_screen_start (Integer_Type r, c)}
\done

\function{slsmg_draw_hline}
\usage{slsmg_draw_hline (Integer_Type dn)}
\done

\function{slsmg_draw_vline}
\usage{slsmg_draw_vline (Integer_Type dn)}
\done

\function{slsmg_draw_object}
\usage{slsmg_draw_object (Integer_Type r, c, obj)}
\done

\function{slsmg_draw_box}
\usage{slsmg_draw_box (Integer_Type r, c, dr, dc)}
\done

\function{slsmg_get_column}
\usage{Integer_Type slsmg_get_column ()}
\done

\function{slsmg_get_row}
\usage{Integer_Type slsmg_get_row ()}
\done

\function{slsmg_forward}
\usage{slsmg_forward (Integer_Type n)}
\done

\function{slsmg_set_color_in_region}
\usage{slsmg_set_color_in_region (Integer_Type color, r, c, dr, dc)}
\done

\function{slsmg_define_color}
\usage{slsmg_define_color (Integer_Type obj, String_Type fg, bg)}
\description
  \exmp{fg} and \exmp{bg} colors can be one of the following strings:
#v+
  "color0" or "black",      "color8"  or "gray",
  "color1" or "red",        "color9"  or "brightred",
  "color2" or "green",      "color10" or "brightgreen",
  "color3" or "brown",      "color11" or "yellow",
  "color4" or "blue",       "color12" or "brightblue",
  "color5" or "magenta",    "color13" or "brightmagenta",
  "color6" or "cyan",       "color14" or "brightcyan",
  "color7" or "lightgray",  "color15" or "white"
#v-
\done

\function{slsmg_write_to_status_line}
\usage{slsmg_write_to_status_line (String_Type s)}
\done
