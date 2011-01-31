/* atmarpd.h - ATMARP demon command interface */

/* Written 1998,1999 by Werner Almesberger, EPFL ICA */

#ifndef _ATMARPD_H
#define _ATMARPD_H

#include <stdint.h>
#include <atm.h>


#define ATMARP_SOCKET_PATH	"/dev/atmarp"	/* it seems awfully silly to
						   put this socket into /dev,
						   but since that's what
						   syslogd and lpd are doing
						   too, ... */

#define ATMARP_DUMP_DIR		"/var/run"	/* ATMARP table file location */
#define ATMARP_DUMP_FILE	"atmarpd.table"	/* ATMARP table file name */
#define	ATMARP_TMP_DUMP_FILE	"~atmarpd.table"/* name during creation */

#define ATF_NULL	0x1000	/* use NULL encapsulation */
#define ATF_ARPSRV	0x2000	/* entry describes ARP server */
#define ATF_NOVC	0x4000	/* query only; do not create a VC */


enum atmarp_req_type {
	art_invalid,		/* catch uninitialized structures */
	art_create,		/* create an interface */
	art_qos,		/* set the default QoS */
	art_set,		/* create or change an entry */
	art_delete,		/* delete an entry */
	art_table,		/* update the ATMARP table file */
	art_query		/* request resolution without VC setup */
};

struct atmarp_req {
	enum atmarp_req_type	type;	/* request type */
	int			itf;	/* interface number; art_create only */
	uint32_t		ip;	/* IP address */
        struct sockaddr_atmsvc	addr;	/* PVC or SVC address */
	int			flags;	/* ARP flags */
	struct atm_qos		qos;	/* requested QOS */
	int			sndbuf;	/* send buffer; 0 if default */
};

#endif
