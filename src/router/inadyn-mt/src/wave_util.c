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

	Utility base routines for WAVE audio file reading.

	History:
		August 2009 -- Begin implementation		
  
*/
#define MODULE_TAG "WAVE_UTIL: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#ifndef _WIN32

#include <dlfcn.h>

#endif

#include <math.h>
#include "wave_util.h"
#include "unicode_util.h"
#include "numbers.h"
#include "debug_if.h"

#define	MIN_UINT8	0.0f
#define	MAX_UINT8	255.0f
#define	MIN_INT8	-128.0f
#define	MAX_INT8	127.0f
#define	MIN_INT16	-32768.0f
#define	MAX_INT16	32767.0f
#define	MIN_INT24	-8388608.0f
#define	MAX_INT24	8388607.0f
#define	MIN_INT32	-2147483648.0f
#define	MAX_INT32	2147483647.0f

#ifndef _WIN32

typedef struct
{
	int			err_num;
	const char	*err_name;
} LIBAO_ERR_NAME;

static const LIBAO_ERR_NAME libao_err_table[] =
{
	{AO_ENODRIVER,"AO_ENODRIVER"},
	{AO_ENOTLIVE,"AO_ENOTLIVE"},
	{AO_EBADOPTION,"AO_EBADOPTION"},
	{AO_EOPENDEVICE,"AO_EOPENDEVICE"},
	{AO_EFAIL,"AO_EFAIL"},
	{0,"Unknown Error"}
};

#endif

static int fact_chunk_handler(WAVE_PLAY *);
static int peak_chunk_handler(WAVE_PLAY *);
static int data_chunk_handler(WAVE_PLAY *);

static char *read_string(char *dest,FILE *f,int bytes);

typedef int (*WAVE_CHUNK_HANDLER_FUNC)(WAVE_PLAY *);

typedef struct
{
	WAVE_CHUNK_HANDLER_FUNC p_func;
} WAVE_CHUNK_HANDLER_TYPE;

typedef struct
{
	char *chunk_name;
	WAVE_CHUNK_HANDLER_TYPE p_handler;
} WAVE_CHUNK_PARSE;

static WAVE_CHUNK_PARSE wave_chunk_parse_table[] =
{
	{"fact",{fact_chunk_handler}},
	{"peak",{peak_chunk_handler}},
	{"data",{data_chunk_handler}},
	{NULL,{NULL}}
};

#ifndef _WIN32

const char *libao_err_name(int err_num)
{

	int	i;


	for(i=0;libao_err_table[i].err_num!=0;i++) {

		if (libao_err_table[i].err_num==err_num)

			return libao_err_table[i].err_name;
	}

	return libao_err_table[i].err_name;
}

#endif

static int fact_chunk_handler(WAVE_PLAY *wave_play)
{

	int	num_in;


	/*fact chunk size*/
	if (!(read_number(wave_play->f,&num_in,4))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'fact' chunk size in " \
					"function fact_chunk_handler...\n"));

		return 0;
	}

	/*samples per channel in file*/
	if (!(read_number(wave_play->f,&num_in,4))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'fact' chunk samples per second in " \
					"function fact_chunk_handler...\n"));

		return 0;
	}


	return 1;
}

static int peak_chunk_handler(WAVE_PLAY *wave_play)
{
	int	num_in;


	/*peak chunk size*/
	if (!(read_number(wave_play->f,&num_in,4))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'PEAK' chunk size in " \
					"function peak_chunk_handler...\n"));

		return 0;
	}

	fseek(wave_play->f,num_in,SEEK_CUR);


	return 1;
}

static int data_chunk_handler(WAVE_PLAY *wave_play)
{

	if (!(read_number(wave_play->f,&wave_play->wave_data,4))) {
	
		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading wave data size in " \
				"function data_chunk_handler...\n"));

		return 0;
	}


	wave_play->data_offset=ftell(wave_play->f);


	return 1;
}

