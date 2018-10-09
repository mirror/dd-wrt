/*
 * sysfs - extract md related information from sysfs.  Part of:
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2006-2009 Neil Brown <neilb@suse.de>
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
#include	<dirent.h>
#include	<ctype.h>

#define MAX_SYSFS_PATH_LEN	120

int load_sys(char *path, char *buf, int len)
{
	int fd = open(path, O_RDONLY);
	int n;
	if (fd < 0)
		return -1;
	n = read(fd, buf, len);
	close(fd);
	if (n <0 || n >= len)
		return -1;
	buf[n] = 0;
	if (n && buf[n-1] == '\n')
		buf[n-1] = 0;
	return 0;
}

void sysfs_free(struct mdinfo *sra)
{
	while (sra) {
		struct mdinfo *sra2 = sra->next;
		while (sra->devs) {
			struct mdinfo *d = sra->devs;
			sra->devs = d->next;
			free(d->bb.entries);
			free(d);
		}
		free(sra->bb.entries);
		free(sra);
		sra = sra2;
	}
}

int sysfs_open(char *devnm, char *devname, char *attr)
{
	char fname[MAX_SYSFS_PATH_LEN];
	int fd;

	snprintf(fname, MAX_SYSFS_PATH_LEN, "/sys/block/%s/md/", devnm);
	if (devname) {
		strncat(fname, devname, MAX_SYSFS_PATH_LEN - strlen(fname));
		strncat(fname, "/", MAX_SYSFS_PATH_LEN - strlen(fname));
	}
	strncat(fname, attr, MAX_SYSFS_PATH_LEN - strlen(fname));
	fd = open(fname, O_RDWR);
	if (fd < 0 && errno == EACCES)
		fd = open(fname, O_RDONLY);
	return fd;
}

void sysfs_init_dev(struct mdinfo *mdi, dev_t devid)
{
	snprintf(mdi->sys_name,
		 sizeof(mdi->sys_name), "dev-%s", devid2kname(devid));
}

int sysfs_init(struct mdinfo *mdi, int fd, char *devnm)
{
	struct stat stb;
	char fname[MAX_SYSFS_PATH_LEN];
	int retval = -ENODEV;

	mdi->sys_name[0] = 0;
	if (fd >= 0)
		devnm = fd2devnm(fd);

	if (devnm == NULL)
		goto out;

	snprintf(fname, MAX_SYSFS_PATH_LEN, "/sys/block/%s/md", devnm);

	if (stat(fname, &stb))
		goto out;
	if (!S_ISDIR(stb.st_mode))
		goto out;
	strcpy(mdi->sys_name, devnm);

	retval = 0;
out:
	return retval;
}

struct mdinfo *sysfs_read(int fd, char *devnm, unsigned long options)
{
	char fname[PATH_MAX];
	char buf[PATH_MAX];
	char *base;
	char *dbase;
	struct mdinfo *sra;
	struct mdinfo *dev, **devp;
	DIR *dir = NULL;
	struct dirent *de;

	sra = xcalloc(1, sizeof(*sra));
	if (sysfs_init(sra, fd, devnm)) {
		free(sra);
		return NULL;
	}

	sprintf(fname, "/sys/block/%s/md/", sra->sys_name);
	base = fname + strlen(fname);

	sra->devs = NULL;
	if (options & GET_VERSION) {
		strcpy(base, "metadata_version");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		if (strncmp(buf, "none", 4) == 0) {
			sra->array.major_version =
				sra->array.minor_version = -1;
			strcpy(sra->text_version, "");
		} else if (strncmp(buf, "external:", 9) == 0) {
			sra->array.major_version = -1;
			sra->array.minor_version = -2;
			strcpy(sra->text_version, buf+9);
		} else {
			sscanf(buf, "%d.%d",
			       &sra->array.major_version,
			       &sra->array.minor_version);
			strcpy(sra->text_version, buf);
		}
	}
	if (options & GET_LEVEL) {
		strcpy(base, "level");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->array.level = map_name(pers, buf);
	}
	if (options & GET_LAYOUT) {
		strcpy(base, "layout");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->array.layout = strtoul(buf, NULL, 0);
	}
	if (options & (GET_DISKS|GET_STATE)) {
		strcpy(base, "raid_disks");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->array.raid_disks = strtoul(buf, NULL, 0);
	}
	if (options & GET_COMPONENT) {
		strcpy(base, "component_size");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->component_size = strtoull(buf, NULL, 0);
		/* sysfs reports "K", but we want sectors */
		sra->component_size *= 2;
	}
	if (options & GET_CHUNK) {
		strcpy(base, "chunk_size");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->array.chunk_size = strtoul(buf, NULL, 0);
	}
	if (options & GET_CACHE) {
		strcpy(base, "stripe_cache_size");
		if (load_sys(fname, buf, sizeof(buf)))
			/* Probably level doesn't support it */
			sra->cache_size = 0;
		else
			sra->cache_size = strtoul(buf, NULL, 0);
	}
	if (options & GET_MISMATCH) {
		strcpy(base, "mismatch_cnt");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->mismatch_cnt = strtoul(buf, NULL, 0);
	}
	if (options & GET_SAFEMODE) {
		int scale = 1;
		int dot = 0;
		unsigned i;
		unsigned long msec;
		size_t len;

		strcpy(base, "safe_mode_delay");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;

		/* remove a period, and count digits after it */
		len = strlen(buf);
		for (i = 0; i < len; i++) {
			if (dot) {
				if (isdigit(buf[i])) {
					buf[i-1] = buf[i];
					scale *= 10;
				}
				buf[i] = 0;
			} else if (buf[i] == '.') {
				dot=1;
				buf[i] = 0;
			}
		}
		msec = strtoul(buf, NULL, 10);
		msec = (msec * 1000) / scale;
		sra->safe_mode_delay = msec;
	}
	if (options & GET_BITMAP_LOCATION) {
		strcpy(base, "bitmap/location");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		if (strncmp(buf, "file", 4) == 0)
			sra->bitmap_offset = 1;
		else if (strncmp(buf, "none", 4) == 0)
			sra->bitmap_offset = 0;
		else if (buf[0] == '+')
			sra->bitmap_offset = strtol(buf+1, NULL, 10);
		else
			goto abort;
	}

	if (options & GET_ARRAY_STATE) {
		strcpy(base, "array_state");
		if (load_sys(fname, buf, sizeof(buf)))
			goto abort;
		sra->array_state = map_name(sysfs_array_states, buf);
	}

	if (options & GET_CONSISTENCY_POLICY) {
		strcpy(base, "consistency_policy");
		if (load_sys(fname, buf, sizeof(buf)))
			sra->consistency_policy = CONSISTENCY_POLICY_UNKNOWN;
		else
			sra->consistency_policy = map_name(consistency_policies,
							   buf);
	}

	if (! (options & GET_DEVS))
		return sra;

	/* Get all the devices as well */
	*base = 0;
	dir = opendir(fname);
	if (!dir)
		goto abort;
	sra->array.spare_disks = 0;
	sra->array.active_disks = 0;
	sra->array.failed_disks = 0;
	sra->array.working_disks = 0;

	devp = &sra->devs;
	sra->devs = NULL;
	while ((de = readdir(dir)) != NULL) {
		char *ep;
		if (de->d_ino == 0 ||
		    strncmp(de->d_name, "dev-", 4) != 0)
			continue;
		strcpy(base, de->d_name);
		dbase = base + strlen(base);
		*dbase++ = '/';

		dev = xcalloc(1, sizeof(*dev));

		/* Always get slot, major, minor */
		strcpy(dbase, "slot");
		if (load_sys(fname, buf, sizeof(buf))) {
			/* hmm... unable to read 'slot' maybe the device
			 * is going away?
			 */
			strcpy(dbase, "block");
			if (readlink(fname, buf, sizeof(buf)) < 0 &&
			    errno != ENAMETOOLONG) {
				/* ...yup device is gone */
				free(dev);
				continue;
			} else {
				/* slot is unreadable but 'block' link
				 * still intact... something bad is happening
				 * so abort
				 */
				free(dev);
				goto abort;
			}

		}
		strcpy(dev->sys_name, de->d_name);
		dev->disk.raid_disk = strtoul(buf, &ep, 10);
		if (*ep) dev->disk.raid_disk = -1;

		sra->array.nr_disks++;
		strcpy(dbase, "block/dev");
		if (load_sys(fname, buf, sizeof(buf))) {
			/* assume this is a stale reference to a hot
			 * removed device
			 */
			free(dev);
			continue;
		}
		sscanf(buf, "%d:%d", &dev->disk.major, &dev->disk.minor);

		/* special case check for block devices that can go 'offline' */
		strcpy(dbase, "block/device/state");
		if (load_sys(fname, buf, sizeof(buf)) == 0 &&
		    strncmp(buf, "offline", 7) == 0) {
			free(dev);
			continue;
		}

		/* finally add this disk to the array */
		*devp = dev;
		devp = & dev->next;
		dev->next = NULL;

		if (options & GET_OFFSET) {
			strcpy(dbase, "offset");
			if (load_sys(fname, buf, sizeof(buf)))
				goto abort;
			dev->data_offset = strtoull(buf, NULL, 0);
			strcpy(dbase, "new_offset");
			if (load_sys(fname, buf, sizeof(buf)) == 0)
				dev->new_data_offset = strtoull(buf, NULL, 0);
			else
				dev->new_data_offset = dev->data_offset;
		}
		if (options & GET_SIZE) {
			strcpy(dbase, "size");
			if (load_sys(fname, buf, sizeof(buf)))
				goto abort;
			dev->component_size = strtoull(buf, NULL, 0) * 2;
		}
		if (options & GET_STATE) {
			dev->disk.state = 0;
			strcpy(dbase, "state");
			if (load_sys(fname, buf, sizeof(buf)))
				goto abort;
			if (strstr(buf, "faulty"))
				dev->disk.state |= (1<<MD_DISK_FAULTY);
			else {
				sra->array.working_disks++;
				if (strstr(buf, "in_sync")) {
					dev->disk.state |= (1<<MD_DISK_SYNC);
					sra->array.active_disks++;
				}
				if (dev->disk.state == 0)
					sra->array.spare_disks++;
			}
		}
		if (options & GET_ERROR) {
			strcpy(buf, "errors");
			if (load_sys(fname, buf, sizeof(buf)))
				goto abort;
			dev->errors = strtoul(buf, NULL, 0);
		}
	}

	if ((options & GET_STATE) && sra->array.raid_disks)
		sra->array.failed_disks = sra->array.raid_disks -
			sra->array.active_disks - sra->array.spare_disks;

	closedir(dir);
	return sra;

 abort:
	if (dir)
		closedir(dir);
	sysfs_free(sra);
	return NULL;
}

