/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2022
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

  Header file for estimation of quantiles.

  */

#ifndef GOT_QUANTILES_H
#define GOT_QUANTILES_H

typedef struct QNT_Instance_Record *QNT_Instance;

extern QNT_Instance QNT_CreateInstance(int min_k, int max_k, int q, int repeat,
                                       int large_step_delay, double min_step);
extern void QNT_DestroyInstance(QNT_Instance inst);

extern void QNT_Reset(QNT_Instance inst);
extern void QNT_Accumulate(QNT_Instance inst, double value);
extern int QNT_GetMinK(QNT_Instance inst);
extern int QNT_GetMaxK(QNT_Instance inst);
extern double QNT_GetMinStep(QNT_Instance inst);
extern double QNT_GetQuantile(QNT_Instance inst, int k);

#endif
