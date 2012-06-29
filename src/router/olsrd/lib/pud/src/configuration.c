#include "configuration.h"

/* Plugin includes */
#include "pud.h"
#include "networkInterfaces.h"
#include "netTools.h"
#include "posFile.h"

/* OLSR includes */
#include <olsr_protocol.h>

/* System includes */
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <nmea/util.h>
#include <OlsrdPudWireFormat/nodeIdConversion.h>
#include <limits.h>

/*
 * Utility functions
 */

/**
 Determine the address of the port in an OLSR socket address

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param addr
 A pointer to OLSR socket address
 @param port
 A pointer to the location where the pointer to the port will be stored
 */
static void getOlsrSockaddrPortAddress(int ipVersion,
		union olsr_sockaddr * addr, in_port_t ** port) {
	if (ipVersion == AF_INET) {
		*port = &addr->in4.sin_port;
	} else {
		*port = &addr->in6.sin6_port;
	}
}

/**
 Get pointers to the IP address and port in an OLSR socket address
 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param addr
 A pointer to OLSR socket address
 @param ipAddress
 A pointer to the location where the pointer to the IP address will be stored
 @param port
 A pointer to the location where the pointer to the port will be stored
 */
static void getOlsrSockAddrAndPortAddresses(int ipVersion,
		union olsr_sockaddr * addr, void ** ipAddress, in_port_t ** port) {
	if (ipVersion == AF_INET) {
		*ipAddress = (void *) &addr->in4.sin_addr;
		*port = (void *) &addr->in4.sin_port;
	} else {
		*ipAddress = (void *) &addr->in6.sin6_addr;
		*port = (void *) &addr->in6.sin6_port;
	}
}

/**
 Read an unsigned long long number from a value string

 @param valueName
 the name of the value
 @param value
 the string to convert to a number
 @param valueNumber
 a pointer to the location where to store the number upon successful conversion

 @return
 - true on success
 - false otherwise
 */
static bool readULL(const char * valueName, const char * value,
		unsigned long long * valueNumber) {
	char * endPtr = NULL;
	unsigned long long valueNew;

	errno = 0;
	valueNew = strtoull(value, &endPtr, 10);

	if (!((endPtr != value) && (*value != '\0') && (*endPtr == '\0'))) {
		/* invalid conversion */
		pudError(true, "Configured %s (%s) could not be converted to a number",
				valueName, value);
		return false;
	}

	*valueNumber = valueNew;

	return true;
}

/**
 Read a double number from a value string

 @param valueName
 the name of the value
 @param value
 the string to convert to a number
 @param valueNumber
 a pointer to the location where to store the number upon successful conversion

 @return
 - true on success
 - false otherwise
 */
bool readDouble(const char * valueName, const char * value, double * valueNumber) {
	char * endPtr = NULL;
	double valueNew;

	errno = 0;
	valueNew = strtod(value, &endPtr);

	if (!((endPtr != value) && (*value != '\0') && (*endPtr == '\0'))) {
		/* invalid conversion */
		pudError(true, "Configured %s (%s) could not be converted to a number",
				valueName, value);
		return false;
	}

	*valueNumber = valueNew;

	return true;
}

/*
 * nodeIdType
 */

/** The nodeIdType */
static NodeIdType nodeIdType = PUD_NODE_ID_TYPE_DEFAULT;

/**
 @return
 The node ID type
 */
NodeIdType getNodeIdTypeNumber(void) {
	return nodeIdType;
}

/**
 Set the node ID type.

 @param value
 The value of the node ID type to set (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setNodeIdType(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_NODE_ID_TYPE_NAME;
	unsigned long long nodeIdTypeNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &nodeIdTypeNew)) {
		return true;
	}

	if (!isValidNodeIdType(nodeIdTypeNew)) {
		pudError(false, "Configured %s (%llu) is reserved", valueName,
				nodeIdTypeNew);
		return true;
	}

	nodeIdType = nodeIdTypeNew;

	return false;
}

/*
 * nodeId
 */

/** The nodeId buffer */
static unsigned char nodeId[PUD_TX_NODEID_BUFFERSIZE + 1];

/** The length of the string in the nodeId buffer */
static size_t nodeIdLength = 0;

/** True when the nodeId is set */
static bool nodeIdSet = false;

/** The nodeId as a binary representation, with status */
static nodeIdBinaryType nodeIdBinary;

/**
 @return
 The node ID
 */
unsigned char * getNodeId(void) {
	return getNodeIdWithLength(NULL);
}

/**
 Get the nodeId and its length

 @param length
 a pointer to the variable in which to store the nodeId length (allowed to be
 NULL, in which case the length is not stored)

 @return
 The node ID
 */
unsigned char * getNodeIdWithLength(size_t *length) {
	if (!nodeIdSet) {
		setNodeId("", NULL, (set_plugin_parameter_addon) {.pc = NULL});
	}

	if (length != NULL) {
		*length = nodeIdLength;
	}

	return &nodeId[0];
}

/**
 Get the nodeIdBinary

 @return
 The node ID in binary representation
 */
nodeIdBinaryType * getNodeIdBinary(void) {
	if (!nodeIdBinary.set) {
		setNodeId("", NULL, (set_plugin_parameter_addon) {.pc = NULL});
	}

	return &nodeIdBinary;
}

/**
 Set the node ID.

 @param value
 The value of the node ID to set (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setNodeId(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_NODE_ID_NAME;
	size_t valueLength;

	assert (value != NULL);

	valueLength = strlen(value);
	if (valueLength > PUD_TX_NODEID_BUFFERSIZE) {
		pudError(false, "Configured %s is too long, maximum length is"
			" %u, current length is %lu", valueName, PUD_TX_NODEID_BUFFERSIZE,
				(unsigned long) valueLength);
		return true;
	}

	strcpy((char *) &nodeId[0], value);
	nodeIdLength = valueLength;
	nodeIdSet = true;

	return false;
}

/*
 * nodeId Validation
 */

/**
 Validate whether the configured nodeId is valid w.r.t. the configured
 nodeIdType, for types that are MAC addresses

 @return
 - true when ok
 - false on failure
 */
static bool intSetupNodeIdBinaryMAC(void) {
	unsigned char * mac = getMainIpMacAddress();
	if (!mac) {
		return false;
	}

	return setupNodeIdBinaryMAC(&nodeIdBinary, mac);
}

/**
 Validate whether the configured nodeId is valid w.r.t. the configured
 nodeIdType, for types that fit in an unsigned long long (64 bits)

 @param min
 the minimum value
 @param max
 the maximum value
 @param bytes
 the number of bytes in the buffer

 @return
 - true when ok
 - false on failure
 */
