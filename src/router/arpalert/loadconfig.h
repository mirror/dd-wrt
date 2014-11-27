/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: loadconfig.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __LOADCONFIG_H__
#define __LOADCONFIG_H__

#define TRUE                1
#define FALSE               0
#define CONFIGFILE_LEN      512

enum {
	CF_MACLIST,
	CF_LOGFILE,
	CF_ACTION,
	CF_LOCKFILE,
	CF_DAEMON,
	CF_RELOAD,
	CF_LOGLEVEL,
	CF_TIMEOUT,
	CF_MAXTH,
	CF_BLACKLST,
	CF_LEASES,
	CF_IF,
	CF_ABUS,
	CF_MAXENTRY,
	CF_DMPWL,
	CF_DMPBL,
	CF_DMPAPP,
	CF_TOOOLD,
	CF_AUTHFILE,
	CF_IGNORE_UNKNOWN,
	CF_DUMP_PAQUET,
	CF_DUMP_PACKET,
	CF_PROMISC,
	CF_ANTIFLOOD_INTER,
	CF_ANTIFLOOD_GLOBAL,
	CF_IGNORE_ME,
	CF_UMASK,
	CF_USER,
	CF_CHROOT,
	CF_USESYSLOG,
	CF_IGNORESELFTEST,
	CF_UNAUTH_TO_METHOD,
	CF_ONLY_ARP,
	CF_DUMP_INTER,

	// mac addr to vendor conversion
	CF_MACCONV_FILE,
	CF_LOG_VENDOR,
	CF_ALERT_VENDOR,
	CF_MOD_VENDOR,

	// module path
	CF_MOD_ALERT,
	// module config string
	CF_MOD_CONFIG,

	// config alerts for logs
	CF_LOG_FLOOD,
	CF_LOG_NEWMAC,
	CF_LOG_NEW,
	CF_LOG_MACCHG,
	CF_LOG_IPCHG,
	CF_LOG_UNAUTH_RQ,
	CF_LOG_BOGON,
	CF_LOG_ABUS,
	CF_LOG_ALLOW,
	CF_LOG_DENY,

	// config alerts for script
	CF_ALERT_FLOOD,
	CF_ALERT_NEWMAC,
	CF_ALERT_NEW,
	CF_ALERT_MACCHG,
	CF_ALERT_IPCHG,
	CF_ALERT_UNAUTH_RQ,
	CF_ALERT_BOGON,
	CF_ALERT_ABUS,
	CF_ALERT_ALLOW,
	CF_ALERT_DENY,
	
	// config alerts for module
	CF_MOD_FLOOD,
	CF_MOD_NEWMAC,
	CF_MOD_NEW,
	CF_MOD_MACCHG,
	CF_MOD_IPCHG,
	CF_MOD_UNAUTH_RQ,
	CF_MOD_BOGON,
	CF_MOD_ABUS,
	CF_MOD_ALLOW,
	CF_MOD_DENY,

	// total number of arguments
	NUM_PARAMS
};

// if true, the data is updated
int flagdump;


/*
 * types:
 *  0: char
 *  1: int
 *  2: boolean
 *  3: octal
 *
 * attrib:
 *  parameter value in config file
 *
 * value:
 *  valeur du parametre de type indefini
 */
typedef struct {
	int		type;
	char		*attrib;
	union {
		char	*string;
		int	integer;
	} valeur;
} config_cell;

config_cell config[NUM_PARAMS];
char config_file[CONFIGFILE_LEN];

// load config file values
void config_load(int, char **);

// can not modify config
void set_end_of_conf(void);

#endif
