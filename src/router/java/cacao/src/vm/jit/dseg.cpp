/* src/vm/jit/dseg.cpp - data segment handling stuff

   Copyright (C) 1996-2014
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

#include "vm/jit/dseg.hpp"
#include <assert.h>                     // for assert
#include <stdio.h>                      // for printf
#include <inttypes.h>                   // for printf formatting macros
#include "config.h"                     // for SIZEOF_VOID_P
#include "mm/dumpmemory.hpp"            // for DNEW
#include "vm/jit/code.hpp"              // for codeinfo
#include "vm/jit/codegen-common.hpp"    // for codegendata, dataref, etc
#include "vm/jit/jit.hpp"               // for jitdata, etc
#include "vm/jit/methodheader.hpp"
#include "vm/options.hpp"               // for opt_debugcolor
#include "vm/types.hpp"                 // for s4, s8, u4, ptrint, u1



/* dseg_finish *****************************************************************

   Fills the data segment with the values stored.

*******************************************************************************/

void dseg_finish(jitdata *jd)
{
	codeinfo    *code;
	codegendata *cd;
	dsegentry   *de;

	/* get required compiler data */

	code = jd->code;
	cd   = jd->cd;

	/* process all data segment entries */

	for (de = cd->dseg; de != NULL; de = de->next) {
		switch (de->type) {
		case TYPE_INT:
			*((s4 *)     (code->entrypoint + de->disp)) = de->val.i;
			break;

		case TYPE_LNG:
			*((s8 *)     (code->entrypoint + de->disp)) = de->val.l;
			break;

		case TYPE_FLT:
			*((float *)  (code->entrypoint + de->disp)) = de->val.f;
			break;

		case TYPE_DBL:
			*((double *) (code->entrypoint + de->disp)) = de->val.d;
			break;

		case TYPE_ADR:
			*((void **)  (code->entrypoint + de->disp)) = de->val.a;
			break;
		}
	}
}


static s4 dseg_find_s4(codegendata *cd, s4 value)
{
	dsegentry *de;

	/* search all data segment entries for a matching entry */

	for (de = cd->dseg; de != NULL; de = de->next) {
		if (IS_INT_TYPE(de->type))
			if (de->flags & DSEG_FLAG_READONLY)
				if (de->val.i == value)
					return de->disp;
	}

	/* no matching entry was found */

	return 0;
}


static s4 dseg_find_s8(codegendata *cd, s8 value)
{
	dsegentry *de;

	/* search all data segment entries for a matching entry */

	for (de = cd->dseg; de != NULL; de = de->next) {
		if (IS_LNG_TYPE(de->type))
			if (de->flags & DSEG_FLAG_READONLY)
				if (de->val.l == value)
					return de->disp;
	}

	/* no matching entry was found */

	return 0;
}


static s4 dseg_find_float(codegendata *cd, float value)
{
	dsegentry *de;
	imm_union  val;

	/* we compare the hex value of the float as 0.0 == -0.0 */

	val.f = value;

	/* search all data segment entries for a matching entry */

	for (de = cd->dseg; de != NULL; de = de->next) {
		if (IS_FLT_TYPE(de->type))
			if (de->flags & DSEG_FLAG_READONLY)
				if (de->val.i == val.i)
					return de->disp;
	}

	/* no matching entry was found */

	return 0;
}


static s4 dseg_find_double(codegendata *cd, double value)
{
	dsegentry *de;
	imm_union  val;

	/* we compare the hex value of the double as 0.0 == -0.0 */

	val.d = value;

	/* search all data segment entries for a matching entry */

	for (de = cd->dseg; de != NULL; de = de->next) {
		if (IS_DBL_TYPE(de->type))
			if (de->flags & DSEG_FLAG_READONLY)
				if (de->val.l == val.l)
					return de->disp;
	}

	/* no matching entry was found */

	return 0;
}


