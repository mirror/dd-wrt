/*
  Copyright (c) 2014, Matthias Schiffer <mschiffer at universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*
   mktplinkfw3

   Image generation tool for the TP-LINK SafeLoader as seen on
   TP-LINK Pharos devices (CPE210/220/510/520)
*/


#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <byteswap.h>

#include "md5.h"


#define ALIGN(x,a) ({ typeof(a) __a = (a); (((x) + __a - 1) & ~(__a - 1)); })


/** An image partition table entry */
struct image_partition_entry {
	const char *name;
	size_t size;
	uint8_t *data;
};

/** A flash partition table entry */
struct flash_partition_entry {
	const char *name;
	uint32_t base;
	uint32_t size;
};


static const uint8_t jffs2_eof_mark[4] = {0xde, 0xad, 0xc0, 0xde};


/**
   Salt for the MD5 hash

   Fortunately, TP-LINK seems to use the same salt for most devices which use
   the new image format.
*/
static const uint8_t md5_salt[16] = {
	0x7a, 0x2b, 0x15, 0xed,
	0x9b, 0x98, 0x59, 0x6d,
	0xe5, 0x04, 0xab, 0x44,
	0xac, 0x2a, 0x9f, 0x4e,
};


/** Vendor information for CPE210/220/510/520 */
static const unsigned char cpe510_vendor[] = "\x00\x00\x00\x1f""CPE510(TP-LINK|UN|N300-5):1.0\r\n";

/** Vendor information for 646*/
static const unsigned char cpe646_vendor[] = "fw-type:Cloud\n";

/**
    The flash partition table for CPE210/220/510/520

    It is the same as the one used by the stock images,
    with one exception: The soft-version partition between
    the os-image and the support-list has been removed
    to get the same flash content after flashing factory and
    sysupgrade.
*/
static const struct flash_partition_entry cpe510_partitions[] = {
	{"fs-uboot", 0x00000, 0x40000},
	{"os-image", 0x40000, 0x200000},
	{"file-system", 0x240000, 0xc00000},
	{"default-mac", 0xe40000, 0x000200},
	{"pin", 0xe40200, 0x00200},
	{"product-info", 0xe40400, 0x00200},
	{"partition-table", 0xe50000, 0x010000},
	{"soft-version", 0xe60000, 0x00200},
	{"support-list", 0xe61000, 0x0f000},
	{"profile", 0xe70000, 0x10000},
	{"default-config", 0xe80000, 0x10000},
	{"user-config", 0xe90000, 0x50000},
	{"log", 0xee0000, 0x100000},
	{"radio_bk", 0xfe0000, 0x10000},
	{"radio", 0xff0000, 0x10000},
	{NULL, 0, 0}
};

/**
    The flash partition table for 0646

    It is the same as the one used by the stock images,
    with one exception: The soft-version partition between
    the os-image and the support-list has been removed
    to get the same flash content after flashing factory and
    sysupgrade.
*/
static const struct flash_partition_entry cpe646_partitions[] = {
	{"factory-boot", 0x00000, 0x40000},
	{"fs-uboot", 0x40000, 0x40000},
	{"os-image", 0x80000, 0x200000},
	{"file-system", 0x280000, 0xc80000},
	{"default-mac", 0xf00000, 0x000200},
	{"pin", 0xf00200, 0x00100},
	{"device-id", 0xf00300, 0x00100},
	{"product-info", 0xf10000, 0x006000},
	{"soft-version", 0xf16000, 0x01000},
	{"extra-para", 0xf17000, 0x01000},
	{"support-list", 0xf18000, 0x008000},
	{"profile", 0xf20000, 0x03000},
	{"default-config", 0xf23000, 0x0d000},
	{"user-config", 0xf30000, 0x40000},
	{"qos-db", 0xf70000, 0x40000},
	{"partition-table", 0xfb0000, 0x010000},
	{"log", 0xfc0000, 0x20000},
	{"radio_bk", 0xfe0000, 0x10000},
	{"radio", 0xff0000, 0x10000},
	{NULL, 0, 0}
};

