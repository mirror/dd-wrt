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

  This is the header file for the module that keeps track of the current
  reference.

  */

#ifndef GOT_REFERENCE_H
#define GOT_REFERENCE_H

#include "sysincl.h"

#include "ntp.h"
#include "reports.h"

/* Leap second handling modes */
typedef enum {
  REF_LeapModeSystem,
  REF_LeapModeSlew,
  REF_LeapModeStep,
  REF_LeapModeIgnore,
} REF_LeapMode;

/* Init function */
extern void REF_Initialise(void);

/* Fini function */
extern void REF_Finalise(void);

typedef enum {
  REF_ModeNormal,
  REF_ModeInitStepSlew,
  REF_ModeUpdateOnce,
  REF_ModePrintOnce,
  REF_ModeIgnore,
} REF_Mode;

/* Set reference update mode */
extern void REF_SetMode(REF_Mode mode);

/* Get reference update mode */
extern REF_Mode REF_GetMode(void);

/* Function type for handlers to be called back when mode ends */
typedef void (*REF_ModeEndHandler)(int result);

/* Set the handler for being notified of mode ending */
extern void REF_SetModeEndHandler(REF_ModeEndHandler handler);

/* Get leap second handling mode */
extern REF_LeapMode REF_GetLeapMode(void);

/* Function which takes a local cooked time and returns the estimated
   time of the reference.  It also returns the other parameters
   required for forming the outgoing NTP packet.

   local_time is the cooked local time returned by the LCL module

   is_synchronised indicates whether we are synchronised to anything
   at the moment.

   leap indicates the current leap status

   stratum is the stratum of this machine, when considered to be sync'd to the
   reference
   
   ref_id is the reference_id of the source

   ref_time is the time at which the we last set the reference source up

   root_delay is the root delay of the sample we are using

   root_dispersion is the root dispersion of the sample we are using, with all the
   skew etc added on.

   */

extern void REF_GetReferenceParams
(
 struct timespec *local_time,
 int *is_synchronised,
 NTP_Leap *leap,
 int *stratum,
 uint32_t *ref_id,
 struct timespec *ref_time,
 double *root_delay,
 double *root_dispersion
);

/* Function called by the clock selection process to register a new
   reference source and its parameters

   stratum is the stratum of the reference

   leap is the leap status read from the source

   ref_id is the reference id of the reference

   ref_time is the time at which the parameters are assumed to be
   correct, in terms of local time

   frequency is the amount of local clock gain relative to the
   reference per unit time interval of the local clock

   skew is the maximum estimated frequency error (so we are within
   [frequency+-skew])

   root_delay is the root delay of the sample we are using

   root_dispersion is the root dispersion of the sample we are using

   */

extern void REF_SetReference
(
 int stratum,
 NTP_Leap leap,
 int combined_sources,
 uint32_t ref_id,
 IPAddr *ref_ip,
 struct timespec *ref_time,
 double offset,
 double offset_sd,
 double frequency,
 double frequency_sd,
 double skew,
 double root_delay,
 double root_dispersion
);

extern void REF_SetManualReference
(
 struct timespec *ref_time,
 double offset,
 double frequency,
 double skew
);

/* Mark the local clock as unsynchronised */
extern void
REF_SetUnsynchronised(void);

/* Return the current stratum of this host or 16 if the host is not
   synchronised */
extern int REF_GetOurStratum(void);

/* Return stratum of the local reference if orphan mode is enabled */
extern int REF_GetOrphanStratum(void);

/* Return the current skew */
extern double REF_GetSkew(void);

/* Modify the setting for the maximum skew we are prepared to allow updates on (in ppm). */
extern void REF_ModifyMaxupdateskew(double new_max_update_skew);

/* Modify makestep settings */
extern void REF_ModifyMakestep(int limit, double threshold);

extern void REF_EnableLocal(int stratum, double distance, int orphan);
extern void REF_DisableLocal(void);

/* Check if current raw or cooked time is close to a leap second
   and is better to discard any measurements */
extern int REF_IsLeapSecondClose(void);

/* Return TAI-UTC offset corresponding to a time in UTC if available */
extern int REF_GetTaiOffset(struct timespec *ts);

extern void REF_GetTrackingReport(RPT_TrackingReport *rep);

#endif /* GOT_REFERENCE_H */