static bool intSetupNodeIdBinaryLongLong(unsigned long long min,
		unsigned long long max, unsigned int bytes) {
	unsigned long long longValue = 0;
	if (!readULL(PUD_NODE_ID_NAME, (char *) getNodeId(), &longValue)) {
		return false;
	}

	if ((longValue < min) || (longValue > max)) {
		pudError(false, "%s value %llu is out of range [%llu,%llu]",
				PUD_NODE_ID_NAME, longValue, min, max);
		return false;
	}

	return setupNodeIdBinaryLongLong(&nodeIdBinary, longValue, bytes);
}

/**
 Validate whether the configured nodeId is valid w.r.t. the configured
 nodeIdType, for types that are strings

 @return
 - true when ok
 - false on failure
 */
static bool intSetupNodeIdBinaryString(void) {
	bool invalidChars;
	char report[256];
	size_t nodeidlength;
	char * nodeid = (char *)getNodeIdWithLength(&nodeidlength);

	invalidChars = nmea_string_has_invalid_chars(nodeid,
			PUD_NODE_ID_NAME, &report[0], sizeof(report));
	if (invalidChars) {
		pudError(false, "%s", &report[0]);
		return false;
	}

	if (nodeidlength > PUD_TX_NODEID_BUFFERSIZE) {
		pudError(false, "%s value \"%s\" is too long", PUD_NODE_ID_NAME, &nodeid[0]);
		return false;
	}

	return setupNodeIdBinaryString(&nodeIdBinary, nodeid, nodeidlength);
}

/**
 Validate whether the configured nodeId is valid w.r.t. the configured
 nodeIdType, for types that are IP addresses

 @return
 - true when ok
 - false on failure
 */
static bool intSetupNodeIdBinaryIp(void) {
	void * src;
	size_t length;
	if (olsr_cnf->ip_version == AF_INET) {
		src = &olsr_cnf->main_addr.v4;
		length = sizeof(struct in_addr);
	} else {
		src = &olsr_cnf->main_addr.v6;
		length = sizeof(struct in6_addr);
	}

	return setupNodeIdBinaryIp(&nodeIdBinary, src, length);
}

/**
 Validate whether the configured nodeId is valid w.r.t. the configured
 nodeIdType

 @return
 - true when ok
 - false on failure
 */
static bool setupNodeIdBinaryAndValidate(NodeIdType nodeIdTypeNumber) {
	switch (nodeIdTypeNumber) {
		case PUD_NODEIDTYPE_MAC: /* hardware address */
			return intSetupNodeIdBinaryMAC();

		case PUD_NODEIDTYPE_MSISDN: /* an MSISDN number */
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_MSISDN_MIN,
				PUD_NODEIDTYPE_MSISDN_MAX, PUD_NODEIDTYPE_MSISDN_BYTES);

		case PUD_NODEIDTYPE_TETRA: /* a Tetra number */
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_TETRA_MIN,
				PUD_NODEIDTYPE_TETRA_MAX, PUD_NODEIDTYPE_TETRA_BYTES);

		case PUD_NODEIDTYPE_DNS: /* DNS name */
			return intSetupNodeIdBinaryString();

		case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_MMSI_MIN,
				PUD_NODEIDTYPE_MMSI_MAX, PUD_NODEIDTYPE_MMSI_BYTES);

		case PUD_NODEIDTYPE_URN: /* a URN number */
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_URN_MIN,
				PUD_NODEIDTYPE_URN_MAX, PUD_NODEIDTYPE_URN_BYTES);

		case PUD_NODEIDTYPE_192:
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_192_MIN,
				PUD_NODEIDTYPE_192_MAX, PUD_NODEIDTYPE_192_BYTES);

		case PUD_NODEIDTYPE_193:
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_193_MIN,
				PUD_NODEIDTYPE_193_MAX, PUD_NODEIDTYPE_193_BYTES);

		case PUD_NODEIDTYPE_194:
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_194_MIN,
				PUD_NODEIDTYPE_194_MAX, PUD_NODEIDTYPE_194_BYTES);

		case PUD_NODEIDTYPE_IPV4: /* IPv4 address */
		case PUD_NODEIDTYPE_IPV6: /* IPv6 address */
		default: /* unsupported */
			return intSetupNodeIdBinaryIp();
	}

	return false;
}

/*
 * rxNonOlsrIf
 */

/** The maximum number of RX non-OLSR interfaces */
#define PUD_RX_NON_OLSR_IF_MAX 32

/** Array with RX non-OLSR interface names */
static unsigned char rxNonOlsrInterfaceNames[PUD_RX_NON_OLSR_IF_MAX][IFNAMSIZ + 1];

/** The number of RX non-OLSR interface names in the array */
static unsigned int rxNonOlsrInterfaceCount = 0;

/**
 Determine whether a give interface name is configured as a receive non-OLSR
 interface.

 @param ifName
 The interface name to check

 @return
 - true when the given interface name is configured as a receive non-OLSR
 interface
 - false otherwise
 */
bool isRxNonOlsrInterface(const char *ifName) {
	unsigned int i;

	assert (ifName != NULL);

	for (i = 0; i < rxNonOlsrInterfaceCount; i++) {
		if (strncmp((char *) &rxNonOlsrInterfaceNames[i][0], ifName, IFNAMSIZ
				+ 1) == 0) {
			return true;
		}
	}

	return false;
}

/**
 Add a receive non-OLSR interface

 @param value
 The name of the non-OLSR interface to add
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int addRxNonOlsrInterface(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	unsigned long valueLength;

	assert (value != NULL);

	valueLength = strlen(value);
	if (valueLength > IFNAMSIZ) {
		pudError(false, "Configured %s (%s) is too long,"
			" maximum length is %u, current length is %lu",
				PUD_RX_NON_OLSR_IF_NAME, value, IFNAMSIZ, valueLength);
		return true;
	}

	if (!isRxNonOlsrInterface(value)) {
		if (rxNonOlsrInterfaceCount >= PUD_RX_NON_OLSR_IF_MAX) {
			pudError(false, "Can't configure more than %u receive interfaces",
					PUD_RX_NON_OLSR_IF_MAX);
			return true;
		}

		strcpy((char *) &rxNonOlsrInterfaceNames[rxNonOlsrInterfaceCount][0],
				value);
		rxNonOlsrInterfaceCount++;
	}

	return false;
}

/*
 * rxAllowedSourceIpAddress
 */

