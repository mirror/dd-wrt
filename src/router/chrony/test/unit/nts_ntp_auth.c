/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020, 2024
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
 */

#include <config.h>
#include "test.h"

#ifdef FEAT_NTS

#include <nts_ntp_auth.c>

#include "ntp_ext.h"
#include "siv.h"

void
test_unit(void)
{
  unsigned char key[SIV_MAX_KEY_LENGTH], nonce[256], plaintext[256], plaintext2[256];
  NTP_PacketInfo info;
  NTP_Packet packet;
  SIV_Algorithm algo;
  SIV_Instance siv;
  int i, j, r, packet_length, nonce_length, key_length;
  int plaintext_length, plaintext2_length, min_ef_length;

  TEST_CHECK(get_padding_length(0) == 0);
  TEST_CHECK(get_padding_length(1) == 3);
  TEST_CHECK(get_padding_length(2) == 2);
  TEST_CHECK(get_padding_length(3) == 1);
  TEST_CHECK(get_padding_length(4) == 0);
  TEST_CHECK(get_padded_length(0) == 0);
  TEST_CHECK(get_padded_length(1) == 4);
  TEST_CHECK(get_padded_length(2) == 4);
  TEST_CHECK(get_padded_length(3) == 4);
  TEST_CHECK(get_padded_length(4) == 4);
  TEST_CHECK(get_padded_length(5) == 8);

  for (algo = 1; algo < 100; algo++) {
    siv = SIV_CreateInstance(algo);
    if (!siv) {
      TEST_CHECK(algo != AEAD_AES_SIV_CMAC_256);
      continue;
    }

    DEBUG_LOG("algo=%d", (int)algo);

    for (i = 0; i < 10000; i++) {
      key_length = SIV_GetKeyLength(algo);
      for (j = 0; j < key_length; j++)
        key[j] = random() % 256;
      TEST_CHECK(SIV_SetKey(siv, key, key_length));

      assert(sizeof (nonce) >= SIV_GetMinNonceLength(siv));
      nonce_length = SIV_GetMinNonceLength(siv) +
                     random() % (MIN(sizeof (nonce), SIV_GetMaxNonceLength(siv)) -
                                 SIV_GetMinNonceLength(siv) + 1);
      for (j = 0; j < nonce_length; j++)
        nonce[j] = random() % 256;

      plaintext_length = random() % (sizeof (plaintext) + 1);
      for (j = 0; j < plaintext_length; j++)
        plaintext[j] = random() % 256;

      packet_length = NTP_HEADER_LENGTH + random() % 100 * 4;
      min_ef_length = random() % (sizeof (packet) - packet_length);

      memset(&packet, 0, sizeof (packet));
      packet.lvm = NTP_LVM(0, 4, 0);
      memset(&info, 0, sizeof (info));
      info.version = 4;
      info.length = packet_length;

      DEBUG_LOG("packet_length=%d nonce_length=%d plaintext_length=%d min_ef_length=%d",
                packet_length, nonce_length, plaintext_length, min_ef_length);

      r = NNA_GenerateAuthEF(&packet, &info, siv, nonce, nonce_length, plaintext,
                             -1, 0);
      TEST_CHECK(!r);
      r = NNA_GenerateAuthEF(&packet, &info, siv, nonce, 0, plaintext,
                             plaintext_length, 0);
      TEST_CHECK(!r);
      if (SIV_GetMinNonceLength(siv) > 1) {
        r = NNA_GenerateAuthEF(&packet, &info, siv, nonce, SIV_GetMinNonceLength(siv) - 1,
                               plaintext, plaintext_length, 0);
        TEST_CHECK(!r);
        TEST_CHECK(info.ext_fields == 0);
      }
      if (SIV_GetMaxNonceLength(siv) <= sizeof (nonce)) {
        r = NNA_GenerateAuthEF(&packet, &info, siv, nonce, SIV_GetMaxNonceLength(siv) - 1,
                               plaintext, plaintext_length, 0);
        TEST_CHECK(!r);
        TEST_CHECK(info.ext_fields == 0);
      }
      r = NNA_GenerateAuthEF(&packet, &info, siv, nonce, nonce_length, plaintext,
                             plaintext_length, sizeof (packet) - info.length + 1);
      TEST_CHECK(!r);

      r = NNA_GenerateAuthEF(&packet, &info, siv, nonce, nonce_length, plaintext,
                             plaintext_length, min_ef_length);
      TEST_CHECK(r);
      TEST_CHECK(info.ext_fields == 1);
      TEST_CHECK(info.length - packet_length >= min_ef_length);
      TEST_CHECK(info.length - packet_length == get_padded_length(min_ef_length) ||
                 info.length - packet_length == 4 + 4 +
                 get_padded_length(MAX(MIN(16, SIV_GetMaxNonceLength(siv)), nonce_length)) +
                 get_padded_length(plaintext_length) + SIV_GetTagLength(siv));

      r = NNA_DecryptAuthEF(&packet, &info, siv, packet_length, plaintext2,
                            -1, &plaintext2_length);
      TEST_CHECK(!r);

      if (random() % 2) {
        for (j = 0; j < get_padding_length(nonce_length); j++)
          ((unsigned char *)&packet)[packet_length + 4 + 4 + nonce_length + j]++;
        for (j = packet_length + 4 + 4 + get_padded_length(nonce_length) +
             plaintext_length + SIV_GetTagLength(siv); j < sizeof (packet); j++)
          ((unsigned char *)&packet)[j]++;
      }

      r = NNA_DecryptAuthEF(&packet, &info, siv, packet_length, plaintext2,
                            sizeof (plaintext2), &plaintext2_length);
      TEST_CHECK(r);
      TEST_CHECK(plaintext_length == plaintext2_length);
      TEST_CHECK(memcmp(plaintext, plaintext2, plaintext_length) == 0);

      if (random() % 2) {
        j = random() % (packet_length + 4 + 4 + nonce_length);
      } else {
        j = packet_length + 4 + 4 + get_padded_length(nonce_length);
        j += random() % (plaintext_length + SIV_GetTagLength(siv));
      }
      ((unsigned char *)&packet)[j]++;
      r = NNA_DecryptAuthEF(&packet, &info, siv, packet_length, plaintext2,
                            sizeof (plaintext2), &plaintext2_length);
      TEST_CHECK(!r);
      ((unsigned char *)&packet)[j]--;
    }

    SIV_DestroyInstance(siv);
  }
}
#else
void
test_unit(void)
{
  TEST_REQUIRE(0);
}
#endif
