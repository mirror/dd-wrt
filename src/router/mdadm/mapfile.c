/*
 * mapfile - keep track of uuid <-> array mapping. Part of:
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2006-2010 Neil Brown <neilb@suse.de>
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
 *    Paper: Neil Brown
 *           Novell Inc
 *           GPO Box Q1283
 *           QVB Post Office, NSW 1230
 *           Australia
 */

/* The mapfile is used to track arrays being created in --incremental
 * mode.  It particularly allows lookup from UUID to array device, but
 * also allows the array device name to be easily found.
 *
 * The map file is line based with space separated fields.  The fields are:
 *  Device id  -  mdX or mdpX  where X is a number.
 *  metadata   -  0.90 1.0 1.1 1.2 ddf ...
 *  UUID       -  uuid of the array
 *  path       -  path where device created: /dev/md/home
 *
 * The best place for the mapfile is /run/mdadm/map.  Distros and users
 * which have not switched to /run yet can choose a different location
 * at compile time via MAP_DIR and MAP_FILE.
 */
#include	"mdadm.h"
#include	<sys/file.h>
#include	<ctype.h>

#define MAP_READ 0
#define MAP_NEW 1
#define MAP_LOCK 2
#define MAP_DIRNAME 3

char *mapname[4] = {
	MAP_DIR "/" MAP_FILE,
	MAP_DIR "/" MAP_FILE ".new",
	MAP_DIR "/" MAP_FILE ".lock",
	MAP_DIR
};

int mapmode[3] = { O_RDONLY, O_RDWR|O_CREAT, O_RDWR|O_CREAT|O_TRUNC };
char *mapsmode[3] = { "r", "w", "w"};

FILE *open_map(int modenum)
{
	int fd;
	if ((mapmode[modenum] & O_CREAT))
		/* Attempt to create directory, don't worry about
		 * failure.
		 */
		(void)mkdir(mapname[MAP_DIRNAME], 0755);
	fd = open(mapname[modenum], mapmode[modenum], 0600);
	if (fd >= 0)
		return fdopen(fd, mapsmode[modenum]);
	return NULL;
}

int map_write(struct map_ent *mel)
{
	FILE *f;
	int err;

	f = open_map(MAP_NEW);

	if (!f)
		return 0;
	for (; mel; mel = mel->next) {
		if (mel->bad)
			continue;
		fprintf(f, "%s ", mel->devnm);
		fprintf(f, "%s ", mel->metadata);
		fprintf(f, "%08x:%08x:%08x:%08x ", mel->uuid[0],
			mel->uuid[1], mel->uuid[2], mel->uuid[3]);
		fprintf(f, "%s\n", mel->path?:"");
	}
	fflush(f);
	err = ferror(f);
	fclose(f);
	if (err) {
		unlink(mapname[1]);
		return 0;
	}
	return rename(mapname[1],
		      mapname[0]) == 0;
}

static FILE *lf = NULL;
int map_lock(struct map_ent **melp)
{
	while (lf == NULL) {
		struct stat buf;
		lf = open_map(MAP_LOCK);
		if (lf == NULL)
			return -1;
		if (flock(fileno(lf), LOCK_EX) != 0) {
			fclose(lf);
			lf = NULL;
			return -1;
		}
		if (fstat(fileno(lf), &buf) != 0 ||
		    buf.st_nlink == 0) {
			/* The owner of the lock unlinked it,
			 * so we have a lock on a stale file,
			 * try again
			 */
			fclose(lf);
			lf = NULL;
		}
	}
	if (*melp)
		map_free(*melp);
	map_read(melp);
	return 0;
}

void map_unlock(struct map_ent **melp)
{
	if (lf) {
		/* must unlink before closing the file,
		 * as only the owner of the lock may
		 * unlink the file
		 */
		unlink(mapname[2]);
		fclose(lf);
	}
	if (*melp)
		map_free(*melp);
	lf = NULL;
}

void map_fork(void)
{
	/* We are forking, so must close the lock file.
	 * Don't risk flushing anything though.
	 */
	if (lf) {
		close(fileno(lf));
		fclose(lf);
		lf = NULL;
	}
}

void map_add(struct map_ent **melp,
	     char * devnm, char *metadata, int uuid[4], char *path)
{
	struct map_ent *me = xmalloc(sizeof(*me));

	strcpy(me->devnm, devnm);
	strcpy(me->metadata, metadata);
	memcpy(me->uuid, uuid, 16);
	me->path = path ? xstrdup(path) : NULL;
	me->next = *melp;
	me->bad = 0;
	*melp = me;
}