int sysfs_attr_match(const char *attr, const char *str)
{
	/* See if attr, read from a sysfs file, matches
	 * str.  They must either be the same, or attr can
	 * have a trailing newline or comma
	 */
	while (*attr && *str && *attr == *str) {
		attr++;
		str++;
	}

	if (*str || (*attr && *attr != ',' && *attr != '\n'))
		return 0;
	return 1;
}

int sysfs_match_word(const char *word, char **list)
{
	int n;
	for (n=0; list[n]; n++)
		if (sysfs_attr_match(word, list[n]))
			break;
	return n;
}

unsigned long long get_component_size(int fd)
{
	/* Find out the component size of the array.
	 * We cannot trust GET_ARRAY_INFO ioctl as it's
	 * size field is only 32bits.
	 * So look in /sys/block/mdXXX/md/component_size
	 *
	 * This returns in units of sectors.
	 */
	struct stat stb;
	char fname[MAX_SYSFS_PATH_LEN];
	int n;
	if (fstat(fd, &stb))
		return 0;
	snprintf(fname, MAX_SYSFS_PATH_LEN,
		 "/sys/block/%s/md/component_size", stat2devnm(&stb));
	fd = open(fname, O_RDONLY);
	if (fd < 0)
		return 0;
	n = read(fd, fname, sizeof(fname));
	close(fd);
	if (n < 0 || n == sizeof(fname))
		return 0;
	fname[n] = 0;
	return strtoull(fname, NULL, 10) * 2;
}

