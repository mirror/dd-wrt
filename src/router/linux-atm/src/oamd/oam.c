#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <atm.h>
#include <linux/atmoam.h>

#include "oam.h"
#include "io.h"
#include "crc10.h"

#define COMPONENT "OAM"

extern int timer;
struct oamfsm fsm;
struct oamvcc vcc;

void
handle_circuit_state(int state)
{
    vcc.state = state;
    diag(COMPONENT, DIAG_DEBUG, "VCC STATE CHANGE: %s",
	vcc.state ? "DOWN" : "UP");
    if (vcc.state == VCC_UP) {
	system("backuprdsi stop&");
    } else if (vcc.state == VCC_DOWN) {
	system("backuprdsi start&");
    }
}

void
oam_state_print(void)
{
	return;

	printf
	    ("-------------------------------------------------------------------\n");
	printf("T: %d UP: %d DOWN: %d STATE: %d VCCSTATE %d CTag: %lx\n", timer,
	       fsm.upretry, fsm.downretry, fsm.state, vcc.state, vcc.CTag);
	printf("InOAM: %lu OutOAM: %lu CrcErrors: %lu CTagErrors: %lu\n",
	       vcc.stats.InOAM, vcc.stats.OutOAM, vcc.stats.CrcErrors,
	       vcc.stats.CTagErrors);
	printf("InEndLoop: %lu OutEndLoop: %lu\n", vcc.stats.F5.InEndLoop,
	       vcc.stats.F5.OutEndLoop);
	printf
	    ("-------------------------------------------------------------------\n");
}

static int
put_ctag(unsigned char *pdu, unsigned long ctag)
{
	if (pdu == NULL)
		return -1;

	pdu[2] = (unsigned char) (ctag >> 24);
	pdu[3] = (unsigned char) (ctag >> 16);
	pdu[4] = (unsigned char) (ctag >> 8);
	pdu[5] = (unsigned char) ctag;

	return 0;
}

static unsigned long
get_ctag(unsigned char *pdu)
{
	unsigned long ctag;

	if (pdu == NULL)
		return -1;

	ctag = (unsigned long) pdu[5];
	ctag += (unsigned long) (pdu[4] << 8);
	ctag += (unsigned long) (pdu[3] << 16);
	ctag += (unsigned long) (pdu[2] << 24);

	return ctag;
}

void
oam_send(struct atmoam_ctrl *ctrl)
{
    int error;
    
    error = send_kernel(ctrl);
    if (error > 0) {
	fsm.state = OAM_SENT;
	vcc.stats.OutOAM++;
	vcc.stats.F5.OutEndLoop++;
    } else if (error == -EUNATCH) {
	OAM_WAIT_10_S;
	vcc.stats.OutDrops++;
    }
}

static unsigned long
oam_build_lb_cell(struct atmoam_ctrl *ctrl)
{
	unsigned char *oamcell = (unsigned char *) &ctrl->cell;
	static unsigned long tag = 0;
	const unsigned char lpBackLocId[] = {
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF
	};
	const unsigned char srcId[] = {
		0x6A, 0x6A, 0x6A, 0x6A,
		0x6A, 0x6A, 0x6A, 0x6A,
		0x6A, 0x6A, 0x6A, 0x6A,
		0x6A, 0x6A, 0x6A, 0x6A
	};
	unsigned char *pdu = oamcell + 4;
	unsigned char *hdr = oamcell;
	unsigned short crc;

	// fill vpi
	hdr[0] = ctrl->vpi >> 4;
	hdr[1] = (ctrl->vpi & 0x0F) << 4;
	// fill vci
	hdr[1] |= ctrl->vci >> 12;
	hdr[2] = (ctrl->vci >> 4) & 0xFF;
	hdr[3] = 0x0A;
	hdr[3] |= (ctrl->vci & 0x0F) << 4;
	// updates the HEC

	// loopback function
	pdu[0] = 0x18;
	pdu[1] = 0x01;

	put_ctag(pdu, tag);
	tag++;

	// fill the loopback id
	memcpy(pdu + 6, lpBackLocId, 16);
	// fill the Src Id
	memcpy(pdu + 22, (unsigned char *) srcId, 16);
	// fill the unused bytes
	memcpy(pdu + 38, (unsigned char *) srcId, 8);
	// computes the CRC
	pdu[ATM_CELL_PAYLOAD - 2] = 0;
	pdu[ATM_CELL_PAYLOAD - 1] = 0;
	crc = crc10(pdu);
	pdu[ATM_CELL_PAYLOAD - 1] = (unsigned char) crc;
	pdu[ATM_CELL_PAYLOAD - 2] = (unsigned char) (crc >> 8);

	return tag - 1;
}

