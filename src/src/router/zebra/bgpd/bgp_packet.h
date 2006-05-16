/*
 * BGP packet management header.
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

#ifndef _ZEBRA_BGP_PACKET_H
#define _ZEBRA_BGP_PACKET_H

/* Packet send and receive function prototypes. */
int bgp_read (struct thread *);
int bgp_write (struct thread *);

void bgp_keepalive_send (struct peer *);
void bgp_open_send (struct peer *);
void bgp_notify_send (struct peer *, u_char, u_char);
void bgp_notify_send_with_data (struct peer *, u_char, u_char, u_char *, size_t);
void bgp_update_send (struct peer_conf *, struct peer *, struct prefix *, struct attr *, u_int16_t, u_char, struct peer *, struct prefix_rd *, u_char *);
void bgp_withdraw_send (struct peer *, struct prefix *, afi_t, safi_t, struct prefix_rd *, u_char *);
void bgp_route_refresh_send (struct peer *, afi_t, safi_t);

#endif /* _ZEBRA_BGP_PACKET_H */