void map_read(struct map_ent **melp)
{
	FILE *f;
	char buf[8192];
	char path[201];
	int uuid[4];
	char devnm[32];
	char metadata[30];

	*melp = NULL;

	f = open_map(MAP_READ);
	if (!f) {
		RebuildMap();
		f = open_map(MAP_READ);
	}
	if (!f)
		return;

	while (fgets(buf, sizeof(buf), f)) {
		path[0] = 0;
		if (sscanf(buf, " %s %s %x:%x:%x:%x %200s",
			   devnm, metadata, uuid, uuid+1,
			   uuid+2, uuid+3, path) >= 7) {
			map_add(melp, devnm, metadata, uuid, path);
		}
	}
	fclose(f);
}

void map_free(struct map_ent *map)
{
	while (map) {
		struct map_ent *mp = map;
		map = mp->next;
		free(mp->path);
		free(mp);
	}
}

int map_update(struct map_ent **mpp, char *devnm, char *metadata,
	       int *uuid, char *path)
{
	struct map_ent *map, *mp;
	int rv;

	if (mpp && *mpp)
		map = *mpp;
	else
		map_read(&map);

	for (mp = map ; mp ; mp=mp->next)
		if (strcmp(mp->devnm, devnm) == 0) {
			strcpy(mp->metadata, metadata);
			memcpy(mp->uuid, uuid, 16);
			free(mp->path);
			mp->path = path ? xstrdup(path) : NULL;
			mp->bad = 0;
			break;
		}
	if (!mp)
		map_add(&map, devnm, metadata, uuid, path);
	if (mpp)
		*mpp = NULL;
	rv = map_write(map);
	map_free(map);
	return rv;
}

void map_delete(struct map_ent **mapp, char *devnm)
{
	struct map_ent *mp;

	if (*mapp == NULL)
		map_read(mapp);

	for (mp = *mapp; mp; mp = *mapp) {
		if (strcmp(mp->devnm, devnm) == 0) {
			*mapp = mp->next;
			free(mp->path);
			free(mp);
		} else
			mapp = & mp->next;
	}
}

void map_remove(struct map_ent **mapp, char *devnm)
{
	if (devnm[0] == 0)
		return;

	map_delete(mapp, devnm);
	map_write(*mapp);
	map_free(*mapp);
	*mapp = NULL;
}

struct map_ent *map_by_uuid(struct map_ent **map, int uuid[4])
{
	struct map_ent *mp;
	if (!*map)
		map_read(map);

	for (mp = *map ; mp ; mp = mp->next) {
		if (memcmp(uuid, mp->uuid, 16) != 0)
			continue;
		if (!mddev_busy(mp->devnm)) {
			mp->bad = 1;
			continue;
		}
		return mp;
	}
	return NULL;
}

struct map_ent *map_by_devnm(struct map_ent **map, char *devnm)
{
	struct map_ent *mp;
	if (!*map)
		map_read(map);

	for (mp = *map ; mp ; mp = mp->next) {
		if (strcmp(mp->devnm, devnm) != 0)
			continue;
		if (!mddev_busy(mp->devnm)) {
			mp->bad = 1;
			continue;
		}
		return mp;
	}
	return NULL;
}

struct map_ent *map_by_name(struct map_ent **map, char *name)
{
	struct map_ent *mp;
	if (!*map)
		map_read(map);

	for (mp = *map ; mp ; mp = mp->next) {
		if (!mp->path)
			continue;
		if (strncmp(mp->path, "/dev/md/", 8) != 0)
			continue;
		if (strcmp(mp->path+8, name) != 0)
			continue;
		if (!mddev_busy(mp->devnm)) {
			mp->bad = 1;
			continue;
		}
		return mp;
	}
	return NULL;
}

/* sets the proper subarray and container_dev according to the metadata
 * version super_by_fd does this automatically, this routine is meant as
 * a supplement for guess_super()
 */
static char *get_member_info(struct mdstat_ent *ent)
{

	if (ent->metadata_version == NULL ||
	    strncmp(ent->metadata_version, "external:", 9) != 0)
		return NULL;

	if (is_subarray(&ent->metadata_version[9])) {
		char *subarray;

		subarray = strrchr(ent->metadata_version, '/');
		return subarray + 1;
	}
	return NULL;
}

