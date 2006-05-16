/*
 * BGP open message handling
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _ZEBRA_BGP_OPEN_H
#define _ZEBRA_BGP_OPEN_H

/* Multiprotocol Extensions capabilities. */
#define CAPABILITY_CODE_MP            1
#define CAPABILITY_CODE_MP_LEN        4

/* Route refresh capabilities. */
#define CAPABILITY_CODE_REFRESH     128
#define CAPABILITY_CODE_REFRESH_01    2
#define CAPABILITY_CODE_REFRESH_LEN   0

int bgp_open_option_parse (struct peer *, u_char, int *);
void bgp_open_capability (struct stream *, struct peer *);
void bgp_capability_vty_out (struct vty *, struct peer *);

#endif /* _ZEBRA_BGP_OPEN_H */
