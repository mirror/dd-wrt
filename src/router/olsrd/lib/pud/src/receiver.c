#include "receiver.h"

/* Plugin includes */
#include "pud.h"
#include "state.h"
#include "configuration.h"
#include "gpsConversion.h"
#include "networkInterfaces.h"
#include "timers.h"
#include "uplinkGateway.h"
#include "posFile.h"

/* OLSRD includes */
#include "net_olsr.h"

/* System includes */
#include <nmea/parser.h>
#include <nmea/gmath.h>
#include <nmea/sentence.h>
#include <nmea/context.h>
#include <OlsrdPudWireFormat/wireFormat.h>

/*
 * NMEA parser
 */

/** The NMEA string parser */
static nmeaPARSER nmeaParser;

/*
 * State
 */

/** Type describing movement calculations */
typedef struct _MovementType {
	TristateBoolean moving; /**< SET: we are moving */

	TristateBoolean overThresholds; /**< SET: at least 1 threshold state is set */
	TristateBoolean speedOverThreshold; /**< SET: speed is over threshold */
	TristateBoolean hDistanceOverThreshold; /**< SET: horizontal distance is outside threshold */
	TristateBoolean vDistanceOverThreshold; /**< SET: vertical distance is outside threshold */

	TristateBoolean outside; /**< SET: at least 1 outside state is SET */
	TristateBoolean outsideHdop; /**< SET: avg is outside lastTx HDOP */
	TristateBoolean outsideVdop; /**< SET: avg is outside lastTx VDOP */

	TristateBoolean inside; /**< SET: all inside states are SET */
	TristateBoolean insideHdop; /**< SET: avg is inside lastTx HDOP */
	TristateBoolean insideVdop; /**< SET: avg is inside lastTx VDOP */
} MovementType;

/*
 * Averaging
 */

/** The average position with its administration */
static PositionAverageList positionAverageList;

/*
 * TX to OLSR
 */

typedef enum _TimedTxInterface {
	TX_INTERFACE_OLSR = 1,
	TX_INTERFACE_UPLINK = 2
} TimedTxInterface;

/** The latest position information that is transmitted */
static TransmitGpsInformation transmitGpsInformation;

/** The size of the buffer in which the OLSR and uplink messages are assembled */
#define TX_BUFFER_SIZE_FOR_OLSR 1024

/*
 * Functions
 */

/**
 Clear the MovementType
 * @param result a pointer to the MovementType
 */
static void clearMovementType(MovementType * result) {
	/* clear outputs */
	result->moving = TRISTATE_BOOLEAN_UNKNOWN;
	result->overThresholds = TRISTATE_BOOLEAN_UNKNOWN;
	result->speedOverThreshold = TRISTATE_BOOLEAN_UNKNOWN;
	result->hDistanceOverThreshold = TRISTATE_BOOLEAN_UNKNOWN;
	result->vDistanceOverThreshold = TRISTATE_BOOLEAN_UNKNOWN;
	result->outside = TRISTATE_BOOLEAN_UNKNOWN;
	result->outsideHdop = TRISTATE_BOOLEAN_UNKNOWN;
	result->outsideVdop = TRISTATE_BOOLEAN_UNKNOWN;
	result->inside = TRISTATE_BOOLEAN_UNKNOWN;
	result->insideHdop = TRISTATE_BOOLEAN_UNKNOWN;
	result->insideVdop = TRISTATE_BOOLEAN_UNKNOWN;
}

/**
 Determine whether s position is valid.

 @param position
 a pointer to a position

 @return
 - true when valid
 - false otherwise
 */
static bool positionValid(PositionUpdateEntry * position) {
	return (nmea_INFO_is_present(position->nmeaInfo.present, FIX)
			&& (position->nmeaInfo.fix != NMEA_FIX_BAD));
}

/**
 Send the transmit buffer out over all designated interfaces, called as a
 timer callback and also immediately on an external state change.

 @param interfaces
 a bitmap defining which interfaces to send over
 */
