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
#include <ctime>
#include <string>
#include <vector>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

#include "rational.h"
#include "block.h"
#include "loggers.h"
#include "mapbook.h"
#include "rescuebook.h"


namespace {

// Round "size" to the next multiple of sector size (hardbs).
//
long long round_up( long long size, const int hardbs )
  {
  if( size % hardbs )
    {
    size -= size % hardbs;
    if( LLONG_MAX - size >= hardbs ) size += hardbs;
    }
  return size;
  }

} // end namespace


void Rescuebook::change_chunk_status( const Block & b, const Sblock::Status st )
  {
  Sblock::Status old_st = st;
  bad_areas += Mapfile::change_chunk_status( b, st, domain(), &old_st );
  if( st == old_st ) return;
  switch( old_st )
    {
    case Sblock::non_tried:     non_tried_size -= b.size(); break;
    case Sblock::non_trimmed: non_trimmed_size -= b.size(); break;
    case Sblock::non_scraped: non_scraped_size -= b.size(); break;
    case Sblock::bad_sector:          bad_size -= b.size(); break;
    case Sblock::finished:       finished_size -= b.size(); break;
    }
  switch( st )
    {
    case Sblock::non_tried:     non_tried_size += b.size(); break;
    case Sblock::non_trimmed: non_trimmed_size += b.size(); break;
    case Sblock::non_scraped: non_scraped_size += b.size(); break;
    case Sblock::bad_sector:          bad_size += b.size(); break;
    case Sblock::finished:       finished_size += b.size(); break;
    }
  }


void Rescuebook::do_pause_on_error()
  {
  if( simulated_poe ) tp += pause_on_error;
  else if( pause_on_error >= 1 ) sleep( pause_on_error.trunc() );
  }


bool Rescuebook::extend_outfile_size()
  {
  if( min_outfile_size > 0 || sparse_size > 0 )
    {
    const long long min_size = std::max( min_outfile_size, sparse_size );
    const long long size = lseek( odes_, 0, SEEK_END );
    if( size < 0 ) return false;
    if( min_size > size )
      {
      int ret;
      do ret = ftruncate( odes_, min_size );
        while( ret != 0 && errno == EINTR );
      if( ret != 0 || lseek( odes_, 0, SEEK_END ) != min_size )
        {
        const uint8_t zero = 0;		// if ftruncate fails, write a zero
        if( writeblockp( odes_, &zero, 1, min_size - 1 ) != 1 ) return false;
        }
      fsync( odes_ );
      }
    }
  return true;
  }


/* Return values: 1 fatal error, 0 OK (I/O errors are ignored).
   If OK && copied_size + error_size < b.size(), it means EOF has been reached.
*/
int Rescuebook::copy_block( const Block & b, int & copied_size, int & error_size )
  {
  if( b.size() <= 0 ) internal_error( "bad size copying a Block." );
  if( !test_domain || test_domain->includes( b ) )
    {
    if( o_direct_in )
      {
      const int pre = b.pos() % hardbs();
      const int disp = b.end() % hardbs();
      const int post = ( disp > 0 ) ? hardbs() - disp : 0;
      const int size = pre + b.size() + post;
      if( size > iobuf_size() )
        internal_error( "(size > iobuf_size) copying a Block." );
      copied_size = readblockp( ides_, iobuf(), size, b.pos() - pre );
      copied_size -= std::min( pre, copied_size );
      if( copied_size > b.size() ) copied_size = b.size();
      if( pre > 0 && copied_size > 0 )
        std::memmove( iobuf(), iobuf() + pre, copied_size );
      }
    else copied_size = readblockp( ides_, iobuf(), b.size(), b.pos() );
    error_size = errno ? b.size() - copied_size : 0;
    if( copied_size <= 0 ) switch( errno )
      {
      case EACCES: case EBADF: case EBUSY: case EISDIR: case ENOBUFS:
      case ENODEV: case ENOENT: case ENOMEM: case ENOSYS: case ENXIO:
      case EPERM: case ESPIPE:
        final_msg( iname_, "Fatal error reading the input file", errno );
        return 1;
      case EINVAL:
        final_msg( iname_, "Unaligned read error. Is sector size correct?" );
        return 1;
      }
    }
  else { copied_size = 0; error_size = b.size(); }

  if( copied_size > 0 )
    {
    iobuf_ipos = b.pos();
    const long long pos = b.pos() + offset();
    if( sparse_size >= 0 && block_is_zero( iobuf(), copied_size ) )
      {
      const long long end = pos + copied_size;
      if( end > sparse_size ) sparse_size = end;
      }
    else
      if( !compare_before_write ||
          readblockp( odes_, iobuf2(), copied_size, pos ) != copied_size ||
          std::memcmp( iobuf(), iobuf2(), copied_size ) != 0 )
        if( writeblockp( odes_, iobuf(), copied_size, pos ) != copied_size ||
              ( synchronous_ && fsync( odes_ ) != 0 && errno != EINVAL ) )
          { final_msg( oname_, "Write error", errno ); return 1; }
    }
  else iobuf_ipos = -1;

  read_logger.print_line( b.pos(), b.size(), copied_size, error_size );

  if( verify_on_error )
    {
    if( copied_size >= hardbs() && b.pos() % hardbs() == 0 )
      { voe_ipos = b.pos(); std::memcpy( voe_buf, iobuf(), hardbs() ); }
    if( error_size > 0 )
      {
      if( voe_ipos >= 0 )
        {
        const int size = readblockp( ides_, iobuf2(), hardbs(), voe_ipos );
        if( size != hardbs() )
          { final_msg( iname_, "Input file no longer returns data", errno );
            e_code |= 8; }
        else if( std::memcmp( voe_buf, iobuf2(), hardbs() ) != 0 )
          { final_msg( iname_, "Input file returns inconsistent data." );
            e_code |= 8; }
        }
      else
        { final_msg( iname_, "Read error found before the first good read." );
          e_code |= 8; }
      }
    }
  return 0;
  }


