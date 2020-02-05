/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  This module provides an interface to the system time, and
  insulates the rest of the program from the different way
  that interface has to be done on various operating systems.
  */

#ifndef GOT_LOCAL_H
#define GOT_LOCAL_H

#include "sysincl.h"

/* Read the system clock */
extern void LCL_ReadRawTime(struct timespec *ts);

/* Read the system clock, corrected according to all accumulated
   drifts and uncompensated offsets.

   In a kernel implementation with vernier frequency control (like
   Linux), and if we were to apply offsets by stepping the clock, this
   would be identical to raw time.  In any other case (use of
   adjtime()-like interface to correct offsets, and to adjust the
   frequency), we must correct the raw time to get this value */

extern void LCL_ReadCookedTime(struct timespec *ts, double *err);

/* Convert raw time to cooked. */
extern void LCL_CookTime(struct timespec *raw, struct timespec *cooked, double *err);

/* Read the current offset between the system clock and true time
   (i.e. 'cooked' - 'raw') (in seconds). */

extern void LCL_GetOffsetCorrection(struct timespec *raw, double *correction, double *err);

/* Type of routines that may be invoked as callbacks when there is a
   change to the frequency or offset.

   raw : raw local clock time at which change occurred

   cooked : cooked local time at which change occurred

   dfreq : delta frequency relative to previous value (in terms of
   seconds gained by system clock per unit system clock time)

   doffset : delta offset applied (positive => make local system fast
   by that amount, negative => make it slow by that amount)

   change_type : what type of change is being applied
   
   anything : Passthrough argument from call to registration routine */


typedef enum {
  LCL_ChangeAdjust,
  LCL_ChangeStep,
  LCL_ChangeUnknownStep
} LCL_ChangeType;

typedef void (*LCL_ParameterChangeHandler)
     (struct timespec *raw, struct timespec *cooked,
      double dfreq,
      double doffset,
      LCL_ChangeType change_type,
      void *anything
      );

/* Add a handler.  Then handler MUST NOT deregister itself!!! */
extern void LCL_AddParameterChangeHandler(LCL_ParameterChangeHandler handler, void *anything);

/* Remove a handler */
extern void LCL_RemoveParameterChangeHandler(LCL_ParameterChangeHandler, void *anything);

/* Check if a handler is invoked first when dispatching */
extern int LCL_IsFirstParameterChangeHandler(LCL_ParameterChangeHandler handler);

/* Function type for handlers to be called back when an indeterminate
   offset is introduced into the local time.  This situation occurs
   when the frequency must be adjusted to effect a clock slew and
   there is doubt about one of the endpoints of the interval over
   which the frequency change was applied.It is expected that such
   handlers will add extra dispersion to any existing samples stored
   in their registers. 

   dispersion : The bound on how much error has been introduced in the
   local clock, in seconds.

   anything : passthrough from the registration routine

   */

typedef void (*LCL_DispersionNotifyHandler)(double dispersion, void *anything);

/* Register a handler for being notified of dispersion being added to
   the local clock.  The handler MUST NOT unregister itself!!! */

extern void LCL_AddDispersionNotifyHandler(LCL_DispersionNotifyHandler handler, void *anything);

/* Delete a handler */

extern void LCL_RemoveDispersionNotifyHandler(LCL_DispersionNotifyHandler handler, void *anything);


/* Read the absolute system frequency, relative to the uncompensated
   system.  Returned in units of parts per million.  Thus the result of
   this is how many seconds fast the uncompensated system would be after
   its own time has reached 1 million seconds from the start of the
   measurement.  */
extern double LCL_ReadAbsoluteFrequency(void);

/* Routine to set the absolute frequency.  Only expected to be used
   when either (i) reading the drift from a file at the start of a
   run, or (ii) responsing to a user parameter 'poke'.  This is
   defined in ppm, as for the absolute frequency reading routine. */

extern void LCL_SetAbsoluteFrequency(double afreq);

/* Routine to apply a change of frequency to the local clock.  The
   argument is the estimated gain (positive) or loss (negative) of the
   local clock relative to true time, per unit time of the PREVIOUS
   frequency setting of the local clock.  This is assumed to be based
   on a regression of y=offset v x=cooked local time. */

extern void LCL_AccumulateDeltaFrequency(double dfreq);

/* Routine to apply an offset (in seconds) to the local clock.  The
   argument should be positive to move the clock backwards (i.e. the
   local clock is currently fast of true time), or negative to move it
   forwards (i.e. it is currently slow of true time).  Provided is also
   a suggested correction rate (correction time * offset). */

extern void LCL_AccumulateOffset(double offset, double corr_rate);

/* Routine to apply an immediate offset by doing a sudden step if
   possible. (Intended for use after an initial estimate of offset has
   been obtained, so that we don't end up using adjtime to achieve a
   slew of an hour or something like that). A positive argument means
   the system clock is fast on true time, i.e. it needs to be stepped
   backwards. (Same convention as for AccumulateOffset routine). */

extern int LCL_ApplyStepOffset(double offset);

/* Routine to invoke notify handlers on an unexpected time jump
   in system clock */
extern void LCL_NotifyExternalTimeStep(struct timespec *raw, struct timespec *cooked,
    double offset, double dispersion);

/* Routine to invoke notify handlers on leap second when the system clock
   doesn't correct itself */
extern void LCL_NotifyLeap(int leap);

/* Perform the combination of modifying the frequency and applying
   a slew, in one easy step */
extern void LCL_AccumulateFrequencyAndOffset(double dfreq, double doffset, double corr_rate);

/* Routine to read the system precision as a log to base 2 value. */
extern int LCL_GetSysPrecisionAsLog(void);

/* Routine to read the system precision in terms of the actual time step */
extern double LCL_GetSysPrecisionAsQuantum(void);

/* Routine to read the maximum frequency error of the local clock.  This
   is a frequency stability, not an absolute error. */
extern double LCL_GetMaxClockError(void);

/* Routine to initialise the module (to be called once at program
   start-up) */

extern void LCL_Initialise(void);

/* Routine to finalise the module (to be called once at end of
   run). */
extern void LCL_Finalise(void);

/* Routine to convert the outstanding system clock error to a step and
   apply it, e.g. if the system clock has ended up an hour wrong due
   to a timezone problem. */
extern int LCL_MakeStep(void);

/* Check if the system driver supports leap seconds, i.e. LCL_SetSystemLeap
   does something */
extern int LCL_CanSystemLeap(void);

/* Routine to set the system clock to correct itself for a leap second and also
   set its TAI-UTC offset.  If supported, leap second will be inserted at the
   end of the day if the argument is positive, deleted if negative, and zero
   resets the setting. */
extern void LCL_SetSystemLeap(int leap, int tai_offset);

/* Routine to set a frequency correction (in ppm) that should be applied
   to local clock to compensate for temperature changes.  A positive
   argument means that the clock frequency should be increased. Return the
   actual compensation (may be different from the requested compensation
   due to clamping or rounding). */
extern double LCL_SetTempComp(double comp);

/* Routine to update the synchronisation status in the kernel to allow other
   applications to know if the system clock is synchronised and error bounds */
extern void LCL_SetSyncStatus(int synchronised, double est_error, double max_error);

#endif /* GOT_LOCAL_H */
