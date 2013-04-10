#ifndef _PUD_RECEIVER_H_
#define _PUD_RECEIVER_H_

/* Plugin includes */
#include "posAvg.h"

/* OLSRD includes */
#include "olsr_types.h"

/* System includes */
#include <stddef.h>
#include <stdbool.h>

/** Structure of the latest GPS information that is transmitted */
typedef struct _TransmitGpsInformation {
	unsigned char * nodeId; /**< the nodeId */
	bool positionUpdated; /**< true when the position information was updated */
	PositionUpdateEntry txPosition; /**< The last transmitted position */
	union olsr_ip_addr txGateway; /**< the best gateway */
} TransmitGpsInformation;

bool startReceiver(void);
void stopReceiver(void);

void updatePositionFromFile(void);
bool receiverUpdateGpsInformation(unsigned char * rxBuffer, size_t rxCount);

#endif /* _PUD_RECEIVER_H_ */