static void txToAllOlsrInterfaces(TimedTxInterface interfaces) {
	/** txBuffer is used to concatenate the position update and cluster leader messages in */
	unsigned char txBuffer[TX_BUFFER_SIZE_FOR_OLSR];
	unsigned int txBufferBytesUsed = 0;
	#define txBufferBytesFree	(sizeof(txBuffer) - txBufferBytesUsed)

	/*
	 * The first message in txBuffer is an OLSR position update.
	 *
	 * The position update is always present.
	 *
	 * The second message is the cluster leader message, but only when uplink
	 * was requested and correctly configured.
	 */

	UplinkMessage * pu_uplink = (UplinkMessage *) &txBuffer[0];
	union olsr_message * pu = &pu_uplink->msg.olsrMessage;
	unsigned int pu_size = 0;
	union olsr_ip_addr gateway;
	MovementState externalState;
	nmeaINFO nmeaInfo;

	externalState = getExternalState();

	/* only fixup timestamp when the position is valid _and_ when the position was not updated */
	if (positionValid(&transmitGpsInformation.txPosition) && !transmitGpsInformation.positionUpdated) {
		nmea_time_now(&transmitGpsInformation.txPosition.nmeaInfo.utc, &transmitGpsInformation.txPosition.nmeaInfo.present);
	}

	nmeaInfo = transmitGpsInformation.txPosition.nmeaInfo;
	transmitGpsInformation.positionUpdated = false;
	gateway = transmitGpsInformation.txGateway;

	/* convert nmeaINFO to wireformat olsr message */
	txBufferBytesUsed += sizeof(UplinkHeader); /* keep before txBufferSpaceFree usage */
	pu_size = gpsToOlsr(&nmeaInfo, pu, txBufferBytesFree,
			((externalState == MOVEMENT_STATE_STATIONARY) ? getUpdateIntervalStationary() : getUpdateIntervalMoving()));
	txBufferBytesUsed += pu_size;

	/*
	 * push out to all OLSR interfaces
	 */
	if (((interfaces & TX_INTERFACE_OLSR) != 0) && getOlsrTtl() && (pu_size > 0)) {
		int r;
		struct interface_olsr *ifn;
		for (ifn = ifnet; ifn; ifn = ifn->int_next) {
			/* force the pending buffer out if there's not enough space for our message */
			if ((int)pu_size > net_outbuffer_bytes_left(ifn)) {
			  net_output(ifn);
			}
			r = net_outbuffer_push(ifn, pu, pu_size);
			if (r != (int) pu_size) {
				pudError(
						false,
						"Could not send to OLSR interface %s: %s (size=%u, r=%d)",
						ifn->int_name,
						((r == -1) ? "no buffer was found" :
							(r == 0) ? "there was not enough room in the buffer" : "unknown reason"), pu_size, r);
			}
		}

		/* loopback to tx interface when so configured */
		if (getUseLoopback()) {
			(void) packetReceivedFromOlsr(pu, NULL, NULL);
		}
	}

	/* push out over uplink when an uplink is configured */
	if (((interfaces & TX_INTERFACE_UPLINK) != 0) && isUplinkAddrSet()) {
		int fd = getDownlinkSocketFd();
		if (fd != -1) {
			union olsr_sockaddr * uplink_addr = getUplinkAddr();
			void * addr;
			socklen_t addrSize;

			UplinkMessage * cl_uplink = (UplinkMessage *) &txBuffer[txBufferBytesUsed];
			UplinkClusterLeader * cl = &cl_uplink->msg.clusterLeader;
			union olsr_ip_addr * cl_originator = getClusterLeaderOriginator(olsr_cnf->ip_version, cl);
			union olsr_ip_addr * cl_clusterLeader = getClusterLeaderClusterLeader(olsr_cnf->ip_version, cl);

			unsigned int cl_size =
					sizeof(UplinkClusterLeader) - sizeof(cl->leader)
							+ ((olsr_cnf->ip_version == AF_INET) ? sizeof(cl->leader.v4) :
									sizeof(cl->leader.v6));

			unsigned long long uplinkUpdateInterval =
					(externalState == MOVEMENT_STATE_STATIONARY) ? getUplinkUpdateIntervalStationary() : getUplinkUpdateIntervalMoving();

			if (uplink_addr->in.sa_family == AF_INET) {
				addr = &uplink_addr->in4;
				addrSize = sizeof(struct sockaddr_in);
			} else {
				addr = &uplink_addr->in6;
				addrSize = sizeof(struct sockaddr_in6);
			}

			/*
			 * position update message (pu)
			 */

			/* set header fields in position update uplink message and adjust
			 * the validity time to the uplink validity time */
			if (pu_size > 0) {
				PudOlsrPositionUpdate * pu_gpsMessage = getOlsrMessagePayload(olsr_cnf->ip_version, pu);

				setUplinkMessageType(&pu_uplink->header, POSITION);
				setUplinkMessageLength(&pu_uplink->header, pu_size);
				setUplinkMessageIPv6(&pu_uplink->header, (olsr_cnf->ip_version != AF_INET));
				setUplinkMessagePadding(&pu_uplink->header, 0);

				/* fixup validity time */
				setValidityTime(&pu_gpsMessage->validityTime, uplinkUpdateInterval);
			}

			/*
			 * cluster leader message (cl)
			 */

			/* set cl_uplink header fields */
			setUplinkMessageType(&cl_uplink->header, CLUSTERLEADER);
			setUplinkMessageLength(&cl_uplink->header, cl_size);
			setUplinkMessageIPv6(&cl_uplink->header, (olsr_cnf->ip_version != AF_INET));
			setUplinkMessagePadding(&cl_uplink->header, 0);

			/* setup cl */
			setClusterLeaderVersion(cl, PUD_WIRE_FORMAT_VERSION);
			setValidityTime(&cl->validityTime, uplinkUpdateInterval);

			/* really need 2 memcpy's here because of olsr_cnf->ipsize */
			memcpy(cl_originator, &olsr_cnf->main_addr, olsr_cnf->ipsize);
			memcpy(cl_clusterLeader, &gateway, olsr_cnf->ipsize);

			txBufferBytesUsed += sizeof(UplinkHeader);
			txBufferBytesUsed += cl_size;

			errno = 0;
			if (sendto(fd, &txBuffer, txBufferBytesUsed, 0, addr, addrSize) < 0) {
				pudError(true, "Could not send to uplink (size=%u)", txBufferBytesUsed);
			}
		}
	}
}