/** The maximum number of RX allowed source IP addresses */
#define PUD_RX_ALLOWED_SOURCE_IP_MAX 32

/** Array with RX allowed source IP addresses */
static struct sockaddr rxAllowedSourceIpAddresses[PUD_RX_ALLOWED_SOURCE_IP_MAX];

/** The number of RX allowed source IP addresses in the array */
static unsigned int rxAllowedSourceIpAddressesCount = 0;

/**
 Determine whether a give IP address is configured as an allowed source IP
 address.

 @param sender
 The IP address to check

 @return
 - true when the given IP address is configured as an allowed source IP
 address
 - false otherwise
 */
bool isRxAllowedSourceIpAddress(struct sockaddr * sender) {
	void * addr;
	unsigned int addrSize;
	unsigned int i;

	if (rxAllowedSourceIpAddressesCount == 0) {
		return true;
	}

	if (sender == NULL) {
		return false;
	}

	if (sender->sa_family == AF_INET) {
		addr = (void *) (&((struct sockaddr_in *) sender)->sin_addr);
		addrSize = sizeof(struct in_addr);
	} else {
		addr = (void *) (&((struct sockaddr_in6 *) sender)->sin6_addr);
		addrSize = sizeof(struct in6_addr);
	}

	for (i = 0; i < rxAllowedSourceIpAddressesCount; i++) {
		if ((rxAllowedSourceIpAddresses[i].sa_family == sender->sa_family)
				&& (memcmp(&rxAllowedSourceIpAddresses[i].sa_data, addr,
						addrSize) == 0)) {
			return true;
		}
	}

	return false;
}

/**
 Set the RX allowed source IP addresses.

 @param value
 The RX allowed source IP address (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int addRxAllowedSourceIpAddress(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_RX_ALLOWED_SOURCE_IP_NAME;
	const char * valueInternal = value;
	int conversion;
	struct sockaddr addr;

	assert (value != NULL);

	memset(&addr, 0, sizeof(addr));

	addr.sa_family = olsr_cnf->ip_version;
	conversion = inet_pton(olsr_cnf->ip_version, valueInternal, &addr.sa_data);
	if (conversion != 1) {
		pudError((conversion == -1) ? true : false,
				"Configured %s (%s) is not an IP address", valueName,
				valueInternal);
		return true;
	}

	if ((rxAllowedSourceIpAddressesCount == 0) || !isRxAllowedSourceIpAddress(&addr)) {
		if (rxAllowedSourceIpAddressesCount >= PUD_RX_ALLOWED_SOURCE_IP_MAX) {
			pudError(false, "Can't configure more than %u allowed source IP"
				" addresses", PUD_RX_ALLOWED_SOURCE_IP_MAX);
			return true;
		}

		rxAllowedSourceIpAddresses[rxAllowedSourceIpAddressesCount] = addr;
		rxAllowedSourceIpAddressesCount++;
	}

	return false;
}

/*
 * rxMcAddr
 */

/** The rx multicast address */
static union olsr_sockaddr rxMcAddr;

/** True when the rx multicast address is set */
static bool rxMcAddrSet = false;

/**
 @return
 The receive multicast address (in network byte order). Sets both the address
 and the port to their default values when the address was not yet set.
 */
union olsr_sockaddr * getRxMcAddr(void) {
	if (!rxMcAddrSet) {
		setRxMcAddr(NULL, NULL, ((set_plugin_parameter_addon) {.pc = NULL}));
	}
	return &rxMcAddr;
}

/**
 Set the receive multicast address. Sets the address to its default value when
 the value is NULL. Also sets the port to its default value when the address
 was not yet set.

 @param value
 The receive multicast address (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setRxMcAddr(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_RX_MC_ADDR_NAME;
	void * ipAddress;
	in_port_t * port;
	const char * valueInternal = value;
	int conversion;

	getOlsrSockAddrAndPortAddresses(olsr_cnf->ip_version, &rxMcAddr, &ipAddress,
			&port);
	if (olsr_cnf->ip_version == AF_INET) {
		rxMcAddr.in4.sin_family = olsr_cnf->ip_version;
		if (valueInternal == NULL) {
			valueInternal = PUD_RX_MC_ADDR_4_DEFAULT;
		}
	} else {
		rxMcAddr.in6.sin6_family = olsr_cnf->ip_version;
		if (valueInternal == NULL) {
			valueInternal = PUD_RX_MC_ADDR_6_DEFAULT;
		}
	}

	if (!rxMcAddrSet) {
		*port = htons(PUD_RX_MC_PORT_DEFAULT);
	}

	conversion = inet_pton(olsr_cnf->ip_version, valueInternal, ipAddress);
	if (conversion != 1) {
		pudError((conversion == -1) ? true : false,
				"Configured %s (%s) is not an IP address", valueName,
				valueInternal);
		return true;
	}

	if (!isMulticast(olsr_cnf->ip_version, &rxMcAddr)) {
		pudError(false, "Configured %s (%s) is not a multicast address",
				valueName, valueInternal);
		return true;
	}

	rxMcAddrSet = true;
	return false;
}

/*
 * rxMcPort
 */

/**
 @return
 The receive multicast port (in network byte order)
 */
unsigned short getRxMcPort(void) {
	in_port_t * port;
	getOlsrSockaddrPortAddress(olsr_cnf->ip_version, getRxMcAddr(), &port);
	return *port;
}

/**
 Set the receive multicast port

 @param value
 The receive multicast port (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setRxMcPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_RX_MC_PORT_NAME;
	unsigned long long rxMcPortNew;
	in_port_t * port;
	union olsr_sockaddr * addr = getRxMcAddr();

	assert (value != NULL);

	if (!readULL(valueName, value, &rxMcPortNew)) {
		return true;
	}

	if ((rxMcPortNew < 1) || (rxMcPortNew > 65535)) {
		pudError(false, "Configured %s (%llu) is outside of"
			" valid range 1-65535", valueName, rxMcPortNew);
		return true;
	}

	getOlsrSockaddrPortAddress(olsr_cnf->ip_version, addr, &port);
	*port = htons((uint16_t) rxMcPortNew);

	return false;
}

/*
 * positionFile
 */

/** The positionFile buffer */
static char positionFile[PATH_MAX + 1];

/** True when the positionFile is set */
static bool positionFileSet = false;

/**
 @return
 The positionFile (NULL when not set)
 */
char * getPositionFile(void) {
	if (!positionFileSet) {
		return NULL;
	}

	return &positionFile[0];
}

