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

		struct interface *mainInterface = if_ifwithaddr(&olsr_cnf->main_addr);
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
 * RX interfaces
 */

/** The list of network interface objects, receiving GPS NMEA sentences */
static TRxTxNetworkInterface *rxNetworkInterfacesListHead = NULL;

/** Pointer to the last network interface object, receiving GPS NMEA sentences */
static TRxTxNetworkInterface *lastRxNetworkInterface = NULL;

/**
 @return
 The list of network interface objects, receiving GPS NMEA sentences
 */
TRxTxNetworkInterface *getRxNetworkInterfaces(void) {
	return rxNetworkInterfacesListHead;
}

/**
 Create a receive socket for a network interface

 @param networkInterface
 The network interface object. This function expects it to be filled with all
 information, except for the socket descriptor.
 @param rxSocketHandlerFunction
 The function that handles reception of data on the network interface
 @param rxMcAddr
 The receive multicast address

 @return
 - the socket descriptor (>= 0)
 - -1 if an error occurred
 */
static int createRxSocket(TRxTxNetworkInterface * networkInterface,
		socket_handler_func rxSocketHandlerFunction, union olsr_sockaddr * rxMcAddr) {
	int ipFamilySetting;
	int ipProtoSetting;
	int ipMcLoopSetting;
	int ipAddMembershipSetting;

	union olsr_sockaddr address;
	void * addr;
	size_t addrSize;

	int rxSocket = -1;

	int socketReuseFlagValue = 1;
	int mcLoopValue = 1;

	assert(networkInterface != NULL);
	assert(rxSocketHandlerFunction != NULL);
	assert(strncmp((char *) &networkInterface->name[0], "",
					sizeof(networkInterface->name)) != 0);

	memset(&address, 0, sizeof(address));
	if (rxMcAddr->in.sa_family == AF_INET) {
		assert(rxMcAddr->in4.sin_addr.s_addr != INADDR_ANY);

		ipFamilySetting = AF_INET;
		ipProtoSetting = IPPROTO_IP;
		ipMcLoopSetting = IP_MULTICAST_LOOP;
		ipAddMembershipSetting = IP_ADD_MEMBERSHIP;

		address.in4.sin_family = ipFamilySetting;
		address.in4.sin_addr.s_addr = INADDR_ANY;
		address.in4.sin_port = getRxMcPort();
		addr = &address.in4;
		addrSize = sizeof(struct sockaddr_in);
	} else {
		assert(rxMcAddr->in6.sin6_addr.s6_addr != in6addr_any.s6_addr);

		ipFamilySetting = AF_INET6;
		ipProtoSetting = IPPROTO_IPV6;
		ipMcLoopSetting = IPV6_MULTICAST_LOOP;
		ipAddMembershipSetting = IPV6_ADD_MEMBERSHIP;

		address.in6.sin6_family = ipFamilySetting;
		address.in6.sin6_addr = in6addr_any;
		address.in6.sin6_port = getRxMcPort();
		addr = &address.in6;
		addrSize = sizeof(struct sockaddr_in6);
	}

	/* Create a datagram socket on which to receive. */
	errno = 0;
	rxSocket = socket(ipFamilySetting, SOCK_DGRAM, 0);
	if (rxSocket < 0) {
		pudError(true, "Could not create a receive socket for interface %s",
				networkInterface->name);
		goto bail;
	}

	/* Enable SO_REUSEADDR to allow multiple applications to receive the same
	 * multicast messages */
	errno = 0;
	if (setsockopt(rxSocket, SOL_SOCKET, SO_REUSEADDR, &socketReuseFlagValue,
			sizeof(socketReuseFlagValue)) < 0) {
		pudError(true, "Could not set the reuse flag on the receive socket for"
			" interface %s", networkInterface->name);
		goto bail;
	}

	/* Bind to the proper port number with the IP address INADDR_ANY
	 * (INADDR_ANY is really required here, do not change it) */
	errno = 0;
	if (bind(rxSocket, addr, addrSize) < 0) {
		pudError(true, "Could not bind the receive socket for interface"
			" %s to port %u", networkInterface->name, ntohs(getRxMcPort()));
		goto bail;
	}

	/* Enable multicast local loopback */
	errno = 0;
	if (setsockopt(rxSocket, ipProtoSetting, ipMcLoopSetting, &mcLoopValue,
			sizeof(mcLoopValue)) < 0) {
		pudError(true, "Could not %s multicast loopback on the"
			" receive socket for interface %s", mcLoopValue ? "enable"
				: "disable", networkInterface->name);
		goto bail;
	}

	/* Join the multicast group on the local interface. Note that this
	 * ADD_MEMBERSHIP option must be called for each local interface over
	 * which the multicast datagrams are to be received. */
	if (ipFamilySetting == AF_INET) {
		struct ip_mreq mc_settings;

		struct ifreq ifr;
		struct in_addr * ifAddr = getIPv4Address(networkInterface->name, &ifr);
		if (!ifAddr) {
			pudError(true, "Could not get interface address of %s", networkInterface->name);
			goto bail;
		}

		(void) memset(&mc_settings, 0, sizeof(mc_settings));
		mc_settings.imr_multiaddr = rxMcAddr->in4.sin_addr;
		mc_settings.imr_interface = *ifAddr;
		errno = 0;
		if (setsockopt(rxSocket, ipProtoSetting, ipAddMembershipSetting,
				&mc_settings, sizeof(mc_settings)) < 0) {
			pudError(true, "Could not subscribe interface %s to the configured"
				" multicast group", networkInterface->name);
			goto bail;
		}
	} else {
		struct ipv6_mreq mc6_settings;
		(void) memset(&mc6_settings, 0, sizeof(mc6_settings));
		mc6_settings.ipv6mr_multiaddr = rxMcAddr->in6.sin6_addr;
		mc6_settings.ipv6mr_interface = if_nametoindex(networkInterface->name);
		errno = 0;
		if (setsockopt(rxSocket, ipProtoSetting, ipAddMembershipSetting,
				&mc6_settings, sizeof(mc6_settings)) < 0) {
			pudError(true, "Could not subscribe interface %s to the configured"
				" multicast group", networkInterface->name);
			goto bail;
		}
	}

	add_olsr_socket(rxSocket, rxSocketHandlerFunction, NULL, networkInterface,
			SP_PR_READ);

	return rxSocket;

	bail: if (rxSocket >= 0) {
		close(rxSocket);
	}
	return -1;

}

