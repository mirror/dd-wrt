/* Copyright (c) 2013
 *      Mike Gerwitz (mtg@gnu.org)
 *
 * This file is part of GNU screen.
 *
 * GNU screen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * <https://www.gnu.org/licenses>.
 *
 ****************************************************************
 */

#include "config.h"

#include "winmsgcond.h"

#include <assert.h>
#include <stdlib.h>


/* Initialize new condition and set to false; can be used to re-initialize a
 * condition for re-use */
void wmc_init(WinMsgCond *cond, int offset)
{
	cond->locked = false;
	cond->offset = offset;
	cond->initialized = true;
	wmc_clear(cond);
}

/* Mark condition as true */
void wmc_set(WinMsgCond *cond)
{
	if (cond->locked)
		return;

	cond->state = true;
}

/* Clear condition (equivalent to non-match) */
void wmc_clear(WinMsgCond *cond)
{
	if (cond->locked)
		return;

	cond->state = false;
}

/* Determine if condition is active (has been initialized and can be used) */
bool wmc_is_active(const WinMsgCond *cond)
{
	return cond->initialized;
}

/* Determine if a condition is true; the result is undefined if
 * !wmc_active(cond) */
bool wmc_is_set(const WinMsgCond *cond)
{
	return cond->state;
}

/* "else" encounted */
int wmc_else(WinMsgCond *cond, int offset, bool *changed)
{
	assert(wmc_is_active(cond));

	/* if we're already set, then there is no point in processing the "else";
	 * we will therefore consider the previous condition to have succeeded, so
	 * now keep track of the start of the else so that it may be clobbered
	 * instead of the beginning of the true condition---that is, we're accepting
	 * the destination string up until this point */
	if (wmc_is_set(cond)) {
		wmc_init(cond, offset);  /* track this as a new condition */
		cond->locked = true;  /* "else" shall never succeed at this point */

		/* we want to keep the string we have so far (the truth string) */
		if (changed)
			*changed = false;
		return offset;
	}

	/* now that we have reached "else" and are not true, we can never be true;
	 * discard the truth part of the string */
	int prevoffset = cond->offset;
	cond->offset = offset;

	/* the "else" part must always be true at this point, because the previous
	 * condition failed */
	wmc_set(cond);
	cond->locked = true;

	if (changed)
		*changed = true;
	return prevoffset;
}

/* End condition and determine if string should be reset or kept---if our value
 * is truthful, then accept the string, otherwise reject and reset to the
 * position that we were initialized with */
int wmc_end(const WinMsgCond *cond, int offset, bool *changed)
{
	bool set = wmc_is_set(cond);
	if (changed)
		*changed = !set;

	return (set) ? offset : cond->offset;
}

/* Deactivate a condition, preventing its use; this allows a single allocation
 * to be re-used and ignored until activated */
void wmc_deinit(WinMsgCond *cond)
{
	cond->state = false;
	cond->offset = 0;
	cond->locked = true;
	cond->initialized = false;
}