int sysfs_set_str(struct mdinfo *sra, struct mdinfo *dev,
		  char *name, char *val)
{
	char fname[MAX_SYSFS_PATH_LEN];
	unsigned int n;
	int fd;

	snprintf(fname, MAX_SYSFS_PATH_LEN, "/sys/block/%s/md/%s/%s",
		sra->sys_name, dev?dev->sys_name:"", name);
	fd = open(fname, O_WRONLY);
	if (fd < 0)
		return -1;
	n = write(fd, val, strlen(val));
	close(fd);
	if (n != strlen(val)) {
		dprintf("failed to write '%s' to '%s' (%s)\n",
			val, fname, strerror(errno));
		return -1;
	}
	return 0;
}

int sysfs_set_num(struct mdinfo *sra, struct mdinfo *dev,
		  char *name, unsigned long long val)
{
	char valstr[50];
	sprintf(valstr, "%llu", val);
	return sysfs_set_str(sra, dev, name, valstr);
}

int sysfs_set_num_signed(struct mdinfo *sra, struct mdinfo *dev,
			 char *name, long long val)
{
	char valstr[50];
	sprintf(valstr, "%lli", val);
	return sysfs_set_str(sra, dev, name, valstr);
}

int sysfs_uevent(struct mdinfo *sra, char *event)
{
	char fname[MAX_SYSFS_PATH_LEN];
	int n;
	int fd;

	snprintf(fname, MAX_SYSFS_PATH_LEN, "/sys/block/%s/uevent",
		sra->sys_name);
	fd = open(fname, O_WRONLY);
	if (fd < 0)
		return -1;
	n = write(fd, event, strlen(event));
	close(fd);
	if (n != (int)strlen(event)) {
		dprintf("failed to write '%s' to '%s' (%s)\n",
			event, fname, strerror(errno));
		return -1;
	}
	return 0;
}