/**
 Create a receive interface and add it to the list of receive network interface
 objects

 @param ifName
 the network interface name
 @param rxSocketHandlerFunction
 the function that handles reception of data on the network interface
 @param rxMcAddr
 The receive multicast address

 @return
 - true on success
 - false on failure
 */
static bool createRxInterface(const char * ifName,
		socket_handler_func rxSocketHandlerFunction, union olsr_sockaddr * rxMcAddr) {
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

	/* networkInterface needs to be filled in when calling createRxSocket */
	socketFd = createRxSocket(networkInterface, rxSocketHandlerFunction, rxMcAddr);
	if (socketFd < 0) {
		goto bail;
	}
	networkInterface->socketFd = socketFd;
	networkInterface->handler = rxSocketHandlerFunction;

	/* Add new object to the end of the global list. */
	if (rxNetworkInterfacesListHead == NULL) {
		rxNetworkInterfacesListHead = networkInterface;
		lastRxNetworkInterface = networkInterface;
	} else {
		lastRxNetworkInterface->next = networkInterface;
		lastRxNetworkInterface = networkInterface;
	}

	return true;

	bail: if (networkInterface != NULL) {
		free(networkInterface);
	}
	return false;

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

	int mcLoopValue = 0;
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
		pudError(true, "Could not %s multicast loopback on the"
			" transmit socket for interface %s", mcLoopValue ? "enable"
				: "disable", networkInterface->name);
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

 @param rxSocketHandlerFunction
 The function to call upon reception of data on a receive socket
 @param rxSocketHandlerFunctionDownlink
 The function to call upon reception of data on a downlink receive socket

 @return
 - true on success
 - false on failure
 */
bool createNetworkInterfaces(socket_handler_func rxSocketHandlerFunction,
		socket_handler_func rxSocketHandlerFunctionDownlink) {
	union olsr_sockaddr * rxMcAddr = getRxMcAddr();
	union olsr_sockaddr * txMcAddr = getTxMcAddr();
	unsigned int count = 0;

	/* loop over all configured rx interfaces */
	count = getRxNonOlsrInterfaceCount();
	while (count--) {
		if (!createRxInterface((char *)getRxNonOlsrInterfaceName(count), rxSocketHandlerFunction, rxMcAddr)) {
			/* creating a receive interface failed */
			return false;
		}
	}

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
	if (rxNetworkInterfacesListHead != NULL) {
		closeInterfaces(rxNetworkInterfacesListHead);
		rxNetworkInterfacesListHead = NULL;
	}

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
