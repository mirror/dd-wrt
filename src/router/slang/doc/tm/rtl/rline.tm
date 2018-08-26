#d notes_comment This function is part of the S-Lang readline interface.

\function{rline_bolp}
\synopsis{Test of the editing point is at the beginning of the line}
\usage{Int_Type rline_bolp()}
\description
  The \ifun{rline_bolp} function returns a non-zero value if the
  current editing position is at the beginning of the line.
\notes
 \notes_comment
\seealso{rline_eolp, rline_get_point, rline_get_line}
\done

\function{rline_eolp}
\synopsis{Test of the editing point is at the end of the line}
\usage{Int_Type rline_eolp()}
\description
  The \ifun{rline_bolp} function returns a non-zero value if the
  current editing position is at the end of the line.
\notes
 \notes_comment
\seealso{rline_bolp, rline_get_point, rline_get_line}
\done

\function{rline_call}
\synopsis{Invoke an internal readline function}
\usage{rline_call (String_Type func)}
\description
 Not all of the readline functions are available directly from the
 \slang interpreter.  For example, the "deleol" function, which
 deletes through the end of the line may be executed using
#v+
    rline_call("deleol");
#v-
  See the documentation for the \ifun{rline_setkey} function for a
  list of internal functions that may be invoked by \ifun{rline_call}.
\notes
 \notes_comment
\seealso{rline_setkey, rline_del, rline_ins}
\done

\function{rline_del}
\synopsis{Delete a specified number of characters at the current position}
\usage{rline_del(Int_Type n)}
\description
 This function delete a specified number of characters at the current
 editing position.  If the number \exmp{n} is less than zero, then the
 previous \exmp{n} characters will be deleted.  Otherwise, the next
 \exmp{n} characters will be deleted.
\notes
 \notes_comment
\seealso{rline_ins, rline_setkey}
\done

\function{rline_get_edit_width}
\synopsis{Get the width of the readline edit window}
\usage{Int_Type rline_get_edit_width ()}
\description
  This function returns the width of the edit window.  For \slsh, this
  number corresponds to the width of the terminal window.
\notes
 \notes_comment
\seealso{rline_ins}
\done

\function{rline_get_history}
\synopsis{Retrieve the readline history}
\usage{Array_Type rline_get_history ()}
\description
  This function returns the readline edit history as an array of
  strings.
\notes
 \notes_comment
\seealso{rline_set_line}
\done

\function{rline_get_line}
\synopsis{Get a copy of the line being edited}
\usage{String_Type rline_get_line ()}
\description
  This function returns the current edit line.
\notes
 \notes_comment
\seealso{rline_set_line, rline_get_history}
\done

\function{rline_get_point}
\synopsis{Get the current editing position}
\usage{Int_Type rline_get_point ()}
\description
 The \ifun{rline_get_point} function returns the byte-offset of the
 current editing position.
\notes
 \notes_comment
\seealso{rline_set_point}
\done

\function{rline_getkey}
\synopsis{Obtain the next byte in the readline input stream}
\usage{Int_Type rline_getkey ()}
\description
  This function returns the next byte in the readline input stream.
  If no byte is available, the function will wait until one is.
\notes
 \notes_comment
\seealso{rline_input_pending, rline_setkey}
\done

\function{rline_input_pending}
\synopsis{Test to see if readline input is available for reading}
\usage{Int_Type rline_input_pending (Int_Type tsecs)}
\description
  This function returns a non-zero value if readline input is
  available to be read.  If none is immediately available, it will
  wait for up to \exmp{tsecs} tenths of a second for input before
  returning.
\notes
 \notes_comment
\seealso{rline_getkey}
\done

\function{rline_ins}
\synopsis{Insert a string at the current editing point}
\usage{rline_ins (String_Type text)}
\description
  This function inserts the specified string into the line being edited.
\notes
 \notes_comment
\seealso{rline_set_line, rline_del}
\done

\function{rline_set_history}
\synopsis{Replace the current history list with a new one}
\usage{rline_set_history (Array_Type lines)}
\description
  The \ifun{rline_set_history} function replaces the current history
  by the specified array of strings.
\notes
 \notes_comment
\seealso{rline_get_history}
\done

\function{rline_set_line}
\synopsis{Replace the current line with a new one}
\usage{rline_set_line (String_Type line)}
\description
  The \ifun{rline_set_line} function replaces the line being edited by
  the specified one.
\notes
 \notes_comment
\seealso{rline_get_line}
\done

\function{rline_set_point}
\synopsis{Move the current editing position to another}
\usage{rline_set_point (Int_Type ofs)}
\description
 The \ifun{rline_set_point} function sets the editing point to the
 specified byte-offset from the beginning of the line.
\notes
 \notes_comment
\seealso{rline_get_point}
\done

\function{rline_setkey}
\synopsis{Bind a key in the readline keymap to a function}
\usage{rline_setkey (func, keyseq)}
\description
  The \ifun{rline_setkey} function binds the function \exmp{func} to
  the specified key sequence \exmp{keyseq}.  The value of \exmp{func}
  may be either a reference to a \slang function, or a string giving
  the name of an internal readline function.

  Functions that are internal to the readline interface include:
#v+
   bdel             Delete the previous character
   bol              Move to the beginning of the line
   complete         The command line completion function
   del              Delete the character at the current position
   delbol           Delete to the beginning of the line
   deleol           Delete through the end of the line
   down             Goto the next line in the history
   enter            Return to the caller of the readline function
   eol              Move to the end of the line
   kbd_quit         Abort editing of the current line
   left             Move left one character
   quoted_insert    Insert the next byte into the line
   redraw           Redraw the line
   right            Move right one character
   self_insert      Insert the byte that invoked the function
   trim             Remove whitespace about the current position
   up               Goto the previous line in the history
#v-
\notes
 \notes_comment
\seealso{rline_unsetkey}
\done

\function{rline_unsetkey}
\synopsis{Unset a key binding from the readline keymap}
\usage{rline_unsetkey (String_Type keyseq)}
\description
  The \ifun{rline_unsetkey} function unbinds the specified key sequence
  from the readline keymap.
\notes
 \notes_comment
\seealso{rline_setkey}
\done

\function{rline_set_list_completions_callback}
\synopsis{Set a callback function to display the list of completions}
\usage{rline_set_list_completions_callback (Ref_Type func)}
\description
  This function sets the \slang function that is to be used to display the
  list of possible completions for current word at the readline prompt.
  The callback function must be defined to accept a single parameter
  representing an array of completion strings.
\example
  This callback function writes the completions using the message
  functions:
#v+
     private define display_completions (strings)
     {
        variable str;
        vmessage ("There are %d completions:\n", length(strings));
        foreach str (strings) vmessage ("%s\n", str);
     }
     rline_set_list_completions_callback (&display_completions);
#v-
\seealso{rline_set_completion_callback}
\done

\function{rline_set_completion_callback}
\synopsis{Set the function to be used for completion at the readline prompt}
\usage{rline_set_completion_callback (Ref_Type func)}
\description
  This function sets the callback function to be used for completion at the
  readline prompt.  The callback function must be defined to accept
  two values, the first being a string containing the text of the line
  being edited, and an integer giving the position of the byte-offset
  into the string where completion was requested.

  The callback function must return two values: an array giving the
  list of possible completion strings, and an integer giving the byte
  offset into the string of the start of the text to be completed.
\example
  See completion-callback function defined in the \slsh library file
  \exmp{rline/complete.sl}.
\notes
 \notes_comment
\seealso{rline_set_list_completions_callback}
\done
