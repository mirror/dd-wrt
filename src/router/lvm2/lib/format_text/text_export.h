/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2005 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_TEXT_EXPORT_H
#define _LVM_TEXT_EXPORT_H

#define outsize(args...) do {if (!out_size(args)) return_0;} while (0)
#define outhint(args...) do {if (!out_hint(args)) return_0;} while (0)
#define outfc(args...) do {if (!out_text_with_comment(args)) return_0;} while (0)
#define outf(args...) do {if (!out_text(args)) return_0;} while (0)
#define outfgo(args...) do {if (!out_text(args)) goto_out;} while (0)
#define outnl(f) do {if (!out_newline(f)) return_0;} while (0)
#define outnlgo(f) do {if (!out_newline(f)) goto_out;} while (0)

struct formatter;
struct lv_segment;
struct dm_config_node;

int out_size(struct formatter *f, uint64_t size, const char *fmt, ...)
    __attribute__ ((format(printf, 3, 4)));

int out_hint(struct formatter *f, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

int out_text(struct formatter *f, const char *fmt, ...)
    __attribute__ ((format(printf, 2, 3)));

int out_config_node(struct formatter *f, const struct dm_config_node *cn);

int out_areas(struct formatter *f, const struct lv_segment *seg,
	      const char *type);

int out_text_with_comment(struct formatter *f, const char* comment, const char *fmt, ...)
    __attribute__ ((format(printf, 3, 4)));

void out_inc_indent(struct formatter *f);
void out_dec_indent(struct formatter *f);
int out_newline(struct formatter *f);

#endif
