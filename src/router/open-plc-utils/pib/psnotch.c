/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================r
 *
 *   psnotch.c - Atheros Prescaler Notching Utility
 *
 *   Atheros Powerline Toolkit;
 *
 *   this program is the Atheros INT6000 Dynamic Notching Utility
 *
 *   this program inspects the following frequency bands for SW signals:
 *
 *       120 m 2,300 - 2,495 kHz tropic band
 *        90 m 3,200 - 3,400 kHz tropic band
 *        75 m 3,900 - 4,000 kHz shared with the amateur radio 75/80 meter band
 *        60 m 4,750 - 5,060 kHz tropic band
 *        49 m 5,900 - 6,200 kHz
 *        40 m/41m 7,100 - 7,350 kHz shared with the amateur radio 40 meter band
 *        31 m 9,400 - 9,900 kHz Currently most heavily used band
 *        25 m 11,600 - 12,100 kHz
 *        22 m 13,570 - 13,870 kHz substantially used only in Eurasia
 *        19 m 15,100 - 15,800 kHz
 *        16 m 17,480 - 17,900 kHz
 *        15 m 18,900 - 19,020 kHz almost unused, could become a DRM band
 *        13 m 21,450 - 21,850 kHz
 *        11 m 25,600 - 26,100 kHz may be used for local DRM broadcasting
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/error.h"
#include "../tools/flags.h"
#include "../tools/chars.h"
#include "../tools/number.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define _PRINTF_DEBUG

#define PSNOTCH_VERBOSE (1 << 0)
#define PSNOTCH_SILENCE (1 << 1)
#define PSNOTCH_COMMA   (1 << 2)

#define CARRIERS 1155
#define TONES 917
#define SLOTS 6

#define INDEX_TO_FREQ(index) ((float)(index + 74)/40.96)
#define FREQ_TO_INDEX(freq)  ((unsigned)(40.96 * freq)-74)

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

typedef struct carrier

{
	uint16_t amplitude;
	uint8_t slots [SLOTS];
}

carrier;
typedef struct map

{
	unsigned slots;
	struct carrier carriers [CARRIERS];
}

map;
uint8_t hambands [CARRIERS] =

{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};


/*====================================================================*
 *
 *   int read_scalers (struct map * map);
 *
 *   read a prescaler file from stdin; permit comments and blank
 *   input lines; the set of prescalers are technically known as
 *   an amplitude map;
 *
 *   an amplitude map consists an offset (frequency) and a scaler
 *   (amplitude); we read, check and discard the offset but store
 *   the scaler;
 *
 *--------------------------------------------------------------------*/

void read_scalers (struct map * map)

{
	unsigned carriers = 0;
	unsigned tones = 0;
	uint32_t carrier;
	uint32_t amplitude;
	signed c;
	while ((c = getc (stdin)) != EOF)
	{
		if (isspace (c))
		{
			continue;
		}
		if ((c == '#') || (c == ';'))
		{
			do
			{
				c = getc (stdin);
			}
			while (nobreak (c));
			continue;
		}
		carrier = 0;
		while (isdigit (c))
		{
			carrier *= 10;
			carrier += c - '0';
			c = getc (stdin);
		}
		if (carrier != carriers)
		{
			error (1, 0, "Prescaler %d/%d out of order", carrier, carriers);
		}
		if (carrier >= CARRIERS)
		{
			break;
		}
		while (isblank (c))
		{
			c = getc (stdin);
		}
		amplitude = 0;
		while (isxdigit (c))
		{
			amplitude *= 16;
			amplitude += todigit (c);
			c = getc (stdin);
		}
		map->carriers [carrier].amplitude = amplitude;
		if (amplitude)
		{
			tones++;
		}
		while ((c != EOF) && (c != '\n'))
		{
			c = getc (stdin);
		}
		carriers++;
	}
	if (carriers != CARRIERS)
	{
		error (1, 0, "Have %d amplitude map carriers but need %d", carriers, CARRIERS);
	}
	if (tones != TONES)
	{
		error (1, 0, "Expected %d amplitude map scalers but read %d", TONES, tones);
	}
	return;
}


/*====================================================================*
 *
 *   void write_scalers (struct map * map);
 *
 *   print amplitude map on stdout in a format suitabl for input to
 *   program psin or the Windows Device Manager;
 *
 *
 *--------------------------------------------------------------------*/

void write_scalers (struct map * map)

{
	unsigned carrier = 0;
	for (carrier = 0; carrier < CARRIERS; carrier++)
	{
		printf ("%.8u %.8hX\n", carrier, map->carriers [carrier].amplitude);
	}
	return;
}


