/*
 *	Configuration for Linux 2.2 based systems
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#define CONFIG_AUTO_ROUTES
#define CONFIG_SELF_CONSCIOUS
#define CONFIG_MULTIPLE_TABLES
#define CONFIG_ALL_TABLES_AT_ONCE
#define CONFIG_MC_PROPER_SRC

#undef CONFIG_SKIP_MC_BIND

#define CONFIG_LINUX_MC_MREQN
#define CONFIG_UNIX_DONTROUTE

/*
Link: sysdep/linux/netlink
Link: sysdep/linux
Link: sysdep/unix
 */
