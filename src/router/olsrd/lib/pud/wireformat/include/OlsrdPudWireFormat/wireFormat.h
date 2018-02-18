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

#ifndef _PUD_WIREFORMAT_H_
#define _PUD_WIREFORMAT_H_

#include "olsr_types.h"
#include "olsr_protocol.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <OlsrdPudWireFormat/compiler.h>

/*
 * Version
 */

/** The version of the wire format */
#define PUD_WIRE_FORMAT_VERSION		3

/*
 * Flags
 */

/**
 * Bitmask used in the GPS information present field to signal nodeId presence
 */
#define PUD_PRESENT_ID          0x80000000

/**
 * Bitmask used in the GPS information present field to signal that it's
 * originating from a gateway
 */
#define PUD_PRESENT_GATEWAY     0x40000000

/*
 * Time
 */

/** The number of bits for the time field */
#define PUD_TIME_BITS				17

/*
 * Latitude
 */

/** The number of bits for the latitude field */
#define PUD_LATITUDE_BITS			28

/** The maximum size of the string representation of the latitude
 * sign [0,90] [0,59] dot [0,59] [0,999] (including \0) */
#define PUD_TX_LATITUDE_DIGITS		(1 + 2 + 2 + 1 + 2 + 3 + 1)

/** The number of decimals of the latitude in the transmit sentence */
#define PUD_TX_LATITUDE_DECIMALS	"5"

/*
 * Longitude
 */

/** The number of bits for the longitude field */
#define PUD_LONGITUDE_BITS			27

/** The maximum size of the string representation of the longitude
 * sign [0,180] [0,59] dot [0,59] [0,999] (including \0) */
#define PUD_TX_LONGITUDE_DIGITS		(1 + 3 + 2 + 1 + 2 + 3 + 1)

/** The number of decimals of the longitude in the transmit sentence */
#define PUD_TX_LONGITUDE_DECIMALS	"5"

/*
 * Altitude
 */

/** The number of bits for the altitude field */
#define PUD_ALTITUDE_BITS			16

/** The minimum altitude */
#define PUD_ALTITUDE_MIN			(-400)

/** The maximum altitude */
#define PUD_ALTITUDE_MAX	(((1 << PUD_ALTITUDE_BITS) - 1) + PUD_ALTITUDE_MIN)

/** The maximum size of the string representation of the altitude (including \0) */
#define PUD_TX_ALTITUDE_DIGITS		(6 + 1)

/*
 * Speed
 */

/** The number of bits for the speed field */
#define PUD_SPEED_BITS				12

/** The maximum speed value */
#define PUD_SPEED_MAX				((1 << PUD_SPEED_BITS) - 1)

/** The maximum size of the string representation of the speed (including \0) */
#define PUD_TX_SPEED_DIGITS			(4 + 1)

/*
 * Track
 */

/** The number of bits for the track angle field */
#define PUD_TRACK_BITS				9

/** The maximum size of the string representation of the track angle (including \0) */
#define PUD_TX_TRACK_DIGITS			(3 + 1)

/*
 * HDOP
 */

/** The number of bits for the HDOP field */
#define PUD_HDOP_BITS				11

/** The HDOP resolution (in m) */
#define PUD_HDOP_RESOLUTION			(0.1)

/** The maximum HDOP value (in m) */
#define PUD_HDOP_MAX		(((1 << PUD_HDOP_BITS) - 1) * PUD_HDOP_RESOLUTION)

/** The maximum size of the string representation of the HDOP (including \0) */
#define PUD_TX_HDOP_DIGITS			(5 + 1)

/** The number of decimals of the HDOP in the transmit sentence */
#define PUD_TX_HDOP_DECIMALS		"3"

/*
 * Node ID Type
 */

