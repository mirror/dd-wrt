/*
 *  cb-dump.c - Dump control structures of the NAND
 *
 *  Copyright (c) 2008 by Embedded Alley Solution Inc.
 *  Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 *   Author: Pantelis Antoniou <pantelis@embeddedalley.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <asm/byteorder.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "mtd.h"

#include "config.h"
#include "version.h"
#include "bootstream.h"

void usage(void) __attribute__ ((noreturn));

void usage(void)
{

	printf("ROM Version %d\n", rom_version);

	printf(
	"usage: kobs-ng [COMMAND] [ARGS]\n"
	"Where [COMMAND] is one of:\n"
	"\n"
	"  dump [-v] [KOBS] ........................ Verify/dump boot structures\n"
	"    -v .................................... Verbose mode\n"
	"\n"
	"  imgverify [-v] [KEY] <file> ............. Verify image validity\n"
	"    -v .................................... Verbose mode\n"
	"\n"
	"  init [-v] [KEY] [KOBS] <file> ........... Initialize boot structure &\n"
	"                                            install bootstreams\n"
	"    -v .................................... Verbose mode\n"
	"    -n .................................... Dry run (don't commit to flash)\n"
	"    -w .................................... Commit to flash\n"
	"\n"
	"  update [-v] [KEY] [KOBS] [-0|1] <file> .. Update a single bootstream\n"
	"    -v .................................... Verbose mode\n"
	"    -0|1 .................................. Update specified bootstream #\n"
	"\n"
	"  extract [-v] [KEY] [KOBS] [-0|1] <file> . Extract a bootstream from flash\n"
	"    -v .................................... Verbose mode\n"
	"    -0|1 .................................. Extract specified bootstream #\n"
	"\n"
	"  [KOBS] boot structures config options\n"
	"    --chip_0_device_path=<path> .......... Device of boot (default /dev/mtd0)\n"
	"    --chip_1_device_path=<path> .......... The second chip in case of multichip NAND\n"
	"    --search_exponent=<value> ............ NCB field (default 2)\n"
	"    --data_setup_time=<value> ............ NCB field (default 80)\n"
	"    --data_hold_time=<value> ............. NCB field (default 60)\n"
	"    --address_setup_time=<value> ......... NCB field (default 25)\n"
	"    --data_sample_time=<value> ........... NCB field (default 6)\n"
	"    --row_address_size=<value> ........... NCB field (default 3)\n"
	"    --column_address_size=<value> ........ NCB field (default 2)\n"
	"    --read_command_code1=<value> ......... NCB field (default 0x00)\n"
	"    --read_command_code2=<value> ......... NCB field (default 0x30)\n"
	"    --boot_stream_major_version=<value> .. NCB field (default 1)\n"
	"    --boot_stream_minor_version=<value> .. NCB field (default 0)\n"
	"    --boot_stream_sub_version=<value> .... NCB field (default 0)\n"
	"    --ncb_version=<value> ................ NCB field (default 3)\n"
	"\n"
	"   [KEY] key management related options\n"
	"        -d ............................... Use device key (OTP) (not yet supported)\n"
	"        -z ............................... Use key of all zeroes (default)\n"
	"        -k<hexadecimalkey> ............... Use hex key of 16 bytes\n"
	"\n"
	);

	exit(5);
}

int dump_main(int argc, char **argv)
{
	int i, r;
	struct mtd_data *md;
	int flags;
	struct mtd_config cfg;

	/* copy defaults */
	memcpy(&cfg, &default_mtd_config, sizeof(cfg));

	flags = 0;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0)
			flags |= F_VERBOSE;
	}

	/* first parse kobs */
	mtd_parse_kobs(&cfg, ".kobs", flags & F_VERBOSE);

	/* then the arguments */
	mtd_parse_args(&cfg, argc, argv);

	if (flags & F_VERBOSE)
		mtd_cfg_dump(&cfg);

	md = mtd_open(&cfg, flags);
	if (md == NULL) {
		fprintf(stderr, "Unable to open mtd device(s)\n");
		usage();
	}

	if (flags & F_VERBOSE)
		mtd_dump(md);

	r = mtd_load_all_boot_structures(md);
	if (r != 0) {
		fprintf(stderr, "Unable to load boot structures\n");
		exit(5);
	}

	if (flags & F_VERBOSE)
		mtd_dump_structure(md);

	mtd_close(md);

	return 0;
}

