/*  GNU ddrescuelog - Tool for ddrescue logfiles
    Copyright (C) 2011-2014 Antonio Diaz Diaz.

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
/*
    Exit status: 0 for a normal exit, 1 for environmental problems
    (file not found, invalid flags, I/O errors, etc), 2 to indicate a
    corrupt or invalid input file, 3 for an internal consistency error
    (eg, bug) which caused ddrescuelog to panic.
*/

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <stdint.h>

#include "arg_parser.h"
#include "block.h"
#include "ddrescue.h"


namespace {

const char * const Program_name = "GNU ddrescuelog";
const char * const program_name = "ddrescuelog";
const char * invocation_name = 0;

enum Mode { m_none, m_and, m_change, m_compare, m_complete, m_create,
            m_delete, m_done_st, m_invert, m_list, m_or, m_status, m_xor };


void show_help( const int hardbs )
  {
  std::printf( "%s - Tool for ddrescue logfiles.\n", Program_name );
  std::printf( "Manipulates ddrescue logfiles, shows their contents, converts them to/from\n"
               "other formats, compares them, and tests rescue status.\n"
               "\nUsage: %s [options] logfile\n", invocation_name );
  std::printf( "\nOptions:\n"
               "  -h, --help                      display this help and exit\n"
               "  -V, --version                   output version information and exit\n"
               "  -a, --change-types=<ot>,<nt>    change the block types of logfile\n"
               "  -b, --block-size=<bytes>        block size in bytes [default %d]\n", hardbs );
  std::printf( "  -B, --binary-prefixes           show binary multipliers in numbers [SI]\n"
               "  -c, --create-logfile[=<tt>]     create logfile from list of blocks [+-]\n"
               "  -C, --complete-logfile[=<t>]    complete logfile adding blocks of type t [?]\n"
               "  -d, --delete-if-done            delete the logfile if rescue is finished\n"
               "  -D, --done-status               return 0 if rescue is finished\n"
               "  -f, --force                     overwrite existing output files\n"
               "  -i, --input-position=<bytes>    starting position of rescue domain [0]\n"
               "  -l, --list-blocks=<types>       print block numbers of given types (?*/-+)\n"
               "  -L, --loose-domain              accept an incomplete domain logfile\n"
               "  -m, --domain-logfile=<file>     restrict domain to finished blocks in file\n"
               "  -n, --invert-logfile            invert block types (finished <--> others)\n"
               "  -o, --output-position=<bytes>   starting position in output file [ipos]\n"
               "  -p, --compare-logfile=<file>    compare block types in domain of both files\n"
               "  -P, --compare-as-domain=<file>  like -p but compare finished blocks only\n"
               "  -q, --quiet                     suppress all messages\n"
               "  -s, --size=<bytes>              maximum size of rescue domain to be processed\n"
               "  -t, --show-status               show a summary of logfile contents\n"
               "  -v, --verbose                   be verbose (a 2nd -v gives more)\n"
               "  -x, --xor-logfile=<file>        XOR the finished blocks in file with logfile\n"
               "  -y, --and-logfile=<file>        AND the finished blocks in file with logfile\n"
               "  -z, --or-logfile=<file>         OR the finished blocks in file with logfile\n"
               "Numbers may be in decimal, hexadecimal or octal, and may be followed by a\n"
               "multiplier: s = sectors, k = 1000, Ki = 1024, M = 10^6, Mi = 2^20, etc...\n"
               "\nExit status: 0 for a normal exit, 1 for environmental problems (file\n"
               "not found, invalid flags, I/O errors, etc), 2 to indicate a corrupt or\n"
               "invalid input file, 3 for an internal consistency error (eg, bug) which\n"
               "caused ddrescuelog to panic.\n"
               "\nReport bugs to bug-ddrescue@gnu.org\n"
               "Ddrescue home page: http://www.gnu.org/software/ddrescue/ddrescue.html\n"
               "General help using GNU software: http://www.gnu.org/gethelp\n" );
  }


void parse_types( const std::string & arg,
                  std::string & types1, std::string & types2 )
  {
  std::string * p = &types1;
  bool error = false, comma_found = false;
  types1.clear();
  types2.clear();

  for( unsigned i = 0; i < arg.size(); ++i )
    {
    const char ch = arg[i];
    if( ch == ',' )
      {
      if( comma_found ) { error = true; break; }
      else { comma_found = true; p = &types2; continue; }
      }
    if( !Sblock::isstatus( ch ) ) { error = true; break; }
    *p += ch;
    }
  if( types1.empty() || types2.empty() ) error = true;
  if( error )
    {
    show_error( "Invalid type for 'change-types' option.", 0, true );
    std::exit( 1 );
    }
  if( types1.size() > types2.size() )
    types2.append( types1.size() - types2.size(), types2[types2.size()-1] );
  }


void parse_2types( const std::string & arg,
                   Sblock::Status & type1, Sblock::Status & type2 )
  {
  if( arg.empty() ) return;
  if( arg.size() != 2 || arg[0] == arg[1] ||
      !Sblock::isstatus( arg[0] ) || !Sblock::isstatus( arg[1] ) )
    {
    show_error( "Invalid type for 'create-logfile' option.", 0, true );
    std::exit( 1 );
    }
  type1 = Sblock::Status( arg[0] );
  type2 = Sblock::Status( arg[1] );
  }


void parse_type( const std::string & arg, Sblock::Status & complete_type )
  {
  if( arg.empty() ) return;
  if( arg.size() != 1 || !Sblock::isstatus( arg[0] ) )
    {
    show_error( "Invalid type for 'complete-logfile' option.", 0, true );
    std::exit( 1 );
    }
  complete_type = Sblock::Status( arg[0] );
  }


int do_logic_ops( Domain & domain, const char * const logname,
                  const char * const second_logname, const Mode program_mode )
  {
  Logfile logfile( logname );
  if( !logfile.read_logfile() ) return not_readable( logname );
  logfile.compact_sblock_vector();

  Logfile logfile2( second_logname );
  if( !logfile2.read_logfile() ) return not_readable( second_logname );
  logfile2.compact_sblock_vector();

  domain.crop( logfile.extent() );
  domain.crop( logfile2.extent() );
  if( domain.empty() ) return empty_domain();
  logfile.split_by_domain_borders( domain );
  logfile2.split_by_domain_borders( domain );
  logfile.split_by_logfile_borders( logfile2 );
  logfile2.split_by_logfile_borders( logfile );

  for( int i = 0, j = 0; ; ++i, ++j )
    {
    while( i < logfile.sblocks() && !domain.includes( logfile.sblock( i ) ) )
      ++i;
    while( j < logfile2.sblocks() && !domain.includes( logfile2.sblock( j ) ) )
      ++j;
    if( i >= logfile.sblocks() || j >= logfile2.sblocks() ) break;
    const Sblock & sb1 = logfile.sblock( i );
    const Sblock & sb2 = logfile2.sblock( j );
    if( sb1.pos() != sb2.pos() || sb1.size() != sb2.size() )
      internal_error( "blocks got out of sync." );
    const bool f1 = ( sb1.status() == Sblock::finished );
    const bool f2 = ( sb2.status() == Sblock::finished );
    switch( program_mode )
      {
      case m_and:
        if( f1 && !f2 ) logfile.change_sblock_status( i, Sblock::bad_sector );
        break;
      case m_or:
        if( !f1 && f2 ) logfile.change_sblock_status( i, Sblock::finished );
        break;
      case m_xor:
        if( f1 != ( ( f1 || f2 ) && !( f1 && f2 ) ) )
          logfile.change_sblock_status( i, f1 ? Sblock::bad_sector : Sblock::finished );
        break;
      default: internal_error( "invalid program_mode." );
      }
    }
  logfile.compact_sblock_vector();
  logfile.write_logfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Can't close stdout", errno ); return 1; }
  return 0;
  }


int change_types( Domain & domain, const char * const logname,
                  const std::string & types1, const std::string & types2 )
  {
  Logfile logfile( logname );
  if( !logfile.read_logfile() ) return not_readable( logname );
  domain.crop( logfile.extent() );
  if( domain.empty() ) return empty_domain();
  logfile.split_by_domain_borders( domain );

  for( int i = 0; i < logfile.sblocks(); ++i )
    {
    const Sblock & sb = logfile.sblock( i );
    if( !domain.includes( sb ) )
      { if( domain < sb ) break; else continue; }
    const unsigned j = types1.find( sb.status() );
    if( j < types1.size() )
      logfile.change_sblock_status( i, Sblock::Status( types2[j] ) );
    }
  logfile.compact_sblock_vector();
  logfile.write_logfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Can't close stdout", errno ); return 1; }
  return 0;
  }


int set_for_compare( Domain & domain, Logfile & logfile,
                     const bool as_domain, const bool loose )
  {
  if( !logfile.read_logfile( ( as_domain && loose ) ? '?' : 0 ) )
    return not_readable( logfile.filename() );
  logfile.compact_sblock_vector();
  domain.crop( logfile.extent() );
  if( domain.empty() ) return empty_domain();
  logfile.split_by_domain_borders( domain );
  return -1;
  }

int compare_logfiles( Domain & domain, const char * const logname,
                      const char * const second_logname,
                      const bool as_domain, const bool loose )
  {
  Domain domain2( domain );
  Logfile logfile( logname );
  int retval = set_for_compare( domain, logfile, as_domain, loose );
  if( retval >= 0 ) return retval;

  Logfile logfile2( second_logname );
  retval = set_for_compare( domain2, logfile2, as_domain, loose );
  if( retval >= 0 ) return retval;

  retval = 0;
  if( !as_domain && domain != domain2 ) retval = 1;
  else
    {
    int i = 0, j = 0;
    while( true )
      {
      while( i < logfile.sblocks() &&
             ( !domain.includes( logfile.sblock( i ) ) ||
             ( as_domain && logfile.sblock( i ).status() != Sblock::finished ) ) )
        ++i;
      while( j < logfile2.sblocks() &&
             ( !domain2.includes( logfile2.sblock( j ) ) ||
             ( as_domain && logfile2.sblock( j ).status() != Sblock::finished ) ) )
        ++j;
      if( ( i < logfile.sblocks() ) != ( j < logfile2.sblocks() ) )
        { retval = 1; break; }			// one file has more blocks
      if( i >= logfile.sblocks() ) break;	// successful compare
      if( logfile.sblock( i++ ) != logfile2.sblock( j++ ) )
        { retval = 1; break; }
      }
    }
  if( retval )
    {
    char buf[80];
    snprintf( buf, sizeof buf, "Logfiles '%s' and '%s' differ.",
              logfile.filename(), logfile2.filename() );
    show_error( buf );
    }
  return retval;
  }


int complete_logfile( const char * const logname,
                      const Sblock::Status complete_type )
  {
  Logfile logfile( logname );
  if( !logfile.read_logfile( complete_type ) ) return not_readable( logname );
  logfile.compact_sblock_vector();
  logfile.write_logfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Can't close stdout", errno ); return 1; }
  return 0;
  }


int create_logfile( Domain & domain, const char * const logname,
                    const int hardbs, const Sblock::Status type1,
                    const Sblock::Status type2, const bool force )
  {
  char buf[80];
  Logfile logfile( logname );
  if( !force && logfile.read_logfile() )
    {
    snprintf( buf, sizeof buf,
              "Logfile '%s' exists. Use '--force' to overwrite it.", logname );
    show_error( buf );
    return 1;
    }
  if( domain.empty() ) return empty_domain();
  logfile.make_blank();
  logfile.split_by_domain_borders( domain );

  for( int i = 0; i < logfile.sblocks(); ++i )	// mark all logfile as type2
    logfile.change_sblock_status( i, type2 );

  // mark every block read from stdin and in domain as type1
  for( int linenum = 1; ; ++linenum )
    {
    long long block;
    const int n = std::scanf( "%lli\n", &block );
    if( n < 0 ) break;				// EOF
    if( n != 1 || block < 0 || block > LLONG_MAX / hardbs )
      {
      snprintf( buf, sizeof buf,
                "error reading block number from stdin, line %d", linenum );
      show_error( buf );
      return 2;
      }
    const Block b( block * hardbs, hardbs );
    if( domain.includes( b ) )
      logfile.change_chunk_status( b, type1, domain );
    }
  logfile.truncate_vector( domain.end(), true );
  if( !logfile.write_logfile() ) return 1;
  return 0;
  }


int test_if_done( Domain & domain, const char * const logname, const bool del )
  {
  char buf[80];
  Logfile logfile( logname );
  if( !logfile.read_logfile() ) return not_readable( logname );
  domain.crop( logfile.extent() );
  if( domain.empty() ) return empty_domain();
  logfile.split_by_domain_borders( domain );

  for( int i = 0; i < logfile.sblocks(); ++i )
    {
    const Sblock & sb = logfile.sblock( i );
    if( !domain.includes( sb ) )
      { if( domain < sb ) break; else continue; }
    if( sb.status() != Sblock::finished )
      {
      if( verbosity >= 1 )
        {
        snprintf( buf, sizeof buf, "Logfile '%s' not done.", logname );
        show_error( buf );
        }
      return 1;
      }
    }
  if( !del ) return 0;
  if( std::remove( logname ) != 0 )
    {
    snprintf( buf, sizeof buf, "Error deleting logfile '%s'", logname );
    show_error( buf, errno );
    return 1;
    }
  if( verbosity >= 1 )
    {
    snprintf( buf, sizeof buf, "Logfile '%s' successfully deleted.", logname );
    show_error( buf );
    }
  return 0;
  }


int to_badblocks( const long long offset, Domain & domain,
                  const char * const logname, const int hardbs,
                  const std::string & blocktypes )
  {
  long long last_block = -1;
  Logfile logfile( logname );
  if( !logfile.read_logfile() ) return not_readable( logname );
  domain.crop( logfile.extent() );
  if( domain.empty() ) return empty_domain();
  logfile.split_by_domain_borders( domain );

  for( int i = 0; i < logfile.sblocks(); ++i )
    {
    const Sblock & sb = logfile.sblock( i );
    if( !domain.includes( sb ) )
      { if( domain < sb ) break; else continue; }
    if( blocktypes.find( sb.status() ) >= blocktypes.size() ) continue;
    for( long long block = ( sb.pos() + offset ) / hardbs;
         block * hardbs < sb.end() + offset; ++block )
      {
      if( block > last_block )
        {
        last_block = block;
        std::printf( "%lld\n", block );
        }
      else if( block < last_block ) internal_error( "block out of order." );
      }
    }
  return 0;
  }


// Shows the fraction "num/den" as a percentage with "prec" decimals.
// If 'prec' is negative, only the needed decimals are shown.
//
const char * format_percentage( long long num, long long den,
                                const int iwidth = 3, int prec = -2 )
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


int do_show_status( Domain & domain, const char * const logname )
  {
  long long size_non_tried = 0, size_non_trimmed = 0, size_non_scraped = 0;
  long long size_bad_sector = 0, size_finished = 0;
  int areas_non_tried = 0, areas_non_trimmed = 0, areas_non_scraped = 0;
  int areas_bad_sector = 0, areas_finished = 0;
  int errors = 0;
  Sblock::Status old_status = Sblock::non_tried;
  bool first_block = true, good = true;
  Logfile logfile( logname );
  if( !logfile.read_logfile() ) return not_readable( logname );
  const Block extent = logfile.extent();
  domain.crop( extent );
  if( domain.empty() ) return empty_domain();
  const int true_sblocks = logfile.sblocks();
  logfile.split_by_domain_borders( domain );

  for( int i = 0; i < logfile.sblocks(); ++i )
    {
    const Sblock & sb = logfile.sblock( i );
    if( !domain.includes( sb ) )
      {
      if( domain < sb ) break;
      else { first_block = true; good = true; continue; }
      }
    const bool sc = ( first_block || sb.status() != old_status );
    first_block = false;
    switch( sb.status() )
      {
      case Sblock::non_tried:   size_non_tried += sb.size(); good = true;
                                if( sc ) ++areas_non_tried; break;
      case Sblock::finished:    size_finished += sb.size(); good = true;
                                if( sc ) ++areas_finished; break;
      case Sblock::non_trimmed: size_non_trimmed += sb.size();
                                if( good ) { good = false; ++errors; }
                                if( sc ) ++areas_non_trimmed; break;
      case Sblock::non_scraped: size_non_scraped += sb.size();
                                if( good ) { good = false; ++errors; }
                                if( sc ) ++areas_non_scraped; break;
      case Sblock::bad_sector:  size_bad_sector += sb.size();
                                if( good ) { good = false; ++errors; }
                                if( sc ) ++areas_bad_sector; break;
      }
    old_status = sb.status();
    }

  const long long domain_size = domain.in_size();
  const long long errsize = size_non_trimmed + size_non_scraped + size_bad_sector;
  std::printf( "\n   current pos: %10sB,  current status: %s\n",
               format_num( logfile.current_pos() ),
               logfile.status_name( logfile.current_status() ) );
  std::printf( "logfile extent: %10sB,  in %5d area(s)\n",
               format_num( extent.size() ), true_sblocks );
  if( domain.pos() > 0 || domain.end() < extent.end() )
    {
    std::printf( "  domain begin: %10sB,  domain end: %10sB\n",
                 format_num( domain.pos() ), format_num( domain.end() ) );
    std::printf( "   domain size: %10sB,  in %5d area(s)\n",
                 format_num( domain_size ), domain.blocks() );
    }
  std::printf( "       rescued: %10sB,  in %5d area(s)  (%s)\n",
               format_num( size_finished ), areas_finished,
               format_percentage( size_finished, domain_size ) );
  std::printf( "     non-tried: %10sB,  in %5d area(s)  (%s)\n",
               format_num( size_non_tried ), areas_non_tried,
               format_percentage( size_non_tried, domain_size ) );
  std::printf( "\n       errsize: %10sB,  errors: %8u  (%s)\n",
               format_num( errsize ), errors,
               format_percentage( errsize, domain_size ) );
  std::printf( "   non-trimmed: %10sB,  in %5d area(s)  (%s)\n",
               format_num( size_non_trimmed ), areas_non_trimmed,
               format_percentage( size_non_trimmed, domain_size ) );
  std::printf( "   non-scraped: %10sB,  in %5d area(s)  (%s)\n",
               format_num( size_non_scraped ), areas_non_scraped,
               format_percentage( size_non_scraped, domain_size ) );
  std::printf( "    bad-sector: %10sB,  in %5d area(s)  (%s)\n",
               format_num( size_bad_sector ), areas_bad_sector,
               format_percentage( size_bad_sector, domain_size ) );
  return 0;
  }

} // end namespace


