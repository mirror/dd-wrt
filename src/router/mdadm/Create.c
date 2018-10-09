/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2013 Neil Brown <neilb@suse.de>
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
 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include	"mdadm.h"
#include	"md_u.h"
#include	"md_p.h"
#include	<ctype.h>

static int default_layout(struct supertype *st, int level, int verbose)
{
	int layout = UnSet;

	if (st && st->ss->default_geometry)
		st->ss->default_geometry(st, &level, &layout, NULL);

	if (layout == UnSet)
		switch(level) {
		default: /* no layout */
			layout = 0;
			break;
		case 10:
			layout = 0x102; /* near=2, far=1 */
			if (verbose > 0)
				pr_err("layout defaults to n2\n");
			break;
		case 5:
		case 6:
			layout = map_name(r5layout, "default");
			if (verbose > 0)
				pr_err("layout defaults to %s\n", map_num(r5layout, layout));
			break;
		case LEVEL_FAULTY:
			layout = map_name(faultylayout, "default");

			if (verbose > 0)
				pr_err("layout defaults to %s\n", map_num(faultylayout, layout));
			break;
		}

	return layout;
}

int Create(struct supertype *st, char *mddev,
	   char *name, int *uuid,
	   int subdevs, struct mddev_dev *devlist,
	   struct shape *s,
	   struct context *c, unsigned long long data_offset)
{
	/*
	 * Create a new raid array.
	 *
	 * First check that necessary details are available
	 * (i.e. level, raid-disks)
	 *
	 * Then check each disk to see what might be on it
	 * and report anything interesting.
	 *
	 * If anything looks odd, and runstop not set,
	 * abort.
	 *
	 * SET_ARRAY_INFO and ADD_NEW_DISK, and
	 * if runstop==run, or raiddisks disks were used,
	 * RUN_ARRAY
	 */
	int mdfd;
	unsigned long long minsize = 0, maxsize = 0;
	char *mindisc = NULL;
	char *maxdisc = NULL;
	int dnum, raid_disk_num;
	struct mddev_dev *dv;
	dev_t rdev;
	int fail = 0, warn = 0;
	int first_missing = subdevs * 2;
	int second_missing = subdevs * 2;
	int missing_disks = 0;
	int insert_point = subdevs * 2; /* where to insert a missing drive */
	int total_slots;
	int pass;
	int rv;
	int bitmap_fd;
	int have_container = 0;
	int container_fd = -1;
	int need_mdmon = 0;
	unsigned long long bitmapsize;
	struct mdinfo info, *infos;
	int did_default = 0;
	int do_default_layout = 0;
	int do_default_chunk = 0;
	unsigned long safe_mode_delay = 0;
	char chosen_name[1024];
	struct map_ent *map = NULL;
	unsigned long long newsize;
	mdu_array_info_t inf;

	int major_num = BITMAP_MAJOR_HI;
	if (s->bitmap_file && strcmp(s->bitmap_file, "clustered") == 0) {
		major_num = BITMAP_MAJOR_CLUSTERED;
		if (c->nodes <= 1) {
			pr_err("At least 2 nodes are needed for cluster-md\n");
			return 1;
		}
	}

	memset(&info, 0, sizeof(info));
	if (s->level == UnSet && st && st->ss->default_geometry)
		st->ss->default_geometry(st, &s->level, NULL, NULL);
	if (s->level == UnSet) {
		pr_err("a RAID level is needed to create an array.\n");
		return 1;
	}
	if (s->raiddisks < 4 && s->level == 6) {
		pr_err("at least 4 raid-devices needed for level 6\n");
		return 1;
	}
	if (s->raiddisks > 256 && s->level == 6) {
		pr_err("no more than 256 raid-devices supported for level 6\n");
		return 1;
	}
	if (s->raiddisks < 2 && s->level >= 4) {
		pr_err("at least 2 raid-devices needed for level 4 or 5\n");
		return 1;
	}
	if (s->level <= 0 && s->sparedisks) {
		pr_err("This level does not support spare devices\n");
		return 1;
	}