/**
   The support list for CPE210/220/510/520

   The stock images also contain strings for two more devices: BS510 and BS210.
   At the moment, there exists no public information about these devices.
*/
static const unsigned char archerc9_support_list[] =
	"\x00\x00\x00\x4b\x00\x00\x00\x00"
	"SupportList:\n"
	"{product_name:ArcherC9,"
	"product_ver:1.0.0,"
	"special_id:00000000}\n"
	"\x00";

static const unsigned char archerc9v2_support_list[] =
	"\x00\x00\x00\x4b\x00\x00\x00\x00"
	"SupportList:\n"
	"{product_name:ArcherC9,"
	"product_ver:2.0.0,"
	"special_id:00000000}\n"
	"\x00";

static const unsigned char archerc1900_support_list[] =
	"\x00\x00\x00\x5a\x00\x00\x00\x00"
	"SupportList:\n"
	"{product_name:ArcherC1900,product_ver:1.0.0,language=US,special_id:55534100}\n" //US 
	"\x00";

static const unsigned char archerc8_support_list[] =
	"\x00\x00\x00\x4b\x00\x00\x00\x00"
	"SupportList:\n"
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:00000000}\n" // UNIVERSAL
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:55530000}\n" //US 
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:45550000}\n" //EU
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:4B520000}\n" //KR
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:42520000}\n" //BR
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:4A500000}\n" //JP
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:43410000}\n" //CA
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:41550000}\n" //AU
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:52550000}\n" //RU
	"{product_name:ArcherC8,product_ver:1.0.0,special_id:54570000}\n" // TW
	"\x00";

static const unsigned char archerc8v2_support_list[] =
	"\x00\x00\x00\x4b\x00\x00\x00\x00"
	"SupportList:\n"
	"{product_name:ArcherC8,"
	"product_ver:2.0.0,"
	"special_id:00000000}"
	"\x00";

static const unsigned char archerc8v3_support_list[] =
	"\x00\x00\x00\x4b\x00\x00\x00\x00"
	"SupportList:\n"
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:00000000}\n" // UNIVERSAL
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:55530000}\n" //US 
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:45550000}\n" //EU
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:4B520000}\n" //KR
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:42520000}\n" //BR
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:4A500000}\n" //JP
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:43410000}\n" //CA
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:41550000}\n" //AU
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:52550000}\n" //RU
	"{product_name:ArcherC8,product_ver:3.0.0,special_id:54570000}\n" // TW
	"\x00";

static const unsigned char softversion[] =
	"soft_ver:1.1.2 Build 20180126 rel.58698\n"
	"fw_id:4CC8BBF897C91ED05745C2A56463A481\x00";

static const unsigned char extra_para[] =
	"\x00\x00\x00\x02\x00\x00\x00\x00\x01\x00\x00";



/** Allocates a new image partition */
struct image_partition_entry alloc_image_partition(const char *name, size_t len) {
	struct image_partition_entry entry = {name, len, malloc(len)};
	if (!entry.data)
		error(1, errno, "malloc");

	return entry;
}

/** Frees an image partition */
void free_image_partition(struct image_partition_entry entry) {
	free(entry.data);
}

/** Generates the partition-table partition */
struct image_partition_entry make_partition_table_v1(const struct flash_partition_entry *p) {
	struct image_partition_entry entry = alloc_image_partition("partition-table", 0x800);

	char *s = (char*)entry.data, *end = (char*)(s+entry.size);

	*(s++) = 0x00;
	*(s++) = 0x04;
	*(s++) = 0x00;
	*(s++) = 0x00;

	size_t i;
	for (i = 0; p[i].name; i++) {
		size_t len = end-s;
		size_t w = snprintf(s, len, "partition %s base 0x%05x size 0x%05x\n", p[i].name, p[i].base, p[i].size);

		if (w > len-1)
			error(1, 0, "flash partition table overflow?");

		s += w;
	}
	*s=0;
	memset(s+1, 0xff, end-s-1);

	return entry;
}

