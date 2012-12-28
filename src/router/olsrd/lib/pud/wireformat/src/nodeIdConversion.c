#include <OlsrdPudWireFormat/nodeIdConversion.h>

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

/* ************************************************************************
 * Node Information
 * ************************************************************************ */

/**
 Convert the nodeIdType of an OLSR message into a string.

 @param ipVersion
 The ip version (AF_INET or AF_INET6)
 @param olsrGpsMessage
 A pointer to the position update message.
 @param nodeIdTypeBuffer
 A pointer to the buffer in which the nodeIdType string representation is
 written (the buffer needs to be at least PUD_TX_NODEIDTYPE_DIGITS bytes).
 When NULL then the nodeIdType string is not written.
 @param nodeIdTypeBufferSize
 The size of the nodeIdTypeBuffer. When zero then the nodeIdType string is not
 written.
 */
void getNodeTypeStringFromOlsr(int ipVersion,
		PudOlsrPositionUpdate * olsrGpsMessage, char * nodeIdTypeBuffer,
		int nodeIdTypeBufferSize) {
	if (unlikely(!nodeIdTypeBuffer || (nodeIdTypeBufferSize == 0))) {
		return;
	}

	assert(nodeIdTypeBufferSize >= PUD_TX_NODEIDTYPE_DIGITS);

	/* message has NO nodeId information */
	snprintf(&nodeIdTypeBuffer[0], nodeIdTypeBufferSize, "%u",
			getPositionUpdateNodeIdType(ipVersion, olsrGpsMessage));
	return;
}

/**
 Get a nodeId number (in string representation), using a certain number of
 bytes, from the message of an OLSR message.

 @param buffer
 A pointer to the buffer that holds the nodeId
 @param bufferSize
 The number of bytes used by the number in the buffer
 @param nodeIdBuffer
 The buffer in which to place the nodeId number in string representation
 @param nodeIdBufferSize
 The size of the nodeIdbuffer

 @return
 A pointer to the nodeId string representation (&nodeIdBuffer[0])
 */
static char *getNodeIdNumberFromOlsr(unsigned char * buffer,
		unsigned int bufferSize, char *nodeIdBuffer, socklen_t nodeIdBufferSize) {
	unsigned long long val = 0;
	unsigned int i = 0;

	while (i < bufferSize) {
		val <<= 8;
		val += buffer[i];
		i++;
	}

	snprintf(nodeIdBuffer, nodeIdBufferSize, "%llu", val);
	return &nodeIdBuffer[0];
}

/**
 Convert the nodeId of an OLSR message into a string.

 @param ipVersion
 The ip version (AF_INET or AF_INET6)
 @param olsrMessage
 A pointer to the OLSR message. Used to be able to retrieve the IP address of
 the sender.
 @param nodeIdStr
 A pointer to a variable in which to store the pointer to the buffer in which
 the nodeId string representation is written (the buffer needs to be at least
 PUD_TX_NODEID_BUFFERSIZE bytes). Not written to when nodeIdStrBuffer or
 nodeIdStr is NULL or when nodeIdStrBufferSize is zero. Can point to
 nodeIdStrBuffer or straight into the olsrMessage
 @param nodeIdStrBuffer
 A pointer to the buffer in which the nodeId string representation can be
 written. Not written to when nodeIdStrBuffer or nodeIdStr is NULL or when
 nodeIdStrBufferSize is zero.
 @param nodeIdStrBufferSize
 The size of the nodeIdStrBuffer. When zero then nodeIdStrBuffer and nodeIdStr
 are not written to.
 */
