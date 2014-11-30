#include "configuration.h"

/* Plugin includes */
#include "pud.h"
#include "networkInterfaces.h"
#include "netTools.h"
#include "posFile.h"
#include "configTools.h"

/* OLSR includes */
#include <olsr_protocol.h>
#include <olsr.h>

/* System includes */
#include <unistd.h>
#include <nmea/parse.h>
#include <OlsrdPudWireFormat/nodeIdConversion.h>
#include <limits.h>

/* forward declarations */
static bool setupNodeIdBinaryAndValidate(NodeIdType nodeIdTypeNumber);

/*
 * Note:
 * Setters must return true when an error is detected, false otherwise
 */

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

static int setNodeIdType(const char *value, const char * valueName) {
	unsigned long long nodeIdTypeNew;

	if (!readULL(valueName, value, &nodeIdTypeNew, 10)) {
		return true;
	}

	if (!isValidNodeIdType(nodeIdTypeNew)) {
		pudError(false, "Value in parameter %s (%llu) is reserved", valueName,
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
static unsigned char nodeId[PUD_TX_NODEID_BUFFERSIZE];

/** The length of the string in the nodeId buffer */
static size_t nodeIdLength = 0;

/** True when the nodeId is set */
static bool nodeIdSet = false;

/** The nodeId as a binary representation, with status */
static nodeIdBinaryType nodeIdBinary;

/**
 Get the nodeId and its length

 @param length
 a pointer to the variable in which to store the nodeId length (allowed to be
 NULL, in which case the length is not stored)

 @return
 The node ID
 */
unsigned char * getNodeId(size_t *length) {
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

int setNodeId(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	size_t valueLength;
	char * number;
	char * identification;

	assert (value != NULL);

  nodeId[0] = '\0';
  nodeIdLength = 0;
  nodeIdSet = false;
  nodeIdBinary.set = false;

	valueLength = strlen(value);
  number = olsr_malloc(valueLength + 1, "setNodeId");
  strcpy(number, value);

  /* split "number,identification" */
  identification = strchr(number, ',');
  if (identification) {
    *identification = '\0';
    identification++;
  }

  /* parse number into nodeIdType (if present) */
  valueLength = strlen(number);
  if (valueLength && setNodeIdType(number, PUD_NODE_ID_NAME)) {
    free(number);
    return true;
  }

  /* copy identification into nodeId (if present) */
  if (identification) {
    valueLength = strlen(identification);
    if (valueLength > (PUD_TX_NODEID_BUFFERSIZE - 1)) {
      pudError(false, "Value in parameter %s is too long, maximum length is"
        " %u, current length is %lu", PUD_NODE_ID_NAME, (PUD_TX_NODEID_BUFFERSIZE - 1),
          (unsigned long) valueLength);
      free(number);
      return true;
    }

    if (valueLength) {
      strcpy((char *) &nodeId[0], identification);
      nodeIdLength = valueLength;
      nodeIdSet = true;
    }
  }

  free(number);

  /* fill in automatic values */
  if (!nodeIdSet) {
    if (nodeIdType == PUD_NODEIDTYPE_DNS) {
      memset(nodeId, 0, sizeof(nodeId));
      errno = 0;
      if (gethostname((char *)&nodeId[0], sizeof(nodeId) - 1) < 0) {
        pudError(true, "Could not get the host name");
        return true;
      }

      nodeIdLength = strlen((char *)nodeId);
      nodeIdSet = true;
    } else if ((nodeIdType != PUD_NODEIDTYPE_MAC) && (nodeIdType != PUD_NODEIDTYPE_IPV4)
        && (nodeIdType != PUD_NODEIDTYPE_IPV6)) {
      pudError(false, "No node ID set while one is required for nodeId type %u", nodeIdType);
      return true;
    }
  }

  if (!setupNodeIdBinaryAndValidate(nodeIdType)) {
    pudError(false, "nodeId (type %u) is incorrectly configured", nodeIdType);
    return true;
  }

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
  if (!readULL(PUD_NODE_ID_NAME, (char *) getNodeId(NULL), &longValue, 10)) {
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
 nodeIdType, for types that fit in a double unsigned long long (128 bits) with
 a certain split that defined by chars1

 @param chars1
 the number of characters of the first part
 @param min1
 the minimum value of the first part
 @param max1
 the maximum value of the first part
 @param bytes1
 the number of bytes of the first part in the buffer
 @param min2
 the minimum value of the second part
 @param max2
 the maximum value of the second part
 @param bytes2
 the number of bytes of the second part in the buffer
 @param base
 the base of the number conversion: 10 for decimal, 16 for hexadecimal

 @return
 - true when ok
 - false on failure
 */
static bool intSetupNodeIdBinaryDoubleLongLong(
    unsigned char * dst,
    unsigned int chars1,
    unsigned long long min1, unsigned long long max1,
    unsigned int bytes1,
		unsigned long long min2, unsigned long long max2,
		unsigned int bytes2,
		int base) {
	unsigned long long longValue1 = 0;
	unsigned long long longValue2 = 0;

	unsigned char * node_id = getNodeId(NULL);
	size_t node_id_len = strlen((char *)node_id);

	assert(chars1 > 0);
	assert(bytes1 > 0);
	assert(bytes2 > 0);

  /* part 1 */
	if (node_id_len > 0) {
    unsigned char first[chars1 + 1];
    int cpylen = node_id_len < chars1 ? node_id_len : chars1;

    memcpy(first, node_id, cpylen);
    first[cpylen] = '\0';

    if (!readULL(PUD_NODE_ID_NAME, (char *)first, &longValue1, base)) {
      return false;
    }

    if ((longValue1 < min1) || (longValue1 > max1)) {
      pudError(false, "First %u character(s) of %s value %llu are out of range [%llu,%llu]",
          cpylen, PUD_NODE_ID_NAME, longValue1, min1, max1);
      return false;
    }
	}

  /* part 2 */
	if (node_id_len > chars1) {
    if (!readULL(PUD_NODE_ID_NAME, (char *)&node_id[chars1], &longValue2, base)) {
      return false;
    }

    if ((longValue2 < min2) || (longValue2 > max2)) {
      pudError(false, "Last %u character(s) of %s value %llu are out of range [%llu,%llu]",
          (unsigned int)(node_id_len - chars1), PUD_NODE_ID_NAME, longValue2, min2, max2);
      return false;
    }
  } else {
    /* longvalue1 is the only value, so it is the least significant value:
     * exchange the 2 values */
    unsigned long long tmp = longValue1;
    longValue1 = longValue2;
    longValue2 = tmp;
  }

	return setupNodeIdBinaryDoubleLongLong(&nodeIdBinary,
	    longValue1, &dst[0], bytes1,
	    longValue2, &dst[bytes1], bytes2);
}

/**
 Validate whether the configured nodeId is valid w.r.t. the configured
 nodeIdType, for types that are strings

 @return
 - true when ok
 - false on failure
 */
static bool intSetupNodeIdBinaryString(void) {
  const char * invalidCharName;
  size_t nodeidlength;
  char * nodeid = (char *) getNodeId(&nodeidlength);

  invalidCharName = nmea_parse_sentence_has_invalid_chars(nodeid, nodeidlength);
  if (invalidCharName) {
    char report[256];
    snprintf(report, sizeof(report), "Configured %s (%s),"
        " contains invalid NMEA characters (%s)", PUD_NODE_ID_NAME, nodeid, invalidCharName);
    pudError(false, "%s", &report[0]);
    return false;
  }

  if (nodeidlength > (PUD_TX_NODEID_BUFFERSIZE - 1)) {
    pudError(false, "Length of parameter %s (%s) is too great", PUD_NODE_ID_NAME, &nodeid[0]);
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
 nodeIdType and setup the binary value

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

		case PUD_NODEIDTYPE_IPV4: /* IPv4 address */
		case PUD_NODEIDTYPE_IPV6: /* IPv6 address */
			return intSetupNodeIdBinaryIp();

		case PUD_NODEIDTYPE_UUID: /* a UUID number */
			return intSetupNodeIdBinaryDoubleLongLong(
			    &nodeIdBinary.buffer.uuid[0],
			    PUD_NODEIDTYPE_UUID_CHARS1,
			    PUD_NODEIDTYPE_UUID_MIN1, PUD_NODEIDTYPE_UUID_MAX1,
			    PUD_NODEIDTYPE_UUID_BYTES1,
			    PUD_NODEIDTYPE_UUID_MIN2, PUD_NODEIDTYPE_UUID_MAX2,
			    PUD_NODEIDTYPE_UUID_BYTES - PUD_NODEIDTYPE_UUID_BYTES1,
			    16);

		case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_MMSI_MIN,
				PUD_NODEIDTYPE_MMSI_MAX, PUD_NODEIDTYPE_MMSI_BYTES);

		case PUD_NODEIDTYPE_URN: /* a URN number */
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_URN_MIN,
				PUD_NODEIDTYPE_URN_MAX, PUD_NODEIDTYPE_URN_BYTES);

		case PUD_NODEIDTYPE_MIP: /* a MIP OID number */
			return intSetupNodeIdBinaryDoubleLongLong(
			    &nodeIdBinary.buffer.mip[0],
			    PUD_NODEIDTYPE_MIP_CHARS1,
			    PUD_NODEIDTYPE_MIP_MIN1, PUD_NODEIDTYPE_MIP_MAX1,
			    PUD_NODEIDTYPE_MIP_BYTES1,
			    PUD_NODEIDTYPE_MIP_MIN2, PUD_NODEIDTYPE_MIP_MAX2,
			    PUD_NODEIDTYPE_MIP_BYTES - PUD_NODEIDTYPE_MIP_BYTES1,
			    10);

		case PUD_NODEIDTYPE_192:
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_192_MIN,
				PUD_NODEIDTYPE_192_MAX, PUD_NODEIDTYPE_192_BYTES);

		case PUD_NODEIDTYPE_193:
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_193_MIN,
				PUD_NODEIDTYPE_193_MAX, PUD_NODEIDTYPE_193_BYTES);

		case PUD_NODEIDTYPE_194:
			return intSetupNodeIdBinaryLongLong(PUD_NODEIDTYPE_194_MIN,
				PUD_NODEIDTYPE_194_MAX, PUD_NODEIDTYPE_194_BYTES);

		default:
		  pudError(false, "nodeId type %u is not supported", nodeIdTypeNumber);
		  return false;
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

int addRxNonOlsrInterface(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	size_t valueLength;

	if (rxNonOlsrInterfaceCount >= PUD_RX_NON_OLSR_IF_MAX) {
		pudError(false, "Can't configure more than %u receive interfaces",
				PUD_RX_NON_OLSR_IF_MAX);
		return true;
	}

	assert (value != NULL);

	valueLength = strlen(value);
	if (valueLength > IFNAMSIZ) {
		pudError(false, "Value of parameter %s (%s) is too long,"
			" maximum length is %u, current length is %lu",
				PUD_RX_NON_OLSR_IF_NAME, value, IFNAMSIZ, (long unsigned int)valueLength);
		return true;
	}

	if (!isRxNonOlsrInterface(value)) {
		strcpy((char *) &rxNonOlsrInterfaceNames[rxNonOlsrInterfaceCount][0],
				value);
		rxNonOlsrInterfaceCount++;
	}

	return false;
}

/**
 * @return the number of configured non-olsr receive interfaces
 */
unsigned int getRxNonOlsrInterfaceCount(void) {
	return rxNonOlsrInterfaceCount;
}

/**
 * @param idx the index of the configured non-olsr receive interface
 * @return the index-th interface name
 */
unsigned char * getRxNonOlsrInterfaceName(unsigned int idx) {
	return &rxNonOlsrInterfaceNames[idx][0];
}

/*
 * rxAllowedSourceIpAddress
 */

/** The maximum number of RX allowed source IP addresses */
#define PUD_RX_ALLOWED_SOURCE_IP_MAX 32

/** Array with RX allowed source IP addresses */
static union olsr_sockaddr rxAllowedSourceIpAddresses[PUD_RX_ALLOWED_SOURCE_IP_MAX];

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
bool isRxAllowedSourceIpAddress(union olsr_sockaddr * sender) {
	unsigned int i;

	if (rxAllowedSourceIpAddressesCount == 0) {
		return true;
	}

	if (sender == NULL) {
		return false;
	}

	for (i = 0; i < rxAllowedSourceIpAddressesCount; i++) {
		if (sender->in.sa_family != rxAllowedSourceIpAddresses[i].in.sa_family) {
			continue;
		}

		if (sender->in.sa_family == AF_INET) {
			if (memcmp(&rxAllowedSourceIpAddresses[i].in4.sin_addr, &sender->in4.sin_addr, sizeof(struct in_addr))
					== 0) {
				return true;
			}
		} else {
			if (memcmp(&rxAllowedSourceIpAddresses[i].in6.sin6_addr, &sender->in6.sin6_addr, sizeof(struct in6_addr))
					== 0) {
				return true;
			}
		}
	}

	return false;
}

int addRxAllowedSourceIpAddress(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_RX_ALLOWED_SOURCE_IP_NAME;
	union olsr_sockaddr addr;
	bool addrSet = false;

	if (rxAllowedSourceIpAddressesCount >= PUD_RX_ALLOWED_SOURCE_IP_MAX) {
		pudError(false, "Can't configure more than %u allowed source IP"
			" addresses", PUD_RX_ALLOWED_SOURCE_IP_MAX);
		return true;
	}

	if (!readIPAddress(valueName, value, 0, &addr, &addrSet)) {
		return true;
	}

	if (!isRxAllowedSourceIpAddress(&addr)) {
		rxAllowedSourceIpAddresses[rxAllowedSourceIpAddressesCount] = addr;
		rxAllowedSourceIpAddressesCount++;
	}

	return false;
}

/*
 * rxMcAddr + rxMcPort
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
		setRxMcAddr((olsr_cnf->ip_version == AF_INET) ? PUD_RX_MC_ADDR_4_DEFAULT : PUD_RX_MC_ADDR_6_DEFAULT,
				NULL, ((set_plugin_parameter_addon) {.pc = NULL}));
	}
	return &rxMcAddr;
}

int setRxMcAddr(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_RX_MC_ADDR_NAME;

	if (!readIPAddress(valueName, value, PUD_RX_MC_PORT_DEFAULT, &rxMcAddr, &rxMcAddrSet)) {
			return true;
	}

	if (!isMulticast(&rxMcAddr)) {
		pudError(false, "Value of parameter %s (%s) is not a multicast address",
				valueName, value);
		return true;
	}

	return false;
}

/**
 @return
 The receive multicast port (in network byte order)
 */
unsigned short getRxMcPort(void) {
	return getOlsrSockaddrPort(getRxMcAddr(), PUD_RX_MC_PORT_DEFAULT);
}

int setRxMcPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_RX_MC_PORT_NAME;
	unsigned short rxMcPortNew;

	if (!readUS(valueName, value, &rxMcPortNew)) {
		return true;
	}

	if (rxMcPortNew < 1) {
		pudError(false, "Value of parameter %s (%u) is outside of valid range 1-65535", valueName, rxMcPortNew);
		return true;
	}

	setOlsrSockaddrPort(getRxMcAddr(), htons((in_port_t) rxMcPortNew));

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

int setPositionFile(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	size_t valueLength;

	assert(value != NULL);

	valueLength = strlen(value);
	if (valueLength > PATH_MAX) {
		pudError(false, "Value of parameter %s is too long, maximum length is"
				" %u, current length is %lu", PUD_POSFILE_NAME, PATH_MAX, (unsigned long) valueLength);
		return true;
	}

	strcpy((char *) &positionFile[0], value);
	positionFileSet = true;

	return false;
}

/*
 * positionFilePeriod
 */

/** The positionFilePeriod value (milliseconds) */
unsigned long long positionFilePeriod = PUD_POSFILEPERIOD_DEFAULT;

/**
 @return
 The positionFilePeriod (in milliseconds)
 */
unsigned long long getPositionFilePeriod(void) {
	return positionFilePeriod;
}

/**
 Set the positionFilePeriod

 @param value
 The positionFilePeriod (a number in string representation)
 @param data
 Unused
 @param addon
 Unused

 @return
 - true when an error is detected
 - false otherwise
 */
int setPositionFilePeriod(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_POSFILEPERIOD_NAME;
	unsigned long long positionFilePeriodNew;

	assert(value != NULL);

	if (!readULL(valueName, value, &positionFilePeriodNew, 10)) {
		return true;
	}

	if ((positionFilePeriodNew != 0)
			&& ((positionFilePeriodNew < PUD_POSFILEPERIOD_MIN) || (positionFilePeriodNew > PUD_POSFILEPERIOD_MAX))) {
		pudError(false, "Configured %s (%llu) is outside of"
				" valid range %llu-%llu", valueName, positionFilePeriodNew, PUD_POSFILEPERIOD_MIN, PUD_POSFILEPERIOD_MAX);
		return true;
	}

	positionFilePeriod = positionFilePeriodNew;

	return false;
}

/*
 * txNonOlsrIf
 */

/** The maximum number of tx non-olsr interfaces */
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
		if (strncmp((char *) &txNonOlsrInterfaceNames[i][0], ifName, IFNAMSIZ + 1) == 0) {
			return true;
		}
	}

	return false;
}

int addTxNonOlsrInterface(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	size_t valueLength;

	assert (value != NULL);

	if (txNonOlsrInterfaceCount >= PUD_TX_NON_OLSR_IF_MAX) {
		pudError(false, "Can not configure more than %u transmit interfaces", PUD_TX_NON_OLSR_IF_MAX);
		return true;
	}

	valueLength = strlen(value);
	if (valueLength > IFNAMSIZ) {
		pudError(false, "Value of parameter %s (%s) is too long, maximum length is %u, current length is %lu",
				PUD_TX_NON_OLSR_IF_NAME, value, IFNAMSIZ, (long unsigned int)valueLength);
		return true;
	}

	if (!isTxNonOlsrInterface(value)) {
		strcpy((char *) &txNonOlsrInterfaceNames[txNonOlsrInterfaceCount][0], value);
		txNonOlsrInterfaceCount++;
	}

	return false;
}

/**
 * @return the number of configured non-olsr transmit interfaces
 */
unsigned int getTxNonOlsrInterfaceCount(void) {
	return txNonOlsrInterfaceCount;
}

/**
 * @param idx the index of the configured non-olsr transmit interface
 * @return the index-th interface name
 */
unsigned char * getTxNonOlsrInterfaceName(unsigned int idx) {
	return &txNonOlsrInterfaceNames[idx][0];
}

/*
 * txMcAddr + txMcPort
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
		setTxMcAddr((olsr_cnf->ip_version == AF_INET) ? PUD_TX_MC_ADDR_4_DEFAULT : PUD_TX_MC_ADDR_6_DEFAULT,
				NULL, ((set_plugin_parameter_addon) {.pc = NULL}));
	}
	return &txMcAddr;
}

int setTxMcAddr(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_MC_ADDR_NAME;

	if (!readIPAddress(valueName, value, PUD_TX_MC_PORT_DEFAULT, &txMcAddr, &txMcAddrSet)) {
			return true;
	}

	if (!isMulticast(&txMcAddr)) {
		pudError(false, "Value of parameter %s (%s) is not a multicast address",
				valueName, value);
		return true;
	}

	return false;
}

/**
 @return
 The transmit multicast port (in network byte order)
 */
unsigned short getTxMcPort(void) {
	return getOlsrSockaddrPort(getTxMcAddr(), PUD_TX_MC_PORT_DEFAULT);
}

int setTxMcPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_MC_PORT_NAME;
	unsigned short txMcPortNew;

	if (!readUS(valueName, value, &txMcPortNew)) {
		return true;
	}

	if (txMcPortNew < 1) {
		pudError(false, "Value of parameter %s (%u) is outside of valid range 1-65535", valueName, txMcPortNew);
		return true;
	}

	setOlsrSockaddrPort(getTxMcAddr(), htons((in_port_t) txMcPortNew));

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

int setTxTtl(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_TX_TTL_NAME;

	if (!readUC(valueName, value, &txTtl)) {
		return true;
	}

	if ((txTtl < 1) /* || (txTtl > MAX_TTL) */) {
		pudError(false, "Value of parameter %s (%u) is outside of"
			" valid range 1-%u", valueName, txTtl, MAX_TTL);
		return true;
	}

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

int setTxNmeaMessagePrefix(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	const char * invalidCharName;
	static const char * valueName = PUD_TX_NMEAMESSAGEPREFIX_NAME;
	size_t valueLength;

	assert (value != NULL);

	valueLength = strlen(value);
	if (valueLength != PUD_TXNMEAMESSAGEPREFIXLENGTH) {
		pudError(false, "Length of parameter %s (%s) must be exactly %u characters",
				valueName, value, PUD_TXNMEAMESSAGEPREFIXLENGTH);
		return true;
	}

  invalidCharName = nmea_parse_sentence_has_invalid_chars(value, valueLength);
  if (invalidCharName) {
    char report[256];
    snprintf(report, sizeof(report), "Configured %s (%s),"
        " contains invalid NMEA characters (%s)", valueName, value, invalidCharName);
    pudError(false, "%s", report);
    return true;
  }

	if ((strchr(value, ' ') != NULL) || (strchr(value, '\t') != NULL)) {
		pudError(false, "Value of parameter %s (%s) can not contain whitespace",
				valueName, value);
		return true;
	}

	strcpy((char *) &txNmeaMessagePrefix[0], value);
	txNmeaMessagePrefixSet = true;
	return false;
}

/*
 * uplinkAddr + uplinkPort
 */

/** The uplink address */
static union olsr_sockaddr uplinkAddr;

/** True when the uplink address is set */
static bool uplinkAddrSet = false;

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
		setUplinkAddr((olsr_cnf->ip_version == AF_INET) ? PUD_UPLINK_ADDR_4_DEFAULT : PUD_UPLINK_ADDR_6_DEFAULT,
				NULL, ((set_plugin_parameter_addon) {.pc = NULL}));
	}
	return &uplinkAddr;
}

int setUplinkAddr(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readIPAddress(PUD_UPLINK_ADDR_NAME, value, PUD_UPLINK_PORT_DEFAULT, &uplinkAddr, &uplinkAddrSet);
}

/**
 @return
 The uplink port (in network byte order)
 */
unsigned short getUplinkPort(void) {
	return getOlsrSockaddrPort(getUplinkAddr(), PUD_UPLINK_PORT_DEFAULT);
}

int setUplinkPort(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_PORT_NAME;
	unsigned short uplinkPortNew;

	if (!readUS(valueName, value, &uplinkPortNew)) {
		return true;
	}

	if (uplinkPortNew < 1) {
		pudError(false, "Value of parameter %s (%u) is outside of valid range 1-65535", valueName, uplinkPortNew);
		return true;
	}

	setOlsrSockaddrPort(getUplinkAddr(), htons((in_port_t) uplinkPortNew));

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

int setDownlinkPort(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_DOWNLINK_PORT_NAME;
	unsigned short downlinkPortNew;

	if (!readUS(valueName, value, &downlinkPortNew)) {
		return true;
	}

	if (downlinkPortNew < 1) {
		pudError(false, "Value of parameter %s (%u) is outside of valid range 1-65535", valueName, downlinkPortNew);
		return true;
	}

	downlinkPort = htons(downlinkPortNew);
	downlinkPortSet = true;

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

int setOlsrTtl(const char *value, void *data __attribute__ ((unused)), set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_OLSR_TTL_NAME;

	if (!readUC(valueName, value, &olsrTtl)) {
		return true;
	}

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

int setUpdateIntervalStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPDATE_INTERVAL_STATIONARY_NAME;

	if (!readULL(valueName, value, &updateIntervalStationary, 10)) {
		return true;
	}

	if (updateIntervalStationary < 1) {
		pudError(false, "Value of parameter %s must be at least 1", valueName);
		return true;
	}

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

int setUpdateIntervalMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPDATE_INTERVAL_MOVING_NAME;

	if (!readULL(valueName, value, &updateIntervalMoving, 10)) {
		return true;
	}

	if (updateIntervalMoving < 1) {
		pudError(false, "Value of parameter %s must be at least 1", valueName);
		return true;
	}

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

int setUplinkUpdateIntervalStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_UPDATE_INTERVAL_STATIONARY_NAME;

	if (!readULL(valueName, value, &uplinkUpdateIntervalStationary, 10)) {
		return true;
	}

	if (uplinkUpdateIntervalStationary < 1) {
		pudError(false, "Value of parameter %s must be at least 1", valueName);
		return true;
	}

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

int setUplinkUpdateIntervalMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_UPLINK_UPDATE_INTERVAL_MOVING_NAME;

	if (!readULL(valueName, value, &uplinkUpdateIntervalMoving, 10)) {
		return true;
	}

	if (uplinkUpdateIntervalMoving < 1) {
		pudError(false, "Value of parameter %s must be at least 1", valueName);
		return true;
	}

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

int setGatewayDeterminationInterval(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_GATEWAY_DETERMINATION_INTERVAL_NAME;

	if (!readULL(valueName, value, &gatewayDeterminationInterval, 10)) {
		return true;
	}

	if (gatewayDeterminationInterval < 1) {
		pudError(false, "Value of parameter %s must be at least 1", valueName);
		return true;
	}

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

int setMovingSpeedThreshold(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_MOVING_SPEED_THRESHOLD_NAME, value, &movingSpeedThreshold, 10);
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

int setMovingDistanceThreshold(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_MOVING_DISTANCE_THRESHOLD_NAME, value, &movingDistanceThreshold, 10);
}

/*
 * dopMultiplier
 */

/** The DOP multiplier plugin parameter */
static double dopMultiplier = PUD_DOP_MULTIPLIER_DEFAULT;

/**
 @return
 The DOP multiplier plugin parameter
 */
double getDopMultiplier(void) {
	return dopMultiplier;
}

int setDopMultiplier(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readDouble(PUD_DOP_MULTIPLIER_NAME, value, &dopMultiplier);
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

int setDefaultHdop(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_DEFAULT_HDOP_NAME, value, &defaultHdop, 10);
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

int setDefaultVdop(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_DEFAULT_VDOP_NAME, value, &defaultVdop, 10);
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

int setAverageDepth(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	static const char * valueName = PUD_AVERAGE_DEPTH_NAME;

	if (!readULL(valueName, value, &averageDepth, 10)) {
		return true;
	}

	if (averageDepth < 1) {
		pudError(false, "Value of parameter %s must be at least 1", valueName);
		return true;
	}

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

int setHysteresisCountToStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_HYSTERESIS_COUNT_2STAT_NAME, value, &hysteresisCountToStationary, 10);
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

int setHysteresisCountToMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_HYSTERESIS_COUNT_2MOV_NAME, value, &hysteresisCountToMoving, 10);
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

int setGatewayHysteresisCountToStationary(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_GAT_HYSTERESIS_COUNT_2STAT_NAME, value, &gatewayHysteresisCountToStationary, 10);
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

int setGatewayHysteresisCountToMoving(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_GAT_HYSTERESIS_COUNT_2MOV_NAME, value, &gatewayHysteresisCountToMoving, 10);
}

/*
 * useDeDup
 */

/** when true then duplicate message detection is performed */
static bool useDeDup = PUD_USE_DEDUP_DEFAULT;

/**
 @return
 The duplicate message detection setting
 */
bool getUseDeDup(void) {
	return useDeDup;
}

int setUseDeDup(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readBool(PUD_USE_DEDUP_NAME, value, &useDeDup);
}

/*
 * deDupDepth
 */

/** The deduplication depth */
static unsigned long long deDupDepth = PUD_DEDUP_DEPTH_DEFAULT;

/**
 @return
 The deduplication depth
 */
unsigned long long getDeDupDepth(void) {
	return deDupDepth;
}

int setDeDupDepth(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readULL(PUD_DEDUP_DEPTH_NAME, value, &deDupDepth, 10);
}

/*
 * useLoopback
 */

/** when true then loopback is performed */
static bool useLoopback = PUD_USE_LOOPBACK_DEFAULT;

/**
 @return
 The loopback usage setting
 */
bool getUseLoopback(void) {
	return useLoopback;
}

int setUseLoopback(const char *value, void *data __attribute__ ((unused)),
		set_plugin_parameter_addon addon __attribute__ ((unused))) {
	return !readBool(PUD_USE_LOOPBACK_NAME, value, &useLoopback);
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

	if (rxNonOlsrInterfaceCount == 0) {
		pudError(false, "No receive non-OLSR interfaces configured");
		retval = false;
	}

	if (txNonOlsrInterfaceCount == 0) {
		pudError(false, "No transmit non-OLSR interfaces configured");
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

	if (isUplinkAddrSet() && (getUplinkPort() == getDownlinkPort())) {
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
	return true;
}