void RebuildMap(void)
{
	struct mdstat_ent *mdstat = mdstat_read(0, 0);
	struct mdstat_ent *md;
	struct map_ent *map = NULL;
	int require_homehost;
	char sys_hostname[256];
	char *homehost = conf_get_homehost(&require_homehost);

	if (homehost == NULL || strcmp(homehost, "<system>")==0) {
		if (gethostname(sys_hostname, sizeof(sys_hostname)) == 0) {
			sys_hostname[sizeof(sys_hostname)-1] = 0;
			homehost = sys_hostname;
		}
	}

	for (md = mdstat ; md ; md = md->next) {
		struct mdinfo *sra = sysfs_read(-1, md->devnm, GET_DEVS);
		struct mdinfo *sd;

		if (!sra)
			continue;

		for (sd = sra->devs ; sd ; sd = sd->next) {
			char namebuf[100];
			char dn[30];
			int dfd;
			int ok;
			dev_t devid;
			struct supertype *st;
			char *subarray = NULL;
			char *path;
			struct mdinfo *info;

			sprintf(dn, "%d:%d", sd->disk.major, sd->disk.minor);
			dfd = dev_open(dn, O_RDONLY);
			if (dfd < 0)
				continue;
			st = guess_super(dfd);
			if ( st == NULL)
				ok = -1;
			else {
				subarray = get_member_info(md);
				ok = st->ss->load_super(st, dfd, NULL);
			}
			close(dfd);
			if (ok != 0)
				continue;
			if (subarray)
				info = st->ss->container_content(st, subarray);
			else {
				info = xmalloc(sizeof(*info));
				st->ss->getinfo_super(st, info, NULL);
			}
			if (!info)
				continue;

			devid = devnm2devid(md->devnm);
			path = map_dev(major(devid), minor(devid), 0);
			if (path == NULL ||
			    strncmp(path, "/dev/md/", 8) != 0) {
				/* We would really like a name that provides
				 * an MD_DEVNAME for udev.
				 * The name needs to be unique both in /dev/md/
				 * and in this mapfile.
				 * It needs to match what -I or -As would come
				 * up with.
				 * That means:
				 *   Check if array is in mdadm.conf
				 *        - if so use that.
				 *   determine trustworthy from homehost etc
				 *   find a unique name based on metadata name.
				 *
				 */
				struct mddev_ident *match = conf_match(st, info,
								       NULL, 0,
								       NULL);
				struct stat stb;
				if (match && match->devname && match->devname[0] == '/') {
					path = match->devname;
					if (path[0] != '/') {
						strcpy(namebuf, "/dev/md/");
						strcat(namebuf, path);
						path = namebuf;
					}
				} else {
					int unum = 0;
					char *sep = "_";
					const char *name;
					int conflict = 1;
					if ((homehost == NULL ||
					     st->ss->match_home(st, homehost) != 1) &&
					    st->ss->match_home(st, "any") != 1 &&
					    (require_homehost ||
					     !conf_name_is_free(info->name)))
						/* require a numeric suffix */
						unum = 0;
					else
						/* allow name to be used as-is if no conflict */
						unum = -1;
					name = info->name;
					if (!*name) {
						name = st->ss->name;
						if (!isdigit(name[strlen(name)-1]) &&
						    unum == -1) {
							unum = 0;
							sep = "";
						}
					}
					if (strchr(name, ':')) {
						/* Probably a uniquifying
						 * hostname prefix.  Allow
						 * without a suffix, and strip
						 * hostname if it is us.
						 */
						if (homehost && unum == -1 &&
						    strncmp(name, homehost,
							    strlen(homehost)) == 0 &&
						    name[strlen(homehost)] == ':')
							name += strlen(homehost)+1;
						unum = -1;
					}

					while (conflict) {
						if (unum >= 0)
							sprintf(namebuf, "/dev/md/%s%s%d",
								name, sep, unum);
						else
							sprintf(namebuf, "/dev/md/%s",
								name);
						unum++;
						if (lstat(namebuf, &stb) != 0 &&
						    (map == NULL ||
						     !map_by_name(&map, namebuf+8)))
							conflict = 0;
					}
					path = namebuf;
				}
			}
			map_add(&map, md->devnm,
				info->text_version,
				info->uuid, path);
			st->ss->free_super(st);
			free(info);
			break;
		}
		sysfs_free(sra);
	}
	/* Only trigger a change if we wrote a new map file */
	if (map_write(map))
		for (md = mdstat ; md ; md = md->next) {
			struct mdinfo *sra = sysfs_read(-1, md->devnm,
							GET_VERSION);
			if (sra)
				sysfs_uevent(sra, "change");
			sysfs_free(sra);
		}
	map_free(map);
	free_mdstat(mdstat);
}
