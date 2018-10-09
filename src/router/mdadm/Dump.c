/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2013 Neil Brown <neilb@suse.de>
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

 *    Author: Neil Brown
 *    Email: <neilb@suse.de>
 */

#include "mdadm.h"
#include <sys/dir.h>

int Dump_metadata(char *dev, char *dir, struct context *c,
		  struct supertype *st)
{
	/* create a new file in 'dir' named for the basename of 'dev'.
	 * Truncate to the same size as 'dev' and ask the metadata
	 * handler to copy metadata there.
	 * For every name in /dev/disk/by-id that points to this device,
	 * create a hardlink in 'dir'.
	 * Complain if any of those hardlinks cannot be created.
	 */
	int fd, fl;
	struct stat stb, dstb;
	char *base;
	char *fname = NULL;
	unsigned long long size;
	DIR *dirp;
	struct dirent *de;

	if (stat(dir, &stb) != 0 ||
	    (S_IFMT & stb.st_mode) != S_IFDIR) {
		pr_err("--dump requires an existing directory, not: %s\n",
			dir);
		return 16;
	}

	fd = dev_open(dev, O_RDONLY);
	if (fd < 0) {
		pr_err("Cannot open %s to dump metadata: %s\n",
		       dev, strerror(errno));
		return 1;
	}
	if (!get_dev_size(fd, dev, &size)) {
		close(fd);
		return 1;
	}

	if (st == NULL)
		st = guess_super_type(fd, guess_array);
	if (!st) {
		pr_err("Cannot find RAID metadata on %s\n", dev);
		close(fd);
		return 1;
	}

	st->ignore_hw_compat = 1;
	if (st->ss->load_super(st, fd, NULL) != 0) {
		pr_err("No %s metadata found on %s\n",
		       st->ss->name, dev);
		close(fd);
		return 1;
	}
	if (st->ss->copy_metadata == NULL) {
		pr_err("%s metadata on %s cannot be copied\n",
		       st->ss->name, dev);
		close(fd);
		return 1;
	}

	base = strrchr(dev, '/');
	if (base)
		base++;
	else
		base = dev;
	xasprintf(&fname, "%s/%s", dir, base);
	fl = open(fname, O_RDWR|O_CREAT|O_EXCL, 0666);
	if (fl < 0) {
		pr_err("Cannot create dump file %s: %s\n",
		       fname, strerror(errno));
		close(fd);
		free(fname);
		return 1;
	}
	if (ftruncate(fl, size) < 0) {
		pr_err("failed to set size of dump file: %s\n",
		       strerror(errno));
		close(fd);
		close(fl);
		free(fname);
		return 1;
	}

	if (st->ss->copy_metadata(st, fd, fl) != 0) {
		pr_err("Failed to copy metadata from %s to %s\n",
		       dev, fname);
		close(fd);
		close(fl);
		unlink(fname);
		free(fname);
		return 1;
	}
	if (c->verbose >= 0)
		printf("%s saved as %s.\n", dev, fname);
	fstat(fd, &dstb);
	close(fd);
	close(fl);
	if ((dstb.st_mode & S_IFMT) != S_IFBLK) {
		/* Not a block device, so cannot create links */
		free(fname);
		return 0;
	}
	/* mostly done: just want to find some other names */
	dirp = opendir("/dev/disk/by-id");
	if (!dirp) {
		free(fname);
		return 0;
	}
	while ((de = readdir(dirp)) != NULL) {
		char *p = NULL;
		if (de->d_name[0] == '.')
			continue;
		xasprintf(&p, "/dev/disk/by-id/%s", de->d_name);
		if (stat(p, &stb) != 0 ||
		    (stb.st_mode & S_IFMT) != S_IFBLK ||
		    stb.st_rdev != dstb.st_rdev) {
			/* Not this one */
			free(p);
			continue;
		}
		free(p);
		xasprintf(&p, "%s/%s", dir, de->d_name);
		if (link(fname, p) == 0) {
			if (c->verbose >= 0)
				printf("%s also saved as %s.\n",
				       dev, p);
		} else {
			pr_err("Could not save %s as %s!!\n",
				       dev, p);
		}
		free(p);
	}
	closedir(dirp);
	free(fname);
	return 0;
}

