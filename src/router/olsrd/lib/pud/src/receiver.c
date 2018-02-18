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
#include <nmealib/context.h>
#include <nmealib/nmath.h>
#include <nmealib/sentence.h>
#include <OlsrdPudWireFormat/wireFormat.h>
#include <unistd.h>

static void receiverProcessIncomingEntry(PositionUpdateEntry * incomingEntry);

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

/** the prefix of all parameters written to the position output file */
#define PUD_POSOUT_FILE_PARAM_PREFIX "OLSRD_PUD_"

/** the position output file */
static char * positionOutputFile = NULL;

/** true when a postion output file error was already reported */
static bool positionOutputFileError = false;

/** gpsd daemon data */
struct gps_data_t gpsdata;

/** gpsd daemon connection tracking */
static struct GpsdConnectionState connectionTracking;

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
	return (nmeaInfoIsPresentAll(position->nmeaInfo.present, NMEALIB_PRESENT_FIX)
			&& (position->nmeaInfo.fix != NMEALIB_FIX_BAD));
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
	NmeaInfo nmeaInfo;

	externalState = getExternalState();

	/* only update the timestamp when the position is invalid AND when the position was not updated */
	if (!positionValid(&transmitGpsInformation.txPosition) && !transmitGpsInformation.positionUpdated) {
	  nmeaTimeSet(&transmitGpsInformation.txPosition.nmeaInfo.utc, &transmitGpsInformation.txPosition.nmeaInfo.present, NULL);
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
				/* do not report send errors, they're not really relevant */
			}
		}
	}
}

/**
 * Write the position output file (if so configured)
 *
 * @return true on success
 */
static bool writePositionOutputFile(void) {
  FILE * fp = NULL;
  NmeaInfo * nmeaInfo;
  const char * signal;
  const char * fix;

  if (!positionOutputFile) {
    return true;
  }

  fp = fopen(positionOutputFile, "w");
  if (!fp) {
    /* could not open the file */
    if (!positionOutputFileError) {
      pudError(true, "Could not write to the position output file \"%s\"", positionOutputFile);
      positionOutputFileError = true;
    }
    return false;
  }
  positionOutputFileError = false;

  nmeaInfo = &transmitGpsInformation.txPosition.nmeaInfo;

  /* node id */
  fprintf(fp, "%s%s=\"%s\"\n", PUD_POSOUT_FILE_PARAM_PREFIX, "NODE_ID", transmitGpsInformation.nodeId);

  /* sig */
  switch (nmeaInfo->sig) {
    case 0:
      signal = "INVALID";
      break;

    case 1:
      signal = "FIX";
      break;

    case 2:
      signal = "DIFFERENTIAL";
      break;

    case 3:
      signal = "SENSITIVE";
      break;

    case 4:
      signal = "REALTIME";
      break;

    case 5:
      signal = "FLOAT";
      break;

    case 6:
      signal = "ESTIMATED";
      break;

    case 7:
      signal = "MANUAL";
      break;

    case 8:
      signal = "SIMULATED";
      break;

    default:
      signal = NULL;
      break;
  }
  if (!signal) {
    fprintf(fp, "%s%s=%d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "SIGNAL", nmeaInfo->sig);
  } else {
    fprintf(fp, "%s%s=%s\n", PUD_POSOUT_FILE_PARAM_PREFIX, "SIGNAL", signal);
  }

  /* fix */
  switch (nmeaInfo->fix) {
    case 1:
      fix = "NONE";
      break;

    case 2:
      fix = "2D";
      break;

    case 3:
      fix = "3D";
      break;

    default:
      fix = NULL;
      break;
  }
  if (!fix) {
    fprintf(fp, "%s%s=%d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "FIX", nmeaInfo->fix);
  } else {
    fprintf(fp, "%s%s=%s\n", PUD_POSOUT_FILE_PARAM_PREFIX, "FIX", fix);
  }

  /* utc */
  fprintf(fp, "%s%s=%04d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "YEAR", nmeaInfo->utc.year);
  fprintf(fp, "%s%s=%02d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "MONTH", nmeaInfo->utc.mon);
  fprintf(fp, "%s%s=%02d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "DAY", nmeaInfo->utc.day);
  fprintf(fp, "%s%s=%02d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "HOUR", nmeaInfo->utc.hour);
  fprintf(fp, "%s%s=%02d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "MINUTE", nmeaInfo->utc.min);
  fprintf(fp, "%s%s=%02d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "SECONDS", nmeaInfo->utc.sec);
  fprintf(fp, "%s%s=%03d\n", PUD_POSOUT_FILE_PARAM_PREFIX, "MILLISECONDS", nmeaInfo->utc.hsec * 10);

  /* DOPs */
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "PDOP", nmeaInfo->pdop);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "HDOP", nmeaInfo->hdop);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "VDOP", nmeaInfo->vdop);

  /* lat, lon, ... */
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "LATTITUDE", nmeaInfo->latitude);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "LONGITUDE", nmeaInfo->longitude);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "ELEVATION", nmeaInfo->elevation);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "SPEED", nmeaInfo->speed);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "TRACK", nmeaInfo->track);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "MAGNETIC_TRACK", nmeaInfo->mtrack);
  fprintf(fp, "%s%s=%f\n", PUD_POSOUT_FILE_PARAM_PREFIX, "MAGNETIC_VARIATION", nmeaInfo->magvar);

  fclose(fp);
  return true;
}

