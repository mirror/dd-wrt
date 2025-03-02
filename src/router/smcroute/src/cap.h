/* Daemon capability API */
#ifndef SMCROUTE_CAP_H_
#define SMCROUTE_CAP_H_

#include "config.h"

#ifdef ENABLE_LIBCAP
void cap_drop_root (uid_t uid, gid_t gid);
void cap_set_user  (char *arg, uid_t *uid, gid_t *gid);
#else
#define cap_drop_root(uid, gid)
#define cap_set_user(arg, uid, gid)  warnx("Drop privs support not available.")
#endif

#endif /* SMCROUTE_CAP_H_ */