int extract_main(int argc, char **argv)
{
	int i, j, r;
	struct mtd_data *md;
	int flags, image;
	int startpage, size;
	char *outfile = NULL;
	FILE *outfp;
	loff_t start;
	char buf[512];
	struct mtd_config cfg;
	uint8_t key[16];
	long end_of_file, pos;
	char ascii[20 * 2 + 1];
	FILE *tfp;
	int readn, chunk, curr;
	int device_key;
	uint8_t *keyp;
	int chip;

	memset(key, 0, sizeof(key));
	device_key = 0;

	/* copy defaults */
	memcpy(&cfg, &default_mtd_config, sizeof(cfg));

	/* first parse kobs */
	mtd_parse_kobs(&cfg, ".kobs", false);

	/* then the arguments */
	mtd_parse_args(&cfg, argc, argv);

	image = 0;	/* first image */
	flags = 0;
	j = 0;
	for (i = 1; i < argc; i++) {

		if (argv[i][0] != '-') {
			if (outfile == NULL)
				outfile = argv[i];
			continue;
		}

		switch (argv[i][1]) {
			case '0':
				image = 0;
				break;
			case '1':
				image = 1;
				break;
			case 'd':
				device_key = 1;	/* use device key */
				break;
			case 'z':
				memset(key, 0, sizeof(key));
				break;
			case 'k':
				if (ascii_vec(&argv[i][2], key) == NULL) {
					fprintf(stderr, "Illegal key '%s'\n", &argv[i][2]);
					exit(5);
				}
				break;
			case 'v':
				flags |= F_VERBOSE;
				break;
		}
	}

	if (outfile == NULL)
		usage();

	if (flags & F_VERBOSE)
		mtd_cfg_dump(&cfg);

	outfp = fopen(outfile, "w+");
	if (outfp == NULL) {
		fprintf(stderr, "Unable to open output file '%s'\n", outfile);
		usage();
	}

	md = mtd_open(&cfg, flags);
	if (md == NULL) {
		fprintf(stderr, "Unable to open mtd device(s)\n");
		usage();
	}

	if (flags & F_VERBOSE)
		mtd_dump(md);

	r = mtd_load_all_boot_structures(md);
	if (r != 0) {
		fprintf(stderr, "Unable to load boot structures\n");
		exit(5);
	}

	chip = 0;
	startpage = image == 0 ?
		md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector :
		md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2;
	size =  image == 0 ?
		md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware :
		md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware2 ;
	if (md->flags & F_MULTICHIP)
		chip = image;

	if (flags & F_VERBOSE)
		printf("startpage=%u, size=%u\n", startpage, size);

	start = startpage * 2048;
	size = size * 2048;
	while (size > 0) {

		/* skip bad blocks */
		if (mtd_isbad(md, chip, start)) {
			start = (start / mtd_erasesize(md)) * mtd_erasesize(md);
			start += mtd_erasesize(md);
			continue;
		}

		r = mtd_read(md, 0, buf, sizeof(buf), start);	// FIXME: chip
		if (r != sizeof(buf)) {
			fprintf(stderr, "Unable to read\n");
			exit(5);
		}
		fwrite(buf, sizeof(buf), 1, outfp);

		start += sizeof(buf);
		size -= sizeof(buf);
	}

	mtd_close(md);

	keyp = !device_key ? key : NULL;

	if (flags & F_VERBOSE)
		printf("%s: verifying using key '%s'\n", outfile,
			vec_ascii(keyp, ascii));

	r = bootstream_verify(flags, outfp, keyp, &end_of_file);
	if (r != 0) {
		fprintf(stderr, "ERROR: bootstream '%s' is invalid\n", outfile);
		exit(5);
	}

	if (flags & F_VERBOSE)
		printf("%s: is %s bootstream for key '%s'\n", outfile,
			r == 0 ? "a valid" : "an INVALID",
			vec_ascii(keyp, ascii));

	/* now truncate to actual size (which is only possible to find after
	 * decoding the whole bootstream; what a drag...
	 */
	fseek(outfp, 0, SEEK_END);
	pos = ftell(outfp);
	if (pos > end_of_file) {

		tfp = tmpfile();
		if (tfp == NULL) {
			fprintf(stderr, "ERROR: unable to open tmp file\n");
			exit(5);
		}

		rewind(outfp);

		curr = 0;
		while (curr < end_of_file) {
			if (curr + sizeof(buf) > end_of_file)
				chunk = end_of_file - curr;
			else
				chunk = sizeof(buf);

			readn = fread(buf, 1, chunk, outfp);
			if (readn != chunk) {
				fprintf(stderr, "ERROR: unable to read tmp file\n");
				exit(5);
			}
			fwrite(buf, 1, chunk, tfp);
			curr += chunk;
		}

		fclose(outfp);

		outfp = fopen(outfile, "wb");
		if (outfp == NULL) {
			fprintf(stderr, "ERROR: unable to open file %s\n", outfile);
			exit(5);
		}

		rewind(tfp);
		curr = 0;
		while (curr < end_of_file) {
			if (curr + sizeof(buf) > end_of_file)
				chunk = end_of_file - curr;
			else
				chunk = sizeof(buf);

			readn = fread(buf, 1, chunk, tfp);
			if (readn != chunk) {
				fprintf(stderr, "ERROR: unable to read tmp file\n");
				exit(5);
			}
			fwrite(buf, 1, chunk, outfp);
			curr += chunk;
		}

		fclose(tfp);

	}

	fclose(outfp);

	return 0;
}

