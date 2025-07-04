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

  This is the header for the module that manages the collection of all
  sources that we are making measurements from.  This include all NTP
  servers & peers, locally connected reference sources, eye/wristwatch
  drivers etc */

#ifndef GOT_SOURCES_H
#define GOT_SOURCES_H

#include "sysincl.h"

#include "ntp.h"
#include "reports.h"
#include "sourcestats.h"

/* Size of the source reachability register */
#define SOURCE_REACH_BITS 8

/* This datatype is used to hold information about sources.  The
   instance must be passed when calling many of the interface
   functions */

typedef struct SRC_Instance_Record *SRC_Instance;

/* Initialisation function */
extern void SRC_Initialise(void);

/* Finalisation function */
extern void SRC_Finalise(void);

/* Modes for selecting NTP sources based on their authentication status */
typedef enum {
  SRC_AUTHSELECT_IGNORE,
  SRC_AUTHSELECT_MIX,
  SRC_AUTHSELECT_PREFER,
  SRC_AUTHSELECT_REQUIRE,
} SRC_AuthSelectMode;

typedef enum {
  SRC_NTP,                      /* NTP client/peer */
  SRC_REFCLOCK                  /* Rerefence clock */
} SRC_Type;

/* Function to create a new instance.  This would be called by one of
   the individual source-type instance creation routines. */

extern SRC_Instance SRC_CreateNewInstance(uint32_t ref_id, SRC_Type type, int authenticated,
                                          int sel_options, IPAddr *addr, int min_samples,
                                          int max_samples, double min_delay, double asymmetry);

/* Function to get rid of a source when it is being unconfigured.
   This may cause the current reference source to be reselected, if this
   was the reference source or contributed significantly to a
   falseticker decision. */

extern void SRC_DestroyInstance(SRC_Instance instance);

/* Function to reset a source */
extern void SRC_ResetInstance(SRC_Instance instance);

/* Function to change the sources's reference ID and IP address */
extern void SRC_SetRefid(SRC_Instance instance, uint32_t ref_id, IPAddr *addr);

/* Function to get access to the sourcestats instance */
extern SST_Stats SRC_GetSourcestats(SRC_Instance instance);

/* Function to update the stratum and leap status of the source */
extern void SRC_UpdateStatus(SRC_Instance instance, int stratum, NTP_Leap leap);

/* Function to accumulate a new sample from the source */
extern void SRC_AccumulateSample(SRC_Instance instance, NTP_Sample *sample);

/* This routine sets the source as receiving reachability updates */
extern void SRC_SetActive(SRC_Instance inst);

/* This routine sets the source as not receiving reachability updates */
extern void SRC_UnsetActive(SRC_Instance inst);

/* This routine updates the reachability register */
extern void SRC_UpdateReachability(SRC_Instance inst, int reachable);

/* This routine marks the source unreachable */
extern void SRC_ResetReachability(SRC_Instance inst);

/* This routine is used to select the best source from amongst those
   we currently have valid data on, and use it as the tracking base
   for the local time.  Updates are made to the local reference only
   when the selected source was updated (set as updated_inst) since
   the last reference update.  This avoids updating the frequency
   tracking for every sample from other sources - only the ones from
   the selected reference make a difference. */
extern void SRC_SelectSource(SRC_Instance updated_inst);

/* Force reselecting the best source */
extern void SRC_ReselectSource(void);

/* Set reselect distance */
extern void SRC_SetReselectDistance(double distance);

extern void SRC_DumpSources(void);
extern void SRC_ReloadSources(void);
extern void SRC_RemoveDumpFiles(void);

extern void SRC_ResetSources(void);

extern int SRC_IsSyncPeer(SRC_Instance inst);
extern int SRC_IsReachable(SRC_Instance inst);
extern int SRC_ReadNumberOfSources(void);
extern int SRC_ActiveSources(void);

/* Modify selection options of an NTP source specified by address, or
   refclock specified by its reference ID */
extern int SRC_ModifySelectOptions(IPAddr *ip, uint32_t ref_id, int options, int mask);

extern int SRC_ReportSource(int index, RPT_SourceReport *report, struct timespec *now);
extern int SRC_ReportSourcestats(int index, RPT_SourcestatsReport *report, struct timespec *now);
extern int SRC_GetSelectReport(int index, RPT_SelectReport *report);

extern SRC_Type SRC_GetType(int index);

#endif /* GOT_SOURCES_H */
