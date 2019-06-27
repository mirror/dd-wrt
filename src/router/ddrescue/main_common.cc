/*  GNU ddrescue - Data recovery tool
    Copyright (C) 2004-2014 Antonio Diaz Diaz.

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

namespace {

const char * const program_year = "2014";
std::string command_line;


void show_version()
  {
  std::printf( "GNU %s %s\n", program_name, PROGVERSION );
  std::printf( "Copyright (C) %s Antonio Diaz Diaz.\n", program_year );
  std::printf( "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>\n"
               "This is free software: you are free to change and redistribute it.\n"
               "There is NO WARRANTY, to the extent permitted by law.\n" );
  }


long long getnum( const char * const ptr, const int hardbs,
                  const long long min = LLONG_MIN + 1,
                  const long long max = LLONG_MAX, const bool comma = false )
  {
  errno = 0;
  char * tail;
  long long result = strtoll( ptr, &tail, 0 );
  if( tail == ptr )
    {
    show_error( "Bad or missing numerical argument.", 0, true );
    std::exit( 1 );
    }

  if( !errno && tail[0] )
    {
    int factor = ( tail[1] == 'i' ) ? 1024 : 1000;
    int exponent = 0;
    bool bad_multiplier = false;
    switch( tail[0] )
      {
      case ' ': break;
      case ',': if( !comma ) { bad_multiplier = true; } break;
      case 'b':
      case 's': if( hardbs > 0 ) { factor = hardbs; exponent = 1; }
                else bad_multiplier = true;
                break;
      case 'Y': exponent = 8; break;
      case 'Z': exponent = 7; break;
      case 'E': exponent = 6; break;
      case 'P': exponent = 5; break;
      case 'T': exponent = 4; break;
      case 'G': exponent = 3; break;
      case 'M': exponent = 2; break;
      case 'K': if( factor == 1024 ) exponent = 1; else bad_multiplier = true;
                break;
      case 'k': if( factor == 1000 ) exponent = 1; else bad_multiplier = true;
                break;
      default: bad_multiplier = true;
      }
    if( bad_multiplier )
      {
      show_error( "Bad multiplier in numerical argument.", 0, true );
      std::exit( 1 );
      }
    for( int i = 0; i < exponent; ++i )
      {
      if( LLONG_MAX / factor >= llabs( result ) ) result *= factor;
      else { errno = ERANGE; break; }
      }
    }
  if( !errno && ( result < min || result > max ) ) errno = ERANGE;
  if( errno )
    {
    show_error( "Numerical argument out of limits." );
    std::exit( 1 );
    }
  return result;
  }


void check_types( const std::string & types, const char * const opt_name )
  {
  bool error = false;
  for( unsigned i = 0; i < types.size(); ++i )
    if( !Sblock::isstatus( types[i] ) )
      { error = true; break; }
  if( !types.size() || error )
    {
    char buf[80];
    snprintf( buf, sizeof buf, "Invalid type for '%s' option.", opt_name );
    show_error( buf, 0, true );
    std::exit( 1 );
    }
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


int verbosity = 0;
bool sgpt = false;
int ata = false;
bool checked_ata = false;
bool bad_ata_read = false;
bool partial_read = false;
bool mark_error = false;
int passthrough_error = 0;
int sector_size = 0;
bool extended = false;



void show_error( const char * const msg, const int errcode, const bool help )
  {
  if( verbosity >= 0 )
    {
    if( msg && msg[0] )
      {
      std::fprintf( stderr, "%s: %s", program_name, msg );
      if( errcode > 0 )
        std::fprintf( stderr, ": %s", std::strerror( errcode ) );
      std::fprintf( stderr, "\n" );
      }
    if( help )
      std::fprintf( stderr, "Try '%s --help' for more information.\n",
                    invocation_name );
    }
  }


void internal_error( const char * const msg )
  {
  if( verbosity >= 0 )
    std::fprintf( stderr, "%s: internal error: %s\n", program_name, msg );
  std::exit( 3 );
  }


int empty_domain() { show_error( "Empty domain." ); return 0; }


int not_readable( const char * const logname )
  {
  char buf[80];
  snprintf( buf, sizeof buf,
            "Logfile '%s' does not exist or is not readable.", logname );
  show_error( buf );
  return 1;
  }


int not_writable( const char * const logname )
  {
  char buf[80];
  snprintf( buf, sizeof buf, "Logfile '%s' is not writable.", logname );
  show_error( buf );
  return 1;
  }


long initial_time()
  {
  static long initial_time_ = 0;

  if( initial_time_ == 0 ) initial_time_ = std::time( 0 );
  return initial_time_;
  }


bool write_logfile_header( FILE * const f, const char * const logtype )
  {
  static std::string timestamp;

  if( timestamp.empty() ) timestamp = get_timestamp( initial_time() );
  return ( std::fprintf( f, "# %s Logfile. Created by %s version %s\n"
                            "# Command line: %s\n"
                            "# Start time:   %s\n",
           logtype, Program_name, PROGVERSION, command_line.c_str(),
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
  return ( std::fprintf( f, "# End time: %s\n", timestamp.c_str() ) >= 0 );
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
