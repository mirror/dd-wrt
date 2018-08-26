\function{readascii}
\synopsis{Read data from a text file}
\usage{Int_Type readascii (file, &v1,...&vN ; qualifiers)}
\description
 This function may be used to read formatted data from a text (as
 opposed to binary) file and stores the values as arrays in the
 specified variables \exmp{v1,..., vN} (passed as references).  It
 returns the number of lines read from the file that matched the
 format (implicit or specified by a qualifier).

 The file parameter may be a string that gives the filename to read,
 a \exmp{File_Type} object representing an open file pointer, or an
 array of lines to be scanned.
\qualifiers
 The following qualifiers are supported by the function
#v+
   format=string        The sscanf format string to be used when
                          parsing lines.
   nrows=integer        The maximum number of matching rows to handle.
   ncols=integer        If a single reference is passed, it will be
                          assigned an array of ncols arrays of data values.
   skip=integer         Skip the specified number of lines before
                          scanning
   maxlines=integer     Read no more than this many lines.
   cols=Array_Type      Read only the specified (1-based) columns.
                          Used with an implict format
   delim=string         For an implicit format, use this as a field
                          separator.  The default is whitespace.
   type=string          For an implicit format, use this sscanf type
                          specifier.  Default is %lf (Double_Type).
   size=integer         Use this value as the initial size for the
                          arrays.
   dsize=integer        Use this value as an increment when
                          reallocating the arrays.
   stop_on_mismatch     Stop reading input when a line does not match
                          the format
   lastline=&v          Assign the last line read to the variable v.
   lastlinenum=&v       Assign the last line number (1-based to v)
   comment=string       Lines beginning with this string are ignored.
   as_list              If present, then return data in lists rather
                          than arrays.
#v-
\example
 As a simple example, consider a file called \exmp{imped.dat} containing
#v+
    # Distance    Zr   Zi
    0.0          50.2    0.1
    1.0          47.3  -12.2
    2.0          43.9  -15.8
#v-
  The 3 columns may be read and stored in the variables \exmp{x}, \exmp{zr},
  \exmp{zi} using
#v+
     n = readascii ("imped.dat", &x, &zr, &zi);
#v-
 After return, The value of \exmp{x} will be set to
 \exmp{[0.0,1.0,2.0]}, \exmp{zr} to \exmp{[50.2,47.3,43.9]},
 \exmp{zi} to \exmp{[0.1,-12.2,-15.8]}, and \exmp{n} to 3.

 Another way to read the same data is to use
#v+
    n = readascii ("imped.dat", &data; ncols=3);
#v-
 In this case, \exmp{data} will be \exmp{Array_Type[3]}, with each
 element of the array containing the array of data values for the
 corresponding column.  As before, \exmp{n} will be 3.

 As a more complex example, Consider a file called \exmp{score.dat}
 that contains:
#v+
    Name      Score     Date          Flags
    Bill       73.2     03-Nov-2046    1
    James      22.9     03-Nov-2046    1
    Lucy       89.1     04-Nov-2046    3
#v-
 This file may be read using
#v+
     n = readascii ("score.dat", &name, &score, &date, &flags;
                    format="%s %lf %s %d");
#v-
 In this case, \exmp{n} will be 3, \exmp{name} and \exmp{date} will
 be \dtype{String_Type} arrays, \exmp{score} will be a
 \dtype{Double_Type} array, and \exmp{flags} will be an
 \exmp{Int_Type} array.

 Now suppose that only the score and flags column are of interest.
 The \exmp{name} and \exmp{date} fields may be ignored using
#v+
     n = readascii ("score.dat", &score, &flags";
                    format="%*s %lf %*s %d");
#v-
 Here, \exmp{%*s} indicates that the field is to be parsed as a
 string, but not assigned to a variable.

 Consider the task of reading columns from a file called
 \exmp{books.dat} that contain quoted strings such as:
#v+
     # Year  Author Title
     "1605" "Miguel de Cervantes"  "Don Quixote de la Mancha"
     "1885" "Mark Twain" "The Adventures of Huckleberry Finn"
     "1955" "Vladimir Nabokov" "Lolita"
#v-
 Such a file may be read using
#v+
     n = readascii ("books.dat", &year, &author, &title;
                    format="\"%[^\"]\" \"%[^\"]\" \"%[^\"]\"");

#v-
\notes
 This current version of this function does not handle missing data.
 For such files, the \sfun{csv_readcol} function might be a better
 choice.

 By default, lines not matching the expected format are assumed to
 be comments and are skipped.  So normally the \exmp{comment}
 qualifier is not needed.  However, it is useful in conjunction with
 the \exmp{stop_on_mismatch} qualifier to force the parser to skip
 lines beginning with the comment string and continue scanning.
\seealso{sscanf, atof, fopen, fgets, fgetslines, csv_readcol}
\done
