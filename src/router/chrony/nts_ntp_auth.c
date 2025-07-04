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

  NTS Authenticator and Encrypted Extension Fields extension field
  */

#include "config.h"

#include "sysincl.h"

#include "nts_ntp_auth.h"

#include "logging.h"
#include "ntp_ext.h"
#include "nts_ntp.h"
#include "siv.h"
#include "util.h"

struct AuthHeader {
  uint16_t nonce_length;
  uint16_t ciphertext_length;
};

/* ================================================== */

static int
get_padding_length(int length)
{
  return length % 4U ? 4 - length % 4U : 0;
}

/* ================================================== */

static int
get_padded_length(int length)
{
  return length + get_padding_length(length);
}

/* ================================================== */

int
NNA_GenerateAuthEF(NTP_Packet *packet, NTP_PacketInfo *info, SIV_Instance siv,
                   const unsigned char *nonce, int max_nonce_length,
                   const unsigned char *plaintext, int plaintext_length,
                   int min_ef_length)
{
  int auth_length, ciphertext_length, assoc_length, nonce_length, max_siv_nonce_length;
  int nonce_padding, ciphertext_padding, additional_padding;
  unsigned char *ciphertext, *body;
  struct AuthHeader *header;

  assert(sizeof (*header) == 4);

  if (max_nonce_length <= 0 || plaintext_length < 0) {
    DEBUG_LOG("Invalid nonce/plaintext length");
    return 0;
  }

  assoc_length = info->length;
  max_siv_nonce_length = SIV_GetMaxNonceLength(siv);
  nonce_length = MIN(max_nonce_length, max_siv_nonce_length);
  ciphertext_length = SIV_GetTagLength(siv) + plaintext_length;
  nonce_padding = get_padding_length(nonce_length);
  ciphertext_padding = get_padding_length(ciphertext_length);
  min_ef_length = get_padded_length(min_ef_length);

  auth_length = sizeof (*header) + nonce_length + nonce_padding +
                ciphertext_length + ciphertext_padding;
  additional_padding = MAX(min_ef_length - auth_length - 4, 0);
  additional_padding = MAX(MIN(NTS_MIN_UNPADDED_NONCE_LENGTH, max_siv_nonce_length) -
                           nonce_length - nonce_padding, additional_padding);
  auth_length += additional_padding;

  if (!NEF_AddBlankField(packet, info, NTP_EF_NTS_AUTH_AND_EEF, auth_length,
                         (void **)&header)) {
    DEBUG_LOG("Could not add EF");
    return 0;
  }

  header->nonce_length = htons(nonce_length);
  header->ciphertext_length = htons(ciphertext_length);

  body = (unsigned char *)(header + 1);
  ciphertext = body + nonce_length + nonce_padding;

  BRIEF_ASSERT((unsigned char *)header + auth_length ==
               ciphertext + ciphertext_length + ciphertext_padding + additional_padding);

  memcpy(body, nonce, nonce_length);
  memset(body + nonce_length, 0, nonce_padding);

  if (!SIV_Encrypt(siv, nonce, nonce_length, packet, assoc_length,
                   plaintext, plaintext_length, ciphertext, ciphertext_length)) {
    DEBUG_LOG("SIV encrypt failed");
    info->length = assoc_length;
    info->ext_fields--;
    return 0;
  }

  memset(ciphertext + ciphertext_length, 0, ciphertext_padding + additional_padding);

  return 1;
}

/* ================================================== */

int
NNA_DecryptAuthEF(NTP_Packet *packet, NTP_PacketInfo *info, SIV_Instance siv, int ef_start,
                  unsigned char *plaintext, int buffer_length, int *plaintext_length)
{
  int siv_tag_length, max_siv_nonce_length, nonce_length, ciphertext_length;
  unsigned char *nonce, *ciphertext;
  int ef_type, ef_body_length;
  void *ef_body;
  struct AuthHeader *header;

  if (buffer_length < 0)
    return 0;

  if (!NEF_ParseField(packet, info->length, ef_start,
                      NULL, &ef_type, &ef_body, &ef_body_length))
    return 0;

  if (ef_type != NTP_EF_NTS_AUTH_AND_EEF || ef_body_length < sizeof (*header))
    return 0;

  header = ef_body;

  nonce_length = ntohs(header->nonce_length);
  ciphertext_length = ntohs(header->ciphertext_length);

  if (get_padded_length(nonce_length) +
      get_padded_length(ciphertext_length) > ef_body_length)
    return 0;

  nonce = (unsigned char *)(header + 1);
  ciphertext = nonce + get_padded_length(nonce_length);

  max_siv_nonce_length = SIV_GetMaxNonceLength(siv);
  siv_tag_length = SIV_GetTagLength(siv);

  if (nonce_length < 1 ||
      ciphertext_length < siv_tag_length ||
      ciphertext_length - siv_tag_length > buffer_length) {
    DEBUG_LOG("Unexpected nonce/ciphertext length");
    return 0;
  }

  if (sizeof (*header) + MIN(NTS_MIN_UNPADDED_NONCE_LENGTH, max_siv_nonce_length) +
      get_padded_length(ciphertext_length) > ef_body_length) {
    DEBUG_LOG("Missing padding");
    return 0;
  }

  *plaintext_length = ciphertext_length - siv_tag_length;
  assert(*plaintext_length >= 0);

  if (!SIV_Decrypt(siv, nonce, nonce_length, packet, ef_start,
                   ciphertext, ciphertext_length, plaintext, *plaintext_length)) {
    DEBUG_LOG("SIV decrypt failed");
    return 0;
  }

  return 1;
}
