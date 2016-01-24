/*
 *  Copyright (C) 2003, 2004, 2005  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* hexdump in "readable" format ... note this is a bit more fleshed out than
 * some of the other examples mainly because I actually use it */

/* this is roughly equiv. to the Linux hexdump command...
% rpm -qf /usr/bin/hexdump
util-linux-2.11r-10
% hexdump -e '"%08_ax:"
            " " 2/1 "%02X"
            " " 2/1 "%02X"
            " " 2/1 "%02X"
            " " 2/1 "%02X"
            " " 2/1 "%02X"
            " " 2/1 "%02X"
            " " 2/1 "%02X"
            " " 2/1 "%02X"'
        -e '"  " 16 "%_p" "\n"'

       
 * ...except that it prints the address in big hex digits, and it doesn't take
 * you 30 minutes to remember how to type it out.
 *  It also acts differently in that seperate files aren't merged
 * into one output line (Ie. in this version each file starts on a new line,
 * however the addresses are continuious).

 * It's also similar to "xxd" in vim, and "od -tx1z -Ax".
 */
#define EX_UTILS_NO_FUNCS 1
#include "ex_utils.h"

#include "hexdump.h"

/* number of characters we output per line (assumes 80 char width screen)... */
#define CHRS_PER_LINE 16

#ifndef CONF_USE_FAST_NUM_PRINT
#define CONF_USE_FAST_NUM_PRINT 1
#endif

#define APOS() (apos + ((s1)->len - orig_len))

#if !CONF_USE_FAST_NUM_PRINT
/* simple print of a number */

/* print the address */
# define EX_HEXDUMP_X8(s1, num) \
    vstr_add_fmt(s1, APOS(), "0x%08X:", (num))
/* print a set of two bytes */
# define EX_HEXDUMP_X2X2(s1, num1, num2) \
    vstr_add_fmt(s1, APOS(), " %02X%02X", (num1), (num2))
/* print a byte and spaces for the missing byte */
# define EX_HEXDUMP_X2__(s1, num1) \
    vstr_add_fmt(s1, APOS(), " %02X  ",   (num1))
#else
/* fast print of a number */
static const char *hexnums = "0123456789ABCDEF";

# define EX_HEXDUMP_BYTE(buf, b) do {           \
      (buf)[1] = hexnums[((b) >> 0) & 0xf];     \
      (buf)[0] = hexnums[((b) >> 4) & 0xf];     \
    } while (FALSE)

# define EX_HEXDUMP_UINT(buf, i) do {           \
      EX_HEXDUMP_BYTE((buf) + 6, (i) >>  0);    \
      EX_HEXDUMP_BYTE((buf) + 4, (i) >>  8);    \
      EX_HEXDUMP_BYTE((buf) + 2, (i) >> 16);    \
      EX_HEXDUMP_BYTE((buf) + 0, (i) >> 24);    \
    } while (FALSE)

/* print the address */
# define EX_HEXDUMP_X8(s1, num) do {                                    \
      unsigned char xbuf[9];                                            \
                                                                        \
      xbuf[8] = ':';                                                    \
      EX_HEXDUMP_UINT(xbuf, num);                                       \
      vstr_add_buf(s1, APOS(), xbuf, sizeof(xbuf));                     \
    } while (FALSE)
/* print a set of two bytes */
# define EX_HEXDUMP_X2X2(s1, num1, num2) do {                        \
      unsigned char xbuf[5];                                         \
                                                                     \
      xbuf[0] = ' ';                                                 \
      EX_HEXDUMP_BYTE(xbuf + 3, num2);                               \
      EX_HEXDUMP_BYTE(xbuf + 1, num1);                               \
      vstr_add_buf(s1, APOS(), xbuf, sizeof(xbuf));                  \
    } while (FALSE)
/* print a byte and spaces for the missing byte */
# define EX_HEXDUMP_X2__(s1, num1) do {                                 \
      unsigned char xbuf[5];                                            \
                                                                        \
      xbuf[4] = ' ';                                                    \
      xbuf[3] = ' ';                                                    \
      EX_HEXDUMP_BYTE(xbuf + 1, num1);                                  \
      xbuf[0] = ' ';                                                    \
      vstr_add_buf(s1, APOS(), xbuf, sizeof(xbuf));                     \
    } while (FALSE)
#endif

static unsigned int addr = 0;

