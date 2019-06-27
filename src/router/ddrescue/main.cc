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
/*
    Exit status: 0 for a normal exit, 1 for environmental problems
    (file not found, invalid flags, I/O errors, etc), 2 to indicate a
    corrupt or invalid input file, 3 for an internal consistency error
    (eg, bug) which caused ddrescue to panic.
*/

#define _FILE_OFFSET_BITS 64

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

#include "arg_parser.h"
#include "rational.h"
#include "block.h"
#include "ddrescue.h"
#include "linux.h"
#include "loggers.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef O_DIRECT
#define O_DIRECT 0
#endif


namespace {

const char * const Program_name = "GNU ddrescue";
const char * const program_name = "ddrescue";
const char * invocation_name = 0;

enum Mode { m_none, m_fill, m_generate };
const mode_t outmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;


void show_help( const int cluster, const int hardbs, const int skipbs )
  {
  std::printf( "%s - Data recovery tool.\n", Program_name );
  std::printf( "Copies data from one file or block device to another,\n"
               "trying to rescue the good parts first in case of read errors.\n"
               "\nUsage: %s [options] infile outfile [logfile]\n", invocation_name );
  std::printf( "You should use a logfile unless you know what you are doing.\n"
               "If you reboot, check the device names before restarting ddrescue.\n"
               "Do not use options '-F' or '-G' without reading the manual first.\n"
               "\nOptions:\n"
               "  -h, --help                     display this help and exit\n"
               "  -V, --version                  output version information and exit\n"
               "  -a, --min-read-rate=<bytes>    minimum read rate of good areas in bytes/s\n"
               "  -A, --try-again                mark non-trimmed, non-scraped as non-tried\n"
               "  -b, --sector-size=<bytes>      sector size of input device [default %d]\n", hardbs );
  std::printf( "  -B, --binary-prefixes          show binary multipliers in numbers [SI]\n"
               "  -c, --cluster-size=<sectors>   sectors to copy at a time [%d]\n", cluster );
  std::printf( "  -C, --complete-only            do not read new data beyond logfile limits\n"
               "  -d, --direct                   use direct disc access for input file\n"
               "  -D, --synchronous              use synchronous writes for output file\n"
               "  -e, --max-errors=[+]<n>        maximum number of [new] error areas allowed\n"
               "  -E, --max-error-rate=<bytes>   maximum allowed rate of read errors per second\n"
               "  -f, --force                    overwrite output device or partition\n"
               "  -F, --fill-mode=<types>        fill given type blocks with infile data (?*/-+)\n"
               "  -G, --generate-mode            generate approximate logfile from partial copy\n"
               "  -H, --test-mode=<file>         set map of good/bad blocks from given logile\n"
               "  -i, --input-position=<bytes>   starting position of domain in input file [0]\n"
               "  -I, --verify-input-size        verify input file size with size in logfile\n"
               "  -K, --skip-size=<min>[,<max>]  initial size to skip on read error [%sB]\n",
               format_num( skipbs, 9999, -1 ) );
  std::printf( "  -L, --loose-domain             accept an incomplete domain logfile\n"
               "  -m, --domain-logfile=<file>    restrict domain to finished blocks in file\n"
               "  -M, --retrim                   mark all failed blocks as non-trimmed\n"
               "  -n, --no-scrape                skip the scraping phase\n"
               "  -N, --no-trim                  skip the trimming phase\n"
               "  -o, --output-position=<bytes>  starting position in output file [ipos]\n"
               "  -O, --reopen-on-error          reopen input file after every read error\n"
               "  -p, --preallocate              preallocate space on disc for output file\n"
               "  -P, --data-preview[=<lines>]   show some lines of the latest data read [3]\n"
               "  -q, --quiet                    suppress all messages\n"
               "  -r, --retry-passes=<n>         exit after <n> retry passes (-1=infinity) [0]\n"
               "  -R, --reverse                  reverse the direction of all passes\n"
               "  -s, --size=<bytes>             maximum size of input data to be copied\n"
               "  -S, --sparse                   use sparse writes for output file\n"
               "  -t, --truncate                 truncate output file to zero size\n"
               "  -T, --timeout=<interval>       maximum time since last successful read\n"
               "  -u, --unidirectional           run all passes in the same direction\n"
               "  -v, --verbose                  be verbose (a 2nd -v gives more)\n"
               "  -w, --ignore-write-errors      make fill mode ignore write errors\n"
               "  -x, --extend-outfile=<bytes>   extend outfile size to be at least this long\n"
               "  -X, --exit-on-error            exit after the first read error\n"
               "  -1, --log-rates=<file>         log rates and error sizes in file\n"
               "  -2, --log-reads=<file>         log all read operations in file\n"
               "      --ask                      ask for confirmation before starting the copy\n"
               "      --cpass=<n>[,<n>]          select what copying pass(es) to run\n"
               "      --pause=<interval>         time to wait between passes [0]\n" );

    std::printf( "      --scsi-passthrough         use ioctl sg_io direct SCSI passthrough-2.1\n"
                 "      --ata-passthrough          use ioctl sg_io direct ATA passthrough-2.1\n"
                 "      --mark-abnormal-error      mark abnormal error as non-trimmed-2.1\n" );

  std::printf( "Numbers may be in decimal, hexadecimal or octal, and may be followed by a\n"
               "multiplier: s = sectors, k = 1000, Ki = 1024, M = 10^6, Mi = 2^20, etc...\n"
               "Time intervals have the format 1[.5][smhd] or 1/2[smhd].\n"
               "\nExit status: 0 for a normal exit, 1 for environmental problems (file\n"
               "not found, invalid flags, I/O errors, etc), 2 to indicate a corrupt or\n"
               "invalid input file, 3 for an internal consistency error (eg, bug) which\n"
               "caused ddrescue to panic.\n"
               "\nReport bugs to bug-ddrescue@gnu.org\n"
               "Ddrescue home page: http://www.gnu.org/software/ddrescue/ddrescue.html\n"
               "General help using GNU software: http://www.gnu.org/gethelp\n" );
  }


// Recognized formats: <rational_number>[unit]
// Where the optional "unit" is one of 's', 'm', 'h' or 'd'.
// Returns the number of seconds, or exits with 1 status if error.
//
long parse_time_interval( const char * const s )
  {
  Rational r;
  int c = r.parse( s );

  if( c > 0 )
    {
    switch( s[c] )
      {
      case 'd': r *= 86400; break;			// 24 * 60 * 60
      case 'h': r *= 3600; break;			// 60 * 60
      case 'm': r *= 60; break;
      case 's':
      case  0 : break;
      default : show_error( "Bad unit in time interval", 0, true );
                std::exit( 1 );
      }
    const long interval = r.round();
    if( !r.error() && interval >= 0 ) return interval;
    }
  show_error( "Bad value for time interval.", 0, true );
  std::exit( 1 );
  }


bool check_identical( const char * const iname, const char * const oname,
                      const char * const logname )
  {
  struct stat istat, ostat, logstat;
  bool iexists = false, oexists = false, logexists = false;
  bool same = ( std::strcmp( iname, oname ) == 0 );
  if( !same )
    {
    iexists = ( stat( iname, &istat ) == 0 );
    oexists = ( stat( oname, &ostat ) == 0 );
    if( iexists && oexists && istat.st_ino == ostat.st_ino &&
        istat.st_dev == ostat.st_dev ) same = true;
    }
  if( same )
    { show_error( "Infile and outfile are the same." ); return true; }
  if( logname )
    {
    same = ( std::strcmp( iname, logname ) == 0 );
    if( !same )
      {
      logexists = ( stat( logname, &logstat ) == 0 );
      if( iexists && logexists && istat.st_ino == logstat.st_ino &&
          istat.st_dev == logstat.st_dev ) same = true;
      }
    if( same )
      { show_error( "Infile and logfile are the same." ); return true; }
    if( std::strcmp( oname, logname ) == 0 ||
        ( oexists && logexists && ostat.st_ino == logstat.st_ino &&
          ostat.st_dev == logstat.st_dev ) )
      { show_error( "Outfile and logfile are the same." ); return true; }
    }
  return false;
  }


bool check_files( const char * const iname, const char * const oname,
                  const char * const logname,
                  const long long min_outfile_size, const bool force,
                  const bool generate, const bool preallocate,
                  const bool sparse )
  {
  if( !iname || !oname )
    {
    show_error( "Both input and output files must be specified.", 0, true );
    return false;
    }
  if( check_identical( iname, oname, logname ) ) return false;
  if( !generate && ( min_outfile_size > 0 || !force || preallocate || sparse ) )
    {
    struct stat st;
    if( stat( oname, &st ) == 0 && !S_ISREG( st.st_mode ) )
      {
      show_error( "Output file exists and is not a regular file." );
      if( !force )
        show_error( "Use '--force' if you really want to overwrite it, but be\n"
                    "          aware that all existing data in the output file will be lost.",
                    0, true );
      else if( min_outfile_size > 0 )
        show_error( "Only regular files can be extended.", 0, true );
      else if( preallocate )
        show_error( "Only regular files can be preallocated.", 0, true );
      else if( sparse )
        show_error( "Only regular files can be sparse.", 0, true );
      return false;
      }
    }
  return true;
  }


int do_fill( const long long offset, Domain & domain,
             const char * const iname, const char * const oname,
             const char * const logname,
             const int cluster, const int hardbs,
             const std::string & filltypes, const bool ignore_write_errors,
             const bool synchronous )
  {
  if( !logname )
    {
    show_error( "Logfile required in fill mode.", 0, true );
    return 1;
    }

  Fillbook fillbook( offset, domain, logname, cluster, hardbs,
                     ignore_write_errors, synchronous );
  if( !fillbook.logfile_exists() ) return not_readable( logname );
  if( fillbook.domain().empty() ) return empty_domain();
  if( fillbook.read_only() ) return not_writable( logname );

  const int ides = open( iname, O_RDONLY | O_BINARY );
  if( ides < 0 )
    { show_error( "Can't open input file", errno ); return 1; }
  if( !fillbook.read_buffer( ides ) )
    { show_error( "Error reading fill data from input file." ); return 1; }

  const int odes = open( oname, O_CREAT | O_WRONLY | O_BINARY, outmode );
  if( odes < 0 )
    { show_error( "Can't open output file", errno ); return 1; }
  if( lseek( odes, 0, SEEK_SET ) )
    { show_error( "Output file is not seekable." ); return 1; }

  if( verbosity >= 0 )
    std::printf( "%s %s\n", Program_name, PROGVERSION );
  if( verbosity >= 1 )
    {
    std::printf( "About to fill with data from %s blocks of %s marked %s\n",
                 iname, oname, filltypes.c_str() );
    std::printf( "    Maximum size to fill: %sBytes\n",
                 format_num( fillbook.domain().in_size() ) );
    std::printf( "    Starting positions: infile = %sB,  outfile = %sB\n",
                 format_num( fillbook.domain().pos() ),
                 format_num( fillbook.domain().pos() + fillbook.offset() ) );
    std::printf( "    Copy block size: %3d sectors\n", cluster );
    std::printf( "Sector size: %sBytes\n\n", format_num( hardbs, 99999 ) );
    }

  return fillbook.do_fill( odes, filltypes );
  }


int do_generate( const long long offset, Domain & domain,
                 const char * const iname, const char * const oname,
                 const char * const logname,
                 const int cluster, const int hardbs )
  {
  if( !logname )
    {
    show_error( "Logfile must be specified in generate mode.", 0, true );
    return 1;
    }

  const int ides = open( iname, O_RDONLY | O_BINARY );
  if( ides < 0 )
    { show_error( "Can't open input file", errno ); return 1; }
  const long long isize = lseek( ides, 0, SEEK_END );
  if( isize < 0 )
    { show_error( "Input file is not seekable." ); return 1; }

  Genbook genbook( offset, isize, domain, logname, cluster, hardbs );
  if( genbook.domain().empty() ) return empty_domain();
  if( !genbook.blank() && genbook.current_status() != Logbook::generating )
    {
    show_error( "Logfile alredy exists and is non-empty.", 0, true );
    return 1;
    }
  if( genbook.read_only() ) return not_writable( logname );

  const int odes = open( oname, O_RDONLY | O_BINARY );
  if( odes < 0 )
    { show_error( "Can't open output file", errno ); return 1; }
  if( lseek( odes, 0, SEEK_SET ) )
    { show_error( "Output file is not seekable." ); return 1; }

  if( verbosity >= 0 )
    std::printf( "%s %s\n", Program_name, PROGVERSION );
  if( verbosity >= 1 )
    {
    std::printf( "About to generate an approximate logfile for %s and %s\n",
                 iname, oname );
    std::printf( "    Starting positions: infile = %sB,  outfile = %sB\n",
                 format_num( genbook.domain().pos() ),
                 format_num( genbook.domain().pos() + genbook.offset() ) );
    std::printf( "    Copy block size: %3d sectors\n", cluster );
    std::printf( "Sector size: %sBytes\n\n", format_num( hardbs, 99999 ) );
    }
  return genbook.do_generate( odes );
  }


const char * device_id_or_size( const int fd )
  {
  static char buf[32];
  const char * p = device_id( fd );
  if( !p )
    {
    const long long size = lseek( fd, 0, SEEK_END );
    snprintf( buf, sizeof buf, "%lld", size );
    p = buf;
    }
  return p;
  }

const char * device_id_or_size( const char * const name )
  {
  const int fd = open( name, O_RDONLY );
  const char * const p = ( fd >= 0 ) ? device_id_or_size( fd ) : "";
  if( fd >= 0 ) close( fd );
  return p;
  }


void about_to_copy( const Rescuebook & rescuebook, const char * const iname,
                    const char * const oname, const int ides, const bool ask )
  {
  if( ask || verbosity >= 0 )
    std::printf( "%s %s\n", Program_name, PROGVERSION );
  if( ask || verbosity >= 1 )
    {
    std::string iid, oid;
    if( ask || verbosity >= 2 )
      {
      iid = " ["; iid += device_id_or_size( ides ); iid += ']';
      oid = " ["; oid += device_id_or_size( oname ); oid += ']';
      }
    std::printf( "About to copy %sBytes from %s%s to %s%s.\n",
                 rescuebook.domain().full() ? "an unknown number of " :
                   format_num( rescuebook.domain().in_size() ),
                 iname, iid.c_str(), oname, oid.c_str() );
    }
  }


bool user_agrees_ids( const Rescuebook & rescuebook, const char * const iname,
                      const char * const oname, const int ides )
  {
  about_to_copy( rescuebook, iname, oname, ides, true );
  std::printf( "Proceed (y/N)? " );
  std::fflush( stdout );
  return ( std::tolower( std::fgetc( stdin ) ) == 'y' );
  }


int do_rescue( const long long offset, Domain & domain,
               const Domain * const test_domain, const Rb_options & rb_opts,
               const char * const iname, const char * const oname,
               const char * const logname, const int cluster,
               const int hardbs, const int o_trunc,
               const bool ask, const bool preallocate,
               const bool synchronous, const bool verify_input_size )
  {
  const int ides = open( iname, O_RDONLY | rb_opts.o_direct | O_BINARY );
  if( ides < 0 )
    { show_error( "Can't open input file", errno ); return 1; }
  long long isize = lseek( ides, 0, SEEK_END );
  if( isize < 0 )
    { show_error( "Input file is not seekable." ); return 1; }
  if( test_domain )
    { const long long size = test_domain->end();
      if( isize <= 0 || isize > size ) isize = size; }

  Rescuebook rescuebook( offset, isize, domain, test_domain, rb_opts, iname,
                         logname, cluster, hardbs, synchronous );

  if( verify_input_size )
    {
    if( !rescuebook.logfile_exists() || isize <= 0 ||
        rescuebook.logfile_isize() <= 0 ||
        rescuebook.logfile_isize() >= LLONG_MAX )
      {
      show_error( "Can't verify input file size.\n"
                  "          Logfile is unfinished or missing or size is invalid." );
      return 1;
      }
    if( rescuebook.logfile_isize() != isize )
      {
      show_error( "Input file size differs from size calculated from logfile." );
      return 1;
      }
    }
  if( rescuebook.domain().empty() )
    {
    if( rescuebook.complete_only && !rescuebook.logfile_exists() )
      { show_error( "Nothing to complete; logfile is missing or empty.", 0, true );
        return 1; }
    return empty_domain();
    }
  if( o_trunc && !rescuebook.blank() )
    {
    show_error( "Outfile truncation and logfile input are incompatible.", 0, true );
    return 1;
    }
  if( rescuebook.read_only() ) return not_writable( logname );

  if( ask && !user_agrees_ids( rescuebook, iname, oname, ides ) ) return 1;

  const int odes = open( oname, O_CREAT | O_WRONLY | o_trunc | O_BINARY,
                         outmode );
  if( odes < 0 )
    { show_error( "Can't open output file", errno ); return 1; }
  if( lseek( odes, 0, SEEK_SET ) )
    { show_error( "Output file is not seekable." ); return 1; }
  if( preallocate )
    {
#if defined _POSIX_ADVISORY_INFO && _POSIX_ADVISORY_INFO > 0
    if( posix_fallocate( odes, rescuebook.domain().pos() + rescuebook.offset(),
                         rescuebook.domain().size() ) != 0 )
      { show_error( "Can't preallocate output file", errno ); return 1; }
#else
    show_error( "warning: Preallocation not available." );
#endif
    }
  if( !rate_logger.open_file() )
    { show_error( "Can't open file for logging rates", errno ); return 1; }
  if( !read_logger.open_file() )
    { show_error( "Can't open file for logging reads", errno ); return 1; }

  if( !ask ) about_to_copy( rescuebook, iname, oname, ides, false );
  if( verbosity >= 1 )
    {
    std::printf( "    Starting positions: infile = %sB,  outfile = %sB\n",
                 format_num( rescuebook.domain().pos() ),
                 format_num( rescuebook.domain().pos() + rescuebook.offset() ) );
    std::printf( "    Copy block size: %3d sectors", cluster );
    if( rescuebook.skipbs > 0 )
      std::printf( "       Initial skip size: %d sectors\n",
                   rescuebook.skipbs / hardbs );
    else
      std::printf( "       Skipping disabled\n" );
    std::printf( "Sector size: %sBytes\n", format_num( hardbs, 99999 ) );
    if( verbosity >= 2 )
      {
      bool nl = false;
      if( rescuebook.max_error_rate >= 0 )
        { nl = true; std::printf( "Max error rate: %6sB/s    ",
                                  format_num( rescuebook.max_error_rate, 99999 ) ); }
      if( rescuebook.max_errors >= 0 )
        {
        nl = true;
        std::printf( "Max %serrors: %d    ", rescuebook.new_errors_only ?
                     "new " : "", rescuebook.max_errors );
        }
      if( rescuebook.max_retries >= 0 )
        { nl = true;
          std::printf( "Max retry passes: %d    ", rescuebook.max_retries ); }
      if( nl ) { nl = false; std::fputc( '\n', stdout ); }

      if( rescuebook.min_read_rate == 0 )
        { nl = true; std::printf( "Min read rate: auto    " ); }
      else if( rescuebook.min_read_rate > 0 )
        { nl = true; std::printf( "Min read rate:  %6sB/s    ",
                                  format_num( rescuebook.min_read_rate, 99999 ) ); }
      if( rescuebook.pause > 0 )
        { nl = true; std::printf( "Pause: %s    ",
                                  format_time( rescuebook.pause ) ); }
      if( rescuebook.timeout >= 0 )
        { nl = true; std::printf( "Timeout: %s",
                                  format_time( rescuebook.timeout ) ); }
      if( nl ) { nl = false; std::fputc( '\n', stdout ); }

      std::printf( "Direct: %s    ", rescuebook.o_direct ? "yes" : "no" );
      std::printf( "Sparse: %s    ", rescuebook.sparse ? "yes" : "no" );
      std::printf( "Trim: %s    ", !rescuebook.notrim ? "yes" : "no" );
      std::printf( "Scrape: %s    ", !rescuebook.noscrape ? "yes" : "no" );
      std::printf( "Truncate: %s    ", o_trunc ? "yes" : "no" );
      std::fputc( '\n', stdout );
      if( rescuebook.complete_only )
        { nl = true; std::printf( "Complete only    " ); }
      if( rescuebook.reverse ) { nl = true; std::printf( "Reverse mode" ); }
      if( nl ) { nl = false; std::fputc( '\n', stdout ); }
      }
    std::fputc( '\n', stdout );
    }
  return rescuebook.do_rescue( ides, odes );
  }

} // end namespace


