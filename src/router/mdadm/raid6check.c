/*
 * raid6check - extended consistency check for RAID-6
 *
 * Copyright (C) 2011 Piergiorgio Sartor
 *
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    Author: Piergiorgio Sartor
 *    Based on "restripe.c" from "mdadm" codebase
 */

#include "mdadm.h"
#include <stdint.h>
#include <signal.h>
#include <sys/mman.h>

#define CHECK_PAGE_BITS (12)
#define CHECK_PAGE_SIZE (1 << CHECK_PAGE_BITS)

char const Name[] = "raid6check";

enum repair {
	NO_REPAIR = 0,
	MANUAL_REPAIR,
	AUTO_REPAIR
};

int geo_map(int block, unsigned long long stripe, int raid_disks,
	    int level, int layout);
int is_ddf(int layout);
void qsyndrome(uint8_t *p, uint8_t *q, uint8_t **sources, int disks, int size);
void make_tables(void);
void ensure_zero_has_size(int chunk_size);
void raid6_datap_recov(int disks, size_t bytes, int faila, uint8_t **ptrs,
		       int neg_offset);
void raid6_2data_recov(int disks, size_t bytes, int faila, int failb,
		       uint8_t **ptrs, int neg_offset);
void xor_blocks(char *target, char **sources, int disks, int size);

/* Collect per stripe consistency information */
void raid6_collect(int chunk_size, uint8_t *p, uint8_t *q,
		   char *chunkP, char *chunkQ, int *results)
{
	int i;
	int data_id;
	uint8_t Px, Qx;
	extern uint8_t raid6_gflog[];

	for(i = 0; i < chunk_size; i++) {
		Px = (uint8_t)chunkP[i] ^ (uint8_t)p[i];
		Qx = (uint8_t)chunkQ[i] ^ (uint8_t)q[i];

		if((Px != 0) && (Qx == 0))
			results[i] = -1;

		if((Px == 0) && (Qx != 0))
			results[i] = -2;

		if((Px != 0) && (Qx != 0)) {
			data_id = (raid6_gflog[Qx] - raid6_gflog[Px]);
			if(data_id < 0) data_id += 255;
			results[i] = data_id;
		}

		if((Px == 0) && (Qx == 0))
			results[i] = -255;
	}
}

/* Try to find out if a specific disk has problems in a CHECK_PAGE_SIZE page size */
int raid6_stats_blk(int *results, int raid_disks)
{
	int i;
	int curr_broken_disk = -255;
	int prev_broken_disk = -255;
	int broken_status = 0;

	for(i = 0; i < CHECK_PAGE_SIZE; i++) {

		if(results[i] != -255)
			curr_broken_disk = results[i];

		if(curr_broken_disk >= raid_disks)
			broken_status = 2;

		switch(broken_status) {
		case 0:
			if(curr_broken_disk != -255) {
				prev_broken_disk = curr_broken_disk;
				broken_status = 1;
			}
			break;

		case 1:
			if(curr_broken_disk != prev_broken_disk)
				broken_status = 2;
			break;

		case 2:
		default:
			curr_broken_disk = prev_broken_disk = -65535;
			break;
		}
	}

	return curr_broken_disk;
}

/* Collect disks status for a strip in CHECK_PAGE_SIZE page size blocks */
void raid6_stats(int *disk, int *results, int raid_disks, int chunk_size)
{
	int i, j;

	for(i = 0, j = 0; i < chunk_size; i += CHECK_PAGE_SIZE, j++) {
		disk[j] = raid6_stats_blk(&results[i], raid_disks);
	}
}

int lock_stripe(struct mdinfo *info, unsigned long long start,
		int chunk_size, int data_disks, sighandler_t *sig) {
	int rv;
	if(mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
		return 2;
	}

	sig[0] = signal(SIGTERM, SIG_IGN);
	sig[1] = signal(SIGINT, SIG_IGN);
	sig[2] = signal(SIGQUIT, SIG_IGN);

	rv = sysfs_set_num(info, NULL, "suspend_lo", start * chunk_size * data_disks);
	rv |= sysfs_set_num(info, NULL, "suspend_hi", (start + 1) * chunk_size * data_disks);
	return rv * 256;
}

