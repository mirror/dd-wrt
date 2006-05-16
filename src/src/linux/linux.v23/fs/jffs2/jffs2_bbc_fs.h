/*
 * JFFS2 BBC: File System Extension for Linux Kernel - headers
 *
 * $Id: jffs2_bbc_fs.h,v 1.1 2005/09/01 11:59:15 seg Exp $
 *
 * Copyright (C) 2004, Ferenc Havasi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

extern int jffs2_bbc_inode_not_found;

void jffs2_bbc_load_model(void *sb);
void jffs2_bbc_unload_model(void *sb);

void jffs2_bbc_proc_init(void);
void jffs2_bbc_proc_deinit(void);