static int parse_wave_chunks(WAVE_PLAY *wave_play)
{

	char	sRIFF[256];
	int	i;


	do {

		memset(sRIFF,0,256);
		
		if (!(strcmp(utf_8_strtolwr(read_string(sRIFF,wave_play->f,4)),"")))

			return 0;


		i=0;


		do {

			if (!(strcmp(wave_chunk_parse_table[i].chunk_name,sRIFF))) {

				if (!(wave_chunk_parse_table[i].p_handler.p_func(wave_play)))

					return 0;

				break;
			}

			i++;

		} while (wave_chunk_parse_table[i].chunk_name);

	} while (strcmp("data",sRIFF));
	

	return 1;
}

#ifndef _WIN32

long get_func_addr(void **dl_handle,char *lib,char *func,long ptr_func_default)
{
	long		func_ptr=0;
	const char	*error_code=NULL;


#ifdef NO_DYN_LOAD

	return ptr_func_default;
#endif

	/*reset dlerror()*/
	dlerror();


	if (!(*dl_handle))

		*dl_handle=dlopen(lib,RTLD_LAZY);

	if (!(*dl_handle)) {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error loading shared library, %s:  %s\n",lib,dlerror()));

		return ptr_func_default;
	}

	func_ptr=(long) dlsym(*dl_handle,func);

	error_code=dlerror();

	if (!(error_code==NULL)) {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error getting shared library function, %s:  %s\n",func,error_code));

		func_ptr=ptr_func_default;

		dlclose(*dl_handle);

		*dl_handle=NULL;
	}

	return func_ptr;
}

#endif

/*
	Adapted from PortAudio, pa_converters.c

	convert bit depth upwards to ieee 32 bit floating point

	convert bit depth downwards from ieee 32 bit floating point

	handles PCM depths 8, 16, 24, and ieee 32 bit floating point
*/
static double cnvrt_8_to_float(char *dest,void *src)
{

	float	float_sample;


	/*convert 8 bit pcm to ieee 32 bit float*/

	float_sample=(float) (((*((unsigned char *) src))-MAX_INT8-1)/(MAX_INT8+1));
		
	memcpy(dest,&float_sample,4);


	return float_sample;
}

static double cnvrt_16_to_float(char *dest,void *src)
{

	float	float_sample;


	/*convert 16 bit pcm to ieee 32 bit float*/

	float_sample=(float) ((*((short *) src))/(MAX_INT16+1));

	memcpy(dest,&float_sample,4);


	return float_sample;
}

static double cnvrt_24_to_float(char *dest,void *src)
{

	float	float_sample;


	/*convert 24 bit pcm to ieee 32 bit float*/

	*((int *) src)=*((int *) src)<<8;

	float_sample=(float) ((*((int *) src))/(MAX_INT32+1));

	memcpy(dest,&float_sample,4);


	return float_sample;
}

static double cnvrt_32_to_float(char *dest,void *src)
{

	float	float_sample;


	float_sample=(float) ((*((int *) src))/(MAX_INT32+1));

	memcpy(dest,&float_sample,4);


	return float_sample;
}

static double cnvrt_float_to_float(char *dest,void *src)
{

	memcpy(dest,src,4);


	return *((float *) src);
}

static double cnvrt_float_to_8(char *dest,void *src)
{

	unsigned char	uchar_sample;


	uchar_sample=(unsigned char) (((*((float*) src))*(MAX_INT8+1))+MAX_INT8+1);

	memcpy(dest,&uchar_sample,1);


	return uchar_sample;
}

static double cnvrt_float_to_16(char *dest,void *src)
{

	short	short_sample;


	short_sample=(short) ((*((float*) src))*(MAX_INT16+1));

	memcpy(dest,&short_sample,2);


	return short_sample;
}

static double cnvrt_float_to_24(char *dest,void *src)
{

	int	int_sample;


	int_sample=(int) ((*((float*) src))*(MAX_INT32+1));

	int_sample=int_sample>>8;

	memcpy(dest,&int_sample,4);


	return int_sample;
}

static double cnvrt_float_to_32(char *dest,void *src)
{

	int	int_sample;


	int_sample=(int) ((*((float*) src))*(MAX_INT32+1));

	memcpy(dest,&int_sample,4);


	return int_sample;
}

static double cnvrt_copy_int(char *dest,void *src)
{

	memcpy(dest,src,4);


	return *((int *) src);
}