/*
 * Timer Callbacks
 */

/**
 The OLSR tx timer callback

 @param context
 unused
 */
static void pud_olsr_tx_timer_callback(void *context __attribute__ ((unused))) {
	txToAllOlsrInterfaces(TX_INTERFACE_OLSR);
}

/**
 The uplink timer callback

 @param context
 unused
 */
static void pud_uplink_timer_callback(void *context __attribute__ ((unused))) {
	txToAllOlsrInterfaces(TX_INTERFACE_UPLINK);
}

/**
 Restart the OLSR tx timer
 */
static void restartOlsrTimer(MovementState externalState) {
	if (!restartOlsrTxTimer(
			(externalState == MOVEMENT_STATE_STATIONARY) ? getUpdateIntervalStationary() :
					getUpdateIntervalMoving(), &pud_olsr_tx_timer_callback)) {
		pudError(0, "Could not restart OLSR tx timer, no periodic"
				" position updates will be sent to the OLSR network");
	}
}

/**
 Restart the uplink tx timer
 */
static void restartUplinkTimer(MovementState externalState) {
	if (!restartUplinkTxTimer(
			(externalState == MOVEMENT_STATE_STATIONARY) ? getUplinkUpdateIntervalStationary() :
					getUplinkUpdateIntervalMoving(),
			&pud_uplink_timer_callback)) {
		pudError(0, "Could not restart uplink timer, no periodic"
				" position updates will be uplinked");
	}
}

