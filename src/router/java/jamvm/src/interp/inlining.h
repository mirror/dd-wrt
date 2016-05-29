/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
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

#define LABELS_SIZE  256
#define GOTO_START   230
#define GOTO_END     255

#ifdef USE_CACHE
#define HANDLERS      3
#define ENTRY_LABELS  0
#define START_LABELS  3
#define END_LABELS    6
#define BRANCH_LABELS 9
#else
#define HANDLERS      1
#define ENTRY_LABELS  0
#define START_LABELS  1
#define END_LABELS    2
#define BRANCH_LABELS 3
#endif

/* Number of entries in the branch patch table */
#define BRANCH_NUM    16

#define MEMCMP_FAILED    -1
#define END_REORDERED    -2
#define END_BEFORE_ENTRY -3

#define FALLTHROUGH 1
#define HANDLER     2
#define TARGET      4
#define END         8

#ifdef INLINING
extern uintptr_t *executeJava2();
extern int calculateRelocatability(int handler_sizes[HANDLERS][LABELS_SIZE]);
#endif
