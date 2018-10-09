/*
 * mdadm - manage Linux "md" devices aka RAID arrays.
 *
 * Copyright (C) 2001-2009 Neil Brown <neilb@suse.de>
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
#include	"md_p.h"
#include	"md_u.h"
#include	<sys/wait.h>
#include	<signal.h>
#include	<limits.h>
#include	<syslog.h>

struct state {
	char *devname;
	char devnm[32];	/* to sync with mdstat info */
	unsigned int utime;
	int err;
	char *spare_group;
	int active, working, failed, spare, raid;
	int from_config;
	int from_auto;
	int expected_spares;
	int devstate[MAX_DISKS];
	dev_t devid[MAX_DISKS];
	int percent;
	char parent_devnm[32]; /* For subarray, devnm of parent.
				* For others, ""
				*/
	struct supertype *metadata;
	struct state *subarray;/* for a container it is a link to first subarray
				* for a subarray it is a link to next subarray
				* in the same container */
	struct state *parent;  /* for a subarray it is a link to its container
				*/
	struct state *next;
};

struct alert_info {
	char *mailaddr;
	char *mailfrom;
	char *alert_cmd;
	int dosyslog;
};
static int make_daemon(char *pidfile);
static int check_one_sharer(int scan);
static void alert(char *event, char *dev, char *disc, struct alert_info *info);
static int check_array(struct state *st, struct mdstat_ent *mdstat,
		       int test, struct alert_info *info,
		       int increments, char *prefer);
static int add_new_arrays(struct mdstat_ent *mdstat, struct state **statelist,
			  int test, struct alert_info *info);
static void try_spare_migration(struct state *statelist, struct alert_info *info);
static void link_containers_with_subarrays(struct state *list);

