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
	Routines to output WAVE audio file on MS Windows.  For Linux, BSD,
	and like, we'll use one of the libraries available on these platforms.

	History:
		July 2009 -- Begin implementation 
*/



#define MODULE_TAG "WAVE: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include <io.h>

#endif

#include "waveout.h"
#include "safe_mem.h"
#include "unicode_util.h"
#include "numbers.h"
#include "debug_if.h"

#ifndef _WIN32

#ifndef NO_DYN_LOAD

#include <dlfcn.h>

static int	is_no_aolib=0;

#endif

/*get_func_addr return value coercion defs*/

typedef void (*FUNC_1)(void); /*ao_initialize, ao_shutdown*/ 
typedef int (*FUNC_2)(void); /*ao_default_driver_id*/
typedef ao_device* (*FUNC_3)(int,ao_sample_format*,ao_option*); /*ao_open_live*/
typedef int (*FUNC_4)(ao_device*,void*,uint_32); /*ao_play*/
typedef int (*FUNC_5)(ao_device*); /*ao_close*/
typedef int (*FUNC_7)(char*); /*ao_driver_id*/
typedef ao_info* (*FUNC_8)(int); /*ao_driver_info*/

static int	is_init=0;
static void	*aolib_handle=NULL;

#endif

#ifdef _WIN32

static const WAVE_ERR wave_error_table[]  =
{

	{MMSYSERR_ALLOCATED,"MMSYSERR_ALLOCATED"},
	{MMSYSERR_BADDEVICEID,"MMSYSERR_BADDEVICEID"},
	{MMSYSERR_INVALHANDLE,"MMSYSERR_INVALHANDLE"},
	{MMSYSERR_NODRIVER,"MMSYSERR_NODRIVER"},
	{MMSYSERR_NOMEM,"MMSYSERR_NOMEM"},
	{WAVERR_BADFORMAT,"WAVERR_BADFORMAT"},
	{WAVERR_SYNC,"WAVERR_SYNC"},
	{WAVERR_STILLPLAYING,"WAVERR_STILLPLAYING"},
	{WAVERR_UNPREPARED,"WAVERR_UNPREPARED"},
	{0,NULL}
};

static const char* unknown_error = "Unknown error";


const char* wave_error_get_name(int error)
{
	const WAVE_ERR *it = wave_error_table;


	while (it->p_name != NULL)
	{
		if (it->error == error)
		{
			return it->p_name;
		}

		++it;
	}

	return unknown_error;
}

static void CALLBACK cbWavePlay(HWAVEOUT hwave,int msg,DWORD p_data,DWORD param1,DWORD param2)
{

	WAVE_PLAY	*p_wave_play;


	p_wave_play=(WAVE_PLAY *) p_data;


	if (msg == MM_WOM_DONE)	{			

		ReleaseSemaphore(p_wave_play->win_data.hBuffSem,1,NULL);

		p_wave_play->is_quit=p_wave_play->cb_is_exit(p_wave_play->cbClientData);
	}
}

static int do_stream_wave(WAVE_PLAY *p_wave_play)
{
	int			bytes_read=0;
	char		is_done=0;
	int			cur_buffer;
	MMRESULT	out_write_ret=MMSYSERR_NOERROR;


	is_done=fill_wave_buffer(p_wave_play,p_wave_play->cur_buffer,&bytes_read);	

	p_wave_play->win_data.whdr[p_wave_play->cur_buffer].dwBufferLength=bytes_read;

	cur_buffer=p_wave_play->cur_buffer;

	p_wave_play->cur_buffer=(p_wave_play->cur_buffer + 1)%2;

	out_write_ret=waveOutWrite(p_wave_play->win_data.hwave,&(p_wave_play->win_data.whdr[cur_buffer]), sizeof(WAVEHDR));

	if (MMSYSERR_NOERROR==out_write_ret) {

		DBG_PRINTF((LOG_DEBUG, "D:" MODULE_TAG "waveOutWrite success in function do_stream_wave...\n"));
	}
	else {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutWrite failed in function do_stream_wave, " \
					"with error code:  %d:  %s.  Try increasing wave output buffer...\n",out_write_ret,wave_error_get_name(out_write_ret)));
		
		/*
			avoid deadlock in case cbWavePlay above never gets a callback
		*/

		ReleaseSemaphore(p_wave_play->win_data.hBuffSem,1,NULL);
	}

	if (is_done) {

		return (!(reset_wave(p_wave_play)));
	}	

	return is_done;
}