static void doImmediateTransmit(MovementState externalState) {
	TimedTxInterface interfaces = TX_INTERFACE_OLSR; /* always send over olsr */
	restartOlsrTimer(externalState);

	if (isUplinkAddrSet()) {
		interfaces |= TX_INTERFACE_UPLINK;
		restartUplinkTimer(externalState);
	}

	/* do an immediate transmit */
	txToAllOlsrInterfaces(interfaces);
}

/**
 The gateway timer callback

 @param context
 unused
 */
static void pud_gateway_timer_callback(void *context __attribute__ ((unused))) {
	union olsr_ip_addr bestGateway;
	bool externalStateChange;
	MovementState externalState;
	TristateBoolean movingNow = TRISTATE_BOOLEAN_UNSET;

	getBestUplinkGateway(&bestGateway);

	/*
	 * Movement detection
	 */

	if (!ipequal(&bestGateway, &transmitGpsInformation.txGateway)) {
		movingNow = TRISTATE_BOOLEAN_SET;
	}

	/*
	 * State Determination
	 */

	determineStateWithHysteresis(SUBSTATE_GATEWAY, movingNow, &externalState, &externalStateChange, NULL);

	/*
	 * Update transmitGpsInformation
	 */

	if (movingNow == TRISTATE_BOOLEAN_SET) {
		transmitGpsInformation.txGateway = bestGateway;
	}

	if (externalStateChange) {
		doImmediateTransmit(externalState);
	}
}

/**
 Detemine whether we are moving from the position, by comparing fields from the
 average position against those of the last transmitted position.

 MUST be called which the position average list locked.

 @param avg
 the average position
 @param lastTx
 the last transmitted position
 @param result
 the results of all movement criteria
 */
