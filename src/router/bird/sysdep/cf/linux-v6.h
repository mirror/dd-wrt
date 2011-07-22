/*
 *	Configuration for Linux 2.2 based systems running IPv6
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#define IPV6

#define CONFIG_AUTO_ROUTES
#define CONFIG_ALL_MULTICAST
#define CONFIG_SELF_CONSCIOUS

/*
 *  Netlink supports multiple tables, but kernel IPv6 code doesn't, so we
 *  treat it as a multiple table system with number of tables set to 1.
 */
#define CONFIG_MULTIPLE_TABLES
#define CONFIG_ALL_TABLES_AT_ONCE

#define CONFIG_RESTRICTED_PRIVILEGES

/*
Link: sysdep/linux/netlink
Link: sysdep/linux
Link: sysdep/unix
 */
