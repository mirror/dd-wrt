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

  Client NTS-NTP authentication
  */

#include "config.h"

#include "sysincl.h"

#include "nts_ntp_client.h"

#include "conf.h"
#include "logging.h"
#include "memory.h"
#include "ntp.h"
#include "ntp_ext.h"
#include "ntp_sources.h"
#include "nts_ke_client.h"
#include "nts_ntp.h"
#include "nts_ntp_auth.h"
#include "sched.h"
#include "siv.h"
#include "util.h"

/* Maximum length of all cookies to avoid IP fragmentation */
#define MAX_TOTAL_COOKIE_LENGTH (8 * 108)

/* Retry interval for NTS-KE start (which doesn't generate network traffic) */
#define RETRY_INTERVAL_KE_START 2.0

/* Magic string of files containing keys and cookies */
#define DUMP_IDENTIFIER "NNC0\n"

struct NNC_Instance_Record {
  /* Address of NTS-KE server */
  IPSockAddr nts_address;
  /* Hostname or IP address for certificate verification */
  char *name;
  /* ID of trusted certificates */
  uint32_t cert_set;
  /* Configured NTP port */
  uint16_t default_ntp_port;
  /* Address of NTP server (can be negotiated in NTS-KE) */
  IPSockAddr ntp_address;

  NKC_Instance nke;
  SIV_Instance siv;

  int nke_attempts;
  double next_nke_attempt;
  double last_nke_success;

  NKE_Context context;
  NKE_Context alt_context;
  unsigned int context_id;
  NKE_Cookie cookies[NTS_MAX_COOKIES];
  int num_cookies;
  int cookie_index;
  int auth_ready;
  int nak_response;
  int ok_response;
  unsigned char nonce[NTS_MIN_UNPADDED_NONCE_LENGTH];
  unsigned char uniq_id[NTS_MIN_UNIQ_ID_LENGTH];
};

/* ================================================== */

static void save_cookies(NNC_Instance inst);
static void load_cookies(NNC_Instance inst);

/* ================================================== */

static void
reset_instance(NNC_Instance inst)
{
  if (inst->nke)
    NKC_DestroyInstance(inst->nke);
  inst->nke = NULL;
  if (inst->siv)
    SIV_DestroyInstance(inst->siv);
  inst->siv = NULL;

  inst->nke_attempts = 0;
  inst->next_nke_attempt = 0.0;
  inst->last_nke_success = 0.0;

  memset(&inst->context, 0, sizeof (inst->context));
  memset(&inst->alt_context, 0, sizeof (inst->alt_context));
  inst->context_id = 0;
  memset(inst->cookies, 0, sizeof (inst->cookies));
  inst->num_cookies = 0;
  inst->cookie_index = 0;
  inst->auth_ready = 0;
  inst->nak_response = 0;
  inst->ok_response = 1;
  memset(inst->nonce, 0, sizeof (inst->nonce));
  memset(inst->uniq_id, 0, sizeof (inst->uniq_id));
}

/* ================================================== */

NNC_Instance
NNC_CreateInstance(IPSockAddr *nts_address, const char *name, uint32_t cert_set, uint16_t ntp_port)
{
  NNC_Instance inst;

  inst = MallocNew(struct NNC_Instance_Record);

  inst->nts_address = *nts_address;
  inst->name = Strdup(name);
  inst->cert_set = cert_set;
  inst->default_ntp_port = ntp_port;
  inst->ntp_address.ip_addr = nts_address->ip_addr;
  inst->ntp_address.port = ntp_port;
  inst->siv = NULL;
  inst->nke = NULL;

  reset_instance(inst);

  /* Try to reload saved keys and cookies */
  load_cookies(inst);

  return inst;
}

/* ================================================== */

void
NNC_DestroyInstance(NNC_Instance inst)
{
  save_cookies(inst);

  reset_instance(inst);

  Free(inst->name);
  Free(inst);
}

