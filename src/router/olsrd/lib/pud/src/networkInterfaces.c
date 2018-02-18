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

#include "networkInterfaces.h"

/* Plugin includes */
#include "pud.h"
#include "configuration.h"
#include "netTools.h"

/* OLSRD includes */
#include "olsr.h"
#include "interfaces.h"

/* System includes */
#include <unistd.h>
#include <fcntl.h>

/*
 * Main IP MAC address
 */

/** the MAC address of the main IP address */
static unsigned char mac[PUD_NODEIDTYPE_MAC_BYTES] = { 0 };

/** true when the MAC address of the main IP address has been retrieved */
static bool macSet = false;

/**
 * @return
 * the MAC address of the main IP address
 */
unsigned char * getMainIpMacAddress(void) {
	if (!macSet) {
		struct ifreq ifr;
		unsigned char * macInIfr;

		struct interface_olsr *mainInterface = if_ifwithaddr(&olsr_cnf->main_addr);
		if (!mainInterface) {
			pudError(true, "Could not get the main interface");
			return NULL;
		}
		macInIfr = getHardwareAddress(mainInterface->int_name, olsr_cnf->ip_version, &ifr);
		if (!macInIfr) {
			pudError(true, "Could not get the MAC address of the main interface");
			return NULL;
		}
		memcpy(&mac[0], &macInIfr[0], PUD_NODEIDTYPE_MAC_BYTES);
		macSet = true;
	}

	return &mac[0];
}

/*
 * TX interfaces
 */

/** The list of network interface objects, sending our NMEA sentences */
static TRxTxNetworkInterface *txNetworkInterfacesListHead = NULL;

/** Pointer to the last network interface object, sending our NMEA sentences */
static TRxTxNetworkInterface *lastTxNetworkInterface = NULL;

/**
 @return
 The list of network interface objects, sending our NMEA sentences
 */
TRxTxNetworkInterface *getTxNetworkInterfaces(void) {
	return txNetworkInterfacesListHead;
}

/**
 Create a transmit socket for a network interface

 @param networkInterface
 The network interface object. This function expects it to be filled with all
 information, except for the socket descriptor.
 @param txMcAddr
 The transmit multicast address

 @return
 - the socket descriptor (>= 0)
 - -1 if an error occurred
 */
static int createTxSocket(TRxTxNetworkInterface * networkInterface, union olsr_sockaddr * txMcAddr) {
	int ipFamilySetting;
	int ipProtoSetting;
	int ipMcLoopSetting;
	int ipMcIfSetting;
	int ipTtlSetting;
	unsigned int ifIndex;

	union olsr_sockaddr address;
	void * addr;
	size_t addrSize;

	int txSocket = -1;

	int mcLoopValue = 1;
	int txTtl = getTxTtl();

	assert(networkInterface != NULL);
	assert(strncmp((char *) &networkInterface->name[0], "",
					sizeof(networkInterface->name)) != 0);

	memset(&address, 0, sizeof(address));
	if (txMcAddr->in.sa_family == AF_INET) {
		struct ifreq ifr;
		struct in_addr * ifAddr = getIPv4Address(networkInterface->name, &ifr);
		if (!ifAddr) {
			pudError(true, "Could not get interface address of %s", networkInterface->name);
			goto bail;
		}

		assert(txMcAddr->in4.sin_addr.s_addr != INADDR_ANY);

		ipFamilySetting = AF_INET;
		ipProtoSetting = IPPROTO_IP;
		ipMcLoopSetting = IP_MULTICAST_LOOP;
		ipMcIfSetting = IP_MULTICAST_IF;
		ipTtlSetting = IP_MULTICAST_TTL;
		ifIndex = 0;

		address.in4.sin_family = ipFamilySetting;
		address.in4.sin_addr = *ifAddr;
		address.in4.sin_port = getTxMcPort();
		addr = &address.in4;
		addrSize = sizeof(struct sockaddr_in);
	} else {
		assert(txMcAddr->in6.sin6_addr.s6_addr != in6addr_any.s6_addr);

		ipFamilySetting = AF_INET6;
		ipProtoSetting = IPPROTO_IPV6;
		ipMcLoopSetting = IPV6_MULTICAST_LOOP;
		ipMcIfSetting = IPV6_MULTICAST_IF;
		ipTtlSetting = IPV6_MULTICAST_HOPS;
		ifIndex = if_nametoindex(networkInterface->name);

		addr = &ifIndex;
		addrSize = sizeof(ifIndex);
	}

	/*  Create a datagram socket on which to transmit */
	errno = 0;
	txSocket = socket(ipFamilySetting, SOCK_DGRAM, 0);
	if (txSocket < 0) {
		pudError(true, "Could not create a transmit socket for interface %s",
				networkInterface->name);
		goto bail;
	}

	/* Bind the socket to the desired interface */
	errno = 0;
	if (setsockopt(txSocket, ipProtoSetting, ipMcIfSetting, addr, addrSize) < 0) {
		pudError(true, "Could not set the multicast interface on the"
			" transmit socket to interface %s", networkInterface->name);
		goto bail;
	}

	/* Disable multicast local loopback */
	errno = 0;
	if (setsockopt(txSocket, ipProtoSetting, ipMcLoopSetting, &mcLoopValue,
			sizeof(mcLoopValue)) < 0) {
		pudError(true, "Could not disable multicast loopback on the"
			" transmit socket for interface %s", networkInterface->name);
		goto bail;
	}

	/* Set the TTL on the socket */
	errno = 0;
	if (setsockopt(txSocket, ipProtoSetting, ipTtlSetting, &txTtl,
			sizeof(txTtl)) < 0) {
		pudError(true, "Could not set TTL on the transmit socket"
			" for interface %s", networkInterface->name);
		goto bail;
	}

	/* Set the no delay option on the socket */
	errno = 0;
	if (fcntl(txSocket, F_SETFL, O_NDELAY) < 0) {
		pudError(true, "Could not set the no delay option on the"
			" transmit socket for interface %s", networkInterface->name);
		goto bail;
	}

	return txSocket;

	bail: if (txSocket >= 0) {
		close(txSocket);
	}
	return -1;
}