int Monitor(struct mddev_dev *devlist,
	    char *mailaddr, char *alert_cmd,
	    struct context *c,
	    int daemonise, int oneshot,
	    int dosyslog, char *pidfile, int increments,
	    int share)
{
	/*
	 * Every few seconds, scan every md device looking for changes
	 * When a change is found, log it, possibly run the alert command,
	 * and possibly send Email
	 *
	 * For each array, we record:
	 *   Update time
	 *   active/working/failed/spare drives
	 *   State of each device.
	 *   %rebuilt if rebuilding
	 *
	 * If the update time changes, check out all the data again
	 * It is possible that we cannot get the state of each device
	 * due to bugs in the md kernel module.
	 * We also read /proc/mdstat to get rebuild percent,
	 * and to get state on all active devices incase of kernel bug.
	 *
	 * Events are:
	 *    Fail
	 *	An active device had Faulty set or Active/Sync removed
	 *    FailSpare
	 *      A spare device had Faulty set
	 *    SpareActive
	 *      An active device had a reverse transition
	 *    RebuildStarted
	 *      percent went from -1 to +ve
	 *    RebuildNN
	 *      percent went from below to not-below NN%
	 *    DeviceDisappeared
	 *      Couldn't access a device which was previously visible
	 *
	 * if we detect an array with active<raid and spare==0
	 * we look at other arrays that have same spare-group
	 * If we find one with active==raid and spare>0,
	 *  and if we can get_disk_info and find a name
	 *  Then we hot-remove and hot-add to the other array
	 *
	 * If devlist is NULL, then we can monitor everything because --scan
	 * was given.  We get an initial list from config file and add anything
	 * that appears in /proc/mdstat
	 */

	struct state *statelist = NULL;
	struct state *st2;
	int finished = 0;
	struct mdstat_ent *mdstat = NULL;
	char *mailfrom;
	struct alert_info info;
	struct mddev_ident *mdlist;

	if (!mailaddr) {
		mailaddr = conf_get_mailaddr();
		if (mailaddr && ! c->scan)
			pr_err("Monitor using email address \"%s\" from config file\n",
			       mailaddr);
	}
	mailfrom = conf_get_mailfrom();

	if (!alert_cmd) {
		alert_cmd = conf_get_program();
		if (alert_cmd && !c->scan)
			pr_err("Monitor using program \"%s\" from config file\n",
			       alert_cmd);
	}
	if (c->scan && !mailaddr && !alert_cmd && !dosyslog) {
		pr_err("No mail address or alert command - not monitoring.\n");
		return 1;
	}
	info.alert_cmd = alert_cmd;
	info.mailaddr = mailaddr;
	info.mailfrom = mailfrom;
	info.dosyslog = dosyslog;

	if (daemonise) {
		int rv = make_daemon(pidfile);
		if (rv >= 0)
			return rv;
	}

	if (share)
		if (check_one_sharer(c->scan))
			return 1;

	if (devlist == NULL) {
		mdlist = conf_get_ident(NULL);
		for (; mdlist; mdlist = mdlist->next) {
			struct state *st;

			if (mdlist->devname == NULL)
				continue;
			if (strcasecmp(mdlist->devname, "<ignore>") == 0)
				continue;
			st = xcalloc(1, sizeof *st);
			if (mdlist->devname[0] == '/')
				st->devname = xstrdup(mdlist->devname);
			else {
				st->devname = xmalloc(8+strlen(mdlist->devname)+1);
				strcpy(strcpy(st->devname, "/dev/md/"),
				       mdlist->devname);
			}
			st->next = statelist;
			st->devnm[0] = 0;
			st->percent = RESYNC_UNKNOWN;
			st->from_config = 1;
			st->expected_spares = mdlist->spare_disks;
			if (mdlist->spare_group)
				st->spare_group = xstrdup(mdlist->spare_group);
			statelist = st;
		}
	} else {
		struct mddev_dev *dv;

		for (dv = devlist; dv; dv = dv->next) {
			struct state *st = xcalloc(1, sizeof *st);
			mdlist = conf_get_ident(dv->devname);
			st->devname = xstrdup(dv->devname);
			st->next = statelist;
			st->devnm[0] = 0;
			st->percent = RESYNC_UNKNOWN;
			st->expected_spares = -1;
			if (mdlist) {
				st->expected_spares = mdlist->spare_disks;
				if (mdlist->spare_group)
					st->spare_group = xstrdup(mdlist->spare_group);
			}
			statelist = st;
		}
	}

	while (!finished) {
		int new_found = 0;
		struct state *st, **stp;
		int anydegraded = 0;

		if (mdstat)
			free_mdstat(mdstat);
		mdstat = mdstat_read(oneshot ? 0 : 1, 0);
		if (!mdstat)
			mdstat_close();

		for (st = statelist; st; st = st->next)
			if (check_array(st, mdstat, c->test, &info,
					increments, c->prefer))
				anydegraded = 1;

		/* now check if there are any new devices found in mdstat */
		if (c->scan)
			new_found = add_new_arrays(mdstat, &statelist, c->test,
						   &info);

		/* If an array has active < raid && spare == 0 && spare_group != NULL
		 * Look for another array with spare > 0 and active == raid and same spare_group
		 *  if found, choose a device and hotremove/hotadd
		 */
		if (share && anydegraded)
			try_spare_migration(statelist, &info);
		if (!new_found) {
			if (oneshot)
				break;
			else
				mdstat_wait(c->delay);
		}
		c->test = 0;

		for (stp = &statelist; (st = *stp) != NULL; ) {
			if (st->from_auto && st->err > 5) {
				*stp = st->next;
				free(st->devname);
				free(st->spare_group);
				free(st);
			} else
				stp = &st->next;
		}
	}
	for (st2 = statelist; st2; st2 = statelist) {
		statelist = st2->next;
		free(st2);
	}

	if (pidfile)
		unlink(pidfile);
	return 0;
}

static int make_daemon(char *pidfile)
{
	/* Return:
	 * -1 in the forked daemon
	 *  0 in the parent
	 *  1 on error
	 * so a none-negative becomes the exit code.
	 */
	int pid = fork();
	if (pid > 0) {
		if (!pidfile)
			printf("%d\n", pid);
		else {
			FILE *pid_file;
			pid_file=fopen(pidfile, "w");
			if (!pid_file)
				perror("cannot create pid file");
			else {
				fprintf(pid_file,"%d\n", pid);
				fclose(pid_file);
			}
		}
		return 0;
	}
	if (pid < 0) {
		perror("daemonise");
		return 1;
	}
	close(0);
	open("/dev/null", O_RDWR);
	dup2(0, 1);
	dup2(0, 2);
	setsid();
	return -1;
}

