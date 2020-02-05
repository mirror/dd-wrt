/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
 * Copyright (C) Miroslav Lichvar  2014
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

  Header for the part of the software that deals with the set of
  current NTP servers and peers, which can resolve an IP address into
  a source record for further processing.

  */

#ifndef GOT_NTP_SOURCES_H
#define GOT_NTP_SOURCES_H

#include "ntp.h"
#include "addressing.h"
#include "srcparams.h"
#include "ntp_core.h"
#include "reports.h"

/* Status values returned by operations that indirectly result from user
   input. */
typedef enum {
  NSR_Success, /* Operation successful */
  NSR_NoSuchSource, /* Remove - attempt to remove a source that is not known */
  NSR_AlreadyInUse, /* AddSource - attempt to add a source that is already known */ 
  NSR_TooManySources, /* AddSource - too many sources already present */
  NSR_InvalidAF /* AddSource - attempt to add a source with invalid address family */
} NSR_Status;

/* Procedure to add a new server or peer source. */
extern NSR_Status NSR_AddSource(NTP_Remote_Address *remote_addr, NTP_Source_Type type, SourceParameters *params);

/* Procedure to add a new server, peer source, or pool of servers specified by
   name instead of address.  The name is resolved in exponentially increasing
   intervals until it succeeds or fails with a non-temporary error. */
extern void NSR_AddSourceByName(char *name, int port, int pool, NTP_Source_Type type, SourceParameters *params);

/* Function type for handlers to be called back when an attempt
 * (possibly unsuccessful) to resolve unresolved sources ends */
typedef void (*NSR_SourceResolvingEndHandler)(void);

/* Set the handler, or NULL to disable the notification */
extern void NSR_SetSourceResolvingEndHandler(NSR_SourceResolvingEndHandler handler);

/* Procedure to start resolving unresolved sources */
extern void NSR_ResolveSources(void);

/* Procedure to start all sources */
extern void NSR_StartSources(void);

/* Start new sources automatically */
extern void NSR_AutoStartSources(void);

/* Procedure to remove a source */
extern NSR_Status NSR_RemoveSource(NTP_Remote_Address *remote_addr);

/* Procedure to remove all sources */
extern void NSR_RemoveAllSources(void);

/* Procedure to try to find a replacement for a bad source */
extern void NSR_HandleBadSource(IPAddr *address);

/* Procedure to resolve all names again */
extern void NSR_RefreshAddresses(void);

/* Procedure to get local reference ID corresponding to a source */
extern uint32_t NSR_GetLocalRefid(IPAddr *address);

/* This routine is called by ntp_io when a new packet arrives off the network */
extern void NSR_ProcessRx(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                          NTP_Local_Timestamp *rx_ts, NTP_Packet *message, int length);

/* This routine is called by ntp_io when a packet was sent to the network and
   an accurate transmit timestamp was captured */
extern void NSR_ProcessTx(NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr,
                          NTP_Local_Timestamp *tx_ts, NTP_Packet *message, int length);

/* Initialisation function */
extern void NSR_Initialise(void);

/* Finalisation function */
extern void NSR_Finalise(void);

/* This routine is used to indicate that sources whose IP addresses
   match a particular subnet should be set online or offline.  It returns
   a flag indicating whether any hosts matched the address. */
extern int NSR_SetConnectivity(IPAddr *mask, IPAddr *address, SRC_Connectivity connectivity);

extern int NSR_ModifyMinpoll(IPAddr *address, int new_minpoll);

extern int NSR_ModifyMaxpoll(IPAddr *address, int new_maxpoll);

extern int NSR_ModifyMaxdelay(IPAddr *address, double new_max_delay);

extern int NSR_ModifyMaxdelayratio(IPAddr *address, double new_max_delay_ratio);

extern int NSR_ModifyMaxdelaydevratio(IPAddr *address, double new_max_delay_ratio);

extern int NSR_ModifyMinstratum(IPAddr *address, int new_min_stratum);

extern int NSR_ModifyPolltarget(IPAddr *address, int new_poll_target);

extern int NSR_InitiateSampleBurst(int n_good_samples, int n_total_samples, IPAddr *mask, IPAddr *address);

extern void NSR_ReportSource(RPT_SourceReport *report, struct timespec *now);

extern int NSR_GetNTPReport(RPT_NTPReport *report);

extern void NSR_GetActivityReport(RPT_ActivityReport *report);

#endif /* GOT_NTP_SOURCES_H */
