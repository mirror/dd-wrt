//==========================================================================
//
//      vprintf.c
//
//      Stripped down (no floating point) for debugging printf in ROMable BSP.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

/*
 * Copyright (c) 1990, 1999 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Stripped down (no floating point) for debugging printf in ROMable BSP.
 */
#include <string.h>
#include <stdarg.h>

#define	BUF		40

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double; unimplemented */
#define	LONGINT		0x010		/* long integer */
#define	QUADINT		0x020		/* quad integer */
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */


void
__vprintf(void (*putc_func)(char c), const char *fmt0, va_list ap)
{
    char *fmt;		/* format string */
    int ch;		/* character from fmt */
    int n, m;		/* handy integers (short term usage) */
    char *cp;		/* handy char pointer (short term usage) */
    int flags;		/* flags as above */
    int width;			/* width from format (%8d), or 0 */
    int prec;			/* precision from format (%.3d), or -1 */
    char sign;			/* sign prefix (' ', '+', '-', or \0) */
    unsigned long _uquad;
    enum {OCT, DEC, HEX} base;  /* base for [diouxX] conversion */
    int dprec;			/* a copy of prec if [diouxX], 0 otherwise */
    int realsz;			/* field size expanded by dprec */
    int size;			/* size of converted field or string */
    char *xdigs = NULL;		/* digits for [xX] conversion */
    char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
    char ox[2];			/* space for 0x hex-prefix */

#define	PRINT(ptr, len) {         \
        for(n=0;n<(len);n++) {    \
	  if((ptr)[n] == '\n')    \
            (*putc_func)('\r');   \
	  (*putc_func)((ptr)[n]); \
	} \
}

#define	PAD(howmany, with) {        \
	if ((n = (howmany)) > 0) {  \
	    while (n--)             \
		(*putc_func)(with); \
	} \
}

    /*
     * To extend shorts properly, we need both signed and unsigned
     * argument extraction methods.
     */
#define	SARG() \
        (flags&LONGINT ? va_arg(ap, long) : \
	    flags&SHORTINT ? (long)(short)va_arg(ap, int) : \
	    (long)va_arg(ap, int))