static int check_one_sharer(int scan)
{
	int pid, rv;
	FILE *fp;
	char dir[20];
	char path[100];
	struct stat buf;
	sprintf(path, "%s/autorebuild.pid", MDMON_DIR);
	fp = fopen(path, "r");
	if (fp) {
		if (fscanf(fp, "%d", &pid) != 1)
			pid = -1;
		sprintf(dir, "/proc/%d", pid);
		rv = stat(dir, &buf);
		if (rv != -1) {
			if (scan) {
				pr_err("Only one autorebuild process allowed in scan mode, aborting\n");
				fclose(fp);
				return 1;
			} else {
				pr_err("Warning: One autorebuild process already running.\n");
			}
		}
		fclose(fp);
	}
	if (scan) {
		if (mkdir(MDMON_DIR, S_IRWXU) < 0 && errno != EEXIST) {
			pr_err("Can't create autorebuild.pid file\n");
		} else {
			fp = fopen(path, "w");
			if (!fp)
				pr_err("Cannot create autorebuild.pidfile\n");
			else {
				pid = getpid();
				fprintf(fp, "%d\n", pid);
				fclose(fp);
			}
		}
	}
	return 0;
}

static void alert(char *event, char *dev, char *disc, struct alert_info *info)
{
	int priority;

	if (!info->alert_cmd && !info->mailaddr && !info->dosyslog) {
		time_t now = time(0);

		printf("%1.15s: %s on %s %s\n", ctime(&now) + 4,
		       event, dev, disc?disc:"unknown device");
	}
	if (info->alert_cmd) {
		int pid = fork();
		switch(pid) {
		default:
			waitpid(pid, NULL, 0);
			break;
		case -1:
			break;
		case 0:
			execl(info->alert_cmd, info->alert_cmd,
			      event, dev, disc, NULL);
			exit(2);
		}
	}
	if (info->mailaddr && (strncmp(event, "Fail", 4) == 0 ||
			       strncmp(event, "Test", 4) == 0 ||
			       strncmp(event, "Spares", 6) == 0 ||
			       strncmp(event, "Degrade", 7) == 0)) {
		FILE *mp = popen(Sendmail, "w");
		if (mp) {
			FILE *mdstat;
			char hname[256];
			gethostname(hname, sizeof(hname));
			signal(SIGPIPE, SIG_IGN);
			if (info->mailfrom)
				fprintf(mp, "From: %s\n", info->mailfrom);
			else
				fprintf(mp, "From: %s monitoring <root>\n",
					Name);
			fprintf(mp, "To: %s\n", info->mailaddr);
			fprintf(mp, "Subject: %s event on %s:%s\n\n",
				event, dev, hname);

			fprintf(mp,
				"This is an automatically generated mail message from %s\n", Name);
			fprintf(mp, "running on %s\n\n", hname);

			fprintf(mp,
				"A %s event had been detected on md device %s.\n\n", event, dev);

			if (disc && disc[0] != ' ')
				fprintf(mp,
					"It could be related to component device %s.\n\n", disc);
			if (disc && disc[0] == ' ')
				fprintf(mp, "Extra information:%s.\n\n", disc);

			fprintf(mp, "Faithfully yours, etc.\n");

			mdstat = fopen("/proc/mdstat", "r");
			if (mdstat) {
				char buf[8192];
				int n;
				fprintf(mp,
					"\nP.S. The /proc/mdstat file currently contains the following:\n\n");
				while ((n = fread(buf, 1, sizeof(buf),
						  mdstat)) > 0)
					n = fwrite(buf, 1, n, mp);
				fclose(mdstat);
			}
			pclose(mp);
		}
	}

	/* log the event to syslog maybe */
	if (info->dosyslog) {
		/* Log at a different severity depending on the event.
		 *
		 * These are the critical events:  */
		if (strncmp(event, "Fail", 4) == 0 ||
		    strncmp(event, "Degrade", 7) == 0 ||
		    strncmp(event, "DeviceDisappeared", 17) == 0)
			priority = LOG_CRIT;
		/* Good to know about, but are not failures: */
		else if (strncmp(event, "Rebuild", 7) == 0 ||
			 strncmp(event, "MoveSpare", 9) == 0 ||
			 strncmp(event, "Spares", 6) != 0)
			priority = LOG_WARNING;
		/* Everything else: */
		else
			priority = LOG_INFO;

		if (disc && disc[0] != ' ')
			syslog(priority,
			       "%s event detected on md device %s, component device %s", event, dev, disc);
		else if (disc)
			syslog(priority,
			       "%s event detected on md device %s: %s",
			       event, dev, disc);
		else
			syslog(priority,
			       "%s event detected on md device %s",
			       event, dev);
	}
}

