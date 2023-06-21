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

#ifndef LLONG_MAX
#define LLONG_MAX 0x7FFFFFFFFFFFFFFFLL
#endif

// requires '#include <cstdio>' for 'FILE *'

class Block
  {
  long long pos_, size_;  // pos >= 0 && size >= 0 && pos + size <= LLONG_MAX

  void fix_size()		// limit size_ to largest possible value
    { if( size_ < 0 || size_ > LLONG_MAX - pos_ ) size_ = LLONG_MAX - pos_; }

public:
  Block() {}					// default constructor
  Block( const long long p, const long long s ) : pos_( p ), size_( s )
    { if( p < 0 ) { pos_ = 0; if( s > 0 ) size_ -= std::min( s, -p ); }
      fix_size(); }

  long long pos() const { return pos_; }
  long long size() const { return size_; }
  long long end() const { return pos_ + size_; }
  bool full() const { return ( end() >= LLONG_MAX ); }

  void pos( const long long p )
    { pos_ = std::max( p, 0LL );
      if( size_ > LLONG_MAX - pos_ ) size_ = LLONG_MAX - pos_; }
  void shift( const long long offset )
    { if( offset >= 0 )
        { pos_ += std::min( offset, LLONG_MAX - pos_ );
          if( size_ > LLONG_MAX - pos_ ) size_ = LLONG_MAX - pos_; }
      else if( ( pos_ += offset ) < 0 )
        { size_ = std::max( size_ + pos_, 0LL ); pos_ = 0; } }
  void size( const long long s ) { size_ = s; fix_size(); }
  void enlarge( long long s )
    { if( s < 0 ) s = LLONG_MAX;
      if( s > LLONG_MAX - end() ) s = LLONG_MAX - end();
      size_ += s; }
  void end( long long e )			// also moves pos
    { if( e < 0 ) e = LLONG_MAX;
      if( size_ <= e ) pos_ = e - size_; else { pos_ = 0; size_ = e; } }
  Block & assign( const long long p, const long long s )
    {
    pos_ = p; size_ = s;
    if( p < 0 ) { pos_ = 0; if( s > 0 ) size_ -= std::min( s, -p ); }
    fix_size(); return *this;
    }

  void align_pos( const int alignment );
  void align_end( const int alignment );

  bool operator==( const Block & b ) const
    { return pos_ == b.pos_ && size_ == b.size_; }
  bool operator!=( const Block & b ) const
    { return pos_ != b.pos_ || size_ != b.size_; }

  bool operator<( const Block & b ) const { return ( end() <= b.pos_ ); }

  bool follows( const Block & b ) const
    { return ( pos_ == b.end() ); }
  bool includes( const Block & b ) const
    { return ( pos_ <= b.pos_ && end() >= b.end() ); }
  bool includes( const long long pos ) const
    { return ( pos_ <= pos && end() > pos ); }
  bool strictly_includes( const long long pos ) const
    { return ( pos_ < pos && end() > pos ); }
  bool overlaps( const Block & b ) const
    { return ( pos_ < b.end() && b.pos_ < end() ); }

  void crop( const Block & b );
  bool join( const Block & b );			// join contiguous blocks
  void shift_boundary( Block & b, const long long pos );
  Block split( long long pos, const int hardbs = 1 );
  };


