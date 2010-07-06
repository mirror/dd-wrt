/*
 * gateway_default_handler.h
 *
 *  Created on: Jan 29, 2010
 *      Author: rogge
 */

#ifndef GATEWAY_DEFAULT_HANDLER_H_
#define GATEWAY_DEFAULT_HANDLER_H_

#ifndef WIN32
#include "gateway.h"

#define GW_DEFAULT_TIMER_INTERVAL 10*1000
#define GW_DEFAULT_STABLE_COUNT   6

void olsr_gw_default_init(void);
void olsr_gw_default_lookup_gateway(bool, bool);

#endif /* !WIN32 */
#endif /* GATEWAY_DEFAULT_HANDLER_H_ */
