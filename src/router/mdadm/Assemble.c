/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2016 Neil Brown <neilb@suse.com>
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
#include	<ctype.h>

static int name_matches(char *found, char *required, char *homehost, int require_homehost)
{
	/* See if the name found matches the required name, possibly
	 * prefixed with 'homehost'
	 */
	char *sep;
	unsigned int l;

	if (strcmp(found, required)==0)
		return 1;
	sep = strchr(found, ':');
	if (!sep)
		return 0;
	l = sep - found;
	if (strncmp(found, "any:", 4) == 0 ||
	    (homehost && strcmp(homehost, "any") == 0) ||
	    !require_homehost ||
	    (homehost && strlen(homehost) == l &&
	     strncmp(found, homehost, l) == 0)) {
		/* matching homehost */
		if (strcmp(sep+1, required) == 0)
			return 1;
	}
	return 0;
}

static int is_member_busy(char *metadata_version)
{
	/* check if the given member array is active */
	struct mdstat_ent *mdstat = mdstat_read(0, 0);
	struct mdstat_ent *ent;
	int busy = 0;

	for (ent = mdstat; ent; ent = ent->next) {
		if (ent->metadata_version == NULL)
			continue;
		if (strncmp(ent->metadata_version, "external:", 9) != 0)
			continue;
		if (!is_subarray(&ent->metadata_version[9]))
			continue;
		/* Skip first char - it can be '/' or '-' */
		if (strcmp(&ent->metadata_version[10], metadata_version+1) == 0) {
			busy = 1;
			break;
		}
	}
	free_mdstat(mdstat);

	return busy;
}

static int ident_matches(struct mddev_ident *ident,
			 struct mdinfo *content,
			 struct supertype *tst,
			 char *homehost, int require_homehost,
			 char *update, char *devname)
{

	if (ident->uuid_set && (!update || strcmp(update, "uuid")!= 0) &&
	    same_uuid(content->uuid, ident->uuid, tst->ss->swapuuid)==0 &&
	    memcmp(content->uuid, uuid_zero, sizeof(int[4])) != 0) {
		if (devname)
			pr_err("%s has wrong uuid.\n", devname);
		return 0;
	}
	if (ident->name[0] && (!update || strcmp(update, "name")!= 0) &&
	    name_matches(content->name, ident->name, homehost, require_homehost)==0) {
		if (devname)
			pr_err("%s has wrong name.\n", devname);
		return 0;
	}
	if (ident->super_minor != UnSet &&
	    ident->super_minor != content->array.md_minor) {
		if (devname)
			pr_err("%s has wrong super-minor.\n",
			       devname);
		return 0;
	}
	if (ident->level != UnSet &&
	    ident->level != content->array.level) {
		if (devname)
			pr_err("%s has wrong raid level.\n",
			       devname);
		return 0;
	}
	if (ident->raid_disks != UnSet &&
	    content->array.raid_disks != 0 && /* metadata doesn't know how many to expect */
	    ident->raid_disks!= content->array.raid_disks) {
		if (devname)
			pr_err("%s requires wrong number of drives.\n",
			       devname);
		return 0;
	}
	if (ident->member && ident->member[0]) {
		/* content->text_version must match */
		char *s = strchr(content->text_version+1, '/');
		if (s == NULL) {
			if (devname)
				pr_err("%s is not a container and one is required.\n",
				       devname);
			return 0;
		} else if (strcmp(ident->member, s+1) != 0) {
			if (devname)
				pr_err("skipping wrong member %s is %s\n",
				       content->text_version, devname);
			return 0;
		}
	}
	return 1;
}

