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

class Logger
  {
protected:
  const char * filename_;
  FILE * f;				// output stream
  bool error;

public:
  Logger() : filename_( 0 ), f( 0 ), error( false ) {}

  bool active() const { return ( f != 0 && !error ); }
  const char * filename() const { return filename_; }
  void set_filename( const char * const name );
  bool close_file();
  };


class Event_logger : public Logger
  {
public:
  bool open_file();
  bool echo_msg( const char * const msg );
  bool print_msg( const long long time, const char * const percent_rescued,
                  const char * const msg );
  bool print_eor( const long long time, const char * const percent_rescued,
                  const long long current_pos,
                  const char * const current_status_name );
  };

extern Event_logger event_logger;


class Rate_logger : public Logger
  {
  long long last_time;
public:
  Rate_logger() : last_time( -1 ) {}
  bool open_file();
  bool print_line( const long long time, const long long ipos,
                   const long long a_rate, const long long c_rate,
                   const unsigned long bad_areas,
                   const long long bad_size );
  };

extern Rate_logger rate_logger;


class Read_logger : public Logger
  {
  bool prev_is_msg;
public:
  Read_logger() : prev_is_msg( true ) {}
  bool open_file();
  bool print_line( const long long ipos, const long long size,
                   const int copied_size, const int error_size );
  bool print_msg( const long long time, const char * const msg );
  bool print_time( const long long time );
  };

extern Read_logger read_logger;
