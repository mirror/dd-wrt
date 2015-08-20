/*
 * This file is part of nmealib.
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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nmea/generator.h>

#include <nmea/context.h>
#include <nmea/gmath.h>
#include <nmea/generate.h>

#include "random.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * Initialise the generator
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * (present and smask are preserved, other fields are reset before generation starts)
 * @return 1 (true) on success, 0 (false) otherwise
 */
int nmea_gen_init(nmeaGENERATOR *gen, nmeaINFO *info) {
	int retval = 1;
	int present = info->present;
	int smask = info->smask;
	nmeaGENERATOR *igen = gen;

	nmea_init_random();

	nmea_zero_INFO(info);
	info->present = present;
	info->smask = smask;
	nmea_INFO_set_present(&info->present, SMASK);

	info->lat = NMEA_DEF_LAT;
	info->lon = NMEA_DEF_LON;
	nmea_INFO_set_present(&info->present, LAT);
	nmea_INFO_set_present(&info->present, LON);

	while (retval && igen) {
		if (igen->init_call)
			retval = (*igen->init_call)(igen, info);
		igen = igen->next;
	}

	return retval;
}

/**
 * Loop the generator.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
int nmea_gen_loop(nmeaGENERATOR *gen, nmeaINFO *info) {
	int retVal = 1;

	if (gen->loop_call)
		retVal = (*gen->loop_call)(gen, info);

	if (retVal && gen->next)
		retVal = nmea_gen_loop(gen->next, info);

	return retVal;
}

/**
 * Reset the generator.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
int nmea_gen_reset(nmeaGENERATOR *gen, nmeaINFO *info) {
	int RetVal = 1;

	if (gen->reset_call)
		RetVal = (*gen->reset_call)(gen, info);

	return RetVal;
}

/**
 * Destroy the generator.
 *
 * @param gen a pointer to the generator
 */
void nmea_gen_destroy(nmeaGENERATOR *gen) {
	if (gen->next) {
		nmea_gen_destroy(gen->next);
		gen->next = 0;
	}

	if (gen->destroy_call)
		(*gen->destroy_call)(gen);

	free(gen);
}

/**
 * Add a generator to the existing ones.
 *
 * @param to the generators to add to
 * @param gen the generator to add
 */
void nmea_gen_add(nmeaGENERATOR *to, nmeaGENERATOR *gen) {
	nmeaGENERATOR * next = to;
	while (next->next)
		next = to->next;

	next->next = gen;
}

/**
 * Run a new generation loop on the generator
 *
 * @param s a pointer to the string buffer in which to generate
 * @param len the size of the buffer
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @param gen a pointer to the generator
 * @param generate_mask the smask of sentences to generate
 * @return the total length of the generated sentences
 */
int nmea_generate_from(char *s, int len, nmeaINFO *info, nmeaGENERATOR *gen, int generate_mask) {
	int retval;

	if ((retval = nmea_gen_loop(gen, info)))
		retval = nmea_generate(s, len, info, generate_mask);

	return retval;
}

/*
 * NOISE generator
 */

/**
 * NOISE Generator loop function.
 * Does not touch smask and utc in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_noise_loop(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	int it;
	int in_use;

	info->sig = lrint(nmea_random(1, 3));
	info->fix = lrint(nmea_random(2, 3));
	info->PDOP = nmea_random(0, 9);
	info->HDOP = nmea_random(0, 9);
	info->VDOP = nmea_random(0, 9);
	info->lat = nmea_random(0, 100);
	info->lon = nmea_random(0, 100);
	info->elv = lrint(nmea_random(-100, 100));
	info->speed = nmea_random(0, 100);
	info->track = nmea_random(0, 360);
	info->mtrack = nmea_random(0, 360);
	info->magvar = nmea_random(0, 360);

	nmea_INFO_set_present(&info->present, SIG);
	nmea_INFO_set_present(&info->present, FIX);
	nmea_INFO_set_present(&info->present, PDOP);
	nmea_INFO_set_present(&info->present, HDOP);
	nmea_INFO_set_present(&info->present, VDOP);
	nmea_INFO_set_present(&info->present, LAT);
	nmea_INFO_set_present(&info->present, LON);
	nmea_INFO_set_present(&info->present, ELV);
	nmea_INFO_set_present(&info->present, SPEED);
	nmea_INFO_set_present(&info->present, TRACK);
	nmea_INFO_set_present(&info->present, MTRACK);
	nmea_INFO_set_present(&info->present, MAGVAR);

	info->satinfo.inuse = 0;
	info->satinfo.inview = 0;

	for (it = 0; it < NMEA_MAXSAT; it++) {
		in_use = lrint(nmea_random(0, 3));
		info->satinfo.in_use[it] = in_use ? it : 0;
		info->satinfo.sat[it].id = it;
		info->satinfo.sat[it].elv = lrint(nmea_random(0, 90));
		info->satinfo.sat[it].azimuth = lrint(nmea_random(0, 359));
		info->satinfo.sat[it].sig = (int) (in_use ? nmea_random(40, 99) : nmea_random(0, 40));

		if (in_use)
			info->satinfo.inuse++;
		if (info->satinfo.sat[it].sig > 0)
			info->satinfo.inview++;
	}

	nmea_INFO_set_present(&info->present, SATINUSECOUNT);
	nmea_INFO_set_present(&info->present, SATINUSE);
	nmea_INFO_set_present(&info->present, SATINVIEW);

	return 1;
}

/*
 * STATIC generator
 */

