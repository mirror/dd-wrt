#ifndef NETLINK_H
#define NETLINK_H 1

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <unistd.h>

#ifndef SO_RCVBUFFORCE
#if defined(__alpha__) || defined(__hppa__) || defined(__sparc__) || defined(__sparc_v9__)
#define SO_RCVBUFFORCE 			0x100b
#else
#define SO_RCVBUFFORCE			33
#endif
#endif

#ifndef SO_SNDBUFFORCE
#if defined(__alpha__) || defined(__hppa__) || defined(__sparc__) || defined(__sparc_v9__)
#define SO_SNDBUFFORCE 			0x100a
#else
#define SO_SNDBUFFORCE			32
#endif
#endif

#ifndef NETLINK_KOBJECT_UEVENT
#define NETLINK_KOBJECT_UEVENT		15
#endif

int netlink_init();
int netlink_connect(int);
int netlink_bind(int);

#endif /* NETLINK_H */