static int select_devices(struct mddev_dev *devlist,
			  struct mddev_ident *ident,
			  struct supertype **stp,
			  struct mdinfo **contentp,
			  struct context *c,
			  int inargv, int auto_assem)
{
	struct mddev_dev *tmpdev;
	int num_devs;
	struct supertype *st = *stp;
	struct mdinfo *content = NULL;
	int report_mismatch = ((inargv && c->verbose >= 0) || c->verbose > 0);
	struct domainlist *domains = NULL;
	dev_t rdev;

	tmpdev = devlist; num_devs = 0;
	while (tmpdev) {
		if (tmpdev->used)
			tmpdev->used = 2;
		else
			num_devs++;
		tmpdev->disposition = 0;
		tmpdev = tmpdev->next;
	}

	/* first walk the list of devices to find a consistent set
	 * that match the criterea, if that is possible.
	 * We flag the ones we like with 'used'.
	 */
	for (tmpdev = devlist;
	     tmpdev;
	     tmpdev = tmpdev ? tmpdev->next : NULL) {
		char *devname = tmpdev->devname;
		int dfd;
		struct supertype *tst;
		struct dev_policy *pol = NULL;
		int found_container = 0;

		if (tmpdev->used > 1)
			continue;

		if (ident->container) {
			if (ident->container[0] == '/' &&
			    !same_dev(ident->container, devname)) {
				if (report_mismatch)
					pr_err("%s is not the container required (%s)\n",
					       devname, ident->container);
				continue;
			}
		} else if (ident->devices &&
			   !match_oneof(ident->devices, devname)) {
			/* Note that we ignore the "device=" identifier if a
			 * "container=" is given.  Checking both is unnecessarily
			 * complicated.
			 */
			if (report_mismatch)
				pr_err("%s is not one of %s\n", devname, ident->devices);
			continue;
		}

		tst = dup_super(st);

		dfd = dev_open(devname, O_RDONLY);
		if (dfd < 0) {
			if (report_mismatch)
				pr_err("cannot open device %s: %s\n",
				       devname, strerror(errno));
			tmpdev->used = 2;
		} else if (!fstat_is_blkdev(dfd, devname, &rdev)) {
			tmpdev->used = 2;
		} else if (must_be_container(dfd)) {
			if (st) {
				/* already found some components, this cannot
				 * be another one.
				 */
				if (report_mismatch)
					pr_err("%s is a container, but we are looking for components\n",
					       devname);
				tmpdev->used = 2;
			} if (!tst && (tst = super_by_fd(dfd, NULL)) == NULL) {
				if (report_mismatch)
					pr_err("not a recognisable container: %s\n",
					       devname);
				tmpdev->used = 2;
			} else if (!tst->ss->load_container ||
				   tst->ss->load_container(tst, dfd, NULL)) {
				if (report_mismatch)
					pr_err("no correct container type: %s\n",
					       devname);
				tmpdev->used = 2;
			} else if (auto_assem &&
				   !conf_test_metadata(tst->ss->name,
						       (pol = devid_policy(rdev)),
						       tst->ss->match_home(tst, c->homehost) == 1)) {
				if (report_mismatch)
					pr_err("%s has metadata type %s for which auto-assembly is disabled\n",
					       devname, tst->ss->name);
				tmpdev->used = 2;
			} else
				found_container = 1;
		} else {
			if (!tst && (tst = guess_super(dfd)) == NULL) {
				if (report_mismatch)
					pr_err("no recogniseable superblock on %s\n",
					       devname);
				tmpdev->used = 2;
			} else if ((tst->ignore_hw_compat = 0),
				   tst->ss->load_super(tst, dfd,
						       report_mismatch ? devname : NULL)) {
				if (report_mismatch)
					pr_err("no RAID superblock on %s\n",
					       devname);
				tmpdev->used = 2;
			} else if (tst->ss->compare_super == NULL) {
				if (report_mismatch)
					pr_err("Cannot assemble %s metadata on %s\n",
					       tst->ss->name, devname);
				tmpdev->used = 2;
			} else if (auto_assem && st == NULL &&
				   !conf_test_metadata(tst->ss->name,
						       (pol = devid_policy(rdev)),
						       tst->ss->match_home(tst, c->homehost) == 1)) {
				if (report_mismatch)
					pr_err("%s has metadata type %s for which auto-assembly is disabled\n",
					       devname, tst->ss->name);
				tmpdev->used = 2;
			}
		}
		if (dfd >= 0) close(dfd);
		if (tmpdev->used == 2) {
			if (auto_assem || !inargv)
				/* Ignore unrecognised devices during auto-assembly */
				goto loop;
			if (ident->uuid_set || ident->name[0] ||
			    ident->super_minor != UnSet)
				/* Ignore unrecognised device if looking for
				 * specific array */
				goto loop;

			pr_err("%s has no superblock - assembly aborted\n",
			       devname);
			if (st)
				st->ss->free_super(st);
			dev_policy_free(pol);
			domain_free(domains);
			if (tst)
				tst->ss->free_super(tst);
			return -1;
		}

		if (found_container) {
			/* tmpdev is a container.  We need to be either
			 * looking for a member, or auto-assembling
			 */
			/* should be safe to try an exclusive open now, we
			 * have rejected anything that some other mdadm might
			 * be looking at
			 */
			dfd = dev_open(devname, O_RDONLY | O_EXCL);
			if (dfd < 0) {
				if (report_mismatch)
					pr_err("%s is busy - skipping\n", devname);
				goto loop;
			}
			close(dfd);

			if (ident->container && ident->container[0] != '/') {
				/* we have a uuid */
				int uuid[4];

				content = *contentp;
				tst->ss->getinfo_super(tst, content, NULL);

				if (!parse_uuid(ident->container, uuid) ||
				    !same_uuid(content->uuid, uuid, tst->ss->swapuuid)) {
					if (report_mismatch)
						pr_err("%s has wrong UUID to be required container\n",
						       devname);
					goto loop;
				}
			}
			/* It is worth looking inside this container.
			 */
			if (c->verbose > 0)
				pr_err("looking in container %s\n",
				       devname);

			for (content = tst->ss->container_content(tst, NULL);
			     content;
			     content = content->next) {

				if (!ident_matches(ident, content, tst,
						   c->homehost, c->require_homehost,
						   c->update,
						   report_mismatch ? devname : NULL))
					/* message already printed */;
				else if (is_member_busy(content->text_version)) {
					if (report_mismatch)
						pr_err("member %s in %s is already assembled\n",
						       content->text_version,
						       devname);
				} else if (content->array.state & (1<<MD_SB_BLOCK_VOLUME)) {
					/* do not assemble arrays with unsupported configurations */
					pr_err("Cannot activate member %s in %s.\n",
					       content->text_version,
					       devname);
				} else
					break;
			}
			if (!content) {
				tmpdev->used = 2;
				goto loop; /* empty container */
			}

			st = tst; tst = NULL;
			if (!auto_assem && inargv && tmpdev->next != NULL) {
				pr_err("%s is a container, but is not only device given: confused and aborting\n",
				       devname);
				st->ss->free_super(st);
				dev_policy_free(pol);
				domain_free(domains);
				return -1;
			}
			if (c->verbose > 0)
				pr_err("found match on member %s in %s\n",
				       content->text_version, devname);

			/* make sure we finished the loop */
			tmpdev = NULL;
			goto loop;
		} else {
			content = *contentp;
			tst->ss->getinfo_super(tst, content, NULL);

			if (!ident_matches(ident, content, tst,
					   c->homehost, c->require_homehost,
					   c->update,
					   report_mismatch ? devname : NULL))
				goto loop;

			if (auto_assem) {
				/* Never auto-assemble things that conflict
				 * with mdadm.conf in some way
				 */
				struct mddev_ident *match;
				int rv = 0;

				match = conf_match(tst, content, devname,
						   report_mismatch ? c->verbose : -1,
						   &rv);
				if (!match && rv == 2)
					goto loop;
				if (match && match->devname &&
				    strcasecmp(match->devname, "<ignore>") == 0) {
					if (report_mismatch)
						pr_err("%s is a member of an explicitly ignored array\n",
						       devname);
					goto loop;
				}
				if (match && !ident_matches(match, content, tst,
							    c->homehost, c->require_homehost,
							    c->update,
							    report_mismatch ? devname : NULL))
					/* Array exists  in mdadm.conf but some
					 * details don't match, so reject it
					 */
					goto loop;
			}

			/* should be safe to try an exclusive open now, we
			 * have rejected anything that some other mdadm might
			 * be looking at
			 */
			dfd = dev_open(devname, O_RDONLY | O_EXCL);
			if (dfd < 0) {
				if (report_mismatch)
					pr_err("%s is busy - skipping\n", devname);
				goto loop;
			}
			close(dfd);

			if (st == NULL)
				st = dup_super(tst);
			if (st->minor_version == -1)
				st->minor_version = tst->minor_version;

			if (memcmp(content->uuid, uuid_zero,
				   sizeof(int[4])) == 0) {
				/* this is a floating spare.  It cannot define
				 * an array unless there are no more arrays of
				 * this type to be found.  It can be included
				 * in an array of this type though.
				 */
				tmpdev->used = 3;
				goto loop;
			}

			if (st->ss != tst->ss ||
			    st->minor_version != tst->minor_version ||
			    st->ss->compare_super(st, tst) != 0) {
				/* Some mismatch. If exactly one array matches this host,
				 * we can resolve on that one.
				 * Or, if we are auto assembling, we just ignore the second
				 * for now.
				 */
				if (auto_assem)
					goto loop;
				if (c->homehost) {
					int first = st->ss->match_home(st, c->homehost);
					int last = tst->ss->match_home(tst, c->homehost);
					if (first != last &&
					    (first == 1 || last == 1)) {
						/* We can do something */
						if (first) {/* just ignore this one */
							if (report_mismatch)
								pr_err("%s misses out due to wrong homehost\n",
								       devname);
							goto loop;
						} else { /* reject all those sofar */
							struct mddev_dev *td;
							if (report_mismatch)
								pr_err("%s overrides previous devices due to good homehost\n",
								       devname);
							for (td=devlist; td != tmpdev; td=td->next)
								if (td->used == 1)
									td->used = 0;
							tmpdev->used = 1;
							goto loop;
						}
					}
				}
				pr_err("superblock on %s doesn't match others - assembly aborted\n",
				       devname);
				tst->ss->free_super(tst);
				st->ss->free_super(st);
				dev_policy_free(pol);
				domain_free(domains);
				return -1;
			}
			tmpdev->used = 1;
		}
	loop:
		/* Collect domain information from members only */
		if (tmpdev && tmpdev->used == 1) {
			if (!pol)
				pol = devid_policy(rdev);
			domain_merge(&domains, pol, tst?tst->ss->name:NULL);
		}
		dev_policy_free(pol);
		pol = NULL;
		if (tst)
			tst->ss->free_super(tst);
	}

	/* Check if we found some imsm spares but no members */
	if ((auto_assem ||
	     (ident->uuid_set &&
	      memcmp(uuid_zero, ident->uuid,sizeof(uuid_zero)) == 0)) &&
	    (!st || !st->sb))
		for (tmpdev = devlist; tmpdev; tmpdev = tmpdev->next) {
			if (tmpdev->used != 3)
				continue;
			tmpdev->used = 1;
			content = *contentp;

			if (!st->sb) {
				/* we need sb from one of the spares */
				int dfd = dev_open(tmpdev->devname, O_RDONLY);
				if (dfd < 0 ||
				    st->ss->load_super(st, dfd, NULL))
					tmpdev->used = 2;
				if (dfd > 0)
					close(dfd);
			}
		}

	/* Now reject spares that don't match domains of identified members */
	for (tmpdev = devlist; tmpdev; tmpdev = tmpdev->next) {
		if (tmpdev->used != 3)
			continue;
		if (!stat_is_blkdev(tmpdev->devname, &rdev)) {
			tmpdev->used = 2;
		} else {
			struct dev_policy *pol = devid_policy(rdev);
			int dt = domain_test(domains, pol, NULL);
			if (inargv && dt != 0)
				/* take this spare as domains match
				 * if there are any */
				tmpdev->used = 1;
			else if (!inargv && dt == 1)
				/* device wasn't explicitly listed, so need
				 * explicit domain match - which we have */
				tmpdev->used = 1;
			else
				/* if domains don't match mark as unused */
				tmpdev->used = 0;
			dev_policy_free(pol);
		}
	}
	domain_free(domains);
	*stp = st;
	if (st && st->sb && content == *contentp)
		st->ss->getinfo_super(st, content, NULL);
	*contentp = content;

	return num_devs;
}

struct devs {
	char *devname;
	int uptodate; /* set once we decide that this device is as
		       * recent as everything else in the array.
		       */
	int included; /* set if the device is already in the array
		       * due to a previous '-I'
		       */
	struct mdinfo i;
};

