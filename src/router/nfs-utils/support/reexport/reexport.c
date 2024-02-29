#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <sys/types.h>
#include <sys/vfs.h>
#include <errno.h>

#include "nfsd_path.h"
#include "conffile.h"
#include "nfslib.h"
#include "reexport.h"
#include "xcommon.h"
#include "xlog.h"

static int fsidd_srv = -1;

static bool connect_fsid_service(void)
{
	struct sockaddr_un addr;
	char *sock_file;
	int ret;
	int s;

	if (fsidd_srv != -1)
		return true;

	sock_file = conf_get_str_with_def("reexport", "fsidd_socket", FSID_SOCKET_NAME);

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, sock_file, sizeof(addr.sun_path) - 1);
	if (addr.sun_path[0] == '@')
		/* "abstract" socket namespace */
		addr.sun_path[0] = 0;

	s = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (s == -1) {
		xlog(L_WARNING, "Unable to create AF_UNIX socket for %s: %m\n", sock_file);
		return false;
	}

	ret = connect(s, (const struct sockaddr *)&addr, sizeof(struct sockaddr_un));
	if (ret == -1) {
		xlog(L_WARNING, "Unable to connect %s: %m, is fsidd running?\n", sock_file);
		return false;
	}

	fsidd_srv = s;

	return true;
}

int reexpdb_init(void)
{
	int try_count = 3;

	while (try_count > 0 && !connect_fsid_service()) {
		sleep(1);
		try_count--;
	}

	return try_count > 0;
}

void reexpdb_destroy(void)
{
	close(fsidd_srv);
	fsidd_srv = -1;
}

static bool parse_fsidd_reply(const char *cmd_info, char *buf, size_t len, char **result)
{
	if (len == 0) {
		xlog(L_WARNING, "Unable to read %s result: server closed the connection", cmd_info);
		return false;
	} else if (len < 2) {
		xlog(L_WARNING, "Unable to read %s result: server sent too few bytes", cmd_info);
		return false;
	}

	if (buf[0] == '-') {
		if (len > 2) {
			char *reason = buf + 2;
			xlog(L_WARNING, "Command %s failed, server said: %s", cmd_info, reason);
		} else {
			xlog(L_WARNING, "Command %s failed at server side", cmd_info);
		}

		return false;
	}

	if (buf[0] != '+') {
		xlog(L_WARNING, "Unable to read %s result: server sent malformed answer", cmd_info);
		return false;
	}

	if (len > 2) {
		*result = strdup(buf + 2);
	} else {
		*result = NULL;
	}

	return true;
}

static bool do_fsidd_cmd(const char *cmd_info, char *msg, size_t len, char **result)
{
	char recvbuf[1024];
	int n;

	if (fsidd_srv == -1) {
		xlog(L_NOTICE, "Reconnecting to fsid services");
		if (reexpdb_init() == false)
			return false;
	}

	xlog(D_GENERAL, "Request to fsidd: msg=\"%s\" len=%zd", msg, len);

	if (write(fsidd_srv, msg, len) == -1) {
		xlog(L_WARNING, "Unable to send %s command: %m", cmd_info);
		goto out_close;
	}

	n = read(fsidd_srv, recvbuf, sizeof(recvbuf) - 1);
	if (n <= -1) {
		xlog(L_WARNING, "Unable to recv %s answer: %m", cmd_info);
		goto out_close;
	} else if (n == sizeof(recvbuf) - 1) {
		//TODO: use better way to detect truncation
		xlog(L_WARNING, "Unable to recv %s answer: answer truncated", cmd_info);
		goto out_close;
	}
	recvbuf[n] = '\0';

	xlog(D_GENERAL, "Answer from fsidd: msg=\"%s\" len=%i", recvbuf, n);

	if (parse_fsidd_reply(cmd_info, recvbuf, n, result) == false) {
		goto out_close;
	}

	return true;

out_close:
	close(fsidd_srv);
	fsidd_srv = -1;
	return false;
}