/**
 Set the positionFile.

 @param value
 The value of the positionFile to set (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setPositionFile(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_POSFILE_NAME;
	size_t valueLength;

	assert(value != NULL);

	if (!startPositionFile()) {
		stopPositionFile();
		return true;
	}

	valueLength = strlen(value);
	if (valueLength > PATH_MAX) {
		pudError(false, "Configured %s is too long, maximum length is"
				" %u, current length is %lu", valueName, PATH_MAX, (unsigned long) valueLength);
		return true;
	}

	strcpy((char *) &positionFile[0], value);
	positionFileSet = true;

	return false;
}

/*
 * txNonOlsrIf
 */

/** The maximum number of rx non-olsr interfaces */
#define PUD_TX_NON_OLSR_IF_MAX 32

/** Array with tx non-olsr interface names */
static unsigned char txNonOlsrInterfaceNames[PUD_TX_NON_OLSR_IF_MAX][IFNAMSIZ + 1];

/** The number of tx interface names in the array */
static unsigned int txNonOlsrInterfaceCount = 0;

/**
 Determine whether a give interface name is configured as a transmit non-OLSR
 interface.

 @param ifName
 The interface to check

 @return
 - true when the given interface name is configured as a transmit non-OLSR
 interface
 - false otherwise
 */
bool isTxNonOlsrInterface(const char *ifName) {
	unsigned int i;

	assert (ifName != NULL);

	for (i = 0; i < txNonOlsrInterfaceCount; i++) {
		if (strncmp((char *) &txNonOlsrInterfaceNames[i][0], ifName, IFNAMSIZ
				+ 1) == 0) {
			return true;
		}
	}

	return false;
}

/**
 Add a transmit non-OLSR interface

 @param value
 The name of the non-OLSR interface to add
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int addTxNonOlsrInterface(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	unsigned long valueLength;

	assert (value != NULL);

	valueLength = strlen(value);
	if (valueLength > IFNAMSIZ) {
		pudError(false, "Configured %s (%s) is too long,"
			" maximum length is %u, current length is %lu",
				PUD_TX_NON_OLSR_IF_NAME, value, IFNAMSIZ, valueLength);
		return true;
	}

	if (!isTxNonOlsrInterface(value)) {
		if (txNonOlsrInterfaceCount >= PUD_TX_NON_OLSR_IF_MAX) {
			pudError(false, "Can not configure more than %u transmit"
				" interfaces", PUD_TX_NON_OLSR_IF_MAX);
			return true;
		}

		strcpy((char *) &txNonOlsrInterfaceNames[txNonOlsrInterfaceCount][0],
				value);
		txNonOlsrInterfaceCount++;
	}

	return false;
}

/*
 * txMcAddr
 */

/** The tx multicast address */
static union olsr_sockaddr txMcAddr;

/** True when the tx multicast address is set */
static bool txMcAddrSet = false;

/**
 @return
 The transmit multicast address (in network byte order). Sets both the address
 and the port to their default values when the address was not yet set.
 */
union olsr_sockaddr * getTxMcAddr(void) {
	if (!txMcAddrSet) {
		setTxMcAddr(NULL, NULL, ((set_plugin_parameter_addon) {.pc = NULL}));
	}
	return &txMcAddr;
}

/**
 Set the transmit multicast address. Sets the address to its default value when
 the value is NULL. Also sets the port to its default value when the address
 was not yet set.

 @param value
 The transmit multicast address (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setTxMcAddr(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_MC_ADDR_NAME;
	void * ipAddress;
	in_port_t * port;
	const char * valueInternal = value;
	int conversion;

	getOlsrSockAddrAndPortAddresses(olsr_cnf->ip_version, &txMcAddr, &ipAddress,
			&port);
	if (olsr_cnf->ip_version == AF_INET) {
		txMcAddr.in4.sin_family = olsr_cnf->ip_version;
		if (valueInternal == NULL) {
			valueInternal = PUD_TX_MC_ADDR_4_DEFAULT;
		}
	} else {
		txMcAddr.in6.sin6_family = olsr_cnf->ip_version;
		if (valueInternal == NULL) {
			valueInternal = PUD_TX_MC_ADDR_6_DEFAULT;
		}
	}

	if (!txMcAddrSet) {
		*port = htons(PUD_TX_MC_PORT_DEFAULT);
	}

	conversion = inet_pton(olsr_cnf->ip_version, valueInternal, ipAddress);
	if (conversion != 1) {
		pudError((conversion == -1) ? true : false,
				"Configured %s (%s) is not an IP address", valueName,
				valueInternal);
		return true;
	}

	if (!isMulticast(olsr_cnf->ip_version, &txMcAddr)) {
		pudError(false, "Configured %s (%s) is not a multicast address",
				valueName, valueInternal);
		return true;
	}

	txMcAddrSet = true;
	return false;
}

/*
 * txMcPort
 */

/**
 @return
 The transmit multicast port (in network byte order)
 */
unsigned short getTxMcPort(void) {
	in_port_t * port;
	getOlsrSockaddrPortAddress(olsr_cnf->ip_version, getTxMcAddr(), &port);
	return *port;
}

/**
 Set the transmit multicast port

 @param value
 The transmit multicast port (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setTxMcPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_MC_PORT_NAME;
	unsigned long long txMcPortNew;
	in_port_t * port;
	union olsr_sockaddr * addr = getTxMcAddr();

	assert (value != NULL);

	if (!readULL(valueName, value, &txMcPortNew)) {
		return true;
	}

	if ((txMcPortNew < 1) || (txMcPortNew > 65535)) {
		pudError(false, "Configured %s (%llu) is outside of"
			" valid range 1-65535", valueName, txMcPortNew);
		return true;
	}

	getOlsrSockaddrPortAddress(olsr_cnf->ip_version, addr, &port);
	*port = htons((uint16_t) txMcPortNew);

	return false;
}

/*
 * uplinkAddr
 */

/** The uplink address */
static union olsr_sockaddr uplinkAddr;

/** True when the uplink address is set */
static bool uplinkAddrSet = false;

/** True when the uplink address is set */
static bool uplinkPortSet = false;

/**
 @return
 - true when the uplink address is set
 - false otherwise
 */
bool isUplinkAddrSet(void) {
	return uplinkAddrSet;
}

/**
 @return
 The uplink address (in network byte order). Sets both the address
 and the port to their default values when the address was not yet set.
 */
union olsr_sockaddr * getUplinkAddr(void) {
	if (!uplinkAddrSet) {
		setUplinkAddr(NULL, NULL, ((set_plugin_parameter_addon) {.pc = NULL}));
	}
	return &uplinkAddr;
}