static int load_devices(struct devs *devices, char *devmap,
			struct mddev_ident *ident, struct supertype **stp,
			struct mddev_dev *devlist, struct context *c,
			struct mdinfo *content,
			int mdfd, char *mddev,
			int *most_recentp, int *bestcntp, int **bestp,
			int inargv)
{
	struct mddev_dev *tmpdev;
	int devcnt = 0;
	int nextspare = 0;
	int bitmap_done = 0;
	int most_recent = -1;
	int bestcnt = 0;
	int *best = *bestp;
	struct supertype *st = *stp;

	for (tmpdev = devlist; tmpdev; tmpdev=tmpdev->next) {
		char *devname = tmpdev->devname;
		struct stat stb;
		struct supertype *tst;
		int i;
		int dfd;

		if (tmpdev->used != 1)
			continue;
		/* looks like a good enough match to update the super block if needed */
		if (c->update) {
			/* prepare useful information in info structures */
			struct stat stb2;
			int err;
			fstat(mdfd, &stb2);

			if (strcmp(c->update, "uuid") == 0 && !ident->uuid_set)
				random_uuid((__u8 *)ident->uuid);

			if (strcmp(c->update, "ppl") == 0 &&
			    ident->bitmap_fd >= 0) {
				pr_err("PPL is not compatible with bitmap\n");
				close(mdfd);
				free(devices);
				free(devmap);
				return -1;
			}

			dfd = dev_open(devname,
				       tmpdev->disposition == 'I'
				       ? O_RDWR : (O_RDWR|O_EXCL));

			tst = dup_super(st);
			if (dfd < 0 || tst->ss->load_super(tst, dfd, NULL) != 0) {
				pr_err("cannot re-read metadata from %s - aborting\n",
				       devname);
				if (dfd >= 0)
					close(dfd);
				close(mdfd);
				free(devices);
				free(devmap);
				tst->ss->free_super(tst);
				free(tst);
				*stp = st;
				return -1;
			}
			tst->ss->getinfo_super(tst, content, devmap + devcnt * content->array.raid_disks);

			memcpy(content->uuid, ident->uuid, 16);
			strcpy(content->name, ident->name);
			content->array.md_minor = minor(stb2.st_rdev);

			if (strcmp(c->update, "byteorder") == 0)
				err = 0;
			else if (strcmp(c->update, "home-cluster") == 0) {
				tst->cluster_name = c->homecluster;
				err = tst->ss->write_bitmap(tst, dfd, NameUpdate);
			} else if (strcmp(c->update, "nodes") == 0) {
				tst->nodes = c->nodes;
				err = tst->ss->write_bitmap(tst, dfd, NodeNumUpdate);
			} else if (strcmp(c->update, "revert-reshape") == 0 &&
				   c->invalid_backup)
				err = tst->ss->update_super(tst, content,
							    "revert-reshape-nobackup",
							    devname, c->verbose,
							    ident->uuid_set,
							    c->homehost);
			else
				err = tst->ss->update_super(tst, content, c->update,
							    devname, c->verbose,
							    ident->uuid_set,
							    c->homehost);
			if (err < 0) {
				if (err == -1)
					pr_err("--update=%s not understood for %s metadata\n",
					       c->update, tst->ss->name);
				tst->ss->free_super(tst);
				free(tst);
				close(mdfd);
				close(dfd);
				free(devices);
				free(devmap);
				*stp = st;
				return -1;
			}
			if (strcmp(c->update, "uuid")==0 &&
			    !ident->uuid_set) {
				ident->uuid_set = 1;
				memcpy(ident->uuid, content->uuid, 16);
			}
			if (tst->ss->store_super(tst, dfd))
				pr_err("Could not re-write superblock on %s.\n",
				       devname);

			if (strcmp(c->update, "uuid")==0 &&
			    ident->bitmap_fd >= 0 && !bitmap_done) {
				if (bitmap_update_uuid(ident->bitmap_fd,
						       content->uuid,
						       tst->ss->swapuuid) != 0)
					pr_err("Could not update uuid on external bitmap.\n");
				else
					bitmap_done = 1;
			}
		} else {
			dfd = dev_open(devname,
				       tmpdev->disposition == 'I'
				       ? O_RDWR : (O_RDWR|O_EXCL));
			tst = dup_super(st);

			if (dfd < 0 || tst->ss->load_super(tst, dfd, NULL) != 0) {
				pr_err("cannot re-read metadata from %s - aborting\n",
				       devname);
				if (dfd >= 0)
					close(dfd);
				close(mdfd);
				free(devices);
				free(devmap);
				tst->ss->free_super(tst);
				free(tst);
				*stp = st;
				return -1;
			}
			tst->ss->getinfo_super(tst, content, devmap + devcnt * content->array.raid_disks);
		}

		fstat(dfd, &stb);
		close(dfd);

		if (c->verbose > 0)
			pr_err("%s is identified as a member of %s, slot %d%s.\n",
			       devname, mddev, content->disk.raid_disk,
			       (content->disk.state & (1<<MD_DISK_REPLACEMENT)) ? " replacement":"");
		devices[devcnt].devname = devname;
		devices[devcnt].uptodate = 0;
		devices[devcnt].included = (tmpdev->disposition == 'I');
		devices[devcnt].i = *content;
		devices[devcnt].i.disk.major = major(stb.st_rdev);
		devices[devcnt].i.disk.minor = minor(stb.st_rdev);

		if (devices[devcnt].i.disk.state == 6) {
			if (most_recent < 0 ||
			    devices[devcnt].i.events
			    > devices[most_recent].i.events) {
				struct supertype *tmp = tst;
				tst = st;
				st = tmp;
				most_recent = devcnt;
			}
		}
		tst->ss->free_super(tst);
		free(tst);

		if (content->array.level == LEVEL_MULTIPATH)
			/* with multipath, the raid_disk from the superblock is meaningless */
			i = devcnt;
		else
			i = devices[devcnt].i.disk.raid_disk;
		if (i+1 == 0 || i == MD_DISK_ROLE_JOURNAL) {
			if (nextspare < content->array.raid_disks*2)
				nextspare = content->array.raid_disks*2;
			i = nextspare++;
		} else {
			/* i is raid_disk - double it so there is room for
			 * replacements */
			i *= 2;
			if (devices[devcnt].i.disk.state & (1<<MD_DISK_REPLACEMENT))
				i++;
			if (i >= content->array.raid_disks*2 &&
			    i >= nextspare)
				nextspare = i+1;
		}
		if (i < 10000) {
			if (i >= bestcnt) {
				int newbestcnt = i+10;
				int *newbest = xmalloc(sizeof(int)*newbestcnt);
				int c;
				for (c=0; c < newbestcnt; c++)
					if (c < bestcnt)
						newbest[c] = best[c];
					else
						newbest[c] = -1;
				if (best)free(best);
				best = newbest;
				bestcnt = newbestcnt;
			}
			if (best[i] >=0 &&
			    devices[best[i]].i.events ==
			    devices[devcnt].i.events &&
			    (devices[best[i]].i.disk.minor !=
			     devices[devcnt].i.disk.minor) &&
			    st->ss == &super0 &&
			    content->array.level != LEVEL_MULTIPATH) {
				/* two different devices with identical superblock.
				 * Could be a mis-detection caused by overlapping
				 * partitions.  fail-safe.
				 */
				pr_err("WARNING %s and %s appear to have very similar superblocks.\n"
				       "      If they are really different, please --zero the superblock on one\n"
				       "      If they are the same or overlap, please remove one from %s.\n",
				       devices[best[i]].devname, devname,
				       inargv ? "the list" :
				       "the\n      DEVICE list in mdadm.conf"
					);
				close(mdfd);
				free(devices);
				free(devmap);
				*stp = st;
				return -1;
			}
			if (best[i] == -1 || (devices[best[i]].i.events
					      < devices[devcnt].i.events))
				best[i] = devcnt;
			else if (st->ss == &super_imsm)
				best[i+1] = devcnt;
		}
		devcnt++;
	}
	if (most_recent >= 0)
		*most_recentp = most_recent;
	*bestcntp = bestcnt;
	*bestp = best;
	*stp = st;
	return devcnt;
}

