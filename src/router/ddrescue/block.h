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

#ifndef LLONG_MAX
#define LLONG_MAX  0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN  (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX 0xFFFFFFFFFFFFFFFFULL
#endif


class Block
  {
  long long pos_, size_;  // pos >= 0 && size >= 0 && pos + size <= LLONG_MAX

  void fix_size()		// limit size_ to largest possible value
    { if( size_ < 0 || size_ > LLONG_MAX - pos_ ) size_ = LLONG_MAX - pos_; }

public:
  Block( const long long p, const long long s ) : pos_( p ), size_( s )
    { if( p < 0 ) { pos_ = 0; if( s > 0 ) size_ -= std::min( s, -p ); }
      fix_size(); }

  long long pos() const { return pos_; }
  long long size() const { return size_; }
  long long end() const { return pos_ + size_; }

  void pos( const long long p )
    { pos_ = std::max( p, 0LL );
      if( size_ > LLONG_MAX - pos_ ) size_ = LLONG_MAX - pos_; }
  void size( const long long s ) { size_ = s; fix_size(); }
  void end( long long e )
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

  bool operator<( const Block & b ) const { return ( end() <= b.pos() ); }

  bool follows( const Block & b ) const
    { return ( pos_ == b.end() ); }
  bool includes( const Block & b ) const
    { return ( pos_ <= b.pos_ && end() >= b.end() ); }
  bool includes( const long long pos ) const
    { return ( pos_ <= pos && end() > pos ); }
  bool strictly_includes( const long long pos ) const
    { return ( pos_ < pos && end() > pos ); }

  void crop( const Block & b );
  bool join( const Block & b );
  void shift( Block & b, const long long pos );
  Block split( long long pos, const int hardbs = 1 );
  };


class Sblock : public Block
  {
public:
  enum Status
    { non_tried = '?', non_trimmed = '*', non_scraped = '/',
      bad_sector = '-', finished = '+' };
private:
  Status status_;

public:
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
  static bool is_good_status( const Status st )
    { return ( st == non_tried || st == finished ); }
  };


class Domain
  {
  std::vector< Block > block_vector;	// blocks are ordered and don't overlap

public:
  Domain( const long long p, const long long s,
          const char * const logname = 0, const bool loose = false );

  long long pos() const { return block_vector.front().pos(); }
  long long end() const { return block_vector.back().end(); }
  long long size() const { return end() - pos(); }
  const Block & block( const int i ) const { return block_vector[i]; }
  int blocks() const { return (int)block_vector.size(); }
  bool empty() const { return ( end() <= pos() ); }
  bool full() const { return ( !empty() && end() >= LLONG_MAX ); }

  long long in_size() const
    {
    long long s = 0;
    for( unsigned i = 0; i < block_vector.size(); ++i )
      s += block_vector[i].size();
    return s;
    }

  bool operator!=( const Domain & d ) const
    {
    if( block_vector.size() != d.block_vector.size() ) return true;
    for( unsigned i = 0; i < block_vector.size(); ++i )
      if( block_vector[i] != d.block_vector[i] ) return true;
    return false;
    }

  bool operator<( const Block & b ) const { return ( end() <= b.pos() ); }
  bool operator>( const Block & b ) const { return ( pos() >= b.end() ); }

  bool includes( const Block & b ) const
    {
    unsigned l = 0, r = block_vector.size();
    while( l < r )
      {
      const int m = ( l + r ) / 2;
      const Block & db = block_vector[m];
      if( db.includes( b ) ) return true;
      if( db < b ) l = m + 1; else if( b < db ) r = m; else break;
      }
    return false;
    }

  bool includes( const long long pos ) const
    {
    for( unsigned i = 0; i < block_vector.size(); ++i )
      if( block_vector[i].includes( pos ) ) return true;
    return false;
    }

  void clear()
    { block_vector.clear(); block_vector.push_back( Block( 0, 0 ) ); }
  void crop( const Block & b );
  void crop_by_file_size( const long long size ) { crop( Block( 0, size ) ); }
  };


class Logfile
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
  mutable int index_;			// cached index of last find or change
  bool read_only_;
  std::vector< Sblock > sblock_vector;	// note: blocks are consecutive

  void erase_sblock( const int i )
    { sblock_vector.erase( sblock_vector.begin() + i ); }
  void insert_sblock( const int i, const Sblock & sb )
    { sblock_vector.insert( sblock_vector.begin() + i, sb ); }

public:
  explicit Logfile( const char * const logname )
    : current_pos_( 0 ), filename_( logname ), current_status_( copying ),
      index_( 0 ), read_only_( false ) {}

  void compact_sblock_vector();
  void extend_sblock_vector( const long long isize );
  bool truncate_vector( const long long end, const bool force = false );
  void make_blank()
    { sblock_vector.clear();
      sblock_vector.push_back( Sblock( 0, -1, Sblock::non_tried ) ); }
  bool read_logfile( const int default_sblock_status = 0 );
  int write_logfile( FILE * f = 0, const bool timestamp = false ) const;

  bool blank() const;
  long long current_pos() const { return current_pos_; }
  Status current_status() const { return current_status_; }
  const char * filename() const { return filename_; }
  bool read_only() const { return read_only_; }

  void current_pos( const long long pos ) { current_pos_ = pos; }
  void current_status( const Status st, const char * const msg = "" )
    { current_status_ = st;
      current_msg = ( st == finished ) ? "Finished" : msg; }

  Block extent() const
    { if( sblock_vector.empty() ) return Block( 0, 0 );
      return Block( sblock_vector.front().pos(),
                    sblock_vector.back().end() - sblock_vector.front().pos() ); }
  const Sblock & sblock( const int i ) const { return sblock_vector[i]; }
  int sblocks() const { return (int)sblock_vector.size(); }
  void change_sblock_status( const int i, const Sblock::Status st )
    { sblock_vector[i].status( st ); }

  void split_by_domain_borders( const Domain & domain );
  void split_by_logfile_borders( const Logfile & logfile );
  bool try_split_sblock_by( const long long pos, const int i )
    {
    if( sblock_vector[i].strictly_includes( pos ) )
      { insert_sblock( i, sblock_vector[i].split( pos ) ); return true; }
    return false;
    }

  int find_index( const long long pos ) const;
  void find_chunk( Block & b, const Sblock::Status st,
                   const Domain & domain, const int alignment = 1 ) const;
  void rfind_chunk( Block & b, const Sblock::Status st,
                    const Domain & domain, const int alignment = 1 ) const;
  int change_chunk_status( const Block & b, const Sblock::Status st,
                           const Domain & domain );

  static bool isstatus( const int st )
    { return ( st == copying || st == trimming || st == scraping ||
               st == retrying || st == filling || st == generating ||
               st == finished ); }
  static const char * status_name( const Status st );
  };


// Defined in main_common.cc
//
extern int verbosity;
void show_error( const char * const msg,
                 const int errcode = 0, const bool help = false );
void internal_error( const char * const msg );
int empty_domain();
int not_readable( const char * const logname );
int not_writable( const char * const logname );
long initial_time();
bool write_logfile_header( FILE * const f, const char * const logtype );
bool write_timestamp( FILE * const f );
bool write_final_timestamp( FILE * const f );
const char * format_num( long long num, long long limit = 999999,
                         const int set_prefix = 0 );