	if (subdevs == 1 && strcmp(devlist->devname, "missing") != 0) {
		/* If given a single device, it might be a container, and we can
		 * extract a device list from there
		 */
		int fd;

		memset(&inf, 0, sizeof(inf));
		fd = open(devlist->devname, O_RDONLY);
		if (fd >= 0 &&
		    md_get_array_info(fd, &inf) == 0 && inf.raid_disks == 0) {
			/* yep, looks like a container */
			if (st) {
				rv = st->ss->load_container(st, fd,
							    devlist->devname);
				if (rv == 0)
					have_container = 1;
			} else {
				st = super_by_fd(fd, NULL);
				if (st && !(rv = st->ss->
					    load_container(st, fd,
							   devlist->devname)))
					have_container = 1;
				else
					st = NULL;
			}
			if (have_container) {
				subdevs = s->raiddisks;
				first_missing = subdevs * 2;
				second_missing = subdevs * 2;
				insert_point = subdevs * 2;
			}
		}
		if (fd >= 0)
			close(fd);
	}
	if (st && st->ss->external && s->sparedisks) {
		pr_err("This metadata type does not support spare disks at create time\n");
		return 1;
	}
	if (subdevs > s->raiddisks+s->sparedisks+s->journaldisks) {
		pr_err("You have listed more devices (%d) than are in the array(%d)!\n", subdevs, s->raiddisks+s->sparedisks);
		return 1;
	}
	if (!have_container && subdevs < s->raiddisks+s->sparedisks+s->journaldisks) {
		pr_err("You haven't given enough devices (real or missing) to create this array\n");
		return 1;
	}
	if (s->bitmap_file && s->level <= 0) {
		pr_err("bitmaps not meaningful with level %s\n",
			map_num(pers, s->level)?:"given");
		return 1;
	}

	/* now set some defaults */

	if (s->layout == UnSet) {
		do_default_layout = 1;
		s->layout = default_layout(st, s->level, c->verbose);
	}

	if (s->level == 10)
		/* check layout fits in array*/
		if ((s->layout&255) * ((s->layout>>8)&255) > s->raiddisks) {
			pr_err("that layout requires at least %d devices\n",
				(s->layout&255) * ((s->layout>>8)&255));
			return 1;
		}

	switch(s->level) {
	case 4:
	case 5:
	case 10:
	case 6:
	case 0:
		if (s->chunk == 0 || s->chunk == UnSet) {
			s->chunk = UnSet;
			do_default_chunk = 1;
			/* chunk will be set later */
		}
		break;
	case LEVEL_LINEAR:
		/* a chunksize of zero 0s perfectly valid (and preferred) since 2.6.16 */
		if (get_linux_version() < 2006016 && s->chunk == 0) {
			s->chunk = 64;
			if (c->verbose > 0)
				pr_err("chunk size defaults to 64K\n");
		}
		break;
	case 1:
	case LEVEL_FAULTY:
	case LEVEL_MULTIPATH:
	case LEVEL_CONTAINER:
		if (s->chunk) {
			s->chunk = 0;
			if (c->verbose > 0)
				pr_err("chunk size ignored for this level\n");
		}
		break;
	default:
		pr_err("unknown level %d\n", s->level);
		return 1;
	}
	if (s->size == MAX_SIZE)
		/* use '0' to mean 'max' now... */
		s->size = 0;
	if (s->size && s->chunk && s->chunk != UnSet)
		s->size &= ~(unsigned long long)(s->chunk - 1);
	newsize = s->size * 2;
	if (st && ! st->ss->validate_geometry(st, s->level, s->layout, s->raiddisks,
					      &s->chunk, s->size*2,
					      data_offset, NULL,
					      &newsize, s->consistency_policy,
					      c->verbose >= 0))
		return 1;

	if (s->chunk && s->chunk != UnSet) {
		newsize &= ~(unsigned long long)(s->chunk*2 - 1);
		if (do_default_chunk) {
			/* default chunk was just set */
			if (c->verbose > 0)
				pr_err("chunk size defaults to %dK\n", s->chunk);
			s->size &= ~(unsigned long long)(s->chunk - 1);
			do_default_chunk = 0;
		}
	}

	if (s->size == 0) {
		s->size = newsize / 2;
		if (s->level == 1)
			/* If this is ever reshaped to RAID5, we will
			 * need a chunksize.  So round it off a bit
			 * now just to be safe
			 */
			s->size &= ~(64ULL-1);

		if (s->size && c->verbose > 0)
			pr_err("setting size to %lluK\n", s->size);
	}