static int apply_amp(WAVE_PLAY *wave_play,int src,char *byte_ptr)
{
	
	float			float_gain;


	wave_play->sample_up((char *) &src,&src);

	if (wave_play->gain)

		float_gain=(float) (*((float *) (&src))*pow(10.0,(wave_play->gain*
					WAVE_DECIBEL_STEP/20.0)));
	else
		/*no attenuation, but possible depth conversion*/
		float_gain=*((float *) &src);

	wave_play->sample_down(byte_ptr,&float_gain);

	/*report clipping*/
	return (!(float_gain<-1 || float_gain>1));
}

static int do_read_bytes(char *dest,FILE *f,int bytes,int size,int output_size,WAVE_PLAY *wave_play)
{
	int			i=0;
	int			cur_byte=0;
	int			j;	
	int			c;
	int			read_cume;
	char		read_cume_ptr[4];
	int			eof=0;


	if (feof(f))

		return 0;


	while(i<bytes && !(eof)) {

		read_cume=0;

		for (j=0;j<size;j++) {

			c=fgetc(f);

			if ((eof=feof(f)))

				break;

			/*add c shifted j*8*/
			read_cume|=c<<(j<<3);

			i++;
		}

		if (eof && (cur_byte>=i))

			break;

		cur_byte=i;		

		memcpy(read_cume_ptr,&read_cume,size);

		if (wave_play)

			while(1) {
										
				if (apply_amp(wave_play,read_cume,read_cume_ptr))
			
					break;

				if (wave_play->gain==WAVE_MIN_DECIBEL_STEP)

					break;
				else
					wave_play->gain-=1;
			}

		for (j=0;j<output_size;j++)

			*(dest++)=(*(read_cume_ptr+j));
	}

	return i;
}

static int read_bytes(char *dest,FILE *f,int bytes)
{

	return do_read_bytes(dest,f,bytes,1,1,NULL);
}

static char *read_string(char *dest,FILE *f,int bytes)
{

	*(dest+read_bytes(dest,f,bytes))='\0';


	return dest;
}

static int get_riff_info(FILE *f,char *sRIFF,char *sWAVE,int *wave_size)
{
	int		ret=1;


	if (strcmp(utf_8_strtolwr(read_string(sRIFF,f,4)),"riff"))

		ret=0;

	else {

		read_number(f,wave_size,4);
		*wave_size-=8;


		if (strcmp(utf_8_strtolwr(read_string(sRIFF,f,4)),"wave"))

			ret=0;
	}

	return ret;
}

int check_header(FILE *f)
{
	int		ret;
	char		sRIFF[5];
	int		wave_size;
	int		f_pos=0;


	f_pos=ftell(f);
	fseek(f,0,SEEK_SET);


	ret=get_riff_info(f,sRIFF,sRIFF,&wave_size);


	fseek(f,f_pos,SEEK_SET);


	return ret;
}

static void get_conversion_functions(WAVE_PLAY *p_wave_play)
{

	p_wave_play->output_factor=1;

	p_wave_play->output_bit_depth=(float) p_wave_play->bits_per_sample;


	while (1) {

		if (p_wave_play->bits_per_sample==8) {

			/*convert 8 bit files to 16 bit on output*/

			p_wave_play->sample_down=&cnvrt_float_to_16;

			p_wave_play->sample_up=&cnvrt_8_to_float;


			p_wave_play->output_bit_depth=16;

			break;
		}

		if (p_wave_play->bits_per_sample==16) {

			p_wave_play->sample_down=&cnvrt_float_to_16;

			p_wave_play->sample_up=&cnvrt_16_to_float;
			
			break;
		}

		if (p_wave_play->bits_per_sample==24) {

			/*convert 24 bit files to 32 bit on output*/

			p_wave_play->sample_down=&cnvrt_float_to_32;

			p_wave_play->sample_up=&cnvrt_24_to_float;


			p_wave_play->output_bit_depth=32;

			break;
		}

		if (p_wave_play->bits_per_sample==32) {

			if (p_wave_play->compression_code==1) {

				p_wave_play->sample_down=&cnvrt_float_to_32;

				p_wave_play->sample_up=&cnvrt_32_to_float;

				break;
			}
			else {

				/*these are passthroughs -- no conversion takes place*/				

				/*convert float to pcm*/
				p_wave_play->sample_down=&cnvrt_float_to_32;

				p_wave_play->sample_up=&cnvrt_float_to_float;

				break;
			}
		}

		/*default*/

		p_wave_play->sample_down=&cnvrt_copy_int;
		p_wave_play->sample_up=&cnvrt_copy_int;

		break;
	}

	p_wave_play->output_factor=p_wave_play->output_bit_depth/p_wave_play->bits_per_sample;

	p_wave_play->output_bytes_per_sample=(int) (p_wave_play->bits_per_sample/8*p_wave_play->output_factor);
}

