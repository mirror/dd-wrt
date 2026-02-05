/* GNU ddrescue - Data recovery tool
   Copyright (C) 2004-2026 Antonio Diaz Diaz.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

int verbosity = 0;

namespace {

const char * const program_year = "2026";
std::string command_line;
const char * const inval_t_msg = "Invalid type in argument of";


void show_version()
  {
  std::printf( "GNU %s %s\n", program_name, PROGVERSION );
  std::printf( "Copyright (C) %s Antonio Diaz Diaz.\n", program_year );
  std::fputs( "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n"
              "This is free software: you are free to change and redistribute it.\n"
              "There is NO WARRANTY, to the extent permitted by law.\n", stdout );
  }


bool check_types( const char * const arg, std::string & types,
                  const char * const option_name, const bool allow_l )
  {
  bool write_location_data = false;
  for( int i = types.size(); i > 0; )
    {
    if( types[--i] == 'l' && allow_l )
      { write_location_data = true; types.erase( i, 1 ); continue; }
    if( !Sblock::isstatus( types[i] ) )
      { show_option_error( arg, inval_t_msg, option_name ); std::exit( 1 ); }
    }
  if( types.empty() )			// types must not be empty
    { show_option_error( arg, "Missing type in argument of", option_name );
      std::exit( 1 ); }
  return write_location_data;
  }


void set_mode( Mode & program_mode, const Mode new_mode )
  {
  if( program_mode != m_none && program_mode != new_mode )
    {
    show_error( "Only one operation can be specified.", 0, true );
    std::exit( 1 );
    }
  program_mode = new_mode;
  }


void set_name( const char ** name, const char * new_name,
               const char * const option_name )
  {
  if( !*name ) { *name = new_name; return; }
  if( verbosity >= 0 )
    std::fprintf( stderr, "%s: Option '%s' can be specified only once.\n",
                  program_name, option_name );
  std::exit( 1 );
  }


const char * format_timestamp( const long long t = 0 )
  {
  static char buf[64];
  const time_t tt = t ? t : std::time( 0 );
  const struct tm * const tm = std::localtime( &tt );
  if( !tm || std::strftime( buf, sizeof buf, "%Y-%m-%d %H:%M:%S", tm ) == 0 )
    buf[0] = 0;
  return buf;
  }

} // end namespace


void show_option_error( const char * const arg, const char * const msg,
                        const char * const option_name )
  {
  if( verbosity >= 0 )
    std::fprintf( stderr, "%s: '%s': %s option '%s'.\n",
                  program_name, arg, msg, option_name );
  }


void show_error( const char * const msg, const int errcode, const bool help )
  {
  if( verbosity < 0 ) return;
  if( msg && *msg )
    std::fprintf( stderr, "%s: %s%s%s\n", program_name, msg,
                  ( errcode > 0 ) ? ": " : "",
                  ( errcode > 0 ) ? std::strerror( errcode ) : "" );
  if( help )
    std::fprintf( stderr, "Try '%s --help' for more information.\n",
                  invocation_name );
  }


void show_file_error( const char * const filename, const char * const msg,
                      const int errcode )
  {
  if( verbosity >= 0 )
    std::fprintf( stderr, "%s: %s: %s%s%s\n", program_name, filename, msg,
                  ( errcode > 0 ) ? ": " : "",
                  ( errcode > 0 ) ? std::strerror( errcode ) : "" );
  }


void internal_error( const char * const msg )
  {
  if( verbosity >= 0 )
    std::fprintf( stderr, "%s: internal error: %s\n", program_name, msg );
  std::exit( 3 );
  }


int empty_domain()
  { show_error( "Nothing to do; domain is empty." ); return 0; }


int not_readable( const char * const mapname )
  {
  show_file_error( mapname, "Mapfile does not exist or is not readable." );
  return 1;
  }


int not_writable( const char * const mapname )
  { show_file_error( mapname, "Mapfile is not writable." ); return 1; }


long long initial_time()
  {
  static long long initial_time_ = 0;

  if( initial_time_ == 0 ) initial_time_ = std::time( 0 );
  return initial_time_;
  }


bool write_file_header( FILE * const f, const char * const filetype )
  {
  static std::string timestamp;		// same inital timestamp on all files

  if( timestamp.empty() ) timestamp = format_timestamp( initial_time() );
  return ( std::fprintf( f, "# %s. Created by %s version %s\n"
                            "# Command line: %s\n"
                            "# Start time:   %s\n",
           filetype, Program_name, PROGVERSION, command_line.c_str(),
           timestamp.c_str() ) >= 0 );
  }


bool write_timestamp( FILE * const f )
  {
  const char * const timestamp = format_timestamp();

  return ( !timestamp || !*timestamp ||
           std::fprintf( f, "# Current time: %s\n", timestamp ) >= 0 );
  }


bool write_final_timestamp( FILE * const f )
  {
  static std::string timestamp;		// same final timestamp on all files

  if( timestamp.empty() ) timestamp = format_timestamp();
  return ( std::fprintf( f, "# End time:     %s\n", timestamp.c_str() ) >= 0 );
  }


int chvalue( const unsigned char ch )
  {
  if( ch >= '0' && ch <= '9' ) return ch - '0';
  if( ch >= 'A' && ch <= 'Z' ) return ch - 'A' + 10;
  if( ch >= 'a' && ch <= 'z' ) return ch - 'a' + 10;
  return 255;
  }

long long strtoll_( const char * const ptr, const char ** tail, int base )
  {
  if( tail ) *tail = ptr;				// error value
  int i = 0;
  while( std::isspace( ptr[i] ) || (unsigned char)ptr[i] == 0xA0 ) ++i;
  const bool minus = ptr[i] == '-';
  if( minus || ptr[i] == '+' ) ++i;
  if( base < 0 || base > 36 || base == 1 ||
      ( base == 0 && !std::isdigit( ptr[i] ) ) ||
      ( base != 0 && chvalue( ptr[i] ) >= base ) )
    { errno = EINVAL; return 0; }

  if( base == 0 )
    {
    if( ptr[i] != '0' ) base = 10;			// decimal
    else if( ptr[i+1] == 'x' || ptr[i+1] == 'X' ) { base = 16; i += 2; }
    else base = 8;					// octal or 0
    }
  const int dpg = ( base != 16 ) ? 3 : 2;	// min digits per group
  int dig = dpg - 1;	// digits in current group, first may have 1 digit
  const unsigned long long limit = minus ? LLONG_MAX + 1ULL : LLONG_MAX;
  unsigned long long result = 0;
  bool erange = false;
  for( ; ptr[i]; ++i )
    {
    if( ptr[i] == '_' ) { if( dig < dpg ) break; else { dig = 0; continue; } }
    const int val = chvalue( ptr[i] ); if( val >= base ) break; else ++dig;
    if( !erange && ( limit - val ) / base >= result )
      result = result * base + val;
    else erange = true;
    }
  if( dig < dpg ) { errno = EINVAL; return 0; }
  if( tail ) *tail = ptr + i;
  if( erange ) { errno = ERANGE; return minus ? LLONG_MIN : LLONG_MAX; }
  return minus ? 0LL - result : result;
  }


const char * format_num( long long num, int limit, const int set_prefix )
  {
  enum { buffers = 8, bufsize = 16 };
  const char * const si_prefix[] =
    { "k", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q", 0 };
  const char * const binary_prefix[] =
    { "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi", "Ri", "Qi", 0 };
  static char buffer[buffers][bufsize];	// circle of buffers for printf
  static int current = 0;
  static bool si = true;

  if( set_prefix ) si = set_prefix > 0;
  const int factor = si ? 1000 : 1024;
  char * const buf = buffer[current++]; current %= buffers;
  const char * const * prefix = si ? si_prefix : binary_prefix;
  const char * p = "";
  limit = std::max( 999, std::min( 999999, limit ) );

  for( int i = 0; llabs( num ) > limit && prefix[i]; ++i )
    { num /= factor; p = prefix[i]; }
  snprintf( buf, bufsize, "%d %s", (int)num, p );	// 6 digits or less
  return buf;
  }


// separate numbers of 5 or more digits in groups of 3 digits using '_'
const char * format_num3p( long long num, const bool space )
  {
  enum { buffers = 8, bufsize = 4 * sizeof num };
  const char * const si_prefix = "kMGTPEZYRQ";
  const char * const binary_prefix = "KMGTPEZYRQ";
  static char buffer[buffers][bufsize];	// circle of buffers for printf
  static int current = 0;

  char * const buf = buffer[current++]; current %= buffers;
  char * p = buf + bufsize - 1;		// fill the buffer backwards
  *p = 0;				// terminator
  const bool negative = num < 0;
  if( num >= 10000 || num <= -10000 )
    {
    char prefix = 0;			// try binary first, then si
    for( int i = 0; num != 0 && num % 1024 == 0 && binary_prefix[i]; ++i )
      { num /= 1024; prefix = binary_prefix[i]; }
    if( prefix ) *(--p) = 'i';
    else
      for( int i = 0; num != 0 && num % 1000 == 0 && si_prefix[i]; ++i )
        { num /= 1000; prefix = si_prefix[i]; }
    if( prefix ) *(--p) = prefix;
    }
  const bool split = num >= 10000 || num <= -10000;
  if( space ) *(--p) = ' ';
  for( int i = 0; ; )
    {
    const long long onum = num; num /= 10;
    *(--p) = llabs( onum - ( 10 * num ) ) + '0'; if( num == 0 ) break;
    if( split && ++i >= 3 ) { i = 0; *(--p) = '_'; }
    }
  if( negative ) *(--p) = '-';
  return p;
  }


// separate numbers of 5 or more digits in groups of 3 digits using '_'
const char * format_num3( unsigned long long num, const bool negative )
  {
  enum { buffers = 8, bufsize = 4 * sizeof num };
  static char buffer[buffers][bufsize];	// circle of buffers for printf
  static int current = 0;

  char * const buf = buffer[current++]; current %= buffers;
  char * p = buf + bufsize - 1;		// fill the buffer backwards
  *p = 0;				// terminator
  const bool split = num >= 10000;

  for( int i = 0; ; )
    {
    *(--p) = num % 10 + '0'; num /= 10; if( num == 0 ) break;
    if( split && ++i >= 3 ) { i = 0; *(--p) = '_'; }
    }
  if( negative ) *(--p) = '-';
  return p;
  }


// Recognized formats: <num>k[Bs], <num>Ki[Bs], <num>[MGTPEZYRQ][i][Bs]
long long getnum( const char * const arg, const char * const option_name,
                  const int hardbs, const long long llimit,
                  const long long ulimit, const char ** const tailp )
  {
  const char * tail;
  errno = 0;
  long long result = strtoll_( arg, &tail, 0 );
  if( tail == arg )
    { show_option_error( arg, "Bad or missing numerical argument in",
                         option_name ); std::exit( 1 ); }

  if( !errno && *tail )
    {
    const char * const p = tail++;
    int factor = 1000;				// default factor
    int exponent = -1;				// -1 = bad multiplier
    char usuf = 0;			// 'B' or 's' unit suffix is present
    switch( *p )
      {
      case 'Q': exponent = 10; break;
      case 'R': exponent = 9; break;
      case 'Y': exponent = 8; break;
      case 'Z': exponent = 7; break;
      case 'E': exponent = 6; break;
      case 'P': exponent = 5; break;
      case 'T': exponent = 4; break;
      case 'G': exponent = 3; break;
      case 'M': exponent = 2; break;
      case 'K': if( *tail == 'i' ) { ++tail; factor = 1024; exponent = 1; } break;
      case 'k': if( *tail != 'i' ) exponent = 1; break;
      case 'B':
      case 's': usuf = *p; exponent = 0; break;
      default: if( tailp ) { tail = p; exponent = 0; }
      }
    if( exponent > 1 && *tail == 'i' ) { ++tail; factor = 1024; }
    if( exponent > 0 && usuf == 0 && ( *tail == 'B' || *tail == 's' ) )
      { usuf = *tail; ++tail; }
    if( exponent < 0 || ( usuf == 's' && hardbs <= 0 ) ||
        ( !tailp && *tail != 0 ) )
      { show_option_error( arg, "Bad multiplier in numerical argument of",
                           option_name ); std::exit( 1 ); }
    for( int i = 0; i < exponent; ++i )
      {
      if( ( result >= 0 && LLONG_MAX / factor >= result ) ||
          ( result < 0 && LLONG_MIN / factor <= result ) ) result *= factor;
      else { errno = ERANGE; break; }
      }
    if( usuf == 's' )
      {
      if( ( result >= 0 && LLONG_MAX / hardbs >= result ) ||
          ( result < 0 && LLONG_MIN / hardbs <= result ) ) result *= hardbs;
      else errno = ERANGE;
      }
    }
  if( !errno && ( result < llimit || result > ulimit ) ) errno = ERANGE;
  if( errno )
    {
    if( verbosity >= 0 )
      std::fprintf( stderr, "%s: '%s': Value out of limits [%s,%s] in "
                    "option '%s'.\n", program_name, arg, format_num3p( llimit ),
                    format_num3p( ulimit ), option_name );
    std::exit( 1 );
    }
  if( tailp ) *tailp = tail;
  return result;
  }


/* Return a string representing the fraction 'num/den' as a percentage with
   'prec' decimals.
   'iwidth' is the minimum width of the integer part, prefixed with spaces
   if needed.
   If 'prec' is negative, produce only the decimals needed.
   If 'rounding', round up the last digit if the next one would be >= 5.
*/
const char * format_percentage( long long num, long long den,
                                const int iwidth, int prec,
                                const bool rounding )
  {
  enum { bufsize = 80 };
  static char buf[bufsize];

  if( den < 0 ) { num = -num; den = -den; }
  if( llabs( num ) <= LLONG_MAX / 100 && den <= LLONG_MAX / 10 ) num *= 100;
  else if( llabs( num ) <= LLONG_MAX / 10 ) { num *= 10; den /= 10; }
  else den /= 100;
  if( den == 0 )
    {
    if( num > 0 ) return "+INF";
    else if( num < 0 ) return "-INF";
    else return "NAN";
    }
  const bool trunc = prec < 0;
  if( prec < 0 ) prec = -prec;

  unsigned i;
  if( num < 0 && num / den == 0 )		// negative but > -1.0
    i = snprintf( buf, bufsize, "%*s", iwidth, "-0" );
  else i = snprintf( buf, bufsize, "%*lld", iwidth, num / den );
  if( i < bufsize - 2 )
    {
    long long rest = llabs( num ) % den;
    if( prec > 0 && ( rest > 0 || !trunc ) )
      {
      buf[i++] = '.';
      while( prec > 0 && ( rest > 0 || !trunc ) && i < bufsize - 2 )
        { rest *= 10; buf[i++] = rest / den + '0'; rest %= den; --prec; }
      }
    if( rounding && rest * 2 >= den )		// round last decimal up
      for( int j = i - 1; j >= 0; --j )
        {
        if( buf[j] == '.' ) continue;
        if( buf[j] >= '0' && buf[j] < '9' ) { ++buf[j]; break; }
        if( buf[j] == '9' ) buf[j] = '0';
        if( j > 0 && buf[j-1] == '.' ) continue;
        if( j > 0 && buf[j-1] == ' ' ) { buf[j-1] = '1'; break; }
        if( j > 1 && buf[j-2] == ' ' && buf[j-1] == '-' )
          { buf[j-2] = '-'; buf[j-1] = '1'; break; }
        if( j == 0 || buf[j-1] < '0' || buf[j-1] > '9' )	// no prev digit
          {
          for( int k = i - 1; k > j; --k ) buf[k] = buf[k-1];
          buf[j] = '1'; break;		// prepend '1' to the first digit
          }
        }
    }
  else i = bufsize - 2;
  buf[i++] = '%';
  buf[i] = 0;
  return buf;
  }