/** nodeIdType legal values */
typedef enum _NodeIdType {
	/** the first id of the globally unique node type IDs */
	PUD_NODEIDTYPE_GLOBAL_FIRST = 0,

	/** MAC address, 48 bits, 6 bytes */
	PUD_NODEIDTYPE_MAC = 0,

	/** MSISDN number, 15 digits, 50 bits, 7 bytes */
	PUD_NODEIDTYPE_MSISDN = 1,

	/** TETRA number, 17 digits, 57 bits, 8 bytes */
	PUD_NODEIDTYPE_TETRA = 2,

	/** DNS name, variable length */
	PUD_NODEIDTYPE_DNS = 3,

	/** IPv4 address, 32 bits, 4 bytes */
	PUD_NODEIDTYPE_IPV4 = 4,

	/** UUID, 32 hexadecimal digits, 128 bits, 16 bytes */
	PUD_NODEIDTYPE_UUID = 5,

	/** IPv6 address, 128 bits, 16 bytes */
	PUD_NODEIDTYPE_IPV6 = 6,

	/** AIS MMSI number, 9 digits, 30 bits, 4 bytes */
	PUD_NODEIDTYPE_MMSI = 7,

	/** URN number, 24 bits, 3 bytes */
	PUD_NODEIDTYPE_URN = 8,

	/** MIP OID number, 67 bits, 9 bytes */
	PUD_NODEIDTYPE_MIP = 9,

	/** the last id of the globally unique node type IDs */
	PUD_NODEIDTYPE_GLOBAL_LAST = PUD_NODEIDTYPE_MIP,

	/** the first id of the locally unique node type IDs */
	PUD_NODEIDTYPE_LOCAL_FIRST = 192,

	/** Brandweer number, 7 digits, 24 bits, 3 bytes */
	PUD_NODEIDTYPE_192 = 192,

	/** Ambulance number, 6 digits, 20 bits, 3 bytes */
	PUD_NODEIDTYPE_193 = 193,

	/** Number in the range [1, 8191], 4 digits, 13 bits, 2 bytes */
	PUD_NODEIDTYPE_194 = 194,

	/** the last id of the locally unique node type IDs */
	PUD_NODEIDTYPE_LOCAL_LAST = PUD_NODEIDTYPE_194
} NodeIdType;

/** the number of nodeId bytes for PUD_NODEIDTYPE_MAC (IFHWADDRLEN) */
#define PUD_NODEIDTYPE_MAC_BYTES		6

/** the number of nodeId bytes for PUD_NODEIDTYPE_MSISDN */
#define PUD_NODEIDTYPE_MSISDN_BYTES		7
#define PUD_NODEIDTYPE_MSISDN_MIN		0LLU
#define PUD_NODEIDTYPE_MSISDN_MAX		999999999999999LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_TETRA */
#define PUD_NODEIDTYPE_TETRA_BYTES		8
#define PUD_NODEIDTYPE_TETRA_MIN		0LLU
#define PUD_NODEIDTYPE_TETRA_MAX		99999999999999999LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_UUID */
#define PUD_NODEIDTYPE_UUID_BYTES   16
#define PUD_NODEIDTYPE_UUID_BYTES1  8
#define PUD_NODEIDTYPE_UUID_BYTES2  (PUD_NODEIDTYPE_UUID_BYTES - PUD_NODEIDTYPE_UUID_BYTES1)
#define PUD_NODEIDTYPE_UUID_CHARS   32
#define PUD_NODEIDTYPE_UUID_CHARS1  16
#define PUD_NODEIDTYPE_UUID_CHARS2  (PUD_NODEIDTYPE_UUID_CHARS - PUD_NODEIDTYPE_UUID_CHARS1)
#define PUD_NODEIDTYPE_UUID_MIN1    0LLU
#define PUD_NODEIDTYPE_UUID_MAX1    0xFFFFFFFFFFFFFFFFLLU
#define PUD_NODEIDTYPE_UUID_MIN2    0LLU
#define PUD_NODEIDTYPE_UUID_MAX2    0xFFFFFFFFFFFFFFFFLLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_MMSI */
#define PUD_NODEIDTYPE_MMSI_BYTES		4
#define PUD_NODEIDTYPE_MMSI_MIN			0LLU
#define PUD_NODEIDTYPE_MMSI_MAX			999999999LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_URN */
#define PUD_NODEIDTYPE_URN_BYTES		3
#define PUD_NODEIDTYPE_URN_MIN			0LLU
#define PUD_NODEIDTYPE_URN_MAX			16777215LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_MIP */
#define PUD_NODEIDTYPE_MIP_BYTES    9
#define PUD_NODEIDTYPE_MIP_BYTES1   1
#define PUD_NODEIDTYPE_MIP_BYTES2   (PUD_NODEIDTYPE_MIP_BYTES - PUD_NODEIDTYPE_MIP_BYTES1)
#define PUD_NODEIDTYPE_MIP_CHARS    20
#define PUD_NODEIDTYPE_MIP_CHARS1   1
#define PUD_NODEIDTYPE_MIP_CHARS2   (PUD_NODEIDTYPE_MIP_CHARS - PUD_NODEIDTYPE_MIP_CHARS1)
#define PUD_NODEIDTYPE_MIP_MIN1     0LLU
#define PUD_NODEIDTYPE_MIP_MAX1     9LLU
#define PUD_NODEIDTYPE_MIP_MIN2     0LLU
#define PUD_NODEIDTYPE_MIP_MAX2     9999999999999999999LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_192 */
#define PUD_NODEIDTYPE_192_BYTES		3
#define PUD_NODEIDTYPE_192_MIN			0LLU
#define PUD_NODEIDTYPE_192_MAX			9999999LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_193 */
#define PUD_NODEIDTYPE_193_BYTES		3
#define PUD_NODEIDTYPE_193_MIN			0LLU
#define PUD_NODEIDTYPE_193_MAX			999999LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_194 */
#define PUD_NODEIDTYPE_194_BYTES		2
#define PUD_NODEIDTYPE_194_MIN			1LLU
#define PUD_NODEIDTYPE_194_MAX			8191LLU

