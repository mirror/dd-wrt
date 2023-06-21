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
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

#include "rational.h"
#include "block.h"
#include "mapbook.h"
#include "rescuebook.h"


int Rescuebook::copy_command( const char * const command )
  {
  long long pos, size;
  const int n = std::sscanf( command, "%lli %lli", &pos, &size );
  if( n != 2 || pos < 0 || size <= 0 ) return 1;
  const long long end = Block( pos, size ).end();
  while( pos < end )
    {
    Block b( pos, softbs() );
    find_chunk( b, Sblock::non_tried, domain(), softbs(), false, true );
    if( b.size() <= 0 || b.pos() >= end ) break;
    if( b.end() > end ) b.size( end - b.pos() );
    pos = b.end();
    int copied_size = 0, error_size = 0;
    int retval = copy_block( b, copied_size, error_size );
    if( retval ) return retval;
    if( copied_size + error_size < b.size() )			// EOF
      {
      if( complete_only ) truncate_domain( b.pos() + copied_size + error_size );
      else if( !truncate_vector( b.pos() + copied_size + error_size ) )
        { final_msg( iname_, "EOF found below the size calculated from mapfile." );
          retval = 1; }
      }
    if( copied_size > 0 )
      change_chunk_status( Block( b.pos(), copied_size ), Sblock::finished );
    if( error_size > 0 )
      {
      change_chunk_status( Block( b.pos() + copied_size, error_size ),
                           Sblock::bad_sector );
      struct stat istat;
      if( stat( iname_, &istat ) != 0 )
        { final_msg( iname_, "Input file disappeared", errno ); retval = 1; }
      }
    if( retval ) return retval;
    }
  return 0;
  }


int Rescuebook::status_command( const char * const command ) const
  {
  long long pos, size;
  const int n = std::sscanf( command, "%lli %lli", &pos, &size );
  if( n != 2 || pos < 0 || size <= 0 ) return 1;
  const Block b( pos, size );
  const long index = find_index( pos );
  if( index < 0 ) return 1;
  for( long i = index; i < sblocks(); ++i )
    {
    const Sblock & sb = sblock( i );
    if( sb.pos() >= b.end() ) break;
    if( !domain().includes( sb ) && domain() < sb ) break;
    Block c( sb ); c.crop( b );
    std::printf( "0x%08llX  0x%08llX  %c\n", c.pos(), c.size(), sb.status() );
    }
  return 0;
  }


// Return values: 1 write error/unknown command, 0 OK.
//
int Rescuebook::do_commands( const int ides, const int odes )
  {
  ides_ = ides; odes_ = odes;

//  set_signals();				// ignore signals
  initial_time();
  int retval = 0;
  while( true )
    {
    std::string command;
    while( true )
      {
      const int c = std::fgetc( stdin );
      if( c == '\n' ) { if( command.size() ) break; else continue; }
      if( c == EOF ) { command = "f"; break; }	// discard partial command
      if( !std::isspace( c ) ) command += c;
      else if( command.size() && !std::isspace( command[command.size()-1] ) )
        command += ' ';
      }
    if( command == "q" ) break;
    int tmp = 0;		// -1 finish, 0 OK, 1 error, 2 fatal error
    const bool finish = ( command == "f" );
    if( finish || command == "u" )
      {
      if( finish ) compact_sblock_vector();
      if( !update_mapfile( odes_, true ) )
        { if( finish ) { emergency_save(); tmp = 2; } else tmp = 1; }
      else if( finish ) tmp = -1;
      }
    else if( command.size() > 1 && command[0] == 'c' )
      tmp = copy_command( command.c_str() + 1 );
    else if( command.size() > 1 && command[0] == 's' )
      tmp = status_command( command.c_str() + 1 );
    else { std::printf( "error: unknown command '%s'\n", command.c_str() );
           retval = 1; if( safe_fflush( stdout ) ) continue; else tmp = 2; }
    if( tmp <= 0 ) std::fputs( "done\n", stdout );
    else
      {
      if( final_msg().size() )
        { printf( "error: %s%s%s\n", final_msg().c_str(),
                  ( final_errno() > 0 ) ? ": " : "",
                  ( final_errno() > 0 ) ? std::strerror( final_errno() ) : "" );
         final_msg( "" ); }
      else std::fputs( "error\n", stdout );
      }
    if( !safe_fflush( stdout ) ) tmp = 2;
    if( tmp ) { if( tmp > 0 ) retval = 1; if( tmp != 1 ) break; }
    }
  if( close( odes_ ) != 0 )
    { show_file_error( oname_, "Error closing outfile", errno );
      if( retval == 0 ) retval = 1; }
  return retval;
  }