static s4 dseg_find_address(codegendata *cd, void *value)
{
	dsegentry *de;

	/* search all data segment entries for a matching entry */

	for (de = cd->dseg; de != NULL; de = de->next) {
		if (IS_ADR_TYPE(de->type))
			if (de->flags & DSEG_FLAG_READONLY)
				if (de->val.a == value)
					return de->disp;
	}

	/* no matching entry was found */

	return 0;
}


/* dseg_add_s4_intern **********************************************************

   Internal function to add an s4 value to the data segment.

*******************************************************************************/

static s4 dseg_add_s4_intern(codegendata *cd, s4 value, u4 flags)
{
	dsegentry *de;

	/* Increase data segment size, which is also the displacement into
	   the data segment. */

	cd->dseglen += 4;

	/* allocate new entry */

	de = DNEW(dsegentry);

	de->type  = TYPE_INT;
	de->flags = flags;
	de->disp  = -(cd->dseglen);
	de->val.i = value;
	de->next  = cd->dseg;

	/* insert into the chain */

	cd->dseg = de;

	return de->disp;
}


/* dseg_add_unique_s4 **********************************************************

   Adds uniquely an s4 value to the data segment.

*******************************************************************************/

s4 dseg_add_unique_s4(codegendata *cd, s4 value)
{
	s4 disp;

	disp = dseg_add_s4_intern(cd, value, DSEG_FLAG_UNIQUE);

	return disp;
}


/* dseg_add_s4 *****************************************************************

   Adds an s4 value to the data segment. It tries to reuse previously
   added values.

*******************************************************************************/

s4 dseg_add_s4(codegendata *cd, s4 value)
{
	s4 disp;

	/* search the data segment if the value is already stored */

	disp = dseg_find_s4(cd, value);

	if (disp != 0)
		return disp;

	disp = dseg_add_s4_intern(cd, value, DSEG_FLAG_READONLY);

	return disp;
}


/* dseg_add_s8_intern **********************************************************

   Internal function to add an s8 value to the data segment.

*******************************************************************************/

static s4 dseg_add_s8_intern(codegendata *cd, s8 value, u4 flags)
{
	dsegentry *de;

	/* Increase data segment size, which is also the displacement into
	   the data segment. */

	cd->dseglen = MEMORY_ALIGN(cd->dseglen + 8, 8);

	/* allocate new entry */

	de = DNEW(dsegentry);

	de->type  = TYPE_LNG;
	de->flags = flags;
	de->disp  = -(cd->dseglen);
	de->val.l = value;
	de->next  = cd->dseg;

	/* insert into the chain */

	cd->dseg = de;

	return de->disp;
}


/* dseg_add_unique_s8 **********************************************************

   Adds uniquely an s8 value to the data segment.

*******************************************************************************/

s4 dseg_add_unique_s8(codegendata *cd, s8 value)
{
	s4 disp;

	disp = dseg_add_s8_intern(cd, value, DSEG_FLAG_UNIQUE);

	return disp;
}


/* dseg_add_s8 *****************************************************************

   Adds an s8 value to the data segment. It tries to reuse previously
   added values.

*******************************************************************************/

s4 dseg_add_s8(codegendata *cd, s8 value)
{
	s4 disp;

	/* search the data segment if the value is already stored */

	disp = dseg_find_s8(cd, value);

	if (disp != 0)
		return disp;

	disp = dseg_add_s8_intern(cd, value, DSEG_FLAG_READONLY);

	return disp;
}


/* dseg_add_float_intern *******************************************************

   Internal function to add a float value to the data segment.

*******************************************************************************/

static s4 dseg_add_float_intern(codegendata *cd, float value, u4 flags)
{
	dsegentry *de;

	/* Increase data segment size, which is also the displacement into
	   the data segment. */

	cd->dseglen += 4;

	/* allocate new entry */

	de = DNEW(dsegentry);

	de->type  = TYPE_FLT;
	de->flags = flags;
	de->disp  = -(cd->dseglen);
	de->val.f = value;
	de->next  = cd->dseg;

	/* insert into the chain */

	cd->dseg = de;

	return de->disp;
}


