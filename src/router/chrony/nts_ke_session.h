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

  Header file for the NTS-KE session
  */

#ifndef GOT_NTS_KE_SESSION_H
#define GOT_NTS_KE_SESSION_H

#include "nts_ke.h"
#include "siv.h"

typedef struct NKSN_Credentials_Record *NKSN_Credentials;

typedef struct NKSN_Instance_Record *NKSN_Instance;

/* Handler for received NTS-KE messages.  A zero return code stops
   the session. */
typedef int (*NKSN_MessageHandler)(void *arg);

/* Get server or client credentials using a server certificate and key,
   or certificates of trusted CAs.  The credentials may be shared between
   different clients or servers. */
extern NKSN_Credentials NKSN_CreateServerCertCredentials(const char **certs, const char **keys,
                                                         int n_certs_keys);
extern NKSN_Credentials NKSN_CreateClientCertCredentials(const char **certs, uint32_t *ids,
                                                         int n_certs_ids,
                                                         uint32_t trusted_cert_set);

/* Destroy the credentials */
extern void NKSN_DestroyCertCredentials(NKSN_Credentials credentials);

/* Create an instance */
extern NKSN_Instance NKSN_CreateInstance(int server_mode, const char *server_name,
                                         NKSN_MessageHandler handler, void *handler_arg);

/* Destroy an instance */
extern void NKSN_DestroyInstance(NKSN_Instance inst);

/* Start a new NTS-KE session */
extern int NKSN_StartSession(NKSN_Instance inst, int sock_fd, const char *label,
                             NKSN_Credentials credentials, double timeout);

/* Begin an NTS-KE message.  A request should be made right after starting
   the session and response should be made in the message handler. */
extern void NKSN_BeginMessage(NKSN_Instance inst);

/* Add a record to the message */
extern int NKSN_AddRecord(NKSN_Instance inst, int critical, int type,
                          const void *body, int body_length);

/* Terminate the message */
extern int NKSN_EndMessage(NKSN_Instance inst);

/* Get the next record from the received message.  This function should be
   called from the message handler. */
extern int NKSN_GetRecord(NKSN_Instance inst, int *critical, int *type, int *body_length,
                          void *body, int buffer_length);

/* Export NTS keys for a specified algorithm (for compatibility reasons the
   RFC5705 exporter context is allowed to have a different algorithm) */
extern int NKSN_GetKeys(NKSN_Instance inst, SIV_Algorithm algorithm,
                        SIV_Algorithm exporter_algorithm,
                        int next_protocol, NKE_Key *c2s, NKE_Key *s2c);

/* Check if the session has stopped */
extern int NKSN_IsStopped(NKSN_Instance inst);

/* Stop the session */
extern void NKSN_StopSession(NKSN_Instance inst);

/* Get a factor to calculate retry interval (in log2 seconds)
   based on the session state or how it was terminated */
extern int NKSN_GetRetryFactor(NKSN_Instance inst);

#endif