static int check_array(struct state *st, struct mdstat_ent *mdstat,
		       int test, struct alert_info *ainfo,
		       int increments, char *prefer)
{
	/* Update the state 'st' to reflect any changes shown in mdstat,
	 * or found by directly examining the array, and return
	 * '1' if the array is degraded, or '0' if it is optimal (or dead).
	 */
	struct { int state, major, minor; } info[MAX_DISKS];
	struct mdinfo *sra = NULL;
	mdu_array_info_t array;
	struct mdstat_ent *mse = NULL, *mse2;
	char *dev = st->devname;
	int fd;
	int i;
	int remaining_disks;
	int last_disk;
	int new_array = 0;
	int retval;
	int is_container = 0;
	unsigned long redundancy_only_flags = 0;

	if (test)
		alert("TestMessage", dev, NULL, ainfo);

	retval = 0;

	fd = open(dev, O_RDONLY);
	if (fd < 0)
		goto disappeared;

	if (st->devnm[0] == 0)
		strcpy(st->devnm, fd2devnm(fd));

	for (mse2 = mdstat; mse2; mse2 = mse2->next)
		if (strcmp(mse2->devnm, st->devnm) == 0) {
			mse2->devnm[0] = 0; /* flag it as "used" */
			mse = mse2;
		}

	if (!mse) {
		/* duplicated array in statelist
		 * or re-created after reading mdstat
		 */
		st->err++;
		goto out;
	}

	if (mse->level == NULL)
		is_container = 1;

	if (!is_container && !md_array_active(fd))
		goto disappeared;

	fcntl(fd, F_SETFD, FD_CLOEXEC);
	if (md_get_array_info(fd, &array) < 0)
		goto disappeared;

	if (!is_container && map_name(pers, mse->level) > 0)
		redundancy_only_flags |= GET_MISMATCH;

	sra = sysfs_read(-1, st->devnm, GET_LEVEL | GET_DISKS | GET_DEVS |
			GET_STATE | redundancy_only_flags);

	if (!sra)
		goto disappeared;

	/* It's much easier to list what array levels can't
	 * have a device disappear than all of them that can
	 */
	if (sra->array.level == 0 || sra->array.level == -1) {
		if (!st->err && !st->from_config)
			alert("DeviceDisappeared", dev, " Wrong-Level", ainfo);
		st->err++;
		goto out;
	}

	/* this array is in /proc/mdstat */
	if (array.utime == 0)
		/* external arrays don't update utime, so
		 * just make sure it is always different. */
		array.utime = st->utime + 1;;

	if (st->err) {
		/* New array appeared where previously had an error */
		st->err = 0;
		st->percent = RESYNC_NONE;
		new_array = 1;
		alert("NewArray", st->devname, NULL, ainfo);
	}

	if (st->utime == array.utime && st->failed == sra->array.failed_disks &&
	    st->working == sra->array.working_disks &&
	    st->spare == sra->array.spare_disks &&
	    (mse == NULL || (mse->percent == st->percent))) {
		if ((st->active < st->raid) && st->spare == 0)
			retval = 1;
		goto out;
	}
	if (st->utime == 0 && /* new array */
	    mse->pattern && strchr(mse->pattern, '_') /* degraded */)
		alert("DegradedArray", dev, NULL, ainfo);

	if (st->utime == 0 && /* new array */ st->expected_spares > 0 &&
	    sra->array.spare_disks < st->expected_spares)
		alert("SparesMissing", dev, NULL, ainfo);
	if (st->percent < 0 && st->percent != RESYNC_UNKNOWN &&
	    mse->percent >= 0)
		alert("RebuildStarted", dev, NULL, ainfo);
	if (st->percent >= 0 && mse->percent >= 0 &&
	    (mse->percent / increments) > (st->percent / increments)) {
		char percentalert[18];
		/*
		 * "RebuildNN" (10 chars) or "RebuildStarted" (15 chars)
		 */

		if((mse->percent / increments) == 0)
			snprintf(percentalert, sizeof(percentalert),
				 "RebuildStarted");
		else
			snprintf(percentalert, sizeof(percentalert),
				 "Rebuild%02d", mse->percent);

		alert(percentalert, dev, NULL, ainfo);
	}

	if (mse->percent == RESYNC_NONE && st->percent >= 0) {
		/* Rebuild/sync/whatever just finished.
		 * If there is a number in /mismatch_cnt,
		 * we should report that.
		 */
		if (sra && sra->mismatch_cnt > 0) {
			char cnt[80];
			snprintf(cnt, sizeof(cnt),
				 " mismatches found: %d (on raid level %d)",
				 sra->mismatch_cnt, sra->array.level);
			alert("RebuildFinished", dev, cnt, ainfo);
		} else
			alert("RebuildFinished", dev, NULL, ainfo);
	}
	st->percent = mse->percent;

	remaining_disks = sra->array.nr_disks;
	for (i = 0; i < MAX_DISKS && remaining_disks > 0; i++) {
		mdu_disk_info_t disc;
		disc.number = i;
		if (md_get_disk_info(fd, &disc) >= 0) {
			info[i].state = disc.state;
			info[i].major = disc.major;
			info[i].minor = disc.minor;
			if (disc.major || disc.minor)
				remaining_disks --;
		} else
			info[i].major = info[i].minor = 0;
	}
	last_disk = i;

	if (mse->metadata_version &&
	    strncmp(mse->metadata_version, "external:", 9) == 0 &&
	    is_subarray(mse->metadata_version+9)) {
		char *sl;
		strcpy(st->parent_devnm, mse->metadata_version + 10);
		sl = strchr(st->parent_devnm, '/');
		if (sl)
			*sl = 0;
	} else
		st->parent_devnm[0] = 0;
	if (st->metadata == NULL && st->parent_devnm[0] == 0)
		st->metadata = super_by_fd(fd, NULL);

	for (i = 0; i < MAX_DISKS; i++) {
		mdu_disk_info_t disc = {0, 0, 0, 0, 0};
		int newstate = 0;
		int change;
		char *dv = NULL;
		disc.number = i;
		if (i < last_disk && (info[i].major || info[i].minor)) {
			newstate = info[i].state;
			dv = map_dev_preferred(info[i].major, info[i].minor, 1,
					       prefer);
			disc.state = newstate;
			disc.major = info[i].major;
			disc.minor = info[i].minor;
		} else
			newstate = (1 << MD_DISK_REMOVED);

		if (dv == NULL && st->devid[i])
			dv = map_dev_preferred(major(st->devid[i]),
					       minor(st->devid[i]), 1, prefer);
		change = newstate ^ st->devstate[i];
		if (st->utime && change && !st->err && !new_array) {
			if ((st->devstate[i]&change) & (1 << MD_DISK_SYNC))
				alert("Fail", dev, dv, ainfo);
			else if ((newstate & (1 << MD_DISK_FAULTY)) &&
				 (disc.major || disc.minor) &&
				 st->devid[i] == makedev(disc.major,
							 disc.minor))
				alert("FailSpare", dev, dv, ainfo);
			else if ((newstate&change) & (1 << MD_DISK_SYNC))
				alert("SpareActive", dev, dv, ainfo);
		}
		st->devstate[i] = newstate;
		st->devid[i] = makedev(disc.major, disc.minor);
	}
	st->active = sra->array.active_disks;
	st->working = sra->array.working_disks;
	st->spare = sra->array.spare_disks;
	st->failed = sra->array.failed_disks;
	st->utime = array.utime;
	st->raid = sra->array.raid_disks;
	st->err = 0;
	if ((st->active < st->raid) && st->spare == 0)
		retval = 1;

 out:
	if (sra)
		sysfs_free(sra);
	if (fd >= 0)
		close(fd);
	return retval;

 disappeared:
	if (!st->err)
		alert("DeviceDisappeared", dev, NULL, ainfo);
	st->err++;
	goto out;
}

