/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <utmp.h>
#include "init.h"
#include "quota.h"

#define SECONDS_IN_A_DAY	(24 * 60 * 60)
#define SECONDS_IN_A_HOUR	(60 * 60)
#define SECONDS_IN_A_MINUTE	(60)

char *
time_to_string(
	time_t		origin,
	uint		flags)
{
	static char	timestamp[32];
	time_t		now, timer;
	uint		days, hours, minutes, seconds;

	if (flags & ABSOLUTE_FLAG) {
		timer = origin;
	} else {
		time(&now);
		timer = MAX(origin - now, 0);
	}
	if (timer > 60)	/* roundup */
		timer += 30;

	days = timer / SECONDS_IN_A_DAY;
	if (days)
		timer %= SECONDS_IN_A_DAY;
	hours = timer / SECONDS_IN_A_HOUR;
	if (hours)
		timer %= SECONDS_IN_A_HOUR;
	minutes = timer / SECONDS_IN_A_MINUTE;
	seconds = timer % SECONDS_IN_A_MINUTE;

	if (flags & LIMIT_FLAG) {
		snprintf(timestamp, sizeof(timestamp), (flags & HUMAN_FLAG) ?
			 _("[-none-]") : _("[--none--]"));
	} else if (origin == 0) {
		snprintf(timestamp, sizeof(timestamp), (flags & HUMAN_FLAG) ?
			 _("[------]") : _("[--------]"));
	} else if ((hours == 0 && minutes == 0 && seconds == 0) ||
		  (!(flags & VERBOSE_FLAG) && days > 0)) {
		snprintf(timestamp, sizeof(timestamp), "[%u %s]",
			 days, days == 1 ? _("day") : _("days"));
	} else if (flags & VERBOSE_FLAG) {
		snprintf(timestamp, sizeof(timestamp), "[%u %s %02u:%02u:%02u]",
			 days, days == 1 ? _("day") : _("days"),
			 hours, minutes, seconds);
	} else { /* non-verbose, less than a day remaining */
		snprintf(timestamp, sizeof(timestamp),
			 (flags & HUMAN_FLAG) ?
				"%02u:%02u:%02u" : "[%02u:%02u:%02u]",
			 hours, minutes, seconds);
	}
	return timestamp;
}

static int
round_snprintf(
	char		*sp,
	size_t		size,
	const char	*fmt_round,
	const char	*fmt_not_round,
	__uint64_t	value,
	__uint64_t	divisor)
{
	double		v = (double)value / divisor;

	value /= divisor;
	if (v == (double)value)
		return snprintf(sp, size, fmt_round, (uint)value);
	else
		return snprintf(sp, size, fmt_not_round, v);
}

/* Basic blocks (512) bytes are returned from quotactl */
#define BBS_TO_EXABYTES(bbs)	((__uint64_t)(bbs)>>51)
#define BBS_TO_PETABYTES(bbs)	((__uint64_t)(bbs)>>41)
#define BBS_TO_TERABYTES(bbs)	((__uint64_t)(bbs)>>31)
#define BBS_TO_GIGABYTES(bbs)	((__uint64_t)(bbs)>>21)
#define BBS_TO_MEGABYTES(bbs)	((__uint64_t)(bbs)>>11)
#define BBS_TO_KILOBYTES(bbs)	((__uint64_t)(bbs)>>1)

#define BBEXABYTE		((__uint64_t)1<<51)
#define BBPETABYTE		((__uint64_t)1<<41)
#define BBTERABYTE		((__uint64_t)1<<31)
#define BBGIGABYTE		((__uint64_t)1<<21)
#define BBMEGABYTE		((__uint64_t)1<<11)
#define BBKILOBYTE		((__uint64_t)1<< 1)

char *
bbs_to_string(
	__uint64_t	v,
	char		*sp,
	uint		size)
{
	if (v == 0)
		snprintf(sp, size, "%4u", (uint)v);
	else if (BBS_TO_EXABYTES(v))
		round_snprintf(sp, size, "%3uE", "%3.1fE", v, BBEXABYTE);
	else if (BBS_TO_PETABYTES(v))
		round_snprintf(sp, size, "%3uP", "%3.1fP", v, BBPETABYTE);
	else if (BBS_TO_TERABYTES(v))
		round_snprintf(sp, size, "%3uT", "%3.1fT", v, BBTERABYTE);
	else if (BBS_TO_GIGABYTES(v))
		round_snprintf(sp, size, "%3uG", "%3.1fG", v, BBGIGABYTE);
	else if (BBS_TO_MEGABYTES(v))
		round_snprintf(sp, size, "%3uM", "%3.1fM", v, BBMEGABYTE);
	else if (BBS_TO_KILOBYTES(v))
		round_snprintf(sp, size, "%3uK", "%3.1fK", v, BBKILOBYTE);
	else
		snprintf(sp, size, "%4u", (uint)v << BBSHIFT);	/* bytes */
	return sp;
}

