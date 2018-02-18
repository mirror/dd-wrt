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

#include <OlsrdPudWireFormat/wireFormat.h>

#include <time.h>

/*
 * NodeIdType
 */

bool isValidNodeIdType(unsigned long long nodeIdType) {
	return
	(
		(
			(/* (nodeIdType >= PUD_NODEIDTYPE_GLOBAL_FIRST) && */ (nodeIdType <= PUD_NODEIDTYPE_GLOBAL_LAST)) ||
			(   (nodeIdType >= PUD_NODEIDTYPE_LOCAL_FIRST ) &&    (nodeIdType <= PUD_NODEIDTYPE_LOCAL_LAST ))
		)
	);
}


/*
 * Validity Time
 */

/** Determine the validity time in seconds from the OLSR wire format value */
#define PUD_VALIDITY_TIME_FROM_OLSR(msn, lsn) ((((lsn) + 16) * (1u << (msn))) - 16)

/**
 Get the validity time from a message

 @param validityTimeField
 A pointer to the validity time field

 @return
 The validity time in seconds
 */
unsigned long getValidityTime(uint8_t * validityTimeField) {
	return PUD_VALIDITY_TIME_FROM_OLSR(*validityTimeField >> 4, *validityTimeField % 16);
}

/**
 Set the validity time of the position update message

 @param validityTimeField
 A pointer to the validity time field
 @param validityTime
 The validity time in seconds
 */
void setValidityTime(uint8_t * validityTimeField, unsigned long long validityTime) {
	unsigned int msn = 1;
	unsigned long long lsn = 0;
	unsigned long long upperBound;

	upperBound = PUD_VALIDITY_TIME_FROM_OLSR(msn, 0);
	while ((msn < 16) && (validityTime >= upperBound)) {
		msn++;
		upperBound = PUD_VALIDITY_TIME_FROM_OLSR(msn, 0);
	}
	msn--;

	if (unlikely(validityTime >= upperBound)) {
		lsn = 15;
	} else {
		unsigned long lowerBound = PUD_VALIDITY_TIME_FROM_OLSR(msn, 0);
		unsigned long resolution = (1u << msn);
		lsn = ((validityTime - lowerBound + (resolution >> 1)) / resolution);
	}

	assert(msn <= 15);
	assert(lsn <= 15);

	*validityTimeField = ((msn << 4) | lsn);
}

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
uint8_t getUplinkMessageType(UplinkHeader * uplinkHeader) {
	return uplinkHeader->type;
}

/**
 Set the type of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @param type
 The type of the uplink message
 */
