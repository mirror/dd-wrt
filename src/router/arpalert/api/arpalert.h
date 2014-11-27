/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: arpalert.h 690 2008-03-31 18:36:43Z  $
 *
 */

char *arpalert_alert_name[] = {
	"ip_change",
	"unknow_address",
	"black_listed",
	"new",
	"unauthrq",
	"rqabus",
	"mac_error",
	"flood",
	"new_mac",
	"mac_change"
};

/* mod_load function
 * This function may be implemented in your module.
 * Is called when arpalert start
 *
 *   char *config : string for the module set in configuration file
 */
void mod_load(char *config);

/* mod_unlod function
 * This function may be implemented in your module.
 * Is called when arpalert stop.
 */
void mod_unload(void);

/* mod_alert function
 * This function must be implemented in your module.
 * is called for each alert.
 *
 *   int type    : type of alert
 *   int nargs   : number of arguments
 *   void **data : argument list
 *
 * -----------------------------------------
 * alert             args    desc list
 * ------------------------------------------
 * 0 ip_change       4       interface, mac_sender, ip_sender, old_ip
 * 1 unknow_address  3       interface, mac_sender, ip_sender
 * 2 black_listed    3       interface, mac_sender, ip_sender
 * 3 new             3       interface, mac_sender, ip_sender
 * 4 unauthrq        4       interface, mac_sender, ip_sender, ip_requested
 * 5 rqabus          3       interface, mac_sender, ip_sender
 * 6 mac_error       4       interface, mac_sender, ip_sender, mac_in_arp_request
 * 7 flood           3       interface, mac_sender, ip_sender
 * 8 new_mac         3       interface, mac_sender, ip_sender,
 * 9 mac_change      4       interface, mac_sender, ip_sender, old_mac
 *
 * types:
 *   char *interface
 *   struct ether_addr *mac_sender
 *   struct in_addr ip_sender
 *   struct in_addr old_ip
 *   struct in_addr ip_requested
 *   struct ether_addr *mac_in_arp_request
 *   struct ether_addr *old_mac
 *
 */
void mod_alert(int type, int nargs, void **data);

/* logmsg function
 * This function can used for send logmsg with the arpalert log system
 *
 *  int loglevel    : the warn level of the message. this level is
 *                    defined by const LOG_EMERG ... LOG_DEBUG
 *  const char *fmt : format of message
 *  ...             : arguments (see man 3 printf)
 */
#ifndef LOG_EMERG
#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7
#endif
void logmsg(int loglevel, const char *fmt, ...);

/* separe function
 * The module is launched with full rights
 * this function permit to take user config rights 
 */
void separe(void);

/* set_option function
 * set an internal arpalert option
 *
 * int opt     : option number
 * void *value : option value
 */
#define TRUE                1
#define FALSE               0
enum {
	CF_MACLIST,                // maclist file
	CF_LOGFILE,                // log file
	CF_ACTION,                 // action on detect
	CF_LOCKFILE,               // lock file
	CF_DAEMON,                 // daemon
	CF_RELOAD,                 // reload interval
	CF_LOGLEVEL,               // log level
	CF_TIMEOUT,                // execution timeout
	CF_MAXTH,                  // max alert
	CF_BLACKLST,               // maclist alert file
	CF_LEASES,                 // maclist leases file
	CF_IF,                     // interface
	CF_ABUS,                   // max request
	CF_MAXENTRY,               // max entry
	CF_DMPWL,                  // dump white list
	CF_DMPBL,                  // dump black list
	CF_DMPAPP,                 // dump new address
	CF_TOOOLD,                 // mac timeout
	CF_AUTHFILE,               // auth request file
	CF_IGNORE_UNKNOWN,         // ignore unknown sender
	CF_DUMP_PAQUET,            // dump paquet
	CF_DUMP_PACKET,            // dump packet
	CF_PROMISC,                // promiscuous
	CF_ANTIFLOOD_INTER,        // anti flood interval
	CF_ANTIFLOOD_GLOBAL,       // anti flood global
	CF_IGNORE_ME,              // ignore me
	CF_UMASK,                  // umask
	CF_USER,                   // user
	CF_CHROOT,                 // chroot dir
	CF_USESYSLOG,              // use syslog
	CF_IGNORESELFTEST,         // ignore self test
	CF_UNAUTH_TO_METHOD,       // unauth ignore time method
	CF_ONLY_ARP,               // catch only arp
	CF_DUMP_INTER,             // dump inter

	// mac addr to vendor conversion
	CF_MACCONV_FILE,           // mac vendor file
	CF_LOG_VENDOR,             // log mac vendor
	CF_ALERT_VENDOR,           // alert mac vendor
	CF_MOD_VENDOR,             // mod mac vendor

	// module path
	CF_MOD_ALERT,               // mod on detect
	// module config string
	CF_MOD_CONFIG,              // mod config

	// config alerts for logs
	CF_LOG_FLOOD,               // log flood
	CF_LOG_NEWMAC,              // log new mac address
	CF_LOG_NEW,                 // log new address
	CF_LOG_MACCHG,              // log mac change
	CF_LOG_IPCHG,               // log ip change
	CF_LOG_UNAUTH_RQ,           // log unauth request
	CF_LOG_BOGON,               // log mac error
	CF_LOG_ABUS,                // log request abus
	CF_LOG_ALLOW,               // log referenced address
	CF_LOG_DENY,                // log deny address

	// config alerts for script
	CF_ALERT_FLOOD,             // alert on flood
	CF_ALERT_NEWMAC,            // alert on new mac address
	CF_ALERT_NEW,               // alert on new address
	CF_ALERT_MACCHG,            // alert on mac change
	CF_ALERT_IPCHG,             // alert on ip change
	CF_ALERT_UNAUTH_RQ,         // alert on unauth request
	CF_ALERT_BOGON,             // alert on mac error
	CF_ALERT_ABUS,              // alert on request abus
	CF_ALERT_ALLOW,             // alert on referenced address
	CF_ALERT_DENY,              // alert on deny address

	// config alerts for module
	CF_MOD_FLOOD,               // mod on flood
	CF_MOD_NEWMAC,              // mod on new mac address
	CF_MOD_NEW,                 // mod on new address
	CF_MOD_MACCHG,              // mod on mac change
	CF_MOD_IPCHG,               // mod on ip change
	CF_MOD_UNAUTH_RQ,           // mod on unauth request
	CF_MOD_BOGON,               // mod on mac error
	CF_MOD_ABUS,                // mod on request abus
	CF_MOD_ALLOW,               // mod on referenced address
	CF_MOD_DENY,                // mod on deny address

	// total number of arguments
	NUM_PARAMS
};
void set_option(int opt, void *value);