static int perform_bootstream_update(struct mtd_data *md, FILE *infp, int image_mask)
{
	int i, r;
	unsigned int size, start, avail, end, update;

	r = mtd_load_all_boot_structures(md);
	if (r != 0) {
		fprintf(stderr, "Unable to load boot structures\n");
		return -1;
	}

	fseek(infp, 0, SEEK_END);
	size = ftell(infp);
	rewind(infp);

	update = 0;
	for (i = 0; i < 2; i++) {

		/* only affected bootstream */
		if ((image_mask & (1 << i)) == 0)
			continue;

		/* first verify it fits */
		if (i == 0) {
			start = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector  * 2048;
			end = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2 * 2048;
		} else {
			start = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2 * 2048;
			end = mtd_size(md);
		}
		avail = end - start;

		if (avail <= size) {
			fprintf(stderr, "image #d does not fit (avail = %u, size = %u)\n", avail, size);
			exit(5);
		}

		/* now update size */
		if (i == 0)
			md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware = (size + 2047) / 2048;
		else
			md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware2 = (size + 2047) / 2048;
		update |= UPDATE_BS(i);
	}

	r = v0_rom_mtd_commit_structures(md, infp, UPDATE_LDLB | update);
	if (r < 0) {
		fprintf(stderr, "FAILED to commit structures\n");
		return -1;
	}

	return 0;
}

