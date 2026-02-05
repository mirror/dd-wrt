/* GNU ddrescuelog - Tool for ddrescue mapfiles
   Copyright (C) 2011-2026 Antonio Diaz Diaz.

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
This option is a work in progress. The geometries implemented so far are
some of those described in the following article:

H. Wong, "Discovering Hard Disk Physical Geometry through Microbenchmarking",
Sept., 2019. Available online at
<http://blog.stuffedcow.net/2019/09/hard-disk-geometry-microbenchmarking/>
*/

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "mapfile.h"
#include "ddrescuelog.h"


namespace {

struct CHS
  {
  enum Track_order{ F, FF };
  int cylinders;
  int heads;
  int first_track_sectors;
  int last_track_sectors;
  int tracks_zone;
  Track_order track_order;
  int bad_head;
  };

CHS * chsp = 0;

} // end namespace


// cylinders:heads:sectors1[-sectors2[,tracks_zone]]:track_order:bad_head
void parse_chs( const char * const arg, const char * const pn )
  {
  const char * tail;
  if( chsp == 0 ) chsp = new CHS;
  CHS & chs = *chsp;
  chs.cylinders = getnum( arg, pn, 0, 40, 1000000, &tail );
  if( *tail != ':' )
err: { show_option_error( arg, "Invalid 'C:H:S:order:head' description in", pn );
       std::exit( 1 ); }
  chs.heads = getnum( tail + 1, pn, 0, 1, 256, &tail );
  if( *tail != ':' ) goto err;
  chs.first_track_sectors = getnum( tail + 1, pn, 0, 8, 10000, &tail );
  if( *tail == '-' )
    {
    chs.last_track_sectors = getnum( tail + 1, pn, 0, 8, 10000, &tail );
    if( chs.last_track_sectors == chs.first_track_sectors ) goto err;
    if( *tail == ',' )
      chs.tracks_zone = getnum( tail + 1, pn, 0, 8, chs.cylinders, &tail );
    else chs.tracks_zone = chs.cylinders / std::min( 24,
         std::abs( chs.last_track_sectors - chs.first_track_sectors ) );
    }
  else { chs.last_track_sectors = chs.first_track_sectors;
         chs.tracks_zone = chs.cylinders; }
  if( *tail++ != ':' ) goto err;
  if( *tail == 'F' )
    { if( tail[1] == 'F' ) { chs.track_order = chs.FF; tail += 2; }
      else { chs.track_order = chs.F; ++tail; } }
  else goto err;
  if( *tail != ':' ) goto err;
  chs.bad_head = getnum( tail + 1, pn, 0, 0, chs.heads - 1, &tail );
  if( *tail != 0 ) goto err;
  if( ( chs.track_order == chs.F ) !=
      ( chs.last_track_sectors == chs.first_track_sectors ) ) goto err;
  }


int make_test( const char * const mapname, const int hardbs,
               const bool force )
  {
  Mapfile mapfile( mapname );
  const bool to_stdout = std::strcmp( mapname, "-" ) == 0;
  if( !to_stdout && !force && mapfile.read_mapfile( 0, false ) )
    {
    show_file_error( mapname, "Mapfile exists. Use '--force' to overwrite it." );
    return 1;
    }
  mapfile.current_status( mapfile.finished );
  const CHS & chs = *chsp;
  const int zones =
    chs.cylinders / chs.tracks_zone + ( chs.cylinders % chs.tracks_zone != 0 );
  switch( chs.track_order )
    {
    case CHS::F:
      for( int c = 0; c < chs.cylinders; ++c )
        for( int h = 0; h < chs.heads; ++h )
          for( int s = 0; s < chs.first_track_sectors; ++s )
            mapfile.append_sblock( hardbs, ( chs.bad_head == h ) ?
                                   Sblock::bad_sector : Sblock::finished );
      break;
    case CHS::FF:
      for( int z = 0; z * chs.tracks_zone < chs.cylinders; ++z )
        {
        const int climit = std::min( chs.cylinders, (z+1) * chs.tracks_zone );
        const int sectors = ( chs.first_track_sectors * ( zones - 1 - z ) +
                              chs.last_track_sectors * z ) / ( zones - 1 );
        for( int h = 0; h < chs.heads; ++h )
          for( int c = z * chs.tracks_zone; c < climit; ++c )
            for( int s = 0; s < sectors; ++s )
              mapfile.append_sblock( hardbs, ( chs.bad_head == h ) ?
                                     Sblock::bad_sector : Sblock::finished );
        }
    }
  mapfile.compact_sblock_vector();
  if( !mapfile.write_mapfile( to_stdout ? stdout : 0 ) ||
      ( to_stdout && std::fclose( stdout ) != 0 ) )
    { show_file_error( mapfile.pname( false ), "Error writing mapfile", errno );
      return 1; }
  return 0;
  }
