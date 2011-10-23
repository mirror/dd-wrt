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
	Some unicode crossplatform compatibility helpers, file reading, uri encoding

	Author: bhoover@wecs.com
	Date: Jan 2008

	History:
		- first implemetnation
*/
#define MODULE_TAG "UNICODE_UTIL: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#include <winnls.h>

#endif

#include "unicode_util.h"
#include "debug_if.h"
#include "numbers.h"
#include "safe_mem.h"


#ifdef _WIN32

static int isWinNT();

static char *do_utf_16_to_8(char *dest,wchar_t *src,int srcChars,int destBytes)
{

	memset(dest,0,destBytes);


	if (!(WideCharToMultiByte(CP_UTF8,0,src,srcChars,dest,destBytes,NULL,NULL)))

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "WideCharToMultiByte failed in do_utf_16_to_8",GetLastError(),LOG_ERR,LOG_DEBUG);

	return dest;
}

static wchar_t *do_utf_8_to_16(wchar_t *dest,char *src,int srcBytes,int destChars)
{

	memset(dest,0,destChars*sizeof(wchar_t));


	if (!(MultiByteToWideChar(CP_UTF8,0,src,srcBytes,dest,destChars)))

		dbg_printf_and_err_str(NULL,"E:" MODULE_TAG "MultiByteToWideChar failed in do_utf_8_to_16",GetLastError(),LOG_ERR,LOG_DEBUG);


	return dest;
}

static int do_is_u_16(FILE *p_file)
{

	char	buff[2048];
	char	c=0;
	int		count=0;
	int		is_unicode_statistics=IS_TEXT_UNICODE_STATISTICS;


	memset(buff,0,2048);


	while(count<2046) {

		c=fgetc(p_file);

		if (feof(p_file))

			break;

		buff[count++]=c;
	}

	buff[count++]='\0';
	buff[count]='\0';

	return IsTextUnicode(buff,count+1,&is_unicode_statistics);
}

#endif

static int do_utf_is_bom_16(unsigned char *ch)
{

	int retVal=0;


	if (ch[0]==0xfe && ch[1]==0xff)

		retVal=-1;

	else

		if (ch[0]==0xff && ch[1]==0xfe)

			retVal=1;


	return retVal;
}

static void do_utf_8_get_char(char *ch,unsigned int cp,int *byte_count)
{
	unsigned char	enc_byte;
	int				byte_index;


	byte_index=*byte_count;


	/*right most 6 bits*/
	enc_byte=(unsigned char) (cp & 63);

	/*meta '10'*/
	enc_byte|=128;

	/*done with these*/
	cp=cp>>6;


	(*byte_count)++;


	if (cp)

		do_utf_8_get_char(ch,cp,byte_count);


	ch[(*byte_count)-byte_index-1]=enc_byte;
}

static void utf_8_encode_size(char *ch,int size)
{

	switch(size)
	{

	case 2:

		ch[0]|=192;

		break;

	case 3:

		ch[0]|=224;

		break;

	case 4:

		ch[0]|=240;

		break;

	case 5:

		ch[0]|=248;

		break;

	case 6:

		ch[0]|=252;

		break;
	}
}

/*********PUBLIC*********/

int utf_8_encoded_size(unsigned char ch)
{

	if (ch<128)

		return 1;

	if (ch>=252)

		return 6;

	if (ch>=248)

		return 5;

	if (ch>=240)

		return 4;

	if (ch>=224)

		return 3;

	if (ch>=192)

		return 2;


	return 0;
}

int utf_8_len(unsigned char *s)
{
	int				i=0;
	int				numBytes;
	int				numCharBytes;
	int				len=0;



	if (!(s))

		return 0;


	numBytes=strlen((char *) s);


	while (i<numBytes) {

		if (!(numCharBytes=utf_8_encoded_size(s[i])))

			return 0;

		i+=numCharBytes;

		len++;
	}

	return len;
}

int utf_8_get_char(char *ch,unsigned int cp)
{

	int	byte_count=0;


	do_utf_8_get_char(ch,cp,&byte_count);

	utf_8_encode_size(ch,byte_count);


	return byte_count;
}

char *utf_8_char(char *ch,unsigned int cp)
{

	utf_8_get_char(ch,cp);


	return ch;
}

int utf_8_get_code_point(unsigned char *ch,unsigned int *cp)
{

	int				i;
	int				numBytes;


	numBytes=utf_8_encoded_size(ch[0]);


	if (numBytes==0) {

		*cp=0;
	}
	else {

		if (numBytes==1) {

			*cp=ch[0];

		}
		else {

			/*mask out bytes count bits, and make room for rest of character*/

			*cp=(ch[0] & (255>>numBytes))<<((numBytes-1)*6);

			for (i=1;i<numBytes;i++) {

				/*mask out (& 0011 1111) meta bits (first two), and make room*/

				*cp+=((ch[i] & 63)<<((numBytes-1-i)*6));
			}
		}
	}


	return numBytes;
}