int get_header_info(WAVE_PLAY *p_wave_play)
{

#ifdef _WIN32

	/*avoid msvc6 compiler errors*/
	const static		GUID  KSDATAFORMAT_SUBTYPE_PCM = 
					{0x00000001,0x0000,0x0010,{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};
	const static		GUID  KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = 
					{0x00000003,0x0000,0x0010,{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};
#endif

	char	sRIFF[5];


	/*do simple header check, and get wave file size*/
	if (!(get_riff_info(p_wave_play->f,sRIFF,sRIFF,&p_wave_play->wave_size))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file header check failed in function get_header_info.  " \
					"Possibly corrupt file, or incompatible format...\n"));

		return 0;
	}

	/*point to beginning of format data*/
	if (strcmp(utf_8_strtolwr(read_string(sRIFF,p_wave_play->f,4)),"fmt ")) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Could not find wave file 'fmt ' section in function get_header_info.  " \
					"Possibly corrupt file, or incompatible format...\n"));

		return 0;
	}

	/*fmt chunk size must be 16, 18, or 40 bytes*/
	read_number(p_wave_play->f,&p_wave_play->fmt_chunk_size,4);

	if ((!(p_wave_play->fmt_chunk_size==16)) && (!(p_wave_play->fmt_chunk_size==18)) && 
		                                                     (!(p_wave_play->fmt_chunk_size==40))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file header check failed in function get_header_info.  " \
					"Format data section must be 16 bytes, 18 bytes, or 40 bytes, but got:  %d.  " \
					"Possibly corrupt file, or incompatible format...\n",p_wave_play->fmt_chunk_size));

		return 0;
	}

	read_number(p_wave_play->f,&p_wave_play->compression_code,2);

	/*require compression code 1, or 3 -- non-compressed wave format, or 32 bit ieee float*/
	if ((!(p_wave_play->compression_code==1)) && (!(p_wave_play->compression_code==3))) {


		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file header check failed in function get_header_info.  " \
					"Require simple wave file format with compression type code 1, or 3, but got compression " \
					"type code, %d...\n",p_wave_play->compression_code));

		return 0;
	}
	
	if (!(read_number(p_wave_play->f,&p_wave_play->channels,2))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading number of channels in " \
					"function get_header_info...\n"));

		return 0;
	}	

	if (!(read_number(p_wave_play->f,&p_wave_play->sample_rate,4))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading sample rate in " \
					"function get_header_info...\n"));

		return 0;
	}	

	if (!(read_number(p_wave_play->f,&p_wave_play->bytes_per_second,4))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading bytes per second in " \
					"function get_header_info...\n"));

		return 0;
	}	

	if (!(read_number(p_wave_play->f,&p_wave_play->block_align,2))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading number of channels in " \
					"function get_header_info...\n"));

		return 0;
	}	

	if (!(read_number(p_wave_play->f,&p_wave_play->bits_per_sample,2))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading bits per sample in " \
					"function get_header_info...\n"));

		return 0;
	}
	
	p_wave_play->valid_bits_per_sample=p_wave_play->bits_per_sample;

	if (!(p_wave_play->compression_code==1)) {

#ifdef _WIN32

		/*convert float to pcm*/
		memcpy(&p_wave_play->subformat,&KSDATAFORMAT_SUBTYPE_PCM,sizeof(GUID));

		/*memcpy(&p_wave_play->subformat,&KSDATAFORMAT_SUBTYPE_IEEE_FLOAT,sizeof(GUID));*/
#endif

		p_wave_play->sample_range_lower=-1;
		p_wave_play->sample_range_upper=1;

	}
	else {

#ifdef _WIN32

		memcpy(&p_wave_play->subformat,&KSDATAFORMAT_SUBTYPE_PCM,sizeof(GUID));
#endif
		if (p_wave_play->bits_per_sample==8) {

			p_wave_play->sample_range_lower=0;
			p_wave_play->sample_range_upper=255;
		}
		else {

			p_wave_play->sample_range_lower=-(pow(2,p_wave_play->bits_per_sample)/2);
			p_wave_play->sample_range_upper=pow(2,p_wave_play->bits_per_sample)/2-1;
		}
	}

#ifdef _WIN32

	/*p_wave_play->channel_mask=SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;*/
#endif

	/*get any "extra" fmt section fields*/
	if (!(p_wave_play->fmt_chunk_size>16)) {

		if (p_wave_play->compression_code==3)

			DBG_PRINTF((LOG_WARNING, "W:" MODULE_TAG "Wave file format data read no 'fmt' chunk \"extra\" section, but " \
					"compression code 3 in function get_header_info...\n"));
	} 
	else {

		/*a little file format error check*/
		if (p_wave_play->compression_code==1) {

			DBG_PRINTF((LOG_WARNING, "W:" MODULE_TAG "Wave file format data read failed reading \"extra\" section. " \
					"Compression code is one, but 'fmt' chunk size (%d) greater than 16, " \
					"in function get_header_info...\n",p_wave_play->fmt_chunk_size));
		}

		/*read regardless*/
		if (!(read_number(p_wave_play->f,&p_wave_play->cbSize,2))) {

			DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'fmt' \"extra\" " \
					"section field, section size in function get_header_info...\n"));

			return 0;
		}

		if (p_wave_play->cbSize) {

			if (!(read_number(p_wave_play->f,&p_wave_play->valid_bits_per_sample,2))) {

				DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'fmt' \"extra\" " \
						"section field, valid bits per sample in function get_header_info...\n"));

				return 0;
			}

			if (!(read_number(p_wave_play->f,&p_wave_play->channel_mask,4))) {

				DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'fmt' \"extra\" " \
						"section field, channel mask in function get_header_info...\n"));

				return 0;
			}

			if (!(strcmp(read_string(p_wave_play->subformat,p_wave_play->f,16),""))) {

				DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Wave file format data read failed reading 'fmt' \"extra\" " \
						"section field, sub format in function get_header_info...\n"));
			}
		}
	}

	/*attenuation functions*/
	get_conversion_functions(p_wave_play);


	/*
		except 'data' chunk, remaining 
		chunks not ordered or guaranteed
	*/
	if (!(parse_wave_chunks(p_wave_play))) {

		DBG_PRINTF((LOG_CRIT, "C:" MODULE_TAG "Could not find wave file 'data' section in function get_header_info.  " \
					"Possibly corrupt file, or incompatible format...\n"));

		return 0;
	}


	return 1;
}

int fill_wave_buffer(WAVE_PLAY *wave_struct,int buff_index,int *bytes_filled)
{

	int	bytes_read;


	memset(wave_struct->wave_buff[buff_index],0,wave_struct->buffer_size);

	bytes_read=do_read_bytes(wave_struct->wave_buff[buff_index],wave_struct->f,
						(int) (wave_struct->buffer_size/wave_struct->output_factor),
						wave_struct->bytes_per_sample,wave_struct->output_bytes_per_sample,wave_struct);

	*bytes_filled=(int) (bytes_read*wave_struct->output_factor);

	wave_struct->wave_pos=ftell(wave_struct->f);


	return (wave_struct->wave_data==wave_struct->wave_pos-wave_struct->data_offset);
}

void wait_done_playing(WAVE_PLAY *p_wave_play)
{

	os_sleep_ms(p_wave_play->buffer_size/p_wave_play->bytes_per_second*1000);
}

int reset_wave(WAVE_PLAY *p_wave_play)
{

	if (!(p_wave_play->loops==-1)) {

		if (!(p_wave_play->loops))

			return 0;
		else

			(p_wave_play->loops)--;
	}

	fseek(p_wave_play->f,p_wave_play->data_offset,SEEK_SET);

	return 1;
}
