/* Arg_parser - POSIX/GNU command-line argument parser. (C++ version)
   Copyright (C) 2006-2026 Antonio Diaz Diaz.

   This library is free software. Redistribution and use in source and
   binary forms, with or without modification, are permitted provided
   that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions, and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions, and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* Arg_parser reads the arguments in 'argv' and creates a number of
   option codes, option arguments, and non-option arguments.

   In case of error, 'error' returns a non-empty error message.

   'options' is an array of 'struct Option' terminated by an element
   containing a code which is zero. A null long_name means a short-only
   option. A code value outside the unsigned char range means a long-only
   option.

   Arg_parser normally makes it appear as if all the options were specified
   before all the non-option arguments for the purposes of parsing, even if
   the user of your program intermixed options and non-option arguments. If
   you want the arguments in the exact order the user typed them, call
   'Arg_parser' with 'flags' = 'in_order'.

   The argument '--' terminates all options; any following arguments are
   treated as non-option arguments, even if they begin with a hyphen.

   The syntax of options with an optional argument is
   '-<short_option><argument>' (without whitespace), or
   '--<long_option>=<argument>'.

   The syntax of options with an empty argument is '-<short_option> ""',
   '--<long_option> ""', or '--<long_option>=""'.
*/

class Arg_parser
  {
public:
  enum Flags { in_order = 1, in_order_stop = 2, in_order_skip = 4,
               neg_non_opt = 8 };		// negative is non-option
  enum Has_arg { no, yes, maybe, yesme };	// yesme = yes but maybe empty

  struct Option
    {
    int code;			// Short option letter or code ( code != 0 )
    const char * long_name;	// Long option name (maybe null)
    Has_arg has_arg;
    };

private:
  struct Record
    {
    int code;
    std::string parsed_name;
    std::string argument;
    explicit Record( const unsigned char c )
      : code( c ), parsed_name( "-" ) { parsed_name += c; }
    Record( const int c, const char * const long_name )
      : code( c ), parsed_name( "--" ) { parsed_name += long_name; }
    explicit Record( const char * const arg ) : code( 0 ), argument( arg ) {}
    };

  const std::string empty_arg;
  std::string error_;
  std::vector< Record > data;
  int argv_index_;

  bool parse_long_option( const char * const opt, const char * const arg,
                          const Option options[], int & argind );
  bool parse_short_option( const char * const opt, const char * const arg,
                           const Option options[], int & argind );

public:
  Arg_parser( const int argc, const char * const argv[],
              const Option options[], const int flags = 0 );

  // Restricted constructor. Parses a single token and argument (if any).
  Arg_parser( const char * const opt, const char * const arg,
              const Option options[] );

  const std::string & error() const { return error_; }
  int argv_index() const { return argv_index_; }

  // The number of arguments parsed. May be different from argc.
  int arguments() const { return data.size(); }

  /* If code( i ) is 0, argument( i ) is a non-option.
     Else argument( i ) is the option's argument (or empty). */
  int code( const int i ) const
    {
    if( i >= 0 && i < arguments() ) return data[i].code;
    else return 0;
    }

  // Full name of the option parsed (short or long).
  const std::string & parsed_name( const int i ) const
    {
    if( i >= 0 && i < arguments() ) return data[i].parsed_name;
    else return empty_arg;
    }

  const std::string & argument( const int i ) const
    {
    if( i >= 0 && i < arguments() ) return data[i].argument;
    else return empty_arg;
    }
  };