/**
 Set the uplink address. Sets the address to its default value when
 the value is NULL. Also sets the port to its default value when the address
 was not yet set.

 @param value
 The uplink address (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUplinkAddr(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_ADDR_NAME;
	void * ipAddress;
	in_port_t * port;
	const char * valueInternal = value;
	int conversion;
	bool defaultValue = false;

	getOlsrSockAddrAndPortAddresses(olsr_cnf->ip_version, &uplinkAddr,
			&ipAddress, &port);
	if (olsr_cnf->ip_version == AF_INET) {
		uplinkAddr.in4.sin_family = olsr_cnf->ip_version;
		if (valueInternal == NULL) {
			valueInternal = PUD_UPLINK_ADDR_4_DEFAULT;
			defaultValue = true;
		}
	} else {
		uplinkAddr.in6.sin6_family = olsr_cnf->ip_version;
		if (valueInternal == NULL) {
			valueInternal = PUD_UPLINK_ADDR_6_DEFAULT;
			defaultValue = true;
		}
	}

	if (!uplinkPortSet) {
		*port = htons(PUD_UPLINK_PORT_DEFAULT);
		uplinkPortSet = true;
	}

	conversion = inet_pton(olsr_cnf->ip_version, valueInternal, ipAddress);
	if (conversion != 1) {
		pudError((conversion == -1) ? true : false,
				"Configured %s (%s) is not an IP address", valueName,
				valueInternal);
		return true;
	}

	if (!defaultValue) {
		uplinkAddrSet = true;
	}

	return false;
}

/*
 * uplinkPort
 */

/**
 @return
 The uplink port (in network byte order)
 */
unsigned short getUplinkPort(void) {
	in_port_t * port;
	getOlsrSockaddrPortAddress(olsr_cnf->ip_version, getUplinkAddr(), &port);
	return *port;
}

/**
 Set the uplink port

 @param value
 The uplink port (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUplinkPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_PORT_NAME;
	unsigned long long uplinkPortNew;
	in_port_t * port;
	union olsr_sockaddr * addr = getUplinkAddr();

	assert (value != NULL);

	if (!readULL(valueName, value, &uplinkPortNew)) {
		return true;
	}

	if ((uplinkPortNew < 1) || (uplinkPortNew > 65535)) {
		pudError(false, "Configured %s (%llu) is outside of"
			" valid range 1-65535", valueName, uplinkPortNew);
		return true;
	}

	getOlsrSockaddrPortAddress(olsr_cnf->ip_version, addr, &port);
	*port = htons((uint16_t) uplinkPortNew);
	uplinkPortSet = true;

	return false;
}


/*
 * downlinkPort
 */

/** the downlink port */
unsigned short downlinkPort = 0;

/** true when the downlinkPort is set */
bool downlinkPortSet = false;

/**
 @return
 The downlink port (in network byte order)
 */
unsigned short getDownlinkPort(void) {
	if (!downlinkPortSet) {
		downlinkPort = htons(PUD_DOWNLINK_PORT_DEFAULT);
		downlinkPortSet = true;
	}

	return downlinkPort;
}

/**
 Set the downlink port

 @param value
 The downlink port (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setDownlinkPort(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_DOWNLINK_PORT_NAME;
	unsigned long long downlinkPortNew;

	assert(value != NULL);

	if (!readULL(valueName, value, &downlinkPortNew)) {
		return true;
	}

	if ((downlinkPortNew < 1) || (downlinkPortNew > 65535)) {
		pudError(false, "Configured %s (%llu) is outside of"
				" valid range 1-65535", valueName, downlinkPortNew);
		return true;
	}

	downlinkPort = htons(downlinkPortNew);
	downlinkPortSet = true;

	return false;
}

/*
 * txTtl
 */

/** The tx TTL */
static unsigned char txTtl = PUD_TX_TTL_DEFAULT;

/**
 @return
 The transmit multicast IP packet time-to-live
 */
unsigned char getTxTtl(void) {
	return txTtl;
}

/**
 Set the transmit multicast IP packet time-to-live

 @param value
 The transmit multicast IP packet time-to-live (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setTxTtl(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_TTL_NAME;
	unsigned long long txTtlNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &txTtlNew)) {
		return true;
	}

	if ((txTtlNew < 1) || (txTtlNew > MAX_TTL)) {
		pudError(false, "Configured %s (%llu) is outside of"
			" valid range 1-%u", valueName, txTtlNew, MAX_TTL);
		return true;
	}

	txTtl = txTtlNew;

	return false;
}

/*
 * txNmeaMessagePrefix
 */

/** The exact length of the tx NMEA message prefix */
#define PUD_TXNMEAMESSAGEPREFIXLENGTH 4

/** The tx NMEA message prefix buffer */
static unsigned char txNmeaMessagePrefix[PUD_TXNMEAMESSAGEPREFIXLENGTH + 1];

/** True when the tx NMEA message prefix is set */
static bool txNmeaMessagePrefixSet = false;

/**
 @return
 The transmit multicast NMEA message prefix
 */
unsigned char * getTxNmeaMessagePrefix(void) {
	if (!txNmeaMessagePrefixSet) {
		setTxNmeaMessagePrefix(PUD_TX_NMEAMESSAGEPREFIX_DEFAULT, NULL,
				(set_plugin_parameter_addon) {.pc = NULL});
	}
	return &txNmeaMessagePrefix[0];
}

/**
 Set the transmit multicast NMEA message prefix

 @param value
 The transmit multicast NMEA message prefix (in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setTxNmeaMessagePrefix(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_NMEAMESSAGEPREFIX_NAME;
	size_t valueLength;
	bool invalidChars;
	char report[256];

	assert (value != NULL);

	valueLength = strlen(value);
	if (valueLength != PUD_TXNMEAMESSAGEPREFIXLENGTH) {
		pudError(false, "Configured %s (%s) must be %u exactly characters",
				valueName, value, PUD_TXNMEAMESSAGEPREFIXLENGTH);
		return true;
	}

	invalidChars = nmea_string_has_invalid_chars(value, valueName, &report[0],
			sizeof(report));
	if (invalidChars) {
		pudError(false, "%s", &report[0]);
		return true;
	}

	if ((strchr(value, ' ') != NULL) || (strchr(value, '\t') != NULL)) {
		pudError(false, "Configured %s (%s) can not contain whitespace",
				valueName, value);
		return true;
	}

	strcpy((char *) &txNmeaMessagePrefix[0], value);
	txNmeaMessagePrefixSet = true;
	return false;
}

/*
 * olsrTtl
 */