	/* now look at the subdevs */
	info.array.active_disks = 0;
	info.array.working_disks = 0;
	dnum = 0;
	for (dv = devlist; dv; dv = dv->next)
		if (data_offset == VARIABLE_OFFSET)
			dv->data_offset = INVALID_SECTORS;
		else
			dv->data_offset = data_offset;

	for (dv=devlist; dv && !have_container; dv=dv->next, dnum++) {
		char *dname = dv->devname;
		unsigned long long freesize;
		int dfd;
		char *doff;

		if (strcasecmp(dname, "missing") == 0) {
			if (first_missing > dnum)
				first_missing = dnum;
			if (second_missing > dnum && dnum > first_missing)
				second_missing = dnum;
			missing_disks ++;
			continue;
		}
		if (data_offset == VARIABLE_OFFSET) {
			doff = strchr(dname, ':');
			if (doff) {
				*doff++ = 0;
				dv->data_offset = parse_size(doff);
			} else
				dv->data_offset = INVALID_SECTORS;
		} else
			dv->data_offset = data_offset;

		dfd = open(dname, O_RDONLY);
		if (dfd < 0) {
			pr_err("cannot open %s: %s\n",
				dname, strerror(errno));
			exit(2);
		}
		if (!fstat_is_blkdev(dfd, dname, NULL)) {
			close(dfd);
			exit(2);
		}
		close(dfd);
		info.array.working_disks++;
		if (dnum < s->raiddisks && dv->disposition != 'j')
			info.array.active_disks++;
		if (st == NULL) {
			struct createinfo *ci = conf_get_create_info();
			if (ci)
				st = ci->supertype;
		}
		if (st == NULL) {
			/* Need to choose a default metadata, which is different
			 * depending on geometry of array.
			 */
			int i;
			char *name = "default";
			for(i = 0; !st && superlist[i]; i++) {
				st = superlist[i]->match_metadata_desc(name);
				if (!st)
					continue;
				if (do_default_layout)
					s->layout = default_layout(st, s->level, c->verbose);
				switch (st->ss->validate_geometry(
						st, s->level, s->layout, s->raiddisks,
						&s->chunk, s->size*2,
						dv->data_offset, dname,
						&freesize, s->consistency_policy,
						c->verbose > 0)) {
				case -1: /* Not valid, message printed, and not
					  * worth checking any further */
					exit(2);
					break;
				case 0: /* Geometry not valid */
					free(st);
					st = NULL;
					s->chunk = do_default_chunk ? UnSet : s->chunk;
					break;
				case 1:	/* All happy */
					break;
				}
			}

			if (!st) {
				int dfd = open(dname, O_RDONLY|O_EXCL);
				if (dfd < 0) {
					pr_err("cannot open %s: %s\n",
						dname, strerror(errno));
					exit(2);
				}
				pr_err("device %s not suitable for any style of array\n",
					dname);
				exit(2);
			}
			if (st->ss != &super0 ||
			    st->minor_version != 90)
				did_default = 1;
		} else {
			if (do_default_layout)
				s->layout = default_layout(st, s->level, 0);
			if (!st->ss->validate_geometry(st, s->level, s->layout,
						       s->raiddisks,
						       &s->chunk, s->size*2,
						       dv->data_offset,
						       dname, &freesize,
						       s->consistency_policy,
						       c->verbose >= 0)) {

				pr_err("%s is not suitable for this array.\n",
				       dname);
				fail = 1;
				continue;
			}
		}

		if (dv->disposition == 'j')
			goto skip_size_check;  /* skip write journal for size check */

		freesize /= 2; /* convert to K */
		if (s->chunk && s->chunk != UnSet) {
			/* round to chunk size */
			freesize = freesize & ~(s->chunk-1);
			if (do_default_chunk) {
				/* default chunk was just set */
				if (c->verbose > 0)
					pr_err("chunk size defaults to %dK\n", s->chunk);
				s->size &= ~(unsigned long long)(s->chunk - 1);
				do_default_chunk = 0;
			}
		}
		if (!freesize) {
			pr_err("no free space left on %s\n", dname);
			fail = 1;
			continue;
		}

		if (s->size && freesize < s->size) {
			pr_err("%s is smaller than given size. %lluK < %lluK + metadata\n",
				dname, freesize, s->size);
			fail = 1;
			continue;
		}
		if (maxdisc == NULL || (maxdisc && freesize > maxsize)) {
			maxdisc = dname;
			maxsize = freesize;
		}
		if (mindisc ==NULL || (mindisc && freesize < minsize)) {
			mindisc = dname;
			minsize = freesize;
		}
	skip_size_check:
		if (c->runstop != 1 || c->verbose >= 0) {
			int fd = open(dname, O_RDONLY);
			if (fd < 0) {
				pr_err("Cannot open %s: %s\n",
					dname, strerror(errno));
				fail = 1;
				continue;
			}
			warn |= check_ext2(fd, dname);
			warn |= check_reiser(fd, dname);
			warn |= check_raid(fd, dname);
			if (strcmp(st->ss->name, "1.x") == 0 &&
			    st->minor_version >= 1)
				/* metadata at front */
				warn |= check_partitions(fd, dname, 0, 0);
			else if (s->level == 1 || s->level == LEVEL_CONTAINER ||
				 (s->level == 0 && s->raiddisks == 1))
				/* partitions could be meaningful */
				warn |= check_partitions(fd, dname, freesize*2, s->size*2);
			else
				/* partitions cannot be meaningful */
				warn |= check_partitions(fd, dname, 0, 0);
			if (strcmp(st->ss->name, "1.x") == 0 &&
			    st->minor_version >= 1 &&
			    did_default &&
			    s->level == 1 &&
			    (warn & 1024) == 0) {
				warn |= 1024;
				pr_err("Note: this array has metadata at the start and\n"
					"    may not be suitable as a boot device.  If you plan to\n"
					"    store '/boot' on this device please ensure that\n"
					"    your boot-loader understands md/v1.x metadata, or use\n"
					"    --metadata=0.90\n");
			}
			close(fd);
		}
	}
	if (missing_disks == dnum && !have_container) {
		pr_err("Subdevs can't be all missing\n");
		return 1;
	}
	if (s->raiddisks + s->sparedisks > st->max_devs) {
		pr_err("Too many devices: %s metadata only supports %d\n",
			st->ss->name, st->max_devs);
		return 1;
	}
	if (have_container)
		info.array.working_disks = s->raiddisks;
	if (fail) {
		pr_err("create aborted\n");
		return 1;
	}
	if (s->size == 0) {
		if (mindisc == NULL && !have_container) {
			pr_err("no size and no drives given - aborting create.\n");
			return 1;
		}
		if (s->level > 0 || s->level == LEVEL_MULTIPATH ||
		    s->level == LEVEL_FAULTY || st->ss->external) {
			/* size is meaningful */
			if (!st->ss->validate_geometry(st, s->level, s->layout,
						       s->raiddisks,
						       &s->chunk, minsize*2,
						       data_offset,
						       NULL, NULL,
						       s->consistency_policy, 0)) {
				pr_err("devices too large for RAID level %d\n", s->level);
				return 1;
			}
			s->size = minsize;
			if (s->level == 1)
				/* If this is ever reshaped to RAID5, we will
				 * need a chunksize.  So round it off a bit
				 * now just to be safe
				 */
				s->size &= ~(64ULL-1);
			if (c->verbose > 0)
				pr_err("size set to %lluK\n", s->size);
		}
	}

