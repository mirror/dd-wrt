/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include "cld.h"
#include "sqlite.h"
#include "xlog.h"
#include "legacy.h"

#define NFSD_RECDIR_FILE "/proc/fs/nfsd/nfsv4recoverydir"

/*
 * Loads client records from the v4recovery directory into the database.
 * Records are prefixed with the string "hash:" and include the '\0' byte.
 *
 * Called during database initialization as part of a one-time "upgrade".
 */
void
legacy_load_clients_from_recdir(int *num_records)
{
	int fd;
	DIR *v4recovery;
	struct dirent *entry;
	char recdirname[PATH_MAX+1];
	char buf[NFS4_OPAQUE_LIMIT];
	char *nl;
	ssize_t n;

	fd = open(NFSD_RECDIR_FILE, O_RDONLY);
	if (fd < 0) {
		xlog(D_GENERAL, "Unable to open %s: %m", NFSD_RECDIR_FILE);
		return;
	}
	n = read(fd, recdirname, PATH_MAX);
	close(fd);
	if (n < 0) {
		xlog(D_GENERAL, "Unable to read from %s: %m", NFSD_RECDIR_FILE);
		return;
	}
	/* the output from the proc file isn't null-terminated */
	recdirname[PATH_MAX] = '\0';
	nl = strchr(recdirname, '\n');
	if (!nl)
		return;
	*nl = '\0';
	v4recovery = opendir(recdirname);
	if (!v4recovery)
		return;
	while ((entry = readdir(v4recovery))) {
		int ret;

		/* skip "." and ".." */
		if (entry->d_name[0] == '.') {
			switch (entry->d_name[1]) {
			case '\0':
				continue;
			case '.':
				if (entry->d_name[2] == '\0')
					continue;
			}
		}
		/* prefix legacy records with the string "hash:" */
		ret = snprintf(buf, sizeof(buf), "hash:%s", entry->d_name);
		/* if there's a problem, then skip this entry */
		if (ret < 0 || (size_t)ret >= sizeof(buf)) {
			xlog(L_WARNING, "%s: unable to build client string for %s!",
				__func__, entry->d_name);
			continue;
		}
		/* legacy client records need to include the null terminator */
		ret = sqlite_insert_client((unsigned char *)buf, strlen(buf) + 1);
		if (ret)
			xlog(L_WARNING, "%s: unable to insert %s: %d", __func__,
				entry->d_name, ret);
		else
			(*num_records)++;
	}
	closedir(v4recovery);
}

/*
 * Cleans out the v4recovery directory.
 *
 * Called upon receipt of the first "GraceDone" upcall only.
 */
void
legacy_clear_recdir(void)
{
	int fd;
	DIR *v4recovery;
	struct dirent *entry;
	char recdirname[PATH_MAX+1];
	char dirname[PATH_MAX];
	char *nl;
	ssize_t n;

	fd = open(NFSD_RECDIR_FILE, O_RDONLY);
	if (fd < 0) {
		xlog(D_GENERAL, "Unable to open %s: %m", NFSD_RECDIR_FILE);
		return;
	}
	n = read(fd, recdirname, PATH_MAX);
	close(fd);
	if (n < 0) {
		xlog(D_GENERAL, "Unable to read from %s: %m", NFSD_RECDIR_FILE);
		return;
	}
	/* the output from the proc file isn't null-terminated */
	recdirname[PATH_MAX] = '\0';
	nl = strchr(recdirname, '\n');
	if (!nl)
		return;
	*nl = '\0';
	v4recovery = opendir(recdirname);
	if (!v4recovery)
		return;
	while ((entry = readdir(v4recovery))) {
		int len;

		/* skip "." and ".." */
		if (entry->d_name[0] == '.') {
			switch (entry->d_name[1]) {
			case '\0':
				continue;
			case '.':
				if (entry->d_name[2] == '\0')
					continue;
			}
		}
		len = snprintf(dirname, sizeof(dirname), "%s/%s", recdirname,
				entry->d_name);
		/* if there's a problem, then skip this entry */
		if (len < 0 || (size_t)len >= sizeof(dirname)) {
			xlog(L_WARNING, "%s: unable to build filename for %s!",
				__func__, entry->d_name);
			continue;
		}
		len = rmdir(dirname);
		if (len)
			xlog(L_WARNING, "%s: unable to rmdir %s: %d", __func__,
				dirname, len);
	}
	closedir(v4recovery);
}