static void stream_wave(WAVE_PLAY *p_wave_play)
{

	char		is_done=0;


	while(!(p_wave_play->is_quit) && !(is_done)) {

		WaitForSingleObject(p_wave_play->win_data.hBuffSem,INFINITE);

		is_done=do_stream_wave(p_wave_play);
	}
}

#endif

static void close_wave_file(FILE **f)
{
	if (*f) {

		fclose(*f);

		*f=NULL;
	}
}

static void destroy(WAVE_PLAY **p_wave_play)
{
	if (!(*p_wave_play))

		return;

	close_wave_file(&(*p_wave_play)->f);

	free((*p_wave_play)->wave_file);

	free((*p_wave_play)->wave_buff[0]);

	free((*p_wave_play)->wave_buff[1]);

#ifdef _WIN32

	CloseHandle((*p_wave_play)->win_data.hBuffSem);
#endif

	free(*p_wave_play);

	*p_wave_play=NULL;
}

static WAVE_PLAY *create(WAVE_PLAY **p_wave_play,WAVE_PARAMS *p_wave_params)
{

	WAVE_PLAY		*p_this_wave=NULL;


	p_this_wave=safe_malloc(sizeof(WAVE_PLAY));
	memset(p_this_wave,0,sizeof(WAVE_PLAY));
	
	p_this_wave->wave_file=strcpy(safe_malloc(strlen(p_wave_params->wave_file)+1),p_wave_params->wave_file);

	p_this_wave->gain=p_wave_params->gain;

	p_this_wave->loops=p_wave_params->loops;

	p_this_wave->cb_is_exit=p_wave_params->p_func;

	p_this_wave->cbClientData=p_wave_params;

	p_this_wave->f=0;

#ifndef _WIN32

	if (!(p_this_wave->f=utf_fopen(p_wave_params->wave_file,"r"))) {
#else

	if (!(p_this_wave->f=utf_fopen(p_wave_params->wave_file,"r+b"))) {
#endif

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "fopen failed in function create, " \
					"attempting to open wave file %s...\n",p_wave_params->wave_file));

		destroy(&p_this_wave);

		return NULL;
	}
	
	if (!(get_header_info(p_this_wave))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "get_header_info failed in function create...\n"));

		destroy(&p_this_wave);

		return NULL;
	}

	if (!(p_this_wave->bits_per_sample==8 || p_this_wave->bits_per_sample==16 || 
		p_this_wave->bits_per_sample==24 || p_this_wave->bits_per_sample==32))

		p_this_wave->gain=0;

	p_this_wave->bytes_per_sample=p_this_wave->bits_per_sample/8;

	/*stream buff_factor mult or div of bytes per second at a time*/

	p_this_wave->buffer_size=(int) (p_this_wave->sample_rate*(!(p_wave_params->buff_factor) ?
		.25:p_wave_params->buff_factor)*p_this_wave->block_align*p_this_wave->output_factor);

	p_this_wave->wave_buff[0]=safe_malloc(p_this_wave->buffer_size);
	p_this_wave->wave_buff[1]=safe_malloc(p_this_wave->buffer_size);

	p_this_wave->cur_buffer=0;

#ifdef _WIN32

	p_this_wave->win_data.whdr[0].lpData=p_this_wave->wave_buff[0];
	p_this_wave->win_data.whdr[1].lpData=p_this_wave->wave_buff[1];
	p_this_wave->win_data.whdr[0].dwBufferLength=p_this_wave->buffer_size;
	p_this_wave->win_data.whdr[1].dwBufferLength=p_this_wave->buffer_size;
	p_this_wave->win_data.whdr[0].dwFlags=0;
	p_this_wave->win_data.whdr[1].dwFlags=0;

	p_this_wave->win_data.hBuffSem=CreateSemaphore(NULL,0,1,NULL);