	if (!s->bitmap_file &&
	    s->level >= 1 &&
	    st->ss->add_internal_bitmap &&
	    (s->consistency_policy != CONSISTENCY_POLICY_RESYNC &&
	     s->consistency_policy != CONSISTENCY_POLICY_PPL) &&
	    (s->write_behind || s->size > 100*1024*1024ULL)) {
		if (c->verbose > 0)
			pr_err("automatically enabling write-intent bitmap on large array\n");
		s->bitmap_file = "internal";
	}
	if (s->bitmap_file && strcmp(s->bitmap_file, "none") == 0)
		s->bitmap_file = NULL;

	if (s->consistency_policy == CONSISTENCY_POLICY_PPL &&
	    !st->ss->write_init_ppl) {
		pr_err("%s metadata does not support PPL\n", st->ss->name);
		return 1;
	}

	if (!have_container && s->level > 0 && ((maxsize-s->size)*100 > maxsize)) {
		if (c->runstop != 1 || c->verbose >= 0)
			pr_err("largest drive (%s) exceeds size (%lluK) by more than 1%%\n",
				maxdisc, s->size);
		warn = 1;
	}

	if (st->ss->detail_platform && st->ss->detail_platform(0, 1, NULL) != 0) {
		if (c->runstop != 1 || c->verbose >= 0)
			pr_err("%s unable to enumerate platform support\n"
				"    array may not be compatible with hardware/firmware\n",
				st->ss->name);
		warn = 1;
	}
	st->nodes = c->nodes;
	st->cluster_name = c->homecluster;

