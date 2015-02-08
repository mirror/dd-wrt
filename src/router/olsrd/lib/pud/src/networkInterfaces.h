#ifndef _PUD_NETWORKINTERFACES_H
#define _PUD_NETWORKINTERFACES_H

/* Plugin includes */

/* OLSR includes */
#include "olsr_types.h"
#include "interfaces.h"
#include "scheduler.h"

/* System includes */
#include <stdbool.h>
#include <net/if.h>

/** A list of TRxTxNetworkInterface objects, used for non-OLSR interfaces */
typedef struct _TRxTxNetworkInterface {
		/** The socket file descriptor for the non-OLSR interface*/
		int socketFd;

		/** The name of the interface */
		char name[IFNAMSIZ + 1];

		/** the socket handler function */
		socket_handler_func handler;

		/** The next TRxTxNetworkInterface in the list */
		struct _TRxTxNetworkInterface * next;
} TRxTxNetworkInterface;

/** A list of TOLSRNetworkInterface objects, used for OLSR interfaces */
typedef struct _TOLSRNetworkInterface {
		/** A pointer to the OLSR interface */
		struct interface_olsr * olsrIntf;

		/** The next TOLSRNetworkInterface in the list */
		struct _TOLSRNetworkInterface * next;
} TOLSRNetworkInterface;

bool createNetworkInterfaces(socket_handler_func rxSocketHandlerFunction,
		socket_handler_func rxSocketHandlerFunctionDownlink);
void closeNetworkInterfaces(void);

unsigned char * getMainIpMacAddress(void);
TRxTxNetworkInterface * getRxNetworkInterfaces(void);
TRxTxNetworkInterface * getTxNetworkInterfaces(void);
int getDownlinkSocketFd(void);
TOLSRNetworkInterface * getOlsrNetworkInterface(struct interface_olsr * olsrIntf);

#endif /* _PUD_NETWORKINTERFACES_H */
