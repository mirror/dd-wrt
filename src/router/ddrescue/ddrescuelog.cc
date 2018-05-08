/*  GNU ddrescuelog - Tool for ddrescue mapfiles
    Copyright (C) 2011-2018 Antonio Diaz Diaz.

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


namespace {

const char * const Program_name = "GNU ddrescuelog";
const char * const program_name = "ddrescuelog";
const char * invocation_name = 0;

enum Mode { m_none, m_and, m_annotate, m_change, m_compare, m_complete,
            m_create, m_delete, m_done_st, m_invert, m_list, m_or,
            m_shift, m_status, m_xor };


void show_help( const int hardbs )
  {
  std::printf( "%s - Tool for ddrescue mapfiles.\n", Program_name );
  std::printf( "Manipulates ddrescue mapfiles, shows their contents, converts them to/from\n"
               "other formats, compares them, and tests rescue status.\n"
               "\nNOTE: In versions of ddrescue prior to 1.20 the mapfile was called\n"
               "'logfile'. The format is the same; only the name has changed.\n"
               "\nUsage: %s [options] mapfile\n", invocation_name );
  std::printf( "\nOptions:\n"
               "  -h, --help                      display this help and exit\n"
               "  -V, --version                   output version information and exit\n"
               "  -a, --change-types=<ot>,<nt>    change the block types of mapfile\n"
               "  -A, --annotate-mapfile          add comments with human-readable pos/sizes\n"
               "  -b, --block-size=<bytes>        block size in bytes [default %d]\n", hardbs );
  std::printf( "  -B, --binary-prefixes           show binary multipliers in numbers [SI]\n"
               "  -c, --create-mapfile[=<tt>]     create mapfile from list of blocks [+-]\n"
               "  -C, --complete-mapfile[=<t>]    complete mapfile adding blocks of type t [?]\n"
               "  -d, --delete-if-done            delete the mapfile if rescue is finished\n"
               "  -D, --done-status               return 0 if rescue is finished\n"
               "  -f, --force                     overwrite existing output files\n"
               "  -i, --input-position=<bytes>    starting position of rescue domain [0]\n"
               "  -l, --list-blocks=<types>       print block numbers of given types (?*/-+)\n"
               "  -L, --loose-domain              accept an incomplete domain mapfile\n"
               "  -m, --domain-mapfile=<file>     restrict domain to finished blocks in file\n"
               "  -n, --invert-mapfile            invert block types (finished <--> others)\n"
               "  -o, --output-position=<bytes>   starting position in output file [ipos]\n"
               "  -p, --compare-mapfile=<file>    compare block types in domain of both files\n"
               "  -P, --compare-as-domain=<file>  like -p but compare finished blocks only\n"
               "  -q, --quiet                     suppress all messages\n"
               "  -s, --size=<bytes>              maximum size of rescue domain to be processed\n"
               "  -t, --show-status               show a summary of mapfile contents\n"
               "  -v, --verbose                   be verbose (a 2nd -v gives more)\n"
               "  -x, --xor-mapfile=<file>        XOR the finished blocks in file with mapfile\n"
               "  -y, --and-mapfile=<file>        AND the finished blocks in file with mapfile\n"
               "  -z, --or-mapfile=<file>         OR the finished blocks in file with mapfile\n"
               "      --shift                     shift all block positions by (opos - ipos)\n"
               "Use '-' to read a mapfile from standard input or to write the mapfile\n"
               "created by '--create-mapfile' to standard output.\n"
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
    show_error( "Invalid type for 'create-mapfile' option.", 0, true );
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
    show_error( "Invalid type for 'complete-mapfile' option.", 0, true );
    std::exit( 1 );
    }
  complete_type = Sblock::Status( arg[0] );
  }