struct image_partition_entry make_partition_table(const struct flash_partition_entry *p) {
	struct image_partition_entry entry = alloc_image_partition("partition-table", 0x800);

	char *s = (char*)entry.data, *end = (char*)(s+entry.size);

	*(s++) = 0x00;
	*(s++) = 0x08;
	*(s++) = 0x00;
	*(s++) = 0x00;

	size_t i;
	for (i = 0; p[i].name; i++) {
		size_t len = end-s;
		size_t w = snprintf(s, len, "partition %s base 0x%05x size 0x%05x\n", p[i].name, p[i].base, p[i].size);

		if (w > len-1)
			error(1, 0, "flash partition table overflow?");

		s += w;
	}
	*s=0;
	memset(s+1, 0xff, end-s-1);

	return entry;
}

/** Generates the support-list partition */
struct image_partition_entry make_support_list(const unsigned char *support_list, size_t len) {
	struct image_partition_entry entry = alloc_image_partition("support-list", len);
	memcpy(entry.data, support_list, len);
	return entry;
}

struct image_partition_entry make_softversion(const unsigned char *support_list, size_t len) {
	struct image_partition_entry entry = alloc_image_partition("soft-version", len);
	memcpy(entry.data, support_list, len);
	return entry;
}

struct image_partition_entry make_extra_para(const unsigned char *extra_para, size_t len) {
	struct image_partition_entry entry = alloc_image_partition("extra-para", len);
	memcpy(entry.data, extra_para, len);
	return entry;
}

/** Creates a new image partition with an arbitrary name from a file */
struct image_partition_entry read_file(const char *part_name, const char *filename, bool add_jffs2_eof) {
	struct stat statbuf;

	if (stat(filename, &statbuf) < 0)
		error(1, errno, "unable to stat file `%s'", filename);

	size_t len = statbuf.st_size;

	if (add_jffs2_eof)
		len = ALIGN(len, 0x10000) + sizeof(jffs2_eof_mark);

	struct image_partition_entry entry = alloc_image_partition(part_name, len);

	FILE *file = fopen(filename, "rb");
	if (!file)
		error(1, errno, "unable to open file `%s'", filename);

	if (fread(entry.data, statbuf.st_size, 1, file) != 1)
		error(1, errno, "unable to read file `%s'", filename);

	if (add_jffs2_eof) {
		uint8_t *eof = entry.data + statbuf.st_size, *end = entry.data+entry.size;

		memset(eof, 0xff, end - eof - sizeof(jffs2_eof_mark));
		memcpy(end - sizeof(jffs2_eof_mark), jffs2_eof_mark, sizeof(jffs2_eof_mark));
	}

	fclose(file);

	return entry;
}


/**
   Copies a list of image partitions into an image buffer and generates the image partition table while doing so

   Example image partition table:

     fwup-ptn partition-table base 0x00800 size 0x00800
     fwup-ptn os-image base 0x01000 size 0x113b45
     fwup-ptn file-system base 0x114b45 size 0x1d0004
     fwup-ptn support-list base 0x2e4b49 size 0x000d1

   Each line of the partition table is terminated with the bytes 09 0a 0d ("\t\n\r"),
   the end of the partition table is marked with a zero byte.

   The firmware image must contain at least the partition-table and support-list partitions
   to be accepted. There aren't any alignment constraints for the image partitions.

   The partition-table partition contains the actual flash layout; partitions
   from the image partition table are mapped to the corresponding flash partitions during
   the firmware upgrade. The support-list partition contains a list of devices supported by
   the firmware image.

   The base offsets in the firmware partition table are relative to the end
   of the vendor information block, so the partition-table partition will
   actually start at offset 0x1814 of the image.

   I think partition-table must be the first partition in the firmware image.
*/
void put_partitions(uint8_t *buffer, const struct image_partition_entry *parts) {
	size_t i;
	char *image_pt = (char *)buffer, *end = image_pt + 0x800;
	size_t bases[7];
	size_t base = 0x800;
	for (i = 0; parts[i].name; i++) {
		memcpy(buffer + base, parts[i].data, parts[i].size);
		bases[i]=base;
		base += parts[i].size;
	}


	for (i = 1; i<4; i++) {
		size_t len = end-image_pt;
		size_t w = snprintf(image_pt, len, "fwup-ptn %s base 0x%05x size 0x%05x\t\r\n", parts[i].name, (unsigned)bases[i], (unsigned)parts[i].size);

		if (w > len-1)
			error(1, 0, "image partition table overflow?");
		image_pt += w;
	}
	// dont ask. evil hack required for tplinks c9, layout description is not in same order as binary and tplink doesnt accept any other order
		size_t len = end-image_pt;
		size_t w = snprintf(image_pt, len, "fwup-ptn %s base 0x%05x size 0x%05x\t\r\n", parts[0].name, (unsigned)bases[0], (unsigned)parts[0].size);
		image_pt += w;
	for (i = 4; parts[i].name; i++) {

		size_t len = end-image_pt;
		size_t w = snprintf(image_pt, len, "fwup-ptn %s base 0x%05x size 0x%05x\t\r\n", parts[i].name, (unsigned)bases[i], (unsigned)parts[i].size);

		if (w > len-1)
			error(1, 0, "image partition table overflow?");
		image_pt += w;
	}
	*image_pt=0; 
	memset(image_pt+1, 0xff, end-image_pt-1);
}

