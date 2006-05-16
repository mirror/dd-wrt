/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDE__ipsecadm_h__200204271540
#define INCLUDE__ipsecadm_h__200204271540

#include <stdint.h>
#include <net/if.h>
#include <netinet/ip.h>

#ifndef ARPHRD_IPSEC
#define ARPHRD_IPSEC 31
#endif

#define SIOCIPSEC_GET_TUNNEL_OLD   (SIOCDEVPRIVATE + 0)
#define SIOCIPSEC_ADD_TUNNEL   (SIOCDEVPRIVATE + 1)
#define SIOCIPSEC_DEL_TUNNEL   (SIOCDEVPRIVATE + 2)
#define SIOCIPSEC_CHG_TUNNEL   (SIOCDEVPRIVATE + 3)
#define SIOCIPSEC_GET_SA       (SIOCDEVPRIVATE + 4)
#define SIOCIPSEC_ADD_SA       (SIOCDEVPRIVATE + 5)
#define SIOCIPSEC_DEL_SA       (SIOCDEVPRIVATE + 6)
#define SIOCIPSEC_CHG_SA       (SIOCDEVPRIVATE + 7)
#define SIOCIPSEC_GET_STATS    (SIOCDEVPRIVATE + 8)
#define SIOCIPSEC_GET_TUNNEL   (SIOCDEVPRIVATE + 9)

#define IPSECDEVNAME       "ipsec0"

#define IPSEC_SPI_ANY                 0

#define IPSEC_SA_VERSION              0
#define IPSEC_SA_CRYPTOLEN           32
#define IPSEC_STATS_VERSION           1

int find_unambiguous_string(const char *const strlist[], const char *str);

const char *ipv4_ntoa(uint32_t addr);

uint32_t ipv4_aton(const char *str);

uint32_t strtospi(const char *str);

int ifname_to_ifindex(const char *name);

char *ifindex_to_ifname(char *ifname, int ifindex);

int ipsec_tunnel_open(const char *name, struct ifreq *ifr, int quiet);

int hex2dec(int c);

int parse_key(const char *str, void *key, int maxsize);

int read_key_file(const char *filename, void *key, int maxsize);

void error(const char *fmt, ...);

/* Main functions: */
int sa_main(int argc, char *argv[]);
int tunnel_main(int argc, char *argv[]);
int key_main(int argc, char *argv[]);
int stats_main(int argc, char *argv[]);

#endif
