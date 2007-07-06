/*
 * netlink/helpers.h		Helper Tools
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef NETLINK_HELPERS_H_
#define NETLINK_HELPERS_H_

#include <netlink/netlink.h>

/**
 * Lower probability limit
 * @ingroup helpers
 */
#define NL_PROB_MIN 0x0

/**
 * Upper probability limit
 * @ingroup helpers
 */
#define NL_PROB_MAX 0xffffffff

extern char *	nl_geterror(void);

extern char *	nl_llproto2str_r(int, char *, size_t);
extern char *	nl_llproto2str(int);
extern int	nl_str2llproto(const char *);

extern char *	nl_ether_proto2str_r(int, char *, size_t);
extern char *	nl_ether_proto2str(int);
extern int	nl_str2ether_proto(const char *);

extern double	nl_cancel_down_bytes(unsigned long long, char **);
extern double	nl_cancel_down_bits(unsigned long long, char **);
extern double	nl_cancel_down_us(uint32_t, char **);

extern int	nl_get_hz(void);
extern uint32_t	nl_us2ticks(uint32_t);
extern uint32_t	nl_ticks2us(uint32_t);

extern char * nl_msec2str(uint64_t);
extern char * nl_msec2str_r(uint64_t, char *, size_t);

extern long	nl_size2int(const char *);
extern long	nl_time2int(const char *);

#endif
