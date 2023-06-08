/*	--*- c -*--
 * Copyright (C) 2014 Enrico Scholz <enrico.scholz@ensc.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define main	main_
#include "../src/main.c"
#undef main

#define TEST(_crc, ...)			\
	do {							\
	    unsigned char	hdr[] = { __VA_ARGS__ };	\
	    uint16_t		crc = ~calculateCheckSum(hdr, sizeof hdr, 0); \
	    assert(crc == (_crc));					\
	} while (0)

int main()
{
	TEST(0xb861,
	     0x45, 0x00, 0x00, 0x73,  0x00, 0x00, 0x40, 0x00,
	     0x40, 0x11,    0,    0,  0xc0, 0xa8, 0x00, 0x01,
	     0xc0, 0xa8, 0x00, 0xc7);
	TEST(0xb761,
	     0x45, 0x00, 0x00, 0x73,  0x00, 0x00, 0x40, 0x00,
	     0x40, 0x11,    0,    0,  0xc0, 0xa8, 0x00, 0x01,
	     0xc0, 0xa8, 0x00, 0xc7,  0x01);

	return 0;
}