	if (warn) {
		if (c->runstop!= 1) {
			if (!ask("Continue creating array? ")) {
				pr_err("create aborted.\n");
				return 1;
			}
		} else {
			if (c->verbose > 0)
				pr_err("creation continuing despite oddities due to --run\n");
		}
	}

	/* If this is raid4/5, we want to configure the last active slot
	 * as missing, so that a reconstruct happens (faster than re-parity)
	 * FIX: Can we do this for raid6 as well?
	 */
	if (st->ss->external == 0 && s->assume_clean == 0 &&
	    c->force == 0 && first_missing >= s->raiddisks) {
		switch (s->level) {
		case 4:
		case 5:
			insert_point = s->raiddisks-1;
			s->sparedisks++;
			info.array.active_disks--;
			missing_disks++;
			break;
		default:
			break;
		}
	}
	/* For raid6, if creating with 1 missing drive, make a good drive
	 * into a spare, else the create will fail
	 */
	if (s->assume_clean == 0 && c->force == 0 && first_missing < s->raiddisks &&
	    st->ss->external == 0 &&
	    second_missing >= s->raiddisks && s->level == 6) {
		insert_point = s->raiddisks - 1;
		if (insert_point == first_missing)
			insert_point--;
		s->sparedisks ++;
		info.array.active_disks--;
		missing_disks++;
	}

	if (s->level <= 0 && first_missing < subdevs * 2) {
		pr_err("This level does not support missing devices\n");
		return 1;
	}

	/* We need to create the device */
	map_lock(&map);
	mdfd = create_mddev(mddev, name, c->autof, LOCAL, chosen_name, 1);
	if (mdfd < 0) {
		map_unlock(&map);
		return 1;
	}
	/* verify if chosen_name is not in use,
	 * it could be in conflict with already existing device
	 * e.g. container, array
	 */
	if (strncmp(chosen_name, "/dev/md/", 8) == 0 &&
	    map_by_name(&map, chosen_name+8) != NULL) {
		pr_err("Array name %s is in use already.\n",
			chosen_name);
		close(mdfd);
		map_unlock(&map);
		udev_unblock();
		return 1;
	}
	mddev = chosen_name;

	memset(&inf, 0, sizeof(inf));
	md_get_array_info(mdfd, &inf);
	if (inf.working_disks != 0) {
		pr_err("another array by this name is already running.\n");
		goto abort_locked;
	}

	/* Ok, lets try some ioctls */

	info.array.level = s->level;
	info.array.size = s->size;
	info.array.raid_disks = s->raiddisks;
	/* The kernel should *know* what md_minor we are dealing
	 * with, but it chooses to trust me instead. Sigh
	 */
	info.array.md_minor = 0;
	if (fstat_is_blkdev(mdfd, mddev, &rdev))
		info.array.md_minor = minor(rdev);
	info.array.not_persistent = 0;

	if (((s->level == 4 || s->level == 5) &&
	     (insert_point < s->raiddisks || first_missing < s->raiddisks)) ||
	    (s->level == 6 && (insert_point < s->raiddisks ||
			       second_missing < s->raiddisks)) ||
	    (s->level <= 0) || s->assume_clean) {
		info.array.state = 1; /* clean, but one+ drive will be missing*/
		info.resync_start = MaxSector;
	} else {
		info.array.state = 0; /* not clean, but no errors */
		info.resync_start = 0;
	}
	if (s->level == 10) {
		/* for raid10, the bitmap size is the capacity of the array,
		 * which is array.size * raid_disks / ncopies;
		 * .. but convert to sectors.
		 */
		int ncopies = ((s->layout>>8) & 255) * (s->layout & 255);
		bitmapsize = s->size * s->raiddisks / ncopies * 2;
/*		printf("bms=%llu as=%d rd=%d nc=%d\n", bitmapsize, s->size, s->raiddisks, ncopies);*/
	} else
		bitmapsize = s->size * 2;