int update_main(int argc, char **argv)
{
	int i, j, r;
	struct mtd_data *md;
	int flags;
	unsigned int image_mask;
	char *infile = NULL;
	FILE *infp;
	struct mtd_config cfg;
	uint8_t key[16];
	char ascii[20 * 2 + 1];
	int device_key;
	uint8_t *keyp;

	memset(key, 0, sizeof(key));
	device_key = 0;

	/* copy defaults */
	memcpy(&cfg, &default_mtd_config, sizeof(cfg));

	/* first parse kobs */
	mtd_parse_kobs(&cfg, ".kobs", false);

	/* then the arguments */
	mtd_parse_args(&cfg, argc, argv);

	image_mask = 0;	/* no image */
	flags = 0;
	j = 0;
	for (i = 1; i < argc; i++) {

		if (argv[i][0] != '-') {
			if (infile == NULL)
				infile = argv[i];
			continue;
		}

		switch (argv[i][1]) {
			case '0':
				image_mask |= 1 << 0;
				break;
			case '1':
				image_mask |= 1 << 1;
				break;
			case 'd':
				device_key = 1;	/* use device key */
				break;
			case 'z':
				memset(key, 0, sizeof(key));
				break;
			case 'k':
				if (ascii_vec(&argv[i][2], key) == NULL) {
					fprintf(stderr, "Illegal key '%s'\n", &argv[i][2]);
					exit(5);
				}
				break;
			case 'v':
				flags |= F_VERBOSE;
				break;
		}
	}

	/* we must select one image */
	if (image_mask == 0 || infile == NULL)
		usage();

	if (flags & F_VERBOSE)
		mtd_cfg_dump(&cfg);

	infp = fopen(infile, "rb");
	if (infp == NULL) {
		fprintf(stderr, "Unable to open output file '%s'\n", infile);
		usage();
	}

	keyp = !device_key ? key : NULL;

	if (flags & F_VERBOSE)
		printf("%s: verifying using key '%s'\n", infile,
			vec_ascii(keyp, ascii));

	r = bootstream_verify(flags, infp, keyp, NULL);
	if (r != 0) {
		fprintf(stderr, "ERROR: bootstream '%s' is invalid\n", infile);
		usage();
	}

	if (flags & F_VERBOSE)
		printf("%s: is %s bootstream for key '%s'\n", infile,
			r == 0 ? "a valid" : "an INVALID",
			vec_ascii(keyp, ascii));

	md = mtd_open(&cfg, flags);
	if (md == NULL) {
		fprintf(stderr, "Unable to open mtd device(s)\n");
		usage();
	}

	if (flags & F_VERBOSE)
		mtd_dump(md);

	r = perform_bootstream_update(md, infp, image_mask);
	if (r != 0) {
		fprintf(stderr, "Unable to perform bootstream update\n");
		usage();
	}

	mtd_close(md);

	fclose(infp);

	return 0;
}