int do_logic_ops( Domain & domain, const char * const mapname,
                  const char * const second_mapname, const Mode program_mode )
  {
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile() ) return not_readable( mapname );
  mapfile.compact_sblock_vector();

  Mapfile mapfile2( second_mapname );
  if( !mapfile2.read_mapfile() ) return not_readable( second_mapname );
  mapfile2.compact_sblock_vector();

  domain.crop( mapfile.extent() );
  domain.crop( mapfile2.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.split_by_domain_borders( domain );
  mapfile2.split_by_domain_borders( domain );
  mapfile.split_by_mapfile_borders( mapfile2 );
  mapfile2.split_by_mapfile_borders( mapfile );

  for( long i = 0, j = 0; ; ++i, ++j )
    {
    while( i < mapfile.sblocks() && !domain.includes( mapfile.sblock( i ) ) )
      ++i;
    while( j < mapfile2.sblocks() && !domain.includes( mapfile2.sblock( j ) ) )
      ++j;
    if( i >= mapfile.sblocks() || j >= mapfile2.sblocks() ) break;
    const Sblock & sb1 = mapfile.sblock( i );
    const Sblock & sb2 = mapfile2.sblock( j );
    if( sb1.pos() != sb2.pos() || sb1.size() != sb2.size() )
      internal_error( "blocks got out of sync." );
    const bool f1 = ( sb1.status() == Sblock::finished );
    const bool f2 = ( sb2.status() == Sblock::finished );
    switch( program_mode )
      {
      case m_and:
        if( f1 && !f2 ) mapfile.change_sblock_status( i, Sblock::bad_sector );
        break;
      case m_or:
        if( !f1 && f2 ) mapfile.change_sblock_status( i, Sblock::finished );
        break;
      case m_xor:
        if( f2 )
          mapfile.change_sblock_status( i, f1 ? Sblock::bad_sector : Sblock::finished );
        break;
      default: internal_error( "invalid program_mode." );
      }
    }
  mapfile.compact_sblock_vector();
  mapfile.write_mapfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Error closing stdout", errno ); return 1; }
  return 0;
  }


int annotate_mapfile( Domain & domain, const char * const mapname )
  {
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile() ) return not_readable( mapname );
  domain.crop( mapfile.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.split_by_domain_borders( domain );
  mapfile.write_mapfile( stdout, false, false, &domain );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Error closing stdout", errno ); return 1; }
  return 0;
  }


int change_types( Domain & domain, const char * const mapname,
                  const std::string & types1, const std::string & types2 )
  {
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile() ) return not_readable( mapname );
  domain.crop( mapfile.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.split_by_domain_borders( domain );

  for( long i = 0; i < mapfile.sblocks(); ++i )
    {
    const Sblock & sb = mapfile.sblock( i );
    if( !domain.includes( sb ) )
      { if( domain < sb ) break; else continue; }
    const unsigned j = types1.find( sb.status() );
    if( j < types1.size() )
      mapfile.change_sblock_status( i, Sblock::Status( types2[j] ) );
    }
  mapfile.compact_sblock_vector();
  mapfile.write_mapfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Error closing stdout", errno ); return 1; }
  return 0;
  }