int Restore_metadata(char *dev, char *dir, struct context *c,
		     struct supertype *st, int only)
{
	/* If 'dir' really is a directory we choose a name
	 * from it that matches a suitable name in /dev/disk/by-id,
	 * and copy metadata from the file to the device.
	 * If two names from by-id match and aren't both the same
	 * inode, we fail.  If none match and basename of 'dev'
	 * can be found  in dir, use that.
	 * If 'dir' is really a file then it is only permitted if
	 * 'only' is set (meaning there was only one device given)
	 * and the metadata is restored irrespective of file names.
	 */
	int fd, fl;
	struct stat stb, dstb;
	char *fname = NULL;
	unsigned long long size;

	if (stat(dir, &stb) != 0) {
		pr_err("%s does not exist: cannot restore from there.\n",
		       dir);
		return 16;
	} else if ((S_IFMT & stb.st_mode) != S_IFDIR && !only) {
		pr_err("--restore requires a directory when multiple devices given\n");
		return 16;
	}

	fd = dev_open(dev, O_RDWR);
	if (fd < 0) {
		pr_err("Cannot open %s to restore metadata: %s\n",
		       dev, strerror(errno));
		return 1;
	}
	if (!get_dev_size(fd, dev, &size)) {
		close(fd);
		return 1;
	}

	if ((S_IFMT & stb.st_mode) == S_IFDIR) {
		/* choose one name from the directory. */
		DIR *d = opendir(dir);
		struct dirent *de;
		char *chosen = NULL;
		unsigned int chosen_inode = 0;

		fstat(fd, &dstb);

		while (d && (de = readdir(d)) != NULL) {
			if (de->d_name[0] == '.')
				continue;
			xasprintf(&fname, "/dev/disk/by-id/%s", de->d_name);
			if (stat(fname, &stb) != 0) {
				free(fname);
				continue;
			}
			free(fname);
			if ((S_IFMT & stb.st_mode) != S_IFBLK)
				continue;
			if (stb.st_rdev != dstb.st_rdev)
				continue;
			/* This file is a good match for our device. */
			xasprintf(&fname, "%s/%s", dir, de->d_name);
			if (stat(fname, &stb) != 0) {
				/* Weird! */
				free(fname);
				continue;
			}
			if (chosen == NULL) {
				chosen = fname;
				chosen_inode = stb.st_ino;
				continue;
			}
			if (chosen_inode == stb.st_ino) {
				/* same, no need to change */
				free(fname);
				continue;
			}
			/* Oh dear, two names both match.  Must give up. */
			pr_err("Both %s and %s seem suitable for %s.  Please choose one.\n",
			       chosen, fname, dev);
			free(fname);
			free(chosen);
			close(fd);
			closedir(d);
			return 1;
		}
		closedir(d);
		if (!chosen) {
			/* One last chance: try basename of device */
			char *base = strrchr(dev, '/');
			if (base)
				base++;
			else
				base = dev;
			xasprintf(&fname, "%s/%s", dir, base);
			if (stat(fname, &stb) == 0)
				chosen = fname;
			else
				free(fname);
		}
		fname = chosen;
	} else
		fname = strdup(dir);

	if (!fname) {
		pr_err("Cannot find suitable file in %s for %s\n",
		       dir, dev);
		close(fd);
		return 1;
	}

	fl = open(fname, O_RDONLY);
	if (!fl) {
		pr_err("Could not open %s for --restore.\n",
		       fname);
		goto err;
	}
	if (((unsigned long long)stb.st_size) != size) {
		pr_err("%s is not the same size as %s - cannot restore.\n",
		       fname, dev);
		goto err;
	}
	if (st == NULL)
		st = guess_super_type(fl, guess_array);
	if (!st) {
		pr_err("Cannot find metadata on %s\n", fname);
		goto err;
	}
	st->ignore_hw_compat = 1;
	if (st->ss->load_super(st, fl, NULL) != 0) {
		pr_err("No %s metadata found on %s\n",
		       st->ss->name, fname);
		goto err;
	}
	if (st->ss->copy_metadata == NULL) {
		pr_err("%s metadata on %s cannot be copied\n",
		       st->ss->name, dev);
		goto err;
	}
	if (st->ss->copy_metadata(st, fl, fd) != 0) {
		pr_err("Failed to copy metadata from %s to %s\n",
		       fname, dev);
		goto err;
	}
	if (c->verbose >= 0)
		printf("%s restored from %s.\n", dev, fname);
	close(fl);
	close(fd);
	free(fname);
	return 0;

err:
	close(fd);
	close(fl);
	free(fname);
	return 1;
}
