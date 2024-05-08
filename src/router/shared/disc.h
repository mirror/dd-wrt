#define NEED_PRINTF
/*
 * global.h
 * Common header file.
 *
 * Copyright (c) 2003 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#define PROGNAME "disktype"

/* global includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

/* constants */

#define FLAG_IN_DISKLABEL (0x0001)

/* types */

typedef signed char s1;
typedef unsigned char u1;
typedef short int s2;
typedef unsigned short int u2;
typedef long int s4;
typedef unsigned long int u4;
typedef long long int s8;
typedef unsigned long long int u8;

typedef struct source {
	u8 size;
	int size_known;
	void *cache_head;

	int sequential;
	u8 seq_pos;
	int blocksize;
	struct source *foundation;

	int (*analyze)(struct source *s, int level);
	u8 (*read_bytes)(struct source *s, u8 pos, u8 len, void *buf);
	int (*read_block)(struct source *s, u8 pos, void *buf);
	void (*close)(struct source *s);

	/* private data may follow */
} SOURCE;

typedef struct section {
	u8 pos, size;
	int flags;
	SOURCE *source;
} SECTION;

typedef int (*DETECTOR)(SECTION *section, int level);

/* detection dispatching functions */

void analyze_source(SOURCE *s, int level);
void analyze_source_special(SOURCE *s, int level, u8 pos, u8 size);
void analyze_recursive(SECTION *section, int level, u8 rel_pos, u8 size, int flags);
void stop_detect(void);

/* file source functions */

SOURCE *init_file_source(int fd, int filekind);

int analyze_cdaccess(int fd, SOURCE *s, int level);

/* buffer functions */

u8 get_buffer(SECTION *section, u8 pos, u8 len, void **buf);
u8 get_buffer_real(SOURCE *s, u8 pos, u8 len, void *inbuf, void **outbuf);
void close_source(SOURCE *s);

/* output functions */

void print_line(int level, const char *fmt, ...);
void start_line(const char *fmt, ...);
void continue_line(const char *fmt, ...);
void finish_line(int level);

/* formatting functions */

void format_blocky_size(char *buf, u8 count, u4 blocksize, const char *blockname, const char *append);
void format_size(char *buf, u8 size);
void format_size_verbose(char *buf, u8 size);

void format_ascii(void *from, char *to);
void format_utf16_be(void *from, u4 len, char *to);
void format_utf16_le(void *from, u4 len, char *to);

void format_uuid(void *from, char *to);
void format_uuid_lvm(void *uuid, char *to);
void format_guid(void *guid, char *to);

/* endian-aware data access */

u2 get_be_short(void *from);
u4 get_be_long(void *from);
u8 get_be_quad(void *from);

u2 get_le_short(void *from);
u4 get_le_long(void *from);
u8 get_le_quad(void *from);

u2 get_ve_short(int endianness, void *from);
u4 get_ve_long(int endianness, void *from);
u8 get_ve_quad(int endianness, void *from);

const char *get_ve_name(int endianness);

/* more data access */

void get_string(void *from, int len, char *to);
void get_pstring(void *from, char *to);
void get_padded_string(void *from, int len, char pad, char *to);

int find_memory(void *haystack, int haystack_len, void *needle, int needle_len);

/* name table lookups */

char *get_name_for_mbrtype(int type);

/* error functions */

void error(const char *msg, ...);
void errore(const char *msg, ...);
void bailout(const char *msg, ...);
void bailoute(const char *msg, ...);

void set_discmessage_on(void);
void set_discmessage_off(void);


/* in amiga.c */
int detect_amiga_partmap(SECTION *section, int level);
int detect_amiga_fs(SECTION *section, int level);

/* in apple.c */
int detect_apple_partmap(SECTION *section, int level);
int detect_apple_volume(SECTION *section, int level);
int detect_udif(SECTION *section, int level);

/* in atari.c */
int detect_atari_partmap(SECTION *section, int level);

/* in dos.c */
int detect_dos_partmap(SECTION *section, int level);
int detect_gpt_partmap(SECTION *section, int level);
int detect_fat(SECTION *section, int level);
int detect_exfat(SECTION *section, int level);
int detect_ntfs(SECTION *section, int level);
int detect_hpfs(SECTION *section, int level);
int detect_apfs_volume(SECTION *section, int level);
int detect_dos_loader(SECTION *section, int level);

/* in cdrom.c */
int detect_iso(SECTION *section, int level);
int detect_cdrom_misc(SECTION *section, int level);

/* in udf.c */
int detect_udf(SECTION *section, int level);

/* in linux.c */
int detect_ext234(SECTION *section, int level);
int detect_btrfs(SECTION *section, int level);
int detect_zfs(SECTION *section, int level);
int detect_f2fs(SECTION *section, int level);
int detect_reiser(SECTION *section, int level);
int detect_reiser4(SECTION *section, int level);
int detect_linux_raid(SECTION *section, int level);
int detect_linux_md(SECTION *section, int level);
int detect_linux_lvm(SECTION *section, int level);
int detect_linux_lvm2(SECTION *section, int level);
int detect_linux_swap(SECTION *section, int level);
int detect_linux_misc(SECTION *section, int level);
int detect_linux_loader(SECTION *section, int level);

/* in unix.c */
int detect_jfs(SECTION *section, int level);
int detect_xfs(SECTION *section, int level);
int detect_ufs(SECTION *section, int level);
int detect_sysv(SECTION *section, int level);
int detect_bsd_disklabel(SECTION *section, int level);
int detect_bsd_loader(SECTION *section, int level);
int detect_solaris_disklabel(SECTION *section, int level);
int detect_solaris_vtoc(SECTION *section, int level);
int detect_qnx(SECTION *section, int level);
int detect_vxfs(SECTION *section, int level);

/* in beos.c */
int detect_bfs(SECTION *section, int level);
int detect_beos_loader(SECTION *section, int level);

/* in compressed.c */
int detect_compressed(SECTION *section, int level);

/* in cdimage.c */
int detect_cdimage(SECTION *section, int level);

/* in vpc.c */
int detect_vhd(SECTION *section, int level);

/* in cloop.c */
int detect_cloop(SECTION *section, int level);

/* in archives.c */
int detect_archive(SECTION *section, int level);

/* in blank.c */
int detect_blank(SECTION *section, int level);
int analyze_stat(struct stat *sb, const char *filename);

/* EOF */