int sysfs_attribute_available(struct mdinfo *sra, struct mdinfo *dev, char *name)
{
	char fname[MAX_SYSFS_PATH_LEN];
	struct stat st;

	snprintf(fname, MAX_SYSFS_PATH_LEN, "/sys/block/%s/md/%s/%s",
		sra->sys_name, dev?dev->sys_name:"", name);

	return stat(fname, &st) == 0;
}

int sysfs_get_fd(struct mdinfo *sra, struct mdinfo *dev,
		       char *name)
{
	char fname[MAX_SYSFS_PATH_LEN];
	int fd;

	snprintf(fname, MAX_SYSFS_PATH_LEN, "/sys/block/%s/md/%s/%s",
		sra->sys_name, dev?dev->sys_name:"", name);
	fd = open(fname, O_RDWR);
	if (fd < 0)
		fd = open(fname, O_RDONLY);
	return fd;
}

int sysfs_fd_get_ll(int fd, unsigned long long *val)
{
	char buf[50];
	int n;
	char *ep;

	lseek(fd, 0, 0);
	n = read(fd, buf, sizeof(buf));
	if (n <= 0 || n == sizeof(buf))
		return -2;
	buf[n] = 0;
	*val = strtoull(buf, &ep, 0);
	if (ep == buf || (*ep != 0 && *ep != '\n' && *ep != ' '))
		return -1;
	return 0;
}

int sysfs_get_ll(struct mdinfo *sra, struct mdinfo *dev,
		       char *name, unsigned long long *val)
{
	int n;
	int fd;

	fd = sysfs_get_fd(sra, dev, name);
	if (fd < 0)
		return -1;
	n = sysfs_fd_get_ll(fd, val);
	close(fd);
	return n;
}

int sysfs_fd_get_two(int fd, unsigned long long *v1, unsigned long long *v2)
{
	/* two numbers in this sysfs file, either
	 *  NNN (NNN)
	 * or
	 *  NNN / NNN
	 */
	char buf[80];
	int n;
	char *ep, *ep2;

	lseek(fd, 0, 0);
	n = read(fd, buf, sizeof(buf));
	if (n <= 0 || n == sizeof(buf))
		return -2;
	buf[n] = 0;
	*v1 = strtoull(buf, &ep, 0);
	if (ep == buf || (*ep != 0 && *ep != '\n' && *ep != ' '))
		return -1;
	while (*ep == ' ' || *ep == '/' || *ep == '(')
		ep++;
	*v2 = strtoull(ep, &ep2, 0);
	if (ep2 == ep || (*ep2 != 0 && *ep2 != '\n' && *ep2 != ' ' && *ep2 != ')')) {
		*v2 = *v1;
		return 1;
	}
	return 2;
}