/** the number of nodeId bytes for PUD_NODEIDTYPE_IPV4 (sizeof(struct in_addr)) */
#define PUD_NODEIDTYPE_IPV4_BYTES		4

/** the number of nodeId bytes for PUD_NODEIDTYPE_IPV6 (sizeof(struct in6_addr)) */
#define PUD_NODEIDTYPE_IPV6_BYTES		16

/** The maximum size of the string representation of the nodeIdType (including \0) */
#define PUD_TX_NODEIDTYPE_DIGITS		(3 + 1)

/*
 * Node ID
 */

/** The maximum size of the string representation of the nodeId (including \0) */
#define PUD_TX_NODEID_BUFFERSIZE		1024

/**
 The type that is used to store the nodeId as a binary representation along
 with its length and setup status
 */
typedef struct _nodeIdBinaryType {
		bool set;
		size_t length;
		union _buffer {
				unsigned char mac[PUD_NODEIDTYPE_MAC_BYTES];
				union olsr_ip_addr ip;
				unsigned long long longValue;
				unsigned char stringValue[PUD_TX_NODEID_BUFFERSIZE];
				unsigned char uuid[PUD_NODEIDTYPE_UUID_BYTES];
				unsigned char mip[PUD_NODEIDTYPE_MIP_BYTES];
		} buffer;
} nodeIdBinaryType;

/*
 * Wire Format Structures
 */

/** Sub-format GPS information, 120 bits = 15 bytes */
typedef struct _GpsInfo {
	uint32_t time :PUD_TIME_BITS; /**< the number of seconds since midnight, ALWAYS present */
	uint32_t lat :PUD_LATITUDE_BITS; /**< latitude */
	uint32_t lon :PUD_LONGITUDE_BITS; /**< longitude */
	uint32_t alt :PUD_ALTITUDE_BITS; /**< altitude */
	uint32_t speed :PUD_SPEED_BITS; /**< speed */
	uint32_t track :PUD_TRACK_BITS; /**< track angle */
	uint32_t hdop :PUD_HDOP_BITS; /**< HDOP */
}__attribute__((__packed__)) GpsInfo;

/** Sub-format Node information, 8 + variable bits = 1 + variable bytes */
typedef struct _NodeInfo {
	uint8_t nodeIdType; /**< the nodeIdType */
	unsigned char nodeId; /**< placeholder for variable length nodeId string */
}__attribute__((__packed__)) NodeInfo;

/** Complete format, 8+8+32+120+(8+variable) bits =  21+(1+variable) bytes*/
typedef struct _PudOlsrPositionUpdate {
	uint8_t version; /**< the version of the sentence */
	uint8_t validityTime; /**< the validity time of the sentence */
	uint32_t present; /**< mask signaling the contents of gpsInfo */
	GpsInfo gpsInfo; /**< the GPS information (MANDATORY) */
	NodeInfo nodeInfo; /**< placeholder for node information (OPTIONAL) */
}__attribute__((__packed__)) PudOlsrPositionUpdate;

/** The size of the wire format, minus the size of the node information */
#define PUD_OLSRWIREFORMATSIZE (sizeof(PudOlsrPositionUpdate) - sizeof(NodeInfo))

/*
 * Uplink
 */

/** the types of the uplink messages */
typedef enum _UplinkMessageType {
	POSITION = 0,
	CLUSTERLEADER = 1
} UplinkMessageType;

/** cluster leader message, 10 bytes (IPv4), 34 bytes (IPv6) */
typedef struct _UplinkClusterLeader {
	uint8_t version; /**< the version of the message */
	uint8_t validityTime; /**< the validity time of the sentence */
	union _leader {
		struct _v4 {
			struct in_addr originator;
			struct in_addr clusterLeader;
		} v4;
		struct _v6 {
			struct in6_addr originator;
			struct in6_addr clusterLeader;
		} v6;
	} leader;
}__attribute__((__packed__)) UplinkClusterLeader;

/** TLV header for uplink messages, 4 bytes */
typedef struct _UplinkHeader {
	uint8_t type; /**< stores a UplinkMessageType */
	uint16_t length; /**< the length of the payload in txBuffer */
	uint8_t ipv6 :1; /**< clear when IPv4, set when IPv6 */
	uint8_t pad :7; /**< padding to align to 4 bytes */
}__attribute__((__packed__)) UplinkHeader;