/* ================================================== */

static int
check_cookies(NNC_Instance inst)
{
  /* Force a new NTS-KE session if a NAK was received without a valid response,
     or the keys encrypting the cookies need to be refreshed */
  if (inst->num_cookies > 0 &&
      ((inst->nak_response && !inst->ok_response) ||
       SCH_GetLastEventMonoTime() - inst->last_nke_success > CNF_GetNtsRefresh())) {

    /* Before dropping the cookies, check whether there is an alternate set of
       keys available (exported with the compliant context for AES-128-GCM-SIV)
       and the NAK was the only valid response after the last NTS-KE session,
       indicating we use incorrect keys and switching to the other set of keys
       for the following NTP requests might work */
    if (inst->alt_context.algorithm != AEAD_SIV_INVALID &&
        inst->alt_context.algorithm == inst->context.algorithm &&
        inst->nke_attempts > 0 && inst->nak_response && !inst->ok_response) {
      inst->context = inst->alt_context;
      inst->alt_context.algorithm = AEAD_SIV_INVALID;
      DEBUG_LOG("Switched to compliant keys");
      return 1;
    }

    inst->num_cookies = 0;
    DEBUG_LOG("Dropped cookies");
  }

  return inst->num_cookies > 0;
}

/* ================================================== */

static int
set_ntp_address(NNC_Instance inst, NTP_Remote_Address *negotiated_address)
{
  NTP_Remote_Address old_address, new_address;

  old_address = inst->ntp_address;
  new_address = *negotiated_address;

  if (new_address.ip_addr.family == IPADDR_UNSPEC)
    new_address.ip_addr = inst->nts_address.ip_addr;
  if (new_address.port == 0)
    new_address.port = inst->default_ntp_port;

  if (UTI_CompareIPs(&old_address.ip_addr, &new_address.ip_addr, NULL) == 0 &&
      old_address.port == new_address.port)
    /* Nothing to do */
    return 1;

  if (NSR_UpdateSourceNtpAddress(&old_address, &new_address) != NSR_Success) {
    LOG(LOGS_ERR, "Could not change %s to negotiated address %s",
        UTI_IPToString(&old_address.ip_addr), UTI_IPToString(&new_address.ip_addr));
    return 0;
  }

  inst->ntp_address = new_address;

  return 1;
}

/* ================================================== */

static void
update_next_nke_attempt(NNC_Instance inst, int failed_start, double now)
{
  int factor, interval;

  if (failed_start) {
    inst->next_nke_attempt = now + RETRY_INTERVAL_KE_START;
    return;
  }

  if (!inst->nke)
    return;

  factor = NKC_GetRetryFactor(inst->nke);
  interval = MIN(factor + inst->nke_attempts - 1, NKE_MAX_RETRY_INTERVAL2);
  inst->next_nke_attempt = now + UTI_Log2ToDouble(interval);
}

/* ================================================== */

static int
get_cookies(NNC_Instance inst)
{
  NTP_Remote_Address ntp_address;
  int got_data, failed_start = 0;
  double now;

  assert(inst->num_cookies == 0);

  now = SCH_GetLastEventMonoTime();

  /* Create and start a new NTS-KE session if not already present */
  if (!inst->nke) {
    if (now < inst->next_nke_attempt) {
      DEBUG_LOG("Limiting NTS-KE request rate (%f seconds)",
                inst->next_nke_attempt - now);
      return 0;
    }

    inst->nke = NKC_CreateInstance(&inst->nts_address, inst->name, inst->cert_set);

    inst->nke_attempts++;

    if (!NKC_Start(inst->nke))
      failed_start = 1;
  }

  update_next_nke_attempt(inst, failed_start, now);

  /* Wait until the session stops */
  if (NKC_IsActive(inst->nke))
    return 0;

  assert(sizeof (inst->cookies) / sizeof (inst->cookies[0]) == NTS_MAX_COOKIES);

  /* Get the new keys, cookies and NTP address if the session was successful */
  got_data = NKC_GetNtsData(inst->nke, &inst->context, &inst->alt_context,
                            inst->cookies, &inst->num_cookies, NTS_MAX_COOKIES,
                            &ntp_address);

  NKC_DestroyInstance(inst->nke);
  inst->nke = NULL;

  if (!got_data)
    return 0;

  if (inst->siv)
    SIV_DestroyInstance(inst->siv);
  inst->siv = NULL;

  inst->context_id++;

  /* Force a new session if the NTP address is used by another source, with
     an expectation that it will eventually get a non-conflicting address */
  if (!set_ntp_address(inst, &ntp_address)) {
    inst->num_cookies = 0;
    return 0;
  }

  inst->last_nke_success = now;
  inst->cookie_index = 0;

  return 1;
}

