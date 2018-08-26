\function{atof}
\synopsis{Convert a string to a double precision number}
\usage{Double_Type atof (String_Type s)}
\description
  This function converts a string \exmp{s} to a double precision value
  and returns the result.  It performs no error checking on the format
  of the string.  The function \ifun{_slang_guess_type} may be used to
  check the syntax of the string.
\example
#v+
     define error_checked_atof (s)
     {
        if (__is_datatype_numeric (_slang_guess_type (s)))
          return atof (s);
        throw InvalidParmError, "$s is not a double"$;
    }
#v-
\seealso{typecast, double, _slang_guess_type}
\done

\function{atoi}
\synopsis{Convert a string to an integer}
\usage{Int_Type atoi (String_Type str)}
\description
  The \ifun{atoi} function converts a string to an \exmp{Int_Type}
  using the standard C library function of the corresponding name.
\notes
  This function performs no syntax checking upon its argument.
\seealso{integer, atol, atoll, atof, sscanf}
\done

\function{atol}
\synopsis{Convert a string to an long integer}
\usage{Long_Type atol (String_Type str)}
\description
  The \ifun{atol} function converts a string to a \exmp{Long_Type}
  using the standard C library function of the corresponding name.
\notes
  This function performs no syntax checking upon its argument.
\seealso{integer, atoi, atoll, atof, sscanf}
\done

\function{atoll}
\synopsis{Convert a string to a long long}
\usage{LLong_Type atoll (String_Type str)}
\description
  The \ifun{atoll} function converts a string to a \exmp{LLong_Type}
  using the standard C library function of the corresponding name.
\notes
  This function performs no syntax checking upon its argument.  Not
  all platforms provide support for the long long data type.
\seealso{integer, atoi, atol, atof, sscanf}
\done

\function{char}
\synopsis{Convert a character code to a string}
\usage{String_Type char (Integer_Type c)}
\description
  The \ifun{char} function converts an integer character code (ascii)
  value \exmp{c} to a string of unit character length such that the
  first character of the string is \exmp{c}.  For example,
  \exmp{char('a')} returns the string \exmp{"a"}.

  If UTF-8 mode is in effect  (\ivar{_slang_utf8_ok} is non-zero), the
  resulting single character may be represented by several bytes.

  If the character code \exmp{c} is less than 0, then byte-semantics
  will be used with the resulting string consisting of a single byte
  whose value is that of \exmp{-c&0xFF}.
\notes
  A better name should have been chosen for this function.
\seealso{integer, string, typedef, sprintf, pack}
\done

\function{define_case}
\synopsis{Define upper-lower case conversion}
\usage{define_case (Integer_Type ch_up, Integer_Type ch_low)}
\description
  This function defines an upper and lowercase relationship between two
  characters specified by the arguments.  This relationship is used by
  routines which perform uppercase and lowercase conversions.
  The first integer \exmp{ch_up} is the ascii value of the uppercase character
  and the second parameter \exmp{ch_low} is the ascii value of its
  lowercase counterpart.
\notes
  This function has no effect in UTF-8 mode.
\seealso{strlow, strup}
\done

\function{double}
\synopsis{Convert an object to double precision}
\usage{Double_Type double (x)}
\description
  The \ifun{double} function typecasts an object \exmp{x} to double
  precision.  For example, if \exmp{x} is an array of integers, an
  array of double types will be returned.  If an object cannot be
  converted to \ifun{Double_Type}, a type-mismatch error will result.
\notes
  The \ifun{double} function is equivalent to the typecast operation
#v+
     typecast (x, Double_Type)
#v-
  To convert a string to a double precision number, use the \ifun{atof}
  function.
\seealso{typecast, atof, int}
\done

\function{int}
\synopsis{Typecast an object to an integer}
\usage{Int_Type int (s)}
\description
  This function performs a typecast of an object \exmp{s} to
  an object of \dtype{Integer_Type}.  If \exmp{s} is a string, it returns
  returns the ascii value of the first bytes of the string
  \exmp{s}.  If \exmp{s} is \dtype{Double_Type}, \ifun{int} truncates the
  number to an integer and returns it.
\example
  \ifun{int} can be used to convert single byte strings to
  integers.  As an example, the intrinsic function \ifun{isdigit} may
  be defined as
#v+
    define isdigit (s)
    {
      if ((int (s) >= '0') and (int (s) <= '9')) return 1;
      return 0;
    }
#v-
\notes
  This function is equivalent to \exmp{typecast (s, Integer_Type)};
\seealso{typecast, double, integer, char, isdigit, isxdigit}
\done

\function{integer}
\synopsis{Convert a string to an integer}
\usage{Integer_Type integer (String_Type s)}
\description
  The \ifun{integer} function converts a string representation of an
  integer back to an integer.  If the string does not form a valid
  integer, a SyntaxError will be thrown.
\example
  \exmp{integer ("1234")} returns the integer value \exmp{1234}.
\notes
  This function operates only on strings and is not the same as the
  more general \ifun{typecast} operator.
\seealso{typecast, _slang_guess_type, string, sprintf, char}
\done