#include "main_common.cc"


namespace {

void parse_cpass( const std::string & arg, Rb_options & rb_opts )
  {
  bool error = arg.empty();
  rb_opts.cpass_bitset = 0;
  for( unsigned i = 0; i < arg.size(); ++i )
    {
    const char ch = arg[i];
    if( ch < '0' || ch > '3' ) { error = true; break; }
    if( ch > '0' ) rb_opts.cpass_bitset |= ( 1 << ( ch - '1' ) );
    if( i + 1 < arg.size() )
      {
      if( arg[i+1] == ',' && i + 2 < arg.size() ) ++i;
      else { error = true; break; }
      }
    }
  if( error )
    {
    show_error( "Bad list of passes in option '--cpass'." );
    std::exit( 1 );
    }
  }

void parse_skipbs( const char * const arg, Rb_options & rb_opts,
                   const int hardbs )
  {
  const char * const arg2 = std::strchr( arg, ',' );

  if( !arg2 || arg2 != arg )
    rb_opts.skipbs = getnum( arg, hardbs, 0, Rb_options::max_max_skipbs, true );
  if( arg2 )
    rb_opts.max_skipbs = getnum( arg2 + 1, hardbs, Rb_options::default_skipbs,
                                 Rb_options::max_max_skipbs );
  if( rb_opts.skipbs > 0 && rb_opts.skipbs < Rb_options::default_skipbs )
    {
    show_error( "Minimum initial skip size is 64KiB." );
    std::exit( 1 );
    }
  if( rb_opts.skipbs > rb_opts.max_skipbs )
    {
    show_error( "'initial skip size' is larger than 'max skip size'." );
    std::exit( 1 );
    }
  }

} // end namespace


