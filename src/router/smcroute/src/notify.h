/* Wrappers for different service monitors */
#ifndef SMCROUTE_NOTIFY_H_
#define SMCROUTE_NOTIFY_H_

#include "config.h"

void    notify_ready(char *pidfn, uid_t uid, gid_t gid);
void    notify_reload(void);

#ifdef HAVE_LIBSYSTEMD
void    systemd_notify_ready(const char *status);
void    systemd_notify_reload(const char *status);
#else
#define systemd_notify_ready(status)
#define systemd_notify_reload(status)
#endif

#endif /* SMCROUTE_NOTIFY_H_ */