/**
 Create a transmit interface and add it to the list of transmit network
 interface objects

 @param ifName
 the network interface name
 @param txMcAddr
 the transmit multicast address

 @return
 - true on success
 - false on failure
 */
static bool createTxInterface(const char * ifName, union olsr_sockaddr * txMcAddr) {
	int socketFd = -1;
	TRxTxNetworkInterface * networkInterface = NULL;

	if (ifName == NULL) {
		goto bail;
	}

	networkInterface = olsr_malloc(sizeof(TRxTxNetworkInterface),
			"TRxTxNetworkInterface (PUD)");
	if (networkInterface == NULL) {
		goto bail;
	}

	memcpy(networkInterface->name, ifName, sizeof(networkInterface->name));
	networkInterface->name[IFNAMSIZ] = '\0';
	networkInterface->handler = NULL;
	networkInterface->next = NULL;

	/* networkInterface needs to be filled in when calling createTxSocket */
	socketFd = createTxSocket(networkInterface, txMcAddr);
	if (socketFd < 0) {
		goto bail;
	}
	networkInterface->socketFd = socketFd;

	/* Add new object to the end of the global list. */
	if (txNetworkInterfacesListHead == NULL) {
		txNetworkInterfacesListHead = networkInterface;
		lastTxNetworkInterface = networkInterface;
	} else {
		lastTxNetworkInterface->next = networkInterface;
		lastTxNetworkInterface = networkInterface;
	}

	return true;

	bail: if (networkInterface != NULL) {
		free(networkInterface);
	}
	return false;
}

/*
 * Downlink interface
 */

/** The socket fd, receiving downlinked messages */
static int downlinkSocketFd = -1;

/** the downlink handler function */
static socket_handler_func downlinkHandler = NULL;


/**
 @return
 The downlink socket fd. -1 when not valid.
 */
int getDownlinkSocketFd(void) {
	return downlinkSocketFd;
}

/**
 Create an downlink socket

 @param ipVersion
 The IP version (AF_INET or AF_INET6) for the socket
 @param rxSocketHandlerFunction
 The socket handler function

 @return
 - the socket descriptor (>= 0)
 - -1 if an error occurred
 */
