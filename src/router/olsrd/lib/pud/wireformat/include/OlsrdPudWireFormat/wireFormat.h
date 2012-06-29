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
#define PUD_WIRE_FORMAT_VERSION		2

/*
 * Flags
 */

/** Flags that the GPS information contains the nodeId */
#define PUD_FLAGS_ID				0x80

/** Flags that the GPS information is originating from a gateway */
#define PUD_FLAGS_GATEWAY			0x40

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
 * sign [0,90] [0,59] dot [0,59] [0,999] */
#define PUD_TX_LATITUDE_DIGITS		(1 + 2 + 2 + 1 + 2 + 3)

/** The number of decimals of the latitude in the transmit sentence */
#define PUD_TX_LATITUDE_DECIMALS	"5"

/*
 * Longitude
 */

/** The number of bits for the longitude field */
#define PUD_LONGITUDE_BITS			27

/** The maximum size of the string representation of the longitude
 * sign [0,180] [0,59] dot [0,59] [0,999] */
#define PUD_TX_LONGITUDE_DIGITS		(1 + 3 + 2 + 1 + 2 + 3)

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

/** The maximum size of the string representation of the altitude */
#define PUD_TX_ALTITUDE_DIGITS		6

/*
 * Speed
 */

/** The number of bits for the speed field */
#define PUD_SPEED_BITS				12

/** The maximum speed value */
#define PUD_SPEED_MAX				((1 << PUD_SPEED_BITS) - 1)

/** The maximum size of the string representation of the speed */
#define PUD_TX_SPEED_DIGITS			4

/*
 * Track
 */

/** The number of bits for the track angle field */
#define PUD_TRACK_BITS				9

/** The maximum size of the string representation of the track angle */
#define PUD_TX_TRACK_DIGITS			3

/*
 * HDOP
 */

/** The number of bits for the HDOP field */
#define PUD_HDOP_BITS				11

/** The HDOP resolution (in m) */
#define PUD_HDOP_RESOLUTION			(0.1)

/** The maximum HDOP value (in m) */
#define PUD_HDOP_MAX		(((1 << PUD_HDOP_BITS) - 1) * PUD_HDOP_RESOLUTION)

/** The maximum size of the string representation of the HDOP */
#define PUD_TX_HDOP_DIGITS			5

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

	/** gap 1 */
	PUD_NODEIDTYPE_GAP1 = 5,

	/** IPv6 address, 128 bits, 16 bytes */
	PUD_NODEIDTYPE_IPV6 = 6,

	/** AIS MMSI number, 9 digits, 30 bits, 4 bytes */
	PUD_NODEIDTYPE_MMSI = 7,

	/** URN number, 24 bits, 3 bytes */
	PUD_NODEIDTYPE_URN = 8,

	/** the last id of the globally unique node type IDs */
	PUD_NODEIDTYPE_GLOBAL_LAST = PUD_NODEIDTYPE_URN,

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
#define PUD_NODEIDTYPE_MSISDN_MIN		0LL
#define PUD_NODEIDTYPE_MSISDN_MAX		999999999999999LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_TETRA */
#define PUD_NODEIDTYPE_TETRA_BYTES		8
#define PUD_NODEIDTYPE_TETRA_MIN		0LL
#define PUD_NODEIDTYPE_TETRA_MAX		99999999999999999LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_MMSI */
#define PUD_NODEIDTYPE_MMSI_BYTES		4
#define PUD_NODEIDTYPE_MMSI_MIN			0LL
#define PUD_NODEIDTYPE_MMSI_MAX			999999999LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_URN */
#define PUD_NODEIDTYPE_URN_BYTES		3
#define PUD_NODEIDTYPE_URN_MIN			0LL
#define PUD_NODEIDTYPE_URN_MAX			16777215LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_192 */
#define PUD_NODEIDTYPE_192_BYTES		3
#define PUD_NODEIDTYPE_192_MIN			0LL
#define PUD_NODEIDTYPE_192_MAX			9999999LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_193 */
#define PUD_NODEIDTYPE_193_BYTES		3
#define PUD_NODEIDTYPE_193_MIN			0LL
#define PUD_NODEIDTYPE_193_MAX			999999LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_194 */
#define PUD_NODEIDTYPE_194_BYTES		2
#define PUD_NODEIDTYPE_194_MIN			1LL
#define PUD_NODEIDTYPE_194_MAX			8191LL

