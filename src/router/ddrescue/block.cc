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
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "block.h"


// Align pos to next boundary if size is big enough.
//
void Block::align_pos( const int alignment )
  {
  if( alignment > 1 )
    {
    const int disp = alignment - ( pos_ % alignment );
    if( disp < alignment && disp < size_ )
      { pos_ += disp; size_ -= disp; }
    }
  }


// Align end to previous boundary if size is big enough.
//
void Block::align_end( const int alignment )
  {
  if( alignment > 1 )
    {
    const int rest = end() % alignment;
    if( size_ > rest ) size_ -= rest;
    }
  }


void Block::crop( const Block & b )
  {
  const long long p = std::max( pos_, b.pos_ );
  const long long s = std::max( 0LL, std::min( end(), b.end() ) - p );
  pos_ = p; size_ = s;
  }


bool Block::join( const Block & b )		// join contiguous blocks
  {
  if( this->follows( b ) ) pos_ = b.pos_;
  else if( !b.follows( *this ) ) return false;
  if( b.size_ > LLONG_MAX - end() )
    internal_error( "size overflow joining two Blocks." );
  size_ += b.size_;
  return true;
  }


// shift the boundary of two consecutive Blocks
void Block::shift_boundary( Block & b, const long long pos )
  {
  if( end() != b.pos_ || pos <= pos_ || pos >= b.end() )
    internal_error( "bad argument shifting the border of two Blocks." );
  b.size_ = b.end() - pos; b.pos_ = pos; size_ = pos - pos_;
  }


Block Block::split( long long pos, const int hardbs )
  {
  if( hardbs > 1 ) pos -= pos % hardbs;
  if( pos_ < pos && end() > pos )
    {
    const Block b( pos_, pos - pos_ );
    pos_ = pos; size_ -= b.size_;
    return b;
    }
  return Block( 0, 0 );
  }


Domain::Domain( const long long p, const long long s,
                const char * const mapname, const bool loose )
  {
  reset_cached_in_size();
  const Block b( p, s );
  if( !mapname || !mapname[0] ) { block_vector.push_back( b ); return; }
  Mapfile mapfile( mapname );
  if( !mapfile.read_mapfile( loose ? '?' : 0 ) )
    { show_file_error( mapname, "Mapfile does not exist or is not readable." );
      std::exit( 1 ); }
  mapfile.compact_sblock_vector();
  for( long i = 0; i < mapfile.sblocks(); ++i )
    {
    const Sblock & sb = mapfile.sblock( i );
    if( sb.status() == Sblock::finished ) block_vector.push_back( sb );
    }
  if( block_vector.empty() ) block_vector.push_back( Block( 0, 0 ) );
  else this->crop( b );
  }


void Domain::crop( const Block & b )
  {
  reset_cached_in_size();
  unsigned long r = block_vector.size();
  while( r > 0 && b < block_vector[r-1] ) --r;
  if( r > 0 ) block_vector[r-1].crop( b );
  if( r <= 0 || block_vector[r-1].size() <= 0 )	// no block overlaps b
    { block_vector.clear(); block_vector.push_back( Block( 0, 0 ) ); return; }
  if( r < block_vector.size() )			// remove blocks beyond b
    block_vector.erase( block_vector.begin() + r, block_vector.end() );
  if( b.pos() <= 0 ) return;
  --r;		// block_vector[r] is now the last non-cropped-out block
  unsigned long l = 0;
  while( l < r && block_vector[l] < b ) ++l;
  if( l < r ) block_vector[l].crop( b );	// crop block overlapping b
  if( l > 0 )					// remove blocks before b
    block_vector.erase( block_vector.begin(), block_vector.begin() + l );
  }