/** The OLSR TTL */
static unsigned char olsrTtl = PUD_OLSR_TTL_DEFAULT;

/**
 @return
 The OLSR multicast IP packet time-to-live
 */
unsigned char getOlsrTtl(void) {
	return olsrTtl;
}

/**
 Set the OLSR multicast IP packet time-to-live

 @param value
 The OLSR multicast IP packet time-to-live (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setOlsrTtl(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_OLSR_TTL_NAME;
	unsigned long long olsrTtlNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &olsrTtlNew)) {
		return true;
	}

	if ((olsrTtlNew < 1) || (olsrTtlNew > MAX_TTL)) {
		pudError(false, "Configured %s (%llu) is outside of valid range 1-%u",
				valueName, olsrTtlNew, MAX_TTL);
		return true;
	}

	olsrTtl = olsrTtlNew;

	return false;
}

/*
 * updateIntervalStationary
 */

/** The stationary interval update plugin parameter (in seconds) */
static unsigned long long updateIntervalStationary = PUD_UPDATE_INTERVAL_STATIONARY_DEFAULT;

/**
 @return
 The stationary interval update plugin parameter (in seconds)
 */
unsigned long long getUpdateIntervalStationary(void) {
	return updateIntervalStationary;
}

/**
 Set stationary interval update plugin parameter

 @param value
 The stationary interval update plugin parameter (in seconds)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUpdateIntervalStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPDATE_INTERVAL_STATIONARY_NAME;
	unsigned long long updateIntervalStationaryNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &updateIntervalStationaryNew)) {
		return true;
	}

	if (updateIntervalStationaryNew < 1) {
		pudError(false, "Configured %s must be at least 1", valueName);
		return true;
	}

	updateIntervalStationary = updateIntervalStationaryNew;

	return false;
}

/*
 * updateIntervalMoving
 */

/** The moving interval update plugin parameter (in seconds) */
static unsigned long long updateIntervalMoving = PUD_UPDATE_INTERVAL_MOVING_DEFAULT;

/**
 @return
 The moving interval update plugin parameter (in seconds)
 */
unsigned long long getUpdateIntervalMoving(void) {
	return updateIntervalMoving;
}

/**
 Set moving interval update plugin parameter

 @param value
 The moving interval update plugin parameter (in seconds)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUpdateIntervalMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPDATE_INTERVAL_MOVING_NAME;
	unsigned long long updateIntervalMovingNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &updateIntervalMovingNew)) {
		return true;
	}

	if (updateIntervalMovingNew < 1) {
		pudError(false, "Configured %s must be at least 1", valueName);
		return true;
	}

	updateIntervalMoving = updateIntervalMovingNew;

	return false;
}

/*
 * uplinkUpdateIntervalStationary
 */

/** The uplink stationary interval update plugin parameter (in seconds) */
static unsigned long long uplinkUpdateIntervalStationary = PUD_UPLINK_UPDATE_INTERVAL_STATIONARY_DEFAULT;

/**
 @return
 The uplink stationary interval update plugin parameter (in seconds)
 */
unsigned long long getUplinkUpdateIntervalStationary(void) {
	return uplinkUpdateIntervalStationary;
}

/**
 Set uplink stationary interval update plugin parameter

 @param value
 The uplink stationary interval update plugin parameter (in seconds)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUplinkUpdateIntervalStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_UPDATE_INTERVAL_STATIONARY_NAME;
	unsigned long long uplinkUpdateIntervalStationaryNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &uplinkUpdateIntervalStationaryNew)) {
		return true;
	}

	if (uplinkUpdateIntervalStationaryNew < 1) {
		pudError(false, "Configured %s must be at least 1", valueName);
		return true;
	}

	uplinkUpdateIntervalStationary = uplinkUpdateIntervalStationaryNew;

	return false;
}

/*
 * uplinkUpdateIntervalMoving
 */

/** The uplink moving interval update plugin parameter (in seconds) */
static unsigned long long uplinkUpdateIntervalMoving = PUD_UPLINK_UPDATE_INTERVAL_MOVING_DEFAULT;

/**
 @return
 The uplink moving interval update plugin parameter (in seconds)
 */
unsigned long long getUplinkUpdateIntervalMoving(void) {
	return uplinkUpdateIntervalMoving;
}

/**
 Set uplink moving interval update plugin parameter

 @param value
 The uplink moving interval update plugin parameter (in seconds)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUplinkUpdateIntervalMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_UPDATE_INTERVAL_MOVING_NAME;
	unsigned long long uplinkUpdateIntervalMovingNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &uplinkUpdateIntervalMovingNew)) {
		return true;
	}

	if (uplinkUpdateIntervalMovingNew < 1) {
		pudError(false, "Configured %s must be at least 1", valueName);
		return true;
	}

	uplinkUpdateIntervalMoving = uplinkUpdateIntervalMovingNew;

	return false;
}

/*
 * gatewayDeterminationInterval
 */

/** The gateway determination interval plugin parameter (in seconds) */
static unsigned long long gatewayDeterminationInterval = PUD_GATEWAY_DETERMINATION_INTERVAL_DEFAULT;

/**
 @return
 The gateway determination interval plugin parameter (in seconds)
 */
unsigned long long getGatewayDeterminationInterval(void) {
	return gatewayDeterminationInterval;
}

/**
 Set gateway determination interval plugin parameter

 @param value
 The gateway determination interval plugin parameter (in seconds)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setGatewayDeterminationInterval(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_GATEWAY_DETERMINATION_INTERVAL_NAME;
	unsigned long long gatewayDeterminationIntervalNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &gatewayDeterminationIntervalNew)) {
		return true;
	}

	if (gatewayDeterminationIntervalNew < 1) {
		pudError(false, "Configured %s must be at least 1", valueName);
		return true;
	}

	gatewayDeterminationInterval = gatewayDeterminationIntervalNew;

	return false;
}

/*
 * movingSpeedThreshold
 */

/** The moving speed threshold plugin parameter (in kph) */
static unsigned long long movingSpeedThreshold = PUD_MOVING_SPEED_THRESHOLD_DEFAULT;

/**
 @return
 The moving speed threshold plugin parameter (in kph)
 */
unsigned long long getMovingSpeedThreshold(void) {
	return movingSpeedThreshold;
}

/**
 Set moving speed threshold plugin parameter

 @param value
 The moving speed threshold plugin parameter (in kph)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setMovingSpeedThreshold(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_MOVING_SPEED_THRESHOLD_NAME;
	unsigned long long movingSpeedThresholdNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &movingSpeedThresholdNew)) {
		return true;
	}

	movingSpeedThreshold = movingSpeedThresholdNew;

	return false;
}

/*
 * movingDistanceThreshold
 */