static int add_new_arrays(struct mdstat_ent *mdstat, struct state **statelist,
			  int test, struct alert_info *info)
{
	struct mdstat_ent *mse;
	int new_found = 0;
	char *name;

	for (mse = mdstat; mse; mse = mse->next)
		if (mse->devnm[0] && (!mse->level || /* retrieve containers */
				      (strcmp(mse->level, "raid0") != 0 &&
				       strcmp(mse->level, "linear") != 0))) {
			struct state *st = xcalloc(1, sizeof *st);
			mdu_array_info_t array;
			int fd;

			name = get_md_name(mse->devnm);
			if (!name) {
				free(st);
				continue;
			}

			st->devname = xstrdup(name);
			if ((fd = open(st->devname, O_RDONLY)) < 0 ||
			    md_get_array_info(fd, &array) < 0) {
				/* no such array */
				if (fd >= 0)
					close(fd);
				put_md_name(st->devname);
				free(st->devname);
				if (st->metadata) {
					st->metadata->ss->free_super(st->metadata);
					free(st->metadata);
				}
				free(st);
				continue;
			}
			close(fd);
			st->next = *statelist;
			st->err = 1;
			st->from_auto = 1;
			strcpy(st->devnm, mse->devnm);
			st->percent = RESYNC_UNKNOWN;
			st->expected_spares = -1;
			if (mse->metadata_version &&
			    strncmp(mse->metadata_version,
				    "external:", 9) == 0 &&
			    is_subarray(mse->metadata_version+9)) {
				char *sl;
				strcpy(st->parent_devnm,
					mse->metadata_version+10);
				sl = strchr(st->parent_devnm, '/');
				*sl = 0;
			} else
				st->parent_devnm[0] = 0;
			*statelist = st;
			if (test)
				alert("TestMessage", st->devname, NULL, info);
			new_found = 1;
		}
	return new_found;
}

