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

#define _FILE_OFFSET_BITS 64

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <stdint.h>
#include <unistd.h>

#include "block.h"
#include "ddrescue.h"


// Return values: 1 unexpected EOF, 0 OK, -1 interrupted, -2 logfile error.
//
int Genbook::check_all()
  {
  const char * const msg = "Generating logfile...";
  long long pos = ( offset() >= 0 ) ? 0 : -offset();
  if( current_status() == generating && domain().includes( current_pos() ) &&
      ( offset() >= 0 || current_pos() >= -offset() ) )
    pos = current_pos();
  bool first_post = true;

  while( pos >= 0 )
    {
    Block b( pos, softbs() );
    find_chunk( b, Sblock::non_tried, domain(), hardbs() );
    if( b.size() <= 0 ) break;
    pos = b.end();
    current_status( generating, msg );
    current_pos( b.pos() );
    if( verbosity >= 0 )
      { show_status( b.pos(), msg, first_post ); first_post = false; }
    if( interrupted() ) return -1;
    int copied_size = 0, error_size = 0;
    check_block( b, copied_size, error_size );
    if( copied_size + error_size < b.size() &&			// EOF
        !truncate_vector( b.pos() + copied_size + error_size ) )
      { final_msg( "EOF found before end of logfile" ); return 1; }
    if( !update_logfile() ) return -2;
    }
  return 0;
  }


void Genbook::show_status( const long long ipos, const char * const msg,
                           bool force )
  {
  const char * const up = "\x1b[A";
  if( t0 == 0 )
    {
    t0 = t1 = initial_time();
    first_size = last_size = gensize;
    force = true;
    std::printf( "\n\n" );
    }

  if( ipos >= 0 ) last_ipos = ipos;
  const long t2 = std::time( 0 );
  if( t2 < t1 )					// clock jumped back
    {
    t0 -= std::min( t0, t1 - t2 );
    t1 = t2;
    }
  if( t2 > t1 || force )
    {
    if( t2 > t1 )
      {
      a_rate = ( gensize - first_size ) / ( t2 - t0 );
      c_rate = ( gensize - last_size ) / ( t2 - t1 );
      t1 = t2;
      last_size = gensize;
      }
    std::printf( "\r%s%s", up, up );
    std::printf( "rescued: %10sB,  generated:%10sB,  current rate: %9sB/s\n",
                 format_num( recsize ), format_num( gensize ),
                 format_num( c_rate, 99999 ) );
    std::printf( "   opos: %10sB,   run time:  %9s,  average rate: %9sB/s\n",
                 format_num( last_ipos + offset() ), format_time( t1 - t0 ),
                 format_num( a_rate, 99999 ) );
    if( msg && msg[0] )
      {
      const int len = std::strlen( msg ); std::printf( "\r%s", msg );
      for( int i = len; i < oldlen; ++i ) std::fputc( ' ', stdout );
      oldlen = len;
      }
    std::fflush( stdout );
    }
  }


// Return values: 1 write error, 0 OK.
//
int Genbook::do_generate( const int odes )
  {
  recsize = 0; gensize = 0;
  odes_ = odes;

  for( int i = 0; i < sblocks(); ++i )
    {
    const Sblock & sb = sblock( i );
    if( !domain().includes( sb ) )
      { if( domain() < sb ) break; else continue; }
    if( sb.status() == Sblock::finished ) recsize += sb.size();
    if( sb.status() != Sblock::non_tried || i + 1 < sblocks() )
      gensize += sb.size();
    }
  set_signals();
  if( verbosity >= 0 )
    {
    std::printf( "Press Ctrl-C to interrupt\n" );
    if( logfile_exists() )
      {
      std::printf( "Initial status (read from logfile)\n" );
      std::printf( "rescued: %10sB,  generated:%10sB\n",
                   format_num( recsize ), format_num( gensize ) );
      std::printf( "Current status\n" );
      }
    }
  int retval = check_all();
  const bool signaled = ( retval == -1 );
  if( signaled ) retval = 0;
  if( verbosity >= 0 )
    {
    show_status( -1, ( retval || signaled ) ? 0 : "Finished", true );
    if( retval == -2 ) std::printf( "\nLogfile error" );
    else if( signaled ) std::printf( "\nInterrupted by user" );
    std::fputc( '\n', stdout );
    }
  if( retval == -2 ) retval = 1;		// logfile error
  else
    {
    if( retval == 0 && !signaled ) current_status( finished );
    compact_sblock_vector();
    if( !update_logfile( -1, true ) && retval == 0 ) retval = 1;
    }
  if( final_msg() ) show_error( final_msg(), final_errno() );
  if( retval ) return retval;		// errors have priority over signals
  if( signaled ) return signaled_exit();
  return 0;
  }
