/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
 * Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* Must be included first to get configure options */
#include "jam.h"

#ifdef INLINING
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inlining.h"

int compare(const void *pntr1, const void *pntr2) {
    char *v1 = *(char **)pntr1;
    char *v2 = *(char **)pntr2;

    return v1 - v2;
}

char *findNextLabel(char **pntrs, char *pntr) {
    int i = 0;

    for(i = 0; i < LABELS_SIZE; i++)
        if(pntrs[i] > pntr)
            return pntrs[i];

    return NULL;
}

int calculateRelocatability(int handler_sizes[HANDLERS][LABELS_SIZE]) {
    char ***handlers1 = (char ***)executeJava();
    char ***handlers2 = (char ***)executeJava2();
    char *sorted_ends[LABELS_SIZE];
    char *goto_start;
    int goto_len;
    int i;

    /* Check relocatability of the indirect goto.  This is copied onto the end
       of each super-instruction.  If this is un-relocatable,  inlining is
       disabled. */

    goto_start = handlers1[ENTRY_LABELS][GOTO_START];
    goto_len = handlers1[ENTRY_LABELS][GOTO_END] - goto_start;

    if(goto_len <= 0)
        goto_len = END_BEFORE_ENTRY;
    else
        if(memcmp(goto_start, handlers2[ENTRY_LABELS][GOTO_START], goto_len) != 0)
            goto_len = MEMCMP_FAILED;

    /* Check relocatability of each handler. */

    for(i = 0; i < HANDLERS; i++) {
        int j;

        memcpy(sorted_ends, handlers1[END_LABELS+i], LABELS_SIZE * sizeof(char *));
        qsort(sorted_ends, LABELS_SIZE, sizeof(char *), compare);

        for(j = 0; j < LABELS_SIZE; j++) {
            char *entry = handlers1[ENTRY_LABELS+i][j];
            char *end = handlers1[END_LABELS+i][j];
            int len = end - entry;

            if(len > 0) {
                char *nearest_end = findNextLabel(sorted_ends, entry);

                if(nearest_end == end) {
                    if(memcmp(entry, handlers2[ENTRY_LABELS+i][j], len) != 0)
                        len = MEMCMP_FAILED;
                } else
                    len = END_REORDERED;
            } else
                len = END_BEFORE_ENTRY;

            handler_sizes[i][j] = len;
        }
    }

    return goto_len;
}
#endif