void setUplinkMessageType(UplinkHeader * uplinkHeader,
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
uint16_t getUplinkMessageLength(UplinkHeader * uplinkHeader) {
	return ntohs(uplinkHeader->length);
}

/**
 Set the length of the uplink message

 @param uplinkHeader
 A pointer to the uplink message
 @param length
 The length of the uplink message
 */
void setUplinkMessageLength(UplinkHeader * uplinkHeader,
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
bool getUplinkMessageIPv6(UplinkHeader * uplinkHeader) {
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
void setUplinkMessageIPv6(UplinkHeader * uplinkHeader,
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
void setUplinkMessagePadding(UplinkHeader * uplinkHeader,
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
unsigned short getOlsrMessageSize(int ipVersion,
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
union olsr_ip_addr * getOlsrMessageOriginator(int ipVersion,
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
PudOlsrPositionUpdate * getOlsrMessagePayload(int ipVersion,
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
uint8_t getPositionUpdateVersion(
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
void setPositionUpdateVersion(
		PudOlsrPositionUpdate * olsrGpsMessage, uint8_t version) {
	olsrGpsMessage->version = version;
}

/**
 Get the presence field of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @return
 The presence field of the position update message
 */
uint32_t getPositionUpdatePresent(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	return ntohl(olsrGpsMessage->present);
}

/**
 Set the presence field of the position update message

 @param olsrGpsMessage
 A pointer to the position update message
 @param present
 The presence field of the position update message
 */
void setPositionUpdatePresent(
		PudOlsrPositionUpdate * olsrGpsMessage, uint32_t present) {
	olsrGpsMessage->present = htonl(present);
}

/*
 * GpsInfo
 */

/**
 Convert the time of an OLSR message (the number of seconds after midnight) to
 a time structure, based on midnight of the current day.

 @param olsrGpsMessage
 A pointer to the position update message
 @param baseDate
 The base date from which to determine the time (number of seconds since Epoch,
 UTC)
 @param nowStruct
 A pointer to the time structure into which to put the converted time
 */
void getPositionUpdateTime(PudOlsrPositionUpdate * olsrGpsMessage,
		time_t baseDate, struct tm *nowStruct) {
	uint32_t olsrTime = olsrGpsMessage->gpsInfo.time;
	unsigned int secNow;

	time_t now = baseDate;
	gmtime_r(&now, nowStruct);

	secNow = ((nowStruct->tm_hour * 60 * 60) + (nowStruct->tm_min * 60)
			+ nowStruct->tm_sec);

	if (secNow <= (12 * 60 * 60)) {
		/* we are now in the first 12h of the day */
		if (unlikely(olsrTime > (secNow + (12 * 60 * 60)))) {
			/* the message was sent more than 12h later in time:
			 the message was sent yesterday: adjust the date by -1 day */
			now -= (24 * 60 * 60);
			gmtime_r(&now, nowStruct);
		}
	} else {
		/* we are now in the last 12h of the day */
		if (unlikely(olsrTime < (secNow - (12 * 60 * 60)))) {
			/* the message was sent more than 12h earlier in time:
			 the message was sent tomorrow: adjust the date by +1 day */
			now += (24 * 60 * 60);
			gmtime_r(&now, nowStruct);
		}
	}

	nowStruct->tm_hour = ((olsrTime % (24 * 60 * 60)) / 3600);
	nowStruct->tm_min = ((olsrTime % (60 * 60)) / 60);
	nowStruct->tm_sec = (olsrTime % 60);
}

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
void setPositionUpdateTime(PudOlsrPositionUpdate * olsrGpsMessage,
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
double getPositionUpdateLatitude(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	uint32_t olsrLat = olsrGpsMessage->gpsInfo.lat;
	double lat = (double) olsrLat;

	/* lat is in [0, 2^LATITUDE_BITS> */

	/* take half of the rounding error */
	lat += 0.5;

	lat /= (double) (1u << PUD_LATITUDE_BITS);
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
void setPositionUpdateLatitude(
		PudOlsrPositionUpdate * olsrGpsMessage, double latitude) {
	double lat = latitude;

	/* lat is in [-90, 90] */
	assert(lat >= -90.0);
	assert(lat <= 90.0);

	lat /= 180.0;
	/* lat is now in [-0.5, 0.5] */

	lat += 0.5;
	/* lat is now in [0, 1] */

	lat *= (double) (1u << PUD_LATITUDE_BITS);
	/* lat is now in [0, LATITUDE_BITS] */

	/* clip max */
	if (unlikely(lat > (double)((1u << PUD_LATITUDE_BITS) - 1))) {
		lat = (double) ((1u << PUD_LATITUDE_BITS) - 1);
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
double getPositionUpdateLongitude(
		PudOlsrPositionUpdate * olsrGpsMessage) {
	uint32_t olsrLon = olsrGpsMessage->gpsInfo.lon;
	double lon = (double) olsrLon;

	/* lon is in [0, 2^LONGITUDE_BITS> */

	/* take half of the rounding error */
	lon += 0.5;

	lon /= (1u << PUD_LONGITUDE_BITS);
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
void setPositionUpdateLongitude(
		PudOlsrPositionUpdate * olsrGpsMessage, double longitude) {
	double lon = longitude;

	/* lon is in [-180, 180] */
	assert(lon >= -180.0);
	assert(lon <= 180.0);

	lon /= 360.0;
	/* lon is now in [-0.5, 0.5] */

	lon += 0.5;
	/* lon is now in [0, 1] */

	lon *= (double) (1u << PUD_LONGITUDE_BITS);
	/* lon is now in [0, LONGITUDE_BITS] */

	/* clip max */
	if (unlikely(lon > (double)((1u << PUD_LATITUDE_BITS) - 1))) {
		lon = (double) ((1u << PUD_LATITUDE_BITS) - 1);
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
long getPositionUpdateAltitude(
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
void setPositionUpdateAltitude(
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
unsigned long getPositionUpdateSpeed(
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
void setPositionUpdateSpeed(
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
unsigned long getPositionUpdateTrack(
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
void setPositionUpdateTrack(
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
double getPositionUpdateHdop(
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
void setPositionUpdateHdop(PudOlsrPositionUpdate * olsrGpsMessage,
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
NodeIdType getPositionUpdateNodeIdType(int ipVersion,
		PudOlsrPositionUpdate * olsrGpsMessage) {
	if (getPositionUpdatePresent(olsrGpsMessage) & PUD_PRESENT_ID) {
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
void setPositionUpdateNodeIdType(
		PudOlsrPositionUpdate * olsrGpsMessage, NodeIdType nodeIdType) {
	olsrGpsMessage->nodeInfo.nodeIdType = nodeIdType;
}

/**
 Get the nodeId and its size, accounting for nodeId presence

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param olsrMessage
 A pointer to the OLSR message
 @param nodeId
 A pointer to the location where a pointer to the nodeId (as contained in the
 olsrMessage) can be stored
 @param nodeIdSize
 A pointer to the location where the number of bytes in the nodeId can be
 stored
 */
void getPositionUpdateNodeId(int ipVersion, union olsr_message * olsrMessage,
		unsigned char ** nodeId, unsigned int * nodeIdSize) {
	PudOlsrPositionUpdate * olsrGpsMessage = getOlsrMessagePayload(ipVersion,
			olsrMessage);

	*nodeId = &olsrGpsMessage->nodeInfo.nodeId;

	switch (getPositionUpdateNodeIdType(ipVersion, olsrGpsMessage)) {
	case PUD_NODEIDTYPE_MAC: /* hardware address */
		*nodeIdSize = PUD_NODEIDTYPE_MAC_BYTES;
		break;

	case PUD_NODEIDTYPE_MSISDN: /* an MSISDN number */
		*nodeIdSize = PUD_NODEIDTYPE_MSISDN_BYTES;
		break;

	case PUD_NODEIDTYPE_TETRA: /* a Tetra number */
		*nodeIdSize = PUD_NODEIDTYPE_TETRA_BYTES;
		break;

	case PUD_NODEIDTYPE_DNS: /* DNS name */
	  {
	    unsigned int len = 0;
	    unsigned char * idx = *nodeId;
	    unsigned char * lastPayloadByte = &((unsigned char *)olsrMessage)[getOlsrMessageSize(ipVersion, olsrMessage) - 1];
	    while ((*idx != '\0') && (idx <= lastPayloadByte)) {
	      idx++;
	      len++;
	    }
	    *nodeIdSize = len;
	  }
		break;

	case PUD_NODEIDTYPE_UUID: /* a UUID number */
		*nodeIdSize = PUD_NODEIDTYPE_UUID_BYTES;
		break;

	case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
		*nodeIdSize = PUD_NODEIDTYPE_MMSI_BYTES;
		break;

	case PUD_NODEIDTYPE_URN: /* a URN number */
		*nodeIdSize = PUD_NODEIDTYPE_URN_BYTES;
		break;

	case PUD_NODEIDTYPE_MIP: /* a MIP OID number */
		*nodeIdSize = PUD_NODEIDTYPE_MIP_BYTES;
		break;

	case PUD_NODEIDTYPE_192:
		*nodeIdSize = PUD_NODEIDTYPE_192_BYTES;
		break;

	case PUD_NODEIDTYPE_193:
		*nodeIdSize = PUD_NODEIDTYPE_193_BYTES;
		break;

	case PUD_NODEIDTYPE_194:
		*nodeIdSize = PUD_NODEIDTYPE_194_BYTES;
		break;

	case PUD_NODEIDTYPE_IPV4: /* IPv4 address */
	case PUD_NODEIDTYPE_IPV6: /* IPv6 address */
	default: /* unsupported */
	{
		*nodeId = (unsigned char *) getOlsrMessageOriginator(ipVersion,
				olsrMessage);
		*nodeIdSize =
				(ipVersion == AF_INET) ?
						PUD_NODEIDTYPE_IPV4_BYTES : PUD_NODEIDTYPE_IPV6_BYTES;
	}
		break;
	}

	return;
}

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
void setPositionUpdateNodeId(
		PudOlsrPositionUpdate * olsrGpsMessage, unsigned char * nodeId,
		unsigned int nodeIdSize, bool padWithNullByte) {
	memcpy(&olsrGpsMessage->nodeInfo.nodeId, nodeId, nodeIdSize);
	if (unlikely(padWithNullByte)) {
		(&olsrGpsMessage->nodeInfo.nodeId)[nodeIdSize] = '\0';
	}
}

/**
 Convert the node information to the node information for an OLSR message and
 put it in the PUD message in the OLSR message. Also updates the PUD message
 presence field to signal whether or not an ID is in the message.

 @param ipVersion
 The IP version (AF_INET or AF_INET6)
 @param olsrGpsMessage
 A pointer to the PUD message in the OLSR message
 @param olsrMessageSize
 The maximum number of bytes available for the olsrMessage
 @param nodeIdType
 The nodeIdType
 @param nodeId
 The (configured) nodeId in binary/wireformat representation
 @param nodeIdLength
 The number of bytes in the nodeId

 @return
 The number of bytes written in the PUD message in the OLSR message (for ALL
 the node information)
 */
size_t setPositionUpdateNodeInfo(int ipVersion,
		PudOlsrPositionUpdate * olsrGpsMessage, unsigned int olsrMessageSize,
		NodeIdType nodeIdType, unsigned char * nodeId, size_t nodeIdLength) {
	unsigned int length = 0;

	setPositionUpdateNodeIdType(olsrGpsMessage, nodeIdType);
	switch (nodeIdType) {
	case PUD_NODEIDTYPE_MAC: /* hardware address */
	case PUD_NODEIDTYPE_MSISDN: /* an MSISDN number */
	case PUD_NODEIDTYPE_TETRA: /* a Tetra number */
	case PUD_NODEIDTYPE_UUID: /* a UUID number */
	case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
	case PUD_NODEIDTYPE_URN: /* a URN number */
	case PUD_NODEIDTYPE_MIP: /* a MIP OID number */
	case PUD_NODEIDTYPE_192:
	case PUD_NODEIDTYPE_193:
	case PUD_NODEIDTYPE_194:
		length = nodeIdLength;
		setPositionUpdateNodeId(olsrGpsMessage, nodeId, nodeIdLength, false);
		break;

	case PUD_NODEIDTYPE_DNS: /* DNS name */
	{
		long charsAvailable = olsrMessageSize
				- (PUD_OLSRWIREFORMATSIZE + sizeof(NodeInfo)
						- sizeof(olsrGpsMessage->nodeInfo.nodeId)) - 1;

		length = nodeIdLength + 1;
		if (unlikely((long) length > charsAvailable)) {
			length = charsAvailable;
		}

		// FIXME do not pad with a null byte (compatibility breaking change!)
		setPositionUpdateNodeId(olsrGpsMessage, nodeId, length, true);
	}
		break;

	case PUD_NODEIDTYPE_IPV4: /* IPv4 address */
	case PUD_NODEIDTYPE_IPV6: /* IPv6 address */
		/* explicit return: no nodeId information in message */
		return 0;

	default: /* unsupported */
		/* fallback to IP address */
		setPositionUpdateNodeIdType(olsrGpsMessage,
				(ipVersion == AF_INET) ? PUD_NODEIDTYPE_IPV4 :
				PUD_NODEIDTYPE_IPV6);

		/* explicit return: no nodeId information in message */
		return 0;
	}

	setPositionUpdatePresent(olsrGpsMessage,
			getPositionUpdatePresent(olsrGpsMessage) | PUD_PRESENT_ID);
	return ((sizeof(NodeInfo)
			- (sizeof(olsrGpsMessage->nodeInfo.nodeId) /* nodeId placeholder */))
			+ length);
}

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
uint8_t getClusterLeaderVersion(
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
void setClusterLeaderVersion(
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
union olsr_ip_addr * getClusterLeaderOriginator(int ipVersion,
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
union olsr_ip_addr * getClusterLeaderClusterLeader(int ipVersion,
		UplinkClusterLeader * clusterLeaderMessage) {
	if (ipVersion == AF_INET) {
		return (union olsr_ip_addr *) &clusterLeaderMessage->leader.v4.clusterLeader;
	}

	return (union olsr_ip_addr *) &clusterLeaderMessage->leader.v6.clusterLeader;
}