/*====================================================================*
 *
 *   void read_tonemaps (struct map * map, FILE *fp);
 *
 *   read tone map from a file; a tonemap file can be created using
 *   program int6ktone;
 *
 *   input consists of 1159 lines; the first line is a comment; the
 *   next two lines contain GIL and AGC information; the remaining
 *   1155 lines consist of an offset follwed by 5 or 6 slot values;
 *
 *   although 1155 values are read, the first 917 contain tonemap
 *   data and rest contain 0 values; the 917 values correspond to
 *   the 917 amplitude scalers;
 *
 *   the tone map may be read either before or after the amplitude
 *   map is read;
 *
 *
 *--------------------------------------------------------------------*/

void read_tonemaps (struct map * map, FILE *fp)

{
	uint8_t slots [SLOTS];
	unsigned tones = 0;
	unsigned tone = 0;
	unsigned slot = 0;
	signed c;
	map->slots = SLOTS;
	memset (slots, 0, sizeof (slots));
	while ((c = getc (fp)) != EOF)
	{
		if (c == '#')
		{
			while (((c = getc (fp)) != EOF) && (c != '\n'));
		}
		if (isspace (c))
		{
			continue;
		}
		tone = 0;
		while (isdigit (c))
		{
			tone *= 10;
			tone += c - '0';
			c = getc (fp);
		}
		if (tone != tones)
		{
			error (1, ECANCELED, "Tonemap %d/%d is out of order", tone, tones);
		}
		while (isblank (c))
		{
			c = getc (fp);
		}
		for (slot = 0; slot < SLOTS; slot++)
		{
			unsigned value = 0;
			while (isdigit (c))
			{
				value *= 10;
				value += c - '0';
				c = getc (fp);
			}
			map->carriers [tone].slots [slot] = value;
			while (isblank (c))
			{
				c = getc (fp);
			}
		}
		while ((c != EOF) && (c != '\n'))
		{
			c = getc (fp);
		}
		tones++;
	}
	if (tones != TONES)
	{
		error (0, 0, "Have %d tone map carriers but need %d", tones, TONES);
	}
	return;
}


/*====================================================================*
 *
 *   void align_tones (struct map * map);
 *
 *   the tonemap consists of 917 consecutive carriers the amplitude
 *   map consists of 1155 carriers having 917 non-zero values; this
 *   function distributes the tonemap entries so that they align to
 *   corresponding amplitude map entries;
 *
 *   alignment cannot be performed until both the amplitude map and
 *   tone map have been read;
 *
 *
 *--------------------------------------------------------------------*/

void align_tones (struct map * map)

{
	unsigned carriers = CARRIERS;
	unsigned tones = TONES;
	while (carriers--)
	{
		if (map->carriers [carriers].amplitude)
		{
			if (tones)
			{
				tones--;
				memcpy (&map->carriers [carriers].slots, &map->carriers [tones].slots, SLOTS);
				memset (&map->carriers [tones].slots, 0, SLOTS);
			}
		}
	}
	return;
}


/*====================================================================*
 *
 *   unsigned notch_tones (struct map * scalers, unsigned lower, unsigned upper);
 *
 *   scan a range of tones for signals; signals are indicated by low
 *   mean-square values computed across all slots for a given tone;
 *
 *   there may not be much change from one carrier to the next; we
 *   accentuate changes by squaring then summing slot values; this
 *   produces a reasonably clean parabolic dip in the map where the
 *   signal occurs;
 *
 *   function watch_tones can be used to observe tone map values and
 *   signal dips over a given range of map values; generally, it is
 *   best to display a wider range of tones than those being notched;
 *
 *
 *--------------------------------------------------------------------*/

unsigned notch_tones (struct map * map, unsigned lower, unsigned upper, unsigned limit)

{
	unsigned slot;
	unsigned notch = 0;
	while (lower < upper)
	{
		unsigned total = 0;
		for (slot = 0; slot < map->slots; slot++)
		{
			unsigned value = 0;
			value = map->carriers [lower].slots [slot];
			value *= value;
			total += value;
		}
		if (slot)
		{
			total /= slot;
		}
		if (total < limit)
		{
			map->carriers [lower].amplitude = 0;
			notch = 2;
		}
		lower++;
	}
	return (notch);
}


/*====================================================================*
 *
 *   void watch_tones (struct map * map, unsigned lower, unsigned upper);
 *
 *   print amplitude and tone values over a given range along with a
 *   plot of the values used to detect signal dips; for best effect,
 *   the range used here should exceed the notching range to provide
 *   context information;
 *
 *
 *--------------------------------------------------------------------*/

