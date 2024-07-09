/*
 * Copyright (c) 2010-2013 BitTorrent, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "utp_hash.h"
#include "utp_types.h"

#ifdef STRICT_ALIGN
inline uint32 Read32(const void *p)
{
	uint32 tmp;
	memcpy(&tmp, p, sizeof tmp);
	return tmp;
}
#else
inline uint32 Read32(const void *p) { return *(uint32*)p; }
#endif

uint utp_hash_mem(const void *keyp, size_t keysize)
{
	uint hash = 0;
	uint n = keysize;
	while (n >= 4) {
		hash ^= Read32(keyp);
		keyp = (byte*)keyp + sizeof(uint32);
		hash = (hash << 13) | (hash >> 19);
		n -= 4;
	}
	while (n != 0) {
		hash ^= *(byte*)keyp;
		keyp = (byte*)keyp + sizeof(byte);
		hash = (hash << 8) | (hash >> 24);
		n--;
	}
	return hash;
}
