/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _RADIUS_PKT_H
#define _RADIUS_PKT_H

struct radius_packet_t {
  uint8_t code;
  uint8_t id;
  uint16_t length;
  uint8_t authenticator[RADIUS_AUTHLEN];
  uint8_t payload[RADIUS_PACKSIZE-RADIUS_HDRSIZE];
} __attribute__((packed));


struct radius_vsattr_t {
  uint32_t i;  /* vendor-id */
  uint8_t t;   /* vsa type */
  uint8_t l;   /* length */
  union {
    uint32_t i;
    uint8_t  t[RADIUS_ATTR_VLEN-4];
  } v;         /* value */
} __attribute__((packed));


struct radius_attr_t {
  uint8_t t;   /* type */
  uint8_t l;   /* length */
  union {
    uint32_t i;
    uint8_t  t[RADIUS_ATTR_VLEN];
    struct radius_vsattr_t vv;
  } v;        /* value */
} __attribute__((packed));

#define AUTH_FMT "%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
#define AUTH_ARG(x) (x)[0],(x)[1],(x)[2],(x)[3],(x)[4],(x)[5],(x)[6],(x)[7],(x)[8],(x)[9],(x)[10],(x)[11],(x)[12],(x)[13],(x)[14],(x)[15]

#endif