static int force_array(struct mdinfo *content,
		       struct devs *devices,
		       int *best, int bestcnt, char *avail,
		       int most_recent,
		       struct supertype *st,
		       struct context *c)
{
	int okcnt = 0;
	while (!enough(content->array.level, content->array.raid_disks,
		       content->array.layout, 1,
		       avail) ||
	       (content->reshape_active && content->delta_disks > 0 &&
		!enough(content->array.level, (content->array.raid_disks
					       - content->delta_disks),
			content->new_layout, 1, avail))) {
		/* Choose the newest best drive which is
		 * not up-to-date, update the superblock
		 * and add it.
		 */
		int fd;
		struct supertype *tst;
		unsigned long long current_events;
		int chosen_drive = -1;
		int i;

		for (i = 0;
		     i < content->array.raid_disks * 2 && i < bestcnt;
		     i += 2) {
			int j = best[i];
			if (j < 0)
				continue;
			if (devices[j].uptodate)
				continue;
			if (devices[j].i.recovery_start != MaxSector) {
				int delta;
				if (!devices[j].i.reshape_active ||
				    devices[j].i.delta_disks <= 0)
					continue;
				/* When increasing number of devices, an
				 * added device also appears to be
				 * recovering.  It is safe to include it
				 * as long as it won't be a source of
				 * data.
				 * For now, just allow for last data
				 * devices in RAID4 or last devices in RAID4/5/6.
				 */
				delta = devices[j].i.delta_disks;
				if (devices[j].i.array.level >= 4 &&
				    devices[j].i.array.level <= 6 &&
				    i/2 >= content->array.raid_disks - delta)
					/* OK */;
				else if (devices[j].i.array.level == 4 &&
					 i/2 >= content->array.raid_disks - delta - 1)
					/* OK */;
				else
					continue;
			} else if (devices[j].i.reshape_active !=
			    content->reshape_active ||
			    (devices[j].i.reshape_active &&
			    devices[j].i.reshape_progress !=
			    content->reshape_progress))
				/* Here, it may be a source of data. If two
				 * devices claim different progresses, it
				 * means that reshape boundaries differ for
				 * their own devices. Kernel will only treat
				 * the first one as reshape progress and
				 * go on. It may cause disaster, so avoid it.
				 */
				continue;
			if (chosen_drive < 0 ||
			     devices[j].i.events
			    > devices[chosen_drive].i.events)
				chosen_drive = j;
		}
		if (chosen_drive < 0)
			break;
		current_events = devices[chosen_drive].i.events;
	add_another:
		if (c->verbose >= 0)
			pr_err("forcing event count in %s(%d) from %d upto %d\n",
			       devices[chosen_drive].devname,
			       devices[chosen_drive].i.disk.raid_disk,
			       (int)(devices[chosen_drive].i.events),
			       (int)(devices[most_recent].i.events));
		fd = dev_open(devices[chosen_drive].devname,
			      devices[chosen_drive].included ? O_RDWR
			      : (O_RDWR|O_EXCL));
		if (fd < 0) {
			pr_err("Couldn't open %s for write - not updating\n",
			       devices[chosen_drive].devname);
			devices[chosen_drive].i.events = 0;
			continue;
		}
		tst = dup_super(st);
		if (tst->ss->load_super(tst,fd, NULL)) {
			close(fd);
			pr_err("RAID superblock disappeared from %s - not updating.\n",
			       devices[chosen_drive].devname);
			devices[chosen_drive].i.events = 0;
			continue;
		}
		content->events = devices[most_recent].i.events;
		tst->ss->update_super(tst, content, "force-one",
				      devices[chosen_drive].devname, c->verbose,
				      0, NULL);

		if (tst->ss->store_super(tst, fd)) {
			close(fd);
			pr_err("Could not re-write superblock on %s\n",
			       devices[chosen_drive].devname);
			devices[chosen_drive].i.events = 0;
			tst->ss->free_super(tst);
			continue;
		}
		close(fd);
		devices[chosen_drive].i.events = devices[most_recent].i.events;
		devices[chosen_drive].uptodate = 1;
		avail[chosen_drive] = 1;
		okcnt++;
		tst->ss->free_super(tst);
		/* If there are any other drives of the same vintage,
		 * add them in as well.  We can't lose and we might gain
		 */
		for (i = 0;
		     i < content->array.raid_disks * 2 && i < bestcnt ;
		     i += 2) {
			int j = best[i];
			if (j >= 0 &&
			    !devices[j].uptodate &&
			    devices[j].i.recovery_start == MaxSector &&
			    devices[j].i.events == current_events &&
			    ((!devices[j].i.reshape_active &&
			    !content->reshape_active) ||
			    (devices[j].i.reshape_active ==
			    content->reshape_active &&
			    devices[j].i.reshape_progress ==
			    content->reshape_progress))) {
				chosen_drive = j;
				goto add_another;
			}
		}
	}
	return okcnt;
}

