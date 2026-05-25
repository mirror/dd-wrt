/* ISC license. */

#include <stdint.h>

#include <skalibs/uint16.h>
#include <skalibs/fmtscan.h>


/* IPv6 scanner

class   |       0       1       2       3
st\ev   |       other   hex     :       .

START   |               m
00      |       X       LIMB    COLON1  X

COLON1  |                       p
01      |       X       X       NEW     X

NEW     |               m
02      |       END     LIMB    X       X

LIMB    |       h               h
03      |       END     LIMB    COLON   V4

COLON   |               m       p
04      |       X       LIMB    NEW     X


END = 05
V4 = 06
X = 07

0x10    m       mark
0x20    p       set doublecolon pos
0x40    h       scan hex, next limb

*/

size_t ip6_scan (char const *s, char *ip6)
{
  static unsigned char const class[256] = "0000000000000000000000000000000000000000000000301111111111200000011111100000000000000000000000000111111000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" ;
  static uint8_t const table[5][4] =
  {
    { 0x07, 0x13, 0x01, 0x07 },
    { 0x07, 0x07, 0x22, 0x07 },
    { 0x05, 0x13, 0x07, 0x07 },
    { 0x45, 0x03, 0x44, 0x06 },
    { 0x07, 0x13, 0x22, 0x07 }
  } ;
  size_t i = 0, mark = 0 ;
  uint16_t limb[8] = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
  uint8_t pos = 8, j = 0, state = 0 ;

  for (; state < 0x05 ; i++)
  {
    uint8_t c = table[state][class[(unsigned char)s[i]] - '0'] ;
    state = c & 0x07 ;
    if (c & 0x10) mark = i ;
    if (c & 0x20) { if (pos < 8) state = 0x07 ; else pos = j ; }
    if (c & 0x40)
      if (j >= 8 || uint16_xscan(s + mark, limb + j++) != i - mark) state = 0x07 ;
  }

  switch (state)
  {
    case 0x05:
      if (pos == 8 && j < 8 || (pos < 8 && j > 6)) return 0 ;
      i-- ;
      break ;
    case 0x06:
    {
      uint32_t ip4 ;
      if (pos == 8 && j != 6 || (pos < 8 && j > 4)) return 0 ;
      i = ip4_scanu32(s + mark, &ip4) ;
      if (!i) return 0 ;
      limb[j++] = ip4 >> 16 ;
      limb[j++] = ip4 & 0xffff ;
      i += mark ;
      break ;
    }
    default : return 0 ;
  }

  for (state = j ; state > pos ; state--) limb[state - j + 7] = limb[state - 1] ;
  for (; state < pos + 8 - j ; state++) limb[state] = 0 ;
  for (j = 0 ; j < 8 ; j++) uint16_pack_big(ip6 + (j<<1), limb[j]) ;
  return i ;
}
