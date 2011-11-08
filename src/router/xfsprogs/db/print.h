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

struct field;
struct flist;

extern void	print_flist(struct flist *flist);
extern void	print_init(void);
extern void	print_sarray(void *obj, int bit, int count, int size, int base,
			     int array, const field_t *flds, int skipnms);
extern void	print_struct(const struct field *fields, int argc, char **argv);
extern void	print_string(const struct field *fields, int argc, char **argv);
