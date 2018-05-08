/*  GNU ddrescue - Data recovery tool
    Copyright (C) 2004-2018 Antonio Diaz Diaz.

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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>

#include "block.h"


namespace {

int my_fgetc( FILE * const f, const bool allow_comment = true )
  {
  int ch = std::fgetc( f );
  if( ch == '#' && allow_comment )			// comment
    { do ch = std::fgetc( f ); while( ch != '\n' && ch != EOF ); }
  return ch;
  }


// Read a line discarding comments, leading whitespace and blank lines.
// Returns 0 if at EOF.
//
const char * my_fgets( FILE * const f, int & linenum )
  {
  const int maxlen = 127;
  static char buf[maxlen+1];
  int ch, len = 1;

  while( len == 1 )			// while line is blank
    {
    do { ch = my_fgetc( f ); if( ch == '\n' ) ++linenum; }
    while( std::isspace( ch ) );
    len = 0;
    while( true )
      {
      if( ch == EOF ) { if( len > 0 ) ch = '\n'; else break; }
      if( len < maxlen ) buf[len++] = ch;
      if( ch == '\n' ) { ++linenum; break; }
      ch = my_fgetc( f, std::isspace( ch ) );
      }
    }
  if( len > 0 ) { buf[len] = 0; return buf; }
  else return 0;
  }


void show_mapfile_error( const char * const mapname, const int linenum )
  {
  char buf[80];
  snprintf( buf, sizeof buf, "error in mapfile %s, line %d.", mapname, linenum );
  show_error( buf );
  }

} // end namespace


void Mapfile::compact_sblock_vector()
  {
  std::vector< Sblock > new_vector;
  unsigned long l = 0;
  while( l < sblock_vector.size() )
    {
    Sblock run = sblock_vector[l];
    unsigned long r = l + 1;
    while( r < sblock_vector.size() &&
           sblock_vector[r].status() == run.status() ) ++r;
    if( r > l + 1 ) run.size( sblock_vector[r-1].end() - run.pos() );
    new_vector.push_back( run );
    l = r;
    }
  sblock_vector.swap( new_vector );
  }


void Mapfile::extend_sblock_vector( const long long isize )
  {
  if( sblock_vector.empty() )
    {
    const Sblock sb( 0, ( isize > 0 ) ? isize : -1, Sblock::non_tried );
    sblock_vector.push_back( sb );
    return;
    }
  Sblock & front = sblock_vector.front();
  if( front.pos() > 0 )
    sblock_vector.insert( sblock_vector.begin(), Sblock( 0, front.pos(), Sblock::non_tried ) );
  Sblock & back = sblock_vector.back();
  const long long end = back.end();
  if( isize > 0 )
    {
    if( back.pos() >= isize )
      {
      if( back.pos() == isize && back.status() != Sblock::finished )
        { sblock_vector.pop_back(); return; }
      show_error( "Last block in mapfile begins past end of input file.\n"
                  "          Use '-C' if you are reading from a partial copy.",
                  0, true );
      std::exit( 1 );
      }
    if( end > isize )
      {
      if( back.status() != Sblock::finished )
        { back.size( isize - back.pos() ); return; }
      show_error( "Rescued data in mapfile goes past end of input file.\n"
                  "          Use '-C' if you are reading from a partial copy.",
                  0, true );
      std::exit( 1 );
      }
    else if( end < isize )
      sblock_vector.push_back( Sblock( end, isize - end, Sblock::non_tried ) );
    }
  else if( end >= 0 )
    {
    const Sblock sb( end, -1, Sblock::non_tried );
    if( sb.size() > 0 ) sblock_vector.push_back( sb );
    }
  }


void Mapfile::shift_blocks( const long long offset )
  {
  if( sblock_vector.empty() ) return;
  if( offset > 0 )
    {
    if( sblock_vector.front().status() == Sblock::non_tried )
      sblock_vector.front().enlarge( offset );
    else
      insert_sblock( 0, Sblock( 0, offset, Sblock::non_tried ) );
    for( unsigned long i = 1; i < sblock_vector.size(); ++i )
      {
      sblock_vector[i].shift( offset );
      if( sblock_vector[i].size() <= 0 )
        { sblock_vector.erase( sblock_vector.begin() + i, sblock_vector.end() );
          break; }
      }
    }
  else if( offset < 0 )
    {
    for( unsigned long i = 0; i < sblock_vector.size(); ++i )
      if( sblock_vector[i].end() + offset > 0 )
        {
        if( i > 0 )
          sblock_vector.erase( sblock_vector.begin(), sblock_vector.begin() + i );
        break;
        }
    for( unsigned long i = 0; i < sblock_vector.size(); ++i )
      sblock_vector[i].shift( offset );
    }
  }


// Returns false only if truncation would remove finished blocks and
// force is false.
//
bool Mapfile::truncate_vector( const long long end, const bool force )
  {
  unsigned long i = sblock_vector.size();
  while( i > 0 && sblock_vector[i-1].pos() >= end ) --i;
  if( !force )
    for( unsigned long j = i; j < sblock_vector.size(); ++j )
      if( sblock_vector[j].status() == Sblock::finished ) return false;
  if( i == 0 )
    {
    sblock_vector.clear();
    sblock_vector.push_back( Sblock( 0, 0, Sblock::non_tried ) );
    }
  else
    {
    Sblock & sb = sblock_vector[i-1];
    if( sb.includes( end ) )
      {
      if( !force && sb.status() == Sblock::finished ) return false;
      sb.size( end - sb.pos() );
      }
    sblock_vector.erase( sblock_vector.begin() + i, sblock_vector.end() );
    }
  return true;
  }


// Returns true if mapfile exists and is readable.
// Fills the gaps if 'default_sblock_status' is a valid status character.
//
bool Mapfile::read_mapfile( const int default_sblock_status, const bool ro )
  {
  FILE * f = 0;
  errno = 0;
  read_only_ = ro;
  if( ro && std::strcmp( filename_, "-" ) == 0 ) f = stdin;
  else if( ro || ( !(f = std::fopen( filename_, "r+" )) && errno != ENOENT ) )
    { f = std::fopen( filename_, "r" ); read_only_ = true; }
  if( !f ) return false;
  int linenum = 0;
  const bool loose = Sblock::isstatus( default_sblock_status );
  sblock_vector.clear();

  const char * line = my_fgets( f, linenum );
  if( line )						// status line
    {
    char ch;
    current_pass_ = 1;					// default value
    const int n = std::sscanf( line, "%lli %c %d\n",
                               &current_pos_, &ch, &current_pass_ );
    if( ( n == 3 || n == 2 ) && current_pos_ >= 0 && isstatus( ch ) &&
        current_pass_ >= 1 )
      current_status_ = Status( ch );
    else
      { show_mapfile_error( filename_, linenum ); std::exit( 2 ); }

    while( true )
      {
      line = my_fgets( f, linenum );
      if( !line ) break;
      long long pos, size;
      const int n = std::sscanf( line, "%lli %lli %c\n", &pos, &size, &ch );
      if( n == 3 && pos >= 0 && Sblock::isstatus( ch ) &&
          ( size > 0 || ( size == 0 && pos == 0 ) ) )
        {
        const Sblock::Status st = Sblock::Status( ch );
        const Sblock sb( pos, size, st );
        const long long end = sblock_vector.size() ?
                              sblock_vector.back().end() : 0;
        if( sb.pos() != end )
          {
          if( loose && sb.pos() > end )
            { const Sblock sb2( end, sb.pos() - end,
                                Sblock::Status( default_sblock_status ) );
              sblock_vector.push_back( sb2 ); }
          else if( end > 0 )
            { show_mapfile_error( filename_, linenum ); std::exit( 2 ); }
          }
        sblock_vector.push_back( sb );
        }
      else
        { show_mapfile_error( filename_, linenum ); std::exit( 2 ); }
      }
    }
  if( std::ferror( f ) || !std::feof( f ) || std::fclose( f ) != 0 )
    { show_mapfile_error( filename_, linenum ); std::exit( 2 ); }
  return true;
  }


int Mapfile::write_mapfile( FILE * f, const bool timestamp,
                            const bool mf_sync,
                            const Domain * const annotate_domainp ) const
  {
  const bool f_given = ( f != 0 );

  if( !f && !filename_ ) return false;
  if( !f ) { f = std::fopen( filename_, "w" ); if( !f ) return false; }
  write_file_header( f, "Mapfile" );
  if( timestamp ) write_timestamp( f );
  if( current_msg.size() ) std::fprintf( f, "# %s\n", current_msg.c_str() );
  char buf[80] = { 0 };		// comment
  if( annotate_domainp )
    snprintf( buf, sizeof buf, "\t#  %sB", format_num( current_pos_ ) );
  std::fprintf( f, "# current_pos  current_status  current_pass\n"
                   "0x%08llX     %c               %d%s\n"
                   "#      pos        size  status\n",
                current_pos_, current_status_, current_pass_, buf );
  for( unsigned long i = 0; i < sblock_vector.size(); ++i )
    {
    const Sblock & sb = sblock_vector[i];
    if( annotate_domainp && annotate_domainp->includes( sb ) )
      snprintf( buf, sizeof buf, "\t#  %9sB  %9s%c", format_num( sb.pos() ),
                format_num( sb.size() ), ( sb.size() > 999999 ) ? 'B' : ' ' );
    else buf[0] = 0;
    std::fprintf( f, "0x%08llX  0x%08llX  %c%s\n",
                  sb.pos(), sb.size(), sb.status(), buf );
    }
  if( mf_sync ) fsync( fileno( f ) );
  return ( f_given || std::fclose( f ) == 0 );
  }


bool Mapfile::blank() const
  {
  for( unsigned long i = 0; i < sblock_vector.size(); ++i )
    if( sblock_vector[i].status() != Sblock::non_tried )
      return false;
  return true;
  }


void Mapfile::split_by_domain_borders( const Domain & domain )
  {
  if( domain.blocks() == 1 )
    {
    const Block & db = domain.block( 0 );
    unsigned long i = 0;
    while( i < sblock_vector.size() && sblock_vector[i] < db ) ++i;
    if( i < sblock_vector.size() ) try_split_sblock_by( db.pos(), i );
    i = sblock_vector.size();
    while( i > 0 && db < sblock_vector[i-1] ) --i;
    if( i > 0 ) try_split_sblock_by( db.end(), i - 1 );
    }
  else
    {
    std::vector< Sblock > new_vector;
    long j = 0;
    for( unsigned long i = 0; i < sblock_vector.size(); )
      {
      Sblock & sb = sblock_vector[i];
      while( j < domain.blocks() && domain.block( j ) < sb ) ++j;
      if( j >= domain.blocks() )		// end of domain tail copy
        { new_vector.insert( new_vector.end(),
                             sblock_vector.begin() + i, sblock_vector.end() );
          break; }
      const Block & db = domain.block( j );
      if( sb.strictly_includes( db.pos() ) )
        new_vector.push_back( sb.split( db.pos() ) );
      if( sb.strictly_includes( db.end() ) )
        new_vector.push_back( sb.split( db.end() ) );
      if( sb.pos() < db.end() ) { new_vector.push_back( sb ); ++i; }
      }
    sblock_vector.swap( new_vector );
    }
  }


void Mapfile::split_by_mapfile_borders( const Mapfile & mapfile )
  {
  std::vector< Sblock > new_vector;
  long j = 0;
  for( unsigned long i = 0; i < sblock_vector.size(); )
    {
    Sblock & sb = sblock_vector[i];
    while( j < mapfile.sblocks() && mapfile.sblock( j ) < sb ) ++j;
    if( j >= mapfile.sblocks() )		// end of mapfile tail copy
      { new_vector.insert( new_vector.end(),
                           sblock_vector.begin() + i, sblock_vector.end() );
        break; }
    const Sblock & db = mapfile.sblock( j );
    if( sb.strictly_includes( db.pos() ) )
      new_vector.push_back( sb.split( db.pos() ) );
    if( sb.strictly_includes( db.end() ) )
      new_vector.push_back( sb.split( db.end() ) );
    if( sb.pos() < db.end() ) { new_vector.push_back( sb ); ++i; }
    }
  sblock_vector.swap( new_vector );
  }


long Mapfile::find_index( const long long pos ) const
  {
  if( index_ < 0 || index_ >= sblocks() ) index_ = sblocks() / 2;
  while( index_ + 1 < sblocks() && pos >= sblock_vector[index_+1].pos() )
    ++index_;
  while( index_ > 0 && pos < sblock_vector[index_].pos() )
    --index_;
  if( !sblock_vector[index_].includes( pos ) ) index_ = -1;
  return index_;
  }


// Find chunk from b.pos forwards of size <= b.size and status st.
// If not found, or if after_finished is true and none of the blocks
// found follows a finished block, put b.size to 0.
// If at least one block of status st is found, return true.
//
bool Mapfile::find_chunk( Block & b, const Sblock::Status st,
                          const Domain & domain, const int alignment,
                          const bool after_finished ) const
  {
  if( b.size() <= 0 ) return false;
  if( b.pos() < sblock_vector.front().pos() )
    b.pos( sblock_vector.front().pos() );
  if( find_index( b.pos() ) < 0 ) { b.size( 0 ); return false; }
  long i;
  bool block_found = false;
  for( i = index_; i < sblocks(); ++i )
    if( sblock_vector[i].status() == st && domain.includes( sblock_vector[i] ) )
      {
      block_found = true;
      if( !after_finished || i <= 0 ||
          sblock_vector[i-1].status() == Sblock::finished )
        { index_ = i; break; }
      }
  if( i >= sblocks() ) { b.size( 0 ); return block_found; }
  if( b.pos() < sblock_vector[index_].pos() )
    b.pos( sblock_vector[index_].pos() );
  if( !sblock_vector[index_].includes( b ) )
    b.crop( sblock_vector[index_] );
  if( b.end() != sblock_vector[index_].end() )
    b.align_end( alignment );
  return block_found;
  }


// Find chunk from b.end backwards of size <= b.size and status st.
// If not found, or if before_finished is true and none of the blocks
// found precedes a finished block, put b.size to 0.
// If at least one block of status st is found, return true.
//
bool Mapfile::rfind_chunk( Block & b, const Sblock::Status st,
                           const Domain & domain, const int alignment,
                           const bool before_finished ) const
  {
  if( b.size() <= 0 ) return false;
  if( b.end() > sblock_vector.back().end() )
    b.end( sblock_vector.back().end() );
  if( find_index( b.end() - 1 ) < 0 ) { b.size( 0 ); return false; }
  long i;
  bool block_found = false;
  for( i = index_; i >= 0; --i )
    if( sblock_vector[i].status() == st && domain.includes( sblock_vector[i] ) )
      {
      block_found = true;
      if( !before_finished || i + 1 >= sblocks() ||
          sblock_vector[i+1].status() == Sblock::finished )
        { index_ = i; break; }
      }
  if( i < 0 ) { b.size( 0 ); return block_found; }
  if( b.end() > sblock_vector[index_].end() )
    b.end( sblock_vector[index_].end() );
  if( !sblock_vector[index_].includes( b ) )
    b.crop( sblock_vector[index_] );
  if( b.pos() != sblock_vector[index_].pos() )
    b.align_pos( alignment );
  return block_found;
  }


// Returns an adjust value (-1, 0, +1) to keep 'bad_areas' updated.
//   - - -   -->   - + -   return +1
//   - - +   -->   - + +   return  0
//   - + -   -->   - - -   return -1
//   - + +   -->   - - +   return  0
//   + - -   -->   + + -   return  0
//   + - +   -->   + + +   return -1
//   + + -   -->   + - -   return  0
//   + + +   -->   + - +   return +1
//
int Mapfile::change_chunk_status( const Block & b, const Sblock::Status st,
                                  const Domain & domain,
                                  Sblock::Status * const old_stp )
  {
  if( b.size() <= 0 ) return 0;
  if( !domain.includes( b ) || find_index( b.pos() ) < 0 ||
      !domain.includes( sblock_vector[index_] ) )
    internal_error( "can't change status of chunk not in rescue domain." );
  if( !sblock_vector[index_].includes( b ) )
    internal_error( "can't change status of chunk spread over more than 1 block." );
  const Sblock::Status old_st = sblock_vector[index_].status();
  if( old_stp ) *old_stp = old_st;
  if( st == old_st ) return 0;
  const bool old_st_good = Sblock::is_good_status( old_st );
  const bool new_st_good = Sblock::is_good_status( st );
  bool bl_st_good = ( index_ <= 0 ||
                      Sblock::is_good_status( sblock_vector[index_-1].status() ) ||
                      !domain.includes( sblock_vector[index_-1] ) );
  bool br_st_good = ( index_ + 1 >= sblocks() ||
                      Sblock::is_good_status( sblock_vector[index_+1].status() ) ||
                      !domain.includes( sblock_vector[index_+1] ) );

  if( sblock_vector[index_].pos() < b.pos() )
    {
    if( sblock_vector[index_].end() == b.end() &&
        index_ + 1 < sblocks() && sblock_vector[index_+1].status() == st &&
        domain.includes( sblock_vector[index_+1] ) )
      {
      sblock_vector[index_].shift_boundary( sblock_vector[index_+1], b.pos() );
      return 0;
      }
    insert_sblock( index_, sblock_vector[index_].split( b.pos() ) );
    ++index_;
    bl_st_good = old_st_good;
    }
  if( sblock_vector[index_].size() > b.size() )
    {
    if( index_ > 0 && sblock_vector[index_-1].status() == st &&
        domain.includes( sblock_vector[index_-1] ) )
      sblock_vector[index_-1].shift_boundary( sblock_vector[index_], b.end() );
    else
      insert_sblock( index_,
                     Sblock( sblock_vector[index_].split( b.end() ), st ) );
    br_st_good = old_st_good;
    }
  else
    {
    sblock_vector[index_].status( st );
    const bool bl_join = ( index_ > 0 &&
                           sblock_vector[index_-1].status() == st &&
                           domain.includes( sblock_vector[index_-1] ) );
    const bool br_join = ( index_ + 1 < sblocks() &&
                           sblock_vector[index_+1].status() == st &&
                           domain.includes( sblock_vector[index_+1] ) );
    if( bl_join || br_join )
      {
      if( br_join ) sblock_vector[index_].join( sblock_vector[index_+1] );
      if( bl_join )
        { --index_; sblock_vector[index_].join( sblock_vector[index_+1] ); }
      sblock_vector.erase( sblock_vector.begin() + ( index_ + 1 ),
                           sblock_vector.begin() + ( index_ + 1 + bl_join + br_join ) );
      }
    }
  int retval = 0;
  if( new_st_good != old_st_good && bl_st_good == br_st_good )
    { if( old_st_good == bl_st_good ) retval = +1; else retval = -1; }
  return retval;
  }


const char * Mapfile::status_name( const Mapfile::Status st )
  {
  switch( st )
    {
    case copying:    return "copying";
    case trimming:   return "trimming";
    case scraping:   return "scraping";
    case retrying:   return "retrying";
    case filling:    return "filling";
    case generating: return "generating";
    case finished:   return "finished";
    }
  return "unknown";			// should not be reached
  }