unsigned int utf_8_code_point(unsigned char *ch)
{

	unsigned int cp;


	utf_8_get_code_point(ch,&cp);


	return cp;
}

char *utf_8_uri_encoded(char **dest,char *src,char *prefix,char *suffix)
{
	int				len;
	int				srcIndex=0;
	unsigned long	ch;
	char			cp_buff[12];


	len=strlen(src);

	*dest=safe_malloc(len*3+len*UTF_8_SIZE+1); /*add potential 2 prefix, 1 suffix chars*/


	while (srcIndex<len) {

		ch=utf_8_code_point((unsigned char *) (src+srcIndex));


		if (!(ch==0)) {


			if (ch==(unsigned char) src[srcIndex]) {


				(*dest)[strlen(*dest)]=src[srcIndex];

			}
			else {


				strcat(*dest,prefix);

				strcat(*dest,numStr(ch,cp_buff));

				strcat(*dest,suffix);
			}
		}

		srcIndex++;
	}

	return *dest;
}

char *utf_read_utf_8(char *c_buff,FILE *in_file)
{
	int  numBytes;
	int  i;
	char *p_c_buff=c_buff;
	char c=0;


	if (feof(in_file))

		return NULL;

	else {

		c=getc(in_file);

		if (feof(in_file))

			return NULL;


		c_buff[0]=c;

		numBytes=utf_8_encoded_size((unsigned char) c_buff[0])-1;

		if (numBytes<0)

			return NULL;

		p_c_buff++;

		for (i=0;i<numBytes;i++) {

			c=getc(in_file);

			if (feof(in_file))

				return NULL;

			p_c_buff[i]=c;

		}
	}

	return c_buff;
}

int utf_is_bom_16(FILE *p_file)
{
	unsigned char ch[2];
	int           retVal=0;
	unsigned long fPos;


	if (p_file) {

		fPos=ftell(p_file);

		fseek(p_file,SEEK_SET,0);


		ch[0]=fgetc(p_file);
		ch[1]=fgetc(p_file);


		fseek(p_file,fPos,0);


		retVal=do_utf_is_bom_16(ch);
	}


	return retVal;
}

int utf_is_bom_8(FILE *p_file)
{

	int				retVal=0;
	char			ch_utf_8_buff[7];
	char			ch_buff[2];
	unsigned int	cp;


	if (p_file) {

		memset(ch_utf_8_buff,0,7);


		if (utf_read_utf_8(ch_utf_8_buff,p_file)) {

			cp=utf_8_code_point((unsigned char *) ch_utf_8_buff);

			ch_buff[0]=((cp >> 8) & 0xff);

			ch_buff[1]=(cp & 0xff);

			retVal=do_utf_is_bom_16((unsigned char *) ch_buff);
		}
	}

	return retVal;
}

int utf_is_u_16_ascii(wchar_t *utf_16)
{
	int   len;
	int   i=0;
	char  *ch;


	len=wcslen(utf_16)*UTF_16_SIZE;

	ch=(char *) utf_16;


	while (i<len) {

		if ((unsigned char) ch[i++]>127)

			return 0;

		if ((unsigned char) ch[i++])

			return 0;
	}

	return 1;
}

int utf_is_utf_8_wide(unsigned char *s)
{

	int				i=0;
	int				numBytes;
	int				numCharBytes;


	numBytes=strlen((char *) s);

	while (i<numBytes) {

		if (!(numCharBytes=utf_8_encoded_size(s[i])))

			return 0;

		if (numCharBytes>1)

			return 1;

		i+=numCharBytes;
	}

	return 0;
}

int utf_char_cpy(char *dest,int src)
{

	int	len_cpy=1;


	if (!(src>127))

		(*dest)=(char) src;

	else {

		len_cpy=utf_8_get_char(dest,src);

		dest[len_cpy]='\0';
	}


	return len_cpy;
}

/*	return pointer to utf_8 character before byte_index
*/
char *utf_prev_char(char *s,int byte_index)
{

	char	*this_s=s+byte_index;


	while(!(s==this_s--)) {

		if (utf_8_encoded_size((unsigned char) *this_s))

			return this_s;
	}

	return NULL;
}

/*	return pointer to utf_8 character after byte_index
*/
char *utf_next_char(char *s,int byte_index)
{

	s+=byte_index;


	while (*s++) {

		if (utf_8_encoded_size((unsigned char) *s))

			return s;
	}

	return NULL;
}

/*	truncate s at max_bytes-1, or the utf-8 character before
*/
char *utf_truncate(char *s,unsigned int max_bytes)
{

	char		*prev_char;


	if (!(s))

		return NULL;


	if (max_bytes > strlen(s))

		return s;

	if (utf_8_encoded_size(s[max_bytes-1])) {

		s[max_bytes-1]='\0';

		return s;
	}

	/*beyond first byte of multibyte character*/
	if (!(prev_char = utf_prev_char(s,max_bytes-1)))

		s[0]='\0';

	else

		*(s+(prev_char-s))='\0';


	return s;
}

