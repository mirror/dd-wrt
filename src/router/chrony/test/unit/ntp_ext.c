/*
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
 */

#include <config.h>
#include "test.h"

#include <util.h>
#include <logging.h>

#include <ntp_ext.c>

void
test_unit(void)
{
  unsigned char *buffer, body[NTP_MAX_EXTENSIONS_LENGTH];
  void *bodyp;
  NTP_PacketInfo info;
  NTP_Packet packet;
  int i, j, start, length, type, type2, body_length, body_length2;

  assert(sizeof (uint16_t) == 2);
  assert(sizeof (body) == sizeof (packet.extensions));

  buffer = (unsigned char *)packet.extensions;

  for (i = 0; i < 10000; i++) {
    body_length = random() % (sizeof (body) - 4 + 1) / 4 * 4;
    start = random() % (sizeof (packet.extensions) - body_length - 4 + 1) / 4 * 4;
    type = random() % 0x10000;

    DEBUG_LOG("body_length=%d start=%d type=%d", body_length, start, type);
    assert(body_length + start <= sizeof (packet.extensions));

    UTI_GetRandomBytes(body, body_length);

    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 4, start,
                             type, body, body_length + 4, &length));
    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 4, start + 4,
                             type, body, body_length, &length));
    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 4, start,
                             type, body, body_length - 1, &length));
    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 4, start,
                             type, body, body_length - 2, &length));
    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 4, start,
                             type, body, body_length - 3, &length));
    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 3, start,
                             type, body, body_length, &length));
    TEST_CHECK(!NEF_SetField(buffer, body_length + start + 5, start + 1,
                             type, body, body_length, &length));

    TEST_CHECK(NEF_SetField(buffer, body_length + start + 4, start,
                            type, body, body_length, &length));
    TEST_CHECK(length == body_length + 4);
    TEST_CHECK(((uint16_t *)buffer)[start / 2] == htons(type));
    TEST_CHECK(((uint16_t *)buffer)[start / 2 + 1] == htons(length));
    TEST_CHECK(memcmp(buffer + start + 4, body, body_length) == 0);

    memset(&packet, 0, sizeof (packet));
    packet.lvm = NTP_LVM(0, 4, MODE_CLIENT);
    memset(&info, 0, sizeof (info));

    info.version = 3;
    info.length = NTP_HEADER_LENGTH;
    TEST_CHECK(!NEF_AddBlankField(&packet, &info, type, body_length, &bodyp));

    info.version = 4;
    info.length = NTP_HEADER_LENGTH - 4;
    TEST_CHECK(!NEF_AddBlankField(&packet, &info, type, body_length, &bodyp));

    info.length = sizeof (packet) - body_length;
    TEST_CHECK(!NEF_AddBlankField(&packet, &info, type, body_length, &bodyp));

    info.length = NTP_HEADER_LENGTH + start;

    if (body_length < 12) {
      TEST_CHECK(!NEF_AddBlankField(&packet, &info, type, body_length, &bodyp));
      continue;
    }

    TEST_CHECK(NEF_AddBlankField(&packet, &info, type, body_length, &bodyp));
    TEST_CHECK(info.length == NTP_HEADER_LENGTH + start + body_length + 4);
    TEST_CHECK(((uint16_t *)buffer)[start / 2] == htons(type));
    TEST_CHECK(((uint16_t *)buffer)[start / 2 + 1] == htons(length));
    TEST_CHECK(bodyp == buffer + start + 4);
    TEST_CHECK(info.ext_fields == 1);

    memset(buffer, 0, sizeof (packet.extensions));
    info.length = NTP_HEADER_LENGTH + start;
    info.ext_fields = 0;

    TEST_CHECK(NEF_AddField(&packet, &info, type, body, body_length));
    TEST_CHECK(info.length == NTP_HEADER_LENGTH + start + body_length + 4);
    TEST_CHECK(((uint16_t *)buffer)[start / 2] == htons(type));
    TEST_CHECK(((uint16_t *)buffer)[start / 2 + 1] == htons(length));
    TEST_CHECK(memcmp(buffer + start + 4, body, body_length) == 0);
    TEST_CHECK(info.ext_fields == 1);

    for (j = 1; j <= 4; j++) {
      TEST_CHECK(((uint16_t *)buffer)[start / 2 + 1] = htons(length + j));
      TEST_CHECK(!NEF_ParseSingleField(buffer, start + body_length + 4, start,
                                       &length, &type2, &bodyp, &body_length2));
    }

    TEST_CHECK(((uint16_t *)buffer)[start / 2 + 1] = htons(length));

    TEST_CHECK(NEF_ParseSingleField(buffer, sizeof (packet.extensions), start,
                                    &length, &type2, &bodyp, &body_length2));
    TEST_CHECK(length == body_length + 4);
    TEST_CHECK(type2 == type);
    TEST_CHECK(bodyp == buffer + start + 4);
    TEST_CHECK(body_length2 == body_length);

    TEST_CHECK(!NEF_ParseField(&packet, sizeof (packet) + 4,
                               NTP_HEADER_LENGTH + start,
                               &length, &type2, &bodyp, &body_length2));

    if (body_length < 24) {
      TEST_CHECK(!NEF_ParseField(&packet, NTP_HEADER_LENGTH + start + length, 
                                 NTP_HEADER_LENGTH + start,
                                 &length, &type2, &bodyp, &body_length2));
      if (sizeof (packet.extensions) - start <= 24) {
        TEST_CHECK(!NEF_ParseField(&packet, sizeof (packet), NTP_HEADER_LENGTH + start,
                                   &length, &type2, &bodyp, &body_length2));
        continue;
      } else {
        TEST_CHECK(NEF_ParseField(&packet, sizeof (packet), NTP_HEADER_LENGTH + start,
                                  &length, &type2, &bodyp, &body_length2));
      }
    } else {
      TEST_CHECK(NEF_ParseField(&packet, NTP_HEADER_LENGTH + start + length, 
                                NTP_HEADER_LENGTH + start,
                                &length, &type2, &bodyp, &body_length2));
    }
    TEST_CHECK(length == body_length + 4);
    TEST_CHECK(type2 == type);
    TEST_CHECK(bodyp == buffer + start + 4);
    TEST_CHECK(body_length2 == body_length);

  }
}