int set_for_compare( Domain & domain, Mapfile & mapfile,
                     const bool as_domain, const bool loose )
  {
  if( !mapfile.read_mapfile( ( as_domain && loose ) ? '?' : 0 ) )
    return not_readable( mapfile.filename() );
  mapfile.compact_sblock_vector();
  domain.crop( mapfile.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.split_by_domain_borders( domain );
  return -1;
  }

int compare_mapfiles( Domain & domain, const char * const mapname,
                      const char * const second_mapname,
                      const bool as_domain, const bool loose )
  {
  Domain domain2( domain );
  Mapfile mapfile( mapname );
  int retval = set_for_compare( domain, mapfile, as_domain, loose );
  if( retval >= 0 ) return retval;

  Mapfile mapfile2( second_mapname );
  retval = set_for_compare( domain2, mapfile2, as_domain, loose );
  if( retval >= 0 ) return retval;

  retval = 0;
  if( !as_domain && domain != domain2 ) retval = 1;
  else
    {
    long i = 0, j = 0;
    while( true )
      {
      while( i < mapfile.sblocks() &&
             ( !domain.includes( mapfile.sblock( i ) ) ||
             ( as_domain && mapfile.sblock( i ).status() != Sblock::finished ) ) )
        ++i;
      while( j < mapfile2.sblocks() &&
             ( !domain2.includes( mapfile2.sblock( j ) ) ||
             ( as_domain && mapfile2.sblock( j ).status() != Sblock::finished ) ) )
        ++j;
      if( ( i < mapfile.sblocks() ) != ( j < mapfile2.sblocks() ) )
        { retval = 1; break; }			// one file has more blocks
      if( i >= mapfile.sblocks() ) break;	// successful compare
      if( mapfile.sblock( i++ ) != mapfile2.sblock( j++ ) )
        { retval = 1; break; }
      }
    }
  if( retval )
    {
    char buf[80];
    snprintf( buf, sizeof buf, "Mapfiles '%s' and '%s' differ.",
              mapfile.filename(), mapfile2.filename() );
    show_error( buf );
    }
  return retval;
  }


int complete_mapfile( const char * const mapname,
                      const Sblock::Status complete_type )
  {
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile( complete_type ) )
    return not_readable( mapname );
  mapfile.compact_sblock_vector();
  mapfile.write_mapfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Error closing stdout", errno ); return 1; }
  return 0;
  }


int create_mapfile( Domain & domain, const char * const mapname,
                    const int hardbs, const Sblock::Status type1,
                    const Sblock::Status type2, const bool force )
  {
  if( domain.empty() ) return empty_domain();
  char buf[80];
  Mapfile mapfile( mapname );
  const bool to_stdout = ( std::strcmp( mapname, "-" ) == 0 );
  if( !to_stdout && !force && mapfile.read_mapfile( 0, false ) )
    {
    snprintf( buf, sizeof buf,
              "Mapfile '%s' exists. Use '--force' to overwrite it.", mapname );
    show_error( buf );
    return 1;
    }
  mapfile.set_to_status( type2 );		// mark all mapfile as type2
  mapfile.split_by_domain_borders( domain );

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
      mapfile.change_chunk_status( b, type1, domain );
    }
  mapfile.truncate_vector( domain.end(), true );
  if( !mapfile.write_mapfile( to_stdout ? stdout : 0 ) ) return 1;
  if( to_stdout && std::fclose( stdout ) != 0 )
    { show_error( "Error closing stdout", errno ); return 1; }
  return 0;
  }


int test_if_done( Domain & domain, const char * const mapname, const bool del )
  {
  char buf[80];
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile( 0, !del ) ) return not_readable( mapname );
  domain.crop( mapfile.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.split_by_domain_borders( domain );

  for( long i = 0; i < mapfile.sblocks(); ++i )
    {
    const Sblock & sb = mapfile.sblock( i );
    if( !domain.includes( sb ) )
      { if( domain < sb ) break; else continue; }
    if( sb.status() != Sblock::finished )
      {
      if( verbosity >= 1 )
        {
        snprintf( buf, sizeof buf, "Mapfile '%s' not done.", mapname );
        show_error( buf );
        }
      return 1;
      }
    }
  if( !del ) return 0;
  if( std::remove( mapname ) != 0 )
    {
    snprintf( buf, sizeof buf, "Error deleting mapfile '%s'", mapname );
    show_error( buf, errno );
    return 1;
    }
  if( verbosity >= 1 )
    {
    snprintf( buf, sizeof buf, "Mapfile '%s' successfully deleted.", mapname );
    show_error( buf );
    }
  return 0;
  }


