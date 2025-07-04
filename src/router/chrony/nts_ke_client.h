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

  Header file for the NTS-KE client
  */

#ifndef GOT_NTS_KE_CLIENT_H
#define GOT_NTS_KE_CLIENT_H

#include "addressing.h"
#include "nts_ke.h"

typedef struct NKC_Instance_Record *NKC_Instance;

/* Create a client NTS-KE instance */
extern NKC_Instance NKC_CreateInstance(IPSockAddr *address, const char *name, uint32_t cert_set);

/* Destroy an instance */
extern void NKC_DestroyInstance(NKC_Instance inst);

/* Connect to the server, start an NTS-KE session, send an NTS-KE request, and
   process the response (asynchronously) */
extern int NKC_Start(NKC_Instance inst);

/* Check if the client is still running */
extern int NKC_IsActive(NKC_Instance inst);

/* Get the NTS data if the session was successful */
extern int NKC_GetNtsData(NKC_Instance inst, NKE_Context *context, NKE_Context *alt_context,
                          NKE_Cookie *cookies, int *num_cookies, int max_cookies,
                          IPSockAddr *ntp_address);

/* Get a factor to calculate retry interval (in log2 seconds) */
extern int NKC_GetRetryFactor(NKC_Instance inst);

#endif