#include "main_common.cc"


int main( const int argc, const char * const argv[] )
  {
  long long ipos = 0;
  long long opos = -1;
  long long max_size = -1;
  const char * domain_logfile_name = 0;
  const char * second_logname = 0;
  const int default_hardbs = 512;
  int hardbs = default_hardbs;
  Mode program_mode = m_none;
  bool as_domain = false;
  bool force = false;
  bool loose = false;
  std::string types1, types2;
  Sblock::Status type1 = Sblock::finished, type2 = Sblock::bad_sector;
  Sblock::Status complete_type = Sblock::non_tried;
  invocation_name = argv[0];
  command_line = argv[0];
  for( int i = 1; i < argc; ++i )
    { command_line += ' '; command_line += argv[i]; }

  const Arg_parser::Option options[] =
    {
    { 'a', "change-types",        Arg_parser::yes },
    { 'b', "block-size",          Arg_parser::yes },
    { 'b', "sector-size",         Arg_parser::yes },
    { 'B', "binary-prefixes",     Arg_parser::no  },
    { 'c', "create-logfile",      Arg_parser::maybe },
    { 'C', "complete-logfile",    Arg_parser::maybe },
    { 'd', "delete-if-done",      Arg_parser::no  },
    { 'D', "done-status",         Arg_parser::no  },
    { 'f', "force",               Arg_parser::no  },
    { 'h', "help",                Arg_parser::no  },
    { 'i', "input-position",      Arg_parser::yes },
    { 'l', "list-blocks",         Arg_parser::yes },
    { 'L', "loose-domain",        Arg_parser::no  },
    { 'm', "domain-logfile",      Arg_parser::yes },
    { 'n', "invert-logfile",      Arg_parser::no  },
    { 'o', "output-position",     Arg_parser::yes },
    { 'p', "compare-logfile",     Arg_parser::yes },
    { 'P', "compare-as-domain",   Arg_parser::yes },
    { 'q', "quiet",               Arg_parser::no  },
    { 's', "size",                Arg_parser::yes },
    { 's', "max-size",            Arg_parser::yes },
    { 't', "show-status",         Arg_parser::no  },
    { 'v', "verbose",             Arg_parser::no  },
    { 'V', "version",             Arg_parser::no  },
    { 'x', "xor-logfile",         Arg_parser::yes },
    { 'y', "and-logfile",         Arg_parser::yes },
    { 'z', "or-logfile",          Arg_parser::yes },
    {  0 , 0,                     Arg_parser::no  } };

  const Arg_parser parser( argc, argv, options );
  if( parser.error().size() )				// bad option
    { show_error( parser.error().c_str(), 0, true ); return 1; }

  int argind = 0;
  for( ; argind < parser.arguments(); ++argind )
    {
    const int code = parser.code( argind );
    if( !code ) break;					// no more options
    const char * const arg = parser.argument( argind ).c_str();
    switch( code )
      {
      case 'a': set_mode( program_mode, m_change );
                parse_types( arg, types1, types2 ); break;
      case 'b': hardbs = getnum( arg, 0, 1, INT_MAX ); break;
      case 'B': format_num( 0, 0, -1 ); break;		// set binary prefixes
      case 'c': set_mode( program_mode, m_create );
                parse_2types( arg, type1, type2 ); break;
      case 'C': set_mode( program_mode, m_complete );
                parse_type( arg, complete_type ); break;
      case 'd': set_mode( program_mode, m_delete ); break;
      case 'D': set_mode( program_mode, m_done_st ); break;
      case 'f': force = true; break;
      case 'h': show_help( default_hardbs ); return 0;
      case 'i': ipos = getnum( arg, hardbs, 0 ); break;
      case 'l': set_mode( program_mode, m_list ); types1 = arg;
                check_types( types1, "list-blocks" ); break;
      case 'L': loose = true; break;
      case 'm': set_name( &domain_logfile_name, arg, code ); break;
      case 'n': set_mode( program_mode, m_invert ); break;
      case 'o': opos = getnum( arg, hardbs, 0 ); break;
      case 'p':
      case 'P': set_mode( program_mode, m_compare );
                second_logname = arg; as_domain = ( code == 'P' ); break;
      case 'q': verbosity = -1; break;
      case 's': max_size = getnum( arg, hardbs, -1 ); break;
      case 't': set_mode( program_mode, m_status ); break;
      case 'v': if( verbosity < 4 ) ++verbosity; break;
      case 'V': show_version(); return 0;
      case 'x': set_mode( program_mode, m_xor );
                second_logname = arg; break;
      case 'y': set_mode( program_mode, m_and );
                second_logname = arg; break;
      case 'z': set_mode( program_mode, m_or );
                second_logname = arg; break;
      default : internal_error( "uncaught option." );
      }
    } // end process options

  if( program_mode == m_none )
    {
    show_error( "You must specify the operation to be performed.", 0, true );
    return 1;
    }

  if( opos < 0 ) opos = ipos;

  if( argind + 1 != parser.arguments() )
    {
    if( argind < parser.arguments() )
      show_error( "Too many files.", 0, true );
    else
      show_error( "A logfile must be specified.", 0, true );
    return 1;
    }

  const char * const logname = parser.argument( argind++ ).c_str();

  // end scan arguments

  Domain domain( ipos, max_size, domain_logfile_name, loose );

  switch( program_mode )
    {
    case m_none: internal_error( "invalid operation." ); break;
    case m_and:
    case m_or:
    case m_xor:
      return do_logic_ops( domain, logname, second_logname, program_mode );
    case m_change: return change_types( domain, logname, types1, types2 );
    case m_compare:
      return compare_logfiles( domain, logname, second_logname, as_domain, loose );
    case m_complete: return complete_logfile( logname, complete_type );
    case m_create: return create_logfile( domain, logname, hardbs,
                                          type1, type2, force );
    case m_delete: return test_if_done( domain, logname, true );
    case m_done_st: return test_if_done( domain, logname, false );
    case m_invert: return change_types( domain, logname, "?*/-+", "++++-" );
    case m_list:
      return to_badblocks( opos - ipos, domain, logname, hardbs, types1 );
    case m_status: return do_show_status( domain, logname );
    }
  }
