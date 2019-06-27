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

class Logbook : public Logfile
  {
  const long long offset_;		// outfile offset (opos - ipos);
  long long logfile_isize_;
  Domain & domain_;			// rescue domain
  uint8_t *iobuf_base, *iobuf_;		// iobuf is aligned to page and hardbs
  const int hardbs_, softbs_;
  const char * final_msg_;
  int final_errno_;
  long ul_t1;				// variable for update_logfile
  bool logfile_exists_;

  Logbook( const Logbook & );		// declared as private
  void operator=( const Logbook & );	// declared as private

public:
  Logbook( const long long offset, const long long isize, Domain & dom,
           const char * const logname, const int cluster,
           const int hardbs, const bool complete_only );
  ~Logbook() { delete[] iobuf_base; }

  bool update_logfile( const int odes = -1, const bool force = false );

  const Domain & domain() const { return domain_; }
  uint8_t * iobuf() const { return iobuf_; }
  int hardbs() const { return hardbs_; }
  int softbs() const { return softbs_; }
  long long offset() const { return offset_; }
  const char * final_msg() const { return final_msg_; }
  int final_errno() const { return final_errno_; }
  bool logfile_exists() const { return logfile_exists_; }
  long long logfile_isize() const { return logfile_isize_; }

  void final_msg( const char * const msg, const int e = 0 )
    { final_msg_ = msg; final_errno_ = e; }

  void truncate_domain( const long long end )
    { domain_.crop_by_file_size( end ); }
  };


class Fillbook : public Logbook
  {
  long long filled_size;		// size already filled
  long long remaining_size;		// size to be filled
  int filled_areas;			// areas already filled
  int remaining_areas;			// areas to be filled
  int odes_;				// output file descriptor
  const bool ignore_write_errors_;
  const bool synchronous_;
					// variables for show_status
  long long a_rate, c_rate, first_size, last_size;
  long long last_ipos;
  long t0, t1;				// start, current times
  int oldlen;

  int fill_areas( const std::string & filltypes );
  int fill_block( const Block & b );
  void show_status( const long long ipos, const char * const msg = 0,
                    bool force = false );

public:
  Fillbook( const long long offset, Domain & dom,
            const char * const logname, const int cluster, const int hardbs,
            const bool ignore_write_errors, const bool synchronous )
    : Logbook( offset, 0, dom, logname, cluster, hardbs, true ),
      ignore_write_errors_( ignore_write_errors ),
      synchronous_( synchronous ),
      a_rate( 0 ), c_rate( 0 ), first_size( 0 ), last_size( 0 ),
      last_ipos( 0 ), t0( 0 ), t1( 0 ), oldlen( 0 )
      {}

  int do_fill( const int odes, const std::string & filltypes );
  bool read_buffer( const int ides );
  };


class Genbook : public Logbook
  {
  long long recsize, gensize;		// total recovered and generated sizes
  int odes_;				// output file descriptor
					// variables for show_status
  long long a_rate, c_rate, first_size, last_size;
  long long last_ipos;
  long t0, t1;				// start, current times
  int oldlen;

  void check_block( const Block & b, int & copied_size, int & error_size );
  int check_all();
  void show_status( const long long ipos, const char * const msg = 0,
                    bool force = false );
public:
  Genbook( const long long offset, const long long isize,
           Domain & dom, const char * const logname,
           const int cluster, const int hardbs )
    : Logbook( offset, isize, dom, logname, cluster, hardbs, false ),
      a_rate( 0 ), c_rate( 0 ), first_size( 0 ), last_size( 0 ),
      last_ipos( 0 ), t0( 0 ), t1( 0 ), oldlen( 0 )
      {}

  int do_generate( const int odes );
  };


