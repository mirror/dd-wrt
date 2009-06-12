//==========================================================================
//
//      lib/tftp_client.c
//
//      TFTP client support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Andrew.Lunn@ascom.ch
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:         2000-04-06
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// TFTP client support

#include <network.h>
#include <arpa/tftp.h>
#include <tftp_support.h>

#define min(x,y) (x<y ? x : y)

//
// Read a file from a host into a local buffer.  Returns the
// number of bytes actually read, or (-1) if an error occurs.
// On error, *err will hold the reason.
// This version uses the server name. This can be a name for DNS lookup
// or a dotty or colony number format for IPv4 or IPv6.
int tftp_client_get(char *filename,
		    char *server,
		    int port,
		    char *buf,
		    int len,
		    int mode,
		    int *err) {
		    
    int result = 0;
    int s=-1;
    int actual_len, data_len, recv_len, from_len;
    static int get_port = 7700;
    struct addrinfo * addrinfo;
    struct addrinfo * res;
    struct addrinfo hints;
    int error;

    struct sockaddr local_addr, from_addr;
    char data[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *hdr = (struct tftphdr *)data;
    char *cp, *fp;
    struct timeval timeout;
    unsigned short last_good_block = 0;
    fd_set fds;
    int total_timeouts = 0;

    *err = 0;  // Just in case

    // Create initial request
    hdr->th_opcode = htons(RRQ);  // Read file
    cp = (char *)&hdr->th_stuff;
    fp = filename;
    while (*fp) *cp++ = *fp++;
    *cp++ = '\0';
    if (mode == TFTP_NETASCII) {
        fp = "NETASCII";
    } else if (mode == TFTP_OCTET) {
        fp = "OCTET";
    } else {
        *err = TFTP_INVALID;
        return -1;
    }
    while (*fp) *cp++ = *fp++;
    *cp++ = '\0';

    memset(&hints,0,sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    error = getaddrinfo(server, "tftp", &hints, &res);
    if (error) {
      *err = TFTP_NETERR;
      return -1;
    }
    
    addrinfo = res;
    while (addrinfo) {
      s = socket(addrinfo->ai_family, addrinfo->ai_socktype, 
		 addrinfo->ai_protocol);
      if (s >= 0) {
	memcpy(&local_addr,addrinfo->ai_addr,addrinfo->ai_addrlen);
	switch(addrinfo->ai_addr->sa_family) {
	case AF_INET: {
	  struct sockaddr_in * saddr = 
	    (struct sockaddr_in *) addrinfo->ai_addr;
	  struct sockaddr_in * laddr = 
	    (struct sockaddr_in *) &local_addr;
	  if (port) {
	    saddr->sin_port = htons(port);
	  }
	  laddr->sin_port = htons(get_port++);
	  laddr->sin_addr.s_addr = INADDR_ANY;
	  break;
	}
#ifdef CYGPKG_NET_INET6
	case AF_INET6: {
	  struct sockaddr_in6 * saddr = 
	    (struct sockaddr_in6 *) addrinfo->ai_addr;
	  struct sockaddr_in6 * laddr = 
	    (struct sockaddr_in6 *) &local_addr;
	  if (port) {
	    saddr->sin6_port = htons(port);
	  }
	  laddr->sin6_port = htons(get_port++);
	  laddr->sin6_addr = in6addr_any;
	  break;
	}
#endif
	default:
	  *err = TFTP_NETERR;
	  goto out;
	}

	if (bind(s,&local_addr,addrinfo->ai_addrlen) < 0) {
          *err = TFTP_NETERR;
          goto out;
        }
	
	// Send request
	if (sendto(s, data, (int)(cp-data), 0, 
		   addrinfo->ai_addr, 
		   addrinfo->ai_addrlen) < 0) {
	  // Problem sending request
	  *err = TFTP_NETERR;
	  goto nextaddr;
	}

	// Read data
	fp = buf;
	while (true) {
	  timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
	  timeout.tv_usec = 0;
	  FD_ZERO(&fds);
	  FD_SET(s, &fds);
	  if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
            total_timeouts++;
            if ((last_good_block == 0) && (total_timeouts > TFTP_RETRIES_MAX)) {
	      // Timeout - no data received. Probably no server.
	      *err = TFTP_TIMEOUT;
	      goto nextaddr;
            }
	    if (total_timeouts > TFTP_TIMEOUT_MAX) {
              // Timeout - have received data. Network problem?
              *err = TFTP_TIMEOUT;
              goto out;
	    }
            
            if (last_good_block == 0 ) {
              // Send request
              if (sendto(s, data, (int)(cp-data), 0, 
                         addrinfo->ai_addr, 
                         addrinfo->ai_addrlen) < 0) {
                // Problem sending request
                *err = TFTP_NETERR;
                goto nextaddr;
              }
            } else {
              // Try resending last ACK
              hdr->th_opcode = htons(ACK);
              hdr->th_block = htons(last_good_block);
              if (sendto(s, data, 4 /* FIXME */, 0, 
                         &from_addr, from_len) < 0) {
                // Problem sending request
                *err = TFTP_NETERR;
                goto out;
              }
            }
	  } else {
	    recv_len = sizeof(data);
	    from_len = sizeof(from_addr);
	    if ((data_len = recvfrom(s, &data, recv_len, 0, 
				     &from_addr, &from_len)) < 0) {
	      // What happened?
	      *err = TFTP_NETERR;
	      goto out;
	    }
	    if (ntohs(hdr->th_opcode) == DATA) {
	      actual_len = 0;
	      if (ntohs(hdr->th_block) == (last_good_block+1)) {
		// Consume this data
		cp = hdr->th_data;
		data_len -= 4;  /* Sizeof TFTP header */
		actual_len = data_len;
		result += actual_len;
		while (data_len-- > 0) {
		  if (len-- > 0) {
		    *fp++ = *cp++;
		  } else {
		    // Buffer overflow
		    *err = TFTP_TOOLARGE;
		    goto out;
		  }
		}
		last_good_block++;
	      } else {
                // To prevent an out-of-sequence packet from
                // terminating transmission prematurely, set
                // actual_len to a full size packet.
		actual_len = SEGSIZE;
	      }
	      // Send out the ACK
	      hdr->th_opcode = htons(ACK);
	      hdr->th_block = htons(last_good_block);
	      if (sendto(s, data, 4 /* FIXME */, 0, 
			 &from_addr, from_len) < 0) {
		// Problem sending request
		*err = TFTP_NETERR;
		goto out;
	      }
              // A short packet marks the end of the file.
	      if ((actual_len >= 0) && (actual_len < SEGSIZE)) {
		// End of data
		close(s);
		freeaddrinfo(res);
		return result;
	      }
	    } else 
	      if (ntohs(hdr->th_opcode) == ERROR) {
		*err = ntohs(hdr->th_code);
		goto out;
	      } else {
		// What kind of packet is this?
                *err = TFTP_PROTOCOL;
		goto out;
	      }
	  }
	}
      }
      // If we got here, it means there was a problem connecting to the 
      // server. Try the next address returned by getaddrinfo
    nextaddr:
      if (-1 != s) {
	close(s);
      }
      addrinfo=addrinfo->ai_next;
    }
    // We ran into problems. Cleanup
 out:
    if (-1 != s) {
      close (s);
    }
    freeaddrinfo(res);
    return -1;
}
//
// Read a file from a host into a local buffer.  Returns the
// number of bytes actually read, or (-1) if an error occurs.
// On error, *err will hold the reason.
//
// Depreciated. Use tftp_client_get instead.
int
tftp_get(char *filename,
         struct sockaddr_in *server,
         char *buf,
         int len,
         int mode,
         int *err)
{
  char server_name[20];
  char *ret;
  int port;

  ret = inet_ntop(AF_INET, (void *)&server->sin_addr, 
		  server_name, sizeof(server_name));
  if (NULL == ret) {
      *err = TFTP_NETERR;
      return -1;
  }
  port = server->sin_port;

  return tftp_client_get(filename, server_name, port, buf, len, mode, err);
}

//
// Send data to a file on a server via TFTP.
//
int
tftp_put(char *filename,
         struct sockaddr_in *server,
         char *buf,
         int len,
         int mode,
         int *err)
{
  char server_name[20];
  char *ret;
  int port;

  ret = inet_ntop(AF_INET, (void *)&server->sin_addr, 
		  server_name, sizeof(server_name));
  if (NULL == ret) {
      *err = TFTP_NETERR;
      return -1;
  }
  port = server->sin_port;

  return tftp_client_put(filename, server_name, port, buf, len, mode, err);
}

//
// Put a file to a host from a local buffer.  Returns the
// number of bytes actually writen, or (-1) if an error occurs.
// On error, *err will hold the reason.
// This version uses the server name. This can be a name for DNS lookup
// or a dotty or colony number format for IPv4 or IPv6.
int tftp_client_put(char *filename,
		    char *server,
		    int port,
		    char *buf,
		    int len,
		    int mode,
		    int *err) {

    int result = 0;
    int s = -1, actual_len, data_len, recv_len, from_len;
    static int put_port = 7800;
    struct sockaddr local_addr, from_addr;
    char data[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *hdr = (struct tftphdr *)data;
    char *cp, *fp, *sfp;
    struct timeval timeout;
    unsigned short last_good_block = 0;
    fd_set fds;
    int total_timeouts = 0;
    struct addrinfo hints;
    struct addrinfo * addrinfo;
    struct addrinfo * res;
    int error;

    *err = 0;  // Just in case

    memset(&hints,0,sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    error = getaddrinfo(server, "tftp", &hints, &res);
    if (error) {
      *err = TFTP_NETERR;
      return -1;
    }
    
    addrinfo = res;
    while (addrinfo) {
      s = socket(addrinfo->ai_family, addrinfo->ai_socktype,
		 addrinfo->ai_protocol);
      if (s >= 0) {
	memcpy(&local_addr,addrinfo->ai_addr,addrinfo->ai_addrlen);
	switch(addrinfo->ai_addr->sa_family) {
	case AF_INET: {
	  struct sockaddr_in * saddr = 
	    (struct sockaddr_in *) addrinfo->ai_addr;
	  struct sockaddr_in * laddr = 
	    (struct sockaddr_in *) &local_addr;
	  if (port) {
	    saddr->sin_port = htons(port);
	  }
	  laddr->sin_port = htons(put_port++);
	  laddr->sin_addr.s_addr = INADDR_ANY;
	  break;
	}
#ifdef CYGPKG_NET_INET6
	case AF_INET6: {
	  struct sockaddr_in6 * saddr = 
	    (struct sockaddr_in6 *) addrinfo->ai_addr;
	  struct sockaddr_in6 * laddr = 
	    (struct sockaddr_in6 *) &local_addr;
	  if (port) {
	    saddr->sin6_port = htons(port);
	  }
	  laddr->sin6_port = htons(put_port++);
	  laddr->sin6_addr = in6addr_any;
	  break;
	}
#endif
	default:
	  *err = TFTP_NETERR;
	  goto out;
	}
	if (bind(s, 
		 (struct sockaddr *)&local_addr, 
		 addrinfo->ai_addrlen) < 0) {
	  // Problem setting up my end
	  *err = TFTP_NETERR;
	  goto out;
	}

	while (1) {
	  // Create initial request
	  hdr->th_opcode = htons(WRQ);  // Create/write file
	  cp = (char *)&hdr->th_stuff;
	  fp = filename;
	  while (*fp) *cp++ = *fp++;
	  *cp++ = '\0';
	  if (mode == TFTP_NETASCII) {
            fp = "NETASCII";
	  } else if (mode == TFTP_OCTET) {
            fp = "OCTET";
	  } else {
            *err = TFTP_INVALID;
	    goto out;
	  }
	  while (*fp) *cp++ = *fp++;
	  *cp++ = '\0';
	  // Send request
	  if (sendto(s, data, (int)(cp-data), 0, 
		     addrinfo->ai_addr,
		     addrinfo->ai_addrlen) < 0) {
            // Problem sending request
            *err = TFTP_NETERR;
	    goto nextaddr;
	  }
	  // Wait for ACK
	  timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
	  timeout.tv_usec = 0;
	  FD_ZERO(&fds);
	  FD_SET(s, &fds);
	  if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
            if (++total_timeouts > TFTP_RETRIES_MAX) {
	      // Timeout - no ACK received
	      *err = TFTP_TIMEOUT;
	      goto nextaddr;
            }
	  } else {
            recv_len = sizeof(data);
            from_len = sizeof(from_addr);
            if ((data_len = recvfrom(s, &data, recv_len, 0, 
                                     &from_addr, &from_len)) < 0) {
	      // What happened?
	      *err = TFTP_NETERR;
	      goto out;
            }
            if (ntohs(hdr->th_opcode) == ACK) {
	      // Write request accepted - start sending data
	      break;
            } else 
	      if (ntohs(hdr->th_opcode) == ERROR) {
                *err = ntohs(hdr->th_code);
                goto out;
	      } else {
                // What kind of packet is this?
		goto out;
	      }
	  }
	}
	
	// Send data
	sfp = buf;
	last_good_block = 1;
	while (result < len) {
	  // Build packet of data to send
	  data_len = min(SEGSIZE, len-result);
	  hdr->th_opcode = htons(DATA);
	  hdr->th_block = htons(last_good_block);
	  cp = hdr->th_data;
	  fp = sfp;
	  actual_len = data_len + 4;
	  // FIXME - what about "netascii" data?
	  while (data_len-- > 0) *cp++ = *fp++;
	  // Send data packet
	  if (sendto(s, data, actual_len, 0, 
		     &from_addr, from_len) < 0) {
            // Problem sending request
            *err = TFTP_NETERR;
	    goto out;
	  }
	  // Wait for ACK
	  timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
	  timeout.tv_usec = 0;
	  FD_ZERO(&fds);
	  FD_SET(s, &fds);
	  if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
            if (++total_timeouts > TFTP_TIMEOUT_MAX) {
	      // Timeout - no data received
	      *err = TFTP_TIMEOUT;
	      goto out;
            }
	  } else {
            recv_len = sizeof(data);
            from_len = sizeof(from_addr);
            if ((data_len = recvfrom(s, &data, recv_len, 0, 
                                     &from_addr, &from_len)) < 0) {
	      // What happened?
	      *err = TFTP_NETERR;
	      goto out;
            }
            if (ntohs(hdr->th_opcode) == ACK) {
	      if (ntohs(hdr->th_block) == last_good_block) {
		// Advance pointers, etc
		sfp = fp;
		result += (actual_len - 4);
		last_good_block++;
	      } else {
		diag_printf("Send block #%d, got ACK for #%d\n", 
			    last_good_block, ntohs(hdr->th_block));
	      }
            } else 
	      if (ntohs(hdr->th_opcode) == ERROR) {
                *err = ntohs(hdr->th_code);
                goto out;
	      } else {
                // What kind of packet is this?
                *err = TFTP_PROTOCOL;
		goto out;
	      }
	  }
	}
	close (s);
	return result;
      }

      // If we got here, it means there was a problem connecting to the 
      // server. Try the next address returned by getaddrinfo
    nextaddr:
      if (-1 != s) {
	close(s);
      }
      addrinfo=addrinfo->ai_next;
    }
    // We ran into problems. Cleanup
 out:
    if (-1 != s) {
      close (s);
    }
    freeaddrinfo(res);
    return -1;
}

// EOF tftp_client.c
