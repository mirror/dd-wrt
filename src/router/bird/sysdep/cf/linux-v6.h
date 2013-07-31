/*
 *	Configuration for Linux based systems running IPv6
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#define IPV6

#define CONFIG_AUTO_ROUTES
#define CONFIG_SELF_CONSCIOUS
#define CONFIG_MULTIPLE_TABLES
#define CONFIG_ALL_TABLES_AT_ONCE

#define CONFIG_RESTRICTED_PRIVILEGES

/*
Link: sysdep/linux
Link: sysdep/unix
 */