	/* There is lots of redundancy in these disk counts,
	 * raid_disks is the most meaningful value
	 *          it describes the geometry of the array
	 *          it is constant
	 * nr_disks is total number of used slots.
	 *          it should be raid_disks+spare_disks
	 * spare_disks is the number of extra disks present
	 *          see above
	 * active_disks is the number of working disks in
	 *          active slots. (With raid_disks)
	 * working_disks is the total number of working disks,
	 *          including spares
	 * failed_disks is the number of disks marked failed
	 *
	 * Ideally, the kernel would keep these (except raid_disks)
	 * up-to-date as we ADD_NEW_DISK, but it doesn't (yet).
	 * So for now, we assume that all raid and spare
	 * devices will be given.
	 */
	info.array.spare_disks=s->sparedisks;
	info.array.failed_disks=missing_disks;
	info.array.nr_disks = info.array.working_disks
		+ info.array.failed_disks;
	info.array.layout = s->layout;
	info.array.chunk_size = s->chunk*1024;

	if (name == NULL || *name == 0) {
		/* base name on mddev */
		/*  /dev/md0 -> 0
		 *  /dev/md_d0 -> d0
		 *  /dev/md_foo -> foo
		 *  /dev/md/1 -> 1
		 *  /dev/md/d1 -> d1
		 *  /dev/md/home -> home
		 *  /dev/mdhome -> home
		 */
		/* FIXME compare this with rules in create_mddev */
		name = strrchr(mddev, '/');
		if (name) {
			name++;
			if (strncmp(name, "md_", 3) == 0 &&
			    strlen(name) > 3 && (name-mddev) == 5 /* /dev/ */)
				name += 3;
			else if (strncmp(name, "md", 2) == 0 &&
				 strlen(name) > 2 && isdigit(name[2]) &&
				 (name-mddev) == 5 /* /dev/ */)
				name += 2;
		}
	}
	if (!st->ss->init_super(st, &info.array, s, name, c->homehost, uuid,
				data_offset))
		goto abort_locked;

	total_slots = info.array.nr_disks;
	st->ss->getinfo_super(st, &info, NULL);
	if (sysfs_init(&info, mdfd, NULL)) {
		pr_err("unable to initialize sysfs\n");
		goto abort_locked;
	}

	if (did_default && c->verbose >= 0) {
		if (is_subarray(info.text_version)) {
			char devnm[32];
			char *ep;
			struct mdinfo *mdi;

			strncpy(devnm, info.text_version+1, 32);
			devnm[31] = 0;
			ep = strchr(devnm, '/');
			if (ep)
				*ep = 0;

			mdi = sysfs_read(-1, devnm, GET_VERSION);

			pr_err("Creating array inside %s container %s\n",
				mdi?mdi->text_version:"managed", devnm);
			sysfs_free(mdi);
		} else
			pr_err("Defaulting to version %s metadata\n", info.text_version);
	}

	map_update(&map, fd2devnm(mdfd), info.text_version,
		   info.uuid, chosen_name);
	/* Keep map locked until devices have been added to array
	 * to stop another mdadm from finding and using those devices.
	 */

	if (s->bitmap_file && (strcmp(s->bitmap_file, "internal") == 0 ||
			       strcmp(s->bitmap_file, "clustered") == 0)) {
		if (!st->ss->add_internal_bitmap) {
			pr_err("internal bitmaps not supported with %s metadata\n",
				st->ss->name);
			goto abort_locked;
		}
		if (st->ss->add_internal_bitmap(st, &s->bitmap_chunk,
						c->delay, s->write_behind,
						bitmapsize, 1, major_num)) {
			pr_err("Given bitmap chunk size not supported.\n");
			goto abort_locked;
		}
		s->bitmap_file = NULL;
	}

	if (sysfs_init(&info, mdfd, NULL)) {
		pr_err("unable to initialize sysfs\n");
		goto abort_locked;
	}

	if (st->ss->external && st->container_devnm[0]) {
		/* member */

		/* When creating a member, we need to be careful
		 * to negotiate with mdmon properly.
		 * If it is already running, we cannot write to
		 * the devices and must ask it to do that part.
		 * If it isn't running, we write to the devices,
		 * and then start it.
		 * We hold an exclusive open on the container
		 * device to make sure mdmon doesn't exit after
		 * we checked that it is running.
		 *
		 * For now, fail if it is already running.
		 */
		container_fd = open_dev_excl(st->container_devnm);
		if (container_fd < 0) {
			pr_err("Cannot get exclusive open on container - weird.\n");
			goto abort_locked;
		}
		if (mdmon_running(st->container_devnm)) {
			if (c->verbose)
				pr_err("reusing mdmon for %s.\n",
					st->container_devnm);
			st->update_tail = &st->updates;
		} else
			need_mdmon = 1;
	}
	rv = set_array_info(mdfd, st, &info);
	if (rv) {
		pr_err("failed to set array info for %s: %s\n",
			mddev, strerror(errno));
		goto abort_locked;
	}

