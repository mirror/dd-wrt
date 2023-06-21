/* GNU ddrescue - Data recovery tool
   Copyright (C) 2004-2023 Antonio Diaz Diaz.

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
#include "mapbook.h"


const char * format_time( const long long t, const bool low_prec )
  {
  enum { buffers = 8, bufsize = 16 };
  static char buffer[buffers][bufsize];	// circle of static buffers for printf
  static int current = 0;
  if( t < 0 ) return "n/a";
  char * const buf = buffer[current++]; current %= buffers;
  const int s = t % 60;
  const int m = ( t / 60 ) % 60;
  const int h = ( t / 3600 ) % 24;
  const long long d = t / 86400;
  int len = 0;				// max len is 11 chars (10h 10m 10s)

  if( d > 0 ) len = snprintf( buf, bufsize, "%lldd", d );
  if( h > 0 && len >= 0 && len <= 7 )
    len += snprintf( buf + len, bufsize - len, "%s%dh", len ? ( (h < 10) ? "  " : " " ) : "", h );
  if( m > 0 && len >= 0 && len <= 7 )
    len += snprintf( buf + len, bufsize - len, "%s%dm", len ? ( (m < 10) ? "  " : " " ) : "", m );
  if( ( s > 0 && len >= 0 && len <= 7 && !low_prec ) || len == 0 )
    len += snprintf( buf + len, bufsize - len, "%s%ds", len ? ( (s < 10) ? "  " : " " ) : "", s );
  return buf;
  }


// If copied_size + error_size < b.size(), it means EOF has been reached.
//
void Genbook::check_block( const Block & b, int & copied_size, int & error_size )
  {
  if( b.size() <= 0 ) internal_error( "bad size checking a Block." );
  copied_size = readblockp( odes_, iobuf(), b.size(), b.pos() + offset() );
  if( errno ) error_size = b.size() - copied_size;

  for( int pos = 0; pos < copied_size; )
    {
    const int size = std::min( hardbs(), copied_size - pos );
    if( !block_is_zero( iobuf() + pos, size ) )
      {
      change_chunk_status( Block( b.pos() + pos, size ),
                           Sblock::finished, domain() );
      finished_size += size;
      }
    gensize += size;
    pos += size;
    }
  }


// Return values: 1 unexpected EOF, 0 OK, -1 interrupted, -2 mapfile error.
//
int Genbook::check_all()
  {
  const char * const msg = "Generating mapfile...";
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
      { final_msg( iname_, "EOF found below the size calculated from mapfile." );
        return 1; }
    if( !update_mapfile() ) return -2;
    }
  return 0;
  }


void Genbook::show_status( const long long ipos, const char * const msg,
                           bool force )
  {
  const char * const up = "\x1B[A";
  if( t0 == 0 )
    {
    t0 = t1 = initial_time();
    first_size = last_size = gensize;
    force = true;
    std::fputs( "\n\n", stdout );
    }

  if( ipos >= 0 ) last_ipos = ipos;
  const long long t2 = std::time( 0 );
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
    std::printf( "rescued: %9sB,  generated: %9sB,  current rate: %8sB/s\n",
                 format_num( finished_size ), format_num( gensize ),
                 format_num( c_rate, 99999 ) );
    std::printf( "   opos: %9sB,  run time: %11s,  average rate: %8sB/s\n",
                 format_num( last_ipos + offset() ), format_time( t1 - t0 ),
                 format_num( a_rate, 99999 ) );
    if( msg && msg[0] )
      {
      const int len = std::strlen( msg ); std::printf( "\r%s", msg );
      for( int i = len; i < oldlen; ++i ) std::fputc( ' ', stdout );
      oldlen = len;
      }
    safe_fflush( stdout );
    }
  }


// Return values: 1 write error, 0 OK.
//
int Genbook::do_generate( const int odes )
  {
  finished_size = 0; gensize = 0;
  odes_ = odes;

  for( long i = 0; i < sblocks(); ++i )
    {
    const Sblock & sb = sblock( i );
    if( !domain().includes( sb ) )
      { if( domain() < sb ) break; else continue; }
    if( sb.status() == Sblock::finished ) finished_size += sb.size();
    if( sb.status() != Sblock::non_tried ) gensize += sb.size();
    }
  set_signals();
  if( verbosity >= 0 )
    {
    std::fputs( "Press Ctrl-C to interrupt\n", stdout );
    if( mapfile_exists() )
      {
      std::fputs( "Initial status (read from mapfile)\n", stdout );
      std::printf( "rescued: %9sB,  generated: %9sB\n",
                   format_num( finished_size ), format_num( gensize ) );
      std::fputs( "Current status\n", stdout );
      }
    }
  int retval = check_all();
  const bool signaled = ( retval == -1 );
  if( signaled ) retval = 0;
  if( verbosity >= 0 )
    {
    show_status( -1, ( retval || signaled ) ? 0 : "Finished", true );
    if( retval == -2 ) std::fputs( "\nMapfile error", stdout );
    else if( signaled ) std::fputs( "\nInterrupted by user", stdout );
    std::fputc( '\n', stdout );
    safe_fflush( stdout );
    }
  if( retval == -2 ) retval = 1;		// mapfile error
  else
    {
    if( retval == 0 && !signaled ) current_status( finished );
    compact_sblock_vector();
    if( !update_mapfile( -1, true ) && retval == 0 ) retval = 1;
    }
  if( final_msg().size() ) show_error( final_msg().c_str(), final_errno() );
  if( retval ) return retval;		// errors have priority over signals
  if( signaled ) return signaled_exit();
  return 0;
  }
