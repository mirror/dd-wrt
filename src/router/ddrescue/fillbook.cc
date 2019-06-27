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
#include <climits>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <stdint.h>

#include "block.h"
#include "ddrescue.h"


// Return values: 1 write error, 0 OK, -1 interrupted, -2 logfile error.
//
int Fillbook::fill_areas( const std::string & filltypes )
  {
  const char * const msg = "Filling blocks...";
  bool first_post = true;

  for( int index = 0; index < sblocks(); ++index )
    {
    const Sblock & sb = sblock( index );
    if( !domain().includes( sb ) ) { if( domain() < sb ) break; else continue; }
    if( sb.end() <= current_pos() ||
        filltypes.find( sb.status() ) >= filltypes.size() ) continue;
    Block b( sb.pos(), softbs() );	// fill the area a softbs at a time
    if( sb.includes( current_pos() ) ) b.pos( current_pos() );
    if( b.end() > sb.end() ) b.crop( sb );
    current_status( filling, msg );
    while( b.size() > 0 )
      {
      current_pos( b.pos() );
      if( verbosity >= 0 )
        { show_status( b.pos(), msg, first_post ); first_post = false; }
      if( interrupted() ) return -1;
      const int retval = fill_block( b );
      if( retval )					// write error
        {
        if( !ignore_write_errors_ ) return retval;
        if( b.size() > hardbs() )	// fill the area a hardbs at a time
          { b.size( hardbs() ); continue; }
        }
      if( !update_logfile( odes_ ) ) return -2;
      b.pos( b.end() );
      if( b.end() > sb.end() ) b.crop( sb );
      }
    ++filled_areas; --remaining_areas;
    }
  return 0;
  }


void Fillbook::show_status( const long long ipos, const char * const msg,
                            bool force )
  {
  const char * const up = "\x1b[A";
  if( t0 == 0 )
    {
    t0 = t1 = initial_time();
    first_size = last_size = filled_size;
    force = true;
    std::printf( "\n\n\n" );
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
      a_rate = ( filled_size - first_size ) / ( t2 - t0 );
      c_rate = ( filled_size - last_size ) / ( t2 - t1 );
      t1 = t2;
      last_size = filled_size;
      }
    std::printf( "\r%s%s%s", up, up, up );
    std::printf( "filled size: %10sB,  filled areas: %6u,  current rate: %9sB/s\n",
                 format_num( filled_size ), filled_areas,
                 format_num( c_rate, 99999 ) );
    std::printf( "remain size: %10sB,  remain areas: %6u,  average rate: %9sB/s\n",
                 format_num( remaining_size ), remaining_areas,
                 format_num( a_rate, 99999 ) );
    std::printf( "current pos: %10sB,  run time:  %9s\n",
                 format_num( last_ipos + offset() ), format_time( t1 - t0 ) );
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
int Fillbook::do_fill( const int odes, const std::string & filltypes )
  {
  filled_size = 0, remaining_size = 0;
  filled_areas = 0, remaining_areas = 0;
  odes_ = odes;
  if( current_status() != filling || !domain().includes( current_pos() ) )
    current_pos( 0 );

  for( int i = 0; i < sblocks(); ++i )
    {
    const Sblock & sb = sblock( i );
    if( !domain().includes( sb ) ) { if( domain() < sb ) break; else continue; }
    if( filltypes.find( sb.status() ) >= filltypes.size() ) continue;
    if( sb.end() <= current_pos() ) { ++filled_areas; filled_size += sb.size(); }
    else if( sb.includes( current_pos() ) )
      {
      filled_size += current_pos() - sb.pos();
      ++remaining_areas; remaining_size += sb.end() - current_pos();
      }
    else { ++remaining_areas; remaining_size += sb.size(); }
    }
  set_signals();
  if( verbosity >= 0 )
    {
    std::printf( "Press Ctrl-C to interrupt\n" );
    if( logfile_exists() )
      {
      std::printf( "Initial status (read from logfile)\n" );
      std::printf( "filled size:    %10sB,  filled areas:    %7u\n",
                   format_num( filled_size ), filled_areas );
      std::printf( "remaining size: %10sB,  remaining areas: %7u\n",
                   format_num( remaining_size ), remaining_areas );
      std::printf( "Current status\n" );
      }
    }
  int retval = fill_areas( filltypes );
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
    if( !update_logfile( odes_, true ) && retval == 0 ) retval = 1;
    }
  if( final_msg() ) show_error( final_msg(), final_errno() );
  if( retval ) return retval;		// errors have priority over signals
  if( signaled ) return signaled_exit();
  return 0;
  }
