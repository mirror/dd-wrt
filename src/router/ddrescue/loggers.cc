/* GNU ddrescue - Data recovery tool
   Copyright (C) 2013-2023 Antonio Diaz Diaz.

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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "block.h"
#include "loggers.h"


namespace {

const char * format_time_dhms( const long long t )
  {
  static char buf[64];			// keep gcc quiet
  const int s = t % 60;
  const int m = ( t / 60 ) % 60;
  const int h = ( t / 3600 ) % 24;
  const long long d = t / 86400;

  if( d ) snprintf( buf, sizeof buf, "%lldd:%02dh:%02dm:%02ds", d, h, m, s );
  else if( h ) snprintf( buf, sizeof buf, "%dh:%02dm:%02ds", h, m, s );
  else if( m ) snprintf( buf, sizeof buf, "%dm:%02ds", m, s );
  else snprintf( buf, sizeof buf, "%ds", s );
  return buf;
  }

} // end namespace


Event_logger event_logger;
Rate_logger rate_logger;
Read_logger read_logger;


void Logger::set_filename( const char * const name )
  {
  if( !name || !name[0] ) return;			// ignore name
  if( std::strcmp( name, "-" ) == 0 )
    { show_error( "I won't write log data to standard output." );
      std::exit( 1 ); }
  struct stat st;
  if( stat( name, &st ) == 0 && !S_ISREG( st.st_mode ) )
    { show_file_error( name, "Logfile exists and is not a regular file." );
      std::exit( 1 ); }
  filename_ = name;
  }


bool Logger::close_file()
  {
  if( f && !error && !write_final_timestamp( f ) ) error = true;
  if( f && std::fclose( f ) != 0 ) error = true;	// close even on error
  f = 0;
  return !error;
  }


bool Event_logger::open_file()
  {
  if( !filename_ ) return true;
  if( !f )
    {
    struct stat st;
    const bool file_exists = ( stat( filename_, &st ) == 0 );
    f = std::fopen( filename_, "a" );
    error = !f || ( file_exists && std::fputc( '\n', f ) == EOF ) ||
            !write_file_header( f, "Events Logfile" ) ||
            std::fputs( "#         Time  Rescued  Event\n", f ) == EOF;
    }
  return !error;
  }


bool Event_logger::echo_msg( const char * const msg )
  {
  if( verbosity >= 0 ) std::printf( "\n  %s", msg );
  if( f && !error && std::fprintf( f, "                      %s\n", msg ) < 0 )
    error = true;
  return !error;
  }


bool Event_logger::print_msg( const long long time,
                              const char * const percent_rescued,
                              const char * const msg )
  {
  if( f && !error &&
      std::fprintf( f, "%14s  %s  %s\n", format_time_dhms( time ),
                    percent_rescued, msg ) < 0 )
    error = true;
  return !error;
  }


bool Event_logger::print_eor( const long long time,
                              const char * const percent_rescued,
                              const long long current_pos,
                              const char * const current_status_name )
  {
  if( f && !error &&
      std::fprintf( f, "%14s  %s  End of run (0x%08llX  %s)\n",
                    format_time_dhms( time ), percent_rescued,
                    current_pos, current_status_name ) < 0 )
    error = true;
  return !error;
  }


bool Rate_logger::open_file()
  {
  if( !filename_ ) return true;
  if( !f )
    {
    last_time = -1;
    f = std::fopen( filename_, "w" );
    error = !f || !write_file_header( f, "Rates Logfile" ) ||
            std::fputs( "#Time  Ipos  Current_rate  Average_rate  Bad_areas  Bad_size\n", f ) == EOF;
    }
  return !error;
  }


bool Rate_logger::print_line( const long long time, const long long ipos,
                              const long long a_rate, const long long c_rate,
                              const unsigned long bad_areas,
                              const long long bad_size )
  {
  if( f && !error && time > last_time )
    {
    last_time = time;
    if( std::fprintf( f, "%2lld  0x%08llX  %8lld  %8lld  %7lu  %8lld\n",
                      time, ipos, c_rate, a_rate, bad_areas, bad_size ) < 0 )
      error = true;
    }
  return !error;
  }


bool Read_logger::open_file()
  {
  if( !filename_ ) return true;
  if( !f )
    {
    prev_is_msg = true;
    f = std::fopen( filename_, "w" );
    error = !f || !write_file_header( f, "Reads Logfile" ) ||
            std::fputs( "#  Ipos       Size  Copied_size  Error_size\n", f ) == EOF;
    }
  return !error;
  }


bool Read_logger::print_line( const long long ipos, const long long size,
                              const int copied_size, const int error_size )
  {
  if( f && !error &&
      std::fprintf( f, "0x%08llX	%lld	%d	%d\n",
                    ipos, size, copied_size, error_size ) < 0 )
    error = true;
  prev_is_msg = false;
  return !error;
  }


bool Read_logger::print_msg( const long long time, const char * const msg )
  {
  if( f && !error &&
      std::fprintf( f, "%s# %s  %s\n", prev_is_msg ? "" : "\n",
                    format_time_dhms( time ), msg ) < 0 )
    error = true;
  prev_is_msg = true;
  return !error;
  }


bool Read_logger::print_time( const long long time )
  {
  if( f && !error && time > 0 &&
      std::fprintf( f, "# %s\n", format_time_dhms( time ) ) < 0 )
    error = true;
  return !error;
  }
