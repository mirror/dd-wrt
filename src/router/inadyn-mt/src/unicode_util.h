/*
Copyright (C) 2007 Bryan Hoover (bhoover@wecs.com)

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
	Some unicode utf-8, utf-16 conversion, encoding, 
	decoding, utility, convenience

	Author: bhoover@wecs.com
	Date: Jan 2008

	History:
		- first implemetnation
*/

#define	UTF_16_SIZE	2
#define UTF_8_SIZE	6

#ifndef _WIN32

#include <stddef.h>

#endif

void utf_free_ar(char *utf_ar[],int size);

wchar_t *utf_8_to_16(wchar_t *dest,char *src);

wchar_t **utf_8_to_16_ar(wchar_t *dest[],char *src[],int size);

char *utf_16_to_8(char *dest,wchar_t *src);

char **utf_16_to_8_ar(char *dest[],wchar_t *src[],int size);

int utf_is_bom_16(FILE *p_file);

int utf_is_bom_8(FILE *p_file);

int utf_is_u_16_ascii(wchar_t *utf_16);

int utf_is_win_utf_file(wchar_t *szFile,int *is_bom);

char *utf_read_utf_8(char *c_buff,FILE *in_file);

int utf_8_encoded_size(unsigned char ch);

int utf_8_get_code_point(unsigned char *ch,unsigned int *cp);

unsigned int utf_8_code_point(unsigned char *ch);

int utf_8_get_char(char *ch,unsigned int cp);

char *utf_8_char(char *ch,unsigned int cp);

char *utf_prev_char(char *s,int byte_index);

char *utf_next_char(char *s,int byte_index);

char *utf_8_uri_encoded(char **dest,char *src,char *prefix,char *suffix);

int utf_8_len(unsigned char *s);

int utf_is_utf_8_wide(unsigned char *s);

char *utf_truncate(char *s,unsigned int max_bytes);

wchar_t *utf_malloc_8_to_16(wchar_t **dest,char *src);

char *utf_malloc_16_to_8(char **dest,wchar_t *src);

int utf_char_cpy(char *dest,int src);

char *utf_strcpy(char **dest,char *src);

char *utf_8_strtolwr(char *s);

FILE *utf_fopen(char *filename,char *mode);


