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

typedef void (*adfnc_t)(void *obj, int bit, typnm_t next);

extern void	fa_agblock(void *obj, int bit, typnm_t next);
extern void	fa_agino(void *obj, int bit, typnm_t next);
extern void	fa_attrblock(void *obj, int bit, typnm_t next);
extern void	fa_cfileoffd(void *obj, int bit, typnm_t next);
extern void	fa_cfsblock(void *obj, int bit, typnm_t next);
extern void	fa_dfiloffd(void *obj, int bit, typnm_t next);
extern void	fa_dfsbno(void *obj, int bit, typnm_t next);
extern void	fa_dinode_union(void *obj, int bit, typnm_t next);
extern void	fa_dirblock(void *obj, int bit, typnm_t next);
extern void	fa_drfsbno(void *obj, int bit, typnm_t next);
extern void	fa_drtbno(void *obj, int bit, typnm_t next);
extern void	fa_ino(void *obj, int bit, typnm_t next);
extern void	fa_cfileoffa(void *obj, int bit, typnm_t next);
extern void	fa_dfiloffa(void *obj, int bit, typnm_t next);
extern void	fa_ino4(void *obj, int bit, typnm_t next);
extern void	fa_ino8(void *obj, int bit, typnm_t next);