static bool fsidnum_get_by_path(char *path, uint32_t *fsidnum, bool may_create)
{
	char *msg, *result;
	bool ret = false;
	int len;

	char *cmd = may_create ? "get_or_create_fsidnum" : "get_fsidnum";

	len = asprintf(&msg, "%s %s", cmd, path);
	if (len == -1) {
		xlog(L_WARNING, "Unable to build %s command: %m", cmd);
		goto out;
	}

	if (do_fsidd_cmd(cmd, msg, len, &result) == false) {
		goto out;
	}

	if (result) {
		bool bad_input = true;
		char *endp;

		errno = 0;
		*fsidnum = strtoul(result, &endp, 10);
		if (errno == 0 && *endp == '\0') {
			bad_input = false;
		}

		free(result);

		if (!bad_input) {
			ret = true;
		} else {
			xlog(L_NOTICE, "Got malformed fsid for path %s", path);
		}
	} else {
		xlog(L_NOTICE, "No fsid found for path %s", path);
	}

out:
	free(msg);
	return ret;
}

static bool path_by_fsidnum(uint32_t fsidnum, char **path)
{
	char *msg, *result;
	bool ret = false;
	int len;

	len = asprintf(&msg, "get_path %d", (unsigned int)fsidnum);
	if (len == -1) {
		xlog(L_WARNING, "Unable to build get_path command: %m");
		goto out;
	}

	if (do_fsidd_cmd("get_path", msg, len, &result) == false) {
		goto out;
	}

	if (result) {
		*path = result;
		ret = true;
	} else {
		xlog(L_NOTICE, "No path found for fsid %u", (unsigned int)fsidnum);
	}

out:
	free(msg);
	return ret;
}

/*
 * reexpdb_fsidnum_by_path - Lookup a fsid by path.
 *
 * @path: File system path used as lookup key
 * @fsidnum: Pointer where found fsid is written to
 * @may_create: If non-zero, allocate new fsid if lookup failed
 *
 */
int reexpdb_fsidnum_by_path(char *path, uint32_t *fsidnum, int may_create)
{
	return fsidnum_get_by_path(path, fsidnum, may_create);
}

/*
 * reexpdb_uncover_subvolume - Make sure a subvolume is present.
 *
 * @fsidnum: Numerical fsid number to look for
 *
 * Subvolumes (NFS cross mounts) get automatically mounted upon first
 * access and can vanish after fs.nfs.nfs_mountpoint_timeout seconds.
 * Also if the NFS server reboots, clients can still have valid file
 * handles for such a subvolume.
 *
 * If kNFSd asks mountd for the path of a given fsidnum it can
 * trigger an automount by calling statfs() on the given path.
 */
void reexpdb_uncover_subvolume(uint32_t fsidnum)
{
	struct statfs st;
	char *path = NULL;
	int ret;

	if (path_by_fsidnum(fsidnum, &path)) {
		ret = nfsd_path_statfs(path, &st);
		if (ret == -1)
			xlog(L_WARNING, "statfs() failed");
	}

	free(path);
}

/*
 * reexpdb_apply_reexport_settings - Apply reexport specific settings to an exportent
 *
 * @ep: exportent to apply to
 * @flname: Current export file, only useful for logging
 * @flline: Current line, only useful for logging
 *
 * This is a helper function for applying reexport specific settings to an exportent.
 * It searches a suitable fsid an sets @ep->e_fsid.
 */
int reexpdb_apply_reexport_settings(struct exportent *ep, char *flname, int flline)
{
	uint32_t fsidnum;
	bool found, is_v4root = ((ep->e_flags & NFSEXP_FSID) && !ep->e_fsid);
	int ret = 0;

	if (ep->e_reexport == REEXP_NONE)
		goto out;

	if (ep->e_uuid)
		goto out;

	if (is_v4root)
		goto out;

	found = reexpdb_fsidnum_by_path(ep->e_path, &fsidnum, 0);
	if (!found) {
		if (ep->e_reexport == REEXP_AUTO_FSIDNUM) {
			found = reexpdb_fsidnum_by_path(ep->e_path, &fsidnum, 1);
			if (!found) {
				xlog(L_ERROR, "%s:%i: Unable to generate fsid for %s",
				     flname, flline, ep->e_path);
				ret = -1;
				goto out;
			}
		} else {
			if (!ep->e_fsid) {
				xlog(L_ERROR, "%s:%i: Selected 'reexport=' mode requires either a UUID 'fsid=' or a numerical 'fsid=' or a reexport db entry %d",
				     flname, flline, ep->e_fsid);
				ret = -1;
			}

			goto out;
		}
	}

	if (ep->e_fsid) {
		if (ep->e_fsid != fsidnum) {
			xlog(L_ERROR, "%s:%i: Selected 'reexport=' mode requires configured numerical 'fsid=' to agree with reexport db entry",
			     flname, flline);
			ret = -1;
		}
	} else {
		ep->e_fsid = fsidnum;
	}

out:
	return ret;
}