static int get_required_spare_criteria(struct state *st,
				       struct spare_criteria *sc)
{
	int fd;

	if (!st->metadata || !st->metadata->ss->get_spare_criteria) {
		sc->min_size = 0;
		sc->sector_size = 0;
		return 0;
	}

	fd = open(st->devname, O_RDONLY);
	if (fd < 0)
		return 1;
	if (st->metadata->ss->external)
		st->metadata->ss->load_container(st->metadata, fd, st->devname);
	else
		st->metadata->ss->load_super(st->metadata, fd, st->devname);
	close(fd);
	if (!st->metadata->sb)
		return 1;

	st->metadata->ss->get_spare_criteria(st->metadata, sc);
	st->metadata->ss->free_super(st->metadata);

	return 0;
}

static int check_donor(struct state *from, struct state *to)
{
	struct state *sub;

	if (from == to)
		return 0;
	if (from->parent)
		/* Cannot move from a member */
		return 0;
	if (from->err)
		return 0;
	for (sub = from->subarray; sub; sub = sub->subarray)
		/* If source array has degraded subarrays, don't
		 * remove anything
		 */
		if (sub->active < sub->raid)
			return 0;
	if (from->metadata->ss->external == 0)
		if (from->active < from->raid)
			return 0;
	if (from->spare <= 0)
		return 0;
	return 1;
}

static dev_t choose_spare(struct state *from, struct state *to,
			  struct domainlist *domlist, struct spare_criteria *sc)
{
	int d;
	dev_t dev = 0;

	for (d = from->raid; !dev && d < MAX_DISKS; d++) {
		if (from->devid[d] > 0 && from->devstate[d] == 0) {
			struct dev_policy *pol;
			unsigned long long dev_size;
			unsigned int dev_sector_size;

			if (to->metadata->ss->external &&
			    test_partition_from_id(from->devid[d]))
				continue;

			if (sc->min_size &&
			    dev_size_from_id(from->devid[d], &dev_size) &&
			    dev_size < sc->min_size)
				continue;

			if (sc->sector_size &&
			    dev_sector_size_from_id(from->devid[d],
						    &dev_sector_size) &&
			    sc->sector_size != dev_sector_size)
				continue;

			pol = devid_policy(from->devid[d]);
			if (from->spare_group)
				pol_add(&pol, pol_domain,
					from->spare_group, NULL);
			if (domain_test(domlist, pol,
					to->metadata->ss->name) == 1)
			    dev = from->devid[d];
			dev_policy_free(pol);
		}
	}
	return dev;
}

static dev_t container_choose_spare(struct state *from, struct state *to,
				    struct domainlist *domlist,
				    struct spare_criteria *sc, int active)
{
	/* This is similar to choose_spare, but we cannot trust devstate,
	 * so we need to read the metadata instead
	 */
	struct mdinfo *list;
	struct supertype *st = from->metadata;
	int fd = open(from->devname, O_RDONLY);
	int err;
	dev_t dev = 0;

