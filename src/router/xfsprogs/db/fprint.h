/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

typedef int (*prfnc_t)(void *obj, int bit, int count, char *fmtstr, int size,
		       int arg, int base, int array);

extern int	fp_charns(void *obj, int bit, int count, char *fmtstr, int size,
			  int arg, int base, int array);
extern int	fp_num(void *obj, int bit, int count, char *fmtstr, int size,
		       int arg, int base, int array);
extern int	fp_sarray(void *obj, int bit, int count, char *fmtstr, int size,
			  int arg, int base, int array);
extern int	fp_time(void *obj, int bit, int count, char *fmtstr, int size,
			int arg, int base, int array);
extern int	fp_uuid(void *obj, int bit, int count, char *fmtstr, int size,
			int arg, int base, int array);