int unlock_all_stripes(struct mdinfo *info, sighandler_t *sig) {
	int rv;
	rv = sysfs_set_num(info, NULL, "suspend_lo", 0x7FFFFFFFFFFFFFFFULL);
	rv |= sysfs_set_num(info, NULL, "suspend_hi", 0);
	rv |= sysfs_set_num(info, NULL, "suspend_lo", 0);

	signal(SIGQUIT, sig[2]);
	signal(SIGINT, sig[1]);
	signal(SIGTERM, sig[0]);

	if(munlockall() != 0)
		return 3;
	return rv * 256;
}

/* Autorepair */
int autorepair(int *disk, unsigned long long start, int chunk_size,
		char *name[], int raid_disks, int syndrome_disks, char **blocks_page,
		char **blocks, uint8_t *p, int *block_index_for_slot,
		int *source, unsigned long long *offsets)
{
	int i, j;
	int pages_to_write_count = 0;
	int page_to_write[chunk_size >> CHECK_PAGE_BITS];
	for(j = 0; j < (chunk_size >> CHECK_PAGE_BITS); j++) {
		if (disk[j] >= -2 && block_index_for_slot[disk[j]] >= 0) {
			int slot = block_index_for_slot[disk[j]];
			printf("Auto-repairing slot %d (%s)\n", slot, name[slot]);
			pages_to_write_count++;
			page_to_write[j] = 1;
			for(i = -2; i < syndrome_disks; i++) {
				blocks_page[i] = blocks[i] + j * CHECK_PAGE_SIZE;
			}
			if (disk[j] == -2) {
				qsyndrome(p, (uint8_t*)blocks_page[-2],
					  (uint8_t**)blocks_page,
					  syndrome_disks, CHECK_PAGE_SIZE);
			}
			else {
				char *all_but_failed_blocks[syndrome_disks];
				for(i = 0; i < syndrome_disks; i++) {
					if (i == disk[j])
						all_but_failed_blocks[i] = blocks_page[-1];
					else
						all_but_failed_blocks[i] = blocks_page[i];
				}
				xor_blocks(blocks_page[disk[j]],
					   all_but_failed_blocks, syndrome_disks,
					   CHECK_PAGE_SIZE);
			}
		}
		else {
			page_to_write[j] = 0;
		}
	}

	if(pages_to_write_count > 0) {
		int write_res = 0;
		for(j = 0; j < (chunk_size >> CHECK_PAGE_BITS); j++) {
			if(page_to_write[j] == 1) {
				int slot = block_index_for_slot[disk[j]];
				lseek64(source[slot], offsets[slot] + start * chunk_size + j * CHECK_PAGE_SIZE, SEEK_SET);
				write_res += write(source[slot],
						   blocks[disk[j]] + j * CHECK_PAGE_SIZE,
						   CHECK_PAGE_SIZE);
			}
		}

		if (write_res != (CHECK_PAGE_SIZE * pages_to_write_count)) {
			fprintf(stderr, "Failed to write a full chunk.\n");
			return -1;
		}
	}

	return 0;
}

