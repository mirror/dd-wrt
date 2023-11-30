/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */


#ifndef _BATCTL_ALLOCATE_H
#define _BATCTL_ALLOCATE_H

/* debug allocate wrapper to keep hash.c happy */

#include <stdlib.h>

#define debugMalloc(length, tag) malloc(length)
#define debugFree(mem, tag) free(mem)

#endif
