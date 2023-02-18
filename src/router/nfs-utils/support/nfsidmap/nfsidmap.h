/*
 *  nfsidmap.h
 *
 *  nfs idmapping library, primarily for nfs4 client/server kernel idmapping
 *  and for userland nfs4 idmapping by acl libraries.
 *
 *  Copyright (c) 2004 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  J. Bruce Fields <bfields@umich.edu>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the University nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* XXX arbitrary */
#define NFS4_MAX_DOMAIN_LEN 512
typedef enum {
	X509_CERT = 1
} extra_mapping_types;
	
typedef struct _extra_mapping_params {
	void *content;
	int content_type;
	int content_len;
} extra_mapping_params;

typedef void (*nfs4_idmap_log_function_t)(const char *, ...);

int nfs4_init_name_mapping(char *conffile);
void nfs4_term_name_mapping(void);
int nfs4_get_default_domain(char *server, char *domain, size_t len);
int nfs4_uid_to_name(uid_t uid, char *domain, char *name, size_t len);
int nfs4_gid_to_name(gid_t gid, char *domain, char *name, size_t len);
int nfs4_uid_to_owner(uid_t uid, char *domain, char *name, size_t len);
int nfs4_gid_to_group_owner(gid_t gid, char *domain, char *name, size_t len);
int nfs4_name_to_uid(char *name, uid_t *uid);
int nfs4_name_to_gid(char *name, gid_t *gid);
int nfs4_owner_to_uid(char *name, uid_t *uid);
int nfs4_owner_to_gid(char *name, gid_t *gid);
int nfs4_group_owner_to_gid(char *name, gid_t *gid);
int nfs4_gss_princ_to_ids(char *secname, char *princ, uid_t *uid, gid_t *gid);
int nfs4_gss_princ_to_grouplist(char *secname, char *princ, gid_t *groups, int *ngroups);
int nfs4_gss_princ_to_ids_ex(char *secname, char *princ, uid_t *uid, gid_t *gid, extra_mapping_params **ex);
int nfs4_gss_princ_to_grouplist_ex(char *secname, char *princ, gid_t *groups, int *ngroups, extra_mapping_params **ex);
void nfs4_set_debug(int dbg_level, nfs4_idmap_log_function_t dbg_logfunc);
