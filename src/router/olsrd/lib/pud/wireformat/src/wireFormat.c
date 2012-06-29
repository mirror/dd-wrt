#include <OlsrdPudWireFormat/wireFormat.h>

#include <time.h>

/* ************************************************************************
 * Validity Time
 * ************************************************************************ */

/* inline getValidityTime */

/**
 Set the validity time of the position update message

 @param validityTimeField
 A pointer to the validity time field
 @param validityTime
 The validity time in seconds
 */
void setValidityTime(uint8_t * validityTimeField,
		unsigned long long validityTime) {
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
		unsigned long resolution = (1 << msn);
		lsn = ((validityTime - lowerBound + (resolution >> 1)) / resolution);
	}

	assert(msn <= 15);
	assert(lsn <= 15);

	*validityTimeField = ((msn << 4) | lsn);
}

/* ************************************************************************
 * UplinkPositionUpdate
 * ************************************************************************ */

/* inline uint8_t getUplinkMessageType */
/* inline void setUplinkMessageType */

/* inline uint16_t getUplinkMessageLength */
/* inline void setUplinkMessageLength */

/* inline bool getUplinkMessageIPv6 */
/* inline void setUplinkMessageIPv6 */

/* inline void setUplinkMessagePadding */

/* ************************************************************************
 * OLSR Header
 * ************************************************************************ */

/* inline union olsr_ip_addr * getOlsrMessageOriginator */

/* inline PudOlsrPositionUpdate * getOlsrMessagePayload */

/* ************************************************************************
 * PudOlsrPositionUpdate
 * ************************************************************************ */

/* inline uint8_t getPositionUpdateVersion */
/* inline void setPositionUpdateVersion */

/* inline uint8_t getPositionUpdateSmask */
/* inline void setPositionUpdateSmask */

/* ************************************************************************
 * GpsInfo
 * ************************************************************************ */

/* inline void setPositionUpdateTime */

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

/* inline double getPositionUpdateLatitude */
/* inline void setPositionUpdateLatitude */

/* inline double getPositionUpdateLongitude */
/* inline void setPositionUpdateLongitude */

/* inline long getPositionUpdateAltitude */
/* inline void setPositionUpdateAltitude */

/* inline unsigned long getPositionUpdateSpeed */
/* inline void setPositionUpdateSpeed */

/* inline unsigned long getPositionUpdateTrack */
/* inline void setPositionUpdateTrack */

/* inline double getPositionUpdateHdop */
/* inline void setPositionUpdateHdop */

/* ************************************************************************
 * NodeInfo
 * ************************************************************************ */

/* inline NodeIdType getPositionUpdateNodeIdType */
/* inline void setPositionUpdateNodeIdType */

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
		*nodeIdSize = strlen((char *) *nodeId);
		/* FIXME for no '\0' at the end, need to scan from the end until
		 * encountering a non-zero byte: end of string address and
		 * subtract the string start address */
		break;

	case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
		*nodeIdSize = PUD_NODEIDTYPE_MMSI_BYTES;
		break;

	case PUD_NODEIDTYPE_URN: /* a URN number */
		*nodeIdSize = PUD_NODEIDTYPE_URN_BYTES;
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
 Convert the node information to the node information for an OLSR message and
 put it in the PUD message in the OLSR message. Also updates the PUD message
 smask.

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
	case PUD_NODEIDTYPE_MMSI: /* an AIS MMSI number */
	case PUD_NODEIDTYPE_URN: /* a URN number */
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

	setPositionUpdateFlags(olsrGpsMessage,
			getPositionUpdateFlags(olsrGpsMessage) | PUD_FLAGS_ID);
	return ((sizeof(NodeInfo)
			- (sizeof(olsrGpsMessage->nodeInfo.nodeId) /* nodeId placeholder */))
			+ length);
}