static void detemineMovingFromPosition(PositionUpdateEntry * avg, PositionUpdateEntry * lastTx, MovementType * result) {
	/* avg field presence booleans */
	bool avgHasSpeed;
	bool avgHasPos;
	bool avgHasHdop;
	bool avgHasElv;
	bool avgHasVdop;

	/* lastTx field presence booleans */bool lastTxHasPos;
	bool lastTxHasHdop;
	bool lastTxHasElv;
	bool lastTxHasVdop;

	/* these have defaults */
	double dopMultiplier;
	double avgHdop;
	double lastTxHdop;
	double avgVdop;
	double lastTxVdop;

	/* calculated values and their validity booleans */
	double hDistance;
	double vDistance;
	double hdopDistanceForOutside;
	double hdopDistanceForInside;
	double vdopDistanceForOutside;
	double vdopDistanceForInside;
	bool hDistanceValid;
	bool hdopDistanceValid;
	bool vDistanceValid;
	bool vdopDistanceValid;

	/*
	 * Validity
	 *
	 * avg  last  movingNow
	 *  0     0   UNKNOWN : can't determine whether we're moving
	 *  0     1   UNKNOWN : can't determine whether we're moving
	 *  1     0   UNKNOWN : can't determine whether we're moving
	 *  1     1   determine via other parameters
	 */

	if (!positionValid(avg)) {
		result->moving = TRISTATE_BOOLEAN_UNKNOWN;
		return;
	}

	/* avg is valid here */

	if (!positionValid(lastTx)) {
		result->moving = TRISTATE_BOOLEAN_UNKNOWN;
		return;
	}

	/* both avg and lastTx are valid here */

	/* avg field presence booleans */
	avgHasSpeed = nmea_INFO_is_present(avg->nmeaInfo.present, SPEED);
	avgHasPos = nmea_INFO_is_present(avg->nmeaInfo.present, LAT)
			&& nmea_INFO_is_present(avg->nmeaInfo.present, LON);
	avgHasHdop = nmea_INFO_is_present(avg->nmeaInfo.present, HDOP);
	avgHasElv = nmea_INFO_is_present(avg->nmeaInfo.present, ELV);
	avgHasVdop = nmea_INFO_is_present(avg->nmeaInfo.present, VDOP);

	/* lastTx field presence booleans */
	lastTxHasPos = nmea_INFO_is_present(lastTx->nmeaInfo.present, LAT)
			&& nmea_INFO_is_present(lastTx->nmeaInfo.present, LON);
	lastTxHasHdop = nmea_INFO_is_present(lastTx->nmeaInfo.present, HDOP);
	lastTxHasElv = nmea_INFO_is_present(lastTx->nmeaInfo.present, ELV);
	lastTxHasVdop = nmea_INFO_is_present(lastTx->nmeaInfo.present, VDOP);

	/* fill in some values _or_ defaults */
	dopMultiplier = getDopMultiplier();
	avgHdop = avgHasHdop ? avg->nmeaInfo.HDOP : getDefaultHdop();
	lastTxHdop = lastTxHasHdop ? lastTx->nmeaInfo.HDOP : getDefaultHdop();
	avgVdop = avgHasVdop ? avg->nmeaInfo.VDOP : getDefaultVdop();
	lastTxVdop = lastTxHasVdop ? lastTx->nmeaInfo.VDOP : getDefaultVdop();

	/*
	 * Calculations
	 */

	/* hDistance */
	if (avgHasPos && lastTxHasPos) {
		nmeaPOS avgPos;
		nmeaPOS lastTxPos;

		avgPos.lat = nmea_degree2radian(avg->nmeaInfo.lat);
		avgPos.lon = nmea_degree2radian(avg->nmeaInfo.lon);

		lastTxPos.lat = nmea_degree2radian(lastTx->nmeaInfo.lat);
		lastTxPos.lon = nmea_degree2radian(lastTx->nmeaInfo.lon);

		hDistance = fabs(nmea_distance_ellipsoid(&avgPos, &lastTxPos, NULL, NULL));
		hDistanceValid = true;
	} else {
		hDistanceValid = false;
	}

	/* hdopDistance */
	if (avgHasHdop || lastTxHasHdop) {
		hdopDistanceForOutside = dopMultiplier * (lastTxHdop + avgHdop);
		hdopDistanceForInside = dopMultiplier * (lastTxHdop - avgHdop);
		hdopDistanceValid = true;
	} else {
		hdopDistanceValid = false;
	}

	/* vDistance */
	if (avgHasElv && lastTxHasElv) {
		vDistance = fabs(lastTx->nmeaInfo.elv - avg->nmeaInfo.elv);
		vDistanceValid = true;
	} else {
		vDistanceValid = false;
	}

	/* vdopDistance */
	if (avgHasVdop || lastTxHasVdop) {
		vdopDistanceForOutside = dopMultiplier * (lastTxVdop + avgVdop);
		vdopDistanceForInside = dopMultiplier * (lastTxVdop - avgVdop);
		vdopDistanceValid = true;
	} else {
		vdopDistanceValid = false;
	}

	/*
	 * Moving Criteria Evaluation Start
	 * We compare the average position against the last transmitted position.
	 */

	/* Speed */
	if (avgHasSpeed) {
		if (avg->nmeaInfo.speed >= getMovingSpeedThreshold()) {
			result->speedOverThreshold = TRISTATE_BOOLEAN_SET;
		} else {
			result->speedOverThreshold = TRISTATE_BOOLEAN_UNSET;
		}
	}

	/*
	 * Position
	 *
	 * avg  last  hDistanceMoving
	 *  0     0   determine via other parameters
	 *  0     1   determine via other parameters
	 *  1     0   MOVING
	 *  1     1   determine via distance threshold and HDOP
	 */
	if (avgHasPos && !lastTxHasPos) {
		result->hDistanceOverThreshold = TRISTATE_BOOLEAN_SET;
	} else if (hDistanceValid) {
		if (hDistance >= getMovingDistanceThreshold()) {
			result->hDistanceOverThreshold = TRISTATE_BOOLEAN_SET;
		} else {
			result->hDistanceOverThreshold = TRISTATE_BOOLEAN_UNSET;
		}

		/*
		 * Position with HDOP
		 *
		 * avg  last  movingNow
		 *  0     0   determine via other parameters
		 *  0     1   determine via position with HDOP (avg has default HDOP)
		 *  1     0   determine via position with HDOP (lastTx has default HDOP)
		 *  1     1   determine via position with HDOP
		 */
		if (hdopDistanceValid) {
			/* we are outside the HDOP when the HDOPs no longer overlap */
			if (hDistance > hdopDistanceForOutside) {
				result->outsideHdop = TRISTATE_BOOLEAN_SET;
			} else {
				result->outsideHdop = TRISTATE_BOOLEAN_UNSET;
			}

			/* we are inside the HDOP when the HDOPs fully overlap */
			if (hDistance <= hdopDistanceForInside) {
				result->insideHdop = TRISTATE_BOOLEAN_SET;
			} else {
				result->insideHdop = TRISTATE_BOOLEAN_UNSET;
			}
		}
	}

	/*
	 * Elevation
	 *
	 * avg  last  movingNow
	 *  0     0   determine via other parameters
	 *  0     1   determine via other parameters
	 *  1     0   MOVING
	 *  1     1   determine via distance threshold and VDOP
	 */
	if (avgHasElv && !lastTxHasElv) {
		result->vDistanceOverThreshold = TRISTATE_BOOLEAN_SET;
	} else if (vDistanceValid) {
		if (vDistance >= getMovingDistanceThreshold()) {
			result->vDistanceOverThreshold = TRISTATE_BOOLEAN_SET;
		} else {
			result->vDistanceOverThreshold = TRISTATE_BOOLEAN_UNSET;
		}

		/*
		 * Elevation with VDOP
		 *
		 * avg  last  movingNow
		 *  0     0   determine via other parameters
		 *  0     1   determine via elevation with VDOP (avg has default VDOP)
		 *  1     0   determine via elevation with VDOP (lastTx has default VDOP)
		 *  1     1   determine via elevation with VDOP
		 */
		if (vdopDistanceValid) {
			/* we are outside the VDOP when the VDOPs no longer overlap */
			if (vDistance > vdopDistanceForOutside) {
				result->outsideVdop = TRISTATE_BOOLEAN_SET;
			} else {
				result->outsideVdop = TRISTATE_BOOLEAN_UNSET;
			}

			/* we are inside the VDOP when the VDOPs fully overlap */
			if (vDistance <= vdopDistanceForInside) {
				result->insideVdop = TRISTATE_BOOLEAN_SET;
			} else {
				result->insideVdop = TRISTATE_BOOLEAN_UNSET;
			}
		}
	}

	/*
	 * Moving Criteria Evaluation End
	 */

	/* accumulate inside criteria */
	if ((result->insideHdop == TRISTATE_BOOLEAN_SET) && (result->insideVdop == TRISTATE_BOOLEAN_SET)) {
		result->inside = TRISTATE_BOOLEAN_SET;
	} else if ((result->insideHdop == TRISTATE_BOOLEAN_UNSET) || (result->insideVdop == TRISTATE_BOOLEAN_UNSET)) {
		result->inside = TRISTATE_BOOLEAN_UNSET;
	}

	/* accumulate outside criteria */
	if ((result->outsideHdop == TRISTATE_BOOLEAN_SET) || (result->outsideVdop == TRISTATE_BOOLEAN_SET)) {
		result->outside = TRISTATE_BOOLEAN_SET;
	} else if ((result->outsideHdop == TRISTATE_BOOLEAN_UNSET)
			|| (result->outsideVdop == TRISTATE_BOOLEAN_UNSET)) {
		result->outside = TRISTATE_BOOLEAN_UNSET;
	}

	/* accumulate threshold criteria */
	if ((result->speedOverThreshold == TRISTATE_BOOLEAN_SET)
			|| (result->hDistanceOverThreshold == TRISTATE_BOOLEAN_SET)
			|| (result->vDistanceOverThreshold == TRISTATE_BOOLEAN_SET)) {
		result->overThresholds = TRISTATE_BOOLEAN_SET;
	} else if ((result->speedOverThreshold == TRISTATE_BOOLEAN_UNSET)
			|| (result->hDistanceOverThreshold == TRISTATE_BOOLEAN_UNSET)
			|| (result->vDistanceOverThreshold == TRISTATE_BOOLEAN_UNSET)) {
		result->overThresholds = TRISTATE_BOOLEAN_UNSET;
	}

	/* accumulate moving criteria */
	if ((result->overThresholds == TRISTATE_BOOLEAN_SET) || (result->outside == TRISTATE_BOOLEAN_SET)) {
		result->moving = TRISTATE_BOOLEAN_SET;
	} else if ((result->overThresholds == TRISTATE_BOOLEAN_UNSET)
			&& (result->outside == TRISTATE_BOOLEAN_UNSET)) {
		result->moving = TRISTATE_BOOLEAN_UNSET;
	}

	return;
}

