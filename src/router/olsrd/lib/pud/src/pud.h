#ifndef _PUD_PUD_H
#define _PUD_PUD_H

/* Plugin includes */

/* OLSR includes */
#include "olsr_protocol.h"
#include "interfaces.h"
#include "olsr_types.h"

/* System includes */
#include <stdbool.h>

/*
 * Global
 */

/** The long plugin name */
#define PUD_PLUGIN_NAME_LONG			"OLSRD Position Update Distribution (PUD) plugin"

/** The short plugin name / abbreviation */
#define PUD_PLUGIN_ABBR					"PUD"

/*
 *  Interface
 */

bool initPud(void);

void closePud(void);

void pudError(bool useErrno, const char *format, ...) __attribute__ ((format(printf, 2, 3)));

bool packetReceivedFromOlsr(union olsr_message *olsrMessage,
		struct interface *in_if __attribute__ ((unused)),
		union olsr_ip_addr *ipaddr __attribute__ ((unused)));

#endif /* _PUD_PUD_H */
