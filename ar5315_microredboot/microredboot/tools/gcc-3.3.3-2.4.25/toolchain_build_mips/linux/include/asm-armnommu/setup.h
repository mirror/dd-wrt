/*
 *  linux/include/asm/setup.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Structure passed to kernel to tell it about the
 *  hardware it's running on.  See linux/Documentation/arm/Setup
 *  for more info.
 */
#ifndef __ASMARM_SETUP_H
#define __ASMARM_SETUP_H

/*
 * Usage:
 *  - do not go blindly adding fields, add them at the end
 *  - when adding fields, don't rely on the address until
 *    a patch from me has been released
 *  - unused fields should be zero (for future expansion)
 *  - this structure is relatively short-lived - only
 *    guaranteed to contain useful data in setup_arch()
 */
#define COMMAND_LINE_SIZE 1024

struct param_struct {
    union {
	struct {
	    unsigned long page_size;		/*  0 */
	    unsigned long nr_pages;		/*  4 */
	    unsigned long ramdisk_size;		/*  8 */
	    unsigned long flags;		/* 12 */
#define FLAG_READONLY	1
#define FLAG_RDLOAD	4
#define FLAG_RDPROMPT	8
	    unsigned long rootdev;		/* 16 */
	    unsigned long video_num_cols;	/* 20 */
	    unsigned long video_num_rows;	/* 24 */
	    unsigned long video_x;		/* 28 */
	    unsigned long video_y;		/* 32 */
	    unsigned long memc_control_reg;	/* 36 */
	    unsigned char sounddefault;		/* 40 */
	    unsigned char adfsdrives;		/* 41 */
	    unsigned char bytes_per_char_h;	/* 42 */
	    unsigned char bytes_per_char_v;	/* 43 */
	    unsigned long pages_in_bank[4];	/* 44 */
	    unsigned long pages_in_vram;	/* 60 */
	    unsigned long initrd_start;		/* 64 */
	    unsigned long initrd_size;		/* 68 */
	    unsigned long rd_start;		/* 72 */
	    unsigned long system_rev;		/* 76 */
	    unsigned long system_serial_low;	/* 80 */
	    unsigned long system_serial_high;	/* 84 */
	    unsigned long mem_fclk_21285;       /* 88 */
	} s;
	char unused[256];
    } u1;
    union {
	char paths[8][128];
	struct {
	    unsigned long magic;
	    char n[1024 - sizeof(unsigned long)];
	} s;
    } u2;
    char commandline[COMMAND_LINE_SIZE];
};

/*
 * New idea - a list of tagged entries
 */
#define ATAG_NONE	0x00000000

struct tag_header {
	u32 size;
	u32 tag;
};

#define ATAG_CORE	0x54410001

struct tag_core {
	u32 flags;		/* bit 0 = read-only */
	u32 pagesize;
	u32 rootdev;
};

#define ATAG_MEM		0x54410002

struct tag_mem32 {
	u32	size;
	u32	start;
};

#define ATAG_VIDEOTEXT	0x54410003

struct tag_videotext {
	u8		x;
	u8		y;
	u16		video_page;
	u8		video_mode;
	u8		video_cols;
	u16		video_ega_bx;
	u8		video_lines;
	u8		video_isvga;
	u16		video_points;
};

#define ATAG_RAMDISK	0x54410004

struct tag_ramdisk {
	u32 flags;		/* b0 = load, b1 = prompt */
	u32 size;
	u32 start;
};

#define ATAG_INITRD	0x54410005

struct tag_initrd {
	u32 start;
	u32 size;
};

#define ATAG_SERIAL	0x54410006

struct tag_serialnr {
	u32 low;
	u32 high;
};

#define ATAG_REVISION	0x54410007

struct tag_revision {
	u32 rev;
};

#define ATAG_VIDEOLFB	0x54410008

struct tag_videolfb {
	u16		lfb_width;
	u16		lfb_height;
	u16		lfb_depth;
	u16		lfb_linelength;
	u32		lfb_base;
	u32		lfb_size;
	u8		red_size;
	u8		red_pos;
	u8		green_size;
	u8		green_pos;
	u8		blue_size;
	u8		blue_pos;
	u8		rsvd_size;
	u8		rsvd_pos;
};

#define ATAG_CMDLINE	0x54410009

struct tag_cmdline {
	char	cmdline[1];
};

#define ATAG_ACORN	0x41000101

struct tag_acorn {
	u32 memc_control_reg;
	u32 vram_pages;
	u8 sounddefault;
	u8 adfsdrives;
};

#define ATAG_MEMCLK	0x41000402

struct tag_memclk {
	u32 fmemclk;
};

struct tag {
	struct tag_header hdr;
	union {
		struct tag_core		core;
		struct tag_mem32	mem;
		struct tag_videotext	videotext;
		struct tag_ramdisk	ramdisk;
		struct tag_initrd	initrd;
		struct tag_serialnr	serialnr;
		struct tag_revision	revision;
		struct tag_videolfb	videolfb;
		struct tag_cmdline	cmdline;

		/*
		 * Acorn specific
		 */
		struct tag_acorn	acorn;

		/*
		 * DC21285 specific
		 */
		struct tag_memclk	memclk;
	} u;
};

struct tagtable {
	u32 tag;
	int (*parse)(const struct tag *);
};

#define __tag __attribute__((unused, __section__(".taglist")))
#define __tagtable(tag, fn) \
static struct tagtable __tagtable_##fn __tag = { tag, fn }

#define tag_member_present(tag,member)				\
	((unsigned long)(&((struct tag *)0L)->member + 1)	\
		<= (tag)->hdr.size * 4)

#define tag_next(t)	((struct tag *)((u32 *)(t) + (t)->hdr.size))
#define tag_size(type)	((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

#define for_each_tag(t,base)       \
	for (t = base; t->hdr.size; t = tag_next(t))

/*
 * Memory map description
 */
#define NR_BANKS 8

struct meminfo {
	int nr_banks;
	unsigned long end;
	struct {
		unsigned long start;
		unsigned long size;
		int           node;
	} bank[NR_BANKS];
};

extern struct meminfo meminfo;

#endif