	if (fd < 0)
		return 0;
	if (!st->ss->getinfo_super_disks) {
		close(fd);
		return 0;
	}

	err = st->ss->load_container(st, fd, NULL);
	close(fd);
	if (err)
		return 0;

	if (from == to) {
		/* We must check if number of active disks has not increased
		 * since ioctl in main loop. mdmon may have added spare
		 * to subarray. If so we do not need to look for more spares
		 * so return non zero value */
		int active_cnt = 0;
		struct mdinfo *dp;
		list = st->ss->getinfo_super_disks(st);
		if (!list) {
			st->ss->free_super(st);
			return 1;
		}
		dp = list->devs;
		while (dp) {
			if (dp->disk.state & (1 << MD_DISK_SYNC) &&
			    !(dp->disk.state & (1 << MD_DISK_FAULTY)))
				active_cnt++;
			dp = dp->next;
		}
		sysfs_free(list);
		if (active < active_cnt) {
			/* Spare just activated.*/
			st->ss->free_super(st);
			return 1;
		}
	}

	/* We only need one spare so full list not needed */
	list = container_choose_spares(st, sc, domlist, from->spare_group,
				       to->metadata->ss->name, 1);
	if (list) {
		struct mdinfo *disks = list->devs;
		if (disks)
			dev = makedev(disks->disk.major, disks->disk.minor);
		sysfs_free(list);
	}
	st->ss->free_super(st);
	return dev;
}

static void try_spare_migration(struct state *statelist, struct alert_info *info)
{
	struct state *from;
	struct state *st;
	struct spare_criteria sc;

	link_containers_with_subarrays(statelist);
	for (st = statelist; st; st = st->next)
		if (st->active < st->raid && st->spare == 0 && !st->err) {
			struct domainlist *domlist = NULL;
			int d;
			struct state *to = st;

			if (to->parent_devnm[0] && !to->parent)
				/* subarray monitored without parent container
				 * we can't move spares here */
				continue;

			if (to->parent)
				/* member of a container */
				to = to->parent;

			if (get_required_spare_criteria(to, &sc))
				continue;
			if (to->metadata->ss->external) {
				/* We must make sure there is
				 * no suitable spare in container already.
				 * If there is we don't add more */
				dev_t devid = container_choose_spare(
					to, to, NULL, &sc, st->active);
				if (devid > 0)
					continue;
			}
			for (d = 0; d < MAX_DISKS; d++)
				if (to->devid[d])
					domainlist_add_dev(&domlist,
							   to->devid[d],
							   to->metadata->ss->name);
			if (to->spare_group)
				domain_add(&domlist, to->spare_group);
			/*
			 * No spare migration if the destination
			 * has no domain. Skip this array.
			 */
			if (!domlist)
				continue;
			for (from=statelist ; from ; from=from->next) {
				dev_t devid;
				if (!check_donor(from, to))
					continue;
				if (from->metadata->ss->external)
					devid = container_choose_spare(
						from, to, domlist, &sc, 0);
				else
					devid = choose_spare(from, to, domlist,
							     &sc);
				if (devid > 0 &&
				    move_spare(from->devname, to->devname,
					       devid)) {
					alert("MoveSpare", to->devname,
					      from->devname, info);
					break;
				}
			}
			domain_free(domlist);
		}
}

/* search the statelist to connect external
 * metadata subarrays with their containers
 * We always completely rebuild the tree from scratch as
 * that is safest considering the possibility of entries
 * disappearing or changing.
 */
static void link_containers_with_subarrays(struct state *list)
{
	struct state *st;
	struct state *cont;
	for (st = list; st; st = st->next) {
		st->parent = NULL;
		st->subarray = NULL;
	}
	for (st = list; st; st = st->next)
		if (st->parent_devnm[0])
			for (cont = list; cont; cont = cont->next)
				if (!cont->err && cont->parent_devnm[0] == 0 &&
				    strcmp(cont->devnm, st->parent_devnm) == 0) {
					st->parent = cont;
					st->subarray = cont->subarray;
					cont->subarray = st;
					break;
				}
}