#define THOUSAND		((__uint64_t)1000)
#define MILLION			((__uint64_t)1000*1000)
#define BILLION			((__uint64_t)1000*1000*1000)
#define TRILLION		((__uint64_t)1000*1000*1000*1000)
#define GAZILLION		((__uint64_t)1000*1000*1000*1000*1000)
#define RIDICULOUS		((__uint64_t)1000*1000*1000*1000*1000*1000)
#define STOPALREADY		((__uint64_t)1000*1000*1000*1000*1000*1000*1000)

char *
num_to_string(
	__uint64_t	v,
	char		*sp,
	uint		size)
{
	if (v == 0)
		snprintf(sp, size, "%4u", (uint)v);
	else if (v > STOPALREADY)
		round_snprintf(sp, size, "%3us", "%3.1fs", v, STOPALREADY);
	else if (v > RIDICULOUS)
		round_snprintf(sp, size, "%3ur", "%3.1fr", v, RIDICULOUS);
	else if (v > GAZILLION)
		round_snprintf(sp, size, "%3ug", "%3.1fg", v, GAZILLION);
	else if (v > TRILLION)
		round_snprintf(sp, size, "%3ut", "%3.1ft", v, TRILLION);
	else if (v > BILLION)
		round_snprintf(sp, size, "%3ub", "%3.1fb", v, BILLION);
	else if (v > MILLION)
		round_snprintf(sp, size, "%3um", "%3.1fm", v, MILLION);
	else if (v > THOUSAND)
		round_snprintf(sp, size, "%3uk", "%3.1fk", v, THOUSAND);
	else
		snprintf(sp, size, "%4u", (uint)v);
	return sp;
}

char *
pct_to_string(
	__uint64_t	portion,
	__uint64_t	whole,
	char		*buf,
	uint		size)
{
	uint		percent;

	percent = whole ? (uint) (100.0 * portion / whole + 0.5) : 0;
	if (snprintf(buf, size, "%3u", percent) < 0)
		return "???";

	return buf;
}

char *
form_to_string(
	uint		form)
{
	char	*forms[] = {
		_("Blocks"), _("Inodes"), _("Realtime Blocks") };

	if (form & XFS_BLOCK_QUOTA)
		return forms[0];
	if (form & XFS_INODE_QUOTA)
		return forms[1];
	if (form & XFS_RTBLOCK_QUOTA)
		return forms[2];
	return NULL;
}

char *
type_to_string(
	uint		type)
{
	char	*types[] = { _("User"), _("Group"), _("Project") };

	if (type & XFS_USER_QUOTA)
		return types[0];
	if (type & XFS_GROUP_QUOTA)
		return types[1];
	if (type & XFS_PROJ_QUOTA)
		return types[2];
	return NULL;
}


/*
 * Identifier caches - user/group/project names/IDs
 */

#define NID		4096
#define IDMASK		(NID-1)

typedef struct {
	__uint32_t	id;
	char		name[NMAX+1];
} idcache_t;

static idcache_t	uidnc[NID];
static idcache_t	gidnc[NID];
static idcache_t	pidnc[NID];
static int		uentriesleft = NID;
static int		gentriesleft = NID;
static int		pentriesleft = NID;

static idcache_t *
getnextpwent(
	__uint32_t	id,
	int		byid)
{
	struct passwd	*pw;
	static idcache_t idc;

	/* /etc/passwd */
	if ((pw = byid? getpwuid(id) : getpwent()) == NULL)
		return NULL;
	idc.id = pw->pw_uid;
	strncpy(idc.name, pw->pw_name, NMAX);
	return &idc;
}

static idcache_t *
getnextgrent(
	__uint32_t	id,
	int		byid)
{
	struct group	*gr;
	static idcache_t idc;

	if ((gr = byid? getgrgid(id) : getgrent()) == NULL)
		return NULL;
	idc.id = gr->gr_gid;
	strncpy(idc.name, gr->gr_name, NMAX);
	return &idc;
}