/* ================================================== */

int
NNC_PrepareForAuth(NNC_Instance inst)
{
  inst->auth_ready = 0;

  /* Prepare data for the next request and invalidate any responses to the
     previous request */
  UTI_GetRandomBytes(inst->uniq_id, sizeof (inst->uniq_id));
  UTI_GetRandomBytes(inst->nonce, sizeof (inst->nonce));

  /* Get new cookies if there are not any, or they are no longer usable */
  if (!check_cookies(inst)) {
    if (!get_cookies(inst))
      return 0;
  }

  inst->nak_response = 0;

  if (!inst->siv)
    inst->siv = SIV_CreateInstance(inst->context.algorithm);

  if (!inst->siv ||
      !SIV_SetKey(inst->siv, inst->context.c2s.key, inst->context.c2s.length)) {
    DEBUG_LOG("Could not set SIV key");
    return 0;
  }

  inst->auth_ready = 1;

  return 1;
}

/* ================================================== */

int
NNC_GenerateRequestAuth(NNC_Instance inst, NTP_Packet *packet,
                        NTP_PacketInfo *info)
{
  NKE_Cookie *cookie;
  int i, req_cookies;
  void *ef_body;

  if (!inst->auth_ready)
    return 0;

  inst->auth_ready = 0;

  if (inst->num_cookies <= 0 || !inst->siv)
    return 0;

  if (info->mode != MODE_CLIENT)
    return 0;

  cookie = &inst->cookies[inst->cookie_index];
  inst->num_cookies--;
  inst->cookie_index = (inst->cookie_index + 1) % NTS_MAX_COOKIES;

  req_cookies = MIN(NTS_MAX_COOKIES - inst->num_cookies,
                    MAX_TOTAL_COOKIE_LENGTH / (cookie->length + 4));

  if (!NEF_AddField(packet, info, NTP_EF_NTS_UNIQUE_IDENTIFIER,
                    inst->uniq_id, sizeof (inst->uniq_id)))
    return 0;

  if (!NEF_AddField(packet, info, NTP_EF_NTS_COOKIE,
                    cookie->cookie, cookie->length))
    return 0;

  for (i = 0; i < req_cookies - 1; i++) {
    if (!NEF_AddBlankField(packet, info, NTP_EF_NTS_COOKIE_PLACEHOLDER,
                           cookie->length, &ef_body))
      return 0;
    memset(ef_body, 0, cookie->length);
  }

  if (!NNA_GenerateAuthEF(packet, info, inst->siv, inst->nonce, sizeof (inst->nonce),
                          (const unsigned char *)"", 0, NTP_MAX_V4_MAC_LENGTH + 4))
    return 0;

  inst->ok_response = 0;

  return 1;
}

/* ================================================== */

static int
parse_encrypted_efs(NNC_Instance inst, unsigned char *plaintext, int length)
{
  int ef_length, parsed;

  for (parsed = 0; parsed < length; parsed += ef_length) {
    if (!NEF_ParseSingleField(plaintext, length, parsed, &ef_length, NULL, NULL, NULL)) {
      DEBUG_LOG("Could not parse encrypted EF");
      return 0;
    }
  }

  return 1;
}

