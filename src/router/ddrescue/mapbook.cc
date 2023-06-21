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
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>

#include "block.h"
#include "mapbook.h"


namespace {

void input_pos_error( const long long pos, const long long insize )
  {
  char buf[128];
  snprintf( buf, sizeof buf,
            "Can't start reading at pos %s.\n"
            "          Input file is only %s bytes long.",
            format_num3( pos ), format_num3( insize ) );
  show_error( buf );
  }

} // end namespace


bool safe_fflush( FILE * const f )
  {
  while( true )
    {
    const int ret = std::fflush( f );
    if( ret == 0 ) return true;
    if( ret == EOF && errno == EINTR ) continue;
    return false;
    }
  }


bool Mapbook::save_mapfile( const char * const name )
  {
  std::remove( name );
  FILE * const f = std::fopen( name, "w" );
  if( f && write_mapfile( f, true, true ) && std::fclose( f ) == 0 )
    {
    char buf[80];
    snprintf( buf, sizeof buf, "Mapfile saved in '%s'", name );
    final_msg( buf );
    return true;
    }
  return false;
  }


bool Mapbook::emergency_save()
  {
  static bool first_time = true;
  static std::string home_name;
  const std::string dead_name( "ddrescue.map" );

  if( filename() != dead_name && save_mapfile( dead_name.c_str() ) )
    return true;
  if( first_time )
    {
    first_time = false;
    const char * const p = std::getenv( "HOME" );
    if( p ) { home_name = p; home_name += '/'; home_name += dead_name; }
    }
  if( home_name.size() &&
      filename() != home_name && save_mapfile( home_name.c_str() ) )
    return true;
  show_file_error( dead_name.c_str(), "Emergency save failed." );
  return false;
  }


Mapbook::Mapbook( const long long offset, const long long insize,
                  Domain & dom, const Mb_options & mb_opts,
                  const char * const mapname, const int cluster,
                  const int hardbs, const bool complete_only,
                  const bool rescue )
  : Mapfile( mapname ), Mb_options( mb_opts ), offset_( offset ),
    mapfile_insize_( 0 ), domain_( dom ), hardbs_( hardbs ),
    softbs_( cluster * hardbs_ ),
    iobuf_size_( softbs_ + hardbs_ ),	// +hardbs for direct unaligned reads
    final_errno_( 0 ), um_t1( 0 ), um_t1s( 0 ), um_prev_mf_sync( false ),
    mapfile_exists_( false )
  {
  long alignment = sysconf( _SC_PAGESIZE );
  if( alignment < hardbs_ || alignment % hardbs_ ) alignment = hardbs_;
  if( alignment < 2 ) alignment = 0;
  iobuf_ = iobuf_base = new uint8_t[ alignment + ( 2 * iobuf_size_ ) ];
  if( alignment > 1 )		// align iobuf for direct disc access
    {
    const int disp =
      alignment - ( reinterpret_cast<unsigned long long> (iobuf_) % alignment );
    if( disp > 0 && disp < alignment ) iobuf_ += disp;
    }

  if( insize > 0 )
    {
    if( domain_.pos() >= insize )
      { input_pos_error( domain_.pos(), insize ); std::exit( 1 ); }
    domain_.crop_by_file_size( insize );
    }
  if( filename() )
    {
    mapfile_exists_ = read_mapfile( 0, false );
    if( mapfile_exists_ ) mapfile_insize_ = extent().end();
    }
  if( !complete_only ) extend_sblock_vector( insize );
  else domain_.crop( extent() );  // limit domain to blocks read from mapfile
  compact_sblock_vector();
  if( rescue ) join_subsectors( hardbs_ );
  split_by_domain_borders( domain_ );
  if( sblocks() == 0 ) domain_.clear();
  }


/* Write periodically the mapfile to disc.
   Return false only if update is attempted and fails.
*/
bool Mapbook::update_mapfile( const int odes, const bool force )
  {
  if( !filename() ) return true;
  const int interval = ( mapfile_save_interval >= 0 ) ? mapfile_save_interval :
    30 + std::min( 270L, sblocks() / 38 );	// auto, 30s to 5m
  const long long t2 = std::time( 0 );
  if( um_t1 == 0 || um_t1 > t2 ) um_t1 = um_t1s = t2;	// initialize
  if( !force && t2 - um_t1 < interval ) return true;
  const bool mf_sync = ( force || t2 - um_t1s >= mapfile_sync_interval );
  if( odes >= 0 ) fsync( odes );
  if( um_prev_mf_sync )
    {
    std::string mapname_bak( filename() ); mapname_bak += ".bak";
    std::remove( mapname_bak.c_str() );		// break possible hard link
    std::rename( filename(), mapname_bak.c_str() );
    }
  um_prev_mf_sync = mf_sync;

  for( bool first_post = true; ; first_post = false )
    {
    errno = 0;
    if( write_mapfile( 0, true, mf_sync ) )
      // update times here to exclude writing time from intervals
      { um_t1 = std::time( 0 ); if( mf_sync ) um_t1s = um_t1; return true; }
    if( verbosity < 0 ) return false;
    const int saved_errno = errno;
    if( first_post ) std::fputc( '\n', stderr );
    show_file_error( filename(), "Error writing mapfile", saved_errno );
    std::fputs( "Fix the problem and press ENTER to retry,\n"
                "                     or E+ENTER for an emergency save and exit,\n"
                "                     or Q+ENTER to abort.\n", stderr );
    safe_fflush( stderr );
    while( true )
      {
      tcflush( STDIN_FILENO, TCIFLUSH );
      const int c = std::tolower( std::fgetc( stdin ) );
      int tmp = c;
      while( tmp != '\n' && tmp != EOF ) tmp = std::fgetc( stdin );
      if( c == '\r' || c == '\n' || c == 'e' || c == 'q' )
        {
        if( c == 'q' || ( c == 'e' && emergency_save() ) )
          { if( !force ) std::fputs( "\n\n\n\n\n", stdout ); return false; }
        break;
        }
      }
    }
  }