/*	convenience function -- conditional compiles detect 
	environment, assume utf 8, utf 16 accordingly
*/
char *utf_strcpy(char **dest,char *src)
{

	int	len=strlen(src);


#ifndef _WIN32

	*dest=safe_malloc(len+1);
	strcpy(*dest,src);

#else

	if (!(isWinNT()))

  #ifndef UNICOWS

	{

		*dest=safe_malloc(len+1);
		strcpy(*dest,src);
	}

  #else

	;

  #endif

  #ifndef UNICOWS

	else

  #endif

	{
		/*Windows utf-16*/

		*dest=safe_malloc(UTF_16_SIZE*(wcslen((wchar_t *) *dest)+1));
		wcscpy((wchar_t *) *dest,(wchar_t *) src);
	}

#endif 


	return *dest;
}

char *utf_8_strtolwr(char *s)
{

	int		src_index=0;
	int		dest_index=0;
	int		bytes_copied;
	int		src_size;
	char	dest_char[7];


	if (!(s))

		return NULL;


	while(*(s+src_index)) {


		src_size=utf_8_encoded_size(s[src_index]);

		bytes_copied=utf_char_cpy(dest_char,towlower(utf_8_code_point((unsigned char *) (s+src_index))));


		memcpy((s+dest_index),dest_char,bytes_copied);

		src_index+=src_size;

		dest_index+=bytes_copied;
	}

	*(s+dest_index)='\0';


	return s;
}

FILE *utf_fopen(char *filename,char *mode)
{

	FILE	*fp=NULL;


#ifndef _WIN32

	fp=fopen(filename,mode);

#else

	if (!(isWinNT())) {

 #ifndef UNICOWS

		fp=fopen(filename,mode);
	}

  #else
		;
	}

  #endif

  #ifndef UNICOWS

	else

  #endif

	{
		wchar_t	*utf_16=NULL;
		wchar_t *utf_16_mode=NULL;


		fp=_wfopen(utf_8_to_16(utf_malloc_8_to_16(&utf_16,filename),filename),
										utf_8_to_16(utf_malloc_8_to_16(&utf_16_mode,mode),mode));

		free(utf_16);
		free(utf_16_mode);
	}
#endif

	return fp;
}

#ifdef _WIN32

static int isWinNT()
{

	return (GetVersion() < 0x80000000);
}

char *utf_malloc_16_to_8(char **dest,wchar_t *src)
{

	*dest=safe_malloc(wcslen(src)*UTF_8_SIZE+1);


	return *dest;
}

wchar_t *utf_malloc_8_to_16(wchar_t **dest,char *src)
{

	*dest=safe_malloc(utf_8_len(src)*UTF_16_SIZE+UTF_16_SIZE);


	return *dest;
}

int utf_is_win_utf_file(wchar_t *szFile,int *is_bom)
{
	FILE	*p_file=NULL;
	int		ret_val=0;
	int		count=0;


	*is_bom=0;

	p_file = _wfopen(szFile,L"r");


	if (p_file) {

		*is_bom=utf_is_bom_16(p_file);


		if (*is_bom) {

			ret_val=1;
		}
		else {

			/*see if file contains unicode*/

			ret_val=do_is_u_16(p_file);

			if (!(ret_val))

				*is_bom=utf_is_bom_8(p_file);
		}

		fclose(p_file);
	}

	return ret_val;
}

void utf_free_ar(char *utf_ar[],int size)
{

	int i;


	for (i=0;i<size;i++) {

		free(utf_ar[i]);
	}
}

wchar_t *utf_8_to_16(wchar_t *dest,char *src)
{

	return do_utf_8_to_16(dest,src,strlen(src)+1,utf_8_len(src)+1);
}

wchar_t **utf_8_to_16_ar(wchar_t *dest[],char *src[],int size)
{

	int i;
	int strLen;
	int destBytes;
	int wcharSize;

	wcharSize=sizeof(wchar_t);


	for (i=0;i<size;i++) {

		strLen=utf_8_len(src[i])+1;

		destBytes=strLen*wcharSize;

		dest[i]=safe_malloc(destBytes);


		do_utf_8_to_16(dest[i],src[i],strlen(src[i])+1,strLen);
	}

	return dest;
}

char *utf_16_to_8(char *dest,wchar_t *src)
{
	int strLen;


	strLen=wcslen(src);

	return do_utf_16_to_8(dest,src,strLen+1,strLen*UTF_8_SIZE+1);
}

char **utf_16_to_8_ar(char *dest[],wchar_t *src[],int size)
{

	int i;
	int strLen;
	int destBytes;


	for (i=0;i<size;i++) {

		strLen=wcslen(src[i]);

		destBytes=strLen*UTF_8_SIZE+1;

		dest[i]=safe_malloc(destBytes);


		do_utf_16_to_8(dest[i],src[i],strLen+1,destBytes);
	}

	return dest;
}

#endif