bool Rescuebook::reopen_infile()
  {
  if( ides_ >= 0 ) close( ides_ );
  ides_ = open( iname_, O_RDONLY | o_direct | O_BINARY );
  if( ides_ < 0 )
    { final_msg( "Can't reopen input file", errno ); return false; }
  const long long isize = lseek( ides_, 0, SEEK_END );
  if( isize < 0 )
    { final_msg( "Input file has become not seekable", errno ); return false; }
  return true;
  }


int main( const int argc, const char * const argv[] )
  {
  enum Optcode { opt_ask = 256, opt_cpa, opt_pau, opt_scsi = 512, opt_ata = 513, opt_markerror = 514 };
  long long ipos = 0;
  long long opos = -1;
  long long max_size = -1;
  const char * domain_logfile_name = 0;
  const char * test_mode_logfile_name = 0;
  const int cluster_bytes = 65536;
  const int default_hardbs = 512;
  const int max_hardbs = Rb_options::max_max_skipbs;
  int cluster = 0;
  int hardbs = default_hardbs;
  sector_size = hardbs;
  int o_trunc = 0;
  Mode program_mode = m_none;
  Rb_options rb_opts;
  bool ask = false;
  bool force = false;
  bool ignore_write_errors = false;
  bool loose = false;
  bool preallocate = false;
  bool synchronous = false;
  bool verify_input_size = false;
  std::string filltypes;
  invocation_name = argv[0];
  command_line = argv[0];
  for( int i = 1; i < argc; ++i )
    { command_line += ' '; command_line += argv[i]; }

  const Arg_parser::Option options[] =
    {
    { '1', "log-rates",           Arg_parser::yes },
    { '2', "log-reads",           Arg_parser::yes },
    { 'a', "min-read-rate",       Arg_parser::yes },
    { 'A', "try-again",           Arg_parser::no  },
    { 'b', "sector-size",         Arg_parser::yes },
    { 'B', "binary-prefixes",     Arg_parser::no  },
    { 'c', "cluster-size",        Arg_parser::yes },
    { 'C', "complete-only",       Arg_parser::no  },
    { 'd', "direct",              Arg_parser::no  },
    { 'D', "synchronous",         Arg_parser::no  },
    { 'e', "max-errors",          Arg_parser::yes },
    { 'E', "max-error-rate",      Arg_parser::yes },
    { 'f', "force",               Arg_parser::no  },
    { 'F', "fill-mode",           Arg_parser::yes },
    { 'G', "generate-mode",       Arg_parser::no  },
    { 'h', "help",                Arg_parser::no  },
    { 'H', "test-mode",           Arg_parser::yes },
    { 'i', "input-position",      Arg_parser::yes },
    { 'I', "verify-input-size",   Arg_parser::no  },
    { 'K', "skip-size",           Arg_parser::yes },
    { 'L', "loose-domain",        Arg_parser::no  },
    { 'm', "domain-logfile",      Arg_parser::yes },
    { 'M', "retrim",              Arg_parser::no  },
    { 'n', "no-scrape",           Arg_parser::no  },
    { 'n', "no-split",            Arg_parser::no  },
    { 'N', "no-trim",             Arg_parser::no  },
    { 'o', "output-position",     Arg_parser::yes },
    { 'O', "reopen-on-error",     Arg_parser::no  },
    { 'p', "preallocate",         Arg_parser::no  },
    { 'P', "data-preview",        Arg_parser::maybe },
    { 'q', "quiet",               Arg_parser::no  },
    { 'r', "retry-passes",        Arg_parser::yes },
    { 'R', "reverse",             Arg_parser::no  },
    { 's', "size",                Arg_parser::yes },
    { 'S', "sparse",              Arg_parser::no  },
    { 't', "truncate",            Arg_parser::no  },
    { 'T', "timeout",             Arg_parser::yes },
    { 'u', "unidirectional",      Arg_parser::no  },
    { 'v', "verbose",             Arg_parser::no  },
    { 'V', "version",             Arg_parser::no  },
    { 'w', "ignore-write-errors", Arg_parser::no  },
    { 'x', "extend-outfile",      Arg_parser::yes },
    { 'X', "exit-on-error",       Arg_parser::no  },
    { opt_ask, "ask",             Arg_parser::no  },
    { opt_cpa, "cpass",           Arg_parser::yes },
    { opt_pau, "pause",           Arg_parser::yes },
    { opt_scsi, "scsi-passthrough",    Arg_parser::no  },
    { opt_ata, "ata-passthrough",     Arg_parser::no  },
    { opt_markerror, "mark-abnormal-error",     Arg_parser::no  },
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
      case '1': rate_logger.set_filename( arg ); break;
      case '2': read_logger.set_filename( arg ); break;
      case 'a': rb_opts.min_read_rate = getnum( arg, hardbs, 0 ); break;
      case 'A': rb_opts.try_again = true; break;
      case 'b': hardbs = getnum( arg, 0, 1, max_hardbs ); break;
      case 'B': format_num( 0, 0, -1 ); break;		// set binary prefixes
      case 'c': cluster = getnum( arg, 0, 1, INT_MAX ); break;
      case 'C': rb_opts.complete_only = true; break;
      case 'd': rb_opts.o_direct = O_DIRECT;
                if( rb_opts.o_direct == 0 )
                  { show_error( "Direct disc access not available." ); return 1; }
                break;
      case 'D': synchronous = true; break;
      case 'e': rb_opts.new_errors_only = ( *arg == '+' );
                rb_opts.max_errors = getnum( arg, 0, 0, INT_MAX ); break;
      case 'E': rb_opts.max_error_rate = getnum( arg, hardbs, 0 ); break;
      case 'f': force = true; break;
      case 'F': set_mode( program_mode, m_fill ); filltypes = arg;
                check_types( filltypes, "fill-mode" ); break;
      case 'G': set_mode( program_mode, m_generate ); break;
      case 'h': show_help( cluster_bytes / default_hardbs, default_hardbs,
                           Rb_options::default_skipbs );
                return 0;
      case 'H': set_name( &test_mode_logfile_name, arg, code ); break;
      case 'i': ipos = getnum( arg, hardbs, 0 ); break;
      case 'I': verify_input_size = true; break;
      case 'K': parse_skipbs( arg, rb_opts, hardbs ); break;
      case 'L': loose = true; break;
      case 'm': set_name( &domain_logfile_name, arg, code ); break;
      case 'M': rb_opts.retrim = true; break;
      case 'n': rb_opts.noscrape = true; break;
      case 'N': rb_opts.notrim = true; break;
      case 'o': opos = getnum( arg, hardbs, 0 ); break;
      case 'O': rb_opts.reopen_on_error = true; break;
      case 'p': preallocate = true; break;
      case 'P': rb_opts.preview_lines = arg[0] ? getnum( arg, 0, 1, 32 ) : 3;
                break;
      case 'q': verbosity = -1; break;
      case 'r': rb_opts.max_retries = getnum( arg, 0, -1, INT_MAX ); break;
      case 'R': rb_opts.reverse = true; break;
      case 's': max_size = getnum( arg, hardbs, -1 ); break;
      case 'S': rb_opts.sparse = true; break;
      case 't': o_trunc = O_TRUNC; break;
      case 'T': rb_opts.timeout = parse_time_interval( arg ); break;
      case 'u': rb_opts.unidirectional = true; break;
      case 'v': if( verbosity < 7 ) ++verbosity; break;
      case 'V': show_version(); return 0;
      case 'w': ignore_write_errors = true; break;
      case 'x': rb_opts.min_outfile_size = getnum( arg, hardbs, 1 ); break;
      case 'X': rb_opts.exit_on_error = true; break;
      case opt_ask: ask = true; break;
      case opt_cpa: parse_cpass( parser.argument( argind ), rb_opts ); break;
      case opt_pau: rb_opts.pause = parse_time_interval( arg ); break;
      case opt_scsi: option_scsi_passthrough(); break;
      case opt_ata: option_ata_passthrough(); break;
      case opt_markerror: option_mark_abnormal_error(); break;
      default : internal_error( "uncaught option." );
      }
    } // end process options

  if( opos < 0 ) opos = ipos;
  if( hardbs < 1 ) hardbs = default_hardbs;
  if( cluster >= INT_MAX / hardbs ) cluster = ( INT_MAX / hardbs ) - 1;
  if( cluster < 1 ) cluster = cluster_bytes / hardbs;
  if( cluster < 1 ) cluster = 1;

  const char *iname = 0, *oname = 0, *logname = 0;
  if( argind < parser.arguments() ) iname = parser.argument( argind++ ).c_str();
  if( argind < parser.arguments() ) oname = parser.argument( argind++ ).c_str();
  if( argind < parser.arguments() ) logname = parser.argument( argind++ ).c_str();
  if( argind < parser.arguments() )
    { show_error( "Too many files.", 0, true ); return 1; }

  // end scan arguments

  if( !check_files( iname, oname, logname, rb_opts.min_outfile_size, force,
                    program_mode == m_generate, preallocate, rb_opts.sparse ) )
    return 1;

  Domain domain( ipos, max_size, domain_logfile_name, loose );

  if ( sgpt )
  {
    int return_value = check_device( iname, verbosity, cluster );
    if( return_value != 0 )
    {
      return 1;
    }
  }

  switch( program_mode )
    {
    case m_fill:
      if( ask )
        { show_error( "Option '--ask' is incompatible with fill mode.", 0, true );
          return 1; }
      if( rb_opts != Rb_options() || test_mode_logfile_name ||
          verify_input_size || preallocate || o_trunc )
        show_error( "warning: Options -aACdeEHIKlMnOpPrRStTuxX are ignored in fill mode." );
      return do_fill( opos - ipos, domain, iname, oname, logname, cluster,
                      hardbs, filltypes, ignore_write_errors, synchronous );
    case m_generate:
      if( ask )
        { show_error( "Option '--ask' is incompatible with generate mode.", 0, true );
          return 1; }
      if( rb_opts != Rb_options() || synchronous || test_mode_logfile_name ||
          verify_input_size || preallocate || o_trunc || ignore_write_errors )
        show_error( "warning: Options -aACdDeEHIKlMnOpPrRStTuwxX are ignored in generate mode." );
      return do_generate( opos - ipos, domain, iname, oname, logname,
                          cluster, hardbs );
    case m_none:
      {
      if( ignore_write_errors )
        { show_error( "Option '-w' is incompatible with rescue mode.", 0, true );
          return 1; }
      const Domain * const test_domain = test_mode_logfile_name ?
        new Domain( 0, -1, test_mode_logfile_name, loose ) : 0;
      int tmp = do_rescue( opos - ipos, domain, test_domain, rb_opts, iname,
                           oname, logname, cluster, hardbs, o_trunc, ask,
                           preallocate, synchronous, verify_input_size );
      if( test_domain ) delete test_domain;
      return tmp;
      }
    }
  }
