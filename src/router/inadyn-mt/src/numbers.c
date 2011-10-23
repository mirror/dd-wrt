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
	integer to string conversion

	Author: bhoover@wecs.com
	Date: Jan 2008

	History:
		- first implemetnation
*/
#define MODULE_TAG "NUMBERS: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdio.h>
#include <string.h>

void doNumConvert(long num,char *rStr,int *strLen,long *x)
{
	if((*x=num/10))

		doNumConvert(*x,rStr,strLen,x);

	*(rStr+(*(strLen))++)=num%10+'0';
}

int convertToString(double num,char *rStr)
{
	int    strLen=0;
	long   x;


	doNumConvert((long) num,rStr,&strLen,&x);

	*(rStr+strLen)=0;


	return strLen;
}

char *numStr(double num,char *dest)
{


	convertToString(num,dest);


	return dest;
}

int do_read_number(FILE *f,int *number,char *bytes,int *cur_byte)
{

	#define	byte_bits	8

	int		byte=0;
	int		this_cur_byte=*cur_byte;


	byte=fgetc(f);


	if (feof(f))

		return 0;


	if (++(*cur_byte)<*bytes)

		do_read_number(f,number,bytes,cur_byte);


	byte<<=this_cur_byte*byte_bits;

	*number+=byte;


	return *number;
}

int read_number(FILE *f,int *number,char bytes)
{	

	int	cur_byte=0;

	*number=0;


	return do_read_number(f,number,&bytes,&cur_byte);
}

/*implementation works well for _small_ numbers (_positive_)!*/
static double mantissa(double num)
{
	
	if (!(num) || (num<1))

		return num;

	do {

		num--;

		if (!(num))

			return 0;

		if (num<1)

			return num;

	} while(1);	
}

static double do_round_up_1(double num,double cut_off)
{

	char	sign=1;
	double	mant;


	if (!(num))

		return 0;	

	if (num<0) { /*operate on abs value of num*/

		sign=-1;

		num=num*sign;
	}

	if ((mant=mantissa(num))) {

		if (sign*mant>=sign*cut_off) {

			num-=mant;

			if (!(sign==-1))

				num++;
		}
	}


	return sign*num;
}

static double do_round_up(double num,double cut_off,char *decimal_sep)
{
	double	num_store=num;	
	char	str[1081];
	char	*dec_point=NULL;	


	memset(str,0,1081);


#ifndef _WIN32
	snprintf(str,1080,"%f",num);
#else
	_snprintf(str,1080,"%f",num);
#endif

	if ((dec_point=strstr(str,decimal_sep))) {

		*(dec_point)='\0';

		sscanf(str,"%lf",&num);

		if (num_store<0) {
			
			if ((num_store-num)>=-cut_off) {

				return num;
			}
			else {

				return num_store;
			}
		}

		if (num_store>num) {

			if ((num_store-num)>=cut_off) {

				num++;
			}
			else {

				return num_store;
			}
		}
	}

	return num;
}

double round_up(double num,double cut_off)
{

	return do_round_up_1(num,cut_off);
}