#endif

	*p_wave_play=p_this_wave;


	return p_this_wave;
}

#ifdef _WIN32

static int unprepair_header(WAVE_PLAY *p_wave_play,int index)
{

	int		wave_ret=MMSYSERR_NOERROR;


	while ((wave_ret=waveOutUnprepareHeader(p_wave_play->win_data.hwave,&(p_wave_play->win_data.whdr[index]),
		   sizeof(WAVEHDR)))==WAVERR_STILLPLAYING)

		wait_done_playing(p_wave_play);


	return wave_ret;
}

int waveout_init()
{

	return 0;
}

int waveout_shutdown()
{

	return 0;
}

#endif

#ifndef _WIN32

#ifndef NO_DYN_LOAD

void report_no_lib(char *lib)
{

	is_no_aolib=1;

	DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error loading shared library, %s...Install %s for audible network status alerts...\n",lib,lib));
}

static void ao_initialize_stubb(void)
{

	report_no_lib("libao");
}

static void ao_shutdown_stubb(void)
{

	report_no_lib("libao");
}

static ao_device* ao_open_live_stubb(int driver_id, ao_sample_format *format,ao_option *option)
{

	report_no_lib("libao");


	return NULL;
}

static int ao_play_stubb(ao_device *device, char *output_samples, uint_32 num_bytes)
{

	report_no_lib("libao");


	return 1;
}

static int ao_close_stubb(ao_device *device)
{

	report_no_lib("libao");


	return 1;
}

static int ao_default_driver_id_stubb(void)
{

	report_no_lib("libao");


	return 1;
}

/*
static int ao_driver_id_stubb(char* short_name)
{

	report_no_lib("libao");

	return 1;
}
*/

static ao_info* ao_driver_info_stubb(int driver_id)
{

	report_no_lib("libao");


	return NULL;
}

#endif 

int waveout_init()
{

#ifndef NO_DYN_LOAD

	static long ao_initialize_ptr=(long) &ao_initialize_stubb;
#else

	static long ao_initialize_ptr=(long) &ao_initialize;
#endif

	if (!(is_init)) {

		((FUNC_1) (get_func_addr(&aolib_handle,"libao.so","ao_initialize",ao_initialize_ptr)))();

#ifndef NO_DYN_LOAD

		if (is_no_aolib)

			return 1;
#endif
		is_init=1;
	}

	return 0;
}

int waveout_shutdown()
{

#ifndef NO_DYN_LOAD

	static long ao_shutdown_ptr=(long) &ao_shutdown_stubb;
#else

	static long ao_shutdown_ptr=(long) &ao_shutdown;
#endif

	if (!(is_init))

		return 1;

	else 

		((FUNC_1) (get_func_addr(&aolib_handle,"libao.so","ao_shutdown",ao_shutdown_ptr)))();


	return 0;
}

