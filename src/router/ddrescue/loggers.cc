/*  GNU ddrescue - Data recovery tool
    Copyright (C) 2013, 2014 Antonio Diaz Diaz.

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

#include <cstdio>
#include <string>
#include <vector>

#include "block.h"
#include "loggers.h"


namespace {

const char * format_time_dhms( const long t )
  {
  static char buf[32];
  const long s = t % 60;
  const long m = ( t / 60 ) % 60;
  const long h = ( t / 3600 ) % 24;
  const long d = t / 86400;

  if( d ) snprintf( buf, sizeof buf, "%lud:%02luh:%02lum:%02lus", d, h, m, s );
  else if( h ) snprintf( buf, sizeof buf, "%luh:%02lum:%02lus", h, m, s );
  else if( m ) snprintf( buf, sizeof buf, "%lum:%02lus", m, s );
  else snprintf( buf, sizeof buf, "%lus", s );
  return buf;
  }

} // end namespace


Rate_logger rate_logger;
Read_logger read_logger;


bool Logger::close_file()
  {
  if( f && !error && !write_final_timestamp( f ) ) error = true;
  if( f && std::fclose( f ) != 0 ) error = true;
  f = 0;
  return !error;
  }


bool Rate_logger::open_file()
  {
  if( !filename_ ) return true;
  if( !f )
    {
    last_time = -1;
    f = std::fopen( filename_, "w" );
    error = !f || !write_logfile_header( f, "Rates" ) ||
            std::fprintf( f, "#Time  Ipos  Current_rate  Average_rate  Errors  Errsize\n" ) < 0;
    }
  return !error;
  }


bool Rate_logger::print_line( const long time, const long long ipos,
                              const long long a_rate, const long long c_rate,
                              const int errors, const long long errsize )
  {
  if( f && !error && time > last_time )
    {
    last_time = time;
    if( std::fprintf( f, "%2lu  0x%08llX  %8llu  %8llu  %7u  %8llu\n",
                      time, ipos, c_rate, a_rate, errors, errsize ) < 0 )
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
    error = !f || !write_logfile_header( f, "Reads" ) ||
            std::fprintf( f, "#  Ipos       Size  Copied_size  Error_size\n" ) < 0;
    }
  return !error;
  }


bool Read_logger::print_line( const long long ipos, const long long size,
                              const int copied_size, const int error_size )
  {
  if( f && !error &&
      std::fprintf( f, "0x%08llX	%llu	%u	%u\n",
                    ipos, size, copied_size, error_size ) < 0 )
    error = true;
  prev_is_msg = false;
  return !error;
  }


bool Read_logger::print_msg( const long time, const char * const msg )
  {
  if( f && !error &&
      std::fprintf( f, "%s# %s  %s\n", prev_is_msg ? "" : "\n\n",
                    format_time_dhms( time ), msg ) < 0 )
    error = true;
  prev_is_msg = true;
  return !error;
  }


bool Read_logger::print_time( const long time )
  {
  if( f && !error && time > 0 &&
      std::fprintf( f, "# %s\n", format_time_dhms( time ) ) < 0 )
    error = true;
  return !error;
  }