/* ================================================== */

static int
extract_cookies(NNC_Instance inst, unsigned char *plaintext, int length)
{
  int ef_type, ef_body_length, ef_length, parsed, index, acceptable, saved;
  void *ef_body;

  acceptable = saved = 0;

  for (parsed = 0; parsed < length; parsed += ef_length) {
    if (!NEF_ParseSingleField(plaintext, length, parsed,
                              &ef_length, &ef_type, &ef_body, &ef_body_length))
      return 0;

    if (ef_type != NTP_EF_NTS_COOKIE)
      continue;

    if (ef_length < NTP_MIN_EF_LENGTH || ef_body_length > sizeof (inst->cookies[0].cookie)) {
      DEBUG_LOG("Unexpected cookie length %d", ef_body_length);
      continue;
    }

    acceptable++;

    if (inst->num_cookies >= NTS_MAX_COOKIES)
      continue;

    index = (inst->cookie_index + inst->num_cookies) % NTS_MAX_COOKIES;
    assert(index >= 0 && index < NTS_MAX_COOKIES);
    assert(sizeof (inst->cookies) / sizeof (inst->cookies[0]) == NTS_MAX_COOKIES);

    memcpy(inst->cookies[index].cookie, ef_body, ef_body_length);
    inst->cookies[index].length = ef_body_length;
    inst->num_cookies++;

    saved++;
  }

  DEBUG_LOG("Extracted %d cookies (saved %d)", acceptable, saved);

  return acceptable > 0;
}

/* ================================================== */

int
NNC_CheckResponseAuth(NNC_Instance inst, NTP_Packet *packet,
                      NTP_PacketInfo *info)
{
  int ef_type, ef_body_length, ef_length, parsed, plaintext_length;
  int has_valid_uniq_id = 0, has_valid_auth = 0;
  unsigned char plaintext[NTP_MAX_EXTENSIONS_LENGTH];
  void *ef_body;

  if (info->ext_fields == 0 || info->mode != MODE_SERVER)
    return 0;

  /* Accept at most one response per request */
  if (inst->ok_response || inst->auth_ready)
    return 0;

  if (!inst->siv ||
      !SIV_SetKey(inst->siv, inst->context.s2c.key, inst->context.s2c.length)) {
    DEBUG_LOG("Could not set SIV key");
    return 0;
  }

  for (parsed = NTP_HEADER_LENGTH; parsed < info->length; parsed += ef_length) {
    if (!NEF_ParseField(packet, info->length, parsed,
                        &ef_length, &ef_type, &ef_body, &ef_body_length))
      /* This is not expected as the packet already passed parsing */
      return 0;

    switch (ef_type) {
      case NTP_EF_NTS_UNIQUE_IDENTIFIER:
        if (ef_body_length != sizeof (inst->uniq_id) ||
            memcmp(ef_body, inst->uniq_id, sizeof (inst->uniq_id)) != 0) {
          DEBUG_LOG("Invalid uniq id");
          return 0;
        }
        has_valid_uniq_id = 1;
        break;
      case NTP_EF_NTS_COOKIE:
        DEBUG_LOG("Unencrypted cookie");
        break;
      case NTP_EF_NTS_AUTH_AND_EEF:
        if (parsed + ef_length != info->length) {
          DEBUG_LOG("Auth not last EF");
          return 0;
        }

        if (!NNA_DecryptAuthEF(packet, info, inst->siv, parsed,
                               plaintext, sizeof (plaintext), &plaintext_length))
          return 0;

        if (!parse_encrypted_efs(inst, plaintext, plaintext_length))
          return 0;

        has_valid_auth = 1;
        break;
      default:
        break;
    }
  }

  if (!has_valid_uniq_id || !has_valid_auth) {
    if (has_valid_uniq_id && packet->stratum == NTP_INVALID_STRATUM &&
        ntohl(packet->reference_id) == NTP_KOD_NTS_NAK) {
      DEBUG_LOG("NTS NAK");
      inst->nak_response = 1;
      return 0;
    }

    DEBUG_LOG("Missing NTS EF");
    return 0;
  }

  if (!extract_cookies(inst, plaintext, plaintext_length))
    return 0;

  inst->ok_response = 1;

  /* At this point we know the client interoperates with the server.  Allow a
     new NTS-KE session to be started as soon as the cookies run out. */
  inst->nke_attempts = 0;
  inst->next_nke_attempt = 0.0;
  inst->alt_context.algorithm = AEAD_SIV_INVALID;

  return 1;
}

