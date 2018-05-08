/*  GNU ddrescue - Data recovery tool
    Copyright (C) 2004-2018 Antonio Diaz Diaz.

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

const char * const program_year = "2018";
std::string command_line;


void show_version()
  {
  std::printf( "GNU %s %s\n", program_name, PROGVERSION );
  std::printf( "Copyright (C) %s Antonio Diaz Diaz.\n", program_year );
  std::printf( "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n"
               "This is free software: you are free to change and redistribute it.\n"
               "There is NO WARRANTY, to the extent permitted by law.\n" );
  }


// Recognized formats: <num>[YZEPTGM][i][Bs], <num>k[Bs], <num>Ki[Bs]
//
long long getnum( const char * const ptr, const int hardbs,
                  const long long llimit = -LLONG_MAX,
                  const long long ulimit = LLONG_MAX,
                  const char ** const tailp = 0 )
  {
  char * tail;
  errno = 0;
  long long result = strtoll( ptr, &tail, 0 );
  if( tail == ptr )
    {
    show_error( "Bad or missing numerical argument.", 0, true );
    std::exit( 1 );
    }

  if( !errno && tail[0] )
    {
    char * const p = tail++;
    int factor = 1000;				// default factor
    int exponent = -1;				// -1 = bad multiplier
    char usuf = 0;			// 'B' or 's' unit suffix is present
    switch( *p )
      {
      case 'Y': exponent = 8; break;
      case 'Z': exponent = 7; break;
      case 'E': exponent = 6; break;
      case 'P': exponent = 5; break;
      case 'T': exponent = 4; break;
      case 'G': exponent = 3; break;
      case 'M': exponent = 2; break;
      case 'K': if( tail[0] == 'i' ) { ++tail; factor = 1024; exponent = 1; } break;
      case 'k': if( tail[0] != 'i' ) exponent = 1; break;
      case 'B':
      case 's': usuf = *p; exponent = 0; break;
      default : if( tailp ) { tail = p; exponent = 0; } break;
      }
    if( exponent > 1 && tail[0] == 'i' ) { ++tail; factor = 1024; }
    if( exponent > 0 && usuf == 0 && ( tail[0] == 'B' || tail[0] == 's' ) )
      { usuf = tail[0]; ++tail; }
    if( exponent < 0 || ( usuf == 's' && hardbs <= 0 ) ||
        ( !tailp && tail[0] != 0 ) )
      {
      show_error( "Bad multiplier in numerical argument.", 0, true );
      std::exit( 1 );
      }
    for( int i = 0; i < exponent; ++i )
      {
      if( LLONG_MAX / factor >= llabs( result ) ) result *= factor;
      else { errno = ERANGE; break; }
      }
    if( usuf == 's' )
      {
      if( LLONG_MAX / hardbs >= llabs( result ) ) result *= hardbs;
      else errno = ERANGE;
      }
    }
  if( !errno && ( result < llimit || result > ulimit ) ) errno = ERANGE;
  if( errno )
    {
    show_error( "Numerical argument out of limits." );
    std::exit( 1 );
    }
  if( tailp ) *tailp = tail;
  return result;
  }


bool check_types( std::string & types, const char * const opt_name,
                  const bool allow_l = false )
  {
  bool error = false;
  bool write_location_data = false;
  for( int i = types.size(); i > 0; )
    {
    if( types[--i] == 'l' )
      { if( !allow_l ) { error = true; break; }
        write_location_data = true; types.erase( i, 1 ); continue; }
    if( !Sblock::isstatus( types[i] ) )
      { error = true; break; }
    }
  if( !types.size() || error )
    {
    char buf[80];
    snprintf( buf, sizeof buf, "Invalid type for '%s' option.", opt_name );
    show_error( buf, 0, true );
    std::exit( 1 );
    }
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


void set_name( const char ** name, const char * new_name, const char opt )
  {
  if( *name )
    {
    std::string msg( "Option '- ' can be specified only once." );
    msg[9] = opt;
    show_error( msg.c_str(), 0, true );
    std::exit( 1 );
    }
  *name = new_name;
  }


const char * get_timestamp( const long t = 0 )
  {
  static char buf[80];
  const time_t tt = t ? t : std::time( 0 );
  const struct tm * const tm = std::localtime( &tt );
  if( !tm || std::strftime( buf, sizeof buf, "%Y-%m-%d %H:%M:%S", tm ) == 0 )
    buf[0] = 0;
  return buf;
  }

} // end namespace


void show_error( const char * const msg, const int errcode, const bool help )
  {
  if( verbosity < 0 ) return;
  if( msg && msg[0] )
    {
    std::fprintf( stderr, "%s: %s", program_name, msg );
    if( errcode > 0 ) std::fprintf( stderr, ": %s", std::strerror( errcode ) );
    std::fputc( '\n', stderr );
    }
  if( help )
    std::fprintf( stderr, "Try '%s --help' for more information.\n",
                  invocation_name );
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
  char buf[80];
  snprintf( buf, sizeof buf,
            "Mapfile '%s' does not exist or is not readable.", mapname );
  show_error( buf );
  return 1;
  }


int not_writable( const char * const mapname )
  {
  char buf[80];
  snprintf( buf, sizeof buf, "Mapfile '%s' is not writable.", mapname );
  show_error( buf );
  return 1;
  }


long initial_time()
  {
  static long initial_time_ = 0;

  if( initial_time_ == 0 ) initial_time_ = std::time( 0 );
  return initial_time_;
  }


bool write_file_header( FILE * const f, const char * const filetype )
  {
  static std::string timestamp;

  if( timestamp.empty() ) timestamp = get_timestamp( initial_time() );
  return ( std::fprintf( f, "# %s. Created by %s version %s\n"
                            "# Command line: %s\n"
                            "# Start time:   %s\n",
           filetype, Program_name, PROGVERSION, command_line.c_str(),
           timestamp.c_str() ) >= 0 );
  }


bool write_timestamp( FILE * const f )
  {
  const char * const timestamp = get_timestamp();

  return ( !timestamp || !timestamp[0] ||
           std::fprintf( f, "# Current time: %s\n", timestamp ) >= 0 );
  }


bool write_final_timestamp( FILE * const f )
  {
  static std::string timestamp;

  if( timestamp.empty() ) timestamp = get_timestamp();
  return ( std::fprintf( f, "# End time:     %s\n", timestamp.c_str() ) >= 0 );
  }


const char * format_num( long long num, long long limit,
                         const int set_prefix )
  {
  const char * const si_prefix[8] =
    { "k", "M", "G", "T", "P", "E", "Z", "Y" };
  const char * const binary_prefix[8] =
    { "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi" };
  enum { buffers = 8, bufsize = 16 };
  static char buffer[buffers][bufsize];	// circle of static buffers for printf
  static int current = 0;
  static bool si = true;

  if( set_prefix ) si = ( set_prefix > 0 );
  const int factor = si ? 1000 : 1024;
  char * const buf = buffer[current++]; current %= buffers;
  const char * const * prefix = si ? si_prefix : binary_prefix;
  const char * p = "";
  limit = std::max( 999LL, std::min( 999999LL, limit ) );

  for( int i = 0; i < 8 && llabs( num ) > limit; ++i )
    { num /= factor; p = prefix[i]; }
  snprintf( buf, bufsize, "%lld %s", num, p );
  return buf;
  }


// Shows the fraction "num/den" as a percentage with "prec" decimals.
// If 'prec' is negative, only the needed decimals are shown.
//
const char * format_percentage( long long num, long long den,
                                const int iwidth, int prec )
  {
  static char buf[80];

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
  const bool trunc = ( prec < 0 );
  if( prec < 0 ) prec = -prec;

  unsigned i;
  if( num < 0 && num / den == 0 )
    i = snprintf( buf, sizeof( buf ), "%*s", iwidth, "-0" );
  else i = snprintf( buf, sizeof( buf ), "%*lld", iwidth, num / den );
  if( i < sizeof( buf ) - 2 )
    {
    long long rest = llabs( num ) % den;
    if( prec > 0 && ( rest > 0 || !trunc ) )
      {
      buf[i++] = '.';
      while( prec > 0 && ( rest > 0 || !trunc ) && i < sizeof( buf ) - 2 )
        { rest *= 10; buf[i++] = (char)( rest / den ) + '0';
          rest %= den; --prec; }
      }
    }
  else i = sizeof( buf ) - 2;
  buf[i++] = '%';
  buf[i] = 0;
  return buf;
  }