int init_main(int argc, char **argv)
{
	int i, j, r;
	struct mtd_data *md;
	int flags, image;
	char *infile = NULL;
	char *badlist = NULL;
	FILE *infp;
	loff_t ofs;
	int dryrun;
	struct mtd_config cfg;
	uint8_t key[16];
	char ascii[20 * 2 + 1];
	int device_key;
	uint8_t *keyp;

	memset(key, 0, sizeof(key));
	device_key = 0;

	/* copy defaults */
	memcpy(&cfg, &default_mtd_config, sizeof(cfg));

	/* first parse kobs */
	mtd_parse_kobs(&cfg, ".kobs", false);

	/* then the arguments */
	mtd_parse_args(&cfg, argc, argv);

	image = 0;	/* first image */
	flags = 0;
	dryrun = 0;
	j = 0;
	for (i = 1; i < argc; i++) {

		if (argv[i][0] != '-') {
			if (infile == NULL)
				infile = argv[i];
			continue;
		}

		switch (argv[i][1]) {
			case 'b':
				badlist = &argv[i][2];
				if (*badlist == '\0') {
					if (++i >= argc)
						usage();
					badlist = &argv[i][0];
				}
				break;
			case 'w':
				dryrun = 0;
				break;
			case 'n':
				dryrun = 1;
				break;
			case 'd':
				device_key = 1;	/* use device key */
				break;
			case 'z':
				memset(key, 0, sizeof(key));
				break;
			case 'k':
				if (ascii_vec(&argv[i][2], key) == NULL) {
					fprintf(stderr, "Illegal key '%s'\n", &argv[i][2]);
					exit(5);
				}
				break;
			case 'v':
				flags |= F_VERBOSE;
				break;
		}
	}

	if (flags & F_VERBOSE)
		mtd_cfg_dump(&cfg);

	infp = fopen(infile, "rb");
	if (infp == NULL) {
		fprintf(stderr, "Unable to open input file '%s'\n", infile);
		usage();
	}

	keyp = !device_key ? key : NULL;

	if (flags & F_VERBOSE)
		printf("%s: verifying using key '%s'\n", infile,
			vec_ascii(keyp, ascii));

	r = bootstream_verify(flags, infp, keyp, NULL);
	if (r != 0) {
		fprintf(stderr, "ERROR: bootstream '%s' is invalid\n", infile);
		usage();
	}

	if (flags & F_VERBOSE)
		printf("%s: is %s bootstream for key '%s'\n", infile,
			r == 0 ? "a valid" : "an INVALID",
			vec_ascii(keyp, ascii));

	md = mtd_open(&cfg, flags);
	if (md == NULL) {
		fprintf(stderr, "Unable to open mtd device(s)\n");
		usage();
	}

	while (badlist != NULL && *badlist) {
		ofs = strtoll(badlist, NULL, 0);
		ofs = (ofs / mtd_erasesize(md)) * mtd_erasesize(md);

		if (mtd_markbad(md, 0, ofs) < 0) {
			fprintf(stderr, "Unable to mark bad 0x%llx\n", ofs);
			usage();
		}
		if (flags & F_VERBOSE)
			printf("marked bad : 0x%08llx\n", ofs);

		badlist = strchr(badlist, ',');
		if (badlist)
			badlist++;
	}

	switch (rom_version) {
		case ROM_Version_0:
			r = v0_rom_mtd_init(md, infp);
			break;
		case ROM_Version_1:
			r = v1_rom_mtd_init(md, infp);
			break;
		case ROM_Version_2:
			r = v2_rom_mtd_init(md, infp);
			break;
		default:
			fprintf(stderr, "Unknown ROM version: %d\n", rom_version);
			break;
	}
	if (r < 0) {
		fprintf(stderr, "mtd_init failed!\n");
		exit(5);
	}

	if (flags & F_VERBOSE)
		mtd_dump_structure(md);

	if (!dryrun) {
		switch (rom_version) {
			case ROM_Version_0:
				r = v0_rom_mtd_commit_structures(md, infp, UPDATE_ALL);
				break;
			case ROM_Version_1:
				r = v1_rom_mtd_commit_structures(md, infp, UPDATE_ALL);
				break;
			case ROM_Version_2:
				r = v2_rom_mtd_commit_structures(md, infp, UPDATE_ALL);
				break;
			default:
				fprintf(stderr, "Unknown ROM version: %d\n", rom_version);
				break;
		}
		if (r < 0) {
			fprintf(stderr, "FAILED to commit structures\n");
			exit(5);
		}
	} else
		fprintf(stderr, "Dry run - not committed!\n");

	fclose(infp);

	return 0;
}