	if (s->bitmap_file) {
		int uuid[4];

		st->ss->uuid_from_super(st, uuid);
		if (CreateBitmap(s->bitmap_file, c->force, (char*)uuid, s->bitmap_chunk,
				 c->delay, s->write_behind,
				 bitmapsize,
				 major_num)) {
			goto abort_locked;
		}
		bitmap_fd = open(s->bitmap_file, O_RDWR);
		if (bitmap_fd < 0) {
			pr_err("weird: %s cannot be openned\n",
				s->bitmap_file);
			goto abort_locked;
		}
		if (ioctl(mdfd, SET_BITMAP_FILE, bitmap_fd) < 0) {
			pr_err("Cannot set bitmap file for %s: %s\n",
				mddev, strerror(errno));
			goto abort_locked;
		}
	}

	infos = xmalloc(sizeof(*infos) * total_slots);
	enable_fds(total_slots);
	for (pass = 1; pass <= 2; pass++) {
		struct mddev_dev *moved_disk = NULL; /* the disk that was moved out of the insert point */

		for (dnum = 0, raid_disk_num = 0, dv = devlist; dv;
		     dv = (dv->next) ? (dv->next) : moved_disk, dnum++) {
			int fd;
			struct mdinfo *inf = &infos[dnum];

			if (dnum >= total_slots)
				abort();
			if (dnum == insert_point) {
				raid_disk_num += 1;
				moved_disk = dv;
				continue;
			}
			if (strcasecmp(dv->devname, "missing") == 0) {
				raid_disk_num += 1;
				continue;
			}
			if (have_container)
				moved_disk = NULL;
			if (have_container && dnum < info.array.raid_disks - 1)
				/* repeatedly use the container */
				moved_disk = dv;

			switch(pass) {
			case 1:
				*inf = info;

				inf->disk.number = dnum;
				inf->disk.raid_disk = raid_disk_num++;

				if (dv->disposition == 'j') {
					inf->disk.raid_disk = MD_DISK_ROLE_JOURNAL;
					inf->disk.state = (1<<MD_DISK_JOURNAL);
					raid_disk_num--;
				} else if (inf->disk.raid_disk < s->raiddisks)
					inf->disk.state = (1<<MD_DISK_ACTIVE) |
						(1<<MD_DISK_SYNC);
				else
					inf->disk.state = 0;

				if (dv->writemostly == FlagSet)
					inf->disk.state |= (1<<MD_DISK_WRITEMOSTLY);
				if (dv->failfast == FlagSet)
					inf->disk.state |= (1<<MD_DISK_FAILFAST);

				if (have_container)
					fd = -1;
				else {
					if (st->ss->external &&
					    st->container_devnm[0])
						fd = open(dv->devname, O_RDWR);
					else
						fd = open(dv->devname, O_RDWR|O_EXCL);

					if (fd < 0) {
						pr_err("failed to open %s after earlier success - aborting\n",
							dv->devname);
						goto abort_locked;
					}
					if (!fstat_is_blkdev(fd, dv->devname, &rdev))
						return 1;
					inf->disk.major = major(rdev);
					inf->disk.minor = minor(rdev);
				}
				if (fd >= 0)
					remove_partitions(fd);
				if (st->ss->add_to_super(st, &inf->disk,
							 fd, dv->devname,
							 dv->data_offset)) {
					ioctl(mdfd, STOP_ARRAY, NULL);
					goto abort_locked;
				}
				st->ss->getinfo_super(st, inf, NULL);
				safe_mode_delay = inf->safe_mode_delay;

				if (have_container && c->verbose > 0)
					pr_err("Using %s for device %d\n",
						map_dev(inf->disk.major,
							inf->disk.minor,
							0), dnum);

				if (!have_container) {
					/* getinfo_super might have lost these ... */
					inf->disk.major = major(rdev);
					inf->disk.minor = minor(rdev);
				}
				break;
			case 2:
				inf->errors = 0;

				rv = add_disk(mdfd, st, &info, inf);

				if (rv) {
					pr_err("ADD_NEW_DISK for %s failed: %s\n",
					       dv->devname, strerror(errno));
					goto abort_locked;
				}
				break;
			}
			if (!have_container &&
			    dv == moved_disk && dnum != insert_point) break;
		}
		if (pass == 1) {
			struct mdinfo info_new;
			struct map_ent *me = NULL;

			/* check to see if the uuid has changed due to these
			 * metadata changes, and if so update the member array
			 * and container uuid.  Note ->write_init_super clears
			 * the subarray cursor such that ->getinfo_super once
			 * again returns container info.
			 */
			st->ss->getinfo_super(st, &info_new, NULL);
			if (st->ss->external && s->level != LEVEL_CONTAINER &&
			    !same_uuid(info_new.uuid, info.uuid, 0)) {
				map_update(&map, fd2devnm(mdfd),
					   info_new.text_version,
					   info_new.uuid, chosen_name);
				me = map_by_devnm(&map, st->container_devnm);
			}

			if (st->ss->write_init_super(st)) {
				st->ss->free_super(st);
				goto abort_locked;
			}

			/* update parent container uuid */
			if (me) {
				char *path = xstrdup(me->path);

				st->ss->getinfo_super(st, &info_new, NULL);
				map_update(&map, st->container_devnm,
					   info_new.text_version,
					   info_new.uuid, path);
				free(path);
			}

			flush_metadata_updates(st);
			st->ss->free_super(st);
		}
	}
	map_unlock(&map);
	free(infos);

