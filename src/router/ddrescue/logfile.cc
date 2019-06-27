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
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "block.h"


namespace {

int my_fgetc( FILE * const f )
  {
  int ch = std::fgetc( f );
  if( ch == '#' )			// comment
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
      ch = my_fgetc( f );
      }
    }
  if( len > 0 ) { buf[len] = 0; return buf; }
  else return 0;
  }


void show_logfile_error( const char * const logname, const int linenum )
  {
  char buf[80];
  snprintf( buf, sizeof buf, "error in logfile %s, line %d.", logname, linenum );
  show_error( buf );
  }

} // end namespace


void Logfile::compact_sblock_vector()
  {
  std::vector< Sblock > new_vector;
  unsigned l = 0;
  while( l < sblock_vector.size() )
    {
    Sblock run = sblock_vector[l];
    unsigned r = l + 1;
    while( r < sblock_vector.size() &&
           sblock_vector[r].status() == run.status() ) ++r;
    if( r > l + 1 ) run.size( sblock_vector[r-1].end() - run.pos() );
    new_vector.push_back( run );
    l = r;
    }
  sblock_vector.swap( new_vector );
  }


void Logfile::extend_sblock_vector( const long long isize )
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
      show_error( "Last block in logfile begins past end of input file.\n"
                  "          Use '-C' if you are reading from a partial copy.",
                  0, true );
      std::exit( 1 );
      }
    if( end > isize )
      {
      if( back.status() != Sblock::finished )
        { back.size( isize - back.pos() ); return; }
      show_error( "Rescued data in logfile goes past end of input file.\n"
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


// Returns false only if truncation would remove finished blocks and
// force is false.
//
bool Logfile::truncate_vector( const long long end, const bool force )
  {
  unsigned i = sblock_vector.size();
  while( i > 0 && sblock_vector[i-1].pos() >= end ) --i;
  if( !force )
    for( unsigned j = i; j < sblock_vector.size(); ++j )
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


// Returns true if logfile exists and is readable.
// Fills the gaps if 'default_sblock_status' is a valid status character.
//
bool Logfile::read_logfile( const int default_sblock_status )
  {
  FILE * const f = std::fopen( filename_, "r" );
  if( !f ) return false;
  int linenum = 0;
  const bool loose = Sblock::isstatus( default_sblock_status );
  read_only_ = false;
  sblock_vector.clear();

  const char * line = my_fgets( f, linenum );
  if( line )						// status line
    {
    char ch;
    int n = std::sscanf( line, "%lli %c\n", &current_pos_, &ch );
    if( n == 2 && current_pos_ >= 0 && isstatus( ch ) )
      current_status_ = Status( ch );
    else
      {
      show_logfile_error( filename_, linenum );
      show_error( "Are you using a logfile from ddrescue 1.5 or older?" );
      std::exit( 2 );
      }

    while( true )
      {
      line = my_fgets( f, linenum );
      if( !line ) break;
      long long pos, size;
      n = std::sscanf( line, "%lli %lli %c\n", &pos, &size, &ch );
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
            { show_logfile_error( filename_, linenum ); std::exit( 2 ); }
          }
        sblock_vector.push_back( sb );
        }
      else
        { show_logfile_error( filename_, linenum ); std::exit( 2 ); }
      }
    }
  if( std::ferror( f ) )
    { show_logfile_error( filename_, linenum ); std::exit( 2 ); }
  if( std::freopen( filename_, "r+", f ) ) std::fclose( f );
  else read_only_ = true;
  return true;
  }


int Logfile::write_logfile( FILE * f, const bool timestamp ) const
  {
  const bool f_given = ( f != 0 );

  if( !f && !filename_ ) return false;
  if( !f ) { f = std::fopen( filename_, "w" ); if( !f ) return false; }
  write_logfile_header( f, "Rescue" );
  if( timestamp ) write_timestamp( f );
  if( current_msg.size() ) std::fprintf( f, "# %s\n", current_msg.c_str() );
  std::fprintf( f, "# current_pos  current_status\n" );
  std::fprintf( f, "0x%08llX     %c\n", current_pos_, current_status_ );
  std::fprintf( f, "#      pos        size  status\n" );
  for( unsigned i = 0; i < sblock_vector.size(); ++i )
    {
    const Sblock & sb = sblock_vector[i];
    std::fprintf( f, "0x%08llX  0x%08llX  %c\n", sb.pos(), sb.size(), sb.status() );
    }
  return ( f_given || std::fclose( f ) == 0 );
  }


bool Logfile::blank() const
  {
  for( unsigned i = 0; i < sblock_vector.size(); ++i )
    if( sblock_vector[i].status() != Sblock::non_tried )
      return false;
  return true;
  }


void Logfile::split_by_domain_borders( const Domain & domain )
  {
  if( domain.blocks() == 1 )
    {
    const Block & db = domain.block( 0 );
    unsigned i = 0;
    while( i < sblock_vector.size() && sblock_vector[i] < db ) ++i;
    if( i < sblock_vector.size() ) try_split_sblock_by( db.pos(), i );
    i = sblock_vector.size();
    while( i > 0 && db < sblock_vector[i-1] ) --i;
    if( i > 0 ) try_split_sblock_by( db.end(), i - 1 );
    }
  else
    {
    std::vector< Sblock > new_vector;
    int j = 0;
    for( unsigned i = 0; i < sblock_vector.size(); )
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


void Logfile::split_by_logfile_borders( const Logfile & logfile )
  {
  std::vector< Sblock > new_vector;
  int j = 0;
  for( unsigned i = 0; i < sblock_vector.size(); )
    {
    Sblock & sb = sblock_vector[i];
    while( j < logfile.sblocks() && logfile.sblock( j ) < sb ) ++j;
    if( j >= logfile.sblocks() )		// end of logfile tail copy
      { new_vector.insert( new_vector.end(),
                           sblock_vector.begin() + i, sblock_vector.end() );
        break; }
    const Sblock & db = logfile.sblock( j );
    if( sb.strictly_includes( db.pos() ) )
      new_vector.push_back( sb.split( db.pos() ) );
    if( sb.strictly_includes( db.end() ) )
      new_vector.push_back( sb.split( db.end() ) );
    if( sb.pos() < db.end() ) { new_vector.push_back( sb ); ++i; }
    }
  sblock_vector.swap( new_vector );
  }


int Logfile::find_index( const long long pos ) const
  {
  if( index_ < 0 || index_ >= sblocks() ) index_ = sblocks() / 2;
  while( index_ + 1 < sblocks() && pos >= sblock_vector[index_].end() )
    ++index_;
  while( index_ > 0 && pos < sblock_vector[index_].pos() )
    --index_;
  if( !sblock_vector[index_].includes( pos ) ) index_ = -1;
  return index_;
  }


// Find chunk from b.pos of size <= b.size and status st.
// If not found, put b.size to 0.
//
void Logfile::find_chunk( Block & b, const Sblock::Status st,
                          const Domain & domain, const int alignment ) const
  {
  if( b.size() <= 0 ) return;
  if( b.pos() < sblock_vector.front().pos() )
    b.pos( sblock_vector.front().pos() );
  if( find_index( b.pos() ) < 0 ) { b.size( 0 ); return; }
  int i;
  for( i = index_; i < sblocks(); ++i )
    if( sblock_vector[i].status() == st && domain.includes( sblock_vector[i] ) )
      { index_ = i; break; }
  if( i >= sblocks() ) { b.size( 0 ); return; }
  if( b.pos() < sblock_vector[index_].pos() )
    b.pos( sblock_vector[index_].pos() );
  if( !sblock_vector[index_].includes( b ) )
    b.crop( sblock_vector[index_] );
  if( b.end() != sblock_vector[index_].end() )
    b.align_end( alignment );
  }


// Find chunk from b.end backwards of size <= b.size and status st.
// If not found, put b.size to 0.
//
void Logfile::rfind_chunk( Block & b, const Sblock::Status st,
                           const Domain & domain, const int alignment ) const
  {
  if( b.size() <= 0 ) return;
  if( sblock_vector.back().end() < b.end() )
    b.end( sblock_vector.back().end() );
  if( find_index( b.end() - 1 ) < 0 ) { b.size( 0 ); return; }
  int i;
  for( i = index_; i >= 0; --i )
    if( sblock_vector[i].status() == st && domain.includes( sblock_vector[i] ) )
      { index_ = i; break; }
  if( i < 0 ) { b.size( 0 ); return; }
  if( b.end() > sblock_vector[index_].end() )
    b.end( sblock_vector[index_].end() );
  if( !sblock_vector[index_].includes( b ) )
    b.crop( sblock_vector[index_] );
  if( b.pos() != sblock_vector[index_].pos() )
    b.align_pos( alignment );
  }


// Returns an adjust value (-1, 0, +1) to keep "errors" updated without
// having to call count_errors every time.
//   - - -   -->   - + -   return +1
//   - - +   -->   - + +   return  0
//   - + -   -->   - - -   return -1
//   - + +   -->   - - +   return  0
//   + - -   -->   + + -   return  0
//   + - +   -->   + + +   return -1
//   + + -   -->   + - -   return  0
//   + + +   -->   + - +   return +1
//
int Logfile::change_chunk_status( const Block & b, const Sblock::Status st,
                                  const Domain & domain )
  {
  if( b.size() <= 0 ) return 0;
  if( !domain.includes( b ) || find_index( b.pos() ) < 0 ||
      !domain.includes( sblock_vector[index_] ) )
    internal_error( "can't change status of chunk not in rescue domain." );
  if( !sblock_vector[index_].includes( b ) )
    internal_error( "can't change status of chunk spread over more than 1 block." );
  if( sblock_vector[index_].status() == st ) return 0;

  const bool old_st_good = Sblock::is_good_status( sblock_vector[index_].status() );
  const bool new_st_good = Sblock::is_good_status( st );
  bool bl_st_good = ( index_ <= 0 ||
                      !domain.includes( sblock_vector[index_-1] ) ||
                      Sblock::is_good_status( sblock_vector[index_-1].status() ) );
  bool br_st_good = ( index_ + 1 >= sblocks() ||
                      !domain.includes( sblock_vector[index_+1] ) ||
                      Sblock::is_good_status( sblock_vector[index_+1].status() ) );
  if( sblock_vector[index_].pos() < b.pos() )
    {
    if( sblock_vector[index_].end() == b.end() &&
        index_ + 1 < sblocks() && sblock_vector[index_+1].status() == st &&
        domain.includes( sblock_vector[index_+1] ) )
      {
      sblock_vector[index_].shift( sblock_vector[index_+1], b.pos() );
      return 0;
      }
    insert_sblock( index_, sblock_vector[index_].split( b.pos() ) );
    ++index_;
    bl_st_good = old_st_good;
    }
  if( sblock_vector[index_].size() > b.size() )
    {
    br_st_good = Sblock::is_good_status( sblock_vector[index_].status() );
    if( index_ > 0 && sblock_vector[index_-1].status() == st &&
        domain.includes( sblock_vector[index_-1] ) )
      sblock_vector[index_-1].shift( sblock_vector[index_], b.end() );
    else
      insert_sblock( index_,
                     Sblock( sblock_vector[index_].split( b.end() ), st ) );
    }
  else
    {
    sblock_vector[index_].status( st );
    if( index_ > 0 && sblock_vector[index_-1].status() == st &&
        domain.includes( sblock_vector[index_-1] ) )
      {
      sblock_vector[index_-1].join( sblock_vector[index_] );
      erase_sblock( index_ ); --index_;
      }
    if( index_ + 1 < sblocks() && sblock_vector[index_+1].status() == st &&
        domain.includes( sblock_vector[index_+1] ) )
      {
      sblock_vector[index_].join( sblock_vector[index_+1] );
      erase_sblock( index_ + 1 );
      }
    }
  int retval = 0;
  if( new_st_good != old_st_good && bl_st_good == br_st_good )
    { if( old_st_good == bl_st_good ) retval = +1; else retval = -1; }
  return retval;
  }


const char * Logfile::status_name( const Logfile::Status st )
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
