\function{array_to_bstring}
\synopsis{Convert an array to a binary string}
\usage{BString_Type array_to_bstring (Array_Type a)}
\description
   The \ifun{array_to_bstring} function returns the elements of an
   array \exmp{a} as a binary string.
\seealso{bstring_to_array, init_char_array}
\done

\function{bstring_to_array}
\synopsis{Convert a binary string to an array of characters}
\usage{UChar_Type[] bstring_to_array (BString_Type b)}
\description
   The \ifun{bstring_to_array} function returns an array of unsigned
   characters whose elements correspond to the bytes in the
   binary string.
\seealso{array_to_bstring, init_char_array}
\done

\function{bstrlen}
\synopsis{Get the length of a binary string}
\usage{UInt_Type bstrlen (BString_Type s)}
\description
  The \ifun{bstrlen} function may be used to obtain the length of a
  binary string.  A binary string differs from an ordinary string (a C
  string) in that a binary string may include null characters.
\example
#v+
    s = "hello\0";
    len = bstrlen (s);      % ==> len = 6
    len = strlen (s);       % ==> len = 5
#v-
\seealso{strlen, length}
\done

\function{count_byte_occurrences}
\synopsis{Count the number of occurrences of a byte in a binary string}
\usage{UInt_Type count_byte_occurrences (bstring, byte)}
\description
  This function returns the number of times the specified byte
  occurs in the binary string \exmp{bstr}.
\notes
  This function uses byte-semantics.  If character semantics are
  desired, use the \ifun{count_char_occurrences} function.
\seealso{count_char_occurrences}
\done

\function{pack}
\synopsis{Pack objects into a binary string}
\usage{BString_Type pack (String_Type fmt, ...)}
\description
  The \ifun{pack} function combines zero or more objects (represented
  by the ellipses above) into a binary string according to the format
  string \exmp{fmt}.

  The format string consists of one or more data-type specification
  characters defined by the following table:
#v+
     c     signed byte
     C     unsigned byte
     h     short
     H     unsigned short
     i     int
     I     unsigned int
     l     long
     L     unsigned long
     m     long long
     M     unsigned long long
     j     16 bit int
     J     16 bit unsigned int
     k     32 bit int
     K     32 bit unsigned int
     q     64 bit int
     Q     64 bit unsigned int
     f     float
     d     double
     F     32 bit float
     D     64 bit float
     s     character string, null padded
     S     character string, space padded
     z     character string, null padded
     x     a null pad character
#v-
  A decimal length specifier may follow the data-type specifier.  With
  the exception of the \exmp{s} and \exmp{S} specifiers, the length
  specifier indicates how many objects of that data type are to be
  packed or unpacked from the string.  When used with the \exmp{s},
  \exmp{S}, or \exmp{z} specifiers, it indicates the field width to be
  used.  If the length specifier is not present, the length defaults
  to one.

  When packing, unlike the \exmp{s} specifier, the \exmp{z} specifier
  guarantees that at least one null byte will be written even if the
  field has to be truncated to do so.

  With the exception of \exmp{c}, \exmp{C}, \exmp{s}, \exmp{S}, and
  \exmp{x}, each of these may be prefixed by a character that indicates
  the byte-order of the object:
#v+
     >    big-endian order (network order)
     <    little-endian order
     =    native byte-order
#v-
  The default is to use native byte order.

  When unpacking via the \ifun{unpack} function, if the length
  specifier is greater than one, then an array of that length will be
  returned.  In addition, trailing whitespace and null characters are
  stripped when unpacking an object given by the \exmp{S} specifier.
  Trailing null characters will be stripped from an object represented
  by the \exmp{z} specifier.  No such stripping is performed by the \exmp{s}
  specifier.
\example
#v+
     a = pack ("cc", 'A', 'B');         % ==> a = "AB";
     a = pack ("c2", 'A', 'B');         % ==> a = "AB";
     a = pack ("xxcxxc", 'A', 'B');     % ==> a = "\0\0A\0\0B";
     a = pack ("h2", 'A', 'B');         % ==> a = "\0A\0B" or "\0B\0A"
     a = pack (">h2", 'A', 'B');        % ==> a = "\0\xA\0\xB"
     a = pack ("<h2", 'A', 'B');        % ==> a = "\0B\0A"
     a = pack ("s4", "AB", "CD");       % ==> a = "AB\0\0"
     a = pack ("s4s2", "AB", "CD");     % ==> a = "AB\0\0CD"
     a = pack ("S4", "AB", "CD");       % ==> a = "AB  "
     a = pack ("S4S2", "AB", "CD");     % ==> a = "AB  CD"
     a = pack ("z4", "AB");             % ==> a = "AB\0\0"
     a = pack ("s4", "ABCDEFG");        % ==> a = "ABCD"
     a = pack ("z4", "ABCDEFG");        % ==> a = "ABC\0"
#v-
\seealso{unpack, sizeof_pack, pad_pack_format, sprintf}
\done

\function{pad_pack_format}
\synopsis{Add padding to a pack format}
\usage{BString_Type pad_pack_format (String_Type fmt)}
\description
  The \ifun{pad_pack_format} function may be used to add the
  appropriate padding characters to the format \exmp{fmt} such that the
  data types specified by the format will be properly aligned on word
  boundaries.  This is especially important when reading or writing files
  that assume the native alignment.
\seealso{pack, unpack, sizeof_pack}
\done

\function{sizeof_pack}
\synopsis{Compute the size implied by a pack format string}
\usage{UInt_Type sizeof_pack (String_Type fmt)}
\description
  The \ifun{sizeof_pack} function returns the size of the binary string
  represented by the format string \exmp{fmt}.  This information may be
  needed when reading a structure from a file.
\seealso{pack, unpack, pad_pack_format}
\done

\function{unpack}
\synopsis{Unpack Objects from a Binary String}
\usage{(...) = unpack (String_Type fmt, BString_Type s)}
\description
  The \ifun{unpack} function unpacks objects from a binary string
  \exmp{s} according to the format \exmp{fmt} and returns the objects to
  the stack in the order in which they were unpacked.  See the
  documentation of the \ifun{pack} function for details about the
  format string.
\example
#v+
    (x,y) = unpack ("cc", "AB");          % ==> x = 'A', y = 'B'
    x = unpack ("c2", "AB");              % ==> x = ['A', 'B']
    x = unpack ("x<H", "\0\xAB\xCD");     % ==> x = 0xCDABuh
    x = unpack ("xxs4", "a b c\0d e f");  % ==> x = "b c\0"
    x = unpack ("xxS4", "a b c\0d e f");  % ==> x = "b c"
#v-
\seealso{pack, sizeof_pack, pad_pack_format}
\done