void Rescuebook::initialize_sizes()
  {
  bool good = true;
  non_tried_size = non_trimmed_size = non_scraped_size = 0;
  bad_size = finished_size = 0;
  bad_areas = 0;

  for( long i = 0; i < sblocks(); ++i )
    {
    const Sblock & sb = sblock( i );
    if( !domain().includes( sb ) )
      { if( domain() < sb ) break; else { good = true; continue; } }
    switch( sb.status() )
      {
      case Sblock::non_tried:     non_tried_size += sb.size(); good = true; break;
      case Sblock::non_trimmed: non_trimmed_size += sb.size(); good = true; break;
      case Sblock::non_scraped: non_scraped_size += sb.size(); good = true; break;
      case Sblock::bad_sector:          bad_size += sb.size();
        if( good ) { good = false; ++bad_areas; } break;
      case Sblock::finished:       finished_size += sb.size(); good = true; break;
      }
    }
  }


// Return values: 1 error, 0 OK, -1 interrupted.
//
int Rescuebook::copy_and_update( const Block & b, int & copied_size,
                                 int & error_size, const char * const msg,
                                 const Status curr_st, const int curr_pass,
                                 const bool forward, const Sblock::Status st )
  {
  if( first_post )
    {
    if( !first_read && pause_on_pass > 0 )
      {
      show_status( -1, "Paused", true );
      sleep( pause_on_pass );
      const long long t2 = std::time( 0 );
      if( t1 < t2 ) t1 = t2;			// clock may have jumped back
      ts = std::min( ts + pause_on_pass, t2 );	// avoid spurious timeout
      }
    current_status( curr_st, msg );
    current_pass( curr_pass );
    event_logger.print_msg( t1 - t0, percent_rescued(), msg );
    read_logger.print_msg( t1 - t0, msg );
    }
  current_pos( forward ? b.pos() : b.end() );
  show_status( b.pos(), msg );
  if( errors_or_timeout() ) return 1;
  if( interrupted() ) return -1;
  int retval = copy_block( b, copied_size, error_size );
  if( retval == 0 )
    {
    if( copied_size + error_size < b.size() )			// EOF
      {
      if( complete_only ) truncate_domain( b.pos() + copied_size + error_size );
      else if( !truncate_vector( b.pos() + copied_size + error_size ) )
        { final_msg( iname_, "EOF found below the size calculated from mapfile." );
          retval = 1; }
      initialize_sizes();
      }
    if( copied_size > 0 )
      change_chunk_status( Block( b.pos(), copied_size ), Sblock::finished );
    if( error_size > 0 )
      {
      error_sum += error_size;
      ++read_errors;
      if( read_errors > max_read_errors ) { e_code |= 16; retval = 1; }
      const Sblock::Status st2 =
        ( error_size > hardbs() ) ? st : Sblock::bad_sector;
      change_chunk_status( Block( b.pos() + copied_size, error_size ), st2 );
      struct stat istat;
      if( stat( iname_, &istat ) != 0 )
        { final_msg( iname_, "Input file disappeared", errno ); retval = 1; }
      }
    }
  return retval;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Read the non-tried part of the domain, skipping over the damaged areas.
*/
int Rescuebook::copy_non_tried()
  {
  char msgbuf[80] = "Copying non-tried blocks... Pass ";
  const int msglen = std::strlen( msgbuf );
  const bool cpass_given = ( cpass_bitset != 31 );
  bool resume = ( !cpass_given && current_status() == copying );
  if( !resume || current_pass() > 5 ) current_pass( 1 );	// reset pass
  const int first_pass = current_pass();
  bool forward = !reverse;

  for( int pass = 1; pass <= 5; ++pass )
    {
    if( pass >= first_pass && cpass_bitset & ( 1 << ( pass - 1 ) ) )
      {
      if( pass != first_pass ) resume = false;
      first_post = true;
      snprintf( msgbuf + msglen, ( sizeof msgbuf ) - msglen, "%d %s",
                pass, forward ? "(forwards)" : "(backwards)" );
      int retval = forward ? fcopy_non_tried( msgbuf, pass, resume ) :
                             rcopy_non_tried( msgbuf, pass, resume );
      if( retval != -3 ) return retval;
      }
    if( pass >= 2 && min_read_rate >= 0 ) min_read_rate = -1;	// reset rate
    if( !unidirectional ) forward = !forward;
    }
  return 0;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Read forwards the non-tried part of the domain, skipping over the
   damaged areas.
*/
int Rescuebook::fcopy_non_tried( const char * const msg, const int pass,
                                 const bool resume )
  {
  long long pos = 0;
  long long eskip_size = skipbs;	// size to skip on error if skipbs > 0
  long long sskip_size = 0;		// size to skip on slow if skipbs > 0
  const bool after_finished = (pass == 3 || pass == 4 );
  bool block_found = false;
  bool block_processed = false;

  if( resume && domain().includes( current_pos() ) )
    {
    Block b( current_pos(), 1 );
    find_chunk( b, Sblock::non_tried, domain(), hardbs() );
    if( b.size() > 0 ) pos = b.pos();		// resume
    }

  while( pos >= 0 )
    {
    Block b( pos, softbs() );
    if( find_chunk( b, Sblock::non_tried, domain(), softbs(), after_finished ) )
      block_found = true;
    if( b.size() <= 0 ) break;
    block_processed = true;
    if( pos != b.pos() )		// reset size on block change
      { eskip_size = skipbs; current_slow = false; }
    pos = b.end();
    int copied_size = 0, error_size = 0;
    const int retval = copy_and_update( b, copied_size, error_size, msg,
                                        copying, pass, true, Sblock::non_trimmed );
    if( retval ) return retval;
    const bool slow = update_rates();
    if( slow )
      { ++slow_reads;
        if( slow_reads > max_slow_reads ) { e_code |= 32; return 1; } }
    if( ( error_size > 0 || ( slow && pass <= 2 ) ) && pos >= 0 )
      {
      if( reopen_on_error && !reopen_infile() ) return 1;
      if( pause_on_error > 0 ) do_pause_on_error();
      if( skipbs > 0 && pass <= 4 )		// don't skip if skipbs == 0
        {
        b.pos( pos );
        if( pass >= 2 )	// skip rest of block at first error or slow read
          b.size( -1 );
        else if( error_size > 0 )
          {
          b.size( eskip_size );
          if( eskip_size <= max_skipbs / 2 ) eskip_size *= 2;
          else eskip_size = max_skipbs;
          }
        else				// slow read on pass 1
          {
          if( !prev_slow )
            sskip_size = std::max( skipbs, std::min( c_rate, max_skipbs ) );
          else if( sskip_size <= max_skipbs / 2 ) sskip_size *= 2;
          else sskip_size = max_skipbs;
          b.size( sskip_size );
          }
        find_chunk( b, Sblock::non_tried, domain(), hardbs() );
        if( pos == b.pos() && b.size() > 0 ) pos = b.end();	// skip
        }
      }
    else if( error_size == 0 && copied_size > 0 ) eskip_size = skipbs;	// reset
    if( !update_mapfile( odes_ ) ) return -2;
    }
  if( !block_found ) return 0;
  if( block_processed ) show_status( -1, msg, true );	// update at end of pass
  return -3;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Read backwards the non-tried part of the domain, skipping over the
   damaged areas.
*/
int Rescuebook::rcopy_non_tried( const char * const msg, const int pass,
                                 const bool resume )
  {
  long long end = LLONG_MAX;
  long long eskip_size = skipbs;	// size to skip on error if skipbs > 0
  long long sskip_size = 0;		// size to skip on slow if skipbs > 0
  const bool before_finished = (pass == 3 || pass == 4 );
  bool block_found = false;
  bool block_processed = false;

  if( resume && domain().includes( current_pos() - 1 ) )
    {
    Block b( current_pos() - 1, 1 );
    rfind_chunk( b, Sblock::non_tried, domain(), hardbs() );
    if( b.size() > 0 ) end = b.end();		// resume
    }

  while( end > 0 )
    {
    Block b( end - softbs(), softbs() );
    if( rfind_chunk( b, Sblock::non_tried, domain(), softbs(), before_finished ) )
      block_found = true;
    if( b.size() <= 0 ) break;
    block_processed = true;
    if( end != b.end() )		// reset size on block change
      { eskip_size = skipbs; current_slow = false; }
    end = b.pos();
    int copied_size = 0, error_size = 0;
    const int retval = copy_and_update( b, copied_size, error_size, msg,
                                        copying, pass, false, Sblock::non_trimmed );
    if( retval ) return retval;
    const bool slow = update_rates();
    if( slow )
      { ++slow_reads;
        if( slow_reads > max_slow_reads ) { e_code |= 32; return 1; } }
    if( ( error_size > 0 || ( slow && pass <= 2 ) ) && end > 0 )
      {
      if( reopen_on_error && !reopen_infile() ) return 1;
      if( pause_on_error > 0 ) do_pause_on_error();
      if( skipbs > 0 && pass <= 4 )		// don't skip if skipbs == 0
        {
        if( pass >= 2 )	// skip rest of block at first error or slow read
          b.assign( 0, end );
        else if( error_size > 0 )
          {
          b.assign( end - eskip_size, eskip_size );
          if( eskip_size <= max_skipbs / 2 ) eskip_size *= 2;
          else eskip_size = max_skipbs;
          }
        else				// slow read on pass 1
          {
          if( !prev_slow )
            sskip_size = std::max( skipbs, std::min( c_rate, max_skipbs ) );
          else if( sskip_size <= max_skipbs / 2 ) sskip_size *= 2;
          else sskip_size = max_skipbs;
          b.assign( end - sskip_size, sskip_size );
          }
        rfind_chunk( b, Sblock::non_tried, domain(), hardbs() );
        if( end == b.end() && b.size() > 0 ) end = b.pos();	// skip
        }
      }
    else if( error_size == 0 && copied_size > 0 ) eskip_size = skipbs;	// reset
    if( !update_mapfile( odes_ ) ) return -2;
    }
  if( !block_found ) return 0;
  if( block_processed ) show_status( -1, msg, true );	// update at end of pass
  return -3;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Trim both edges of each damaged area sequentially. If any edge is
   adjacent to a bad sector, leave it for the scraping phase.
*/
int Rescuebook::trim_errors()
  {
  const char * const msg = reverse ? "Trimming failed blocks... (backwards)" :
                                     "Trimming failed blocks... (forwards)";
  first_post = true;

  for( long i = 0; i < sblocks(); )
    {
    const long idx = reverse ? sblocks() - 1 - i : i;
    const Sblock sb( sblock( idx ) );
    if( !domain().includes( sb ) )
      { if( ( !reverse && domain() < sb ) || ( reverse && domain() > sb ) )
          break;
        ++i; continue; }
    if( sb.status() != Sblock::non_trimmed ) { ++i; continue; }
    const bool lbad = ( idx > 0 &&
                        sblock( idx - 1 ).status() == Sblock::bad_sector );
    const bool rbad = ( idx + 1 < sblocks() &&
                        sblock( idx + 1 ).status() == Sblock::bad_sector );
    if( lbad && rbad )		// leave block for the scraping phase
      { change_sblock_status( idx, Sblock::non_scraped ); ++i; continue; }
    bool error_found = lbad;
    long long pos = sb.pos();
    long long end = sb.end();
    while( pos < end && !error_found )		// trim leading edge
      {
      Block b( pos, std::min( (long long)hardbs(), end - pos ) );
      if( b.end() != end ) b.align_end( hardbs() );
      pos = b.end();
      int copied_size = 0, error_size = 0;
      const int retval = copy_and_update( b, copied_size, error_size, msg,
                                          trimming, 1, true );
      if( retval ) return retval;
      update_rates();
      if( error_size > 0 )
        { error_found = true; if( pause_on_error > 0 ) do_pause_on_error(); }
      if( !update_mapfile( odes_ ) ) return -2;
      }
    error_found = rbad;
    while( pos < end && !error_found )		// trim trailing edge
      {
      const int size = std::min( (long long)hardbs(), end - pos );
      Block b( end - size, size );
      if( b.pos() != pos ) b.align_pos( hardbs() );
      end = b.pos();
      int copied_size = 0, error_size = 0;
      const int retval = copy_and_update( b, copied_size, error_size, msg,
                                          trimming, 1, false );
      if( retval ) return retval;
      update_rates();
      if( error_size > 0 )
        { error_found = true; if( pause_on_error > 0 ) do_pause_on_error(); }
      if( !update_mapfile( odes_ ) ) return -2;
      }
    if( pos < end )
      {
      const long index = find_index( end - 1 );
      if( index >= 0 && domain().includes( sblock( index ) ) &&
          sblock( index ).status() == Sblock::non_trimmed )
        change_chunk_status( sblock( index ), Sblock::non_scraped );
      }
    }
  show_status( -1, msg, true );			// update at end of pass
  return 0;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Scrape the damaged areas sequentially.
*/
int Rescuebook::scrape_errors()
  {
  const char * const msg = reverse ? "Scraping failed blocks... (backwards)" :
                                     "Scraping failed blocks... (forwards)";
  first_post = true;

  for( long i = 0; i < sblocks(); )
    {
    const Sblock sb( sblock( reverse ? sblocks() - 1 - i : i ) );
    if( !domain().includes( sb ) )
      { if( ( !reverse && domain() < sb ) || ( reverse && domain() > sb ) )
          break;
        ++i; continue; }
    if( sb.status() != Sblock::non_scraped ) { ++i; continue; }
    long long pos = sb.pos();
    const long long end = sb.end();
    while( pos < end )
      {
      Block b( pos, std::min( (long long)hardbs(), end - pos ) );
      if( b.end() != end ) b.align_end( hardbs() );
      pos = b.end();
      int copied_size = 0, error_size = 0;
      const int retval = copy_and_update( b, copied_size, error_size, msg,
                                          scraping, 1, true );
      if( retval ) return retval;
      update_rates();
      if( error_size > 0 && pause_on_error > 0 ) do_pause_on_error();
      if( !update_mapfile( odes_ ) ) return -2;
      }
    }
  show_status( -1, msg, true );			// update at end of pass
  return 0;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Try to read the damaged areas, one sector at a time.
*/
int Rescuebook::copy_errors()
  {
  char msgbuf[80] = "Retrying bad sectors... Retry ";
  const int msglen = std::strlen( msgbuf );
  bool resume = ( current_status() == retrying );
  const int first_pass =
    ( !resume || unidirectional || current_pass() & 1 ) ? 1 : 2;  // odd:even
  bool forward = !reverse;
  if( !unidirectional && first_pass == 2 ) forward = !forward;

  for( int pass = first_pass;
       max_retries < 0 || pass - first_pass + 1 <= max_retries; ++pass )
    {
    first_post = true;
    snprintf( msgbuf + msglen, ( sizeof msgbuf ) - msglen, "%d %s",
              pass - first_pass + 1, forward ? "(forwards)" : "(backwards)" );
    int retval = forward ? fcopy_errors( msgbuf, pass, resume ) :
                           rcopy_errors( msgbuf, pass, resume );
    if( retval != -3 ) return retval;
    resume = false;
    if( !unidirectional ) forward = !forward;
    if( pass >= INT_MAX / 2 ) break;
    }
  return 0;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Try to read forwards the damaged areas, one sector at a time.
*/
int Rescuebook::fcopy_errors( const char * const msg, const int pass,
                              const bool resume )
  {
  long long pos = 0;
  bool block_found = false;

  if( resume && domain().includes( current_pos() ) )
    {
    Block b( current_pos(), 1 );
    find_chunk( b, Sblock::bad_sector, domain(), hardbs() );
    if( b.size() > 0 ) pos = b.pos();		// resume
    }

  while( pos >= 0 )
    {
    Block b( pos, hardbs() );
    find_chunk( b, Sblock::bad_sector, domain(), hardbs() );
    if( b.size() <= 0 ) break;			// no more blocks
    pos = b.end();
    block_found = true;
    int copied_size = 0, error_size = 0;
    const int retval = copy_and_update( b, copied_size, error_size, msg,
                                        retrying, pass, true );
    if( retval ) return retval;
    update_rates();
    if( error_size > 0 && pause_on_error > 0 ) do_pause_on_error();
    if( !update_mapfile( odes_ ) ) return -2;
    }
  if( !block_found ) return 0;
  show_status( -1, msg, true );			// update at end of pass
  return -3;
  }


/* Return values: 1 error, 0 OK, -1 interrupted, -2 mapfile error.
   Try to read backwards the damaged areas, one sector at a time.
*/
int Rescuebook::rcopy_errors( const char * const msg, const int pass,
                              const bool resume )
  {
  long long end = LLONG_MAX;
  bool block_found = false;

  if( resume && domain().includes( current_pos() - 1 ) )
    {
    Block b( current_pos() - 1, 1 );
    rfind_chunk( b, Sblock::bad_sector, domain(), hardbs() );
    if( b.size() > 0 ) end = b.end();		// resume
    }

  while( end > 0 )
    {
    Block b( end - hardbs(), hardbs() );
    rfind_chunk( b, Sblock::bad_sector, domain(), hardbs() );
    if( b.size() <= 0 ) break;			// no more blocks
    end = b.pos();
    block_found = true;
    int copied_size = 0, error_size = 0;
    const int retval = copy_and_update( b, copied_size, error_size, msg,
                                        retrying, pass, false );
    if( retval ) return retval;
    update_rates();
    if( error_size > 0 && pause_on_error > 0 ) do_pause_on_error();
    if( !update_mapfile( odes_ ) ) return -2;
    }
  if( !block_found ) return 0;
  show_status( -1, msg, true );			// update at end of pass
  return -3;
  }


// Return true if slow read.
//
bool Rescuebook::update_rates( const bool force )
  {
  if( t0 == 0 )
    {
    t0 = t1 = ts = initial_time();
    first_size = last_size = finished_size;
    rates_updated = true;
    if( verbosity >= 0 )
      {
      std::fputs( "\n\n\n\n\n\n", stdout );
      if( preview_lines > 0 )
        for( int i = -2; i < preview_lines; ++i ) std::fputc( '\n', stdout );
      }
    }

  long long t2 = std::time( 0 );
  if( max_read_rate > 0 && finished_size - last_size > max_read_rate && t2 == t1 )
    { sleep( 1 ); t2 = std::time( 0 ); }
  if( t2 < t1 )					// clock jumped back
    {
    const long long delta = std::min( t0 - 1, t1 - t2 );
    t0 -= delta;
    ts -= delta;
    t1 = t2;
    }
  const bool force_update = ( force && t2 <= t1 );
  if( force_update ) t2 = t1 + 1;		// force update of e_code
  if( t2 > t1 )
    {
    if( tp > 0 )
      {
      const long long delta = std::min( t0 - 1, (long long)tp.round() );
      t0 -= delta;
      t1 -= delta;
      ts -= delta;
      tp = 0;
      }
    a_rate = ( finished_size - first_size ) / ( t2 - t0 );
    c_rate = ( finished_size - last_size ) / ( t2 - t1 );
    if( !( e_code & 4 ) )
      {
      if( finished_size != last_size ) { last_size = finished_size; ts = t2; }
      else if( !force_update && timeout >= 0 && t2 - ts > timeout && t1 > t0 )
        e_code |= 4;
      }
    if( !( e_code & 1 ) )
      {
      error_rate = error_sum / ( t2 - t1 );
      error_sum = 0;
      if( max_error_rate >= 0 && error_rate > max_error_rate ) e_code |= 1;
      }
    rates_updated = true;
    if( !force_update )
      {
      t1 = t2;
      prev_slow = current_slow;
      current_slow = ( t1 - t0 > delay_slow &&	// delay checking slow reads
                       ( ( min_read_rate > 0 && c_rate < min_read_rate ) ||
                         ( min_read_rate == 0 && c_rate < a_rate / 10 ) ) );
      if( !current_slow && reset_slow ) slow_reads = 0;
      return current_slow;
      }
    }
  return false;
  }


void Rescuebook::show_status( const long long ipos, const char * const msg,
                              const bool force )
  {
  const char * const up = "\x1B[A";

  if( ipos >= 0 ) last_ipos = ipos;
  if( rates_updated || force || first_post )
    {
    if( verbosity >= 0 )
      {
      if( first_post && !first_read && !rescue_finished() )
        std::fputc( '\n', stdout );	// scroll forward after each pass
      else std::printf( "\r%s%s%s%s%s%s", up, up, up, up, up, up );
      if( preview_lines > 0 )
        {
        if( !first_post || first_read )
          for( int i = -2; i < preview_lines; ++i ) std::fputs( up, stdout );
        std::fputs( "Data preview:\n", stdout );
        for( int i = 0; i < preview_lines; ++i )
          {
          if( iobuf_ipos >= 0 )
            {
            const uint8_t * const p = iobuf() + ( 16 * i );
            std::printf( "%010llX ", ( iobuf_ipos + ( 16 * i ) ) & 0xFFFFFFFFFFLL );
            for( int j = 0; j < 16; ++j )
              { std::printf( " %02X", p[j] );
                if( j == 7 ) std::fputc( ' ', stdout ); }
            std::fputs( "  ", stdout );
            for( int j = 0; j < 16; ++j )
              std::fputc( std::isprint( p[j] ) ? p[j] : '.', stdout );
            std::fputc( '\n', stdout );
            }
          else if( i == ( preview_lines - 1 ) / 2 )
            std::fputs( "                            No data available                                 \n", stdout );
          else
            std::fputs( "                                                                              \n", stdout );
          }
        std::fputc( '\n', stdout );
        }
      std::printf( "     ipos: %9sB, non-trimmed: %9sB,  current rate: %8sB/s\n",
                   format_num( last_ipos ), format_num( non_trimmed_size ),
                   format_num( c_rate, 99999 ) );
      std::printf( "     opos: %9sB, non-scraped: %9sB,  average rate: %8sB/s\n",
                   format_num( last_ipos + offset() ),
                   format_num( non_scraped_size ), format_num( a_rate, 99999 ) );
      std::printf( "non-tried: %9sB,  bad-sector: %9sB,    error rate: %8sB/s\n",
                   format_num( non_tried_size ), format_num( bad_size ),
                   format_num( error_rate, 99999 ) );
      std::printf( "  rescued: %9sB,   bad areas: %8lu,        run time: %11s\n",
                   format_num( finished_size ), bad_areas,
                   format_time( t1 - t0 ) );
      if( first_post ) sliding_avg.reset();
      else sliding_avg.add_term( c_rate );
      const long long s_rate = domain().full() ? 0 : sliding_avg();
      const long remaining_time = ( s_rate <= 0 ) ? -1 :
        std::min( std::min( (long long)LONG_MAX, 315359999968464000LL ),
                  ( non_tried_size + non_trimmed_size + non_scraped_size +
                    ( max_retries ? bad_size : 0 ) + s_rate - 1 ) / s_rate );
      std::printf( "pct rescued:  %s, read errors:%9lu,  remaining time: %11s\n",
                   percent_rescued(), read_errors,
                   format_time( remaining_time, remaining_time >= 180 ) );
      if( min_read_rate >= -1 ) std::printf( " slow reads:%9lu,", slow_reads );
      else std::fputs( "                      ", stdout );
      std::printf( "        time since last successful read: %11s\n",
                   format_time( ( ts > t0 ) ? t1 - ts : -1 ) );
      if( msg && msg[0] && !errors_or_timeout() )
        {
        const int len = std::strlen( msg ); std::printf( "\r%s", msg );
        for( int i = len; i < oldlen; ++i ) std::fputc( ' ', stdout );
        oldlen = len;
        }
      safe_fflush( stdout );
      }
    rate_logger.print_line( t1 - t0, last_ipos, a_rate, c_rate, bad_areas,
                            bad_size );
    if( !force && !first_post ) read_logger.print_time( t1 - t0 );
    rates_updated = false;
    first_post = false;
    first_read = false;
    }
  }


Rescuebook::Rescuebook( const long long offset, const long long insize,
                        Domain & dom, const Domain * const test_dom,
                        const Mb_options & mb_opts, const Rb_options & rb_opts,
                        const char * const iname, const char * const oname,
                        const char * const mapname, const int cluster,
                        const int hardbs, const bool synchronous )
  : Mapbook( offset, insize, dom, mb_opts, mapname, cluster, hardbs,
             rb_opts.complete_only, true ),
    Rb_options( rb_opts ),
    error_rate( 0 ),
    error_sum( 0 ),
    sparse_size( sparse ? 0 : -1 ),
    non_tried_size( 0 ),
    non_trimmed_size( 0 ),
    non_scraped_size( 0 ),
    bad_size( 0 ),
    finished_size( 0 ),
    test_domain( test_dom ),
    iname_( iname ), oname_( oname ),
    read_errors( 0 ),
    slow_reads( 0 ),
    e_code( 0 ),
    synchronous_( synchronous ),
    voe_ipos( -1 ), voe_buf( new uint8_t[hardbs] ),
    a_rate( 0 ), c_rate( 0 ), first_size( 0 ), last_size( 0 ),
    iobuf_ipos( -1 ), last_ipos( 0 ), t0( 0 ), t1( 0 ), ts( 0 ), tp( 0 ),
    oldlen( 0 ), rates_updated( false ), current_slow( false ),
    prev_slow( false ), sliding_avg( 30 ), first_post( true ),
    first_read( true )
  {
  if( preview_lines > softbs() / 16 ) preview_lines = softbs() / 16;
  if( skipbs < 0 )
    skipbs = round_up( std::max( insize / 100000, (long long)min_skipbs ),
                       min_skipbs );
  const long long csize = insize / 100;
  if( insize > 0 && skipbs > 0 && max_skipbs == max_max_skipbs &&
      csize < max_skipbs )
    max_skipbs = std::max( skipbs, csize );
  skipbs = round_up( skipbs, hardbs );		// make multiple of hardbs
  max_skipbs = round_up( max_skipbs, hardbs );

  if( retrim )
    for( long index = 0; index < sblocks(); ++index )
      {
      const Sblock & sb = sblock( index );
      if( !domain().includes( sb ) )
        { if( domain() < sb ) break; else continue; }
      if( sb.status() == Sblock::non_scraped ||
          sb.status() == Sblock::bad_sector )
        change_sblock_status( index, Sblock::non_trimmed );
      }
  if( try_again )
    for( long index = 0; index < sblocks(); ++index )
      {
      const Sblock & sb = sblock( index );
      if( !domain().includes( sb ) )
        { if( domain() < sb ) break; else continue; }
      if( sb.status() == Sblock::non_scraped ||
          sb.status() == Sblock::non_trimmed )
        change_sblock_status( index, Sblock::non_tried );
      }
  initialize_sizes();				// counts bad_areas
  if( new_bad_areas_only ) max_bad_areas += bad_areas;
  }


// Return values: 0 OK, != 0 error.
//
int Rescuebook::do_rescue( const int ides, const int odes )
  {
  ides_ = ides; odes_ = odes;
  set_signals();
  if( verbosity >= 0 )
    {
    std::fputs( "Press Ctrl-C to interrupt\n", stdout );
    if( mapfile_exists() )
      {
      std::fputs( "Initial status (read from mapfile)\n", stdout );
      if( verbosity >= 3 )
        {
        std::printf( "current position: %9sB,     current sector: %7lld\n",
                     format_num( current_pos() ), current_pos() / hardbs() );
        if( sblocks() )
          std::printf( " last block size: %9sB\n",
                       format_num( sblock( sblocks() - 1 ).size() ) );
        }
      if( domain().pos() > 0 || domain().end() < mapfile_insize() )
        std::printf( "(sizes limited to domain from %s B to %s B of %s B)\n",
                     format_num3( domain().pos() ), format_num3( domain().end() ),
                     format_num3( mapfile_insize() ) );
      std::printf( "rescued: %sB, tried: %sB, bad-sector: %sB, bad areas: %lu\n\n",
                   format_num( finished_size ),
                   format_num( non_trimmed_size + non_scraped_size + bad_size ),
                   format_num( bad_size ), bad_areas );
      std::fputs( "Current status\n", stdout );
      }
    }
  int retval = 0;
  update_rates();				// first call
  if( non_tried_size && !errors_or_timeout() )
    retval = copy_non_tried();
  if( retval == 0 && non_trimmed_size && !notrim && !errors_or_timeout() )
    retval = trim_errors();
  if( retval == 0 && non_scraped_size && !noscrape && !errors_or_timeout() )
    retval = scrape_errors();
  if( retval == 0 && bad_size && max_retries != 0 && !errors_or_timeout() )
    retval = copy_errors();
  if( !rates_updated ) update_rates( true );	// force update of e_code
  show_status( -1, retval ? 0 : "\nFinished", true );

  const bool signaled = ( retval == -1 );
  if( signaled ) retval = 0;
  if( retval == 0 && errors_or_timeout() ) retval = 1;
  if( verbosity >= 0 || event_logger.active() )
    {
    if( retval == -2 ) event_logger.echo_msg( "Mapfile error" );
    else if( retval == 0 && signaled )
      event_logger.echo_msg( "Interrupted by user" );
    else
      {
      if( e_code & 1 )
        {
        char buf[80];
        snprintf( buf, sizeof buf, "Too high error rate reading input file (%sB/s)",
                  format_num( error_rate ) );
        event_logger.echo_msg( buf );
        }
      if( e_code & 2 ) event_logger.echo_msg( "Too many bad areas in input file" );
      if( e_code & 4 ) event_logger.echo_msg( "Timeout expired" );
      if( e_code & 16 ) event_logger.echo_msg( "Too many read errors" );
      if( e_code & 32 ) event_logger.echo_msg( "Too many slow reads" );
      }
    if( verbosity >= 0 )
      { std::fputc( '\n', stdout ); safe_fflush( stdout ); }
    }
  if( retval == -2 ) retval = 1;		// mapfile error
  else
    {
    if( retval == 0 && !signaled ) current_status( finished );
    if( !extend_outfile_size() )		// sparse or -x option
      {
      show_file_error( oname_, "Error extending output file size." );
      if( retval == 0 ) retval = 1;
      }
    compact_sblock_vector();
    if( !update_mapfile( odes_, true ) && retval == 0 ) retval = 1;
    }
  if( final_msg().size() ) show_error( final_msg().c_str(), final_errno() );
  if( close( odes_ ) != 0 )
    { show_file_error( oname_, "Error closing outfile", errno );
      if( retval == 0 ) retval = 1; }
  event_logger.print_eor( t1 - t0, percent_rescued(), current_pos(),
                          status_name( current_status() ) );
  if( !event_logger.close_file() )
    show_file_error( event_logger.filename(),
                     "warning: Error closing the events logging file." );
  if( !rate_logger.close_file() )
    show_file_error( rate_logger.filename(),
                     "warning: Error closing the rates logging file." );
  if( !read_logger.close_file() )
    show_file_error( read_logger.filename(),
                     "warning: Error closing the reads logging file." );
  if( retval ) return retval;		// errors have priority over signals
  if( signaled ) return signaled_exit();
  return 0;
  }