/** The moving distance threshold plugin parameter (in meters) */
static unsigned long long movingDistanceThreshold = PUD_MOVING_DISTANCE_THRESHOLD_DEFAULT;

/**
 @return
 The moving distance threshold plugin parameter (in meters)
 */
unsigned long long getMovingDistanceThreshold(void) {
	return movingDistanceThreshold;
}

/**
 Set moving distance threshold plugin parameter

 @param value
 The moving distance threshold plugin parameter (in meter)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setMovingDistanceThreshold(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_MOVING_DISTANCE_THRESHOLD_NAME;
	unsigned long long movingDistanceThresholdNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &movingDistanceThresholdNew)) {
		return true;
	}

	movingDistanceThreshold = movingDistanceThresholdNew;

	return false;
}

/*
 * dopMultiplier
 */

/* The DOP multiplier plugin parameter */
static double dopMultiplier = PUD_DOP_MULTIPLIER_DEFAULT;

/**
 @return
 The DOP multiplier plugin parameter
 */
double getDopMultiplier(void) {
	return dopMultiplier;
}

/**
 Set DOP multiplier plugin parameter

 @param value
 The DOP multiplier plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setDopMultiplier(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_DOP_MULTIPLIER_NAME;
	double dopMultiplierNew;

	assert (value != NULL);

	if (!readDouble(valueName, value, &dopMultiplierNew)) {
		return true;
	}

	dopMultiplier = dopMultiplierNew;

	return false;
}

/*
 * defaultHdop
 */

/** The default HDOP plugin parameter (in meters) */
static unsigned long long defaultHdop = PUD_DEFAULT_HDOP_DEFAULT;

/**
 @return
 The default HDOP plugin parameter (in meters)
 */
unsigned long long getDefaultHdop(void) {
	return defaultHdop;
}

/**
 Set default HDOP plugin parameter

 @param value
 The default HDOP plugin parameter (in meters)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setDefaultHdop(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_MOVING_DISTANCE_THRESHOLD_NAME;
	unsigned long long defaultHdopNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &defaultHdopNew)) {
		return true;
	}

	defaultHdop = defaultHdopNew;

	return false;
}

/*
 * defaultVdop
 */

/** The default VDOP plugin parameter (in meters) */
static unsigned long long defaultVdop = PUD_DEFAULT_VDOP_DEFAULT;

/**
 @return
 The default VDOP plugin parameter (in meters)
 */
unsigned long long getDefaultVdop(void) {
	return defaultVdop;
}

/**
 Set default VDOP plugin parameter

 @param value
 The default VDOP plugin parameter (in meters)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setDefaultVdop(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_MOVING_DISTANCE_THRESHOLD_NAME;
	unsigned long long defaultVdopNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &defaultVdopNew)) {
		return true;
	}

	defaultVdop = defaultVdopNew;

	return false;
}

/*
 * averageDepth
 */

/** The depth of the average list */
static unsigned long long averageDepth = PUD_AVERAGE_DEPTH_DEFAULT;

/**
 @return
 The depth of the average list
 */
unsigned long long getAverageDepth(void) {
	return averageDepth;
}

/**
 Set average depth plugin parameter

 @param value
 The average depth plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setAverageDepth(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_AVERAGE_DEPTH_NAME;
	unsigned long long averageDepthNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &averageDepthNew)) {
		return true;
	}

	if (averageDepthNew < 1) {
		pudError(false, "Configured %s must be at least 1", valueName);
		return true;
	}

	averageDepth = averageDepthNew;

	return false;
}

/*
 * hysteresisCountToStationary
 */

/** The hysteresis count for changing state from moving to stationary */
static unsigned long long hysteresisCountToStationary = PUD_HYSTERESIS_COUNT_2STAT_DEFAULT;

/**
 @return
 The hysteresis count for changing state from moving to stationary
 */
unsigned long long getHysteresisCountToStationary(void) {
	return hysteresisCountToStationary;
}

/**
 Set hysteresis count plugin parameter

 @param value
 The hysteresis count plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setHysteresisCountToStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_HYSTERESIS_COUNT_2STAT_NAME;
	unsigned long long hysteresisCountNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &hysteresisCountNew)) {
		return true;
	}

	hysteresisCountToStationary = hysteresisCountNew;

	return false;
}

/*
 * hysteresisCountToMoving
 */

/** The hysteresis count for changing state from stationary to moving */
static unsigned long long hysteresisCountToMoving = PUD_HYSTERESIS_COUNT_2MOV_DEFAULT;

/**
 @return
 The hysteresis count for changing state from stationary to moving
 */
unsigned long long getHysteresisCountToMoving(void) {
	return hysteresisCountToMoving;
}

/**
 Set hysteresis count plugin parameter

 @param value
 The hysteresis count plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setHysteresisCountToMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_HYSTERESIS_COUNT_2MOV_NAME;
	unsigned long long hysteresisCountNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &hysteresisCountNew)) {
		return true;
	}

	hysteresisCountToMoving = hysteresisCountNew;

	return false;
}

/*
 * gatewayHysteresisCountToStationary
 */

/** The hysteresis count for changing state from moving to stationary */
static unsigned long long gatewayHysteresisCountToStationary = PUD_GAT_HYSTERESIS_COUNT_2STAT_DEFAULT;

/**
 @return
 The hysteresis count for changing state from moving to stationary
 */
unsigned long long getGatewayHysteresisCountToStationary(void) {
	return gatewayHysteresisCountToStationary;
}

/**
 Set hysteresis count plugin parameter

 @param value
 The hysteresis count plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setGatewayHysteresisCountToStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_GAT_HYSTERESIS_COUNT_2STAT_NAME;
	unsigned long long hysteresisCountNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &hysteresisCountNew)) {
		return true;
	}

	gatewayHysteresisCountToStationary = hysteresisCountNew;

	return false;
}

/*
 * gatewayHysteresisCountToMoving
 */

/** The hysteresis count for changing state from stationary to moving */
static unsigned long long gatewayHysteresisCountToMoving = PUD_GAT_HYSTERESIS_COUNT_2MOV_DEFAULT;

/**
 @return
 The hysteresis count for changing state from stationary to moving
 */
unsigned long long getGatewayHysteresisCountToMoving(void) {
	return gatewayHysteresisCountToMoving;
}

