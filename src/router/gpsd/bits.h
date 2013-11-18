/*
 * bits.h - extract binary data from message buffer
 *
 * These macros extract bytes, words, longwords, floats, doubles, or
 * bitfields of arbitrary length and size from a message that contains
 * these items in either MSB-first or LSB-first byte order.
 *
 * We enforce data sizes of integral types in the casts on these.
 * Both 32- and 64-bit systems with gcc are OK with this set.
 *
 * This file is Copyright (c)2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#ifndef _GPSD_BITS_H_
#define _GPSD_BITS_H_

#include <stdint.h>

/* these are independent of byte order */
#define getsb(buf, off)	((int8_t)buf[off])
#define getub(buf, off)	((uint8_t)buf[off])
#define putbyte(buf,off,b) do {buf[off] = (unsigned char)(b);} while (0)

/* little-endian access */
#define getles16(buf, off)	((int16_t)(((uint16_t)getub((buf),   (off)+1) << 8) | (uint16_t)getub((buf), (off))))
#define getleu16(buf, off)	((uint16_t)(((uint16_t)getub((buf), (off)+1) << 8) | (uint16_t)getub((buf), (off))))
#define getles32(buf, off)	((int32_t)(((uint16_t)getleu16((buf),  (off)+2) << 16) | (uint16_t)getleu16((buf), (off))))
#define getleu32(buf, off)	((uint32_t)(((uint16_t)getleu16((buf),(off)+2) << 16) | (uint16_t)getleu16((buf), (off))))
#define getles64(buf, off)	((int64_t)(((uint64_t)getleu32(buf, (off)+4) << 32) | getleu32(buf, (off))))
#define getleu64(buf, off)	((uint64_t)(((uint64_t)getleu32(buf, (off)+4) << 32) | getleu32(buf, (off))))
extern float getlef32(const char *, int);
extern double getled64(const char *, int);

#define putle16(buf, off, w) do {putbyte(buf, (off)+1, (uint)(w) >> 8); putbyte(buf, (off), (w));} while (0)
#define putle32(buf, off, l) do {putle16(buf, (off)+2, (uint)(l) >> 16); putle16(buf, (off), (l));} while (0)

/* big-endian access */
#define getbes16(buf, off)	((int16_t)(((uint16_t)getub(buf, (off)) << 8) | (uint16_t)getub(buf, (off)+1)))
#define getbeu16(buf, off)	((uint16_t)(((uint16_t)getub(buf, (off)) << 8) | (uint16_t)getub(buf, (off)+1)))
#define getbes32(buf, off)	((int32_t)(((uint16_t)getbeu16(buf, (off)) << 16) | getbeu16(buf, (off)+2)))
#define getbeu32(buf, off)	((uint32_t)(((uint16_t)getbeu16(buf, (off)) << 16) | getbeu16(buf, (off)+2)))
#define getbes64(buf, off)	((int64_t)(((uint64_t)getbeu32(buf, (off)) << 32) | getbeu32(buf, (off)+4)))
#define getbeu64(buf, off)	((uint64_t)(((uint64_t)getbeu32(buf, (off)) << 32) | getbeu32(buf, (off)+4)))
extern float getbef32(const char *, int);
extern double getbed64(const char *, int);

#define putbe16(buf,off,w) do {putbyte(buf, (off), (w) >> 8); putbyte(buf, (off)+1, (w));} while (0)
#define putbe32(buf,off,l) do {putbe16(buf, (off), (l) >> 16); putbe16(buf, (off)+2, (l));} while (0)

extern void putbef32(char *, int, float);

/* bitfield extraction */
extern uint64_t ubits(unsigned char buf[], unsigned int, unsigned int, bool);
extern int64_t sbits(signed char buf[], unsigned int, unsigned int, bool);

#endif /* _GPSD_BITS_H_ */