static int createDownlinkSocket(int ipVersion, socket_handler_func rxSocketHandlerFunction) {
	union olsr_sockaddr address;
	void * addr;
	size_t addrSize;

	int downlinkSocket = -1;

	int socketReuseFlagValue = 1;

	memset(&address, 0, sizeof(address));
	if (ipVersion == AF_INET) {
		address.in4.sin_family = AF_INET;
		address.in4.sin_addr.s_addr = INADDR_ANY;
		address.in4.sin_port = getDownlinkPort();
		addr = &address.in4;
		addrSize = sizeof(struct sockaddr_in);
	} else {
		address.in6.sin6_family = AF_INET6;
		address.in6.sin6_addr = in6addr_any;
		address.in6.sin6_port = getDownlinkPort();
		addr = &address.in6;
		addrSize = sizeof(struct sockaddr_in6);
	}

	/*  Create a datagram socket on which to receive */
	errno = 0;
	downlinkSocket = socket(ipVersion, SOCK_DGRAM, 0);
	if (downlinkSocket < 0) {
		pudError(true, "Could not create the downlink socket");
		goto bail;
	}

	/* Enable SO_REUSEADDR to allow multiple applications to receive the same
	 * messages */
	errno = 0;
	if (setsockopt(downlinkSocket, SOL_SOCKET, SO_REUSEADDR, &socketReuseFlagValue,
			sizeof(socketReuseFlagValue)) < 0) {
		pudError(true, "Could not set REUSE option on the downlink socket");
		goto bail;
	}

	/* Bind to the proper port number with the IP address INADDR_ANY
	 * (INADDR_ANY is really required here, do not change it) */
	errno = 0;
	if (bind(downlinkSocket, addr, addrSize)) {
		pudError(true, "Could not bind downlink socket to port %d",
				getDownlinkPort());
		goto bail;
	}

	add_olsr_socket(downlinkSocket, rxSocketHandlerFunction, NULL, NULL,
			SP_PR_READ);

	downlinkHandler = rxSocketHandlerFunction;

	return downlinkSocket;

	bail: if (downlinkSocket >= 0) {
		close(downlinkSocket);
	}
	return -1;
}

/*
 * Interface Functions
 */

/**
 Creates receive and transmit sockets and register the receive sockets with
 the OLSR stack

 @param rxSocketHandlerFunctionDownlink
 The function to call upon reception of data on a downlink receive socket

 @return
 - true on success
 - false on failure
 */
bool createNetworkInterfaces(socket_handler_func rxSocketHandlerFunctionDownlink) {
	union olsr_sockaddr * txMcAddr = getTxMcAddr();
	unsigned int count = 0;

	/* loop over all configured tx interfaces */
	count = getTxNonOlsrInterfaceCount();
	while (count--) {
		if (!createTxInterface((char *)getTxNonOlsrInterfaceName(count), txMcAddr)) {
			/* creating a transmit interface failed */
			return false;
		}
	}

	/* create uplink socket when needed */
	if (isUplinkAddrSet()) {
		downlinkSocketFd = createDownlinkSocket(getUplinkAddr()->in.sa_family, rxSocketHandlerFunctionDownlink);
		if (downlinkSocketFd == -1) {
			return false;
		}
	} else {
		downlinkSocketFd = -1;
	}

	return true;
}

/**
 Close and cleanup the network interfaces in the given list

 @param networkInterface
 the list of network interface to close and clean up
 */
static void closeInterfaces(TRxTxNetworkInterface * networkInterface) {
	TRxTxNetworkInterface * nextNetworkInterface = networkInterface;
	while (nextNetworkInterface != NULL) {
		TRxTxNetworkInterface * iteratedNetworkInterface = nextNetworkInterface;
		if (iteratedNetworkInterface->socketFd >= 0) {
			if (iteratedNetworkInterface->handler) {
				remove_olsr_socket(iteratedNetworkInterface->socketFd,
						iteratedNetworkInterface->handler, NULL);
			}
			close(iteratedNetworkInterface->socketFd);
			iteratedNetworkInterface->socketFd = -1;
		}
		nextNetworkInterface = iteratedNetworkInterface->next;
		iteratedNetworkInterface->next = NULL;
		free(iteratedNetworkInterface);
	}
}

/**
 Close and cleanup all receive and transmit network interfaces
 */
void closeNetworkInterfaces(void) {
	if (txNetworkInterfacesListHead != NULL) {
		closeInterfaces(txNetworkInterfacesListHead);
		txNetworkInterfacesListHead = NULL;
	}

	if (downlinkSocketFd != -1 ) {
		if (downlinkHandler) {
			remove_olsr_socket (downlinkSocketFd, downlinkHandler, NULL);
			downlinkHandler = NULL;
		}
		close(downlinkSocketFd);
		downlinkSocketFd = -1;
	}
}
