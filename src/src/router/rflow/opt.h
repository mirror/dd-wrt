#ifndef	__OPT_H__
#define	__OPT_H__

#include "headers.h"

extern volatile sig_atomic_t daemon_mode;
extern volatile sig_atomic_t display_now;
extern volatile sig_atomic_t signoff_now;

extern volatile int max_clients;
extern pthread_mutex_t max_clients_lock;
extern pthread_mutex_t pndev_lock;	/* /proc/net/dev protection */
extern pthread_attr_t thread_attr_detach;

extern unsigned char default_ttl;

int init_pthread_options();

#endif	/* __OPT_H__ */
