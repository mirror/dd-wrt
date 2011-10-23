/*
Copyright (C) 2009 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*

	Utility base routines header for WAVE audio file reading.

	History:
		August 2009 -- Begin implementation
*/

#ifndef _WAVE_UTIL_H_INCLUDED
#define _WAVE_UTIL_H_INCLUDED

#include <stdio.h>

#ifndef _WIN32

#ifdef USE_SNDFILE

#include <ao/ao.h>
#include <math.h>

#endif

#else

#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>

#endif

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD_PTR;

#define LOBYTE(w)	((BYTE)((DWORD_PTR)(w) & 0xff))
#define LOWORD(l)	((WORD)((DWORD_PTR)(l) & 0xffff))

#define WAVE_DEFAULT_AMPLITUDE	0
#define WAVE_DECIBEL_STEP		3
#define WAVE_DYNAMIC_RANGE		120
#define WAVE_MAX_DECIBEL_STEP	WAVE_DYNAMIC_RANGE/WAVE_DECIBEL_STEP
#define WAVE_MIN_DECIBEL_STEP	-1*WAVE_DYNAMIC_RANGE/WAVE_DECIBEL_STEP

typedef struct
{
	int			error;
	const char 	*p_name;

} WAVE_ERR;

#ifdef _WIN32

typedef struct WAVE_WIN {

	HWAVEOUT			hwave;
	WAVEHDR				whdr[2];
	HANDLE				hBuffSem;	/* signal that a buffer is free */

} WAVE_WIN;

#endif

struct WAVE_PARAMS;

/*

client callback function

return 1 to exit wave play; parameters are dynamic

*/

typedef int (*CB_WAVE)(struct WAVE_PARAMS*);
typedef double (*SAMPLE_CONVERT)(char *dest,void *src);

typedef struct WAVE_PARAMS {

	char		*wave_file;
	float		buff_factor;
	int			loops;
	double		gain;
	void		*cb_client_data;	

	CB_WAVE		p_func;

} WAVE_PARAMS;

typedef struct WAVE_PLAY {

	char			*wave_file;
	double			gain;
	int				loops;
	CB_WAVE			cb_is_exit;
	void			*cbClientData;

	SAMPLE_CONVERT	sample_up;
	SAMPLE_CONVERT	sample_down;

	FILE			*f;

	int			wave_size;
	int			fmt_chunk_size;
	int			compression_code;
	double		sample_range_lower;
	double		sample_range_upper;
	int			channels;
	int			sample_rate;
	int			bytes_per_second;
	int			block_align;
	int			bits_per_sample;
	int			cbSize;
	int			valid_bits_per_sample;
	int			channel_mask;
	char		subformat[17];
	int			wave_data;

	/*
		convenient if driver, hardware can't handle 

		bit depth -- convert it on output
	*/

	float		output_bit_depth;
	int			output_bytes_per_sample;

	/*
		change bits per sample, buffer reads, 

		etcetera according to output_bit_depth
	*/

	float		output_factor;
	int			data_offset;
	int			wave_pos;
	char		bytes_per_sample;
	char		*wave_buff[2];
	int			buffer_size;
	int			cur_buffer;
	char		is_quit;

#ifdef _WIN32

	WAVE_WIN	win_data;
#endif

} WAVE_PLAY;

int check_header(FILE *f);
int get_header_info(WAVE_PLAY *p_wave_play);
int fill_wave_buffer(WAVE_PLAY *wave_struct,int buff_index,int *bytes_filled);
void wait_done_playing(WAVE_PLAY *p_wave_play);
int reset_wave(WAVE_PLAY *p_wave_play);

#ifndef _WIN32

long get_func_addr(void **dl_handle,char *lib,char *func,long ptr_func_default);

#endif

const char* libao_err_name(int err_num);

#endif /*_WAVE_UTIL_H_INCLUDED*/

