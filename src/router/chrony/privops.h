/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Bryan Christianson 2015
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

  Perform privileged operations over a unix socket to a privileged fork.

*/

#ifndef GOT_PRIVOPS_H
#define GOT_PRIVOPS_H

#ifdef PRIVOPS_ADJUSTTIME
int PRV_AdjustTime(const struct timeval *delta, struct timeval *olddelta);
#else
#define PRV_AdjustTime adjtime
#endif

#ifdef PRIVOPS_ADJUSTTIMEX
int PRV_AdjustTimex(struct timex *txc);
#else
#define PRV_AdjustTimex ntp_adjtime
#endif

#ifdef PRIVOPS_SETTIME
int PRV_SetTime(const struct timeval *tp, const struct timezone *tzp);
#else
#define PRV_SetTime settimeofday
#endif

#ifdef PRIVOPS_BINDSOCKET
int PRV_BindSocket(int sock, struct sockaddr *address, socklen_t address_len);
#else
#define PRV_BindSocket bind
#endif

#ifdef PRIVOPS_NAME2IPADDRESS
int PRV_Name2IPAddress(const char *name, IPAddr *ip_addrs, int max_addrs);
#else
#define PRV_Name2IPAddress DNS_Name2IPAddress
#endif

#ifdef PRIVOPS_RELOADDNS
void PRV_ReloadDNS(void);
#else
#define PRV_ReloadDNS DNS_Reload
#endif

#ifdef PRIVOPS_HELPER
void PRV_Initialise(void);
void PRV_StartHelper(void);
void PRV_Finalise(void);
#else
#define PRV_Initialise()
#define PRV_StartHelper()
#define PRV_Finalise()
#endif

#endif
