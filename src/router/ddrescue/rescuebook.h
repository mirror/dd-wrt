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

class Sliding_average		// Calculates the average of the last N terms
  {
  unsigned index;		// either index or data.size() contain N
  std::vector<long long> data;

public:
  explicit Sliding_average( const unsigned terms ) : index( terms )
    { data.reserve( terms ); }

  void reset() { if( index < data.size() ) index = data.size(); data.clear(); }

  void add_term( const long long term )
    {
    if( index < data.size() ) data[index++] = term;
    else if( index > data.size() ) data.push_back( term );
    if( index == data.size() ) index = 0;
    }

  long long operator()() const
    {
    long long avg = 0;
    for( unsigned i = 0; i < data.size(); ++i ) avg += data[i];
    if( data.size() ) avg /= data.size();
    return avg;
    }
  };


struct Rb_options
  {
  enum { min_skipbs = 65536 };

  const long long max_max_skipbs;
  long long max_error_rate;
  long long min_outfile_size;
  long long max_read_rate;
  long long min_read_rate;	// -2 = not set, -1 = reset
  long long skipbs;		// initial size to skip on read error
  long long max_skipbs;		// maximum size to skip on read error
  unsigned long max_bad_areas;
  unsigned long max_read_errors;
  unsigned long max_slow_reads;
  int cpass_bitset;		// 1 << ( pass - 1 ) for passes 1 to 5
  int delay_slow;
  int max_retries;
  int o_direct_in;		// O_DIRECT or 0
  Rational pause_on_error;
  int pause_on_pass;
  int preview_lines;		// preview lines to show. 0 = disable
  int timeout;
  bool compare_before_write;
  bool complete_only;
  bool new_bad_areas_only;
  bool noscrape;
  bool notrim;
  bool reopen_on_error;
  bool reset_slow;
  bool retrim;
  bool reverse;
  bool same_file;
  bool simulated_poe;
  bool sparse;
  bool try_again;
  bool unidirectional;
  bool verify_on_error;

  Rb_options()
    : max_max_skipbs( 1LL << 60 ), max_error_rate( -1 ), min_outfile_size( -1 ),
      max_read_rate( 0 ), min_read_rate( -2 ), skipbs( -1 ),
      max_skipbs( max_max_skipbs ), max_bad_areas( ULONG_MAX ),
      max_read_errors( ULONG_MAX ), max_slow_reads( ULONG_MAX ),
      cpass_bitset( 31 ), delay_slow( 30 ), max_retries( 0 ), o_direct_in( 0 ),
      pause_on_error( 0 ), pause_on_pass( 0 ), preview_lines( 0 ),
      timeout( -1 ), compare_before_write( false ), complete_only( false ),
      new_bad_areas_only( false ), noscrape( false ), notrim( false ),
      reopen_on_error( false ), reset_slow( false ), retrim( false ),
      reverse( false ), same_file( false ), simulated_poe( false ),
      sparse( false ), try_again( false ), unidirectional( false ),
      verify_on_error( false )
      {}

  bool operator==( const Rb_options & o ) const
    { return ( max_error_rate == o.max_error_rate &&
               min_outfile_size == o.min_outfile_size &&
               max_read_rate == o.max_read_rate &&
               min_read_rate == o.min_read_rate &&
               skipbs == o.skipbs && max_skipbs == o.max_skipbs &&
               max_bad_areas == o.max_bad_areas &&
               max_read_errors == o.max_read_errors &&
               max_slow_reads == o.max_slow_reads &&
               cpass_bitset == o.cpass_bitset &&
               delay_slow == o.delay_slow &&
               max_retries == o.max_retries &&
               o_direct_in == o.o_direct_in &&
               pause_on_error == o.pause_on_error &&
               pause_on_pass == o.pause_on_pass &&
               preview_lines == o.preview_lines && timeout == o.timeout &&
               compare_before_write == o.compare_before_write &&
               complete_only == o.complete_only &&
               new_bad_areas_only == o.new_bad_areas_only &&
               noscrape == o.noscrape && notrim == o.notrim &&
               reopen_on_error == o.reopen_on_error &&
               reset_slow == o.reset_slow &&
               retrim == o.retrim && reverse == o.reverse &&
               same_file == o.same_file &&
               simulated_poe == o.simulated_poe &&
               sparse == o.sparse && try_again == o.try_again &&
               unidirectional == o.unidirectional &&
               verify_on_error == o.verify_on_error ); }
  bool operator!=( const Rb_options & o ) const
    { return !( *this == o ); }
  };


