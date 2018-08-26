\function{SLsmg_fill_region}
\synopsis{Fill a rectangular region with a character}
\usage{void SLsmg_fill_region (r, c, nr, nc, ch)}
#v+
    int r
    int c
    unsigned int nr
    unsigned int nc
    unsigned char ch
#v-
\description
  The \var{SLsmg_fill_region} function may be used to a
  rectangular region with the character \var{ch} in the current color.
  The rectangle's upper left corner is at row \var{r} and column
  \var{c}, and spans \var{nr} rows and \var{nc} columns.  The position
  of the virtual cursor will be left at (\var{r}, \var{c}).
\seealso{SLsmg_write_char, SLsmg_set_color}
\done

\function{SLsmg_set_char_set}
\synopsis{Turn on or off line drawing characters}
\usage{void SLsmg_set_char_set (int a);}
\description
  \var{SLsmg_set_char_set} may be used to select or deselect the line drawing
  character set as the current character set.  If \var{a} is non-zero,
  the line drawing character set will be selected.  Otherwise, the
  standard character set will be selected.
\notes
  There is no guarantee that this function will actually enable the
  use of line drawing characters.  All it does is cause subsequent
  characters to be rendered using the terminal's alternate character
  set.  Such character sets usually contain line drawing characters.
\seealso{SLsmg_write_char, SLtt_get_terminfo}
\done

\variable{int SLsmg_Scroll_Hash_Border;}
\synopsis{Set the size of the border for the scroll hash}
\usage{int SLsmg_Scroll_Hash_Border = 0;}
\description
  This variable may be used to ignore the characters that occur at the
  beginning and the end of a row when performing the hash calculation
  to determine whether or not a line has scrolled.  The default value
  is zero which means that all the characters on a line will be used.
\seealso{SLsmg_refresh}
\done

\function{SLsmg_suspend_smg}
\synopsis{Suspend screen management}
\usage{int SLsmg_suspend_smg (void)}
\description
  \var{SLsmg_suspend_smg} can be used to suspend the state of the
  screen management facility during suspension of the program.  Use of
  this function will reset the display back to its default state.  The
  funtion \var{SLsmg_resume_smg} should be called after suspension.

  It returns zero upon success, or \-1 upon error.

  This function is similar to \var{SLsmg_reset_smg} except that the
  state of the display prior to calling \var{SLsmg_suspend_smg} is saved.
\seealso{SLsmg_resume_smg, SLsmg_reset_smg}
\done

\function{SLsmg_resume_smg}
\synopsis{Resume screen management}
\usage{int SLsmg_resume_smg (void)}
\description
  \var{SLsmg_resume_smg} should be called after
  \var{SLsmg_suspend_smg} to redraw the display exactly like it was
  before \var{SLsmg_suspend_smg} was called.  It returns zero upon
  success, or \-1 upon error.
\seealso{SLsmg_suspend_smg}
\done

\function{SLsmg_erase_eol}
\synopsis{Erase to the end of the row}
\usage{void SLsmg_erase_eol (void);}
\description
  \var{SLsmg_erase_eol} erases all characters from the current
  position to the end of the line.  The newly created space is given
  the color of the current color.  This function has no effect on the
  position of the virtual cursor.
\seealso{SLsmg_gotorc, SLsmg_erase_eos, SLsmg_fill_region}
\done

\function{SLsmg_gotorc}
\synopsis{Move the virtual cursor}
\usage{void SLsmg_gotorc (int r, int c)}
\description
  The \var{SLsmg_gotorc} function moves the virtual cursor to the row
  \var{r} and column \var{c}.  The first row and first column is
  specified by \exmp{r = 0} and \exmp{c = 0}.
\seealso{SLsmg_refresh}
\done

\function{SLsmg_erase_eos}
\synopsis{Erase to the end of the screen}
\usage{void SLsmg_erase_eos (void);}
\description
  The \var{SLsmg_erase_eos} is like \var{SLsmg_erase_eol} except that
  it erases all text from the current position to the end of the
  display.  The current color will be used to set the background of
  the erased area.
\seealso{SLsmg_erase_eol}
\done

\function{SLsmg_reverse_video}
\synopsis{Set the current color to 1}
\usage{void SLsmg_reverse_video (void);}
\description
  This function is nothing more than \exmp{SLsmg_set_color(1)}.
\seealso{SLsmg_set_color}
\done

\function{SLsmg_set_color (int)}
\synopsis{Set the current color}
\usage{void SLsmg_set_color (int c);}
\description
  \var{SLsmg_set_color} is used to set the current color.  The
  parameter \var{c} is really a color object descriptor.  Actual
  foreground and background colors as well as other visual attributes
  may be associated with a color descriptor via the
  \var{SLtt_set_color} function.