/**
 * STATIC Generator loop function.
 * Only touches utc in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_static_loop(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	nmea_time_now(&info->utc, &info->present);
	return 1;
}

/**
 * STATIC Generator reset function.
 * Resets only the satinfo to 4 sats in use and in view.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_static_reset(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	info->satinfo.inuse = 4;
	info->satinfo.inview = 4;

	info->satinfo.in_use[0] = 1;
	info->satinfo.sat[0].id = 1;
	info->satinfo.sat[0].elv = 50;
	info->satinfo.sat[0].azimuth = 0;
	info->satinfo.sat[0].sig = 99;

	info->satinfo.in_use[1] = 2;
	info->satinfo.sat[1].id = 2;
	info->satinfo.sat[1].elv = 50;
	info->satinfo.sat[1].azimuth = 90;
	info->satinfo.sat[1].sig = 99;

	info->satinfo.in_use[2] = 3;
	info->satinfo.sat[2].id = 3;
	info->satinfo.sat[2].elv = 50;
	info->satinfo.sat[2].azimuth = 180;
	info->satinfo.sat[2].sig = 99;

	info->satinfo.in_use[3] = 4;
	info->satinfo.sat[3].id = 4;
	info->satinfo.sat[3].elv = 50;
	info->satinfo.sat[3].azimuth = 270;
	info->satinfo.sat[3].sig = 99;

	nmea_INFO_set_present(&info->present, SATINUSECOUNT);
	nmea_INFO_set_present(&info->present, SATINUSE);
	nmea_INFO_set_present(&info->present, SATINVIEW);

	return 1;
}

/**
 * STATIC Generator initialiser function.
 * Only touches sig, fix and satinfo in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_static_init(nmeaGENERATOR *gen, nmeaINFO *info) {
	info->sig = 3;
	info->fix = 3;

	nmea_INFO_set_present(&info->present, SIG);
	nmea_INFO_set_present(&info->present, FIX);

	nmea_igen_static_reset(gen, info);

	return 1;
}

/*
 * SAT_ROTATE generator
 */

/**
 * SAT_ROTATE Generator loop function.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_rotate_loop(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	int it;
	int count = info->satinfo.inview;
	double deg = 360.0 / (count ? count : 1);
	double srt = (count ? (info->satinfo.sat[0].azimuth) : 0) + 5;

	nmea_time_now(&info->utc, &info->present);

	for (it = 0; it < count; it++) {
		info->satinfo.sat[it].azimuth = (int) ((srt >= 360) ? srt - 360 : srt);
		srt += deg;
	}

	nmea_INFO_set_present(&info->present, SATINVIEW);

	return 1;
}

/**
 * SAT_ROTATE Generator reset function.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_rotate_reset(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	int it;
	double deg = 360 / 8;
	double srt = 0;

	info->satinfo.inuse = 8;
	info->satinfo.inview = 8;

	for (it = 0; it < info->satinfo.inview; it++) {
		info->satinfo.in_use[it] = it + 1;
		info->satinfo.sat[it].id = it + 1;
		info->satinfo.sat[it].elv = 5;
		info->satinfo.sat[it].azimuth = (int) srt;
		info->satinfo.sat[it].sig = 80;
		srt += deg;
	}

	nmea_INFO_set_present(&info->present, SATINUSECOUNT);
	nmea_INFO_set_present(&info->present, SATINUSE);
	nmea_INFO_set_present(&info->present, SATINVIEW);

	return 1;
}

/**
 * SAT_ROTATE Generator initialiser function.
 * Only touches sig, fix and satinfo in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_rotate_init(nmeaGENERATOR *gen, nmeaINFO *info) {
	info->sig = 3;
	info->fix = 3;

	nmea_INFO_set_present(&info->present, SIG);
	nmea_INFO_set_present(&info->present, FIX);

	nmea_igen_rotate_reset(gen, info);

	return 1;
}

/*
 * POS_RANDMOVE generator
 */