/*
 * Timer Callbacks
 */

/**
 The gpsd fetch timer callback

 @param context
 unused
 */
static void pud_gpsd_fetch_timer_callback(void *context __attribute__ ((unused))) {
  static char cnt = 0;

  PositionUpdateEntry * incomingEntry;
  GpsDaemon *gpsd = getGpsd();
  if (!gpsd) {
    return;
  }

  /* fetch info from gpsd daemon into the incoming entry */
  incomingEntry = getPositionAverageEntry(&positionAverageList, INCOMING);
  if (!cnt) {
    nmeaInfoClear(&incomingEntry->nmeaInfo);
    nmeaTimeSet(&incomingEntry->nmeaInfo.utc, &incomingEntry->nmeaInfo.present, NULL);
  }
  readFromGpsd(gpsd, &gpsdata, &connectionTracking, &incomingEntry->nmeaInfo);

  if (cnt >= (TIMER_GPSD_READS_PER_SEC - 1)) {
    receiverProcessIncomingEntry(incomingEntry);
    cnt = 0;
  } else {
    cnt++;
  }
}

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
 Restart the gpsd fetch timer
 */
static void restartGpsdTimer(void) {
  if (!getGpsd()) {
    return;
  }

  if (!restartGpsdFetchTimer(&pud_gpsd_fetch_timer_callback)) {
    pudError(0, "Could not restart gpsd fetch timer, no position information will be available");
  }
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

	/* write the position output file */
	writePositionOutputFile();
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

	determineStateWithHysteresis(SUBSTATE_GATEWAY, movingNow, &externalState, &externalStateChange, NULL, false);

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

	/* lastTx field presence booleans */
	bool lastTxHasPos;
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
	avgHasSpeed = nmeaInfoIsPresentAll(avg->nmeaInfo.present, NMEALIB_PRESENT_SPEED);
	avgHasPos = nmeaInfoIsPresentAll(avg->nmeaInfo.present, NMEALIB_PRESENT_LAT)
			&& nmeaInfoIsPresentAll(avg->nmeaInfo.present, NMEALIB_PRESENT_LON);
	avgHasHdop = nmeaInfoIsPresentAll(avg->nmeaInfo.present, NMEALIB_PRESENT_HDOP);
	avgHasElv = nmeaInfoIsPresentAll(avg->nmeaInfo.present, NMEALIB_PRESENT_ELV);
	avgHasVdop = nmeaInfoIsPresentAll(avg->nmeaInfo.present, NMEALIB_PRESENT_VDOP);

	/* lastTx field presence booleans */
	lastTxHasPos = nmeaInfoIsPresentAll(lastTx->nmeaInfo.present, NMEALIB_PRESENT_LAT)
			&& nmeaInfoIsPresentAll(lastTx->nmeaInfo.present, NMEALIB_PRESENT_LON);
	lastTxHasHdop = nmeaInfoIsPresentAll(lastTx->nmeaInfo.present, NMEALIB_PRESENT_HDOP);
	lastTxHasElv = nmeaInfoIsPresentAll(lastTx->nmeaInfo.present, NMEALIB_PRESENT_ELV);
	lastTxHasVdop = nmeaInfoIsPresentAll(lastTx->nmeaInfo.present, NMEALIB_PRESENT_VDOP);

	/* fill in some values _or_ defaults */
	dopMultiplier = getDopMultiplier();
	avgHdop = avgHasHdop ? avg->nmeaInfo.hdop : getDefaultHdop();
	lastTxHdop = lastTxHasHdop ? lastTx->nmeaInfo.hdop : getDefaultHdop();
	avgVdop = avgHasVdop ? avg->nmeaInfo.vdop : getDefaultVdop();
	lastTxVdop = lastTxHasVdop ? lastTx->nmeaInfo.vdop: getDefaultVdop();

	/*
	 * Calculations
	 */

	/* hDistance */
	if (avgHasPos && lastTxHasPos) {
		NmeaPosition avgPos;
		NmeaPosition lastTxPos;

		avgPos.lat = nmeaMathDegreeToRadian(avg->nmeaInfo.latitude);
		avgPos.lon = nmeaMathDegreeToRadian(avg->nmeaInfo.longitude);

		lastTxPos.lat = nmeaMathDegreeToRadian(lastTx->nmeaInfo.latitude);
		lastTxPos.lon = nmeaMathDegreeToRadian(lastTx->nmeaInfo.longitude);

		hDistance = fabs(nmeaMathDistanceEllipsoid(&avgPos, &lastTxPos, NULL, NULL));
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
		vDistance = fabs(lastTx->nmeaInfo.elevation - avg->nmeaInfo.elevation);
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

static void receiverProcessIncomingEntry(PositionUpdateEntry * incomingEntry) {
	static bool gpnonPrev = false;

	PositionUpdateEntry * posAvgEntry;
	MovementType movementResult;
	bool subStateExternalStateChange;
	bool externalStateChange;
	bool updateTransmitGpsInformation = false;
	MovementState externalState;
	bool gpnon;
	bool gpnonChanged;

	/* we always work with latitude, longitude in degrees and DOPs in meters */
	nmeaInfoUnitConversion(&incomingEntry->nmeaInfo, true);

	gpnon = !nmeaInfoIsPresentAll(incomingEntry->nmeaInfo.present, NMEALIB_PRESENT_SMASK) || (incomingEntry->nmeaInfo.smask == NMEALIB_SENTENCE_GPNON);
	gpnonChanged = gpnon != gpnonPrev;
	gpnonPrev = gpnon;

	/*
	 * Averaging
	 */

	if ((getInternalState(SUBSTATE_POSITION) == MOVEMENT_STATE_MOVING) || gpnonChanged) {
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
			&subStateExternalStateChange, gpnonChanged);

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
}

/**
 * Log nmea library errors as plugin errors
 * @param str
 * @param sz
 */
static void nmea_errors(const char *s, size_t sz __attribute__((unused))) {
	pudError(false, "NMEA library error: %s", s);
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

	/* hook up the NMEA library error callback */
	nmeaContextSetErrorFunction(&nmea_errors);

	nmeaInfoClear(&transmitGpsInformation.txPosition.nmeaInfo);
	nmeaTimeSet(&transmitGpsInformation.txPosition.nmeaInfo.utc, &transmitGpsInformation.txPosition.nmeaInfo.present, NULL);
	updatePositionFromFile();

	transmitGpsInformation.txGateway = olsr_cnf->main_addr;
	transmitGpsInformation.positionUpdated = false;
	transmitGpsInformation.nodeId = getNodeId(NULL);

	olsr_cnf->pud_position = &transmitGpsInformation;

	initPositionAverageList(&positionAverageList, getAverageDepth());

	memset(&gpsdata, 0, sizeof(gpsdata));
	gpsdata.gps_fd = -1;
	memset(&connectionTracking, 0, sizeof(connectionTracking));

	if (!initGpsdFetchTimer()) {
	  stopReceiver();
	  return false;
	}

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

	positionOutputFile = getPositionOutputFile();
	positionOutputFileError = false;

	/* write the position output file */
	if (!writePositionOutputFile()) {
	  return false;
	}

	externalState = getExternalState();
	restartGpsdTimer();
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
	if (positionOutputFile) {
	  unlink(positionOutputFile);
	}

	destroyGatewayTimer();
	destroyUplinkTxTimer();
	destroyOlsrTxTimer();
	destroyGpsdFetchTimer();

	destroyPositionAverageList(&positionAverageList);

	gpsdDisconnect(&gpsdata, &connectionTracking);
	memset(&connectionTracking, 0, sizeof(connectionTracking));
	memset(&gpsdata, 0, sizeof(gpsdata));
	gpsdata.gps_fd = -1;

	nmeaInfoClear(&transmitGpsInformation.txPosition.nmeaInfo);
	transmitGpsInformation.txGateway = olsr_cnf->main_addr;
	transmitGpsInformation.positionUpdated = false;
}