static int start_array(int mdfd,
		       char *mddev,
		       struct mdinfo *content,
		       struct supertype *st,
		       struct mddev_ident *ident,
		       int *best, int bestcnt,
		       int chosen_drive,
		       struct devs *devices,
		       unsigned int okcnt,
		       unsigned int sparecnt,
		       unsigned int rebuilding_cnt,
		       unsigned int journalcnt,
		       struct context *c,
		       int clean, char *avail,
		       int start_partial_ok,
		       int err_ok,
		       int was_forced
	)
{
	int rv;
	int i;
	unsigned int req_cnt;

	if (content->journal_device_required && (content->journal_clean == 0)) {
		if (!c->force) {
			pr_err("Not safe to assemble with missing or stale journal device, consider --force.\n");
			return 1;
		}
		pr_err("Journal is missing or stale, starting array read only.\n");
		c->readonly = 1;
	}

	if (content->consistency_policy == CONSISTENCY_POLICY_PPL)
		clean = 1;

	rv = set_array_info(mdfd, st, content);
	if (rv && !err_ok) {
		pr_err("failed to set array info for %s: %s\n",
		       mddev, strerror(errno));
		return 1;
	}
	if (ident->bitmap_fd >= 0) {
		if (ioctl(mdfd, SET_BITMAP_FILE, ident->bitmap_fd) != 0) {
			pr_err("SET_BITMAP_FILE failed.\n");
			return 1;
		}
	} else if (ident->bitmap_file) {
		/* From config file */
		int bmfd = open(ident->bitmap_file, O_RDWR);
		if (bmfd < 0) {
			pr_err("Could not open bitmap file %s\n",
			       ident->bitmap_file);
			return 1;
		}
		if (ioctl(mdfd, SET_BITMAP_FILE, bmfd) != 0) {
			pr_err("Failed to set bitmapfile for %s\n", mddev);
			close(bmfd);
			return 1;
		}
		close(bmfd);
	}

	/* First, add the raid disks, but add the chosen one last */
	for (i = 0; i <= bestcnt; i++) {
		int j;
		if (i < bestcnt) {
			j = best[i];
			if (j == chosen_drive)
				continue;
		} else
			j = chosen_drive;

		if (j >= 0 && !devices[j].included) {
			int dfd;

			dfd = dev_open(devices[j].devname, O_RDWR|O_EXCL);
			if (dfd >= 0) {
				remove_partitions(dfd);
				close(dfd);
			}
			rv = add_disk(mdfd, st, content, &devices[j].i);

			if (rv) {
				pr_err("failed to add %s to %s: %s\n",
				       devices[j].devname, mddev,
				       strerror(errno));
				if (i < content->array.raid_disks * 2 ||
				    i == bestcnt)
					okcnt--;
				else
					sparecnt--;
			} else if (c->verbose > 0) {
				pr_err("added %s to %s as %d%s%s\n",
				       devices[j].devname, mddev,
				       devices[j].i.disk.raid_disk,
				       devices[j].uptodate?"":
				       " (possibly out of date)",
				       (devices[j].i.disk.state &
					(1<<MD_DISK_REPLACEMENT)) ?
				       " replacement":"");
			}
		} else if (j >= 0) {
			if (c->verbose > 0)
				pr_err("%s is already in %s as %d\n",
				       devices[j].devname, mddev,
				       devices[j].i.disk.raid_disk);
		} else if (c->verbose > 0 &&
			   i < content->array.raid_disks * 2 && (i & 1) == 0)
			pr_err("no uptodate device for slot %d of %s\n",
			       i/2, mddev);
	}

	if (content->array.level == LEVEL_CONTAINER) {
		if (c->verbose >= 0) {
			pr_err("Container %s has been assembled with %d drive%s",
			       mddev, okcnt + sparecnt + journalcnt,
			       okcnt + sparecnt + journalcnt == 1 ? "" : "s");
			if (okcnt < (unsigned)content->array.raid_disks)
				fprintf(stderr, " (out of %d)",
					content->array.raid_disks);
			fprintf(stderr, "\n");
		}

		if (st->ss->validate_container) {
			struct mdinfo *devices_list;
			struct mdinfo *info_devices;
			unsigned int count;

			devices_list = NULL;
			info_devices = xmalloc(sizeof(struct mdinfo) *
					       (okcnt + sparecnt));
			for (count = 0; count < okcnt + sparecnt; count++) {
				info_devices[count] = devices[count].i;
				info_devices[count].next = devices_list;
				devices_list = &info_devices[count];
			}
			if (st->ss->validate_container(devices_list))
				pr_err("Mismatch detected!\n");
			free(info_devices);
		}

		st->ss->free_super(st);
		sysfs_uevent(content, "change");
		if (err_ok && okcnt < (unsigned)content->array.raid_disks)
			/* Was partial, is still partial, so signal an error
			 * to ensure we don't retry */
			return 1;
		return 0;
	}

	/* Get number of in-sync devices according to the superblock.
	 * We must have this number to start the array without -s or -R
	 */
	req_cnt = content->array.working_disks;

	if (c->runstop == 1 ||
	    (c->runstop <= 0 &&
	     (enough(content->array.level, content->array.raid_disks,
		     content->array.layout, clean, avail) &&
	       (okcnt + rebuilding_cnt >= req_cnt || start_partial_ok)))) {
		/* This array is good-to-go.
		 * If a reshape is in progress then we might need to
		 * continue monitoring it.  In that case we start
		 * it read-only and let the grow code make it writable.
		 */
		int rv;

		if (content->reshape_active &&
		    !(content->reshape_active & RESHAPE_NO_BACKUP) &&
		    content->delta_disks <= 0) {
			if (!c->backup_file) {
				pr_err("%s: Need a backup file to complete reshape of this array.\n",
				       mddev);
				pr_err("Please provided one with \"--backup-file=...\"\n");
				if (c->update &&
				    strcmp(c->update, "revert-reshape") == 0)
					pr_err("(Don't specify --update=revert-reshape again, that part succeeded.)\n");
				return 1;
			}
			rv = sysfs_set_str(content, NULL,
					   "array_state", "readonly");
			if (rv == 0)
				rv = Grow_continue(mdfd, st, content,
						   c->backup_file, 0,
						   c->freeze_reshape);
		} else if (c->readonly &&
			   sysfs_attribute_available(content, NULL,
						     "array_state")) {
			rv = sysfs_set_str(content, NULL,
					   "array_state", "readonly");
		} else
			rv = ioctl(mdfd, RUN_ARRAY, NULL);
		reopen_mddev(mdfd); /* drop O_EXCL */
		if (rv == 0) {
			if (c->verbose >= 0) {
				pr_err("%s has been started with %d drive%s",
				       mddev, okcnt, okcnt==1?"":"s");
				if (okcnt < (unsigned)content->array.raid_disks)
					fprintf(stderr, " (out of %d)",
						content->array.raid_disks);
				if (rebuilding_cnt)
					fprintf(stderr, "%s %d rebuilding",
						sparecnt?",":" and",
						rebuilding_cnt);
				if (sparecnt)
					fprintf(stderr, " and %d spare%s",
						sparecnt,
						sparecnt == 1 ? "" : "s");
				if (content->journal_clean)
					fprintf(stderr, " and %d journal",
						journalcnt);
				fprintf(stderr, ".\n");
			}
			if (content->reshape_active &&
			    content->array.level >= 4 &&
			    content->array.level <= 6) {
				/* might need to increase the size
				 * of the stripe cache - default is 256
				 */
				int chunk_size = content->array.chunk_size;

				if (content->reshape_active &&
				    content->new_chunk > chunk_size)
					chunk_size = content->new_chunk;
				if (256 < 4 * ((chunk_size+4065)/4096)) {
					struct mdinfo *sra;

					sra = sysfs_read(mdfd, NULL, 0);
					if (sra)
						sysfs_set_num(sra, NULL,
							      "stripe_cache_size",
							      (4 * chunk_size / 4096) + 1);
					sysfs_free(sra);
				}
			}
			if (okcnt < (unsigned)content->array.raid_disks) {
				/* If any devices did not get added
				 * because the kernel rejected them based
				 * on event count, try adding them
				 * again providing the action policy is
				 * 're-add' or greater.  The bitmap
				 * might allow them to be included, or
				 * they will become spares.
				 */
				for (i = 0; i < bestcnt; i++) {
					int j = best[i];
					if (j >= 0 && !devices[j].uptodate) {
						if (!disk_action_allows(&devices[j].i, st->ss->name, act_re_add))
							continue;
						rv = add_disk(mdfd, st, content,
							      &devices[j].i);
						if (rv == 0 && c->verbose >= 0)
							pr_err("%s has been re-added.\n",
							       devices[j].devname);
					}
				}
			}
			if (content->array.level == 6 &&
			    okcnt + 1 == (unsigned)content->array.raid_disks &&
			    was_forced) {
				struct mdinfo *sra;

				sra = sysfs_read(mdfd, NULL, 0);
				if (sra)
					sysfs_set_str(sra, NULL,
						      "sync_action", "repair");
				sysfs_free(sra);
			}
			return 0;
		}
		pr_err("failed to RUN_ARRAY %s: %s\n", mddev, strerror(errno));

		if (!enough(content->array.level, content->array.raid_disks,
			    content->array.layout, 1, avail))
			pr_err("Not enough devices to start the array.\n");
		else if (!enough(content->array.level,
				 content->array.raid_disks,
				 content->array.layout, clean, avail))
			pr_err("Not enough devices to start the array while not clean - consider --force.\n");

		return 1;
	}
	if (c->runstop == -1) {
		pr_err("%s assembled from %d drive%s",
		       mddev, okcnt, okcnt == 1 ? "" : "s");
		if (okcnt != (unsigned)content->array.raid_disks)
			fprintf(stderr, " (out of %d)",
				content->array.raid_disks);
		fprintf(stderr, ", but not started.\n");
		return 2;
	}
	if (c->verbose >= -1) {
		pr_err("%s assembled from %d drive%s",
		       mddev, okcnt, okcnt == 1 ? "" : "s");
		if (rebuilding_cnt)
			fprintf(stderr, "%s %d rebuilding",
				sparecnt ? "," : " and", rebuilding_cnt);
		if (sparecnt)
			fprintf(stderr, " and %d spare%s", sparecnt,
				sparecnt == 1 ? "" : "s");
		if (!enough(content->array.level, content->array.raid_disks,
			    content->array.layout, 1, avail))
			fprintf(stderr, " - not enough to start the array.\n");
		else if (!enough(content->array.level,
				 content->array.raid_disks,
				 content->array.layout, clean, avail))
			fprintf(stderr, " - not enough to start the array while not clean - consider --force.\n");
		else {
			if (req_cnt == (unsigned)content->array.raid_disks)
				fprintf(stderr, " - need all %d to start it",
					req_cnt);
			else
				fprintf(stderr, " - need %d to start", req_cnt);
			fprintf(stderr, " (use --run to insist).\n");
		}
	}
	return 1;
}

int Assemble(struct supertype *st, char *mddev,
	     struct mddev_ident *ident,
	     struct mddev_dev *devlist,
	     struct context *c)
{
	/*
	 * The task of Assemble is to find a collection of
	 * devices that should (according to their superblocks)
	 * form an array, and to give this collection to the MD driver.
	 * In Linux-2.4 and later, this involves submitting a
	 * SET_ARRAY_INFO ioctl with no arg - to prepare
	 * the array - and then submit a number of
	 * ADD_NEW_DISK ioctls to add disks into
	 * the array.  Finally RUN_ARRAY might
	 * be submitted to start the array.
	 *
	 * Much of the work of Assemble is in finding and/or
	 * checking the disks to make sure they look right.
	 *
	 * If mddev is not set, then scan must be set and we
	 *  read through the config file for dev+uuid mapping
	 *  We recurse, setting mddev, for each device that
	 *    - isn't running
	 *    - has a valid uuid (or any uuid if !uuidset)
	 *
	 * If mddev is set, we try to determine state of md.
	 *   check version - must be at least 0.90.0
	 *   check kernel version.  must be at least 2.4.
	 *    If not, we can possibly fall back on START_ARRAY
	 *   Try to GET_ARRAY_INFO.
	 *     If possible, give up
	 *     If not, try to STOP_ARRAY just to make sure
	 *
	 * If !uuidset and scan, look in conf-file for uuid
	 *       If not found, give up
	 * If !devlist and scan and uuidset, get list of devs from conf-file
	 *
	 * For each device:
	 *   Check superblock - discard if bad
	 *   Check uuid (set if we don't have one) - discard if no match
	 *   Check superblock similarity if we have a superblock - discard if different
	 *   Record events, devicenum
	 * This should give us a list of devices for the array
	 * We should collect the most recent event number
	 *
	 * Count disks with recent enough event count
	 * While force && !enough disks
	 *    Choose newest rejected disks, update event count
	 *     mark clean and rewrite superblock
	 * If recent kernel:
	 *    SET_ARRAY_INFO
	 *    foreach device with recent events : ADD_NEW_DISK
	 *    if runstop == 1 || "enough" disks and runstop==0 -> RUN_ARRAY
	 * If old kernel:
	 *    Check the device numbers in superblock are right
	 *    update superblock if any changes
	 *    START_ARRAY
	 *
	 */
	int rv = -1;
	int mdfd = -1;
	int clean;
	int auto_assem = (mddev == NULL && !ident->uuid_set &&
			  ident->super_minor == UnSet && ident->name[0] == 0 &&
			  (ident->container == NULL || ident->member == NULL));
	struct devs *devices = NULL;
	char *devmap;
	int *best = NULL; /* indexed by raid_disk */
	int bestcnt = 0;
	int devcnt;
	unsigned int okcnt, sparecnt, rebuilding_cnt, replcnt, journalcnt;
	int journal_clean = 0;
	int i;
	int was_forced = 0;
	int most_recent = 0;
	int chosen_drive;
	int change = 0;
	int inargv = 0;
	int start_partial_ok = (c->runstop >= 0) &&
		(c->force || devlist==NULL || auto_assem);
	int num_devs;
	struct mddev_dev *tmpdev;
	struct mdinfo info;
	struct mdinfo *content = NULL;
	struct mdinfo *pre_exist = NULL;
	char *avail;
	char *name = NULL;
	char chosen_name[1024];
	struct map_ent *map = NULL;
	struct map_ent *mp;