int shift_blocks( const long long ipos, const long long opos,
                  Domain & domain, const char * const mapname )
  {
  if( ipos != 0 && opos != 0 )
    { show_error( "Either '-i' or '-o' must be 0" ); return 1; }
  const long long offset = opos - ipos;
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile() ) return not_readable( mapname );
  domain.crop( mapfile.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.truncate_vector( domain.end(), true );
  mapfile.shift_blocks( offset );
  mapfile.compact_sblock_vector();
  mapfile.write_mapfile( stdout );
  if( std::fclose( stdout ) != 0 )
    { show_error( "Error closing stdout", errno ); return 1; }
  return 0;
  }


int to_badblocks( const long long offset, Domain & domain,
                  const char * const mapname, const int hardbs,
                  const std::string & blocktypes )
  {
  long long last_block = -1;
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile() ) return not_readable( mapname );
  domain.crop( mapfile.extent() );
  if( domain.empty() ) return empty_domain();
  mapfile.split_by_domain_borders( domain );

  for( long i = 0; i < mapfile.sblocks(); ++i )
    {
    const Sblock & sb = mapfile.sblock( i );
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


int do_show_status( Domain & domain, const char * const mapname,
                    const bool loose )
  {
  long long non_tried_size = 0, non_trimmed_size = 0;
  long long non_scraped_size = 0, bad_size = 0, finished_size = 0;
  unsigned long non_tried_areas = 0, non_trimmed_areas = 0;
  unsigned long non_scraped_areas = 0, bad_areas = 0, finished_areas = 0;
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile( loose ? '?' : 0 ) ) return not_readable( mapname );
  mapfile.compact_sblock_vector();
  const Block extent = mapfile.extent();
  domain.crop( extent );
  if( domain.empty() ) return empty_domain();
  const long true_sblocks = mapfile.sblocks();
  mapfile.split_by_domain_borders( domain );

  for( long i = 0; i < mapfile.sblocks(); ++i )
    {
    const Sblock & sb = mapfile.sblock( i );
    if( !domain.includes( sb ) )
      { if( domain < sb ) break; else continue; }
    switch( sb.status() )
      {
      case Sblock::non_tried:   non_tried_size += sb.size();
                                ++non_tried_areas; break;
      case Sblock::non_trimmed: non_trimmed_size += sb.size();
                                ++non_trimmed_areas; break;
      case Sblock::non_scraped: non_scraped_size += sb.size();
                                ++non_scraped_areas; break;
      case Sblock::bad_sector:  bad_size += sb.size();
                                ++bad_areas; break;
      case Sblock::finished:    finished_size += sb.size();
                                ++finished_areas; break;
      }
    }

  const long long domain_size = domain.in_size();
  if( verbosity >= 1 ) std::printf( "\n%s", mapname );
  std::printf( "\n   current pos: %9sB,  current status: %s\n",
               format_num( mapfile.current_pos() ),
               mapfile.status_name( mapfile.current_status() ) );
  std::printf( "mapfile extent: %9sB,  in %6ld area(s)\n",
               format_num( extent.size() ), true_sblocks );
  if( domain.pos() > 0 || domain.end() < extent.end() || domain.blocks() > 1 )
    {
    std::printf( "  domain begin: %9sB,  domain end: %9sB\n",
                 format_num( domain.pos() ), format_num( domain.end() ) );
    std::printf( "   domain size: %9sB,  in %6ld area(s)\n",
                 format_num( domain_size ), domain.blocks() );
    }
  std::printf( "\n     non-tried: %9sB,  in %6lu area(s)  (%s)\n",
               format_num( non_tried_size ), non_tried_areas,
               format_percentage( non_tried_size, domain_size ) );
  std::printf( "       rescued: %9sB,  in %6lu area(s)  (%s)\n",
               format_num( finished_size ), finished_areas,
               format_percentage( finished_size, domain_size ) );
  std::printf( "   non-trimmed: %9sB,  in %6lu area(s)  (%s)\n",
               format_num( non_trimmed_size ), non_trimmed_areas,
               format_percentage( non_trimmed_size, domain_size ) );
  std::printf( "   non-scraped: %9sB,  in %6lu area(s)  (%s)\n",
               format_num( non_scraped_size ), non_scraped_areas,
               format_percentage( non_scraped_size, domain_size ) );
  std::printf( "    bad-sector: %9sB,  in %6lu area(s)  (%s)\n",
               format_num( bad_size ), bad_areas,
               format_percentage( bad_size, domain_size ) );
  return 0;
  }

} // end namespace