int play_wave_open_os(WAVE_PLAY *p_wave_play)
{
	ao_device			*device=NULL;
	SAMPLE_FORMAT		format;
	int					driver;
	int					bytes_read=0;
	int					is_done=0;

#ifndef NO_DYN_LOAD
	static long ao_default_driver_id_ptr=(long) &ao_default_driver_id_stubb;
	static long ao_open_live_ptr=(long) &ao_open_live_stubb;
	static long ao_play_ptr=(long) &ao_play_stubb;
	static long ao_close_ptr=(long) &ao_close_stubb;
	/*static long ao_driver_id_ptr=(long) &ao_driver_id_stubb;*/
	static long ao_driver_info_ptr=(long) &ao_driver_info_stubb;
#else
	static long ao_default_driver_id_ptr=(long) &ao_default_driver_id;
	static long ao_open_live_ptr=(long) &ao_open_live;
	static long ao_play_ptr=(long) &ao_play;
	static long ao_close_ptr=(long) &ao_close;
	/*static long ao_driver_id_ptr=(long) &ao_driver_id;*/
	static long ao_driver_info_ptr=(long) &ao_driver_info;
#endif

	memset(&format,0,sizeof(ao_sample_format)); /*adapt to this type.  SAMPLE_FORMAT inited below*/

	/*
		is_init cond/section here, needs a mutex 
		if more black box than present use
	*/

#ifndef NO_DYN_LOAD

	if (is_no_aolib)

		return 1;
#endif

	if (!(is_init)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Call to play_wave_open_os before waveout_init in play_wave_open_os...\n"));

		return 1;
	}

	/* -- Setup for default driver -- */

	if ((driver=((FUNC_2) (get_func_addr(&aolib_handle,"libao.so","ao_default_driver_id",ao_default_driver_id_ptr)))())==-1) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "ao_default_driver_id() returned NO DRIVER (%s) in play_wave_open_os...\n",libao_err_name(errno)));
	} 
	else {

		ao_info	*driver_info=NULL;

		if (!(driver_info=((FUNC_8) (get_func_addr(&aolib_handle,"libao.so","ao_driver_info",ao_driver_info_ptr)))(driver)))

			DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "ao_driver_info returned NULL (%s) in play_wave_open_os...\n",libao_err_name(errno)));
		else

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "got driver, %s...\n",driver_info->short_name));

		format.bits=p_wave_play->bits_per_sample*p_wave_play->output_factor;
		format.channels=p_wave_play->channels;
		format.rate=p_wave_play->sample_rate;
		format.byte_format=AO_FMT_LITTLE;
		format.matrix="L,R";

		device=((FUNC_3) (get_func_addr(&aolib_handle,"libao.so","ao_open_live",ao_open_live_ptr)))(driver,(ao_sample_format *) &format,NULL);

		if (device==NULL) {

			DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "device NULL (%s) in play_wave_open_os.  returning...\n",libao_err_name(errno)));	

			return 1;
		}

		while (!(is_done)) {

			if ((is_done=fill_wave_buffer(p_wave_play,p_wave_play->cur_buffer,&bytes_read)))

				is_done=(!(reset_wave(p_wave_play)));

			if (bytes_read==0) {

				if (is_done)

					break;
			}
			else {		

				if (!(((FUNC_4) (get_func_addr(&aolib_handle,"libao.so","ao_play",ao_play_ptr)))(device,p_wave_play->wave_buff[p_wave_play->cur_buffer],bytes_read))) {

					DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "ao_play failed (%s) in play_wave_open_os...\n",libao_err_name(errno)));
				}
			}

			if (p_wave_play->cb_is_exit(p_wave_play->cbClientData)) {

				DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Client requested quit.  Returning from play_wave_open_os...\n"));

				break;
			}
		}
	}

	if (device) {

		wait_done_playing(p_wave_play);		

		if (!(((FUNC_5) (get_func_addr(&aolib_handle,"libao.so","ao_close",ao_close_ptr)))(device))) {

			DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "ao_close failed (%s) in play_wave_open_os...\n",libao_err_name(errno)));
		}
	}	

	return 0;
}

#endif

#ifdef _WIN32