	/*
	 * If any subdevs are listed, then any that don't
	 * match ident are discarded.  Remainder must all match and
	 * become the array.
	 * If no subdevs, then we scan all devices in the config file, but
	 * there must be something in the identity
	 */

	if (!devlist &&
	    ident->uuid_set == 0 &&
	    (ident->super_minor < 0 || ident->super_minor == UnSet) &&
	    ident->name[0] == 0 &&
	    (ident->container == NULL || ident->member == NULL) &&
	    ident->devices == NULL) {
		pr_err("No identity information available for %s - cannot assemble.\n",
		       mddev ? mddev : "further assembly");
		return 1;
	}

	if (devlist == NULL)
		devlist = conf_get_devs();
	else if (mddev)
		inargv = 1;

try_again:
	/* We come back here when doing auto-assembly and attempting some
	 * set of devices failed.  Those are now marked as ->used==2 and
	 * we ignore them and try again
	 */
	if (!st && ident->st)
		st = ident->st;
	if (c->verbose>0)
		pr_err("looking for devices for %s\n",
		       mddev ? mddev : "further assembly");

	content = &info;
	if (st && c->force)
		st->ignore_hw_compat = 1;
	num_devs = select_devices(devlist, ident, &st, &content, c,
				  inargv, auto_assem);
	if (num_devs < 0)
		return 1;

	if (!st || !st->sb || !content)
		return 2;

	/* We have a full set of devices - we now need to find the
	 * array device.
	 * However there is a risk that we are racing with "mdadm -I"
	 * and the array is already partially assembled - we will have
	 * rejected any devices already in this address.
	 * So we take a lock on the map file - to prevent further races -
	 * and look for the uuid in there.  If found and the array is
	 * active, we abort.  If found and the array is not active
	 * we commit to that md device and add all the contained devices
	 * to our list.  We flag them so that we don't try to re-add,
	 * but can remove if they turn out to not be wanted.
	 */
	if (map_lock(&map))
		pr_err("failed to get exclusive lock on mapfile - continue anyway...\n");
	if (c->update && strcmp(c->update,"uuid") == 0)
		mp = NULL;
	else
		mp = map_by_uuid(&map, content->uuid);
	if (mp) {
		struct mdinfo *dv;
		/* array already exists. */
		pre_exist = sysfs_read(-1, mp->devnm, GET_LEVEL|GET_DEVS);
		if (pre_exist->array.level != UnSet) {
			pr_err("Found some drive for an array that is already active: %s\n",
			       mp->path);
			pr_err("giving up.\n");
			goto out;
		}
		for (dv = pre_exist->devs; dv; dv = dv->next) {
			/* We want to add this device to our list,
			 * but it could already be there if "mdadm -I"
			 * started *after* we checked for O_EXCL.
			 * If we add it to the top of the list
			 * it will be preferred over later copies.
			 */
			struct mddev_dev *newdev;
			char *devname = map_dev(dv->disk.major,
						dv->disk.minor,
						0);
			if (!devname)
				continue;
			newdev = xmalloc(sizeof(*newdev));
			newdev->devname = devname;
			newdev->disposition = 'I';
			newdev->used = 1;
			newdev->next = devlist;
			devlist = newdev;
			num_devs++;
		}
		strcpy(chosen_name, mp->path);
		if (c->verbose > 0 || mddev == NULL ||
		    strcmp(mddev, chosen_name) != 0)
			pr_err("Merging with already-assembled %s\n",
			       chosen_name);
		mdfd = open_dev_excl(mp->devnm);
	} else {
		int trustworthy = FOREIGN;
		name = content->name;
		switch (st->ss->match_home(st, c->homehost)
			?: st->ss->match_home(st, "any")) {
		case 1:
			trustworthy = LOCAL;
			name = strchr(content->name, ':');
			if (name)
				name++;
			else
				name = content->name;
			break;
		}
		if (!auto_assem)
			/* If the array is listed in mdadm.conf or on
			 * command line, then we trust the name
			 * even if the array doesn't look local
			 */
			trustworthy = LOCAL;

		if (name[0] == 0 &&
		    content->array.level == LEVEL_CONTAINER) {
			name = content->text_version;
			trustworthy = METADATA;
		}

		if (name[0] && trustworthy != LOCAL &&
		    ! c->require_homehost &&
		    conf_name_is_free(name))
			trustworthy = LOCAL;

		if (trustworthy == LOCAL &&
		    strchr(name, ':'))
			/* Ignore 'host:' prefix of name */
			name = strchr(name, ':')+1;

		mdfd = create_mddev(mddev, name, ident->autof, trustworthy,
				    chosen_name, 0);
	}
	if (mdfd < 0) {
		st->ss->free_super(st);
		if (auto_assem)
			goto try_again;
		goto out;
	}
	mddev = chosen_name;
	if (pre_exist == NULL) {
		if (mddev_busy(fd2devnm(mdfd))) {
			pr_err("%s already active, cannot restart it!\n",
			       mddev);
			for (tmpdev = devlist ;
			     tmpdev && tmpdev->used != 1;
			     tmpdev = tmpdev->next)
				;
			if (tmpdev && auto_assem)
				pr_err("%s needed for %s...\n",
				       mddev, tmpdev->devname);
			close(mdfd);
			mdfd = -3;
			st->ss->free_super(st);
			if (auto_assem)
				goto try_again;
			goto out;
		}
		/* just incase it was started but has no content */
		ioctl(mdfd, STOP_ARRAY, NULL);
	}

	if (content != &info) {
		/* This is a member of a container.  Try starting the array. */
		int err;
		err = assemble_container_content(st, mdfd, content, c,
						 chosen_name, NULL);
		close(mdfd);
		return err;
	}

	/* Ok, no bad inconsistancy, we can try updating etc */
	devices = xcalloc(num_devs, sizeof(*devices));
	devmap = xcalloc(num_devs, content->array.raid_disks);
	devcnt = load_devices(devices, devmap, ident, &st, devlist,
			      c, content, mdfd, mddev,
			      &most_recent, &bestcnt, &best, inargv);
	if (devcnt < 0) {
		mdfd = -3;
		/*
		 * devices is already freed in load_devices, so set devices
		 * to NULL to avoid double free devices.
		 */
		devices = NULL;
		goto out;
	}

	if (devcnt == 0) {
		pr_err("no devices found for %s\n",
		       mddev);
		if (st)
			st->ss->free_super(st);
		free(devmap);
		goto out;
	}

	if (c->update && strcmp(c->update, "byteorder")==0)
		st->minor_version = 90;

	st->ss->getinfo_super(st, content, NULL);
	clean = content->array.state & 1;

