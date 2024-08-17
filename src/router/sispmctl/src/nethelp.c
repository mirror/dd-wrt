/*
    Copyright (C) 2003 Evan Buswell

    Based on code from icecast 1.3.11
    sock.c: Copyright (C) 1999 Jack Moffitt, Barath Raghavan, and Alexander Haväng

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <sys/socket.h>
#include <sys/types.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/*
 * Write len bytes from buff to the socket.
 * Returns the return value from send()
 */
int sock_write_bytes(int sockfd, const unsigned char *buff, int len)
{
	int t, n;

	for (t = 0; len > 0;) {
		n = send(sockfd, (void *)buff + t, len, MSG_NOSIGNAL);
		if (n < 0) {
			return ((t == 0) ? n : t);
		}
		t += n;
		len -= n;
	}
	return (t);
}
