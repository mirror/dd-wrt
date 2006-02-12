/*
 * Initialization and support routines for self-booting
 * compressed image.
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <typedefs.h>
#include <stdarg.h>
#include <osl.h>
#include <bcmutils.h>

static const char digits[17] = "0123456789ABCDEF";
static const char ldigits[17] = "0123456789abcdef";

static int
__atox(char *buf, unsigned int num, unsigned int radix, int width,
       const char *digits)
{
	char buffer[16];
	char *op;
	int retval;

	op = &buffer[0];
	retval = 0;

	do {
		*op++ = digits[num % radix];
		retval++;
		num /= radix;
	} while (num != 0);

	if (width && (width > retval)) {
		width = width - retval;
		while (width) {
			*op++ = '0';
			retval++;
			width--;
		}
	}

	while (op != buffer) {
		op--;
		*buf++ = *op;
	}

	return retval;
}

#define isdigit(x) (((x) >= '0') && ((x) <= '9'))

extern int vsprintf(char *buf, const char *fmt, va_list ap);

int
vsprintf(char *buf, const char *fmt, va_list ap)
{
	char *optr;
	const char *iptr;
	unsigned char *tmpptr;
	unsigned int x;
	int i;
	int leadingzero;
	int leadingnegsign;
	int islong;
	int width;
	int width2 = 0;
	int hashash = 0;

	optr = buf;
	iptr = fmt;

	while (*iptr) {
		if (*iptr != '%') {
			*optr++ = *iptr++;
			continue;
		}

		iptr++;

		if (*iptr == '#') {
			hashash = 1;
			iptr++;
		}
		if (*iptr == '-') {
			leadingnegsign = 1;
			iptr++;
		} else
			leadingnegsign = 0;

		if (*iptr == '0')
			leadingzero = 1;
		else
			leadingzero = 0;

		width = 0;
		while (*iptr && isdigit(*iptr)) {
			width += (*iptr - '0');
			iptr++;
			if (isdigit(*iptr))
				width *= 10;
		}
		if (*iptr == '.') {
			iptr++;
			width2 = 0;
			while (*iptr && isdigit(*iptr)) {
				width2 += (*iptr - '0');
				iptr++;
				if (isdigit(*iptr)) width2 *= 10;
			}
		}

		islong = 0;
		if (*iptr == 'l') {
			islong++;
			iptr++;
		}

		switch (*iptr) {
		case 's':
			tmpptr = (unsigned char *) va_arg(ap, unsigned char *);
			if (!tmpptr) tmpptr = (unsigned char *) "(null)";
			if ((width == 0) & (width2 == 0)) {
				while (*tmpptr) *optr++ = *tmpptr++;
				break;
			}
			while (width && *tmpptr) {
				*optr++ = *tmpptr++;
				width--;
			}
			while (width) {
				*optr++ = ' ';
				width--;
			}
			break;
		case 'd':
			i = va_arg(ap, int);
			if (i < 0) { *optr++='-'; i = -i;}
			optr += __atox(optr, i, 10, width, digits);
			break;
		case 'u':
			x = va_arg(ap, unsigned int);
			optr += __atox(optr, x, 10, width, digits);
			break;
		case 'X':
		case 'x':
			x = va_arg(ap, unsigned int);
			optr += __atox(optr, x, 16, width,
				       (*iptr == 'X') ? digits : ldigits);
			break;
		case 'p':
		case 'P':
			x = va_arg(ap, unsigned int);
			optr += __atox(optr, x, 16, 8,
				       (*iptr == 'P') ? digits : ldigits);
			break;
		case 'c':
			x = va_arg(ap, int);
			*optr++ = x & 0xff;
			break;

		default:
			*optr++ = *iptr;
			break;
		}
		iptr++;
	}

	*optr = '\0';

	return (optr - buf);
}

int
sprintf(char *buf, const char *fmt, ...)
{
	va_list ap;
	int count;

	va_start(ap, fmt);
	count = vsprintf(buf, fmt, ap);
	va_end(ap);

	return count;
}

int
printf(const char *fmt, ...)
{
	va_list ap;
	int count, i;
	char buffer[512];

	va_start(ap, fmt);
	count = vsprintf(buffer, fmt, ap);
	va_end(ap);

	for (i = 0; i < count; i++) {
		putc(buffer[i]);
	}

	return count;
}

void *
memset(void *dest, int c, uint n)
{
	unsigned char *d;

	d = (unsigned char *)dest;

	while (n) {
		*d++ = (unsigned char) c;
		n--;
	}

	return d;
}

void *
memcpy(void *dest, const void *src, uint n)
{
	unsigned char *d;
	const unsigned char *s;

	d = (unsigned char *)dest;
	s = (const unsigned char *)src;

	while (n) {
		*d++ = *s++;
		n--;
	}

	return dest;
}

int
memcmp(const void *s1, const void *s2, uint n)
{
	const unsigned char *ss1;
	const unsigned char *ss2;

	ss1 = (const unsigned char *)s1;
	ss2 = (const unsigned char *)s2;

	while (n) {
		if (*ss1 < *ss2)
			return -1;
		if (*ss1 > *ss2)
			return 1;
		ss1++;
		ss2++;
		n--;
	}

	return 0;
}

char *
strcpy(char *dest, const char *src)
{
	char *ptr = dest;

	while (*src)
		*ptr++ = *src++;
	*ptr = '\0';

	return dest;
}

char *
strncpy(char *dest, const char *src, uint n)
{
	char *ptr = dest;

	while (*src && (n > 0)) {
		*ptr++ = *src++;
		n--;
	}
	if (n > 0)
		*ptr = '\0';

	return dest;
}

uint
strlen(const char *s)
{
	uint n = 0;

	while (*s) {
		s++;
		n++;
	}

	return n;
}

int
strcmp(const char *s1, const char *s2)
{
	while (*s2 && *s1) {
		if (*s1 < *s2)
			return -1;
		if (*s1 > *s2)
			return 1;
		s1++;	
		s2++;
	}

	if (*s1 && !*s2)
		return 1;
	if (!*s1 && *s2)
		return -1;
	return 0;
}

int
strncmp(const char *s1, const char *s2, uint n)
{
	while (*s2 && *s1 && n) {
		if (*s1 < *s2)
			return -1;
		if (*s1 > *s2)
			return 1;
		s1++;	
		s2++;
		n--;
	}

	if (!n)
		return 0; 
	if (*s1 && !*s2)
		return 1;
	if (!*s1 && *s2)
		return -1;
	return 0;
}

char *
strchr(const char *str,int c)
{
	char *x = (char *)str; 

	while (*x != (char)c) {
		if (*x++ == '\0')
		return (NULL);
	}
	return (x);
}

char * 
strrchr(const char *str,int c)
{
    char *save = NULL;

    do {
		if (*str == (char) c)
			save = (char*)(str);		  
    } while (*str++ != '\0');

    return (save);
}

char *
strcat(char *d, const char *s)
{
	char *x, *ss = (char *)s;

	x = &d[strlen(d)];
	while ((*x++ = *ss++))
		;

	return (d);
}