/* ================================================== */

void
NNC_ChangeAddress(NNC_Instance inst, IPAddr *address)
{
  save_cookies(inst);

  inst->nts_address.ip_addr = *address;
  inst->ntp_address.ip_addr = *address;

  reset_instance(inst);

  DEBUG_LOG("NTS reset");

  load_cookies(inst);
}

/* ================================================== */

static void
save_cookies(NNC_Instance inst)
{
  char buf[2 * NKE_MAX_COOKIE_LENGTH + 2], *dump_dir, *filename;
  struct timespec now;
  double context_time;
  FILE *f;
  int i;

  if (inst->num_cookies < 1 || !UTI_IsIPReal(&inst->nts_address.ip_addr))
    return;

  dump_dir = CNF_GetNtsDumpDir();
  if (!dump_dir)
    return;

  filename = UTI_IPToString(&inst->nts_address.ip_addr);

  f = UTI_OpenFile(dump_dir, filename, ".tmp", 'w', 0600);
  if (!f)
    return;

  SCH_GetLastEventTime(&now, NULL, NULL);
  context_time = inst->last_nke_success - SCH_GetLastEventMonoTime();
  context_time += UTI_TimespecToDouble(&now);

  if (fprintf(f, "%s%s\n%.1f\n%s %d\n%u %d ",
              DUMP_IDENTIFIER, inst->name, context_time,
              UTI_IPToString(&inst->ntp_address.ip_addr), inst->ntp_address.port,
              inst->context_id, (int)inst->context.algorithm) < 0 ||
      !UTI_BytesToHex(inst->context.s2c.key, inst->context.s2c.length, buf, sizeof (buf)) ||
      fprintf(f, "%s ", buf) < 0 ||
      !UTI_BytesToHex(inst->context.c2s.key, inst->context.c2s.length, buf, sizeof (buf)) ||
      fprintf(f, "%s\n", buf) < 0)
    goto error;

  for (i = 0; i < inst->num_cookies; i++) {
    if (!UTI_BytesToHex(inst->cookies[i].cookie, inst->cookies[i].length, buf, sizeof (buf)) ||
        fprintf(f, "%s\n", buf) < 0)
      goto error;
  }

  fclose(f);

  if (!UTI_RenameTempFile(dump_dir, filename, ".tmp", ".nts"))
    ;
  return;

error:
  DEBUG_LOG("Could not %s cookies for %s", "save", filename);
  fclose(f);

  if (!UTI_RemoveFile(dump_dir, filename, ".nts"))
    ;
}

/* ================================================== */

#define MAX_WORDS 4