	if (s->level == LEVEL_CONTAINER) {
		/* No need to start.  But we should signal udev to
		 * create links */
		sysfs_uevent(&info, "change");
		if (c->verbose >= 0)
			pr_err("container %s prepared.\n", mddev);
		wait_for(chosen_name, mdfd);
	} else if (c->runstop == 1 || subdevs >= s->raiddisks) {
		if (st->ss->external) {
			int err;
			switch(s->level) {
			case LEVEL_LINEAR:
			case LEVEL_MULTIPATH:
			case 0:
				err = sysfs_set_str(&info, NULL, "array_state",
						    c->readonly
						    ? "readonly"
						    : "active");
				need_mdmon = 0;
				break;
			default:
				err = sysfs_set_str(&info, NULL, "array_state",
						    "readonly");
				break;
			}
			sysfs_set_safemode(&info, safe_mode_delay);
			if (err) {
				pr_err("failed to activate array.\n");
				ioctl(mdfd, STOP_ARRAY, NULL);
				goto abort;
			}
		} else if (c->readonly &&
			   sysfs_attribute_available(
				   &info, NULL, "array_state")) {
			if (sysfs_set_str(&info, NULL,
					  "array_state", "readonly") < 0) {
				pr_err("Failed to start array: %s\n",
				       strerror(errno));
				ioctl(mdfd, STOP_ARRAY, NULL);
				goto abort;
			}
		} else {
			/* param is not actually used */
			mdu_param_t param;
			if (ioctl(mdfd, RUN_ARRAY, &param)) {
				pr_err("RUN_ARRAY failed: %s\n",
				       strerror(errno));
				if (info.array.chunk_size & (info.array.chunk_size-1)) {
					cont_err("Problem may be that chunk size is not a power of 2\n");
				}
				ioctl(mdfd, STOP_ARRAY, NULL);
				goto abort;
			}
			/* if start_ro module parameter is set, array is
			 * auto-read-only, which is bad as the resync won't
			 * start.  So lets make it read-write now.
			 */
			ioctl(mdfd, RESTART_ARRAY_RW, NULL);
		}
		if (c->verbose >= 0)
			pr_err("array %s started.\n", mddev);
		if (st->ss->external && st->container_devnm[0]) {
			if (need_mdmon)
				start_mdmon(st->container_devnm);

			ping_monitor(st->container_devnm);
			close(container_fd);
		}
		wait_for(chosen_name, mdfd);
	} else {
		pr_err("not starting array - not enough devices.\n");
	}
	close(mdfd);
	/* Give udev a moment to process the Change event caused
	 * by the close.
	 */
	usleep(100*1000);
	udev_unblock();
	return 0;

 abort:
	udev_unblock();
	map_lock(&map);
 abort_locked:
	map_remove(&map, fd2devnm(mdfd));
	map_unlock(&map);

	if (mdfd >= 0)
		close(mdfd);
	return 1;
}
