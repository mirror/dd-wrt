#ifndef EGRESSFILE_H
#define EGRESSFILE_H

#ifdef __linux__

/* Plugin includes */

/* OLSRD includes */
#include "egressTypes.h"

/* System includes */
#include <stdbool.h>
#include <stdint.h>

#define DEF_EGRESS_UPLINK_KBPS       0
#define DEF_EGRESS_DOWNLINK_KBPS     0
#define DEF_EGRESS_PATH_COSTS        0

bool startEgressFile(void);
void stopEgressFile(void);

struct sgw_egress_if * findEgressInterface(char * name);
struct sgw_egress_if * findEgressInterfaceByIndex(int if_index);

bool egressBwCalculateCosts(struct egress_if_bw * bw, bool up);
void egressBwClear(struct egress_if_bw * egress_if, bool up);

#endif /* __linux__ */

#endif /* EGRESSFILE_H */
