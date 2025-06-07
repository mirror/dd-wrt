/*
 * escape.c - printing handling
 *
 * Copyright © 2011-2024 Jim Warner <james.warner@comcast.net>
 * Copyright © 2016-2024 Craig Small <csmall@dropbear.xyz>
 * Copyright © 1998-2005 Albert Cahalan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "escape.h"
#include "readproc.h"
#include "nls.h"

#define SECURE_ESCAPE_ARGS(dst, bytes) do { \
  if ((bytes) <= 0) return 0; \
  *(dst) = '\0'; \
  if ((bytes) >= INT_MAX) return 0; \
} while (0)


/*
 * Return the number of bytes this UTF-8 string uses
 * Compliant with RFC 3629
 *
 * Returns how many bytes or -1 if invalid
 */
// FIXME: not future-proof
static int u8charlen (const unsigned char *s, unsigned size) {
   unsigned x;

   if (! size) return 0;

   // 0xxxxxxx, U+0000 - U+007F
   if (s[0] <= 0x7f) return 1;
   if (size >= 2 && (s[1] & 0xc0) == 0x80) {
      // 110xxxxx 10xxxxxx, U+0080 - U+07FF
      if (s[0] >= 0xc2 && s[0] <= 0xdf) return 2;
      if (size >= 3 && (s[2] & 0xc0) == 0x80) {
#ifndef OFF_UNICODE_PUA
         x = ((unsigned)s[0] << 16) + ((unsigned)s[1] << 8) + (unsigned)s[2];
         /* 11101110 10000000 10000000, U+E000 - primary PUA begin
            11101111 10100011 10111111, U+F8FF - primary PUA end */
         if (x >= 0xee8080 && x <= 0xefa3bf) return -1;
#endif
         x = (unsigned)s[0] << 6 | (s[1] & 0x3f);
         // 1110xxxx 10xxxxxx 10xxxxxx, U+0800 - U+FFFF minus U+D800 - U+DFFF
         if ((x >= 0x3820 && x <= 0x3b5f) || (x >= 0x3b80 && x <= 0x3bff)) return 3;
         if (size >= 4 && (s[3] & 0xc0) == 0x80) {
#ifndef OFF_UNICODE_PUA
            unsigned y;
            y = ((unsigned)s[0] << 24) + ((unsigned)s[1] << 16) + ((unsigned)s[2] << 8) + (unsigned)s[3];
            /* 11110011 10110000 10000000 10000000, U+F0000  - supplemental PUA-A begin
               11110011 10111111 10111111 10111101, U+FFFFD  - supplemental PUA-A end */
            if (y >= 0xf3b08080 && y <= 0Xf3bfbfbd) return -1;
            /* 11110100 10000000 10000000 10000000, U+100000 - supplemental PUA-B begin
               11110100 10001111 10111111 10111101, U+10FFFD - supplemental PUA-B end */
            if (y >= 0xf4808080 && y <= 0Xf48fbfbd) return -1;
#endif
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx, U+010000 - U+10FFFF
            if (x >= 0x3c10 && x <= 0x3d0f) return 4;
         }
      }
   }

   // invalid or incomplete sequence
   return -1;
}

/*
 * Given a bad locale/corrupt str, replace all non-printing stuff
 */
static inline void esc_all (unsigned char *str) {
   while (*str) {
      if (!isprint(*str))
          *str = '?';
      ++str;
   }
}

static inline void esc_ctl (unsigned char *str, int len) {
   int n;

   if (len <= 0)
      return;

   while ((n = u8charlen(str, len) )) {
      /* Escape the character to a '?' if
       *  Not valid UTF so charlen is -1
       *  Non-control chars below SPACE
       *  DEL
       *  32 unicode multibyte control characters which begin at U+0080 (0xc280)
       */
      if (
         n < 0
         || str[0] < 0x20
         || str[0] == 0x7f
         || (str[0] == 0xc2 && str[1] >= 0x80 && str[1] <= 0x9f)) {

         *str = '?';
         n = 1;
      }
      str += n;
      len -= n;
   }
}

int escape_str (char *dst, const char *src, int bufsize) {
   static __thread int utf_sw = 0;
   int n;

   if (utf_sw == 0) {
      char *enc = nl_langinfo(CODESET);
      utf_sw = enc && strcasecmp(enc, "UTF-8") == 0 ? 1 : -1;
   }
   SECURE_ESCAPE_ARGS(dst, bufsize);
   n = snprintf(dst, bufsize, "%s", src);
   if (n < 0) {
      *dst = '\0';
      return 0;
   }
   if (n >= bufsize) n = bufsize-1;
   if (utf_sw < 0)
      esc_all((unsigned char *)dst);
   else
      esc_ctl((unsigned char *)dst, n);
   return n;
}

int escape_command (char *outbuf, const proc_t *pp, int bytes, unsigned flags) {
   int overhead = 0;
   int end = 0;

   if (flags & ESC_BRACKETS)
      overhead += 2;
   if (flags & ESC_DEFUNCT) {
      if (pp->state == 'Z') overhead += 10;    // chars in " <defunct>"
      else flags &= ~ESC_DEFUNCT;
   }
   if (overhead + 1 >= bytes) {
      // if no room for even one byte of the command name
      outbuf[0] = '\0';
      return 0;
   }
   if (flags & ESC_BRACKETS)
      outbuf[end++] = '[';
   end += escape_str(outbuf+end, pp->cmd, bytes-overhead);
   // we want "[foo] <defunct>", not "[foo <defunct>]"
   if (flags & ESC_BRACKETS)
      outbuf[end++] = ']';
   if (flags & ESC_DEFUNCT) {
      memcpy(outbuf+end, " <defunct>", 10);
      end += 10;
   }
   outbuf[end] = '\0';
   return end;  // bytes, not including the NUL
}
