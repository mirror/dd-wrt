\function{print}
\synopsis{Display a string representation of an object or value}
\usage{print (value [,&var|file-pointer|filename])}
\description
 The \sfun{print} function displays the string representation of a
 value to the display.  An optional second argument may be provided to
 specify where to write the resulting string: a variable, an open file
 pointer, or to a file.

 If the string representation of the object appears to contain more
 lines than are available on the screen, then the output will be piped
 to the program given by the \var{PAGER} environment variable.
 Alternatively the pager program may be specified via the \exmp{pager}
 qualifier.
\qualifiers
#v+
   pager[=string]       Force the use of the pager.  If a value is
                         specified, then use it for the pager command.
   nopager              Do not use a pager.
#v-
\example
  Print the string representation of an array to a file called
  \file{array.dat}:
#v+
   print ([1:20:0.1], "array.dat");
#v-

  Print the string represent of an array to a string \exmp{str}:
#v+
   print ([1:20:0.1], &str);
#v-
\seealso{print_set_pager, print_set_pager_lines}
\done

\function{print_set_pager}
\synopsis{Set the name of the pager program used by the print program}
\usage{print_set_pager (String_Type cmd)}
\description
  This function may be used to specify the name of the default pager to be
  used by the \sfun{print} function.
\seealso{print, print_set_pager_lines}
\done

\function{print_set_pager_lines}
\synopsis{Set the maximum number of lines to print before using a pager}
\usage{print_set_pager_lines (Int_Type num)}
\description
  The \sfun{print_set_pager_lines} function sets the maximum number of
  lines that the string representation of an object can be before the
  \sfun{print} function will use a pager.
\seealso{print, print_set_pager}
\done