/**
 * POS_RANDMOVE Generator initialiser function.
 * Only touches sig, fix, track, mtrack, magvar and speed in info.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_pos_rmove_init(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	info->sig = 3;
	info->fix = 3;
	info->speed = 20;
	info->track = 0;
	info->mtrack = 0;
	info->magvar = 0;

	nmea_INFO_set_present(&info->present, SIG);
	nmea_INFO_set_present(&info->present, FIX);
	nmea_INFO_set_present(&info->present, SPEED);
	nmea_INFO_set_present(&info->present, TRACK);
	nmea_INFO_set_present(&info->present, MTRACK);
	nmea_INFO_set_present(&info->present, MAGVAR);

	return 1;
}

/**
 * POS_RANDMOVE Generator loop function.
 *
 * @param gen a pointer to the generator
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return 1 (true) on success, 0 (false) otherwise
 */
static int nmea_igen_pos_rmove_loop(nmeaGENERATOR *gen __attribute__ ((unused)), nmeaINFO *info) {
	nmeaPOS crd;

	info->track += nmea_random(-10, 10);
	info->mtrack += nmea_random(-10, 10);
	info->speed += nmea_random(-2, 3);

	if (info->track < 0) {
		info->track = 359 + info->track;
	}
	if (info->track > 359) {
		info->track -= 359;
	}
	if (info->mtrack < 0) {
		info->mtrack = 359 + info->mtrack;
	}
	if (info->mtrack > 359) {
		info->mtrack -= 359;
	}

	if (info->speed > 40)
		info->speed = 40;
	if (info->speed < 1)
		info->speed = 1;

	nmea_info2pos(info, &crd);
	nmea_move_horz(&crd, &crd, info->track, info->speed / 3600);
	nmea_pos2info(&crd, info);

	info->magvar = info->track;

	nmea_INFO_set_present(&info->present, LAT);
	nmea_INFO_set_present(&info->present, LON);
	nmea_INFO_set_present(&info->present, SPEED);
	nmea_INFO_set_present(&info->present, TRACK);
	nmea_INFO_set_present(&info->present, MTRACK);
	nmea_INFO_set_present(&info->present, MAGVAR);

	return 1;
}

/**
 * Create the generator.
 *
 * @param type the type of the generator to create (see nmeaGENTYPE)
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return the generator
 */
static nmeaGENERATOR * __nmea_create_generator(const int type, nmeaINFO *info) {
	nmeaGENERATOR *gen = 0;

	switch (type) {
	case NMEA_GEN_NOISE:
		if (!(gen = malloc(sizeof(nmeaGENERATOR))))
			nmea_error("__nmea_create_generator: insufficient memory!");
		else {
			memset(gen, 0, sizeof(nmeaGENERATOR));
			gen->loop_call = &nmea_igen_noise_loop;
		}
		break;
	case NMEA_GEN_STATIC:
	case NMEA_GEN_SAT_STATIC:
		if (!(gen = malloc(sizeof(nmeaGENERATOR))))
			nmea_error("__nmea_create_generator: insufficient memory!");
		else {
			memset(gen, 0, sizeof(nmeaGENERATOR));
			gen->init_call = &nmea_igen_static_init;
			gen->loop_call = &nmea_igen_static_loop;
			gen->reset_call = &nmea_igen_static_reset;
		}
		break;
	case NMEA_GEN_SAT_ROTATE:
		if (!(gen = malloc(sizeof(nmeaGENERATOR))))
			nmea_error("__nmea_create_generator: insufficient memory!");
		else {
			memset(gen, 0, sizeof(nmeaGENERATOR));
			gen->init_call = &nmea_igen_rotate_init;
			gen->loop_call = &nmea_igen_rotate_loop;
			gen->reset_call = &nmea_igen_rotate_reset;
		}
		break;
	case NMEA_GEN_POS_RANDMOVE:
		if (!(gen = malloc(sizeof(nmeaGENERATOR))))
			nmea_error("__nmea_create_generator: insufficient memory!");
		else {
			memset(gen, 0, sizeof(nmeaGENERATOR));
			gen->init_call = &nmea_igen_pos_rmove_init;
			gen->loop_call = &nmea_igen_pos_rmove_loop;
		}
		break;
	default:
		/* case NMEA_GEN_ROTATE: */
		gen = __nmea_create_generator(NMEA_GEN_SAT_ROTATE, info);
		nmea_gen_add(gen, __nmea_create_generator(NMEA_GEN_POS_RANDMOVE, info));
		break;
	};

	return gen;
}

/**
 * Create the generator and initialise it.
 *
 * @param type the type of the generator to create (see nmeaGENTYPE)
 * @param info a pointer to an nmeaINFO structure to use during generation
 * @return the generator
 */
nmeaGENERATOR * nmea_create_generator(const int type, nmeaINFO *info) {
	nmeaGENERATOR *gen = __nmea_create_generator(type, info);

	if (gen)
		nmea_gen_init(gen, info);

	return gen;
}
