#ifndef IPTRAF_NG_DIRS_H
#define IPTRAF_NG_DIRS_H

// TODO: full rewrite

#include "getpath.h"

/*
 * IPTraf working file and directory definitions
 */


/*** 
 *** Directory definitions.  The definitions in the Makefile now override
 *** these directives.
 ***/

/*
 * The IPTraf working directory
 */

#ifndef WORKDIR
#define WORKDIR		"/var/lib/iptraf-ng"
#endif

#ifndef LOGDIR
#define LOGDIR		"/var/log/iptraf-ng"
#endif

/*
 * Lock directory.
 * 
 * !!!!!!! WARNING !!!!!!!!
 * DO NOT LET THIS REFER TO AN EXISTING/SYSTEM DIRECTORY!!!!  THE LOCK
 * OVERRIDE (iptraf -f) WILL ERASE ALL FILES HERE!
 */

#ifndef LOCKDIR
#define LOCKDIR		"/var/lock/iptraf-ng"
#endif

/***
 *** Directory environment variables.  Overrides built in definitions.
 *** You may suit this to your preferences.
 ***/

/*
 * Environment variable for IPTraf working directory.  Overrides builtin.
 */

#define WORKDIR_ENV     "IPTRAF_WORK_PATH"

/*
 * Environment variable for LOGDIR
 */

#define LOGDIR_ENV      "IPTRAF_LOG_PATH"

/***
 *** Filename definitions.  They depend on the directory definitions
 *** above.
 ***/

/*
 *  The IPTraf instance identification file.  IPTraf is running if this
 *  file is present, and is deleted afterwards.  As of this version, this
 *  file is used to restrict configuration to only the first instance.
 */

#define IPTIDFILE	get_path(T_LOCKDIR, "iptraf.tag")

/*
 * The IPTraf facility identification files.  These are used to identify which
 * facilities are running, allowing only one instance any of them to run
 * on a network interface.
 */

#define IPMONIDFILE		get_path(T_LOCKDIR, "iptraf-ipmon.tag")
#define GSTATIDFILE		get_path(T_LOCKDIR, "iptraf-genstat.tag")
#define DSTATIDFILE		get_path(T_LOCKDIR, "iptraf-detstat.tag")
#define TCPUDPIDFILE		get_path(T_LOCKDIR, "iptraf-tcpudp.tag")
#define LANMONIDFILE		get_path(T_LOCKDIR, "iptraf-lanmon.tag")
#define FLTIDFILE		get_path(T_LOCKDIR, "iptraf-filters.tag")
#define OTHIPFLTIDFILE		get_path(T_LOCKDIR, "iptraf-othipfltchg.tag")
#define PKTSIZEIDFILE		get_path(T_LOCKDIR, "iptraf-packetsize.tag")
#define PROCCOUNTFILE		get_path(T_LOCKDIR, "iptraf-processcount.dat")
#define ITRAFMONCOUNTFILE 	get_path(T_LOCKDIR, "iptraf-itrafmoncount.dat")
#define LANMONCOUNTFILE		get_path(T_LOCKDIR, "iptraf-lanmoncount.dat")
#define PROMISCLISTFILE 	get_path(T_WORKDIR, "iptraf-promisclist.tmp")

#define OTHIPFLNAME	get_path(T_WORKDIR, "othipfilters.dat")

/*
 * The filter data file for other protocols
 */

#define FLTSTATEFILE	get_path(T_WORKDIR, "savedfilters.dat")

/*
 * The IPTraf configuration data file
 */

#define CONFIGFILE	get_path(T_WORKDIR, "iptraf.cfg")

/*
 * The IPTraf log files
 */

#define IPMONLOG	get_path(T_LOGDIR, "ip_traffic")
#define GSTATLOG	get_path(T_LOGDIR, "iface_stats_general.log")
#define DSTATLOG	get_path(T_LOGDIR, "iface_stats_detailed")
#define TCPUDPLOG	get_path(T_LOGDIR, "tcp_udp_services")
#define LANLOG		get_path(T_LOGDIR, "lan_statistics")
#define PKTSIZELOG	get_path(T_LOGDIR, "packet_size")
#define DAEMONLOG	get_path(T_LOGDIR, "daemon.log")


/*
 * The additional TCP/UDP ports file
 */
#define PORTFILE	get_path(T_WORKDIR, "ports.dat")

/*
 * The Ethernet and FDDI host description files
 */

#define ETHFILE		get_path(T_WORKDIR, "ethernet.desc")
#define FDDIFILE	get_path(T_WORKDIR, "fddi.desc")

/*
 * The rvnamed log file
 */
#define RVNDLOGFILE	get_path(T_LOGDIR, "rvnamed-ng.log")

#ifndef PATH_MAX
#define PATH_MAX	4095
#endif

#endif	/* IPTRAF_NG_DIRS_H */