/** Generates and writes the image MD5 checksum */
void put_md5(uint8_t *md5, uint8_t *buffer, unsigned int len) {
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, md5_salt, (unsigned int)sizeof(md5_salt));
	MD5_Update(&ctx, buffer, len);
	MD5_Final(md5, &ctx);
}


/**
   Generates the firmware image in factory format

   Image format:

     Bytes (hex)  Usage
     -----------  -----
     0000-0003    Image size (4 bytes, big endian)
     0004-0013    MD5 hash (hash of a 16 byte salt and the image data starting with byte 0x14)
     0014-1013    Vendor information (4096 bytes, padded with 0xff; there seem to be older
                  (VxWorks-based) TP-LINK devices which use a smaller vendor information block)
     1014-1813    Image partition table (2048 bytes, padded with 0xff)
     1814-xxxx    Firmware partitions
*/
void * generate_factory_image(const unsigned char *vendor, size_t vendor_len, const struct image_partition_entry *parts, size_t *len) {
	*len = 0x1814;

	size_t i;
	for (i = 0; parts[i].name; i++)
		*len += parts[i].size;

	uint8_t *image = malloc(*len);
	if (!image)
		error(1, errno, "malloc");

	image[0] = *len >> 24;  //first 4, length of the firmware
	image[1] = *len >> 16;
	image[2] = *len >> 8;
	image[3] = *len;

	// memcpy(image+0x14, vendor, vendor_len);
	// memset(image+0x14+vendor_len, 0xff, 4096);
    memset(image+0x14, 0xff, 4096);

	put_partitions(image + 0x1014, parts);
	put_md5(image+0x04, image+0x14, *len-0x14);

	return image;
}

/**
   Generates the firmware image in sysupgrade format

   This makes some assumptions about the provided flash and image partition tables and
   should be generalized when TP-LINK starts building its safeloader into hardware with
   different flash layouts.
*/
void * generate_sysupgrade_image(const struct flash_partition_entry *flash_parts, const struct image_partition_entry *image_parts, size_t *len) {
	const struct flash_partition_entry *flash_os_image = &flash_parts[5];
	const struct flash_partition_entry *flash_support_list = &flash_parts[6];
	const struct flash_partition_entry *flash_file_system = &flash_parts[7];

	const struct image_partition_entry *image_os_image = &image_parts[2];
	const struct image_partition_entry *image_support_list = &image_parts[1];
	const struct image_partition_entry *image_file_system = &image_parts[3];

	assert(strcmp(flash_os_image->name, "os-image") == 0);
	assert(strcmp(flash_support_list->name, "support-list") == 0);
	assert(strcmp(flash_file_system->name, "file-system") == 0);

	assert(strcmp(image_os_image->name, "os-image") == 0);
	assert(strcmp(image_support_list->name, "support-list") == 0);
	assert(strcmp(image_file_system->name, "file-system") == 0);

	if (image_os_image->size > flash_os_image->size)
		error(1, 0, "kernel image too big (more than %u bytes)", (unsigned)flash_os_image->size);
	if (image_file_system->size > flash_file_system->size)
		error(1, 0, "rootfs image too big (more than %u bytes)", (unsigned)flash_file_system->size);

	*len = flash_file_system->base - flash_os_image->base + image_file_system->size;

	uint8_t *image = malloc(*len);
	if (!image)
		error(1, errno, "malloc");

	memset(image, 0xff, *len);

	memcpy(image, image_os_image->data, image_os_image->size);
	memcpy(image + flash_support_list->base - flash_os_image->base, image_support_list->data, image_support_list->size);
	memcpy(image + flash_file_system->base - flash_os_image->base, image_file_system->data, image_file_system->size);

	return image;
}