	/* now we have some devices that might be suitable.
	 * I wonder how many
	 */
	avail = xcalloc(content->array.raid_disks, 1);
	okcnt = 0;
	replcnt = 0;
	sparecnt=0;
	journalcnt=0;
	rebuilding_cnt=0;
	for (i=0; i< bestcnt; i++) {
		int j = best[i];
		int event_margin = 1; /* always allow a difference of '1'
				       * like the kernel does
				       */
		if (j < 0) continue;
		/* note: we ignore error flags in multipath arrays
		 * as they don't make sense
		 */
		if (content->array.level != LEVEL_MULTIPATH) {
			if (devices[j].i.disk.state & (1<<MD_DISK_JOURNAL)) {
				if (content->journal_device_required)
					journalcnt++;
				else	/* unexpected journal, mark as faulty */
					devices[j].i.disk.state |= (1<<MD_DISK_FAULTY);
			} else if (!(devices[j].i.disk.state & (1<<MD_DISK_ACTIVE))) {
				if (!(devices[j].i.disk.state
				      & (1<<MD_DISK_FAULTY))) {
					devices[j].uptodate = 1;
					sparecnt++;
				}
				continue;
			}
		}
		/* If this device thinks that 'most_recent' has failed, then
		 * we must reject this device.
		 */
		if (j != most_recent && !c->force &&
		    content->array.raid_disks > 0 &&
		    devices[most_recent].i.disk.raid_disk >= 0 &&
		    devmap[j * content->array.raid_disks + devices[most_recent].i.disk.raid_disk] == 0) {
			if (c->verbose > -1)
				pr_err("ignoring %s as it reports %s as failed\n",
				       devices[j].devname, devices[most_recent].devname);
			best[i] = -1;
			continue;
		}
		/* Require event counter to be same as, or just less than,
		 * most recent.  If it is bigger, it must be a stray spare and
		 * should be ignored.
		 */
		if (devices[j].i.events+event_margin >=
		    devices[most_recent].i.events &&
		    devices[j].i.events <=
		    devices[most_recent].i.events
			) {
			devices[j].uptodate = 1;
			if (devices[j].i.disk.state & (1<<MD_DISK_JOURNAL))
				journal_clean = 1;
			if (i < content->array.raid_disks * 2) {
				if (devices[j].i.recovery_start == MaxSector ||
				    (content->reshape_active &&
				     i >= content->array.raid_disks - content->delta_disks)) {
					if (!avail[i/2]) {
						okcnt++;
						avail[i/2]=1;
					} else
						replcnt++;
				} else
					rebuilding_cnt++;
			} else if (devices[j].i.disk.raid_disk != MD_DISK_ROLE_JOURNAL)
				sparecnt++;
		}
	}
	free(devmap);
	if (c->force) {
		int force_ok = force_array(content, devices, best, bestcnt,
					   avail, most_recent, st, c);
		okcnt += force_ok;
		if (force_ok)
			was_forced = 1;
	}
	/* Now we want to look at the superblock which the kernel will base things on
	 * and compare the devices that we think are working with the devices that the
	 * superblock thinks are working.
	 * If there are differences and --force is given, then update this chosen
	 * superblock.
	 */
	chosen_drive = -1;
	st->ss->free_super(st);
	for (i=0; chosen_drive < 0 && i<bestcnt; i+=2) {
		int j = best[i];
		int fd;

		if (j<0)
			continue;
		if (!devices[j].uptodate)
			continue;
		if (devices[j].i.events < devices[most_recent].i.events)
			continue;
		chosen_drive = j;
		if ((fd=dev_open(devices[j].devname,
				 devices[j].included ? O_RDONLY
				 : (O_RDONLY|O_EXCL)))< 0) {
			pr_err("Cannot open %s: %s\n",
			       devices[j].devname, strerror(errno));
			goto out;
		}
		if (st->ss->load_super(st,fd, NULL)) {
			close(fd);
			pr_err("RAID superblock has disappeared from %s\n",
			       devices[j].devname);
			goto out;
		}
		close(fd);
	}
	if (st->sb == NULL) {
		pr_err("No suitable drives found for %s\n", mddev);
		goto out;
	}
	st->ss->getinfo_super(st, content, NULL);
	if (sysfs_init(content, mdfd, NULL)) {
		pr_err("Unable to initialize sysfs\n");
		goto out;
	}

	/* after reload context, store journal_clean in context */
	content->journal_clean = journal_clean;
	for (i=0; i<bestcnt; i++) {
		int j = best[i];
		unsigned int desired_state;

		if (j < 0)
			continue;
		if (devices[j].i.disk.raid_disk == MD_DISK_ROLE_JOURNAL)
			desired_state = (1<<MD_DISK_JOURNAL);
		else if (i >= content->array.raid_disks * 2)
			desired_state = 0;
		else if (i & 1)
			desired_state = (1<<MD_DISK_ACTIVE) | (1<<MD_DISK_REPLACEMENT);
		else
			desired_state = (1<<MD_DISK_ACTIVE) | (1<<MD_DISK_SYNC);

		if (!devices[j].uptodate)
			continue;

		devices[j].i.disk.state = desired_state;
		if (!(devices[j].i.array.state & 1))
			clean = 0;

		if (st->ss->update_super(st, &devices[j].i, "assemble", NULL,
					 c->verbose, 0, NULL)) {
			if (c->force) {
				if (c->verbose >= 0)
					pr_err("clearing FAULTY flag for device %d in %s for %s\n",
					       j, mddev, devices[j].devname);
				change = 1;
			} else {
				if (c->verbose >= -1)
					pr_err("device %d in %s has wrong state in superblock, but %s seems ok\n",
					       i, mddev, devices[j].devname);
			}
		}
#if 0
		if (!(super.disks[i].i.disk.state & (1 << MD_DISK_FAULTY))) {
			pr_err("devices %d of %s is not marked FAULTY in superblock, but cannot be found\n",
			       i, mddev);
		}
#endif
	}
	if (c->force && !clean &&
	    !enough(content->array.level, content->array.raid_disks,
		    content->array.layout, clean,
		    avail)) {
		change += st->ss->update_super(st, content, "force-array",
					       devices[chosen_drive].devname, c->verbose,
					       0, NULL);
		was_forced = 1;
		clean = 1;
	}

	if (change) {
		int fd;
		fd = dev_open(devices[chosen_drive].devname,
			      devices[chosen_drive].included ?
			      O_RDWR : (O_RDWR|O_EXCL));
		if (fd < 0) {
			pr_err("Could not open %s for write - cannot Assemble array.\n",
			       devices[chosen_drive].devname);
			goto out;
		}
		if (st->ss->store_super(st, fd)) {
			close(fd);
			pr_err("Could not re-write superblock on %s\n",
			       devices[chosen_drive].devname);
			goto out;
		}
		if (c->verbose >= 0)
			pr_err("Marking array %s as 'clean'\n",
			       mddev);
		close(fd);
	}

	/* If we are in the middle of a reshape we may need to restore saved data
	 * that was moved aside due to the reshape overwriting live data
	 * The code of doing this lives in Grow.c
	 */
	if (content->reshape_active &&
	    !(content->reshape_active & RESHAPE_NO_BACKUP)) {
		int err = 0;
		int *fdlist = xmalloc(sizeof(int)* bestcnt);
		if (c->verbose > 0)
			pr_err("%s has an active reshape - checking if critical section needs to be restored\n",
			       chosen_name);
		if (!c->backup_file)
			c->backup_file = locate_backup(content->sys_name);
		enable_fds(bestcnt/2);
		for (i = 0; i < bestcnt/2; i++) {
			int j = best[i*2];
			if (j >= 0) {
				fdlist[i] = dev_open(devices[j].devname,
						     devices[j].included
						     ? O_RDWR : (O_RDWR|O_EXCL));
				if (fdlist[i] < 0) {
					pr_err("Could not open %s for write - cannot Assemble array.\n",
					       devices[j].devname);
					err = 1;
					break;
				}
			} else
				fdlist[i] = -1;
		}
		if (!err) {
			if (st->ss->external && st->ss->recover_backup)
				err = st->ss->recover_backup(st, content);
			else
				err = Grow_restart(st, content, fdlist, bestcnt/2,
						   c->backup_file, c->verbose > 0);
			if (err && c->invalid_backup) {
				if (c->verbose > 0)
					pr_err("continuing without restoring backup\n");
				err = 0;
			}
		}
		while (i>0) {
			i--;
			if (fdlist[i]>=0) close(fdlist[i]);
		}
		free(fdlist);
		if (err) {
			pr_err("Failed to restore critical section for reshape, sorry.\n");
			if (c->backup_file == NULL)
				cont_err("Possibly you needed to specify the --backup-file\n");
			goto out;
		}
	}

	/* Almost ready to actually *do* something */
	/* First, fill in the map, so that udev can find our name
	 * as soon as we become active.
	 */
	if (c->update && strcmp(c->update, "metadata")==0) {
		content->array.major_version = 1;
		content->array.minor_version = 0;
		strcpy(content->text_version, "1.0");
	}

	map_update(&map, fd2devnm(mdfd), content->text_version,
		   content->uuid, chosen_name);

