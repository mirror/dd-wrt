/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019-2020
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

  Functions for adding and parsing NTPv4 extension fields
  */

#include "config.h"

#include "sysincl.h"

#include "ntp_ext.h"

struct ExtFieldHeader {
  uint16_t type;
  uint16_t length;
};

/* ================================================== */

static int
format_field(unsigned char *buffer, int buffer_length, int start,
             int type, int body_length, int *length, void **body)
{
  struct ExtFieldHeader *header;

  if (buffer_length < 0 || start < 0 || buffer_length <= start ||
      buffer_length - start < sizeof (*header) || start % 4 != 0)
    return 0;

  header = (struct ExtFieldHeader *)(buffer + start);

  if (body_length < 0 || sizeof (*header) + body_length > 0xffff ||
      start + sizeof (*header) + body_length > buffer_length || body_length % 4 != 0)
    return 0;

  header->type = htons(type);
  header->length = htons(sizeof (*header) + body_length);
  *length = sizeof (*header) + body_length;
  *body = header + 1;

  return 1;
}

/* ================================================== */

int
NEF_SetField(unsigned char *buffer, int buffer_length, int start,
             int type, void *body, int body_length, int *length)
{
  void *ef_body;

  if (!format_field(buffer, buffer_length, start, type, body_length, length, &ef_body))
    return 0;

  memcpy(ef_body, body, body_length);

  return 1;
}

/* ================================================== */

int
NEF_AddBlankField(NTP_Packet *packet, NTP_PacketInfo *info, int type, int body_length, void **body)
{
  int ef_length, length = info->length;

  if (length < NTP_HEADER_LENGTH || length >= sizeof (*packet) || length % 4 != 0)
    return 0;

  /* Only NTPv4 packets can have extension fields */
  if (info->version != 4)
    return 0;

  if (!format_field((unsigned char *)packet, sizeof (*packet), length,
                    type, body_length, &ef_length, body))
    return 0;

  if (ef_length < NTP_MIN_EF_LENGTH)
    return 0;

  info->length += ef_length;
  info->ext_fields++;

  return 1;
}

/* ================================================== */

int
NEF_AddField(NTP_Packet *packet, NTP_PacketInfo *info,
             int type, void *body, int body_length)
{
  void *ef_body;

  if (!NEF_AddBlankField(packet, info, type, body_length, &ef_body))
    return 0;

  memcpy(ef_body, body, body_length);

  return 1;
}

/* ================================================== */

int
NEF_ParseSingleField(unsigned char *buffer, int buffer_length, int start,
                     int *length, int *type, void **body, int *body_length)
{
  struct ExtFieldHeader *header;
  int ef_length;

  if (buffer_length < 0 || start < 0 || buffer_length <= start ||
      buffer_length - start < sizeof (*header))
    return 0;

  header = (struct ExtFieldHeader *)(buffer + start);

  assert(sizeof (*header) == 4);

  ef_length = ntohs(header->length);

  if (ef_length < (int)(sizeof (*header)) || start + ef_length > buffer_length ||
      ef_length % 4 != 0)
    return 0;

  if (length)
    *length = ef_length;
  if (type)
    *type = ntohs(header->type);
  if (body)
    *body = header + 1;
  if (body_length)
    *body_length = ef_length - sizeof (*header);

  return 1;
}

/* ================================================== */

int
NEF_ParseField(NTP_Packet *packet, int packet_length, int start,
               int *length, int *type, void **body, int *body_length)
{
  int ef_length;

  if (packet_length <= NTP_HEADER_LENGTH || packet_length > sizeof (*packet) ||
      packet_length <= start || packet_length % 4 != 0 ||
      start < NTP_HEADER_LENGTH || start % 4 != 0)
    return 0;

  /* Only NTPv4 packets have extension fields */
  if (NTP_LVM_TO_VERSION(packet->lvm) != 4)
    return 0;

  /* Check if the remaining data is a MAC.  RFC 7822 specifies the maximum
     length of a MAC in NTPv4 packets in order to enable deterministic
     parsing. */
  if (packet_length - start <= NTP_MAX_V4_MAC_LENGTH)
    return 0;

  if (!NEF_ParseSingleField((unsigned char *)packet, packet_length, start,
                            &ef_length, type, body, body_length))
    return 0;

  if (ef_length < NTP_MIN_EF_LENGTH)
    return 0;

  if (length)
    *length = ef_length;

  return 1;
}