static int
oam_fault(unsigned char *pdu)
{
	struct oampayload *oampayload = (struct oampayload *) pdu;
	unsigned long ctag;

	if (OAM_FUNC(pdu) == ATM_OAM_F_LOOP) {
		ctag = get_ctag(pdu);

		diag(COMPONENT, DIAG_DEBUG,
		     "\tReceived Fault-Mgmnt (%s) cell, CTag: 0x%lX",
		     oampayload->LoopBackInd ==
		     1 ? "Loopback" : "Loopback reply", ctag);

		if (pdu[1] == 1) {	/* Celda de Loopback */
			vcc.stats.F5.InEndLoop++;
			pdu[1] = 0;	/* Clear Loopback but */

			/* Recalculate CRC10 */
			crc10(pdu);

			/* Volvemos a enviar la celda */
			return 1;
		}
		if (pdu[1] == 0) {	/* Loopback reply */
			if (ctag == vcc.CTag) {
				vcc.stats.F5.InEndLoop++;
				OAM_INC_UPRETRY;
				if (fsm.upretry >= OAM_UPRETRY) {
					if (vcc.state == VCC_DOWN)
						handle_circuit_state(VCC_UP);
				} else {
					if (vcc.state == VCC_DOWN) {
						vcc.CTag =
						    oam_build_lb_cell(&vcc.
								      ctrl);
						oam_send(&vcc.ctrl);
						return 0;
					}
				}
				/* Desde aqui deberiamos ir a ESPERAR a 10 */
				OAM_WAIT_10_S;
				return 0;
			} else {
				printf("Wrong Correlation Tag\n");
				vcc.stats.CTagErrors++;
				OAM_INC_DOWNRETRY;
				if (fsm.downretry >= OAM_DOWNRETRY) {
					if (vcc.state == VCC_UP)
						handle_circuit_state(VCC_DOWN);
				} else {
					if (vcc.state == VCC_UP) {
						vcc.CTag =
						    oam_build_lb_cell(&vcc.
								      ctrl);
						oam_send(&vcc.ctrl);
						return 0;
					}
				}
				/* Desde aqui deberiamos ir a ESPERAR a 10 */
				OAM_WAIT_10_S;
				return 0;
			}
		}
	}
	return 0;
}

void
oam_process(struct atmoam_ctrl *ctrl)
{
	vcc.stats.InOAM++;

	if (crc10_check(ctrl->cell.payload) != 0) {
		diag(COMPONENT, DIAG_ERROR,
		     "Received OAM cell, failed CRC-10 check");
		vcc.stats.CrcErrors++;
		OAM_INC_DOWNRETRY;
		if (fsm.downretry >= OAM_DOWNRETRY) {
			if (vcc.state == VCC_UP)
				handle_circuit_state(VCC_DOWN);
		} else {
			if (vcc.state == VCC_UP) {
				vcc.CTag = oam_build_lb_cell(&vcc.ctrl);
				oam_send(&vcc.ctrl);
				return;
			}
		}
		/* Desde aqui deberiamos ir a ESPERAR a 10 */
		OAM_WAIT_10_S;
		return;
	}

	switch (OAM_TYPE(ctrl->cell.payload)) {
	case ATM_OAM_T_FAULT:
		if (oam_fault(ctrl->cell.payload))
			send_kernel(ctrl);
		break;
	case ATM_OAM_T_PERF:
		break;
	case ATM_OAM_T_ACTDEACT:
		break;
	}
}

void
oam_fsm(void)
{
	switch (fsm.state) {
	case OAM_INIT:
		timer = 0;
		vcc.ctrl.number = 0;
		vcc.ctrl.vpi = 8;
		vcc.ctrl.vci = 32;
		vcc.ctrl.pti = 5;
		vcc.CTag = oam_build_lb_cell(&vcc.ctrl);
		oam_send(&vcc.ctrl);
		break;
	case OAM_SENT:
		OAM_INC_DOWNRETRY;
		if (fsm.downretry >= OAM_DOWNRETRY) {
			if (vcc.state == VCC_UP)
				handle_circuit_state(VCC_DOWN);
		} else {
			if (vcc.state == VCC_UP) {
				vcc.CTag = oam_build_lb_cell(&vcc.ctrl);
				oam_send(&vcc.ctrl);
				return;
			}
		}
		/* Desde aqui deberiamos ir a ESPERAR a 10 */
		OAM_WAIT_10_S;
		break;
	case OAM_WAIT10:
		if (timer != 10)
			return;
		else {
			fsm.state = OAM_INIT;
			return;
		}
	}
}