void ex_hexdump_reset(void)
{
  addr = 0;
}

int ex_hexdump_process(Vstr_base *s1, size_t apos,
                       Vstr_base *s2, size_t fpos, size_t flen,
                       unsigned int prnt_type, size_t max_sz, int del, int last)
{
  size_t orig_len = s1->len;
  /* normal ASCII chars, just allow COMMA and DOT flags */
  unsigned int flags = VSTR_FLAG02(CONV_UNPRINTABLE_ALLOW, COMMA, DOT);
  /* allow spaces, allow COMMA, DOT, underbar _, and space */
  unsigned int flags_sp = VSTR_FLAG04(CONV_UNPRINTABLE_ALLOW,
                                      COMMA, DOT, _, SP);
  /* high ascii too, allow
   * COMMA, DOT, underbar _, space, high space and other high characters */
  unsigned int flags_hsp = VSTR_FLAG06(CONV_UNPRINTABLE_ALLOW,
                                       COMMA, DOT, _, SP, HSP, HIGH);
  unsigned char buf[CHRS_PER_LINE];

  switch (prnt_type)
  {
    case PRNT_HIGH: flags = flags_hsp; break;
    case PRNT_SPAC: flags = flags_sp;  break;
    case PRNT_NONE:                    break;
    default: ASSERT(FALSE);            break;
  }

  /* we don't want to create more data, if we are over our limit */
  if (s1->len > max_sz)
    return (FALSE);

  /* while we have a hexdump line ... */
  while (flen >= CHRS_PER_LINE)
  {
    unsigned int count = 0;
    size_t tmp = 0;
    
    /* get a hexdump line from the vstr */
    vstr_export_buf(s2, fpos, CHRS_PER_LINE, buf, sizeof(buf));

    /* write out a hexdump line address */
    EX_HEXDUMP_X8(s1, addr);

    /* write out hex values */
    while (count < CHRS_PER_LINE)
    {
      EX_HEXDUMP_X2X2(s1, buf[count], buf[count + 1]);
      count += 2;
    }

    vstr_add_rep_chr(s1, APOS(), ' ', 2);

    /* write out characters, converting reference and pointer nodes to
     * _BUF nodes */
    tmp = APOS();
    if (vstr_add_vstr(s1, tmp, s2, fpos, CHRS_PER_LINE,
                      VSTR_TYPE_ADD_ALL_BUF))
      /* convert unprintable characters to the '.' character */
      vstr_conv_unprintable_chr(s1, tmp + 1, CHRS_PER_LINE, flags, '.');

    vstr_add_rep_chr(s1, APOS(), '\n', 1);

    addr += CHRS_PER_LINE;
    
    flen -= CHRS_PER_LINE;
    if (del) /* delete the set of characters just processed */
      vstr_del(s2, fpos, CHRS_PER_LINE);
    else
      fpos += CHRS_PER_LINE;

    /* note that we don't want to create data indefinitely, so stop
     * according to in core configuration */
    if (s1->len > max_sz)
      return (TRUE);
  }

  if (last && flen)
  { /* do the same as above, but print the partial line for
     * the end of a file */
    size_t got = flen;
    size_t missing = CHRS_PER_LINE - flen;
    const unsigned char *ptr = buf;
    size_t tmp = 0;
    
    missing -= (missing % 2);
    vstr_export_buf(s2, fpos, flen, buf, sizeof(buf));

    EX_HEXDUMP_X8(s1, addr);

    while (got >= 2)
    {
      EX_HEXDUMP_X2X2(s1, ptr[0], ptr[1]);
      got -= 2;
      ptr += 2;
    }
    if (got)
    {
      EX_HEXDUMP_X2__(s1, ptr[0]);
      got -= 2;
    }

    /* Add spaces until the point where the characters should start */
    vstr_add_rep_chr(s1, APOS(), ' ', (missing * 2) + (missing / 2) + 2);

    tmp = APOS();
    if (vstr_add_vstr(s1, tmp, s2, fpos, flen, VSTR_TYPE_ADD_ALL_BUF))
      vstr_conv_unprintable_chr(s1, tmp + 1, flen, flags, '.');

    vstr_add_cstr_buf(s1, APOS(), "\n");

    addr += flen;
    if (del)
      vstr_del(s2, fpos, flen);

    return (TRUE);
  }

  return (FALSE);
}
