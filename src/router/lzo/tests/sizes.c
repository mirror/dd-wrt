/* sizes.c -- print sizes of various types

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */


#include <lzoconf.h>
#include <stdio.h>


union _lzo_align1_t
{
	char	a_char;
};

struct _lzo_align2_t
{
	char	a_char;
};

struct _lzo_align3_t
{
	char	a_char;
	long	a_long;
};

struct _lzo_align4_t
{
	char	a_char;
	char *	a_char_p;
};

struct _lzo_align5_t
{
	char	a_char1;
	long	a_long;
	char	a_char2;
	char *	a_char_p;
};

union _lzo_align6_t
{
	char	a_char;
	long	a_long;
	char *	a_char_p;
	lzo_bytep   a_lzobytep;
};


#define print_size(type) \
	sprintf(s,"sizeof(%s)",#type); \
	printf("%-30s %2d\n", s, (int)sizeof(type));

#define print_ssize(type,m) \
	sprintf(s,"sizeof(%s)",#type); \
	printf("%-30s %2d %20ld\n", s, (int)sizeof(type), (long)(m));

#define print_usize(type,m) \
	sprintf(s,"sizeof(%s)",#type); \
	printf("%-30s %2d %20lu\n", s, (int)sizeof(type), (unsigned long)(m));


int main()
{
	char s[80];

	print_ssize(char,CHAR_MAX);
	print_usize(unsigned char,UCHAR_MAX);
	print_ssize(short,SHRT_MAX);
	print_usize(unsigned short,USHRT_MAX);
	print_ssize(int,INT_MAX);
	print_usize(unsigned int,UINT_MAX);
	print_ssize(long,LONG_MAX);
	print_usize(unsigned long,ULONG_MAX);
	printf("\n");
	print_size(char *);
	print_size(void (*)(void));
	printf("\n");
	print_ssize(lzo_int,LZO_INT_MAX);
	print_usize(lzo_uint,LZO_UINT_MAX);
	print_usize(lzo_uint32,LZO_UINT32_MAX);
	print_size(lzo_bytep);
	printf("\n");
	print_size(union _lzo_align1_t);
	print_size(struct _lzo_align2_t);
	print_size(struct _lzo_align3_t);
	print_size(struct _lzo_align4_t);
	print_size(struct _lzo_align5_t);
	print_size(union _lzo_align6_t);

	return 0;
}

/*
vi:ts=4
*/