static idcache_t *
getnextprent(
	__uint32_t	id,
	int		byid)
{
	fs_project_t	*pr;
	static idcache_t idc;

	if ((pr = byid? getprprid(id) : getprent()) == NULL)
		return NULL;
	idc.id = pr->pr_prid;
	strncpy(idc.name, pr->pr_name, NMAX);
	return &idc;
}

char *
uid_to_name(
	__uint32_t	id)
{
	idcache_t	*ncp, *idp;

	/* Check cache for name first */
	ncp = &uidnc[id & IDMASK];
	if (ncp->id == id && ncp->name[0])
		return ncp->name;
	if (uentriesleft) {
		/*
		 * Fill this cache while seaching for a name.
		 * This lets us run through the file serially.
		 */
		if (uentriesleft == NID)
			setpwent();
		while (((idp = getnextpwent(id, 0)) != NULL) && uentriesleft) {
			uentriesleft--;
			ncp = &uidnc[idp->id & IDMASK];
			if (ncp->name[0] == '\0' || idp->id == id)
				memcpy(ncp, idp, sizeof(idcache_t));
			if (idp->id == id)
				return ncp->name;
		}
		endpwent();
		uentriesleft = 0;
		ncp = &uidnc[id & IDMASK];
	}

	/* Not cached - do it the slow way & insert into cache */
	if ((idp = getnextpwent(id, 1)) == NULL)
		return NULL;
	memcpy(ncp, idp, sizeof(idcache_t));
	return ncp->name;
}

char *
gid_to_name(
	__uint32_t	id)
{
	idcache_t	*ncp, *idp;

	/* Check cache for name first */
	ncp = &gidnc[id & IDMASK];
	if (ncp->id == id && ncp->name[0])
		return ncp->name;
	if (gentriesleft) {
		/*
		 * Fill this cache while seaching for a name.
		 * This lets us run through the file serially.
		 */
		if (gentriesleft == NID)
			setgrent();
		while (((idp = getnextgrent(id, 0)) != NULL) && gentriesleft) {
			gentriesleft--;
			ncp = &gidnc[idp->id & IDMASK];
			if (ncp->name[0] == '\0' || idp->id == id)
				memcpy(ncp, idp, sizeof(idcache_t));
			if (idp->id == id)
				return ncp->name;
		}
		endgrent();
		gentriesleft = 0;
		ncp = &gidnc[id & IDMASK];
	}

	/* Not cached - do it the slow way & insert into cache */
	if ((idp = getnextgrent(id, 1)) == NULL)
		return NULL;
	memcpy(ncp, idp, sizeof(idcache_t));
	return ncp->name;
}

char *
prid_to_name(
	__uint32_t	id)
{
	idcache_t	*ncp, *idp;

	/* Check cache for name first */
	ncp = &pidnc[id & IDMASK];
	if (ncp->id == id && ncp->name[0])
		return ncp->name;
	if (pentriesleft) {
		/*
		 * Fill this cache while seaching for a name.
		 * This lets us run through the file serially.
		 */
		if (pentriesleft == NID)
			setprent();
		while (((idp = getnextprent(id, 0)) != NULL) && pentriesleft) {
			pentriesleft--;
			ncp = &pidnc[idp->id & IDMASK];
			if (ncp->name[0] == '\0' || idp->id == id)
				memcpy(ncp, idp, sizeof(idcache_t));
			if (idp->id == id)
				return ncp->name;
		}
		endprent();
		pentriesleft = 0;
		ncp = &pidnc[id & IDMASK];
	}

	/* Not cached - do it the slow way & insert into cache */
	if ((idp = getnextprent(id, 1)) == NULL)
		return NULL;
	memcpy(ncp, idp, sizeof(idcache_t));
	return ncp->name;
}


/*
 * Utility routine for opening an output file so that it can
 * be "securely" written to (i.e. without vulnerability to a
 * symlink attack).
 *
 * Returns NULL on failure, stdout on NULL input.
 */
FILE *
fopen_write_secure(
	char		*fname)
{
	FILE		*fp;
	int		fd;

	if (!fname)
		return stdout;

	if ((fd = open(fname, O_CREAT|O_WRONLY|O_EXCL, 0600)) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: open on %s failed: %s\n"),
			progname, fname, strerror(errno));
		return NULL;
	}
	if ((fp = fdopen(fd, "w")) == NULL) {
		exitcode = 1;
		fprintf(stderr, _("%s: fdopen on %s failed: %s\n"),
			progname, fname, strerror(errno));
		close(fd);
		return NULL;
	}
	return fp;
}
