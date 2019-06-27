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

class Logger
  {
protected:
  const char * filename_;
  FILE * f;				// output stream
  bool error;

public:
  Logger() : f( 0 ), error( false ) {}

  void set_filename( const char * const name ) { filename_ = name; }
  bool close_file();
  };


class Rate_logger : public Logger
  {
  long last_time;
public:
  Rate_logger() : last_time( -1 ) {}
  bool open_file();
  bool print_line( const long time, const long long ipos,
                   const long long a_rate, const long long c_rate,
                   const int errors, const long long errsize );
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
  bool print_msg( const long time, const char * const msg );
  bool print_time( const long time );
  };

extern Read_logger read_logger;