/** the number of nodeId bytes for PUD_NODEIDTYPE_IPV4 (sizeof(struct in_addr)) */
#define PUD_NODEIDTYPE_IPV4_BYTES		4

/** the number of nodeId bytes for PUD_NODEIDTYPE_IPV6 (sizeof(struct in6_addr)) */
#define PUD_NODEIDTYPE_IPV6_BYTES		16

/** The maximum size of the string representation of the nodeIdType */
#define PUD_TX_NODEIDTYPE_DIGITS		3

/*
 * Node ID
 */

/** The maximum size of the string representation of the nodeId */
#define PUD_TX_NODEID_BUFFERSIZE		1023

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
				unsigned char stringValue[PUD_TX_NODEID_BUFFERSIZE + 1];
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

/** Complete format, 8+8+8+120+(8+variable) bits =  18+(1+variable) bytes*/
typedef struct _PudOlsrPositionUpdate {
	uint8_t version; /**< the version of the sentence */
	uint8_t validityTime; /**< the validity time of the sentence */
	uint8_t smask; /**< mask signaling the contents of the sentence */
	uint8_t flags; /**< mask signaling extra contents of the sentence */
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

static inline bool isValidNodeIdType(unsigned long long nodeIdType) {
	return
	(
		(
			(/* (nodeIdType >= PUD_NODEIDTYPE_GLOBAL_FIRST) && */ (nodeIdType <= PUD_NODEIDTYPE_GLOBAL_LAST)) ||
			(   (nodeIdType >= PUD_NODEIDTYPE_LOCAL_FIRST ) &&    (nodeIdType <= PUD_NODEIDTYPE_LOCAL_LAST ))
		)
		&&
		(
			(nodeIdType != PUD_NODEIDTYPE_GAP1)
		)
	);
}

/*
 * Validity Time
 */

/** Determine the validity time in seconds from the OLSR wire format value */
#define PUD_VALIDITY_TIME_FROM_OLSR(msn, lsn) ((((lsn) + 16) * (1 << (msn))) - 16)

/**
 Get the validity time from a message

 @param validityTimeField
 A pointer to the validity time field

 @return
 The validity time in seconds
 */
static inline unsigned long getValidityTime(uint8_t * validityTimeField) {
	return PUD_VALIDITY_TIME_FROM_OLSR(*validityTimeField >> 4, *validityTimeField % 16);
}

void setValidityTime(uint8_t * validityTimeField,
		unsigned long long validityTime);

/*
 * UplinkHeader
 */

/**
 Get the type of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @return
 The type of the uplink message
 */
static inline uint8_t getUplinkMessageType(UplinkHeader * uplinkHeader) {
	return uplinkHeader->type;
}

/**
 Set the type of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @param type
 The type of the uplink message
 */
static inline void setUplinkMessageType(UplinkHeader * uplinkHeader,
		uint8_t type) {
	uplinkHeader->type = type;
}

/**
 Get the length of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @return
 The length of the uplink message
 */
static inline uint16_t getUplinkMessageLength(UplinkHeader * uplinkHeader) {
	return ntohs(uplinkHeader->length);
}

/**
 Set the length of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @param length
 The length of the uplink message
 */
static inline void setUplinkMessageLength(UplinkHeader * uplinkHeader,
		uint16_t length) {
	uplinkHeader->length = ntohs(length);
}

/**
 Get the IPv6 status of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @return
 true when the uplink message is sent from an olsrd stack in IPv6 mode, false
 otherwise
 */
static inline bool getUplinkMessageIPv6(UplinkHeader * uplinkHeader) {
	return (uplinkHeader->ipv6 == 1);
}

/**
 Set the IPv6 status of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @param ipv6
 The IPv6 status of the uplink message (true when the uplink message is sent
 from olsrd stack in IPv6 mode, false otherwise)
 */
static inline void setUplinkMessageIPv6(UplinkHeader * uplinkHeader,
		bool ipv6) {
	uplinkHeader->ipv6 = ipv6 ? 1 : 0;
}

/**
 Set the padding of the uplink message header

 @param uplinkHeader
 A pointer to the uplink message
 @param pad
 The padding of the uplink message header
 */
static inline void setUplinkMessagePadding(UplinkHeader * uplinkHeader,
		uint8_t pad) {
	uplinkHeader->pad = pad;
}

/*
 * OLSR header
 */


/**
 Determine the size of an OLSR message

 @param ipVersion
 The IP version
 @param olsrMessage
 A pointer to the OLSR message
 @return
 The size of the OLSR message
 */
static inline unsigned short getOlsrMessageSize(int ipVersion,
		union olsr_message * olsrMessage) {
	if (ipVersion == AF_INET) {
		return ntohs(olsrMessage->v4.olsr_msgsize);
	}

	return ntohs(olsrMessage->v6.olsr_msgsize);
}

/**
 Get the originator of an OLSR message

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param olsrMessage
 A pointer to the OLSR message
 @return
 A pointer to the originator address
 */
static inline union olsr_ip_addr * getOlsrMessageOriginator(int ipVersion,
		union olsr_message * olsrMessage) {
	if (ipVersion == AF_INET) {
		return (union olsr_ip_addr *) &olsrMessage->v4.originator;
	}

	return (union olsr_ip_addr *) &olsrMessage->v6.originator;
}

/**
 Get the position update message in an OLSR message

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param olsrMessage
 A pointer to the OLSR message
 @return
 A pointer to the position update message
 */
static inline PudOlsrPositionUpdate * getOlsrMessagePayload(int ipVersion,
		union olsr_message * olsrMessage) {
	if (ipVersion == AF_INET) {
		return (PudOlsrPositionUpdate *) &olsrMessage->v4.message;
	}

	return (PudOlsrPositionUpdate *) &olsrMessage->v6.message;
}

/*
 * PudOlsrPositionUpdate
 */

/**
 Get the version of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @return
 The version of the position update message
 */
static inline uint8_t getPositionUpdateVersion(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return olsrGpsMessage->version;
}

/**
 Set the version of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param version
 The version of the position update message
 */
static inline void setPositionUpdateVersion(
		PudOlsrPositionUpdate * olsrGpsMessage, uint8_t version) {
	olsrGpsMessage->version = version;
}

/**
 Get the smask of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @return
 The smask of the position update message
 */
static inline uint8_t getPositionUpdateSmask(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return olsrGpsMessage->smask;
}

/**
 Set the smask of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param smask
 The smask of the position update message
 */
static inline void setPositionUpdateSmask(
		PudOlsrPositionUpdate * olsrGpsMessage, uint8_t smask) {
	olsrGpsMessage->smask = smask;
}

/**
 Get the flags of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @return
 The flags of the position update message
 */
static inline uint8_t getPositionUpdateFlags(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return olsrGpsMessage->flags;
}

/**
 Set the flags of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param flags
 The flags of the position update message
 */
static inline void setPositionUpdateFlags(
		PudOlsrPositionUpdate * olsrGpsMessage, uint8_t flags) {
	olsrGpsMessage->flags = flags;
}

/*
 * GpsInfo
 */

void getPositionUpdateTime(PudOlsrPositionUpdate * olsrGpsMessage,
		time_t baseDate, struct tm *nowStruct);

/**
 Set the time of the position update message (the number of seconds after
 midnight)

 @param olsrGpsMessage
 A pointer to the position update message
 @param hour
 The hours
 @param min
 The minutes
 @param sec
 The seconds
 */
static inline void setPositionUpdateTime(PudOlsrPositionUpdate * olsrGpsMessage,
		int hour, int min, int sec) {
	olsrGpsMessage->gpsInfo.time = ((hour * 60 * 60) + (min * 60) + sec);
}

/**
 Get the latitude of the position update message

 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The latitude converted to degrees: [-90, 90>
 */
static inline double getPositionUpdateLatitude(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	uint32_t olsrLat = olsrGpsMessage->gpsInfo.lat;
	double lat = (double) olsrLat;

	/* lat is in [0, 2^LATITUDE_BITS> */

	/* take half of the rounding error */
	lat += 0.5;

	lat /= (double) (1 << PUD_LATITUDE_BITS);
	/* lat is now in [0, 1> */

	lat -= 0.5;
	/* lat is now in [-0.5, 0.5> */

	lat *= 180.0;
	/* lat is now in [-90, 90> */

	return lat;
}

/**
 Set the latitude of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param latitude
 The latitude in degrees: [-90, 90]
 */
static inline void setPositionUpdateLatitude(
		PudOlsrPositionUpdate * olsrGpsMessage, double latitude) {
	double lat = latitude;

	/* lat is in [-90, 90] */
	assert(lat >= -90.0);
	assert(lat <= 90.0);

	lat /= 180.0;
	/* lat is now in [-0.5, 0.5] */

	lat += 0.5;
	/* lat is now in [0, 1] */

	lat *= (double) (1 << PUD_LATITUDE_BITS);
	/* lat is now in [0, LATITUDE_BITS] */

	/* clip max */
	if (unlikely(lat > (double)((1 << PUD_LATITUDE_BITS) - 1))) {
		lat = (double) ((1 << PUD_LATITUDE_BITS) - 1);
	}
	/* lat is now in [0, 2^LATITUDE_BITS> */

	olsrGpsMessage->gpsInfo.lat = lrint(lat);
}

/**
 Get the longitude of the position update message

 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The longitude converted to degrees: [-180, 180>
 */
static inline double getPositionUpdateLongitude(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	uint32_t olsrLon = olsrGpsMessage->gpsInfo.lon;
	double lon = (double) olsrLon;

	/* lon is in [0, 2^LONGITUDE_BITS> */

	/* take half of the rounding error */
	lon += 0.5;

	lon /= (1 << PUD_LONGITUDE_BITS);
	/* lon is now in [0, 1> */

	lon -= 0.5;
	/* lon is now in [-0.5, 0.5> */

	lon *= 360.0;
	/* lon is now in [-180, 180> */

	return lon;
}

/**
 Set the longitude of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param longitude
 The longitude in degrees: [-90, 90]
 */
static inline void setPositionUpdateLongitude(
		PudOlsrPositionUpdate * olsrGpsMessage, double longitude) {
	double lon = longitude;

	/* lon is in [-180, 180] */
	assert(lon >= -180.0);
	assert(lon <= 180.0);

	lon /= 360.0;
	/* lon is now in [-0.5, 0.5] */

	lon += 0.5;
	/* lon is now in [0, 1] */

	lon *= (double) (1 << PUD_LONGITUDE_BITS);
	/* lon is now in [0, LONGITUDE_BITS] */

	/* clip max */
	if (unlikely(lon > (double)((1 << PUD_LATITUDE_BITS) - 1))) {
		lon = (double) ((1 << PUD_LATITUDE_BITS) - 1);
	}

	/* lon is now in [0, 2^LONGITUDE_BITS> */

	olsrGpsMessage->gpsInfo.lon = lrint(lon);
}

/**
 Get the altitude of the position update message

 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The altitude in meters
 */
static inline long getPositionUpdateAltitude(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return ((long) olsrGpsMessage->gpsInfo.alt + PUD_ALTITUDE_MIN);
}

/**
 Set the altitude of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param altitude
 The altitude in meters
 */
static inline void setPositionUpdateAltitude(
		PudOlsrPositionUpdate * olsrGpsMessage, double altitude) {
	double alt = altitude;

	if (unlikely(alt > PUD_ALTITUDE_MAX)) {
		alt = PUD_ALTITUDE_MAX;
	} else if (unlikely(alt < PUD_ALTITUDE_MIN)) {
		alt = PUD_ALTITUDE_MIN;
	}

	alt -= PUD_ALTITUDE_MIN;

	olsrGpsMessage->gpsInfo.alt = lrint(alt);
}

/**
 Get the speed of the position update message

 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The speed in kph
 */
static inline unsigned long getPositionUpdateSpeed(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return olsrGpsMessage->gpsInfo.speed;
}

/**
 Set the speed of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param speed
 The speed in kph
 */
static inline void setPositionUpdateSpeed(
		PudOlsrPositionUpdate * olsrGpsMessage, double speed) {
	double spd = speed;

	if (unlikely(speed < 0)) {
		spd = 0;
	} else if (unlikely(speed > PUD_SPEED_MAX)) {
		spd = PUD_SPEED_MAX;
	}

	olsrGpsMessage->gpsInfo.speed = lrint(spd);
}

/**
 Get the track angle of the position update message

 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The track angle in degrees
 */
static inline unsigned long getPositionUpdateTrack(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return olsrGpsMessage->gpsInfo.track;
}

/**
 Set the track angle of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param track
 The track angle in degrees
 */
static inline void setPositionUpdateTrack(
		PudOlsrPositionUpdate * olsrGpsMessage, double track) {
	olsrGpsMessage->gpsInfo.track = lrint(track);
}

/**
 Get the HDOP of the position update message

 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The HDOP
 */
static inline double getPositionUpdateHdop(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return (olsrGpsMessage->gpsInfo.hdop * PUD_HDOP_RESOLUTION);
}

/**
 Set the HDOP of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param hdop
 The HDOP
 */
static inline void setPositionUpdateHdop(PudOlsrPositionUpdate * olsrGpsMessage,
		double hdop) {
	double hdopInternal = hdop;

	if (unlikely(hdopInternal > PUD_HDOP_MAX)) {
		hdopInternal = PUD_HDOP_MAX;
	}

	olsrGpsMessage->gpsInfo.hdop = lrint(hdopInternal / PUD_HDOP_RESOLUTION);
}

/*
 * NodeInfo
 */

/**
 Get the nodeIdType of the position update message

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param olsrGpsMessage
 A pointer to the position update message

 @return
 The nodeIdType
 */
static inline NodeIdType getPositionUpdateNodeIdType(int ipVersion,
		PudOlsrPositionUpdate * olsrGpsMessage) {
	if (getPositionUpdateFlags(olsrGpsMessage) & PUD_FLAGS_ID) {
		return olsrGpsMessage->nodeInfo.nodeIdType;
	}

	return ((ipVersion == AF_INET) ? PUD_NODEIDTYPE_IPV4 : PUD_NODEIDTYPE_IPV6);
}

/**
 Set the nodeIdType of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param nodeIdType
 The nodeIdType
 */
static inline void setPositionUpdateNodeIdType(
		PudOlsrPositionUpdate * olsrGpsMessage, NodeIdType nodeIdType) {
	olsrGpsMessage->nodeInfo.nodeIdType = nodeIdType;
}

void getPositionUpdateNodeId(int ipVersion, union olsr_message * olsrMessage,
		unsigned char ** nodeId, unsigned int * nodeIdSize);

/**
 Set the nodeId of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param nodeId
 The nodeId
 @param nodeIdSize
 The number of bytes in nodeId
 @param padWithNullByte
 When true then an extra '\0' byte will be added at the end
 */
static inline void setPositionUpdateNodeId(
		PudOlsrPositionUpdate * olsrGpsMessage, unsigned char * nodeId,
		unsigned int nodeIdSize, bool padWithNullByte) {
	memcpy(&olsrGpsMessage->nodeInfo.nodeId, nodeId, nodeIdSize);
	if (unlikely(padWithNullByte)) {
		(&olsrGpsMessage->nodeInfo.nodeId)[nodeIdSize] = '\0';
	}
}

size_t setPositionUpdateNodeInfo(int ipVersion,
		PudOlsrPositionUpdate * olsrGpsMessage, unsigned int olsrMessageSize,
		NodeIdType nodeIdType, unsigned char * nodeId, size_t nodeIdLength);

/*
 * UplinkClusterLeader
 */

/**
 Get the version of the cluster leader message

 @param clusterLeaderMessage
 A pointer to the cluster leader message
 @return
 The version of the cluster leader message
 */
static inline uint8_t getClusterLeaderVersion(
		UplinkClusterLeader * clusterLeaderMessage) {
	return clusterLeaderMessage->version;
}

/**
 Set the version of the cluster leader message

 @param clusterLeaderMessage
 A pointer to the cluster leader message
 @param version
 The version of the cluster leader message
 */
static inline void setClusterLeaderVersion(
		UplinkClusterLeader * clusterLeaderMessage, uint8_t version) {
	clusterLeaderMessage->version = version;
}

/**
 Get the originator of a cluster leader message

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param clusterLeaderMessage
 A pointer to the cluster leader message
 @return
 A pointer to the originator address
 */
static inline union olsr_ip_addr * getClusterLeaderOriginator(int ipVersion,
		UplinkClusterLeader * clusterLeaderMessage) {
	if (ipVersion == AF_INET) {
		return (union olsr_ip_addr *) &clusterLeaderMessage->leader.v4.originator;
	}

	return (union olsr_ip_addr *) &clusterLeaderMessage->leader.v6.originator;
}

/**
 Get the cluster leader of a cluster leader message

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param clusterLeaderMessage
 A pointer to the cluster leader message
 @return
 A pointer to the clust leader address
 */
static inline union olsr_ip_addr * getClusterLeaderClusterLeader(int ipVersion,
		UplinkClusterLeader * clusterLeaderMessage) {
	if (ipVersion == AF_INET) {
		return (union olsr_ip_addr *) &clusterLeaderMessage->leader.v4.clusterLeader;
	}

	return (union olsr_ip_addr *) &clusterLeaderMessage->leader.v6.clusterLeader;
}

#endif /* _PUD_WIREFORMAT_H_ */