int sysfs_get_two(struct mdinfo *sra, struct mdinfo *dev,
		  char *name, unsigned long long *v1, unsigned long long *v2)
{
	int n;
	int fd;

	fd = sysfs_get_fd(sra, dev, name);
	if (fd < 0)
		return -1;
	n = sysfs_fd_get_two(fd, v1, v2);
	close(fd);
	return n;
}

int sysfs_fd_get_str(int fd, char *val, int size)
{
	int n;

	lseek(fd, 0, 0);
	n = read(fd, val, size);
	if (n <= 0 || n == size)
		return -1;
	val[n] = 0;
	return n;
}

int sysfs_get_str(struct mdinfo *sra, struct mdinfo *dev,
		       char *name, char *val, int size)
{
	int n;
	int fd;

	fd = sysfs_get_fd(sra, dev, name);
	if (fd < 0)
		return -1;
	n = sysfs_fd_get_str(fd, val, size);
	close(fd);
	return n;
}

int sysfs_set_safemode(struct mdinfo *sra, unsigned long ms)
{
	unsigned long sec;
	unsigned long msec;
	char delay[30];

	sec = ms / 1000;
	msec = ms % 1000;

	sprintf(delay, "%ld.%03ld\n", sec, msec);
	/*             this '\n' ^ needed for kernels older than 2.6.28 */
	return sysfs_set_str(sra, NULL, "safe_mode_delay", delay);
}

int sysfs_set_array(struct mdinfo *info, int vers)
{
	int rv = 0;
	char ver[100];
	int raid_disks = info->array.raid_disks;

	ver[0] = 0;
	if (info->array.major_version == -1 &&
	    info->array.minor_version == -2) {
		char buf[1024];

		strcat(strcpy(ver, "external:"), info->text_version);

		/* meta version might already be set if we are setting
		 * new geometry for a reshape.  In that case we don't
		 * want to over-write the 'readonly' flag that is
		 * stored in the metadata version.  So read the current
		 * version first, and preserve the flag
		 */
		if (sysfs_get_str(info, NULL, "metadata_version",
				  buf, 1024) > 0)
			if (strlen(buf) >= 9 && buf[9] == '-')
				ver[9] = '-';

		if ((vers % 100) < 2 ||
		    sysfs_set_str(info, NULL, "metadata_version",
				  ver) < 0) {
			pr_err("This kernel does not support external metadata.\n");
			return 1;
		}
	}
	if (info->array.level < 0)
		return 0; /* FIXME */
	rv |= sysfs_set_str(info, NULL, "level",
			    map_num(pers, info->array.level));
	if (info->reshape_active && info->delta_disks != UnSet)
		raid_disks -= info->delta_disks;
	rv |= sysfs_set_num(info, NULL, "raid_disks", raid_disks);
	rv |= sysfs_set_num(info, NULL, "chunk_size", info->array.chunk_size);
	rv |= sysfs_set_num(info, NULL, "layout", info->array.layout);
	rv |= sysfs_set_num(info, NULL, "component_size", info->component_size/2);
	if (info->custom_array_size) {
		int rc;

		rc = sysfs_set_num(info, NULL, "array_size",
				   info->custom_array_size/2);
		if (rc && errno == ENOENT) {
			pr_err("This kernel does not have the md/array_size attribute, the array may be larger than expected\n");
			rc = 0;
		}
		rv |= rc;
	}

	if (info->array.level > 0)
		rv |= sysfs_set_num(info, NULL, "resync_start", info->resync_start);

	if (info->reshape_active) {
		rv |= sysfs_set_num(info, NULL, "reshape_position",
				    info->reshape_progress);
		rv |= sysfs_set_num(info, NULL, "chunk_size", info->new_chunk);
		rv |= sysfs_set_num(info, NULL, "layout", info->new_layout);
		rv |= sysfs_set_num(info, NULL, "raid_disks",
				    info->array.raid_disks);
		/* We don't set 'new_level' here.  That can only happen
		 * once the reshape completes.
		 */
	}

	if (info->consistency_policy == CONSISTENCY_POLICY_PPL) {
		if (sysfs_set_str(info, NULL, "consistency_policy",
				  map_num(consistency_policies,
					  info->consistency_policy))) {
			pr_err("This kernel does not support PPL. Falling back to consistency-policy=resync.\n");
			info->consistency_policy = CONSISTENCY_POLICY_RESYNC;
		}
	}

	return rv;
}

