/*
 *  nfsidmap_plugin.h
 *
 *  Essentials functions and structs required when building
 *  plugins for libnfsidmap that are otherwise not exposed
 *  in the public API
 *
 *  Copyright (c) 2004 The Regents of the University of Michigan.
 *  All rights reserved.
 *
 *  Andy Adamson <andros@umich.edu>
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

struct trans_func {
	char *name;
	int (*init)(void);
	int (*princ_to_ids)(char *secname, char *princ, uid_t *uid, gid_t *gid, 
		extra_mapping_params **ex);
	int (*name_to_uid)(char *name, uid_t *uid);
	int (*name_to_gid)(char *name, gid_t *gid);
	int (*uid_to_name)(uid_t uid, char *domain, char *name, size_t len);
	int (*gid_to_name)(gid_t gid, char *domain, char *name, size_t len);
	int (*gss_princ_to_grouplist)(char *secname, char *princ, gid_t *groups,
		int *ngroups, extra_mapping_params **ex);
};

extern int idmap_verbosity;
extern nfs4_idmap_log_function_t idmap_log_func;
struct trans_func *libnfsidmap_plugin_init(void);

/* Level zero always prints, others print depending on verbosity level */
#define IDMAP_LOG(LVL, MSG) \
	do { if (LVL <= idmap_verbosity) (*idmap_log_func)MSG; } while (0)

#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED(foo) UNUSED_ ## foo __attribute__((__unused__))
#else
#define UNUSED(foo) UNUSED_ ## foo
#endif
#endif

extern const char *nfsidmap_conf_path;
extern const char *nfsidmap_config_get(const char *section, const char *tag);