/**
 Update the latest GPS information. This function is called when a packet is
 received from a rxNonOlsr interface, containing one or more NMEA strings with
 GPS information.

 @param rxBuffer
 the receive buffer with the received NMEA string(s)
 @param rxCount
 the number of bytes in the receive buffer

 @return
 - false on failure
 - true otherwise
 */
bool receiverUpdateGpsInformation(unsigned char * rxBuffer, size_t rxCount) {
	static const char * rxBufferPrefix = "$GP";
	static const size_t rxBufferPrefixLength = 3;

	bool retval = false;
	PositionUpdateEntry * incomingEntry;
	PositionUpdateEntry * posAvgEntry;
	MovementType movementResult;
	bool subStateExternalStateChange;
	bool externalStateChange;
	bool updateTransmitGpsInformation = false;
	MovementState externalState;

	/* do not process when the message does not start with $GP */
	if ((rxCount < rxBufferPrefixLength) || (strncmp((char *) rxBuffer,
			rxBufferPrefix, rxBufferPrefixLength) != 0)) {
		return true;
	}

	/* parse all NMEA strings in the rxBuffer into the incoming entry */
	incomingEntry = getPositionAverageEntry(&positionAverageList, INCOMING);
	nmea_zero_INFO(&incomingEntry->nmeaInfo);
	nmea_parse(&nmeaParser, (char *) rxBuffer, rxCount,
			&incomingEntry->nmeaInfo);

	/* ignore when no useful information */
	if (incomingEntry->nmeaInfo.smask == GPNON) {
		retval = true;
		goto end;
	}

	nmea_INFO_sanitise(&incomingEntry->nmeaInfo);

	/* we always work with latitude, longitude in degrees and DOPs in meters */
	nmea_INFO_unit_conversion(&incomingEntry->nmeaInfo);

	/*
	 * Averaging
	 */

	if (getInternalState(SUBSTATE_POSITION) == MOVEMENT_STATE_MOVING) {
		/* flush average: keep only the incoming entry */
		flushPositionAverageList(&positionAverageList);
	}
	addNewPositionToAverage(&positionAverageList, incomingEntry);
	posAvgEntry = getPositionAverageEntry(&positionAverageList, AVERAGE);

	/*
	 * Movement detection
	 */

	clearMovementType(&movementResult);

	detemineMovingFromPosition(posAvgEntry, &transmitGpsInformation.txPosition, &movementResult);

	/*
	 * State Determination
	 */

	determineStateWithHysteresis(SUBSTATE_POSITION, movementResult.moving, &externalState, &externalStateChange,
			&subStateExternalStateChange);

	/*
	 * Update transmitGpsInformation
	 */

	updateTransmitGpsInformation = subStateExternalStateChange
			|| (positionValid(posAvgEntry) && !positionValid(&transmitGpsInformation.txPosition))
			|| (movementResult.inside == TRISTATE_BOOLEAN_SET);

	if ((externalState == MOVEMENT_STATE_MOVING) || updateTransmitGpsInformation) {
		transmitGpsInformation.txPosition = *posAvgEntry;
		transmitGpsInformation.positionUpdated = true;

		/*
		 * When we're stationary:
		 * - the track is not reliable or even invalid, so we must clear it.
		 * - to avoid confusion in consumers of the data, we must clear the speed
		 *   because it is possible to have a very low speed while moving.
		 */
		if (externalState == MOVEMENT_STATE_STATIONARY) {
			transmitGpsInformation.txPosition.nmeaInfo.speed = (double)0.0;
			transmitGpsInformation.txPosition.nmeaInfo.track = (double)0.0;
		}
	}

	if (externalStateChange) {
		doImmediateTransmit(externalState);
	}

	retval = true;

	end:
	return retval;
}