\example
  This example defines color \exmp{7} to be green foreground on black
  background and then displays some text in this color:
#v+
      SLtt_set_color (7, NULL, "green", "black");
      SLsmg_set_color (7);
      SLsmg_write_string ("Hello");
      SLsmg_refresh ();
#v-
\notes
  It is important to understand that the screen managment routines
  know nothing about the actual colors associated with a color
  descriptor.  Only the descriptor itself is used by the \var{SLsmg}
  routines.  The lower level \var{SLtt} interface converts the color
  descriptors to actual colors.  Thus
#v+
      SLtt_set_color (7, NULL, "green", "black");
      SLsmg_set_color (7);
      SLsmg_write_string ("Hello");
      SLtt_set_color (7, NULL, "red", "blue");
      SLsmg_write_string ("World");
      SLsmg_refresh ();
#v-
  will result in \exmp{"hello"} displayed in red on blue and \em{not}
  green on black.
\seealso{SLtt_set_color, SLtt_set_color_object}
\done

\function{SLsmg_normal_video}
\synopsis{Set the current color to 0}
\usage{void SLsmg_normal_video (void);}
\description
  \var{SLsmg_normal_video} sets the current color descriptor to \var{0}.
\seealso{SLsmg_set_color}
\done

\function{SLsmg_printf}
\synopsis{Format a string on the virtual display}
\usage{void SLsmg_printf (char *fmt, ...)}
\description
  \var{SLsmg_printf} format a \var{printf} style variable argument
  list and writes it on the virtual display.  The virtual cursor will
  be moved to the end of the string.
\seealso{SLsmg_write_string, SLsmg_vprintf}
\done

\function{SLsmg_vprintf}
\synopsis{Format a string on the virtual display}
\usage{void SLsmg_vprintf (char *fmt, va_list ap)}
\description
  \var{SLsmg_vprintf} formats a string in the manner of \em{vprintf}
  and writes the result to the display.  The virtual cursor is
  advanced to the end of the string.
\seealso{SLsmg_write_string, SLsmg_printf}
\done

\function{SLsmg_write_string}
\synopsis{Write a character string on the display }
\usage{void SLsmg_write_string (char *s)}
\description
  The function \var{SLsmg_write_string} displays the string \var{s} on
  the virtual display at the current position and moves the position
  to the end of the string.
\seealso{SLsmg_printf, SLsmg_write_nstring}
\done

\function{SLsmg_write_nstring}
\synopsis{Write the first n characters of a string on the display}
\usage{void SLsmg_write_nstring (char *s, unsigned int n);}
\description
  \var{SLsmg_write_nstring} writes the first \var{n} characters of
  \var{s} to this virtual display.  If the length of the string
  \var{s} is less than \var{n}, the spaces will used until
  \var{n} characters have been written.  \var{s} can be \var{NULL}, in
  which case \var{n} spaces will be written.
\seealso{SLsmg_write_string, SLsmg_write_nchars}
\done

\function{SLsmg_write_char}
\synopsis{Write a character to the virtual display}
\usage{void SLsmg_write_char (char ch);}
\description
  \var{SLsmg_write_char} writes the character \var{ch} to the virtual
  display.
\seealso{SLsmg_write_nchars, SLsmg_write_string}
\done

\function{SLsmg_write_nchars}
\synopsis{Write n characters to the virtual display}
\usage{void SLsmg_write_nchars (char *s, unsigned int n);}
\description
  \var{SLsmg_write_nchars} writes at most \var{n} characters from the
  string \var{s} to the display.  If the length of \var{s} is less
  than \var{n}, the whole length of the string will get written.

  This function differs from \var{SLsmg_write_nstring} in that
  \var{SLsmg_write_nstring} will pad the string to write exactly
  \var{n} characters.  \var{SLsmg_write_nchars} does not perform any
  padding.
\seealso{SLsmg_write_nchars, SLsmg_write_nstring}
\done

\function{SLsmg_write_wrapped_string}
\synopsis{Write a string to the display with wrapping}
\usage{void SLsmg_write_wrapped_string (s, r, c, nr, nc, fill)}
#v+
    char *s
    int r, c
    unsigned int nr, nc
    int fill
#v-
\description
  \var{SLsmg_write_wrapped_string} writes the string \var{s} to the
  virtual display.  The string will be confined to the rectangular
  region whose upper right corner is at row \var{r} and column \var{c},
  and consists of \var{nr} rows and \var{nc} columns.  The string will
  be wrapped at the boundaries of the box.  If \var{fill} is non-zero,
  the last line to which characters have been written will get padded
  with spaces.
\notes
  This function does not wrap on word boundaries.  However, it will
  wrap when a newline charater is encountered.
\seealso{SLsmg_write_string}
\done