void getNodeIdStringFromOlsr(int ipVersion, union olsr_message *olsrMessage,
		const char **nodeIdStr, char *nodeIdStrBuffer,
		unsigned int nodeIdStrBufferSize) {
	PudOlsrPositionUpdate * olsrGpsMessage;
	unsigned char * nodeId;
	unsigned int nodeIdSize;

	if (unlikely(!nodeIdStrBuffer || (nodeIdStrBufferSize == 0) || !nodeIdStr)) {
		return;
	}

	assert(nodeIdStrBufferSize >= PUD_TX_NODEID_BUFFERSIZE);

	olsrGpsMessage = getOlsrMessagePayload(ipVersion, olsrMessage);

	getPositionUpdateNodeId(ipVersion, olsrMessage, &nodeId, &nodeIdSize);

	switch (getPositionUpdateNodeIdType(ipVersion, olsrGpsMessage)) {
	case PUD_NODEIDTYPE_MAC: /* hardware address */
	{
		assert(nodeIdSize == 6);

		snprintf(nodeIdStrBuffer, nodeIdStrBufferSize,
				"%02x:%02x:%02x:%02x:%02x:%02x", nodeId[0], nodeId[1],
				nodeId[2], nodeId[3], nodeId[4], nodeId[5]);
		*nodeIdStr = &nodeIdStrBuffer[0];
	}
		break;

	case PUD_NODEIDTYPE_DNS: /* DNS name */
		*nodeIdStr = (char *) nodeId;
		break;

	case PUD_NODEIDTYPE_MSISDN: /* an MSISDN number */
	case PUD_NODEIDTYPE_TETRA: /* a Tetra number */
	case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
	case PUD_NODEIDTYPE_URN: /* a URN number */
	case PUD_NODEIDTYPE_192:
	case PUD_NODEIDTYPE_193:
	case PUD_NODEIDTYPE_194:
		*nodeIdStr = getNodeIdNumberFromOlsr(nodeId, nodeIdSize,
				nodeIdStrBuffer, nodeIdStrBufferSize);
		break;

	case PUD_NODEIDTYPE_IPV4: /* IPv4 address */
	case PUD_NODEIDTYPE_IPV6: /* IPv6 address */
	default: /* unsupported */
	{
		void * addr = getOlsrMessageOriginator(ipVersion, olsrMessage);
		*nodeIdStr = inet_ntop(ipVersion, addr, nodeIdStrBuffer,
				nodeIdStrBufferSize);
	}
		break;
	}

	return;
}

/**
 Convert a given MAC address to the binary/wireformat representation of it.

 @param nodeIdBinary
 a pointer to the buffer in which to store the binary/wireformat representation
 @param mac
 a pointer to a buffer in which the MAC address is stored (in network byte-order)
 @return
 - true when ok
 - false on failure
 */
bool setupNodeIdBinaryMAC(nodeIdBinaryType * nodeIdBinary, unsigned char * mac) {
	memcpy(&nodeIdBinary->buffer.mac, mac, PUD_NODEIDTYPE_MAC_BYTES);
	nodeIdBinary->length = PUD_NODEIDTYPE_MAC_BYTES;
	nodeIdBinary->set = true;
	return true;
}

/**
 Convert a given unsigned long long to the binary/wireformat representation of it.

 @param nodeIdBinary
 a pointer to the buffer in which to store the binary/wireformat representation
 @param value
 the value to convert (in machine byte-order)
 @param bytes
 the number of bytes used by the value

 @return
 - true when ok
 - false on failure
 */
bool setupNodeIdBinaryLongLong(nodeIdBinaryType * nodeIdBinary,
		unsigned long long value, size_t bytes) {
	unsigned long long longValue = value;
	int i = bytes - 1;

	while (i >= 0) {
		((unsigned char *) &nodeIdBinary->buffer.longValue)[i] = longValue & 0xff;
		longValue >>= 8;
		i--;
	}

	assert(longValue == 0);

	nodeIdBinary->length = bytes;
	nodeIdBinary->set = true;
	return true;
}

/**
 Convert a given string to the binary/wireformat representation of it.

 @param nodeIdBinary
 a pointer to the buffer in which to store the binary/wireformat representation
 @param nodeId
 a pointer to the nodeId string
 @param nodeIdLength
 the length of the nodeId string
 @return
 - true when ok
 - false on failure
 */
bool setupNodeIdBinaryString(nodeIdBinaryType * nodeIdBinary, char * nodeId,
		size_t nodeIdLength) {
	/* including trailing \0 */
	memcpy(&nodeIdBinary->buffer.stringValue[0], &nodeId[0], nodeIdLength + 1);
	nodeIdBinary->length = nodeIdLength + 1;
	nodeIdBinary->set = true;
	return true;
}

/**
 Convert a given IP address to the binary/wireformat representation of it.

 @param nodeIdBinary
 a pointer to the buffer in which to store the binary/wireformat representation
 @param ip
 a pointer to a buffer in which the IP address is stored (in network byte-order)
 @param ipLength
 the number of bytes used by the IP address
 @return
 - true when ok
 - false on failure
 */
bool setupNodeIdBinaryIp(nodeIdBinaryType * nodeIdBinary, void * ip,
		size_t ipLength) {
	memcpy(&nodeIdBinary->buffer.ip, ip, ipLength);
	nodeIdBinary->length = ipLength;
	nodeIdBinary->set = true;
	return true;
}
