/*
 * dstore.c - OpenBSD global storage for lsof
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef lint
static char copyright[] =
    "@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
#endif

#include "common.h"

#if defined(HASFSTRUCT)
/*
 * Pff_tab[] - table for printing file flags
 */

struct pff_tab Pff_tab[] = {
    {(long)FREAD, FF_READ},       {(long)FWRITE, FF_WRITE},
    {(long)FNONBLOCK, FF_NBLOCK}, {(long)FNDELAY, FF_NDELAY},
    {(long)FAPPEND, FF_APPEND},   {(long)FASYNC, FF_ASYNC},

#    if defined(FDSYNC)
    {(long)FDSYNC, FF_DSYNC},
#    endif /* defined*FDSYNC) */

    {(long)FFSYNC, FF_FSYNC},

#    if defined(FRSYNC)
    {(long)FRSYNC, FF_RSYNC},
#    endif /* defined(FRSYNC) */

#    if defined(FMARK)
    {(long)FMARK, FF_MARK},
#    endif /* defined(FMARK) */

#    if defined(FDEFER)
    {(long)FDEFER, FF_DEFER},
#    endif /* defined(FDEFER) */

#    if defined(FHASLOCK)
    {(long)FHASLOCK, FF_HASLOCK},
#    endif /* defined(FHASLOCK) */
    {(long)O_NOCTTY, FF_NOCTTY},  {(long)0, NULL}};

/*
 * Pof_tab[] - table for print process open file flags
 */

struct pff_tab Pof_tab[] = {

#    if defined(UF_EXCLOSE)
    {(long)UF_EXCLOSE, POF_CLOEXEC},
#    else
    {(long)1, POF_CLOEXEC},
#    endif /* defined(UF_EXCLOSE) */

#    if defined(UF_MAPPED)
    {(long)UF_MAPPED, POF_MAPPED},
#    endif /* defined(UF_MAPPED) */

    {(long)0, NULL}};
#endif /* defined(HASFSTRUCT) */

int pgshift = 0; /* kernel's page shift */