/**
 Set hysteresis count plugin parameter

 @param value
 The hysteresis count plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setGatewayHysteresisCountToMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_GAT_HYSTERESIS_COUNT_2MOV_NAME;
	unsigned long long hysteresisCountNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &hysteresisCountNew)) {
		return true;
	}

	gatewayHysteresisCountToMoving = hysteresisCountNew;

	return false;
}

/*
 * useDeDup
 */

/* when true then duplicate message detection is performed */
static bool useDeDup = PUD_USE_DEDUP_DEFAULT;

/**
 @return
 The duplicate message detection setting
 */
bool getUseDeDup(void) {
	return useDeDup;
}

/**
 Set duplicate message detection setting plugin parameter

 @param value
 The duplicate message detection setting plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUseDeDup(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_USE_DEDUP_NAME;
	unsigned long long useDeDupNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &useDeDupNew)) {
		return true;
	}

	if ((useDeDupNew != 0) && (useDeDupNew != 1)) {
		pudError(false, "Configured %s must be 0 (false) or 1 (true)",
				valueName);
		return true;
	}

	useDeDup = (useDeDupNew == 1);

	return false;
}

/*
 * deDupDepth
 */

/** The hysteresis count for changing state from stationary to moving */
static unsigned long long deDupDepth = PUD_DEDUP_DEPTH_DEFAULT;

/**
 @return
 The hysteresis count for changing state from stationary to moving
 */
unsigned long long getDeDupDepth(void) {
	return deDupDepth;
}

/**
 Set de-duplication depth plugin parameter

 @param value
 The de-duplication depth plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setDeDupDepth(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_DEDUP_DEPTH_NAME;
	unsigned long long deDupDepthNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &deDupDepthNew)) {
		return true;
	}

	deDupDepth = deDupDepthNew;

	return false;
}

/*
 * useLoopback
 */

/* when true then loopback is performed */
static bool useLoopback = PUD_USE_LOOPBACK_DEFAULT;

/**
 @return
 The loopback usage setting
 */
bool getUseLoopback(void) {
	return useLoopback;
}

/**
 Set loopback usage plugin parameter

 @param value
 The loopback usage plugin parameter
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setUseLoopback(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_USE_LOOPBACK_NAME;
	unsigned long long useLoopbackNew;

	assert (value != NULL);

	if (!readULL(valueName, value, &useLoopbackNew)) {
		return true;
	}

	if ((useLoopbackNew != 0) && (useLoopbackNew != 1)) {
		pudError(false, "Configured %s must be 0 (false) or 1 (true)",
				valueName);
		return true;
	}

	useLoopback = (useLoopbackNew == 1);

	return false;
}

/*
 * Check Functions
 */

/**
 Check the configuration for consistency and validity.

 @return
 - true when the configuration is consistent and valid
 - false otherwise
 */
unsigned int checkConfig(void) {
	int retval = true;

	if (!olsr_cnf->smart_gw_active) {
		pudError(false, "Smart Gateway must be active");
		retval = false;
	}

	if (rxNonOlsrInterfaceCount == 0) {
		pudError(false, "No receive non-OLSR interfaces configured");
		retval = false;
	}

	if (txNonOlsrInterfaceCount == 0) {
		pudError(false, "No transmit non-OLSR interfaces configured");
		retval = false;
	}

	if (!nodeIdSet) {
		if (nodeIdType == PUD_NODEIDTYPE_DNS) {
			char name[PUD_TX_NODEID_BUFFERSIZE + 1];

			errno = 0;
			if (gethostname(&name[0], sizeof(name)) < 0) {
				pudError(true, "Could not get the host name");
				retval = false;
			} else {
				setNodeId(&name[0], NULL,
						(set_plugin_parameter_addon) {.pc = NULL});
			}
		} else if ((nodeIdType != PUD_NODEIDTYPE_MAC) && (nodeIdType
				!= PUD_NODEIDTYPE_IPV4) && (nodeIdType != PUD_NODEIDTYPE_IPV6)) {
			pudError(false, "No node ID set while one is required for"
				" node type %u", nodeIdType);
			retval = false;
		}
	}

	if (!setupNodeIdBinaryAndValidate(nodeIdType)) {
		retval = false;
	}

	if (updateIntervalMoving > updateIntervalStationary) {
		pudError(false,"The update interval for moving situations must not be"
		" larger than that for stationary situations");
		retval = false;
	}

	if (uplinkUpdateIntervalMoving > uplinkUpdateIntervalStationary) {
		pudError(false,"The uplink update interval for moving situations must not be"
		" larger than that for stationary situations");
		retval = false;
	}

	if (getUplinkPort() == getDownlinkPort()) {
		pudError(false, "The uplink port and the downlink port must not be the same");
		retval = false;
	}

	return retval;
}

/**
 Check the configuration for consistency and validity after everything has been
 setup.

 @return
 - true when the configuration is consistent and valid
 - false otherwise
 */
unsigned int checkRunSetup(void) {
	int retval = true;
	unsigned int i;

	/* any receive interface name that is configured but is not the name of an
	 * actual receive interface is not a valid interface name */
	for (i = 0; i < rxNonOlsrInterfaceCount; i++) {
		unsigned char * nonOlsrInterfaceName = &rxNonOlsrInterfaceNames[i][0];

		TRxTxNetworkInterface * interfaceObject = getRxNetworkInterfaces();
		bool found = false;
		while (interfaceObject != NULL) {
			if (strncmp((char *) nonOlsrInterfaceName,
					(char *) &interfaceObject->name[0], IFNAMSIZ + 1) == 0) {
				found = true;
				break;
			}
			interfaceObject = interfaceObject->next;
		}
		if (!found) {
			pudError(false, "Configured receive non-OLSR interface %s is not"
				" a known interface name", nonOlsrInterfaceName);
			retval = false;
		}
	}

	/* any transmit interface name that is configured but is not the name of an
	 * actual transmit interface is not a valid interface name */
	for (i = 0; i < txNonOlsrInterfaceCount; i++) {
		unsigned char * nonOlsrInterfaceName = &txNonOlsrInterfaceNames[i][0];

		TRxTxNetworkInterface * interfaceObject = getTxNetworkInterfaces();
		bool found = false;
		while (interfaceObject != NULL) {
			if (strncmp((char *) nonOlsrInterfaceName,
					(char *) &interfaceObject->name[0], IFNAMSIZ + 1) == 0) {
				found = true;
				break;
			}
			interfaceObject = interfaceObject->next;
		}
		if (!found) {
			pudError(false, "Configured transmit non-OLSR interface %s is not"
				" a known interface name", nonOlsrInterfaceName);
			retval = false;
		}
	}

	return retval;
}