int sysfs_add_disk(struct mdinfo *sra, struct mdinfo *sd, int resume)
{
	char dv[PATH_MAX];
	char nm[PATH_MAX];
	char *dname;
	int rv;
	int i;

	sprintf(dv, "%d:%d", sd->disk.major, sd->disk.minor);
	rv = sysfs_set_str(sra, NULL, "new_dev", dv);
	if (rv)
		return rv;

	memset(nm, 0, sizeof(nm));
	dname = devid2kname(makedev(sd->disk.major, sd->disk.minor));
	strcpy(sd->sys_name, "dev-");
	strcpy(sd->sys_name+4, dname);

	/* test write to see if 'recovery_start' is available */
	if (resume && sd->recovery_start < MaxSector &&
	    sysfs_set_num(sra, sd, "recovery_start", 0)) {
		sysfs_set_str(sra, sd, "state", "remove");
		return -1;
	}

	rv = sysfs_set_num(sra, sd, "offset", sd->data_offset);
	rv |= sysfs_set_num(sra, sd, "size", (sd->component_size+1) / 2);
	if (sra->array.level != LEVEL_CONTAINER) {
		if (sra->consistency_policy == CONSISTENCY_POLICY_PPL) {
			rv |= sysfs_set_num(sra, sd, "ppl_sector", sd->ppl_sector);
			rv |= sysfs_set_num(sra, sd, "ppl_size", sd->ppl_size);
		}
		if (sd->recovery_start == MaxSector)
			/* This can correctly fail if array isn't started,
			 * yet, so just ignore status for now.
			 */
			sysfs_set_str(sra, sd, "state", "insync");
		if (sd->disk.raid_disk >= 0)
			rv |= sysfs_set_num(sra, sd, "slot", sd->disk.raid_disk);
		if (resume)
			sysfs_set_num(sra, sd, "recovery_start", sd->recovery_start);
	}
	if (sd->bb.supported) {
		if (sysfs_set_str(sra, sd, "state", "external_bbl")) {
			/*
			 * backward compatibility - if kernel doesn't support
			 * bad blocks for external metadata, let it continue
			 * as long as there are none known so far
			 */
			if (sd->bb.count) {
				pr_err("The kernel has no support for bad blocks in external metadata\n");
				return -1;
			}
		}

		for (i = 0; i < sd->bb.count; i++) {
			char s[30];
			const struct md_bb_entry *entry = &sd->bb.entries[i];

			snprintf(s, sizeof(s) - 1, "%llu %d\n", entry->sector,
				 entry->length);
			rv |= sysfs_set_str(sra, sd, "bad_blocks", s);
		}
	}
	return rv;
}

#if 0
int sysfs_disk_to_sg(int fd)
{
	/* from an open block device, try find and open its corresponding
	 * scsi_generic interface
	 */
	struct stat st;
	char path[256];
	char sg_path[256];
	char sg_major_minor[10];
	char *c;
	DIR *dir;
	struct dirent *de;
	int major, minor, rv;

	if (fstat(fd, &st))
		return -1;

	snprintf(path, sizeof(path), "/sys/dev/block/%d:%d/device",
		 major(st.st_rdev), minor(st.st_rdev));

	dir = opendir(path);
	if (!dir)
		return -1;

	de = readdir(dir);
	while (de) {
		if (strncmp("scsi_generic:", de->d_name,
			    strlen("scsi_generic:")) == 0)
			break;
		de = readdir(dir);
	}
	closedir(dir);

	if (!de)
		return -1;

	snprintf(sg_path, sizeof(sg_path), "%s/%s/dev", path, de->d_name);
	fd = open(sg_path, O_RDONLY);
	if (fd < 0)
		return fd;

	rv = read(fd, sg_major_minor, sizeof(sg_major_minor));
	close(fd);
	if (rv < 0 || rv == sizeof(sg_major_minor))
		return -1;
	else
		sg_major_minor[rv - 1] = '\0';

	c = strchr(sg_major_minor, ':');
	*c = '\0';
	c++;
	major = strtol(sg_major_minor, NULL, 10);
	minor = strtol(c, NULL, 10);
	snprintf(path, sizeof(path), "/dev/.tmp.md.%d:%d:%d",
		 (int) getpid(), major, minor);
	if (mknod(path, S_IFCHR|0600, makedev(major, minor))==0) {
			fd = open(path, O_RDONLY);
			unlink(path);
			return fd;
	}

	return -1;
}
#endif