	rv = start_array(mdfd, mddev, content,
			 st, ident, best, bestcnt,
			 chosen_drive, devices, okcnt, sparecnt,
			 rebuilding_cnt, journalcnt,
			 c,
			 clean, avail, start_partial_ok,
			 pre_exist != NULL,
			 was_forced);
	if (rv == 1 && !pre_exist)
		ioctl(mdfd, STOP_ARRAY, NULL);
	free(devices);
out:
	map_unlock(&map);
	if (rv == 0) {
		wait_for(chosen_name, mdfd);
		close(mdfd);
		if (auto_assem) {
			int usecs = 1;
			/* There is a nasty race with 'mdadm --monitor'.
			 * If it opens this device before we close it,
			 * it gets an incomplete open on which IO
			 * doesn't work and the capacity is
			 * wrong.
			 * If we reopen (to check for layered devices)
			 * before --monitor closes, we loose.
			 *
			 * So: wait upto 1 second for there to be
			 * a non-zero capacity.
			 */
			while (usecs < 1000) {
				mdfd = open(mddev, O_RDONLY);
				if (mdfd >= 0) {
					unsigned long long size;
					if (get_dev_size(mdfd, NULL, &size) &&
					    size > 0)
						break;
					close(mdfd);
				}
				usleep(usecs);
				usecs <<= 1;
			}
		}
	} else if (mdfd >= 0)
		close(mdfd);

	/* '2' means 'OK, but not started yet' */
	if (rv == -1) {
		free(devices);
		return 1;
	}
	return rv == 2 ? 0 : rv;
}

int assemble_container_content(struct supertype *st, int mdfd,
			       struct mdinfo *content, struct context *c,
			       char *chosen_name, int *result)
{
	struct mdinfo *dev, *sra, *dev2;
	int working = 0, preexist = 0;
	int expansion = 0;
	int old_raid_disks;
	int start_reshape;
	char *avail;
	int err;

	if (sysfs_init(content, mdfd, NULL)) {
		pr_err("Unable to initialize sysfs\n");
		return 1;
	}

	sra = sysfs_read(mdfd, NULL, GET_VERSION|GET_DEVS);
	if (sra == NULL || strcmp(sra->text_version, content->text_version) != 0) {
		if (content->array.major_version == -1 &&
		    content->array.minor_version == -2 &&
		    c->readonly &&
		    content->text_version[0] == '/')
			content->text_version[0] = '-';
		if (sysfs_set_array(content, 9003) != 0) {
			sysfs_free(sra);
			return 1;
		}
	}

	/* There are two types of reshape: container wide or sub-array specific
	 * Check if metadata requests blocking container wide reshapes
	 */
	start_reshape = (content->reshape_active &&
			 !((content->reshape_active == CONTAINER_RESHAPE) &&
			   (content->array.state & (1<<MD_SB_BLOCK_CONTAINER_RESHAPE))));

	/* Block subarray here if it is under reshape now
	 * Do not allow for any changes in this array
	 */
	if (st->ss->external && content->recovery_blocked && start_reshape)
		block_subarray(content);

	for (dev2 = sra->devs; dev2; dev2 = dev2->next) {
		for (dev = content->devs; dev; dev = dev->next)
			if (dev2->disk.major == dev->disk.major &&
			    dev2->disk.minor == dev->disk.minor)
				break;
		if (dev)
			continue;
		/* Don't want this one any more */
		if (sysfs_set_str(sra, dev2, "slot", "none") < 0 &&
		    errno == EBUSY) {
			pr_err("Cannot remove old device %s: not updating %s\n", dev2->sys_name, sra->sys_name);
			sysfs_free(sra);
			return 1;
		}
		sysfs_set_str(sra, dev2, "state", "remove");
	}
	old_raid_disks = content->array.raid_disks - content->delta_disks;
	avail = xcalloc(content->array.raid_disks, 1);
	for (dev = content->devs; dev; dev = dev->next) {
		if (dev->disk.raid_disk >= 0)
			avail[dev->disk.raid_disk] = 1;
		if (sysfs_add_disk(content, dev, 1) == 0) {
			if (dev->disk.raid_disk >= old_raid_disks &&
			    content->reshape_active)
				expansion++;
			else
				working++;
		} else if (errno == EEXIST)
			preexist++;
	}
	sysfs_free(sra);
	if (working + expansion == 0 && c->runstop <= 0) {
		free(avail);
		return 1;/* Nothing new, don't try to start */
	}
	map_update(NULL, fd2devnm(mdfd), content->text_version,
		   content->uuid, chosen_name);

	if (content->consistency_policy == CONSISTENCY_POLICY_PPL &&
	    st->ss->validate_ppl) {
		content->array.state |= 1;
		err = 0;

		for (dev = content->devs; dev; dev = dev->next) {
			int dfd;
			char *devpath;
			int ret;

			ret = st->ss->validate_ppl(st, content, dev);
			if (ret == 0)
				continue;

			if (ret < 0) {
				err = 1;
				break;
			}

			if (!c->force) {
				pr_err("%s contains invalid PPL - consider --force or --update-subarray with --update=no-ppl\n",
					chosen_name);
				content->array.state &= ~1;
				avail[dev->disk.raid_disk] = 0;
				break;
			}

			/* have --force - overwrite the invalid ppl */
			devpath = map_dev(dev->disk.major, dev->disk.minor, 0);
			dfd = dev_open(devpath, O_RDWR);
			if (dfd < 0) {
				pr_err("Failed to open %s\n", devpath);
				err = 1;
				break;
			}

			err = st->ss->write_init_ppl(st, content, dfd);
			close(dfd);

			if (err)
				break;
		}

		if (err) {
			free(avail);
			return err;
		}
	}

	if (enough(content->array.level, content->array.raid_disks,
		   content->array.layout, content->array.state & 1, avail) == 0) {
		if (c->export && result)
			*result |= INCR_NO;
		else if (c->verbose >= 0) {
			pr_err("%s assembled with %d device%s",
			       chosen_name, preexist + working,
			       preexist + working == 1 ? "":"s");
			if (preexist)
				fprintf(stderr, " (%d new)", working);
			fprintf(stderr, " but not started\n");
		}
		free(avail);
		return 1;
	}
	free(avail);

	if (c->runstop <= 0 &&
	    (working + preexist + expansion) <
	    content->array.working_disks) {
		if (c->export && result)
			*result |= INCR_UNSAFE;
		else if (c->verbose >= 0) {
			pr_err("%s assembled with %d device%s",
			       chosen_name, preexist + working,
			       preexist + working == 1 ? "":"s");
			if (preexist)
				fprintf(stderr, " (%d new)", working);
			fprintf(stderr, " but not safe to start\n");
		}
		return 1;
	}


	if (start_reshape) {
		int spare = content->array.raid_disks + expansion;
		if (restore_backup(st, content,
				   working,
				   spare, &c->backup_file, c->verbose) == 1)
			return 1;

		err = sysfs_set_str(content, NULL,
				    "array_state", "readonly");
		if (err)
			return 1;

		if (st->ss->external) {
			if (!mdmon_running(st->container_devnm))
				start_mdmon(st->container_devnm);
			ping_monitor(st->container_devnm);
			if (mdmon_running(st->container_devnm) &&
			    st->update_tail == NULL)
				st->update_tail = &st->updates;
		}

		err = Grow_continue(mdfd, st, content, c->backup_file,
				    0, c->freeze_reshape);
	} else switch(content->array.level) {
		case LEVEL_LINEAR:
		case LEVEL_MULTIPATH:
		case 0:
			err = sysfs_set_str(content, NULL, "array_state",
					    c->readonly ? "readonly" : "active");
			break;
		default:
			err = sysfs_set_str(content, NULL, "array_state",
					    "readonly");
			/* start mdmon if needed. */
			if (!err) {
				if (!mdmon_running(st->container_devnm))
					start_mdmon(st->container_devnm);
				ping_monitor(st->container_devnm);
			}
			break;
		}
	if (!err)
		sysfs_set_safemode(content, content->safe_mode_delay);

	/* Block subarray here if it is not reshaped now
	 * It has be blocked a little later to allow mdmon to switch in
	 * in to R/W state
	 */
	if (st->ss->external && content->recovery_blocked &&
	    !start_reshape)
		block_subarray(content);

	if (c->export && result) {
		if (err)
			*result |= INCR_NO;
		else
			*result |= INCR_YES;
	} else if (c->verbose >= 0) {
		if (err)
			pr_err("array %s now has %d device%s",
			       chosen_name, working + preexist,
			       working + preexist == 1 ? "":"s");
		else
			pr_err("Started %s with %d device%s",
			       chosen_name, working + preexist,
			       working + preexist == 1 ? "":"s");
		if (preexist)
			fprintf(stderr, " (%d new)", working);
		if (expansion)
			fprintf(stderr, " ( + %d for expansion)",
				expansion);
		fprintf(stderr, "\n");
	}
	if (!err)
		wait_for(chosen_name, mdfd);
	return err;
	/* FIXME should have an O_EXCL and wait for read-auto */
}
