/*
 * qrencode - QR Code encoder
 *
 * Binary sequence class.
 * Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef BITSTREAM_H
#define BITSTREAM_H

typedef struct {
	size_t length;
	size_t datasize;
	unsigned char *data;
} BitStream;

static BitStream *BitStream_new(void);
#ifdef WITH_TESTS
static BitStream *BitStream_newWithBits(size_t size, unsigned char *bits);
#endif
static int BitStream_append(BitStream *bstream, BitStream *arg);
static int BitStream_appendNum(BitStream *bstream, size_t bits, unsigned int num);
static int BitStream_appendBytes(BitStream *bstream, size_t size, unsigned char *data);
#define BitStream_size(__bstream__) (__bstream__->length)
#define BitStream_reset(__bstream__) (__bstream__->length = 0)
static unsigned char *BitStream_toByte(BitStream *bstream);
static void BitStream_free(BitStream *bstream);

#endif /* BITSTREAM_H */
