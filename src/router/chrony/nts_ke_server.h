/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020
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

  Header file for the NTS-KE server
  */

#ifndef GOT_NTS_KE_SERVER_H
#define GOT_NTS_KE_SERVER_H

#include "nts_ke.h"

/* Init and fini functions */
extern void NKS_PreInitialise(uid_t uid, gid_t gid, int scfilter_level);
extern void NKS_Initialise(void);
extern void NKS_Finalise(void);

/* Save the current server keys */
extern void NKS_DumpKeys(void);

/* Reload the keys */
extern void NKS_ReloadKeys(void);

/* Generate an NTS cookie with a given context */
extern int NKS_GenerateCookie(NKE_Context *context, NKE_Cookie *cookie);

/* Validate a cookie and decode the context */
extern int NKS_DecodeCookie(NKE_Cookie *cookie, NKE_Context *context);

#endif