int imgverify_main(int argc, char **argv)
{
	int i, j;
	int flags;
	char *infile = NULL;
	FILE *infp;
	uint8_t key[16];
	long end_of_file;
	char ascii[20 * 2 + 1];
	int r;
	int device_key;
	uint8_t *keyp;

	/* default key is zero */
	memset(key, 0, sizeof(key));
	device_key = 0;

	flags = 0;
	j = 0;
	for (i = 1; i < argc; i++) {

		if (argv[i][0] != '-') {
			if (infile == NULL)
				infile = argv[i];
			continue;
		}

		switch (argv[i][1]) {
			case 'd':
				device_key = 1;	/* use device key */
				break;
			case 'z':
				memset(key, 0, sizeof(key));
				break;
			case 'k':
				if (ascii_vec(&argv[i][2], key) == NULL) {
					fprintf(stderr, "Illegal key '%s'\n", &argv[i][2]);
					exit(5);
				}
				break;
			case 'v':
				flags |= F_VERBOSE;
				break;
		}
	}

	infp = fopen(infile, "rb");
	if (infp == NULL) {
		fprintf(stderr, "Unable to open output file '%s'\n", infile);
		usage();
	}

	keyp = !device_key ? key : NULL;

	if (flags & F_VERBOSE)
		printf("%s: verifying using key '%s'\n", infile,
			vec_ascii(keyp, ascii));

	r = bootstream_verify(flags, infp, keyp, &end_of_file);
	if (r != 0) {
		fprintf(stderr, "ERROR: bootstream '%s' is invalid\n", infile);
		goto out;
	}

	if (flags & F_VERBOSE)
		printf("%s: is %s bootstream for key '%s'\n", infile,
			r == 0 ? "a valid" : "an INVALID",
			vec_ascii(keyp, ascii));

out:
	fclose(infp);

	return r;
}

void discover_boot_rom_version(void)
{
	FILE         *cpuinfo;
	char         line_buffer[100];
	static char  *banner     = "Hardware\t: Freescale MX";
	static char  *v0_rom_tag = "23";
	static char  *v1_rom_tag = "28";
	static char  *v2_rom_tag = "53";

	//----------------------------------------------------------------------
	// Attempt to open /proc/cpuinfo.
	//----------------------------------------------------------------------

	cpuinfo = fopen("/proc/cpuinfo", "r");

	if (!cpuinfo) {
		fprintf(stderr, "Can't open /proc/cpuinfo to discover Boot ROM version.\n");
		exit(1);
	}

	//----------------------------------------------------------------------
	// Loop over lines from /proc/cpuinfo.
	//----------------------------------------------------------------------

	for (;;) {

		//--------------------------------------------------------------
		// Attempt to get the current line.
		//--------------------------------------------------------------

		if (!fgets(line_buffer, sizeof(line_buffer), cpuinfo)) {
			break;
		}

		//--------------------------------------------------------------
		// Check if the current line contains the information we need.
		//--------------------------------------------------------------

		if (strncmp(line_buffer, banner, strlen(banner))) {
			continue;
		}

		//--------------------------------------------------------------
		// If control arrives here, we found what we're looking for.
		// Extract the information we need.
		//--------------------------------------------------------------

		if (!strncmp(line_buffer + strlen(banner), v0_rom_tag, strlen(v0_rom_tag))) {
			rom_version = ROM_Version_0;
			break;
		}
		else
		if (!strncmp(line_buffer + strlen(banner), v1_rom_tag, strlen(v1_rom_tag))) {
			rom_version = ROM_Version_1;
			break;
		}
		else
		if (!strncmp(line_buffer + strlen(banner), v2_rom_tag, strlen(v2_rom_tag))) {
			rom_version = ROM_Version_2;
			break;
		}

	}

	//----------------------------------------------------------------------
	// Close /proc/cpuinfo.
	//----------------------------------------------------------------------

	fclose(cpuinfo);

	//----------------------------------------------------------------------
	// Check if we found what we needed.
	//----------------------------------------------------------------------

	if (rom_version == ROM_Version_Unknown) {
		fprintf(stderr, "Couldn't discover Boot ROM version\n");
		exit(1);
	}

}

int main(int argc, char **argv)
{

	discover_boot_rom_version();

	if (argc < 2)
		usage();

	argc--;
	argv++;

	if (strcmp(argv[0], "dump") == 0)
		return dump_main(argc, argv);

	if (strcmp(argv[0], "extract") == 0)
		return extract_main(argc, argv);

	if (strcmp(argv[0], "init") == 0)
		return init_main(argc, argv);

	if (strcmp(argv[0], "update") == 0)
		return update_main(argc, argv);

	if (strcmp(argv[0], "imgverify") == 0)
		return imgverify_main(argc, argv);

	/* no arguments == init */
	return init_main(argc + 1, argv - 1);
}