int play_wave_ms(WAVE_PLAY **pp_wave_play)
{

	WAVE_PLAY			*p_wave_play=*pp_wave_play;
	WAVEFORMATPCMEX		wave_format;	
	int					win_ret=0;


	memset(&wave_format,0,sizeof(WAVEFORMATPCMEX));
	memcpy(&wave_format.SubFormat,p_wave_play->subformat,sizeof(GUID));

	wave_format.Samples.wValidBitsPerSample=(int) (p_wave_play->valid_bits_per_sample*p_wave_play->output_factor);
	wave_format.dwChannelMask=p_wave_play->channel_mask;
	wave_format.Format.nChannels=p_wave_play->channels;
	wave_format.Format.wBitsPerSample=(int) (p_wave_play->bits_per_sample*p_wave_play->output_factor);
	wave_format.Format.nSamplesPerSec=p_wave_play->sample_rate;
	wave_format.Format.nBlockAlign=(int) (p_wave_play->block_align*p_wave_play->output_factor);
	wave_format.Format.nAvgBytesPerSec=(int) (p_wave_play->bytes_per_second*p_wave_play->output_factor);
	wave_format.Format.cbSize=22;

	/*32 bit Windows compat -- bits per sample must be 8 or 16*/

	if (p_wave_play->bits_per_sample==8 || p_wave_play->bits_per_sample==16)

		wave_format.Format.wFormatTag=WAVE_FORMAT_PCM;

	else {

		wave_format.Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;

		DBG_PRINTF((LOG_WARNING, "W:" MODULE_TAG "Wave file bits per sample, %d, not 8 or 16, to be output "\
					"as %d bits per sample with format tag, WAVE_FORMAT_EXTENSIBLE...\n",p_wave_play->bits_per_sample,
					wave_format.Format.wBitsPerSample));
	}

	win_ret=waveOutOpen(&(p_wave_play->win_data.hwave),WAVE_MAPPER,(PWAVEFORMATEX) &wave_format,(DWORD_PTR) cbWavePlay,
							(DWORD_PTR) p_wave_play, CALLBACK_FUNCTION);


	if (!(MMSYSERR_NOERROR==win_ret)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutOpen failed in function play_wave," \
					"with error code:  %d:  %s.  Aborting play_wave...\n",win_ret,wave_error_get_name(win_ret)));

		destroy(pp_wave_play);

		return 1;
	}

	win_ret=waveOutPrepareHeader(p_wave_play->win_data.hwave,&(p_wave_play->win_data.whdr[0]),sizeof(WAVEHDR));

	if (!(MMSYSERR_NOERROR==win_ret)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutPrepareHeader failed in function play_wave," \
					"with error code:  %d:  %s.  Aborting play_wave...\n",win_ret,wave_error_get_name(win_ret)));

		destroy(pp_wave_play);


		return 1;
	}

	win_ret=waveOutPrepareHeader(p_wave_play->win_data.hwave,&(p_wave_play->win_data.whdr[1]),sizeof(WAVEHDR));

	if (!(MMSYSERR_NOERROR==win_ret)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutPrepareHeader failed in function play_wave," \
					"with error code:  %d:  %s.  Aborting play_wave...\n",win_ret,wave_error_get_name(win_ret)));

		destroy(pp_wave_play);


		return 1;
	}

	do_stream_wave(p_wave_play);
	do_stream_wave(p_wave_play);

	stream_wave(p_wave_play);	

	win_ret=unprepair_header(p_wave_play,0);

	if (!(MMSYSERR_NOERROR==win_ret)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutUnprepareHeader failed in function play_wave," \
					"with error code:  %d:  %s...\n",win_ret,wave_error_get_name(win_ret)));
	}

	win_ret=unprepair_header(p_wave_play,1);

	if (!(MMSYSERR_NOERROR==win_ret)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutUnprepareHeader failed in function play_wave," \
					"with error code:  %d:  %s...\n",win_ret,wave_error_get_name(win_ret)));
	}

	win_ret=waveOutClose(p_wave_play->win_data.hwave);

	if (!(MMSYSERR_NOERROR==win_ret)) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "waveOutClose failed in function play_wave," \
					"with error code:  %d:  %s...\n",win_ret,wave_error_get_name(win_ret)));
	}

	/*return 0 if okay*/

	return (!(win_ret==MMSYSERR_NOERROR));
}

#endif

int play_wave1(WAVE_PARAMS *p_wave_params)
{
	int			ret=0;
	WAVE_PLAY	*p_wave_play=NULL;


	if (!(create(&p_wave_play,p_wave_params))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "create error in play_wave.  Returning...\n"));


		return 1;
	}

#ifndef _WIN32

	ret=play_wave_open_os(p_wave_play);
#else

	ret=play_wave_ms(&p_wave_play);
#endif

	destroy(&p_wave_play);

	return ret;
}

int play_wave(char *wave_file,int loops,CB_WAVE p_func,void *cbClientData)
{

	WAVE_PARAMS		wave_params;


	memset(&wave_params,0,sizeof(WAVE_PARAMS));

	wave_params.wave_file=wave_file;

	wave_params.loops=loops;

	wave_params.p_func=p_func;

	wave_params.cb_client_data=cbClientData;

	wave_params.gain=WAVE_DEFAULT_AMPLITUDE;

	
	return play_wave1(&wave_params);
}