/* Not really Monitor but ... */
int Wait(char *dev)
{
	char devnm[32];
	dev_t rdev;
	char *tmp;
	int rv = 1;
	int frozen_remaining = 3;

	if (!stat_is_blkdev(dev, &rdev))
		return 2;

	tmp = devid2devnm(rdev);
	if (!tmp) {
		pr_err("Cannot get md device name.\n");
		return 2;
	}

	strcpy(devnm, tmp);

	while(1) {
		struct mdstat_ent *ms = mdstat_read(1, 0);
		struct mdstat_ent *e;

		for (e = ms; e; e = e->next)
			if (strcmp(e->devnm, devnm) == 0)
				break;

		if (e && e->percent == RESYNC_NONE) {
			/* We could be in the brief pause before something
			 * starts. /proc/mdstat doesn't show that, but
			 * sync_action does.
			 */
			struct mdinfo mdi;
			char buf[21];

			if (sysfs_init(&mdi, -1, devnm))
				return 2;
			if (sysfs_get_str(&mdi, NULL, "sync_action",
					  buf, 20) > 0 &&
			    strcmp(buf,"idle\n") != 0) {
				e->percent = RESYNC_UNKNOWN;
				if (strcmp(buf, "frozen\n") == 0) {
					if (frozen_remaining == 0)
						e->percent = RESYNC_NONE;
					else
						frozen_remaining -= 1;
				}
			}
		}
		if (!e || e->percent == RESYNC_NONE) {
			if (e && e->metadata_version &&
			    strncmp(e->metadata_version, "external:", 9) == 0) {
				if (is_subarray(&e->metadata_version[9]))
					ping_monitor(&e->metadata_version[9]);
				else
					ping_monitor(devnm);
			}
			free_mdstat(ms);
			return rv;
		}
		free_mdstat(ms);
		rv = 0;
		mdstat_wait(5);
	}
}

static char *clean_states[] = {
	"clear", "inactive", "readonly", "read-auto", "clean", NULL };

int WaitClean(char *dev, int verbose)
{
	int fd;
	struct mdinfo *mdi;
	int rv = 1;
	char devnm[32];

	if (!stat_is_blkdev(dev, NULL))
		return 2;
	fd = open(dev, O_RDONLY);
	if (fd < 0) {
		if (verbose)
			pr_err("Couldn't open %s: %s\n", dev, strerror(errno));
		return 1;
	}

	strcpy(devnm, fd2devnm(fd));
	mdi = sysfs_read(fd, devnm, GET_VERSION|GET_LEVEL|GET_SAFEMODE);
	if (!mdi) {
		if (verbose)
			pr_err("Failed to read sysfs attributes for %s\n", dev);
		close(fd);
		return 0;
	}

	switch(mdi->array.level) {
	case LEVEL_LINEAR:
	case LEVEL_MULTIPATH:
	case 0:
		/* safemode delay is irrelevant for these levels */
		rv = 0;
	}

	/* for internal metadata the kernel handles the final clean
	 * transition, containers can never be dirty
	 */
	if (!is_subarray(mdi->text_version))
		rv = 0;

	/* safemode disabled ? */
	if (mdi->safe_mode_delay == 0)
		rv = 0;

	if (rv) {
		int state_fd = sysfs_open(fd2devnm(fd), NULL, "array_state");
		char buf[20];
		int delay = 5000;

		/* minimize the safe_mode_delay and prepare to wait up to 5s
		 * for writes to quiesce
		 */
		sysfs_set_safemode(mdi, 1);

		/* wait for array_state to be clean */
		while (1) {
			rv = read(state_fd, buf, sizeof(buf));
			if (rv < 0)
				break;
			if (sysfs_match_word(buf, clean_states) <= 4)
				break;
			rv = sysfs_wait(state_fd, &delay);
			if (rv < 0 && errno != EINTR)
				break;
			lseek(state_fd, 0, SEEK_SET);
		}
		if (rv < 0)
			rv = 1;
		else if (ping_monitor(mdi->text_version) == 0) {
			/* we need to ping to close the window between array
			 * state transitioning to clean and the metadata being
			 * marked clean
			 */
			rv = 0;
		} else {
			rv = 1;
			pr_err("Error connecting monitor with %s\n", dev);
		}
		if (rv && verbose)
			pr_err("Error waiting for %s to be clean\n", dev);

		/* restore the original safe_mode_delay */
		sysfs_set_safemode(mdi, mdi->safe_mode_delay);
		close(state_fd);
	}

	sysfs_free(mdi);
	close(fd);

	return rv;
}
