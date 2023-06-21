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


// Return values: 1 write error, 0 OK.
//
int Fillbook::fill_block( const Sblock & sb )
  {
  if( sb.size() <= 0 || sb.size() > softbs() )
    internal_error( "bad size filling a Block." );
  const int size = sb.size();

  if( write_location_data )	// write location data into each sector
    for( long long pos = sb.pos(); pos < sb.end(); pos += hardbs() )
      {
      char * const buf = (char *)iobuf() + ( pos - sb.pos() );
      const int bufsize = std::min( 80LL, sb.end() - pos );
      const int len = snprintf( buf, bufsize,
                        "\n# position 0x%08llX sector 0x%08llX status %c",
                        pos, pos / hardbs(), sb.status() );
      if( len > 0 && len < bufsize ) buf[len] = ' ';
      }
  if( writeblockp( odes_, iobuf(), size, sb.pos() + offset() ) != size ||
      ( synchronous_ && fsync( odes_ ) != 0 && errno != EINVAL ) )
    {
    if( !ignore_write_errors ) final_msg( oname_, "Write error", errno );
    return 1;
    }
  filled_size += size; remaining_size -= size;
  return 0;
  }


// Return values: 1 write error, 0 OK, -1 interrupted, -2 mapfile error.
//
int Fillbook::fill_areas()
  {
  const char * const msg = "Filling blocks...";
  bool first_post = true;

  for( long index = 0; index < sblocks(); ++index )
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
      const int retval = fill_block( Sblock( b, sb.status() ) );
      if( retval )					// write error
        {
        if( !ignore_write_errors ) return retval;
        if( b.size() > hardbs() )	// fill the area a hardbs at a time
          { b.size( hardbs() ); continue; }
        }
      if( !update_mapfile( odes_ ) ) return -2;
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
  const char * const up = "\x1B[A";
  if( t0 == 0 )
    {
    t0 = t1 = initial_time();
    first_size = last_size = filled_size;
    force = true;
    std::fputs( "\n\n\n", stdout );
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
      a_rate = ( filled_size - first_size ) / ( t2 - t0 );
      c_rate = ( filled_size - last_size ) / ( t2 - t1 );
      t1 = t2;
      last_size = filled_size;
      }
    std::printf( "\r%s%s%s", up, up, up );
    std::printf( "filled size: %9sB,  filled areas: %6lu,  current rate: %8sB/s\n",
                 format_num( filled_size ), filled_areas,
                 format_num( c_rate, 99999 ) );
    std::printf( "remain size: %9sB,  remain areas: %6lu,  average rate: %8sB/s\n",
                 format_num( remaining_size ), remaining_areas,
                 format_num( a_rate, 99999 ) );
    std::printf( "current pos: %9sB,  run time: %11s\n",
                 format_num( last_ipos + offset() ), format_time( t1 - t0 ) );
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
int Fillbook::do_fill( const int odes )
  {
  filled_size = 0, remaining_size = 0;
  filled_areas = 0, remaining_areas = 0;
  odes_ = odes;
  if( current_status() != filling || !domain().includes( current_pos() ) )
    current_pos( 0 );

  for( long i = 0; i < sblocks(); ++i )
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
    std::fputs( "Press Ctrl-C to interrupt\n", stdout );
    if( mapfile_exists() )
      {
      std::fputs( "Initial status (read from mapfile)\n", stdout );
      std::printf( "filled size:    %9sB,  filled areas:    %7lu\n",
                   format_num( filled_size ), filled_areas );
      std::printf( "remaining size: %9sB,  remaining areas: %7lu\n",
                   format_num( remaining_size ), remaining_areas );
      std::fputs( "Current status\n", stdout );
      }
    }
  int retval = fill_areas();
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
    if( !update_mapfile( odes_, true ) && retval == 0 ) retval = 1;
    }
  if( final_msg().size() ) show_error( final_msg().c_str(), final_errno() );
  if( close( odes_ ) != 0 )
    { show_file_error( oname_, "Error closing outfile", errno );
      if( retval == 0 ) retval = 1; }
  if( retval ) return retval;		// errors have priority over signals
  if( signaled ) return signaled_exit();
  return 0;
  }


bool Fillbook::read_buffer( const int ides )
  {
  const int rd = readblock( ides, iobuf(), softbs() );
  if( rd <= 0 || errno != 0 ) return false;
  for( int i = rd; i < softbs(); i *= 2 )
    {
    const int size = std::min( i, softbs() - i );
    std::memcpy( iobuf() + i, iobuf(), size );
    }
  return true;
  }