/* Manual repair */
int manual_repair(int chunk_size, int syndrome_disks,
		  int failed_slot1, int failed_slot2,
		  unsigned long long start, int *block_index_for_slot,
		  char *name[], char **stripes, char **blocks, uint8_t *p,
		  int *source, unsigned long long *offsets)
{
	int i;
	int fd1 = block_index_for_slot[failed_slot1];
	int fd2 = block_index_for_slot[failed_slot2];
	printf("Repairing stripe %llu\n", start);
	printf("Assuming slots %d (%s) and %d (%s) are incorrect\n",
	       fd1, name[fd1],
	       fd2, name[fd2]);

	if (failed_slot1 == -2 || failed_slot2 == -2) {
		char *all_but_failed_blocks[syndrome_disks];
		int failed_data_or_p;

		if (failed_slot1 == -2)
			failed_data_or_p = failed_slot2;
		else
			failed_data_or_p = failed_slot1;

		printf("Repairing D/P(%d) and Q\n", failed_data_or_p);

		for (i = 0; i < syndrome_disks; i++) {
			if (i == failed_data_or_p)
				all_but_failed_blocks[i] = blocks[-1];
			else
				all_but_failed_blocks[i] = blocks[i];
		}
		xor_blocks(blocks[failed_data_or_p],
			   all_but_failed_blocks, syndrome_disks, chunk_size);
		qsyndrome(p, (uint8_t*)blocks[-2], (uint8_t**)blocks,
			  syndrome_disks, chunk_size);
	} else {
		ensure_zero_has_size(chunk_size);
		if (failed_slot1 == -1 || failed_slot2 == -1) {
			int failed_data;
			if (failed_slot1 == -1)
				failed_data = failed_slot2;
			else
				failed_data = failed_slot1;

			printf("Repairing D(%d) and P\n", failed_data);
			raid6_datap_recov(syndrome_disks+2, chunk_size,
					  failed_data, (uint8_t**)blocks, 1);
		} else {
			printf("Repairing D and D\n");
			raid6_2data_recov(syndrome_disks+2, chunk_size,
					  failed_slot1, failed_slot2,
					  (uint8_t**)blocks, 1);
		}
	}

	int write_res1, write_res2;
	off64_t seek_res;

	seek_res = lseek64(source[fd1],
			   offsets[fd1] + start * chunk_size, SEEK_SET);
	if (seek_res < 0) {
		fprintf(stderr, "lseek failed for failed_disk1\n");
		return -1;
	}
	write_res1 = write(source[fd1], blocks[failed_slot1], chunk_size);

	seek_res = lseek64(source[fd2],
			   offsets[fd2] + start * chunk_size, SEEK_SET);
	if (seek_res < 0) {
		fprintf(stderr, "lseek failed for failed_disk2\n");
		return -1;
	}
	write_res2 = write(source[fd2], blocks[failed_slot2], chunk_size);

	if (write_res1 != chunk_size || write_res2 != chunk_size) {
		fprintf(stderr, "Failed to write a complete chunk.\n");
		return -2;
	}

	return 0;
}

int check_stripes(struct mdinfo *info, int *source, unsigned long long *offsets,
		  int raid_disks, int chunk_size, int level, int layout,
		  unsigned long long start, unsigned long long length, char *name[],
		  enum repair repair, int failed_disk1, int failed_disk2)
{
	/* read the data and p and q blocks, and check we got them right */
	int data_disks = raid_disks - 2;
	int syndrome_disks = data_disks + is_ddf(layout) * 2;
	char *stripe_buf;

	/* stripes[] is indexed by raid_disk and holds chunks from each device */
	char **stripes = xmalloc(raid_disks * sizeof(char*));

	/* blocks[] is indexed by syndrome number and points to either one of the
	 * chunks from 'stripes[]', or to a chunk of zeros. -1 and -2 are
	 * P and Q */
	char **blocks = xmalloc((syndrome_disks + 2) * sizeof(char*));

	/* blocks_page[] is a temporary index to just one page of the chunks
	 * that blocks[] points to. */
	char **blocks_page = xmalloc((syndrome_disks + 2) * sizeof(char*));

	/* block_index_for_slot[] provides the reverse mapping from blocks to stripes.
	 * The index is a syndrome position, the content is a raid_disk number.
	 * indicies -1 and -2 work, and are P and Q disks */
	int *block_index_for_slot = xmalloc((syndrome_disks+2) * sizeof(int));

	/* 'p' and 'q' contain calcualted P and Q, to be compared with
	 * blocks[-1] and blocks[-2];
	 */
	uint8_t *p = xmalloc(chunk_size);
	uint8_t *q = xmalloc(chunk_size);
	char *zero = xmalloc(chunk_size);
	int *results = xmalloc(chunk_size * sizeof(int));
	sighandler_t *sig = xmalloc(3 * sizeof(sighandler_t));

