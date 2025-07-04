/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020-2021, 2024
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

  NTS-KE client
  */

#include "config.h"

#include "sysincl.h"

#include "nts_ke_client.h"

#include "conf.h"
#include "logging.h"
#include "memory.h"
#include "nameserv_async.h"
#include "nts_ke_session.h"
#include "siv.h"
#include "socket.h"
#include "util.h"

#define CLIENT_TIMEOUT 16.0

struct NKC_Instance_Record {
  char *name;
  IPSockAddr address;
  NKSN_Credentials credentials;
  NKSN_Instance session;
  int destroying;
  int got_response;
  int resolving_name;

  int compliant_128gcm;
  NKE_Context context;
  NKE_Context alt_context;
  NKE_Cookie cookies[NKE_MAX_COOKIES];
  int num_cookies;
  char server_name[NKE_MAX_RECORD_BODY_LENGTH + 2];
  IPSockAddr ntp_address;
};

/* ================================================== */

static NKSN_Credentials default_credentials = NULL;
static int default_credentials_refs = 0;

/* ================================================== */

static void
name_resolve_handler(DNS_Status status, int n_addrs, IPAddr *ip_addrs, void *arg)
{
  NKC_Instance inst = arg;
  int i;

  inst->resolving_name = 0;

  if (inst->destroying) {
    Free(inst);
    return;
  }

  if (status != DNS_Success || n_addrs < 1) {
    LOG(LOGS_ERR, "Could not resolve NTP server %s from %s", inst->server_name, inst->name);
    /* Force restart */
    inst->got_response = 0;
    return;
  }

  inst->ntp_address.ip_addr = ip_addrs[0];

  /* Prefer an address in the same family as the NTS-KE server */
  for (i = 0; i < n_addrs; i++) {
    DEBUG_LOG("%s resolved to %s", inst->server_name, UTI_IPToString(&ip_addrs[i]));
    if (ip_addrs[i].family == inst->address.ip_addr.family) {
      inst->ntp_address.ip_addr = ip_addrs[i];
      break;
    }
  }
}

/* ================================================== */

#define MAX_AEAD_ALGORITHMS 4

static int
prepare_request(NKC_Instance inst)
{
  NKSN_Instance session = inst->session;
  uint16_t data[MAX_AEAD_ALGORITHMS];
  int i, aead_algorithm, length;

  NKSN_BeginMessage(session);

  data[0] = htons(NKE_NEXT_PROTOCOL_NTPV4);
  if (!NKSN_AddRecord(session, 1, NKE_RECORD_NEXT_PROTOCOL, data, sizeof (data[0])))
    return 0;

  for (i = length = 0; i < ARR_GetSize(CNF_GetNtsAeads()) && length < MAX_AEAD_ALGORITHMS;
       i++) {
    aead_algorithm = *(int *)ARR_GetElement(CNF_GetNtsAeads(), i);
    if (SIV_GetKeyLength(aead_algorithm) > 0)
      data[length++] = htons(aead_algorithm);
  }
  if (!NKSN_AddRecord(session, 1, NKE_RECORD_AEAD_ALGORITHM, data,
                      length * sizeof (data[0])))
    return 0;

  for (i = 0; i < length; i++) {
    if (data[i] == htons(AEAD_AES_128_GCM_SIV)) {
      if (!NKSN_AddRecord(session, 0, NKE_RECORD_COMPLIANT_128GCM_EXPORT, NULL, 0))
        return 0;
      break;
    }
  }

  if (!NKSN_EndMessage(session))
    return 0;

  return 1;
}

/* ================================================== */