class Rescuebook : public Mapbook, public Rb_options
  {
  long long error_rate, error_sum;
  long long sparse_size;		// end position of pending writes
  long long non_tried_size, non_trimmed_size, non_scraped_size;
  long long bad_size, finished_size;
  const Domain * const test_domain;	// good/bad map for test mode
  const char * const iname_, * const oname_;
  unsigned long bad_areas;		// bad areas found so far
  unsigned long read_errors, slow_reads;
  int ides_, odes_;			// input and output file descriptors
  int e_code;				// error code for errors_or_timeout
					// 1 rate, 2 bad_areas, 4 timeout,
					// 8 other (explained in final_msg),
					// 16 read_errors, 32 slow_reads
  const bool synchronous_;
  long long voe_ipos;			// pos of last good sector read, or -1
  uint8_t * const voe_buf;		// copy of last good sector read
					// variables for update_rates
  long long a_rate, c_rate, first_size, last_size;
  long long iobuf_ipos;			// last pos read in iobuf, or -1
  long long last_ipos;
  long long t0, t1, ts;			// start, current, last successful
  Rational tp;				// cumulated pause_on_error
  int oldlen;
  bool rates_updated, current_slow, prev_slow;
  Sliding_average sliding_avg;		// variables for show_status
  bool first_post;			// first read in current pass
  bool first_read;			// first read overall

  void change_chunk_status( const Block & b, const Sblock::Status st );
  void do_pause_on_error();
  bool extend_outfile_size();
  int copy_block( const Block & b, int & copied_size, int & error_size );
  void initialize_sizes();
  bool errors_or_timeout()
    { if( bad_areas > max_bad_areas ) e_code |= 2; return ( e_code != 0 ); }
  const char * percent_rescued() const
    { return format_percentage( finished_size, domain().in_size(), 3, 2, false ); }
  bool rescue_finished() const { return finished_size >= domain().in_size(); }
  int copy_and_update( const Block & b, int & copied_size,
                       int & error_size, const char * const msg,
                       const Status curr_st, const int curr_pass,
                       const bool forward,
                       const Sblock::Status st = Sblock::bad_sector );
  bool reopen_infile();
  int copy_non_tried();
  int fcopy_non_tried( const char * const msg, const int pass,
                       const bool resume );
  int rcopy_non_tried( const char * const msg, const int pass,
                       const bool resume );
  int trim_errors();
  int scrape_errors();
  int copy_errors();
  int fcopy_errors( const char * const msg, const int pass, const bool resume );
  int rcopy_errors( const char * const msg, const int pass, const bool resume );
  bool update_rates( const bool force = false );
  void show_status( const long long ipos, const char * const msg = 0,
                    const bool force = false );
  int copy_command( const char * const command );
  int status_command( const char * const command ) const;

public:
  Rescuebook( const long long offset, const long long insize,
              Domain & dom, const Domain * const test_dom,
              const Mb_options & mb_opts, const Rb_options & rb_opts,
              const char * const iname, const char * const oname,
              const char * const mapname, const int cluster,
              const int hardbs, const bool synchronous );
  ~Rescuebook() { delete[] voe_buf; }

  int do_commands( const int ides, const int odes );
  int do_rescue( const int ides, const int odes );
  };