class Sblock : public Block
  {
public:
  enum Status		// ordered from less to more processed state
    { non_tried = '?', non_trimmed = '*', non_scraped = '/',
      bad_sector = '-', finished = '+' };
private:
  Status status_;

public:
  Sblock() {}					// default constructor
  Sblock( const Block & b, const Status st )
    : Block( b ), status_( st ) {}
  Sblock( const long long p, const long long s, const Status st )
    : Block( p, s ), status_( st ) {}

  Status status() const { return status_; }
  void status( const Status st ) { status_ = st; }

  bool operator!=( const Sblock & sb ) const
    { return Block::operator!=( sb ) || status_ != sb.status_; }

  bool join( const Sblock & sb )
    { if( status_ == sb.status_ ) return Block::join( sb ); else return false; }
  Sblock split( const long long pos, const int hardbs = 1 )
    { return Sblock( Block::split( pos, hardbs ), status_ ); }
  static bool isstatus( const int st )
    { return ( st == non_tried || st == non_trimmed || st == non_scraped ||
               st == bad_sector || st == finished ); }
  static bool is_good_status( const Status st ) { return st != bad_sector; }
  static int processed_state( const Status st )
    {
    switch( st )
      {
      case non_tried:   return 0;
      case non_trimmed: return 1;
      case non_scraped: return 2;
      case bad_sector:  return 3;
      default:          return 4;
      }
    }
  };


class Domain
  {
  std::vector< Block > block_vector;	// blocks are ordered and don't overlap
  mutable long long cached_in_size;
  void reset_cached_in_size() { cached_in_size = -1; }

public:
  Domain( const long long p, const long long s,
          const char * const mapname = 0, const bool loose = false );

  long long pos() const { return block_vector.front().pos(); }
  long long end() const { return block_vector.back().end(); }
  long long size() const { return end() - pos(); }
  const Block & block( const long i ) const { return block_vector[i]; }
  long blocks() const { return block_vector.size(); }
  bool empty() const { return ( end() <= pos() ); }
  bool full() const { return ( !empty() && end() >= LLONG_MAX ); }

  long long in_size() const
    {
    if( cached_in_size < 0 )
      {
      cached_in_size = 0;
      for( unsigned long i = 0; i < block_vector.size(); ++i )
        cached_in_size += block_vector[i].size();
      }
    return cached_in_size;
    }

  bool operator!=( const Domain & d ) const
    {
    if( block_vector.size() != d.block_vector.size() ) return true;
    for( unsigned long i = 0; i < block_vector.size(); ++i )
      if( block_vector[i] != d.block_vector[i] ) return true;
    return false;
    }

  bool operator<( const Block & b ) const { return ( end() <= b.pos() ); }
  bool operator>( const Block & b ) const { return ( pos() >= b.end() ); }

  bool includes( const Block & b ) const
    {
    unsigned long l = 0, r = block_vector.size();
    while( l < r )
      {
      const long m = ( l + r ) / 2;
      const Block & db = block_vector[m];
      if( db.includes( b ) ) return true;
      if( db < b ) l = m + 1; else if( b < db ) r = m; else break;
      }
    return false;
    }

  bool includes( const long long pos ) const
    {
    for( unsigned long i = 0; i < block_vector.size(); ++i )
      if( block_vector[i].includes( pos ) ) return true;
    return false;
    }

  bool overlaps( const Block & b ) const
    {
    unsigned long l = 0, r = block_vector.size();
    while( l < r )
      {
      const long m = ( l + r ) / 2;
      const Block & db = block_vector[m];
      if( db.overlaps( b ) ) return true;
      if( db < b ) l = m + 1; else if( b < db ) r = m; else break;
      }
    return false;
    }

  void clear()
    {
    block_vector.clear(); block_vector.push_back( Block( 0, 0 ) );
    cached_in_size = 0;
    }

  void crop( const Block & b );
  void crop_by_file_size( const long long size ) { crop( Block( 0, size ) ); }
  };


