/*
 *	BIRD Library -- IP One-Complement Checksum
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_CHECKSUM_H_
#define _BIRD_CHECKSUM_H_

/*
 *	Both checksumming functions accept a vararg list of packet
 *	fragments finished by NULL pointer.
 */

int ipsum_verify(void *frag, unsigned len, ...);
u16 ipsum_calculate(void *frag, unsigned len, ...);

#endif