void watch_tones (struct map * map, unsigned lower, unsigned upper)

{
	unsigned slot;
	while (lower < upper)
	{
		unsigned total = 0;
		fprintf (stderr, "%04d %04X", lower, map->carriers [lower].amplitude);
		for (slot = 0; slot < map->slots; slot++)
		{
			unsigned value = 0;
			value = map->carriers [lower].slots [slot];
			fprintf (stderr, " %02X", value);
			value *= value;
			total += value;
		}
		if (slot)
		{
			total /= slot;
		}
		fprintf (stderr, " %6.3f ", INDEX_TO_FREQ (lower));
		while (total--)
		{
			fprintf (stderr, "#");
		}
		fprintf (stderr, "\n");
		lower++;
	}
	return;
}


/*====================================================================*
 *
 *   void watch_tone2 (struct map * map, unsigned lower, unsigned upper);
 *
 *   print amplitude and tone values over a given range along with a
 *   plot of the values used to detect signal dips; for best effect,
 *   the range used here should exceed the notching range to provide
 *   context information;
 *
 *
 *--------------------------------------------------------------------*/

void watch_tone2 (struct map * map, unsigned lower, unsigned upper)

{
	unsigned slot;
	while (lower < upper)
	{
		unsigned total = 0;
		fprintf (stderr, "%d,%d", lower, map->carriers [lower].amplitude);
		for (slot = 0; slot < map->slots; slot++)
		{
			unsigned value = 0;
			value = map->carriers [lower].slots [slot];
			fprintf (stderr, ",%d", value);
			value *= value;
			total += value;
		}
		if (slot)
		{
			total /= slot;
		}
		fprintf (stderr, ",%d", total);
		fprintf (stderr, ",%6.3f", INDEX_TO_FREQ (lower));
		fprintf (stderr, "\n");
		lower++;
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *   read an amplitude map from stdin and print a notched amplitude
 *   map on stdout; use a tone map file to determine if a signal is
 *   present where notching chould occur; notching ranges are input
 *   as map offsets, not carrier frequencies;
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	char const * optv [] =
	{
		"cf:l:L:t:u:U:qv",
		PUTOPTV_S_FILTER,
		"Atheros Prescaler Notching Utility",
		"c\tcomma delimited output",
		"f f\tread tonemap file (f)",
		"t n\tthreshold is (n) units",
		"l n\tlower notch range is (n)",
		"L n\tlower graph range is (n)",
		"u n\tupper notch range is (n)",
		"U n\tupper graph range is (n)",
		"v\tverbose output",
		(char const *)(0)
	};
	struct map map;
	FILE * fp = (FILE *)(0);
	unsigned lower = CARRIERS-1;
	unsigned upper = 0;
	unsigned LOWER = 0;
	unsigned UPPER = CARRIERS-1;
	signed limit = 4;
	signed status = 0;
	flag_t flags = (flag_t)(0);
	signed c;
	memset (&map, 0, sizeof (map));
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'c':
			_setbits (flags, PSNOTCH_COMMA);
			break;
		case 'f':
			if ((fp = fopen (optarg, "rb")) == (FILE *)(0))
			{
				error (1, errno, "Can't open %s", optarg);
			}
			read_tonemaps (&map, fp);
			fclose (fp);
			break;
		case 'l':
			lower = (unsigned)(uintspec (optarg, 0, CARRIERS-1));
			break;
		case 'u':
			upper = (unsigned)(uintspec (optarg, 0, CARRIERS-1));
			break;
		case 'L':
			LOWER = (unsigned)(uintspec (optarg, 0, CARRIERS-1));
			break;
		case 'U':
			UPPER = (unsigned)(uintspec (optarg, 0, CARRIERS-1));
			break;
		case 't':
			limit = (signed)(uintspec (optarg, 0, 49));
			break;
		case 'q':
			_setbits (flags, PSNOTCH_SILENCE);
			break;
		case 'v':
			_setbits (flags, PSNOTCH_VERBOSE);
			break;
		default:
			break;
		}
	}
	argv += optind;
	argc -= optind;
	read_scalers (&map);
	align_tones (&map);
	if (_anyset (flags, PSNOTCH_COMMA))
	{
		watch_tone2 (&map, LOWER, UPPER);
	}
	else if (_anyset (flags, PSNOTCH_VERBOSE))
	{
		watch_tones (&map, LOWER, UPPER);
	}
	if (fp)
	{
		status = notch_tones (&map, lower, upper, limit);
	}
	write_scalers (&map);
	return (status);
}