/* dseg_add_unique_float *******************************************************

   Adds uniquely an float value to the data segment.

*******************************************************************************/

s4 dseg_add_unique_float(codegendata *cd, float value)
{
	s4 disp;

	disp = dseg_add_float_intern(cd, value, DSEG_FLAG_UNIQUE);

	return disp;
}


/* dseg_add_float **************************************************************

   Adds an float value to the data segment. It tries to reuse
   previously added values.

*******************************************************************************/

s4 dseg_add_float(codegendata *cd, float value)
{
	s4 disp;

	/* search the data segment if the value is already stored */

	disp = dseg_find_float(cd, value);

	if (disp != 0)
		return disp;

	disp = dseg_add_float_intern(cd, value, DSEG_FLAG_READONLY);

	return disp;
}


/* dseg_add_double_intern ******************************************************

   Internal function to add a double value to the data segment.

*******************************************************************************/

static s4 dseg_add_double_intern(codegendata *cd, double value, u4 flags)
{
	dsegentry *de;

	/* Increase data segment size, which is also the displacement into
	   the data segment. */

	cd->dseglen = MEMORY_ALIGN(cd->dseglen + 8, 8);

	/* allocate new entry */

	de = DNEW(dsegentry);

	de->type  = TYPE_DBL;
	de->flags = flags;
	de->disp  = -(cd->dseglen);
	de->val.d = value;
	de->next  = cd->dseg;

	/* insert into the chain */

	cd->dseg = de;

	return de->disp;
}


/* dseg_add_unique_double ******************************************************

   Adds uniquely a double value to the data segment.

*******************************************************************************/

s4 dseg_add_unique_double(codegendata *cd, double value)
{
	s4 disp;

	disp = dseg_add_double_intern(cd, value, DSEG_FLAG_UNIQUE);

	return disp;
}


/* dseg_add_double *************************************************************

   Adds a double value to the data segment. It tries to reuse
   previously added values.

*******************************************************************************/

s4 dseg_add_double(codegendata *cd, double value)
{
	s4 disp;

	/* search the data segment if the value is already stored */

	disp = dseg_find_double(cd, value);

	if (disp != 0)
		return disp;

	disp = dseg_add_double_intern(cd, value, DSEG_FLAG_READONLY);

	return disp;
}


/* dseg_add_address_intern *****************************************************

   Internal function to add an address pointer to the data segment.

*******************************************************************************/

static s4 dseg_add_address_intern(codegendata *cd, void *value, u4 flags)
{
	dsegentry *de;

	/* Increase data segment size, which is also the displacement into
	   the data segment. */

#if SIZEOF_VOID_P == 8
	cd->dseglen = MEMORY_ALIGN(cd->dseglen + 8, 8);
#else
	cd->dseglen += 4;
#endif

	/* allocate new entry */

	de = DNEW(dsegentry);

	de->type  = TYPE_ADR;
	de->flags = flags;
	de->disp  = -(cd->dseglen);
	de->val.a = value;
	de->next  = cd->dseg;

	/* insert into the chain */

	cd->dseg = de;

	return de->disp;
}


/* dseg_add_unique_address *****************************************************

   Adds uniquely an address value to the data segment.

*******************************************************************************/

s4 dseg_add_unique_address(codegendata *cd, void *value)
{
	s4 disp;

	disp = dseg_add_address_intern(cd, value, DSEG_FLAG_UNIQUE);

	return disp;
}


/* dseg_add_address ************************************************************

   Adds an address value to the data segment. It tries to reuse
   previously added values.

*******************************************************************************/

s4 dseg_add_address(codegendata *cd, void *value)
{
	s4 disp;

	/* search the data segment if the value is already stored */

	disp = dseg_find_address(cd, value);

	if (disp != 0)
		return disp;

	disp = dseg_add_address_intern(cd, value, DSEG_FLAG_READONLY);

	return disp;
}