#include "main_common.cc"


int main( const int argc, const char * const argv[] )
  {
  long long ipos = 0;
  long long opos = -1;
  long long max_size = -1;
  const char * domain_mapfile_name = 0;
  const char * second_mapname = 0;
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

  enum Optcode { opt_shi = 256 };
  const Arg_parser::Option options[] =
    {
    { 'a', "change-types",        Arg_parser::yes },
    { 'A', "annotate-mapfile",    Arg_parser::no  },
    { 'b', "block-size",          Arg_parser::yes },
    { 'b', "sector-size",         Arg_parser::yes },
    { 'B', "binary-prefixes",     Arg_parser::no  },
    { 'c', "create-mapfile",      Arg_parser::maybe },
    { 'c', "create-logfile",      Arg_parser::maybe },
    { 'C', "complete-mapfile",    Arg_parser::maybe },
    { 'C', "complete-logfile",    Arg_parser::maybe },
    { 'd', "delete-if-done",      Arg_parser::no  },
    { 'D', "done-status",         Arg_parser::no  },
    { 'f', "force",               Arg_parser::no  },
    { 'h', "help",                Arg_parser::no  },
    { 'i', "input-position",      Arg_parser::yes },
    { 'l', "list-blocks",         Arg_parser::yes },
    { 'L', "loose-domain",        Arg_parser::no  },
    { 'm', "domain-mapfile",      Arg_parser::yes },
    { 'm', "domain-logfile",      Arg_parser::yes },
    { 'n', "invert-mapfile",      Arg_parser::no  },
    { 'n', "invert-logfile",      Arg_parser::no  },
    { 'o', "output-position",     Arg_parser::yes },
    { 'p', "compare-mapfile",     Arg_parser::yes },
    { 'p', "compare-logfile",     Arg_parser::yes },
    { 'P', "compare-as-domain",   Arg_parser::yes },
    { 'q', "quiet",               Arg_parser::no  },
    { 's', "size",                Arg_parser::yes },
    { 's', "max-size",            Arg_parser::yes },
    { 't', "show-status",         Arg_parser::no  },
    { 'v', "verbose",             Arg_parser::no  },
    { 'V', "version",             Arg_parser::no  },
    { 'x', "xor-mapfile",         Arg_parser::yes },
    { 'x', "xor-logfile",         Arg_parser::yes },
    { 'y', "and-mapfile",         Arg_parser::yes },
    { 'y', "and-logfile",         Arg_parser::yes },
    { 'z', "or-mapfile",          Arg_parser::yes },
    { 'z', "or-logfile",          Arg_parser::yes },
    { opt_shi, "shift",           Arg_parser::no  },
    {  0 , 0,                     Arg_parser::no  } };

  const Arg_parser parser( argc, argv, options );
  if( parser.error().size() )				// bad option
    { show_error( parser.error().c_str(), 0, true ); return 1; }

  int argind = 0;
  for( ; argind < parser.arguments(); ++argind )
    {
    const int code = parser.code( argind );
    if( !code ) break;					// no more options
    const std::string & arg = parser.argument( argind );
    const char * const ptr = arg.c_str();
    switch( code )
      {
      case 'a': set_mode( program_mode, m_change );
                parse_types( arg, types1, types2 ); break;
      case 'A': set_mode( program_mode, m_annotate ); break;
      case 'b': hardbs = getnum( ptr, 0, 1, INT_MAX ); break;
      case 'B': format_num( 0, 0, -1 ); break;		// set binary prefixes
      case 'c': set_mode( program_mode, m_create );
                parse_2types( arg, type1, type2 ); break;
      case 'C': set_mode( program_mode, m_complete );
                parse_type( arg, complete_type ); break;
      case 'd': set_mode( program_mode, m_delete ); break;
      case 'D': set_mode( program_mode, m_done_st ); break;
      case 'f': force = true; break;
      case 'h': show_help( default_hardbs ); return 0;
      case 'i': ipos = getnum( ptr, hardbs, 0 ); break;
      case 'l': set_mode( program_mode, m_list ); types1 = arg;
                check_types( types1, "list-blocks" ); break;
      case 'L': loose = true; break;
      case 'm': set_name( &domain_mapfile_name, ptr, code ); break;
      case 'n': set_mode( program_mode, m_invert ); break;
      case 'o': opos = getnum( ptr, hardbs, 0 ); break;
      case 'p':
      case 'P': set_mode( program_mode, m_compare );
                second_mapname = ptr; as_domain = ( code == 'P' ); break;
      case 'q': verbosity = -1; break;
      case 's': max_size = getnum( ptr, hardbs, -1 ); break;
      case 't': set_mode( program_mode, m_status ); break;
      case 'v': if( verbosity < 4 ) ++verbosity; break;
      case 'V': show_version(); return 0;
      case 'x': set_mode( program_mode, m_xor );
                second_mapname = ptr; break;
      case 'y': set_mode( program_mode, m_and );
                second_mapname = ptr; break;
      case 'z': set_mode( program_mode, m_or );
                second_mapname = ptr; break;
      case opt_shi: set_mode( program_mode, m_shift ); break;
      default : internal_error( "uncaught option." );
      }
    } // end process options

  if( program_mode == m_none )
    {
    show_error( "You must specify the operation to be performed.", 0, true );
    return 1;
    }

  if( opos < 0 ) opos = ipos;

  if( program_mode == m_status )
    {
    if( argind >= parser.arguments() )
      { show_error( "At least one mapfile must be specified.", 0, true );
        return 1; }
    }
  else if( argind + 1 != parser.arguments() )
    {
    if( argind < parser.arguments() )
      show_error( "Too many files.", 0, true );
    else
      show_error( "A mapfile must be specified.", 0, true );
    return 1;
    }

  int retval = 0;
  for( ; argind < parser.arguments(); ++argind )
    {
    const char * const mapname = parser.argument( argind ).c_str();
    Domain domain( ipos, max_size, domain_mapfile_name, loose );

    switch( program_mode )
      {
      case m_none: internal_error( "invalid operation." ); break;
      case m_and:
      case m_or:
      case m_xor:
        return do_logic_ops( domain, mapname, second_mapname, program_mode );
      case m_annotate: return annotate_mapfile( domain, mapname );
      case m_change: return change_types( domain, mapname, types1, types2 );
      case m_compare: return compare_mapfiles( domain, mapname, second_mapname,
                                               as_domain, loose );
      case m_complete: return complete_mapfile( mapname, complete_type );
      case m_create: return create_mapfile( domain, mapname, hardbs,
                                            type1, type2, force );
      case m_delete: return test_if_done( domain, mapname, true );
      case m_done_st: return test_if_done( domain, mapname, false );
      case m_invert: return change_types( domain, mapname, "?*/-+", "++++-" );
      case m_list:
        return to_badblocks( opos - ipos, domain, mapname, hardbs, types1 );
      case m_shift:
        return shift_blocks( ipos, opos, domain, mapname );
      case m_status:
        retval = std::max( retval, do_show_status( domain, mapname, loose ) );
      }
    }
  return retval;
  }
