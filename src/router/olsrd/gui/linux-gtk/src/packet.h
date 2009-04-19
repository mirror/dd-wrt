
/*
 * OLSR ad-hoc routing table management protocol GUI front-end
 * Copyright (C) 2003 Andreas Tonnesen (andreto@ifi.uio.no)
 *
 * This file is part of olsr.org.
 *
 * uolsrGUI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * uolsrGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsr.org; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 *Andreas Tonnesen (andreto@ifi.uio.no)
 *
 *The list of cached packets
 *
 */

#ifndef _PACKET_H
#define _PACKET_H

struct packnode {
  union olsr_message *packet;
  struct packnode *next;
};

/*
 * Unions for IPv4 <-> IPv6 compability
 */

union hello_message {
  struct hellomsg v4;
  struct hellomsg6 v6;
};

union tc_message {
  struct olsr_tcmsg v4;
  struct olsr_tcmsg6 v6;
};

union mid_message {
  struct midmsg v4;
  struct midmsg6 v6;
};

union hna_message {
  struct hnamsg v4;
  struct hnamsg6 v6;
};

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
