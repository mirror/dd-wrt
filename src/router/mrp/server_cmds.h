// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#ifndef SERVER_CMDS_H
#define SERVER_CMDS_H

#include "utils.h"

enum
{
	IF_OPER_UNKNOWN,
	IF_OPER_NOTPRESENT,
	IF_OPER_DOWN,
	IF_OPER_LOWERLAYERDOWN,
	IF_OPER_TESTING,
	IF_OPER_DORMANT,
	IF_OPER_UP,
};

int CTL_addmrp(int br_index, int ring_nr, int pport, int sport, int ring_role,
	       uint16_t prio, uint8_t ring_recv, uint8_t react_on_link_change,
	       int in_role, uint16_t in_id, int iport);
int CTL_delmrp(int br_index, int ring_nr);
int CTL_getmrp(int *count, struct mrp_status *status);

int CTL_init(void);
void CTL_cleanup(void);

#endif /* SERVER_CMDS_H */