int sysfs_disk_to_scsi_id(int fd, __u32 *id)
{
	/* from an open block device, try to retrieve it scsi_id */
	struct stat st;
	char path[256];
	DIR *dir;
	struct dirent *de;
	int host, bus, target, lun;

	if (fstat(fd, &st))
		return 1;

	snprintf(path, sizeof(path), "/sys/dev/block/%d:%d/device/scsi_device",
		 major(st.st_rdev), minor(st.st_rdev));

	dir = opendir(path);
	if (!dir)
		return 1;

	for (de = readdir(dir); de; de = readdir(dir)) {
		int count;

		if (de->d_type != DT_DIR)
			continue;

		count = sscanf(de->d_name, "%d:%d:%d:%d", &host, &bus, &target, &lun);
		if (count == 4)
			break;
	}
	closedir(dir);

	if (!de)
		return 1;

	*id = (host << 24) | (bus << 16) | (target << 8) | (lun << 0);
	return 0;
}

int sysfs_unique_holder(char *devnm, long rdev)
{
	/* Check that devnm is a holder of rdev,
	 * and is the only holder.
	 * we should be locked against races by
	 * an O_EXCL on devnm
	 * Return values:
	 *  0 - not unique, not even a holder
	 *  1 - unique, this is the only holder.
	 *  2/3 - not unique, there is another holder
	 * -1 - error, cannot find the holders
	 */
	DIR *dir;
	struct dirent *de;
	char dirname[100];
	char l;
	int ret = 0;
	sprintf(dirname, "/sys/dev/block/%d:%d/holders",
		major(rdev), minor(rdev));
	dir = opendir(dirname);
	if (!dir)
		return -1;
	l = strlen(dirname);
	while ((de = readdir(dir)) != NULL) {
		char buf[100];
		char *sl;
		int n;

		if (de->d_ino == 0)
			continue;
		if (de->d_name[0] == '.')
			continue;
		strcpy(dirname+l, "/");
		strcat(dirname+l, de->d_name);
		n = readlink(dirname, buf, sizeof(buf)-1);
		if (n <= 0)
			continue;
		buf[n] = 0;
		sl = strrchr(buf, '/');
		if (!sl)
			continue;
		sl++;

		if (strcmp(devnm, sl) == 0)
			ret |= 1;
		else
			ret |= 2;
	}
	closedir(dir);
	return ret;
}

int sysfs_freeze_array(struct mdinfo *sra)
{
	/* Try to freeze resync/rebuild on this array/container.
	 * Return -1 if the array is busy,
	 * return 0 if this kernel doesn't support 'frozen'
	 * return 1 if it worked.
	 */
	char buf[20];

	if (!sysfs_attribute_available(sra, NULL, "sync_action"))
		return 1; /* no sync_action == frozen */
	if (sysfs_get_str(sra, NULL, "sync_action", buf, 20) <= 0)
		return 0;
	if (strcmp(buf, "frozen\n") == 0)
		/* Already frozen */
		return 0;
	if (strcmp(buf, "idle\n") != 0 && strcmp(buf, "recover\n") != 0)
		return -1;
	if (sysfs_set_str(sra, NULL, "sync_action", "frozen") < 0)
		return 0;
	return 1;
}

int sysfs_wait(int fd, int *msec)
{
	/* Wait up to '*msec' for fd to have an exception condition.
	 * if msec == NULL, wait indefinitely.
	 */
	fd_set fds;
	int n;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	if (msec == NULL)
		n = select(fd+1, NULL, NULL, &fds, NULL);
	else if (*msec < 0)
		n = 0;
	else {
		struct timeval start, end, tv;
		gettimeofday(&start, NULL);
		if (*msec < 1000) {
			tv.tv_sec = 0;
			tv.tv_usec = (*msec)*1000;
		} else {
			tv.tv_sec = (*msec)/1000;
			tv.tv_usec = 0;
		}
		n = select(fd+1, NULL, NULL, &fds, &tv);
		gettimeofday(&end, NULL);
		end.tv_sec -= start.tv_sec;
		*msec -= (end.tv_sec * 1000 + end.tv_usec/1000
			  - start.tv_usec/1000) + 1;
	}
	return n;
}
