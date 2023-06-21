/* GNU ddrescue - Data recovery tool
   Copyright (C) 2014-2023 Antonio Diaz Diaz.

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

#include <string>

#include "non_posix.h"

#ifdef USE_NON_POSIX
#include <cctype>
#include <sys/ioctl.h>

namespace {

void sanitize_string( std::string & str )
  {
  for( unsigned i = str.size(); i > 0; --i )	// remove non-printable chars
    {
    const unsigned char ch = str[i-1];
    if( std::isspace( ch ) ) str[i-1] = ' ';
    else if( ch < 32 || ch > 126 ) str.erase( i - 1, 1 );
    }
  for( unsigned i = str.size(); i > 0; --i )	// remove duplicate spaces
    if( str[i-1] == ' ' && ( i <= 1 || i >= str.size() || str[i-2] == ' ' ) )
      str.erase( i - 1, 1 );
  }

} // end namespace

#ifdef __HAIKU__
#include <Drivers.h>

bool device_id( const int fd, std::string & id_str )
  {
  char buf[256];

  if( ioctl( fd, B_GET_DEVICE_NAME, buf, sizeof buf ) != 0 ) return false;
  buf[(sizeof buf)-1] = 0;	// make sure it is null-terminated
  id_str = (const char *)buf;
  sanitize_string( id_str );
  return true;
  }

#elif defined __CYGWIN__
#include <io.h>
#define _WIN32_WINNT 0x0600	// >= Vista, for BusTypeSata
#include <windows.h>

bool device_id( const int fd, std::string & id_str )
  {
  HANDLE h = (HANDLE) _get_osfhandle( fd );
  if( h == INVALID_HANDLE_VALUE )
    return false;

  STORAGE_PROPERTY_QUERY query =
    { StorageDeviceProperty, PropertyStandardQuery, { 0 } };
  union {
    char raw[1024];
    STORAGE_DEVICE_DESCRIPTOR desc;
  } data = { { 0, } };
  DWORD nout = 0;

  if( !DeviceIoControl( h, IOCTL_STORAGE_QUERY_PROPERTY,
                        &query, sizeof(query),
                        &data, sizeof(data),
                        &nout, (LPOVERLAPPED)0 ) )
    return false;

  if( data.desc.VendorIdOffset )
    id_str = &data.raw[data.desc.VendorIdOffset];
  else
    id_str = "";
  if( data.desc.ProductIdOffset )
    {
    if( data.desc.BusType != BusTypeAta &&
        data.desc.BusType != BusTypeSata )
      id_str += ' ';
    id_str += &data.raw[data.desc.ProductIdOffset];
    }
  sanitize_string( id_str );
  if( id_str.empty() ) return false;

  if( data.desc.SerialNumberOffset )
    {
    std::string id_serial( &data.raw[data.desc.SerialNumberOffset] );
    sanitize_string( id_serial );
    if( !id_serial.empty() )
      { id_str += "::"; id_str += id_serial; }
    }
  return true;
  }

#else				// use linux by default
#include <linux/hdreg.h>

bool device_id( const int fd, std::string & id_str )
  {
  struct hd_driveid id;

  if( ioctl( fd, HDIO_GET_IDENTITY, &id ) != 0 ) return false;
  id_str = (const char *)id.model;
  std::string id_serial( (const char *)id.serial_no );
  sanitize_string( id_str );
  sanitize_string( id_serial );
  if( !id_str.empty() || !id_serial.empty() )
    { id_str += "::"; id_str += id_serial; return true; }
  return false;
  }

#endif

#else	// USE_NON_POSIX

bool device_id( const int, std::string & ) { return false; }

#endif
