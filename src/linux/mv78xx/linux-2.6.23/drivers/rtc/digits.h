/*
 * Zapata Telephony Telephony
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 * Use DTMF/MFv1 tables 
 */
#ifndef _DIGITS_H
#define _DIGITS_H

#define DEFAULT_DTMF_LENGTH	100 * 8
#define DEFAULT_MFV1_LENGTH	60 * 8
#define	PAUSE_LENGTH		500 * 8

/* At the end of silence, the tone stops */
static struct zt_tone dtmf_silence =
	{ 0, 0, 0, 0, 0, 0, DEFAULT_DTMF_LENGTH, NULL };

/* At the end of silence, the tone stops */
static struct zt_tone mfv1_silence =
	{ 0, 0, 0, 0, 0, 0, DEFAULT_MFV1_LENGTH, NULL };

/* A pause in the dialing */
static struct zt_tone tone_pause =
	{ 0, 0, 0, 0, 0, 0, PAUSE_LENGTH, NULL };

#include "tones.h"

#endif 
