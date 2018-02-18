/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

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

bool createNetworkInterfaces(socket_handler_func rxSocketHandlerFunctionDownlink);
void closeNetworkInterfaces(void);

unsigned char * getMainIpMacAddress(void);
TRxTxNetworkInterface * getTxNetworkInterfaces(void);
int getDownlinkSocketFd(void);

#endif /* _PUD_NETWORKINTERFACES_H */