static int
process_response(NKC_Instance inst)
{
  int next_protocol = -1, aead_algorithm = -1, error = 0;
  int i, critical, type, length;
  uint16_t data[NKE_MAX_RECORD_BODY_LENGTH / sizeof (uint16_t)];

  assert(NKE_MAX_COOKIE_LENGTH <= NKE_MAX_RECORD_BODY_LENGTH);
  assert(sizeof (data) % sizeof (uint16_t) == 0);
  assert(sizeof (uint16_t) == 2);

  inst->compliant_128gcm = 0;
  inst->alt_context.algorithm = AEAD_SIV_INVALID;
  inst->num_cookies = 0;
  inst->ntp_address.ip_addr.family = IPADDR_UNSPEC;
  inst->ntp_address.port = 0;
  inst->server_name[0] = '\0';

  while (!error) {
    if (!NKSN_GetRecord(inst->session, &critical, &type, &length, &data, sizeof (data)))
      break;

    if (length > sizeof (data)) {
      DEBUG_LOG("Record too long type=%d length=%d critical=%d", type, length, critical);
      if (critical)
        error = 1;
      continue;
    }

    switch (type) {
      case NKE_RECORD_NEXT_PROTOCOL:
        if (!critical || length != 2 || ntohs(data[0]) != NKE_NEXT_PROTOCOL_NTPV4) {
          DEBUG_LOG("Unexpected NTS-KE next protocol");
          error = 1;
          break;
        }
        next_protocol = NKE_NEXT_PROTOCOL_NTPV4;
        break;
      case NKE_RECORD_AEAD_ALGORITHM:
        if (length != 2) {
          DEBUG_LOG("Unexpected AEAD algorithm");
          error = 1;
          break;
        }
        for (i = 0; i < ARR_GetSize(CNF_GetNtsAeads()); i++) {
          if (ntohs(data[0]) == *(int *)ARR_GetElement(CNF_GetNtsAeads(), i) &&
              SIV_GetKeyLength(ntohs(data[0])) > 0) {
            aead_algorithm = ntohs(data[0]);
            inst->context.algorithm = aead_algorithm;
          }
        }
        if (aead_algorithm < 0) {
          DEBUG_LOG("Unexpected AEAD algorithm");
          error = 1;
        }
        break;
      case NKE_RECORD_COMPLIANT_128GCM_EXPORT:
        if (length != 0) {
          DEBUG_LOG("Non-empty compliant-128gcm record");
          error = 1;
          break;
        }
        DEBUG_LOG("Compliant AES-128-GCM-SIV export");
        inst->compliant_128gcm = 1;
        break;
      case NKE_RECORD_ERROR:
        if (length == 2)
          DEBUG_LOG("NTS-KE error %d", ntohs(data[0]));
        error = 1;
        break;
      case NKE_RECORD_WARNING:
        if (length == 2)
          DEBUG_LOG("NTS-KE warning %d", ntohs(data[0]));
        error = 1;
        break;
      case NKE_RECORD_COOKIE:
        DEBUG_LOG("Got cookie length=%d", length);

        if (length < 1 || length > NKE_MAX_COOKIE_LENGTH || length % 4 != 0 ||
            inst->num_cookies >= NKE_MAX_COOKIES) {
          DEBUG_LOG("Unexpected length/cookie");
          break;
        }

        assert(NKE_MAX_COOKIE_LENGTH == sizeof (inst->cookies[inst->num_cookies].cookie));
        assert(NKE_MAX_COOKIES == sizeof (inst->cookies) /
                                  sizeof (inst->cookies[inst->num_cookies]));
        inst->cookies[inst->num_cookies].length = length;
        memcpy(inst->cookies[inst->num_cookies].cookie, data, length);

        inst->num_cookies++;
        break;
      case NKE_RECORD_NTPV4_SERVER_NEGOTIATION:
        if (length < 1 || length >= sizeof (inst->server_name)) {
          DEBUG_LOG("Invalid server name");
          error = 1;
          break;
        }

        memcpy(inst->server_name, data, length);
        inst->server_name[length] = '\0';

        /* Make sure the name is printable and has no spaces */
        for (i = 0; i < length && isgraph((unsigned char)inst->server_name[i]); i++)
          ;
        if (i != length) {
          DEBUG_LOG("Invalid server name");
          error = 1;
          break;
        }

        DEBUG_LOG("Negotiated server %s", inst->server_name);
        break;
      case NKE_RECORD_NTPV4_PORT_NEGOTIATION:
        if (length != 2) {
          DEBUG_LOG("Invalid port");
          error = 1;
          break;
        }
        inst->ntp_address.port = ntohs(data[0]);
        DEBUG_LOG("Negotiated port %d", inst->ntp_address.port);
        break;
      default:
        DEBUG_LOG("Unknown record type=%d length=%d critical=%d", type, length, critical);
        if (critical)
          error = 1;
    }
  }

  DEBUG_LOG("NTS-KE response: error=%d next=%d aead=%d",
            error, next_protocol, aead_algorithm);

  if (error || inst->num_cookies == 0 ||
      next_protocol != NKE_NEXT_PROTOCOL_NTPV4 ||
      aead_algorithm < 0)
    return 0;

  return 1;
}