/** Generates an image for CPE210/220/510/520 and writes it to a file */
static void do_cpe510(const char *support_list,int size,const char *cfe_name, const char *output, const char *kernel_image, const char *rootfs_image, bool add_jffs2_eof, bool sysupgrade) {
	struct image_partition_entry parts[7] = {};

	
	parts[0] = make_partition_table_v1(cpe510_partitions);
	parts[1] = read_file("fs-uboot", cfe_name, false);
	parts[2] = read_file("os-image", kernel_image, false);
	parts[3] = read_file("file-system", rootfs_image, add_jffs2_eof);
	parts[4] = make_softversion(softversion, sizeof(softversion)-1);
	parts[5] = make_support_list(support_list, size - 1);
	size_t len;
	void *image;
	if (sysupgrade)
		image = generate_sysupgrade_image(cpe510_partitions, parts, &len);
	else
		image = generate_factory_image(cpe510_vendor, sizeof(cpe510_vendor)-1, parts, &len);

	FILE *file = fopen(output, "wb");
	if (!file)
		error(1, errno, "unable to open output file");

	if (fwrite(image, len, 1, file) != 1)
		error(1, 0, "unable to write output file");

	fclose(file);

	free(image);

	size_t i;
	for (i = 0; parts[i].name; i++)
		free_image_partition(parts[i]);
}

/** Generates an image for 646 and writes it to a file ONLY SUPPORT sysupgrade*/
static void do_cpe646(const char *support_list,int size,const char *cfe_name, const char *output, const char *kernel_image, const char *rootfs_image, bool add_jffs2_eof, bool sysupgrade) {
	struct image_partition_entry parts[7] = {};

	
	parts[0] = make_partition_table(cpe646_partitions); //we're skipping fs-uboot and softversion here so 4 parts in total
	// parts[1] = read_file("fs-uboot", cfe_name, false);
	parts[1] = read_file("os-image", kernel_image, false);
	parts[2] = read_file("file-system", rootfs_image, add_jffs2_eof);
	parts[3] = make_extra_para(extra_para, sizeof(extra_para)-1);
	parts[4] = make_support_list(support_list, size - 1);
	size_t len;
	void *image;
	if (sysupgrade)
		image = generate_sysupgrade_image(cpe646_partitions, parts, &len);
	else
		image = generate_factory_image(cpe646_vendor, sizeof(cpe646_vendor)-1, parts, &len);

	FILE *file = fopen(output, "wb");
	if (!file)
		error(1, errno, "unable to open output file");

	if (fwrite(image, len, 1, file) != 1)
		error(1, 0, "unable to write output file");

	fclose(file);

	free(image);

	size_t i;
	for (i = 0; parts[i].name; i++)
		free_image_partition(parts[i]);
}