\function{isalnum, isalpha, isascii, isblank, iscntrl, isdigit, isgraph,
islower, isprint, ispunct, isspace, isupper, isxdigit
}
\synopsis{Character classification functions}
\usage{Char_Type isalnum(wch)
  Char_Type isalpha(wch)
  Char_Type isascii(wch)
  Char_Type isblank(wch)
  Char_Type iscntrl(wch)
  Char_Type isdigit(wch)
  Char_Type isgraph(wch)
  Char_Type islower(wch)
  Char_Type isprint(wch)
  Char_Type ispunct(wch)
  Char_Type isspace(wch)
  Char_Type isupper(wch)
  Char_Type isxdigit(wch)
}
\description
  These functions return a non-zero value if the character given by
  \exmp{wch} is a member of the character class represented by the
  function, according to the table below.  Otherwise, 0 will be
  returned to indicate that the character is not a member of the
  class.  If the parameter \exmp{wch} is a string, then the first
  character (not necessarily a byte) of the string will be used.
#v+
   isalnum : alphanumeric character, equivalent to isalpha or isdigit
   isalpha : alphabetic character
   isascii : 7-bit unsigned ascii character
   isblank : space or a tab
   iscntrl : control character
   isdigit : digit 0-9
   isgraph : non-space printable character
   islower : lower-case character
   isprint : printable character, including a space
   ispunct : non-alphanumeric graphic character
   isspace : whitespace character (space, newline, tab, etc)
   isupper : uppercase case character
   isxdigit: hexadecimal digit character 0-9, a-f, A-F
#v-
\seealso{strtrans}
\done

\function{_slang_guess_type}
\synopsis{Guess the data type that a string represents}
\usage{DataType_Type _slang_guess_type (String_Type s)}
\description
  This function tries to determine whether its argument \exmp{s}
  represents an integer (short, int, long), floating point (float,
  double), or a complex number.  If it appears to be none of these,
  then a string is assumed.  It returns one of the following values
  depending on the format of the string \exmp{s}:
#v+
    Short_Type     :  short integer           (e.g., "2h")
    UShort_Type    :  unsigned short integer  (e.g., "2hu")
    Integer_Type   :  integer                 (e.g., "2")
    UInteger_Type  :  unsigned integer        (e.g., "2")
    Long_Type      :  long integer            (e.g., "2l")
    ULong_Type     :  unsigned long integer   (e.g., "2l")
    Float_Type     :  float                   (e.g., "2.0f")
    Double_Type    :  double                  (e.g., "2.0")
    Complex_Type   :  imaginary               (e.g., "2i")
    String_Type    :  Anything else.          (e.g., "2foo")
#v-
  For example, \exmp{_slang_guess_type("1e2")} returns
  \dtype{Double_Type} but \exmp{_slang_guess_type("e12")} returns
  \dtype{String_Type}.
\seealso{integer, string, double, atof, __is_datatype_numeric}
\done

\function{string}
\synopsis{Convert an object to a string representation.}
\usage{String_Type string (obj)}
\description
   The \ifun{string} function may be used to convert an object
   \exmp{obj} of any type to its string representation.
   For example, \exmp{string(12.34)} returns \exmp{"12.34"}.
\example
#v+
     define print_anything (anything)
     {
        message (string (anything));
     }
#v-
\notes
   This function is \em{not} the same as typecasting to a \dtype{String_Type}
   using the \ifun{typecast} function.
\seealso{typecast, sprintf, integer, char}
\done

\function{tolower}
\synopsis{Convert a character to lowercase.}
\usage{Integer_Type lower (Integer_Type ch)}
\description
  This function takes an integer \exmp{ch} and returns its lowercase
  equivalent.
\seealso{toupper, strup, strlow, int, char, define_case}
\done

\function{toupper}
\synopsis{Convert a character to uppercase.}
\usage{Integer_Type toupper (Integer_Type ch)}
\description
  This function takes an integer \exmp{ch} and returns its uppercase
  equivalent.
\seealso{tolower, strup, strlow, int, char, define_case}
\done

\function{typecast}
\synopsis{Convert an object from one data type to another.}
\usage{typecast (x, new_type)}
\description
  The \ifun{typecast} function performs a generic typecast operation on
  \exmp{x} to convert it to \exmp{new_type}.  If \exmp{x} represents an
  array, the function will attempt to convert all elements of \exmp{x}
  to \exmp{new_type}.  Not all objects can be converted and a
  type-mismatch error will result upon failure.
\example
#v+
    define to_complex (x)
    {
       return typecast (x, Complex_Type);
    }
#v-
  defines a function that converts its argument, \exmp{x} to a complex
  number.
\seealso{int, double, typeof}
\done

\function{_typeof}
\synopsis{Get the data type of an object}
\usage{DataType_Type _typeof (x)}
\description
  This function is similar to the \ifun{typeof} function except in the
  case of arrays.  If the object \exmp{x} is an array, then the data
  type of the array will be returned. Otherwise \ifun{_typeof} returns
  the data type of \exmp{x}.
\example
#v+
    if (Integer_Type == _typeof (x))
      message ("x is an integer or an integer array");
#v-
\seealso{typeof, array_info, _slang_guess_type, typecast}
\done

\function{typeof}
\synopsis{Get the data type of an object}
\usage{DataType_Type typeof (x)}
\description
  This function returns the data type of \exmp{x}.
\example
#v+
  if (Integer_Type == typeof (x)) message ("x is an integer");
#v-
\seealso{_typeof, is_struct_type, array_info, _slang_guess_type, typecast}
\done