/* ================================================== */

static int
handle_message(void *arg)
{
  SIV_Algorithm exporter_algorithm;
  NKC_Instance inst = arg;

  if (!process_response(inst)) {
    LOG(LOGS_ERR, "Received invalid NTS-KE response from %s", inst->name);
    return 0;
  }

  exporter_algorithm = inst->context.algorithm;

  /* With AES-128-GCM-SIV, set the algorithm ID in the RFC5705 key exporter
     context incorrectly for compatibility with older chrony servers unless
     the server confirmed support for the compliant context.  Generate both
     sets of keys in case the server uses the compliant context, but does not
     support the negotiation record, assuming it will respond with an NTS NAK
     to a request authenticated with the noncompliant key. */
  if (exporter_algorithm == AEAD_AES_128_GCM_SIV && !inst->compliant_128gcm) {
    inst->alt_context.algorithm = inst->context.algorithm;
    if (!NKSN_GetKeys(inst->session, inst->alt_context.algorithm, exporter_algorithm,
                      NKE_NEXT_PROTOCOL_NTPV4, &inst->alt_context.c2s, &inst->alt_context.s2c))
      return 0;

    exporter_algorithm = AEAD_AES_SIV_CMAC_256;
  }

  if (!NKSN_GetKeys(inst->session, inst->context.algorithm, exporter_algorithm,
                    NKE_NEXT_PROTOCOL_NTPV4, &inst->context.c2s, &inst->context.s2c))
    return 0;

  if (inst->server_name[0] != '\0') {
    if (inst->resolving_name)
      return 0;
    if (!UTI_StringToIP(inst->server_name, &inst->ntp_address.ip_addr)) {
      int length = strlen(inst->server_name);

      /* Add a trailing dot if not present to force the name to be
         resolved as a fully qualified domain name */
      if (length < 1 || length + 1 >= sizeof (inst->server_name))
        return 0;
      if (inst->server_name[length - 1] != '.') {
        inst->server_name[length] = '.';
        inst->server_name[length + 1] = '\0';
      }

      DNS_Name2IPAddressAsync(inst->server_name, name_resolve_handler, inst);
      inst->resolving_name = 1;
    }
  }

  inst->got_response = 1;

  return 1;
}

/* ================================================== */

NKC_Instance
NKC_CreateInstance(IPSockAddr *address, const char *name, uint32_t cert_set)
{
  const char **trusted_certs;
  uint32_t *certs_ids;
  NKC_Instance inst;
  int n_certs;

  inst = MallocNew(struct NKC_Instance_Record);

  inst->address = *address;
  inst->name = Strdup(name);
  inst->session = NKSN_CreateInstance(0, inst->name, handle_message, inst);
  inst->resolving_name = 0;
  inst->destroying = 0;
  inst->got_response = 0;

  n_certs = CNF_GetNtsTrustedCertsPaths(&trusted_certs, &certs_ids);

  /* Share the credentials among clients using the default set of trusted
     certificates, which likely contains most certificates */
  if (cert_set == 0) {
    if (!default_credentials)
      default_credentials = NKSN_CreateClientCertCredentials(trusted_certs, certs_ids,
                                                             n_certs, cert_set);
    inst->credentials = default_credentials;
    if (default_credentials)
      default_credentials_refs++;
  } else {
    inst->credentials = NKSN_CreateClientCertCredentials(trusted_certs, certs_ids,
                                                         n_certs, cert_set);
  }

  return inst;
}