/** Generates an image for 646 and writes it to a file ONLY SUPPORT sysupgrade*/
static void do_cpe646_osonly(const char *support_list,int size,const char *cfe_name, const char *output, const char *kernel_image, const char *rootfs_image, bool add_jffs2_eof, bool sysupgrade) {
	struct image_partition_entry parts[7] = {};

	
	parts[0] = make_partition_table(cpe646_partitions); //we're skipping fs-uboot and softversion here so 4 parts in total
	// parts[1] = read_file("fs-uboot", cfe_name, false);
	parts[1] = read_file("os-image", kernel_image, false);
	// parts[2] = read_file("file-system", rootfs_image, add_jffs2_eof);
	parts[2] = make_extra_para(extra_para, sizeof(extra_para)-1);
	parts[3] = make_support_list(support_list, size - 1);
	size_t len;
	void *image;
	if (sysupgrade)
		image = generate_sysupgrade_image(cpe646_partitions, parts, &len);
	else
		image = generate_factory_image(cpe646_vendor, sizeof(cpe646_vendor)-1, parts, &len);

	FILE *file = fopen(output, "wb");
	if (!file)
		error(1, errno, "unable to open output file");

	if (fwrite(image, len, 1, file) != 1)
		error(1, 0, "unable to write output file");

	fclose(file);

	free(image);

	size_t i;
	for (i = 0; parts[i].name; i++)
		free_image_partition(parts[i]);
}

/** Usage output */
void usage(const char *argv0) {
	fprintf(stderr,
		"Usage: %s [OPTIONS...]\n"
		"\n"
		"Options:\n"
		"  -B <board>      create image for the board specified with <board>\n"
		"  -k <file>       read kernel image from the file <file>\n"
		"  -r <file>       read rootfs image from the file <file>\n"
		"  -o <file>       write output to the file <file>\n"
		"  -j              add jffs2 end-of-filesystem markers\n"
		"  -S              create sysupgrade instead of factory image\n"
		"  -h              show this help\n",
		argv0
	);
};


int main(int argc, char *argv[]) {
	const char *board = NULL, *kernel_image = NULL, *rootfs_image = NULL, *output = NULL;
	bool add_jffs2_eof = false, sysupgrade = false;

	while (true) {
		int c;

		c = getopt(argc, argv, "B:k:r:o:jSh");
		if (c == -1)
			break;

		switch (c) {
		case 'B':
			board = optarg;
			break;

		case 'k':
			kernel_image = optarg;
			break;

		case 'r':
			rootfs_image = optarg;
			break;

		case 'o':
			output = optarg;
			break;

		case 'j':
			add_jffs2_eof = true;
			break;

		case 'S':
			sysupgrade = true;
			break;

		case 'h':
			usage(argv[0]);
			return 0;

		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (!board)
		error(1, 0, "no board has been specified");
	if (!kernel_image)
		error(1, 0, "no kernel image has been specified");
	if (!rootfs_image)
		error(1, 0, "no rootfs image has been specified");
	if (!output)
		error(1, 0, "no output filename has been specified");

	if (strcmp(board, "ARCHERC9") == 0)
		do_cpe510(archerc9_support_list,sizeof(archerc9_support_list), "archerc9_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else if (strcmp(board, "ARCHERC1900") == 0)
		do_cpe510(archerc1900_support_list,sizeof(archerc1900_support_list), "archerc9v2_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else if (strcmp(board, "ARCHERC9v2") == 0)
		do_cpe510(archerc9v2_support_list,sizeof(archerc9v2_support_list), "archerc9v2_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else if (strcmp(board, "ARCHERC8") == 0)
		do_cpe510(archerc8_support_list,sizeof(archerc8_support_list), "archerc8_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else if (strcmp(board, "ARCHERC8v2") == 0)
		do_cpe510(archerc8v2_support_list,sizeof(archerc8v2_support_list), "archerc8v2_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else if (strcmp(board, "ARCHERC8v3") == 0)
		do_cpe646(archerc8v3_support_list,sizeof(archerc8v3_support_list), "archerc8v3_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else if (strcmp(board, "ARCHERC8v3os") == 0)
		do_cpe646_osonly(archerc8v3_support_list,sizeof(archerc8v3_support_list), "archerc8v3_cfe.bin", output, kernel_image, rootfs_image, add_jffs2_eof, sysupgrade);
	else
		error(1, 0, "unsupported board %s", board);

	return 0;
}