struct Rb_options
  {
  enum { default_skipbs = 65536, max_max_skipbs = 1 << 30 };

  long long max_error_rate;
  long long min_outfile_size;
  long long min_read_rate;
  long pause;
  long timeout;
  int cpass_bitset;		// 1 | 2 | 4 for passes 1, 2, 3
  int max_errors;
  int max_retries;
  int o_direct;			// O_DIRECT or 0
  int preview_lines;		// preview lines to show. 0 = disable
  int skipbs;			// initial size to skip on read error
  int max_skipbs;		// maximum size to skip on read error
  bool complete_only;
  bool exit_on_error;
  bool new_errors_only;
  bool noscrape;
  bool notrim;
  bool reopen_on_error;
  bool retrim;
  bool reverse;
  bool sparse;
  bool try_again;
  bool unidirectional;

  Rb_options()
    : max_error_rate( -1 ), min_outfile_size( -1 ), min_read_rate( -1 ),
      pause( 0 ), timeout( -1 ), cpass_bitset( 7 ), max_errors( -1 ),
      max_retries( 0 ), o_direct( 0 ), preview_lines( 0 ),
      skipbs( default_skipbs ), max_skipbs( max_max_skipbs ),
      complete_only( false ), exit_on_error( false ),
      new_errors_only( false ), noscrape( false ), notrim( false ),
      reopen_on_error( false ), retrim( false ), reverse( false ),
      sparse( false ), try_again( false ), unidirectional( false )
      {}

  bool operator==( const Rb_options & o ) const
    { return ( max_error_rate == o.max_error_rate &&
               min_outfile_size == o.min_outfile_size &&
               min_read_rate == o.min_read_rate && pause == o.pause &&
               timeout == o.timeout && cpass_bitset == o.cpass_bitset &&
               max_errors == o.max_errors && max_retries == o.max_retries &&
               o_direct == o.o_direct && preview_lines == o.preview_lines &&
               skipbs == o.skipbs && max_skipbs == o.max_skipbs &&
               complete_only == o.complete_only &&
               exit_on_error == o.exit_on_error &&
               new_errors_only == o.new_errors_only &&
               noscrape == o.noscrape && notrim == o.notrim &&
               reopen_on_error == o.reopen_on_error &&
               retrim == o.retrim && reverse == o.reverse &&
               sparse == o.sparse && try_again == o.try_again &&
               unidirectional == o.unidirectional ); }
  bool operator!=( const Rb_options & o ) const
    { return !( *this == o ); }
  };


class Rescuebook : public Logbook, public Rb_options
  {
  long long error_rate;
  long long sparse_size;		// end position of pending writes
  long long recsize, errsize;		// total recovered and error sizes
  const Domain * const test_domain;	// good/bad map for test mode
  const char * const iname_;
  int e_code;				// error code for errors_or_timeout
					// 1 rate, 2 errors, 4 timeout
  int errors;				// error areas found so far
  int ides_, odes_;			// input and output file descriptors
  const bool access_works, synchronous_;
					// variables for update_rates
  long long a_rate, c_rate, first_size, last_size;
  long long iobuf_ipos;			// last pos read in iobuf, or -1
  long long last_ipos;
  long t0, t1, ts;			// start, current, last successful
  int oldlen;
  bool rates_updated;
  bool first_post;			// variable for show_status
  bool just_paused;			// variable for update_and_pause

  bool extend_outfile_size();
  int copy_block( const Block & b, int & copied_size, int & error_size );
  void count_errors();
  bool errors_or_timeout()
    { if( max_errors >= 0 && errors > max_errors ) e_code |= 2;
      return ( e_code != 0 ); }
  void reduce_min_read_rate()
    { if( min_read_rate > 0 ) min_read_rate /= 10; }
  bool slow_read() const
    { return ( t1 - t0 >= 30 &&		// no slow reads for first 30s
               ( ( min_read_rate > 0 && c_rate < min_read_rate &&
                   c_rate < a_rate / 2 ) ||
                 ( min_read_rate == 0 && c_rate < a_rate / 10 ) ) ); }
  int update( const Block & b, const Sblock::Status st,
              const int copied_size, const int error_size );
  int copy_and_update( const Block & b, int & error_size,
                       const char * const msg, const bool forward );
  int copy_and_update2( const Block & b, int & copied_size,
                        int & error_size, const char * const msg,
                        const bool forward );
  bool reopen_infile();
  bool update_and_pause();
  int copy_non_tried();
  int fcopy_non_tried( const char * const msg, const int pass );
  int rcopy_non_tried( const char * const msg, const int pass );
  int trim_errors();
  int scrape_errors();
  int copy_errors();
  int fcopy_errors( const char * const msg, const int retry );
  int rcopy_errors( const char * const msg, const int retry );
  void update_rates( const bool force = false );
  void show_status( const long long ipos, const char * const msg = 0,
                    const bool force = false );
public:
  Rescuebook( const long long offset, const long long isize,
              Domain & dom, const Domain * const test_dom,
              const Rb_options & rb_opts, const char * const iname,
              const char * const logname, const int cluster,
              const int hardbs, const bool synchronous );

  int do_rescue( const int ides, const int odes );
  };


// Round "size" to the next multiple of sector size (hardbs).
//
inline int round_up( int size, const int hardbs )
  {
  if( size % hardbs )
    {
    size -= size % hardbs;
    if( INT_MAX - size >= hardbs ) size += hardbs;
    }
  return size;
  }


// Defined in io.cc
//
const char * format_time( long t );
bool interrupted();
void set_signals();
int signaled_exit();
