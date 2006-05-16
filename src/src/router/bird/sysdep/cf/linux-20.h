/*
 *	Configuration for Linux 2.0 based systems
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef CONFIG_AUTO_ROUTES
#undef CONFIG_SELF_CONSCIOUS
#undef CONFIG_MULTIPLE_TABLES

#define CONFIG_UNIX_IFACE
#define CONFIG_UNIX_SET
#define CONFIG_UNIX_DONTROUTE
#undef CONFIG_SKIP_MC_BIND
#define CONFIG_LINUX_SCAN

#define CONFIG_LINUX_MC_MREQ_BIND
#define CONFIG_ALL_MULTICAST
#define CONFIG_UNNUM_MULTICAST

/*
Link: sysdep/linux
Link: sysdep/unix
 */
