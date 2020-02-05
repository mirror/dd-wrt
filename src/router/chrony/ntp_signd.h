/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Header for MS-SNTP authentication via Samba (ntp_signd) */

#ifndef GOT_NTP_SIGND_H
#define GOT_NTP_SIGND_H

#include "addressing.h"
#include "ntp.h"

/* Initialisation function */
extern void NSD_Initialise(void);

/* Finalisation function */
extern void NSD_Finalise(void);

/* Function to get an estimate of delay due to signing */
extern int NSD_GetAuthDelay(uint32_t key_id);

/* Function to sign an NTP packet and send it */
extern int NSD_SignAndSendPacket(uint32_t key_id, NTP_Packet *packet, NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr, int length);

#endif
