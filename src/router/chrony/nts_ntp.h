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

  Header file for the NTS-NTP protocol
  */

#ifndef GOT_NTS_NTP_H
#define GOT_NTS_NTP_H

#define NTP_KOD_NTS_NAK                 0x4e54534e

#define NTS_MIN_UNIQ_ID_LENGTH          32
#define NTS_MIN_UNPADDED_NONCE_LENGTH   16
#define NTS_MAX_COOKIES                 8

#endif
