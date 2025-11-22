/*
 * nfs.c
 *
 * Copyright (C) 2019 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <ddnvram.h>
#include <shutils.h>
#include <services.h>
#include <nfs.h>

void start_nfs(void)
{
	struct nfs_share *cs, *csnext;
	struct nfs_share *nfsshares;
	int cpucount = getlogicalcores();
	if (!nvram_matchi("nfs_enable", 1))
		return;
	FILE *fp = fopen("/tmp/exports", "wb");
	if (fp) {
		nfsshares = getnfsshares();
		for (cs = nfsshares; cs; cs = csnext) {
			// we export only to local lan network now for security reasons. so know what you're doing
			if (*cs->mp && *cs->allowed) {
				fprintf(fp, "%s/%s\t%s%s\n", cs->mp, cs->sd, cs->allowed,
					!strcmp(cs->access_perms, "ro") ? "(ro,no_subtree_check,no_root_squash,sync)" :
									  "(rw,no_subtree_check,no_root_squash,sync)");
			}
			csnext = cs->next;
			free(cs);
		}
		fclose(fp);
	}
	fp = fopen("/tmp/nfs.conf", "wb");
	if (fp) {
		fprintf(fp, "[general]\n");
		fprintf(fp, "# pipefs-directory=/var/lib/nfs/rpc_pipefs\n");
		fprintf(fp, "[nfsrahead]\n");
		fprintf(fp, "nfs=15000\n");
		fprintf(fp, "nfs4=16000\n");
		fprintf(fp, "[exports]\n");
		fprintf(fp, "# rootdir=/export\n");
		fprintf(fp, "#\n");
		fprintf(fp, "[exportfs]\n");
		fprintf(fp, "debug=0\n");
		fprintf(fp, "[gssd]\n");
		fprintf(fp, "verbosity=0\n");
		fprintf(fp, "rpc-verbosity=0\n");
		fprintf(fp, "use-memcache=0\n");
		fprintf(fp, "use-machine-creds=1\n");
		fprintf(fp, "use-gss-proxy=0\n");
		fprintf(fp, "avoid-dns=1\n");
		fprintf(fp, "limit-to-legacy-enctypes=0\n");
		fprintf(fp,
			"allowed-enctypes=aes256-cts-hmac-sha384-192,aes128-cts-hmac-sha256-128,camellia256-cts-cmac,camellia128-cts-cmac,aes256-cts-hmac-sha1-96,aes128-cts-hmac-sha1-96\n");
		fprintf(fp, "context-timeout=0\n");
		fprintf(fp, "rpc-timeout=5\n");
		fprintf(fp, "keytab-file=/etc/krb5.keytab\n");
		//fprintf(fp, "cred-cache-directory=\n");
		//fprintf(fp, "preferred-realm=\n");
		fprintf(fp, "set-home=1\n");
		fprintf(fp, "upcall-timeout=30\n");
		fprintf(fp, "cancel-timed-out-upcalls=0\n");
		fprintf(fp, "[lockd]\n");
		fprintf(fp, "port=0\n");
		fprintf(fp, "udp-port=0\n");
		fprintf(fp, "[exportd]\n");
		fprintf(fp, "debug=\"all|auth|call|general|parse\"\n");
		fprintf(fp, "manage-gids=n\n");
		fprintf(fp, "state-directory-path=/var/lib/nfs\n");
		fprintf(fp, "threads=1\n");
		fprintf(fp, "cache-use-ipaddr=n\n");
		fprintf(fp, "ttl=1800\n");
		fprintf(fp, "[mountd]\n");
		fprintf(fp, "debug=\"all|auth|call|general|parse\"\n");
		fprintf(fp, "manage-gids=n\n");
		fprintf(fp, "descriptors=0\n");
		fprintf(fp, "port=0\n");
		fprintf(fp, "threads=1\n");
		fprintf(fp, "reverse-lookup=n\n");
		fprintf(fp, "state-directory-path=/var/lib/nfs\n");
		//fprintf(fp, "ha-callout=\n");
		fprintf(fp, "cache-use-ipaddr=n\n");
		fprintf(fp, "ttl=1800\n");
		fprintf(fp, "[nfsdcld]\n");
		fprintf(fp, "debug=0\n");
		//fprintf(fp, "storagedir=/var/lib/nfs/nfsdcld\n");
		fprintf(fp, "[nfsd]\n");
		fprintf(fp, "debug=0\n");
		fprintf(fp, "threads=16\n");
		//fprintf(fp, "host=\n");
		fprintf(fp, "port=0\n");
		fprintf(fp, "grace-time=90\n");
		fprintf(fp, "lease-time=90\n");
		fprintf(fp, "udp=y\n");
		fprintf(fp, "tcp=y\n");
		fprintf(fp, "vers3=y\n");
		fprintf(fp, "vers4=y\n");
		fprintf(fp, "vers4.0=y\n");
		fprintf(fp, "vers4.1=y\n");
		fprintf(fp, "vers4.2=y\n");
		fprintf(fp, "rdma=y\n");
		fprintf(fp, "rdma-port=20049\n");
		fprintf(fp, "[statd]\n");
		fprintf(fp, "debug=0\n");
		fprintf(fp, "port=0\n");
		fprintf(fp, "outgoing-port=0\n");
		//fprintf(fp, "name=\n");
		//fprintf(fp, "state-directory-path=/var/lib/nfs/statd\n");
		//fprintf(fp, "ha-callout=\n");
		fprintf(fp, "no-notify=0\n");
		fprintf(fp, "[sm-notify]\n");
		fprintf(fp, "debug=0\n");
		fprintf(fp, "force=0\n");
		fprintf(fp, "retry-time=900\n");
		//fprintf(fp, "outgoing-port=\n");
		//fprintf(fp, "outgoing-addr=\n");
		fprintf(fp, "lift-grace=y\n");
		fprintf(fp, "[svcgssd]\n");
		//fprintf(fp, "principal=\n");
		fclose(fp);
	}
	mkdir("/var/lib", 0777);
	mkdir("/var/lib/nfs", 0777);
	mkdir("/var/lib/nfs/v4recovery", 0777);
	mkdir("/var/lib/nfs/sm", 0777);
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
	if (pidof("rpcbind") <= 0)
		log_eval("rpcbind");
	if (pidof("fsidd") <= 0) {
		int pid;
		char *argv[] = { "fsidd", NULL };
		_log_evalpid(argv, NULL, 0, &pid);
	}
	if (pidof("rpc.mountd") <= 0)
		log_eval("rpc.mountd");
	char threads[32];
	sprintf(threads, "%d", cpucount);
	if (pidof("rpc.nfsd") <= 0)
		log_eval("rpc.nfsd", threads);
	if (pidof("rpc.statd") <= 0)
		log_eval("rpc.statd");
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