/* ================================================== */

void
NKC_DestroyInstance(NKC_Instance inst)
{
  NKSN_DestroyInstance(inst->session);

  Free(inst->name);

  if (inst->credentials) {
    if (inst->credentials == default_credentials) {
      default_credentials_refs--;
      if (default_credentials_refs <= 0) {
        NKSN_DestroyCertCredentials(default_credentials);
        default_credentials = NULL;
      }
    } else {
      NKSN_DestroyCertCredentials(inst->credentials);
    }
  }

  /* If the asynchronous resolver is running, let the handler free
     the instance later */
  if (inst->resolving_name) {
    inst->destroying = 1;
    return;
  }

  Free(inst);
}

/* ================================================== */

int
NKC_Start(NKC_Instance inst)
{
  IPSockAddr local_addr;
  char label[512], *iface;
  int sock_fd;

  assert(!NKC_IsActive(inst));

  inst->got_response = 0;

  if (!inst->credentials) {
    DEBUG_LOG("Missing client credentials");
    return 0;
  }

  /* Don't try to connect if missing the algorithm which all servers
     are required to support */
  if (SIV_GetKeyLength(AEAD_AES_SIV_CMAC_256) <= 0) {
    LOG(LOGS_ERR, "Missing AES-SIV-CMAC-256");
    return 0;
  }

  /* Follow the bindacqaddress and bindacqdevice settings */
  CNF_GetBindAcquisitionAddress(inst->address.ip_addr.family, &local_addr.ip_addr);
  local_addr.port = 0;
  iface = CNF_GetBindAcquisitionInterface();

  /* Make a label containing both the address and name of the server */
  if (snprintf(label, sizeof (label), "%s (%s)",
               UTI_IPSockAddrToString(&inst->address), inst->name) >= sizeof (label))
    ;

  sock_fd = SCK_OpenTcpSocket(&inst->address, &local_addr, iface, 0);
  if (sock_fd < 0) {
    LOG(LOGS_ERR, "Could not connect to %s", label);
    return 0;
  }

  /* Start an NTS-KE session */
  if (!NKSN_StartSession(inst->session, sock_fd, label, inst->credentials, CLIENT_TIMEOUT)) {
    SCK_CloseSocket(sock_fd);
    return 0;
  }

  /* Send a request */
  if (!prepare_request(inst)) {
    DEBUG_LOG("Could not prepare NTS-KE request");
    NKSN_StopSession(inst->session);
    return 0;
  }

  return 1;
}

/* ================================================== */

int
NKC_IsActive(NKC_Instance inst)
{
  return !NKSN_IsStopped(inst->session) || inst->resolving_name;
}

/* ================================================== */

int
NKC_GetNtsData(NKC_Instance inst, NKE_Context *context, NKE_Context *alt_context,
               NKE_Cookie *cookies, int *num_cookies, int max_cookies,
               IPSockAddr *ntp_address)
{
  int i;

  if (!inst->got_response || inst->resolving_name)
    return 0;

  *context = inst->context;
  *alt_context = inst->alt_context;

  for (i = 0; i < inst->num_cookies && i < max_cookies; i++)
    cookies[i] = inst->cookies[i];
  *num_cookies = i;

  *ntp_address = inst->ntp_address;

  return 1;
}

/* ================================================== */

int
NKC_GetRetryFactor(NKC_Instance inst)
{
  return NKSN_GetRetryFactor(inst->session);
}