/** uplink message */
typedef struct _UplinkMessage {
	UplinkHeader header; /**< the uplink TLV header */
	union _msg {
		/** an olsr message (position update) */
		union olsr_message olsrMessage;

		/** a cluster leader message */
		UplinkClusterLeader clusterLeader;
	} msg;
}__attribute__((__packed__)) UplinkMessage;

/* ************************************************************************
 * FUNCTIONS
 * ************************************************************************ */

/*
 * NodeIdType
 */
bool isValidNodeIdType(unsigned long long nodeIdType);

/*
 * Validity Time
 */
unsigned long getValidityTime(uint8_t * validityTimeField);
void setValidityTime(uint8_t * validityTimeField, unsigned long long validityTime);

/*
 * UplinkHeader
 */
uint8_t getUplinkMessageType(UplinkHeader * uplinkHeader);
void setUplinkMessageType(UplinkHeader * uplinkHeader, uint8_t type);
uint16_t getUplinkMessageLength(UplinkHeader * uplinkHeader);
void setUplinkMessageLength(UplinkHeader * uplinkHeader, uint16_t length);
bool getUplinkMessageIPv6(UplinkHeader * uplinkHeader);
void setUplinkMessageIPv6(UplinkHeader * uplinkHeader, bool ipv6);
void setUplinkMessagePadding(UplinkHeader * uplinkHeader, uint8_t pad);

/*
 * OLSR header
 */
unsigned short getOlsrMessageSize(int ipVersion, union olsr_message * olsrMessage);
union olsr_ip_addr * getOlsrMessageOriginator(int ipVersion, union olsr_message * olsrMessage);
PudOlsrPositionUpdate * getOlsrMessagePayload(int ipVersion, union olsr_message * olsrMessage);

/*
 * PudOlsrPositionUpdate
 */
uint8_t getPositionUpdateVersion(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateVersion(PudOlsrPositionUpdate * olsrGpsMessage, uint8_t version);
uint32_t getPositionUpdatePresent(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdatePresent(PudOlsrPositionUpdate * olsrGpsMessage, uint32_t present);

/*
 * GpsInfo
 */
void getPositionUpdateTime(PudOlsrPositionUpdate * olsrGpsMessage, time_t baseDate, struct tm *nowStruct);
void setPositionUpdateTime(PudOlsrPositionUpdate * olsrGpsMessage, int hour, int min, int sec);
double getPositionUpdateLatitude(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateLatitude(PudOlsrPositionUpdate * olsrGpsMessage, double latitude);
double getPositionUpdateLongitude(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateLongitude(PudOlsrPositionUpdate * olsrGpsMessage, double longitude);
long getPositionUpdateAltitude(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateAltitude(PudOlsrPositionUpdate * olsrGpsMessage, double altitude);
unsigned long getPositionUpdateSpeed(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateSpeed(PudOlsrPositionUpdate * olsrGpsMessage, double speed);
unsigned long getPositionUpdateTrack(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateTrack(PudOlsrPositionUpdate * olsrGpsMessage, double track);
double getPositionUpdateHdop(PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateHdop(PudOlsrPositionUpdate * olsrGpsMessage, double hdop);

/*
 * NodeInfo
 */
NodeIdType getPositionUpdateNodeIdType(int ipVersion, PudOlsrPositionUpdate * olsrGpsMessage);
void setPositionUpdateNodeIdType(PudOlsrPositionUpdate * olsrGpsMessage, NodeIdType nodeIdType);

void getPositionUpdateNodeId(int ipVersion, union olsr_message * olsrMessage, unsigned char ** nodeId,
		unsigned int * nodeIdSize);
void setPositionUpdateNodeId(PudOlsrPositionUpdate * olsrGpsMessage, unsigned char * nodeId, unsigned int nodeIdSize,
		bool padWithNullByte);

size_t setPositionUpdateNodeInfo(int ipVersion, PudOlsrPositionUpdate * olsrGpsMessage, unsigned int olsrMessageSize,
		NodeIdType nodeIdType, unsigned char * nodeId, size_t nodeIdLength);

/*
 * UplinkClusterLeader
 */
uint8_t getClusterLeaderVersion(UplinkClusterLeader * clusterLeaderMessage);
void setClusterLeaderVersion(UplinkClusterLeader * clusterLeaderMessage, uint8_t version);
union olsr_ip_addr * getClusterLeaderOriginator(int ipVersion, UplinkClusterLeader * clusterLeaderMessage);
union olsr_ip_addr * getClusterLeaderClusterLeader(int ipVersion, UplinkClusterLeader * clusterLeaderMessage);

#endif /* _PUD_WIREFORMAT_H_ */