static void
load_cookies(NNC_Instance inst)
{
  char line[2 * NKE_MAX_COOKIE_LENGTH + 2], *dump_dir, *filename, *words[MAX_WORDS];
  unsigned int context_id;
  int i, algorithm, port;
  double context_time;
  struct timespec now;
  IPSockAddr ntp_addr;
  FILE *f;

  dump_dir = CNF_GetNtsDumpDir();
  if (!dump_dir)
    return;

  filename = UTI_IPToString(&inst->nts_address.ip_addr);

  f = UTI_OpenFile(dump_dir, filename, ".nts", 'r', 0);
  if (!f)
    return;

  /* Don't load this file again */
  if (!UTI_RemoveFile(dump_dir, filename, ".nts"))
    ;

  if (inst->siv)
    SIV_DestroyInstance(inst->siv);
  inst->siv = NULL;

  if (!fgets(line, sizeof (line), f) || strcmp(line, DUMP_IDENTIFIER) != 0 ||
      !fgets(line, sizeof (line), f) || UTI_SplitString(line, words, MAX_WORDS) != 1 ||
        strcmp(words[0], inst->name) != 0 ||
      !fgets(line, sizeof (line), f) || UTI_SplitString(line, words, MAX_WORDS) != 1 ||
        sscanf(words[0], "%lf", &context_time) != 1 ||
      !fgets(line, sizeof (line), f) || UTI_SplitString(line, words, MAX_WORDS) != 2 ||
        !UTI_StringToIP(words[0], &ntp_addr.ip_addr) || sscanf(words[1], "%d", &port) != 1 ||
      !fgets(line, sizeof (line), f) || UTI_SplitString(line, words, MAX_WORDS) != 4 ||
        sscanf(words[0], "%u", &context_id) != 1 || sscanf(words[1], "%d", &algorithm) != 1)
    goto error;

  inst->alt_context.algorithm = AEAD_SIV_INVALID;
  inst->context.algorithm = algorithm;
  inst->context.s2c.length = UTI_HexToBytes(words[2], inst->context.s2c.key,
                                            sizeof (inst->context.s2c.key));
  inst->context.c2s.length = UTI_HexToBytes(words[3], inst->context.c2s.key,
                                            sizeof (inst->context.c2s.key));

  if (inst->context.s2c.length != SIV_GetKeyLength(algorithm) ||
      inst->context.s2c.length <= 0 ||
      inst->context.c2s.length != inst->context.s2c.length)
    goto error;

  for (i = 0; i < NTS_MAX_COOKIES && fgets(line, sizeof (line), f); i++) {
    if (UTI_SplitString(line, words, MAX_WORDS) != 1)
      goto error;

    inst->cookies[i].length = UTI_HexToBytes(words[0], inst->cookies[i].cookie,
                                             sizeof (inst->cookies[i].cookie));
    if (inst->cookies[i].length == 0)
      goto error;
  }

  inst->num_cookies = i;

  ntp_addr.port = port;
  if (!set_ntp_address(inst, &ntp_addr))
    goto error;

  SCH_GetLastEventTime(&now, NULL, NULL);
  context_time -= UTI_TimespecToDouble(&now);
  if (context_time > 0)
    context_time = 0;
  inst->last_nke_success = context_time + SCH_GetLastEventMonoTime();
  inst->context_id = context_id;

  fclose(f);

  DEBUG_LOG("Loaded %d cookies for %s", i, filename);
  return;

error:
  DEBUG_LOG("Could not %s cookies for %s", "load", filename);
  fclose(f);

  memset(&inst->context, 0, sizeof (inst->context));
  memset(&inst->alt_context, 0, sizeof (inst->alt_context));
  inst->num_cookies = 0;
}

/* ================================================== */

void
NNC_DumpData(NNC_Instance inst)
{
  save_cookies(inst);
}

/* ================================================== */

void
NNC_GetReport(NNC_Instance inst, RPT_AuthReport *report)
{
  report->key_id = inst->context_id;
  report->key_type = inst->context.algorithm;
  report->key_length = 8 * inst->context.s2c.length;
  report->ke_attempts = inst->nke_attempts;
  if (report->key_length > 0)
    report->last_ke_ago = SCH_GetLastEventMonoTime() - inst->last_nke_success;
  else
    report->last_ke_ago = -1;
  report->cookies = inst->num_cookies;
  report->cookie_length = inst->num_cookies > 0 ? inst->cookies[inst->cookie_index].length : 0;
  report->nak = inst->nak_response;
}