/**
 * Log nmea library errors as plugin errors
 * @param str
 * @param str_size
 */
static void nmea_errors(const char *str, int str_size __attribute__((unused))) {
	pudError(false, "NMEA library error: %s", str);
}

/**
 * Helper function to read the position file into the transmit position
 */
void updatePositionFromFile(void) {
  char * positionFile = getPositionFile();
  if (positionFile) {
    readPositionFile(positionFile, &transmitGpsInformation.txPosition.nmeaInfo);
  }
}

/**
 Start the receiver

 @return
 - false on failure
 - true otherwise
 */
bool startReceiver(void) {
	MovementState externalState;

	if (!nmea_parser_init(&nmeaParser)) {
		pudError(false, "Could not initialise NMEA parser");
		return false;
	}

	/* hook up the NMEA library error callback */
	nmea_context_set_error_func(&nmea_errors);

	nmea_zero_INFO(&transmitGpsInformation.txPosition.nmeaInfo);
	updatePositionFromFile();

	transmitGpsInformation.txGateway = olsr_cnf->main_addr;
	transmitGpsInformation.positionUpdated = false;
	transmitGpsInformation.nodeId = getNodeId(NULL);

#ifdef HTTPINFO_PUD
	olsr_cnf->pud_position = &transmitGpsInformation;
#endif /* HTTPINFO_PUD */
	initPositionAverageList(&positionAverageList, getAverageDepth());

	if (!initOlsrTxTimer()) {
		stopReceiver();
		return false;
	}

	if (!initUplinkTxTimer()) {
		stopReceiver();
		return false;
	}

	if (!initGatewayTimer()) {
		stopReceiver();
		return false;
	}

	externalState = getExternalState();
	restartOlsrTimer(externalState);
	restartUplinkTimer(externalState);
	if (!restartGatewayTimer(getGatewayDeterminationInterval(), &pud_gateway_timer_callback)) {
		pudError(0, "Could not start gateway timer");
		stopReceiver();
		return false;
	}

	return true;
}

/**
 Stop the receiver
 */
void stopReceiver(void) {
	destroyGatewayTimer();
	destroyUplinkTxTimer();
	destroyOlsrTxTimer();

	destroyPositionAverageList(&positionAverageList);

	nmea_zero_INFO(&transmitGpsInformation.txPosition.nmeaInfo);
	transmitGpsInformation.txGateway = olsr_cnf->main_addr;
	transmitGpsInformation.positionUpdated = false;
}