#define	UARG() \
	(flags&LONGINT ? va_arg(ap, unsigned long) : \
	    flags&SHORTINT ? (unsigned long)(unsigned short)va_arg(ap, int) : \
	    (unsigned long)va_arg(ap, unsigned int))

    fmt = (char *)fmt0;

    /*
     * Scan the format for conversions (`%' character).
     */
    for (;;) {
	cp = fmt;
	while (*fmt && *fmt != '%')
	    fmt++;
	if ((m = fmt - cp) != 0) {
	    PRINT(cp, m);
	}

	if (*fmt)
	    fmt++;		/* skip over '%' */
	else
	    goto done;

	flags = 0;
	dprec = 0;
	width = 0;
	prec = -1;
	sign = '\0';

    rflag:
	ch = *fmt++;
    reswitch:
	switch (ch) {
	  case ' ':
	    /*
	     * ``If the space and + flags both appear, the space
	     * flag will be ignored.''
	     *	-- ANSI X3J11
	     */
	    if (!sign)
		sign = ' ';
	    goto rflag;
	  case '#':
	    flags |= ALT;
	    goto rflag;
	  case '*':
	    /*
	     * ``A negative field width argument is taken as a
	     * - flag followed by a positive field width.''
	     *	-- ANSI X3J11
	     * They don't exclude field widths read from args.
	     */
	    if ((width = va_arg(ap, int)) >= 0)
		goto rflag;
	    width = -width;
	    /* FALLTHROUGH */
	  case '-':
	    flags |= LADJUST;
	    goto rflag;
	  case '+':
	    sign = '+';
	    goto rflag;
	  case '.':
	    if ((ch = *fmt++) == '*') {
		n = va_arg(ap, int);
		prec = n < 0 ? -1 : n;
		goto rflag;
	    }
	    n = 0;
	    while (is_digit(ch)) {
		n = 10 * n + to_digit(ch);
		ch = *fmt++;
	    }
	    prec = n < 0 ? -1 : n;
	    goto reswitch;
	  case '0':
	    /*
	     * ``Note that 0 is taken as a flag, not as the
	     * beginning of a field width.''
	     *	-- ANSI X3J11
	     */
	    flags |= ZEROPAD;
	    goto rflag;
	  case '1': case '2': case '3': case '4':
	  case '5': case '6': case '7': case '8': case '9':
	    n = 0;
	    do {
		n = 10 * n + to_digit(ch);
		ch = *fmt++;
	    } while (is_digit(ch));
	    width = n;
	    goto reswitch;
	  case 'h':
	    flags |= SHORTINT;
	    goto rflag;
	  case 'l':
	    if (*fmt == 'l') {
		fmt++;
		flags |= QUADINT;
	    } else {
		flags |= LONGINT;
	    }
	    goto rflag;
	  case 'c':
	    *(cp = buf) = va_arg(ap, int);
	    size = 1;
	    sign = '\0';
	    break;
	  case 'd':
	  case 'i':
	    _uquad = SARG();
	    if ((long) _uquad < 0)
		{

		    _uquad = -_uquad;
		    sign = '-';
		}
	    base = DEC;
	    goto number;
	  case 'o':
	    _uquad = UARG();
	    base = OCT;
	    goto nosign;
	  case 's':
	    if ((cp = va_arg(ap, char *)) == NULL)
		cp = "(null)";
	    if (prec >= 0) {
		/*
		 * can't use strlen; can only look for the
		 * NUL in the first `prec' characters, and
		 * strlen() will go further.
		 */
		char *p = memchr(cp, 0, prec);

		if (p != NULL) {
		    size = p - cp;
		    if (size > prec)
			size = prec;
		} else
		    size = prec;
	    } else
		size = strlen(cp);
	    sign = '\0';
	    break;
	  case 'u':
	    _uquad = UARG();
	    base = DEC;
	    goto nosign;
	  case 'X':
	    xdigs = "0123456789ABCDEF";
	    goto hex;
	  case 'x':
	    xdigs = "0123456789abcdef";
	hex:			_uquad = UARG();
	    base = HEX;
	    /* leading 0x/X only if non-zero */
	    if (flags & ALT && _uquad != 0)
		flags |= HEXPREFIX;

	    /* unsigned conversions */
	nosign:			sign = '\0';
	    /*
	     * ``... diouXx conversions ... if a precision is
	     * specified, the 0 flag will be ignored.''
	     *	-- ANSI X3J11
	     */
	number:			if ((dprec = prec) >= 0)
	    flags &= ~ZEROPAD;

	    /*
	     * ``The result of converting a zero value with an
	     * explicit precision of zero is no characters.''
	     *	-- ANSI X3J11
	     */
	    cp = buf + BUF;
	    if (_uquad != 0 || prec != 0) {
		/*
		 * Unsigned mod is hard, and unsigned mod
		 * by a constant is easier than that by
		 * a variable; hence this switch.
		  */
		switch (base) {
		  case OCT:
		    do {
			*--cp = to_char(_uquad & 7);
			_uquad >>= 3;
		    } while (_uquad);
		    /* handle octal leading 0 */
		    if (flags & ALT && *cp != '0')
			*--cp = '0';
		    break;

		  case DEC:
		    /* many numbers are 1 digit */
		    while (_uquad >= 10) {
			*--cp = to_char(_uquad % 10);
			_uquad /= 10;
		    }
		    *--cp = to_char(_uquad);
		    break;

		  case HEX:
		    do {
			*--cp = xdigs[_uquad & 15];
			_uquad >>= 4;
		    } while (_uquad);
		    break;

		  default:
		    cp = "bug in vfprintf: bad base";
		    size = strlen(cp);
		    goto skipsize;
		}
	    }
	    size = buf + BUF - cp;
	skipsize:
	    break;
	  default:	/* "%?" prints ?, unless ? is NUL */
	    if (ch == '\0')
		goto done;
	    /* pretend it was %c with argument ch */
	    cp = buf;
	    *cp = ch;
	    size = 1;
	    sign = '\0';
	    break;
	}

	/*
	 * All reasonable formats wind up here.  At this point, `cp'
	 * points to a string which (if not flags&LADJUST) should be
	 * padded out to `width' places.  If flags&ZEROPAD, it should
	 * first be prefixed by any sign or other prefix; otherwise,
	 * it should be blank padded before the prefix is emitted.
	 * After any left-hand padding and prefixing, emit zeroes
	 * required by a decimal [diouxX] precision, then print the
	 * string proper, then emit zeroes required by any leftover
	 * floating precision; finally, if LADJUST, pad with blanks.
	 *
	 * Compute actual size, so we know how much to pad.
	 * size excludes decimal prec; realsz includes it.
	 */
	realsz = dprec > size ? dprec : size;
	if (sign)
	    realsz++;
	else if (flags & HEXPREFIX)
	    realsz+= 2;

	/* right-adjusting blank padding */
	if ((flags & (LADJUST|ZEROPAD)) == 0)
	    PAD(width - realsz, ' ');

	/* prefix */
	if (sign) {
	    PRINT(&sign, 1);
	} else if (flags & HEXPREFIX) {
	    ox[0] = '0';
	    ox[1] = ch;
	    PRINT(ox, 2);
	}

	/* right-adjusting zero padding */
	if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
	    PAD(width - realsz, '0');

	/* leading zeroes from decimal precision */
	PAD(dprec - size, '0');

	/* the string or number proper */
	PRINT(cp, size);

	/* left-adjusting padding (always blank) */
	if (flags & LADJUST)
	    PAD(width - realsz, ' ');
    }
 done:
}