	int i, j;
	int diskP, diskQ, diskD;
	int err = 0;

	extern int tables_ready;

	if (!tables_ready)
		make_tables();

	if (posix_memalign((void**)&stripe_buf, 4096, raid_disks * chunk_size) != 0)
		exit(4);
	block_index_for_slot += 2;
	blocks += 2;
	blocks_page += 2;

	memset(zero, 0, chunk_size);
	for ( i = 0 ; i < raid_disks ; i++)
		stripes[i] = stripe_buf + i * chunk_size;

	while (length > 0) {
		/* The syndrome number of the broken disk is recorded
		 * in 'disk[]' which allows a different broken disk for
		 * each page.
		 */
		int disk[chunk_size >> CHECK_PAGE_BITS];

		err = lock_stripe(info, start, chunk_size, data_disks, sig);
		if(err != 0) {
			if (err != 2)
				unlock_all_stripes(info, sig);
			goto exitCheck;
		}
		for (i = 0 ; i < raid_disks ; i++) {
			off64_t seek_res = lseek64(source[i], offsets[i] + start * chunk_size,
						   SEEK_SET);
			if (seek_res < 0) {
				fprintf(stderr, "lseek to source %d failed\n", i);
				unlock_all_stripes(info, sig);
				err = -1;
				goto exitCheck;
			}
			int read_res = read(source[i], stripes[i], chunk_size);
			if (read_res < chunk_size) {
				fprintf(stderr, "Failed to read complete chunk disk %d, aborting\n", i);
				unlock_all_stripes(info, sig);
				err = -1;
				goto exitCheck;
			}
		}

		diskP = geo_map(-1, start, raid_disks, level, layout);
		block_index_for_slot[-1] = diskP;
		blocks[-1] = stripes[diskP];

		diskQ = geo_map(-2, start, raid_disks, level, layout);
		block_index_for_slot[-2] = diskQ;
		blocks[-2] = stripes[diskQ];

		if (!is_ddf(layout)) {
			/* The syndrome-order of disks starts immediately after 'Q',
			 * but skips P */
			diskD = diskQ;
			for (i = 0 ; i < data_disks ; i++) {
				diskD = diskD + 1;
				if (diskD >= raid_disks)
					diskD = 0;
				if (diskD == diskP)
					diskD += 1;
				if (diskD >= raid_disks)
					diskD = 0;
				blocks[i] = stripes[diskD];
				block_index_for_slot[i] = diskD;
			}
		} else {
			/* The syndrome-order exactly follows raid-disk
			 * numbers, with ZERO in place of P and Q
			 */
			for (i = 0 ; i < raid_disks; i++) {
				if (i == diskP || i == diskQ) {
					blocks[i] = zero;
					block_index_for_slot[i] = -1;
				} else {
					blocks[i] = stripes[i];
					block_index_for_slot[i] = i;
				}
			}
		}

		qsyndrome(p, q, (uint8_t**)blocks, syndrome_disks, chunk_size);

		raid6_collect(chunk_size, p, q, stripes[diskP], stripes[diskQ], results);
		raid6_stats(disk, results, raid_disks, chunk_size);

		for(j = 0; j < (chunk_size >> CHECK_PAGE_BITS); j++) {
			int role = disk[j];
			if (role >= -2) {
				int slot = block_index_for_slot[role];
				if (slot >= 0)
					printf("Error detected at stripe %llu, page %d: possible failed disk slot %d: %d --> %s\n",
					       start, j, role, slot, name[slot]);
				else
					printf("Error detected at stripe %llu, page %d: failed slot %d should be zeros\n",
					       start, j, role);
			} else if(disk[j] == -65535) {
				printf("Error detected at stripe %llu, page %d: disk slot unknown\n", start, j);
			}
		}

		if(repair == AUTO_REPAIR) {
			err = autorepair(disk, start, chunk_size,
					name, raid_disks, syndrome_disks, blocks_page,
					blocks, p, block_index_for_slot,
					source, offsets);
			if(err != 0) {
				unlock_all_stripes(info, sig);
				goto exitCheck;
			}
		}

		if(repair == MANUAL_REPAIR) {
			int failed_slot1 = -1, failed_slot2 = -1;
			for (i = -2; i < syndrome_disks; i++) {
				if (block_index_for_slot[i] == failed_disk1)
					failed_slot1 = i;
				if (block_index_for_slot[i] == failed_disk2)
					failed_slot2 = i;
			}
			err = manual_repair(chunk_size, syndrome_disks,
					    failed_slot1, failed_slot2,
					    start, block_index_for_slot,
					    name, stripes, blocks, p,
					    source, offsets);
			if(err == -1) {
				unlock_all_stripes(info, sig);
				goto exitCheck;
			}
		}

		err = unlock_all_stripes(info, sig);
		if(err != 0) {
			goto exitCheck;
		}

		length--;
		start++;
	}

exitCheck:

	free(stripe_buf);
	free(stripes);
	free(blocks-2);
	free(blocks_page-2);
	free(block_index_for_slot-2);
	free(p);
	free(q);
	free(results);
	free(sig);

	return err;
}

unsigned long long getnum(char *str, char **err)
{
	char *e;
	unsigned long long rv = strtoull(str, &e, 10);
	if (e==str || *e) {
		*err = str;
		return 0;
	}
	return rv;
}

int main(int argc, char *argv[])
{
	/* md_device start length */
	int *fds = NULL;
	char *buf = NULL;
	char **disk_name = NULL;
	unsigned long long *offsets = NULL;
	int raid_disks = 0;
	int active_disks;
	int chunk_size = 0;
	int layout = -1;
	int level = 6;
	enum repair repair = NO_REPAIR;
	int failed_disk1 = -1;
	int failed_disk2 = -1;
	unsigned long long start, length;
	int i;
	int mdfd;
	struct mdinfo *info = NULL, *comp = NULL;
	char *err = NULL;
	int exit_err = 0;
	int close_flag = 0;
	char *prg = strrchr(argv[0], '/');

	if (prg == NULL)
		prg = argv[0];
	else
		prg++;

	if (argc < 4) {
		fprintf(stderr, "Usage: %s md_device start_stripe length_stripes [autorepair]\n", prg);
		fprintf(stderr, "   or: %s md_device repair stripe failed_slot_1 failed_slot_2\n", prg);
		exit_err = 1;
		goto exitHere;
	}

	mdfd = open(argv[1], O_RDONLY);
	if(mdfd < 0) {
		perror(argv[1]);
		fprintf(stderr, "%s: cannot open %s\n", prg, argv[1]);
		exit_err = 2;
		goto exitHere;
	}

	info = sysfs_read(mdfd, NULL,
			  GET_LEVEL|
			  GET_LAYOUT|
			  GET_DISKS|
			  GET_STATE |
			  GET_COMPONENT|
			  GET_CHUNK|
			  GET_DEVS|
			  GET_OFFSET|
			  GET_SIZE);

	if(info == NULL) {
		fprintf(stderr, "%s: Error reading sysfs information of %s\n", prg, argv[1]);
		exit_err = 9;
		goto exitHere;
	}

	if(info->array.level != level) {
		fprintf(stderr, "%s: %s not a RAID-6\n", prg, argv[1]);
		exit_err = 3;
		goto exitHere;
	}

	if(info->array.failed_disks > 0) {
		fprintf(stderr, "%s: %s degraded array\n", prg, argv[1]);
		exit_err = 8;
		goto exitHere;
	}

	printf("layout: %d\n", info->array.layout);
	printf("disks: %d\n", info->array.raid_disks);
	printf("component size: %llu\n", info->component_size * 512);
	printf("total stripes: %llu\n", (info->component_size * 512) / info->array.chunk_size);
	printf("chunk size: %d\n", info->array.chunk_size);
	printf("\n");

	comp = info->devs;
	for(i = 0, active_disks = 0; active_disks < info->array.raid_disks; i++) {
		printf("disk: %d - offset: %llu - size: %llu - name: %s - slot: %d\n",
			i, comp->data_offset * 512, comp->component_size * 512,
			map_dev(comp->disk.major, comp->disk.minor, 0),
			comp->disk.raid_disk);
		if(comp->disk.raid_disk >= 0)
			active_disks++;
		comp = comp->next;
	}
	printf("\n");

	close(mdfd);

	raid_disks = info->array.raid_disks;
	chunk_size = info->array.chunk_size;
	layout = info->array.layout;
	if (strcmp(argv[2], "repair")==0) {
		if (argc < 6) {
			fprintf(stderr, "For repair mode, call %s md_device repair stripe failed_slot_1 failed_slot_2\n", prg);
			exit_err = 1;
			goto exitHere;
		}
		repair = MANUAL_REPAIR;
		start = getnum(argv[3], &err);
		length = 1;
		failed_disk1 = getnum(argv[4], &err);
		failed_disk2 = getnum(argv[5], &err);

		if(failed_disk1 >= info->array.raid_disks) {
			fprintf(stderr, "%s: failed_slot_1 index is higher than number of devices in raid\n", prg);
			exit_err = 4;
			goto exitHere;
		}
		if(failed_disk2 >= info->array.raid_disks) {
			fprintf(stderr, "%s: failed_slot_2 index is higher than number of devices in raid\n", prg);
			exit_err = 4;
			goto exitHere;
		}
		if(failed_disk1 == failed_disk2) {
			fprintf(stderr, "%s: failed_slot_1 and failed_slot_2 are the same\n", prg);
			exit_err = 4;
			goto exitHere;
		}
	}
	else {
		start = getnum(argv[2], &err);
		length = getnum(argv[3], &err);
		if (argc >= 5 && strcmp(argv[4], "autorepair")==0)
			repair = AUTO_REPAIR;
	}

	if (err) {
		fprintf(stderr, "%s: Bad number: %s\n", prg, err);
		exit_err = 4;
		goto exitHere;
	}

	if(start > ((info->component_size * 512) / chunk_size)) {
		start = (info->component_size * 512) / chunk_size;
		fprintf(stderr, "%s: start beyond disks size\n", prg);
	}

	if((length == 0) ||
	   ((length + start) > ((info->component_size * 512) / chunk_size))) {
		length = (info->component_size * 512) / chunk_size - start;
	}

	disk_name = xmalloc(raid_disks * sizeof(*disk_name));
	fds = xmalloc(raid_disks * sizeof(*fds));
	offsets = xcalloc(raid_disks, sizeof(*offsets));
	buf = xmalloc(raid_disks * chunk_size);

	for(i=0; i<raid_disks; i++) {
		fds[i] = -1;
	}
	close_flag = 1;

	comp = info->devs;
	for (i=0, active_disks=0; active_disks<raid_disks; i++) {
		int disk_slot = comp->disk.raid_disk;
		if(disk_slot >= 0) {
			disk_name[disk_slot] = map_dev(comp->disk.major, comp->disk.minor, 0);
			offsets[disk_slot] = comp->data_offset * 512;
			fds[disk_slot] = open(disk_name[disk_slot], O_RDWR | O_DIRECT);
			if (fds[disk_slot] < 0) {
				perror(disk_name[disk_slot]);
				fprintf(stderr,"%s: cannot open %s\n", prg, disk_name[disk_slot]);
				exit_err = 6;
				goto exitHere;
			}
			active_disks++;
		}
		comp = comp->next;
	}

	int rv = check_stripes(info, fds, offsets,
			       raid_disks, chunk_size, level, layout,
			       start, length, disk_name, repair, failed_disk1, failed_disk2);
	if (rv != 0) {
		fprintf(stderr,	"%s: check_stripes returned %d\n", prg, rv);
		exit_err = 7;
		goto exitHere;
	}

exitHere:

	if (close_flag)
		for(i = 0; i < raid_disks; i++)
			close(fds[i]);

	free(disk_name);
	free(fds);
	free(offsets);
	free(buf);

	exit(exit_err);
}
