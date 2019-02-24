/*
 * nfs.c
 *
 * Copyright (C) 2019 dd-wrt
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_NFS
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>
#include <samba3.h>

void start_nfs(void)
{
	struct samba3_share *cs, *csnext;
	struct samba3_share *nfsshares;
#ifdef _SC_NPROCESSORS_ONLN
	int cpucount = sysconf(_SC_NPROCESSORS_ONLN);
#else
	int cpucount = 1
#endif
	    if (!nvram_matchi("nfs_enable", 1))
		return;
	FILE *fp = fopen("/tmp/exports", "wb");
	if (fp) {
		nfsshares = getsamba3shares();
		for (cs = nfsshares; cs; cs = csnext) {
			if (!cs->public) {
				csnext = cs->next;
				free(cs);
				continue;	// if its not public, ignore until we implemented kerberos based user management for NFS4.
			}
			// we export only to local lan network now for security reasons. so know what you're doing
			fprintf(fp, "%s/%s\t%s/%d%s\n", cs->mp, cs->sd, nvram_safe_get("lan_ipaddr"), getmask(nvram_safe_get("lan_netmask")), !strcmp(cs->access_perms, "ro") ? "(ro,no_subtree_check,no_root_squash,fsid=0)" : "(rw,no_subtree_check,no_root_squash,fsid=0)");
			csnext = cs->next;
			free(cs);
		}
		fclose(fp);
	}
	mkdir("/var/lib", 0777);
	mkdir("/var/lib/nfs", 0777);
	mkdir("/var/lib/nfs/v4recovery", 0777);
	//rpc.mountd requires ipv6 support. so load the drivers
	insmod("ipv6");
	insmod("oid_registry");
	insmod("sunrpc");
	insmod("auth_rpcgss");
	insmod("rpcsec_gss_krb5");
	insmod("grace");
	insmod("lockd");
	insmod("exportfs");
	insmod("nfsd");
	eval("rpcbind");
	dd_loginfo("NFSD", "rpcbind successfully started\n");
	eval("rpc.mountd");
	dd_loginfo("NFSD", "NFS Mount Daemon successfully started\n");
	char threads[32];
	sprintf(threads, "%d", cpucount);
	eval("rpc.nfsd", threads);
	dd_loginfo("NFSD", "rpc.nfsd successfully started\n");
	eval("rpc.statd");
	dd_loginfo("NFSD", "NSM Service Daemon successfully started\n");
	eval("exportfs", "-a", "-r");
	return;
}

void stop_nfs(void)
{
	eval("exportfs", "-a", "-u");
}
#endif
#ifdef TEST
int main(int argc, char *argv[])
{
	start_nfs();
}
#endif
