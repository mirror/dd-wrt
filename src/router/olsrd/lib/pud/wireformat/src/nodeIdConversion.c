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
 Get a nodeId hexadecimal number (in string representation), using a certain
 number of bytes, from the message of an OLSR message.

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
static char *getNodeIdHexNumberFromOlsr(unsigned char * buffer,
		unsigned int bufferSize, char *nodeIdBuffer, socklen_t nodeIdBufferSize) {
	unsigned long long val = 0;
	unsigned int i = 0;

	while (i < bufferSize) {
		val <<= 8;
		val += buffer[i];
		i++;
	}

	snprintf(nodeIdBuffer, nodeIdBufferSize, "%llx", val);
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
		if (nodeIdSize >= nodeIdStrBufferSize) {
		  nodeIdSize = nodeIdStrBufferSize - 1;
		}
		memcpy(nodeIdStrBuffer, nodeId, nodeIdSize);
		nodeIdStrBuffer[nodeIdSize] = '\0';
		*nodeIdStr = &nodeIdStrBuffer[0];
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

	case PUD_NODEIDTYPE_UUID: /* a UUID number */
	  *nodeIdStr = getNodeIdHexNumberFromOlsr(
	      &nodeId[0],
	      PUD_NODEIDTYPE_UUID_BYTES1,
	      &nodeIdStrBuffer[0],
	      PUD_NODEIDTYPE_UUID_CHARS1 + 1);
	  getNodeIdHexNumberFromOlsr(
	      &nodeId[PUD_NODEIDTYPE_UUID_BYTES1],
	      nodeIdSize - PUD_NODEIDTYPE_UUID_BYTES1,
	      &nodeIdStrBuffer[PUD_NODEIDTYPE_UUID_CHARS1],
	      nodeIdStrBufferSize - PUD_NODEIDTYPE_UUID_CHARS1);
		break;

	case PUD_NODEIDTYPE_MIP: /* a MIP OID number */
	  *nodeIdStr = getNodeIdNumberFromOlsr(
	      &nodeId[0],
	      PUD_NODEIDTYPE_MIP_BYTES1,
	      &nodeIdStrBuffer[0],
	      PUD_NODEIDTYPE_MIP_CHARS1 + 1);
	  getNodeIdNumberFromOlsr(
	      &nodeId[PUD_NODEIDTYPE_MIP_BYTES1],
	      nodeIdSize - PUD_NODEIDTYPE_MIP_BYTES1,
	      &nodeIdStrBuffer[PUD_NODEIDTYPE_MIP_CHARS1],
	      nodeIdStrBufferSize - PUD_NODEIDTYPE_MIP_CHARS1);
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
 Convert two given unsigned long longs to the binary/wireformat representation
 of them.

 @param nodeIdBinary
 a pointer to the buffer in which to store the binary/wireformat representation
 @param value1
 the first value to convert (in machine byte-order)
 @param dst1
 A pointer where to store the conversion of value1
 @param bytes1
 the number of bytes used by value1
 @param value2
 the second value to convert (in machine byte-order)
 @param dst2
 A pointer where to store the conversion of value2
 @param bytes2
 the number of bytes used by value2

 @return
 - true when ok
 - false on failure
 */
bool setupNodeIdBinaryDoubleLongLong(nodeIdBinaryType * nodeIdBinary,
    unsigned long long value1, unsigned char * dst1, size_t bytes1,
    unsigned long long value2, unsigned char * dst2, size_t bytes2) {
	unsigned long long longValue1 = value1;
	unsigned long long longValue2 = value2;
	int i1 = bytes1 - 1;
	int i2 = bytes2 - 1;

	while (i1 >= 0) {
		dst1[i1] = longValue1 & 0xff;
		longValue1 >>= 8;
		i1--;
	}
	assert(longValue1 == 0);

	while (i2 >= 0) {
		dst2[i2] = longValue2 & 0xff;
		longValue2 >>= 8;
		i2--;
	}
	assert(longValue2 == 0);

	nodeIdBinary->length = bytes1 + bytes2;
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
