/*
 *	Configuration for Linux 2.1/2.2 based systems without Netlink
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#define CONFIG_AUTO_ROUTES
#undef CONFIG_SELF_CONSCIOUS
#undef CONFIG_MULTIPLE_TABLES

#define CONFIG_UNIX_IFACE
#define CONFIG_UNIX_SET
#define CONFIG_UNIX_DONTROUTE
#undef CONFIG_SKIP_MC_BIND
#define CONFIG_LINUX_SCAN

#define CONFIG_LINUX_MC_MREQN
#define CONFIG_ALL_MULTICAST
#define CONFIG_UNNUM_MULTICAST

/*
Link: sysdep/linux
Link: sysdep/unix
 */
