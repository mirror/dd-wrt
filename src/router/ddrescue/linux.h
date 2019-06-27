/*  GNU ddrescue - Data recovery tool
    Copyright (C) 2014 Antonio Diaz Diaz.

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

const char * device_id( const int fd );

extern bool sgpt;
extern int ata;
extern bool checked_ata;
extern bool bad_ata_read;
extern bool partial_read;
extern bool mark_error;
extern int passthrough_error;
extern int sector_size;
extern bool extended;

void print_sgio_help( void );

void option_scsi_passthrough( void );

void option_ata_passthrough( void );

void option_mark_abnormal_error( void );

int check_device( const char *iname, const int verbosity, const int cluster );

int read_linux( const int fd, uint8_t * const buf, const int size, const long long pos, int sz, const int verbosity );