class Mapfile
  {
public:
  enum Status
    { copying = '?', trimming = '*', scraping = '/', retrying = '-',
      filling = 'F', generating = 'G', finished = '+' };

private:
  long long current_pos_;
  const char * const filename_;
  std::string current_msg;
  Status current_status_;
  int current_pass_;
  mutable long index_;			// cached index of last find or change
  bool read_only_;
  std::vector< Sblock > sblock_vector;	// note: blocks are consecutive

  void insert_sblock( const long i, const Sblock & sb )	// insert before i
    { sblock_vector.insert( sblock_vector.begin() + i, sb ); }

public:
  explicit Mapfile( const char * const mapname )
    : current_pos_( 0 ), filename_( mapname ), current_status_( copying ),
      current_pass_( 1 ), index_( 0 ), read_only_( false ) {}

  void compact_sblock_vector();
  void join_subsectors( const int hardbs );
  void extend_sblock_vector( const long long insize );
  void shift_blocks( const long long offset, const Sblock::Status st );
  bool truncate_vector( const long long end, const bool force = false );
  void set_to_status( const Sblock::Status st )
    { sblock_vector.assign( 1, Sblock( 0, -1, st ) ); }
  bool read_mapfile( const int default_sblock_status = 0, const bool ro = true );
  bool write_mapfile( FILE * f = 0, const bool timestamp = false,
                      const bool mf_sync = false,
                      const Domain * const annotate_domainp = 0 ) const;

  bool blank() const;			// empty or all blocks non_tried
  long long current_pos() const { return current_pos_; }
  Status current_status() const { return current_status_; }
  int current_pass() const { return current_pass_; }
  const char * filename() const { return filename_; }
  const char * pname( const bool in = true ) const;	// printable name
  bool read_only() const { return read_only_; }

  void current_pos( const long long pos ) { current_pos_ = pos; }
  void current_status( const Status st, const char * const msg = "" )
    { current_status_ = st;
      current_msg = ( st == finished ) ? "Finished" : msg; }
  void current_pass( const int pass ) { current_pass_ = pass; }

  Block extent() const			// pos of first block may be > 0
    { if( sblock_vector.empty() ) return Block( 0, 0 );
      return Block( sblock_vector.front().pos(),
                    sblock_vector.back().end() - sblock_vector.front().pos() ); }
  const Sblock & sblock( const long i ) const { return sblock_vector[i]; }
  long sblocks() const { return sblock_vector.size(); }
  void change_sblock_status( const long i, const Sblock::Status st )
    { sblock_vector[i].status( st ); }

  void split_by_domain_borders( const Domain & domain );
  void split_by_mapfile_borders( const Mapfile & mapfile );
  bool try_split_sblock_by( const long long pos, const long i )
    {
    if( sblock_vector[i].strictly_includes( pos ) )
      { insert_sblock( i, sblock_vector[i].split( pos ) ); return true; }
    return false;
    }

  long find_index( const long long pos ) const;
  bool find_chunk( Block & b, const Sblock::Status st,
                   const Domain & domain, const int alignment,
                   const bool after_finished = false,
                   const bool unfinished = false ) const;
  bool rfind_chunk( Block & b, const Sblock::Status st,
                    const Domain & domain, const int alignment,
                    const bool before_finished = false ) const;
  int change_chunk_status( const Block & b, const Sblock::Status st,
                           const Domain & domain,
                           Sblock::Status * const old_stp = 0 );

  static bool isstatus( const int st )
    { return ( st == copying || st == trimming || st == scraping ||
               st == retrying || st == filling || st == generating ||
               st == finished ); }
  static const char * status_name( const Status st );
  };


// Defined in main_common.cc
extern int verbosity;
void show_error( const char * const msg, const int errcode = 0,
                 const bool help = false );
void show_file_error( const char * const filename, const char * const msg,
                      const int errcode = 0 );
void internal_error( const char * const msg );
int empty_domain();
int not_readable( const char * const mapname );
int not_writable( const char * const mapname );
long long initial_time();
bool write_file_header( FILE * const f, const char * const filetype );
bool write_timestamp( FILE * const f );
bool write_final_timestamp( FILE * const f );
const char * format_num( long long num, long long limit = 999999,
                         const int set_prefix = 0 );
const char * format_num3( long long num );
const char * format_percentage( long long num, long long den,
                                const int iwidth = 3, int prec = -2,
                                const bool rounding = true );