\function{SLsmg_cls}
\synopsis{Clear the virtual display}
\usage{void SLsmg_cls (void)}
\description
  \var{SLsmg_cls} erases the virtual display using the current color.
  This will cause the physical display to get cleared the next time
  \var{SLsmg_refresh} is called.
\notes
  This function is not the same as
#v+
     SLsmg_gotorc (0,0); SLsmg_erase_eos ();
#v-
  since these statements do not guarantee that the physical screen
  will get cleared.
\seealso{SLsmg_refresh, SLsmg_erase_eos}
\done

\function{SLsmg_refresh}
\synopsis{Update physical screen}
\usage{void SLsmg_refresh (void)}
\description
  The \var{SLsmg_refresh} function updates the physical display to
  look like the virtual display.
\seealso{SLsmg_suspend_smg, SLsmg_init_smg, SLsmg_reset_smg}
\done

\function{SLsmg_touch_lines}
\synopsis{Mark lines on the virtual display for redisplay}
\usage{void SLsmg_touch_lines (int r, unsigned int nr)}
\description
  \var{SLsmg_touch_lines} marks the \var{nr} lines on the virtual
  display starting at row \var{r} for redisplay upon the next call to
  \var{SLsmg_refresh}.
\notes
  This function should rarely be called, if ever.  If you find that
  you need to call this function, then your application should be
  modified to properly use the \var{SLsmg} screen management routines.
  This function is provided only for curses compatibility.
\seealso{SLsmg_refresh}
\done

\function{SLsmg_init_smg}
\synopsis{Initialize the \var{SLsmg} routines}
\usage{int SLsmg_init_smg (void)}
\description
  The \var{SLsmg_init_smg} function initializes the \var{SLsmg} screen
  management routines.   Specifically, this function allocates space
  for the virtual display and calls \var{SLtt_init_video} to put the
  terminal's physical display in the proper state.  It is up to the
  caller to make sure that the \var{SLtt} routines are initialized via
  \var{SLtt_get_terminfo} before calling \var{SLsmg_init_smg}.

  This function should also be called any time the size of the
  physical display has changed so that it can reallocate a new virtual
  display to match the physical display.

  It returns zero upon success, or \-1 upon failure.
\seealso{SLsmg_reset_smg}
\done

\function{SLsmg_reset_smg}
\synopsis{Reset the \var{SLsmg} routines}
\usage{int SLsmg_reset_smg (void);}
\description
  \var{SLsmg_reset_smg} resets the \var{SLsmg} screen management
  routines by freeing all memory allocated while it was active.  It
  also calls \var{SLtt_reset_video} to put the terminal's display in
  it default state.
\seealso{SLsmg_init_smg}
\done

\function{SLsmg_char_at}
\synopsis{Get the character at the current position on the virtual display}
\usage{unsigned short SLsmg_char_at(void)}
\description
  The \var{SLsmg_char_at} function returns the character and its color
  at the current position on the virtual display.
\seealso{SLsmg_read_raw, SLsmg_write_char}
\done

\function{SLsmg_set_screen_start}
\synopsis{Set the origin of the virtual display}
\usage{void SLsmg_set_screen_start (int *r, int *c)}
\description
  \var{SLsmg_set_screen_start} sets the origin of the virtual display
  to the row \var{*r} and the column \var{*c}.  If either \var{r} or \var{c}
  is \var{NULL}, then the corresponding value will be set to \var{0}.
  Otherwise, the location specified by the pointers will be updated to
  reflect the old origin.

  See \tt{slang/demo/pager.c} for how this function may be used to
  scroll horizontally.
\seealso{SLsmg_init_smg}
\done

\function{SLsmg_draw_hline}
\synopsis{Draw a horizontal line}
\usage{void SLsmg_draw_hline (unsigned int len)}
\description
  The \var{SLsmg_draw_hline} function draws a horizontal line of
  length \var{len} on the virtual display.  The position of the
  virtual cursor is left at the end of the line.
\seealso{SLsmg_draw_vline}
\done

\function{SLsmg_draw_vline}
\synopsis{Draw a vertical line}
\usage{void SLsmg_draw_vline (unsigned int len);}
\description
  The \var{SLsmg_draw_vline} function draws a vertical line of
  length \var{len} on the virtual display.  The position of the
  virtual cursor is left at the end of the line.
\seealso{??}
\done

\function{SLsmg_draw_object}
\synopsis{Draw an object from the alternate character set}
\usage{void SLsmg_draw_object (int r, int c, unsigned char obj)}
\description
  The \var{SLsmg_draw_object} function may be used to place the object
  specified by \var{obj} at row \var{r} and column \var{c}.  The
  object is really a character from the alternate character set and
  may be specified using one of the following constants:
#v+
    SLSMG_HLINE_CHAR         Horizontal line
    SLSMG_VLINE_CHAR         Vertical line
    SLSMG_ULCORN_CHAR        Upper left corner
    SLSMG_URCORN_CHAR        Upper right corner
    SLSMG_LLCORN_CHAR        Lower left corner
    SLSMG_LRCORN_CHAR        Lower right corner
    SLSMG_CKBRD_CHAR         Checkboard character
    SLSMG_RTEE_CHAR          Right Tee
    SLSMG_LTEE_CHAR          Left Tee
    SLSMG_UTEE_CHAR          Up Tee
    SLSMG_DTEE_CHAR          Down Tee
    SLSMG_PLUS_CHAR          Plus or Cross character
#v-
\seealso{SLsmg_draw_vline, SLsmg_draw_hline, SLsmg_draw_box}
\done

\function{SLsmg_draw_box}
\synopsis{Draw a box on the virtual display}
\usage{void SLsmg_draw_box (int r, int c, unsigned int dr, unsigned int dc)}
\description
  \var{SLsmg_draw_box} uses the \var{SLsmg_draw_hline} and
  \var{SLsmg_draw_vline} functions to draw a rectangular box on the
  virtual display.  The box's upper left corner is placed at row
  \var{r} and column \var{c}.  The width and length of the box is
  specified by \var{dc} and \var{dr}, respectively.
\seealso{SLsmg_draw_vline, SLsmg_draw_hline, SLsmg_draw_object}
\done

\function{SLsmg_set_color_in_region}
\synopsis{Change the color of a specifed region}
\usage{void SLsmg_set_color_in_region (color, r, c, dr, dc)}
#v+
  int color;
  int r, c;
  unsigned int dr, dc;
#v-
\description
  \var{SLsmg_set_color_in_region} may be used to change the color of a
  rectangular region whose upper left corner is given by
  (\var{r},\var{c}), and whose width and height is given by \var{dc}
  and \var{dr}, respectively.  The color of the region is given by the
  \var{color} parameter.
\seealso{SLsmg_draw_box, SLsmg_set_color}
\done

\function{SLsmg_get_column}
\synopsis{Get the column of the virtual cursor}
\usage{int SLsmg_get_column(void);}
\description
  The \var{SLsmg_get_column} function returns the current column of
  the virtual cursor on the virtual display.
\seealso{SLsmg_get_row, SLsmg_gotorc}
\done

\function{SLsmg_get_row}
\synopsis{Get the row of the virtual cursor}
\usage{int SLsmg_get_row(void);}
\description
  The \var{SLsmg_get_row} function returns the current row of the
  virtual cursor on the virtual display.
\seealso{SLsmg_get_column, SLsmg_gotorc}
\done

\function{SLsmg_forward}
\synopsis{Move the virtual cursor forward n columns}
\usage{void SLsmg_forward (int n);}
\description
  The \var{SLsmg_forward} function moves the virtual cursor forward
  \var{n} columns.
\seealso{SLsmg_gotorc}
\done

\function{SLsmg_write_color_chars}
\synopsis{Write characters with color descriptors to virtual display}
\usage{void SLsmg_write_color_chars (unsigned short *s, unsigned int len)}
\description
  The \var{SLsmg_write_color_chars} function may be used to write
  \var{len} characters, each with a different color descriptor to the
  virtual display.  Each character and its associated color are
  encoded as an \exmp{unsigned short} such that the lower eight bits
  form the character and the next eight bits form the color.
\seealso{SLsmg_char_at, SLsmg_write_raw}
\done

\function{SLsmg_read_raw}
\synopsis{Read characters from the virtual display}
\usage{unsigned int SLsmg_read_raw (SLsmg_Char_Type *buf, unsigned int len)}
\description
  \var{SLsmg_read_raw} attempts to read \var{len} characters from the
  current position on the virtual display into the buffer specified by
  \var{buf}.  It returns the number of characters actually read.  This
  number will be less than \var{len} if an attempt is made to read
  past the right margin of the display.
\notes
  The purpose of the pair of functions, \var{SLsmg_read_raw} and
  \var{SLsmg_write_raw}, is to permit one to copy the contents of one
  region of the virtual display to another region.
\seealso{SLsmg_char_at, SLsmg_write_raw}
\done

\function{SLsmg_write_raw}
\synopsis{Write characters directly to the virtual display}
\usage{unsigned int SLsmg_write_raw (unsigned short *buf, unsigned int len)}
\description
  The \var{SLsmg_write_raw} function attempts to write \var{len}
  characters specified by \var{buf} to the display at the current
  position.  It returns the number of characters successfully written,
  which will be less than \var{len} if an attempt is made to write
  past the right margin.
\notes
  The purpose of the pair of functions, \var{SLsmg_read_raw} and
  \var{SLsmg_write_raw}, is to permit one to copy the contents of one
  region of the virtual display to another region.
\seealso{SLsmg_read_raw}
\done

