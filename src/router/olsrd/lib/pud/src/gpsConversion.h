#ifndef _PUD_GPSCONVERSION_H_
#define _PUD_GPSCONVERSION_H_

/* Plugin includes */

/* OLSR includes */
#include "olsr_protocol.h"

/* System includes */
#include <nmea/info.h>

/*
 * Version
 */

/** The version of the transmit sentence */
#define PUD_TX_SENTENCE_VERSION		1

/** The OLSRD message type FIXME get an assigned one */
#define PUD_OLSR_MSG_TYPE 			171

/*
 * Functions
 */

unsigned int gpsToOlsr(nmeaINFO *nmeaInfo, union olsr_message *olsrMessage,
		unsigned int olsrMessageSize, unsigned long long validityTime);

unsigned int gpsFromOlsr(union olsr_message *olsrMessage,
		unsigned char * txGpsBuffer, unsigned int txGpsBufferSize);

#endif /* _PUD_GPSCONVERSION_H_ */