/* dseg_add_target *************************************************************

   XXX

*******************************************************************************/

void dseg_add_target(codegendata *cd, basicblock *target)
{
	jumpref *jr;

	jr = DNEW(jumpref);

	jr->tablepos = dseg_add_unique_address(cd, NULL);
	jr->target   = target;
	jr->next     = cd->jumpreferences;

	cd->jumpreferences = jr;
}


/* dseg_adddata ****************************************************************

   Adds a data segment reference to the codegendata.

*******************************************************************************/

#if defined(__I386__) || defined(__X86_64__) || defined(__S390__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP)
void dseg_adddata(codegendata *cd)
{
	dataref *dr;

	dr = DNEW(dataref);

	dr->datapos = cd->mcodeptr - cd->mcodebase;
	dr->next    = cd->datareferences;

	cd->datareferences = dr;
}
#endif


/* dseg_resolve_datareferences *************************************************

   Resolve data segment references.

*******************************************************************************/

#if defined(__I386__) || defined(__X86_64__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP)
void dseg_resolve_datareferences(jitdata *jd)
{
	codeinfo    *code;
	codegendata *cd;
	dataref     *dr;

	/* get required compiler data */

	code = jd->code;
	cd   = jd->cd;

	/* data segment references resolving */

	for (dr = cd->datareferences; dr != NULL; dr = dr->next)
		*((u1 **) (code->entrypoint + dr->datapos - SIZEOF_VOID_P)) = code->entrypoint;
}
#endif


/* dseg_display ****************************************************************

   Displays the content of the methods' data segment.

*******************************************************************************/

#if !defined(NDEBUG)
void dseg_display(jitdata *jd)
{
	codeinfo    *code;
	codegendata *cd;
	dsegentry   *de;
	imm_union   val;

	/* get required compiler data */

	code = jd->code;
	cd   = jd->cd;

	if (opt_debugcolor)
		printf("\033[34m");	/* blue */

	printf("  --- dump of datasegment\n");

	/* process all data segment entries */

	for (de = cd->dseg; de != NULL; de = de->next) {
		printf("0x%0" PRINTF_INTPTR_NUM_HEXDIGITS PRIxPTR ":", (ptrint) (code->entrypoint + de->disp));

		printf("    %6x (%6d): ", de->disp, de->disp);

		/* We read the values from the data segment as some values,
		   like the line number table, have been written directly to
		   the data segment. */

		switch (de->type) {
		case TYPE_INT:
			val.i = *((s4 *) (code->entrypoint + de->disp));
			printf("(INT) %d (0x%08x)", val.i, val.i);
			break;

		case TYPE_LNG:
			val.l = *((s8 *) (code->entrypoint + de->disp));
			printf("(LNG) %" PRId64 " (0x%0" PRINTF_INTPTR_NUM_HEXDIGITS PRIx64 ")",
				   val.l, val.l);
			break;

		case TYPE_FLT:
			val.f = *((float *) (code->entrypoint + de->disp));
			printf("(FLT) %g (0x%08x)", val.f, val.i);
			break;

		case TYPE_DBL:
			val.d = *((double *) (code->entrypoint + de->disp));
			printf("(DBL) %g (0x%016" PRIx64 ")", val.d, val.l);
			break;

		case TYPE_ADR:
			val.a = *((void **) (code->entrypoint + de->disp));
			printf("(ADR) %0" PRINTF_INTPTR_NUM_HEXDIGITS PRIxPTR, (ptrint) val.a);
			break;
		}

		printf("\n");
	}

	printf("  --- begin of data segment: ");
	printf("0x%0" PRINTF_INTPTR_NUM_HEXDIGITS PRIxPTR "\n", (ptrint) code->entrypoint);

	if (opt_debugcolor)
		printf("\033[m");
}
#endif /* !defined(NDEBUG) */


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
