/*
Copyright (C) 2008 Bryan Hoover (bhoover@wecs.com)

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

	Routines to output WAVE audio file on MS Windows.  For Linux, BSD,
	and like, we'll use one of the libraries available on these platforms.

	History:
		July 2009 -- Begin implementation		
  
*/

#include "wave_util.h"

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>

#else

/*libao api*/
typedef struct SAMPLE_FORMAT {

	int		bits;
	int		rate;
	int		channels;
	int		byte_format;
	char	*matrix;
} SAMPLE_FORMAT;

#endif

int waveout_init();
int waveout_shutdown();
int play_wave(char *wave_file,int loops,CB_WAVE p_func,void *cbClientData);
int play_wave1(WAVE_PARAMS *p_wave_params);
