/*
  Copyright (c) 2004 The Regents of the University of Michigan.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of the University nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _RPC_GSSD_H_
#define _RPC_GSSD_H_

#include <sys/types.h>
#include <sys/queue.h>
#include <gssapi/gssapi.h>
#include <event.h>
#include <stdbool.h>
#include <pthread.h>

#ifndef GSSD_PIPEFS_DIR
#define GSSD_PIPEFS_DIR		NFS_STATEDIR "/rpc_pipefs"
#endif
#define DNOTIFY_SIGNAL		(SIGRTMIN + 3)

#define GSSD_DEFAULT_CRED_DIR			"/tmp"
#define GSSD_USER_CRED_DIR			"/run/user/%U"
#define GSSD_DEFAULT_CRED_PREFIX		"krb5cc"
#define GSSD_DEFAULT_MACHINE_CRED_SUFFIX	"machine"
#define GSSD_DEFAULT_KEYTAB_FILE		"/etc/krb5.keytab"
#define GSSD_SERVICE_NAME			"nfs"
#define RPC_CHAN_BUF_SIZE			32768

/* timeouts are in seconds */
#define MIN_UPCALL_TIMEOUT			5
#define DEF_UPCALL_TIMEOUT			30
#define MAX_UPCALL_TIMEOUT			600

/*
 * The gss mechanisms that we can handle
 */
enum {AUTHTYPE_KRB5, AUTHTYPE_LIPKEY};

extern char		       *keytabfile;
extern char		      **ccachesearch;
extern int			use_memcache;
extern int			root_uses_machine_creds;
extern unsigned int 		context_timeout;
extern unsigned int rpc_timeout;
extern char			*preferred_realm;

struct clnt_info {
	TAILQ_ENTRY(clnt_info)	list;
	int			refcount;
	int			wd;
	bool			scanned;
	char			*name;
	char			*relpath;
	char			*servicename;
	char			*servername;
	int			prog;
	int			vers;
	char			*protocol;
	int			krb5_fd;
	struct event		*krb5_ev;
	int			gssd_fd;
	struct event		*gssd_ev;
	struct			sockaddr_storage addr;
};

struct clnt_upcall_info {
	struct clnt_info 	*clp;
	uid_t			uid;
	int			fd;
	char			*srchost;
	char			*target;
	char			*service;
};

struct upcall_thread_info {
	TAILQ_ENTRY(upcall_thread_info) list;
	pthread_t		tid;
	struct timespec		timeout;
	uid_t			uid;
	int			fd;
	unsigned short		flags;
#define UPCALL_THREAD_CANCELED	0x0001
#define UPCALL_THREAD_WARNED	0x0002
};

void handle_krb5_upcall(struct clnt_info *clp);
void handle_gssd_upcall(struct clnt_info *clp);
void free_upcall_info(struct clnt_upcall_info *info);
void gssd_free_client(struct clnt_info *clp);
int do_error_downcall(int k5_fd, uid_t uid, int err);


#endif /* _RPC_GSSD_H_ */
