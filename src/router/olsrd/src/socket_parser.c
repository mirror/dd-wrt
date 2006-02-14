/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * * Redistributions of source code must retain the above copyright 
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright 
 *   notice, this list of conditions and the following disclaimer in 
 *   the documentation and/or other materials provided with the 
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its 
 *   contributors may be used to endorse or promote products derived 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 * $Id: socket_parser.c,v 1.23 2005/05/29 12:47:45 br1 Exp $
 */

#include <unistd.h>
#include "socket_parser.h"
#include "olsr.h"
#include "defs.h"
#include "log.h"
#include "net_os.h"

#ifdef WIN32
#undef EINTR
#define EINTR WSAEINTR
#undef errno
#define errno WSAGetLastError()
#undef strerror
#define strerror(x) StrError(x)
#endif


struct olsr_socket_entry *olsr_socket_entries;

static int hfd = 0;

static struct timeval tvp = {0, 0};
static fd_set ibits;

/**
 * Add a socket and handler to the socketset
 * beeing used in the main select(2) loop
 * in listen_loop
 *
 *@param fd the socket
 *@param pf the processing function
 */
void
add_olsr_socket(int fd, void(*pf)(int))
{
  struct olsr_socket_entry *new_entry;

  if((fd == 0) || (pf == NULL))
    {
      fprintf(stderr, "Bogus socket entry - not registering...\n");
      return;
    }
  OLSR_PRINTF(2, "Adding OLSR socket entry %d\n", fd)

  new_entry = olsr_malloc(sizeof(struct olsr_socket_entry), "Socket entry");

  new_entry->fd = fd;
  new_entry->process_function = pf;

  /* Queue */
  new_entry->next = olsr_socket_entries;
  olsr_socket_entries = new_entry;

  if(fd + 1 > hfd)
    hfd = fd + 1;
}

/**
 * Remove a socket and handler to the socketset
 * beeing used in the main select(2) loop
 * in listen_loop
 *
 *@param fd the socket
 *@param pf the processing function
 */
int
remove_olsr_socket(int fd, void(*pf)(int))
{
  struct olsr_socket_entry *entry, *prev_entry;

  if((fd == 0) || (pf == NULL))
    {
      olsr_syslog(OLSR_LOG_ERR, "Bogus socket entry - not processing...\n");
      return 0;
    }
  OLSR_PRINTF(1, "Removing OLSR socket entry %d\n", fd)

  entry = olsr_socket_entries;
  prev_entry = NULL;

  while(entry)
    {
      if((entry->fd == fd) && (entry->process_function == pf))
	{
	  if(prev_entry == NULL)
	    {
	      olsr_socket_entries = entry->next;
	      free(entry);
	    }
	  else
	    {
	      prev_entry->next = entry->next;
	      free(entry);
	    }

	  if(hfd == fd + 1)
	    {
	      /* Re-calculate highest FD */
	      entry = olsr_socket_entries;
	      hfd = 0;
	      while(entry)
		{
		  if(entry->fd + 1 > hfd)
		    hfd = entry->fd + 1;
		  entry = entry->next;
		}
	    }
	  return 1;
	}
      prev_entry = entry;
      entry = entry->next;
    }

  return 0;
}


void
poll_sockets()
{
  int n;
  struct olsr_socket_entry *olsr_sockets;
  static struct tms tms_buf;

  /* If there are no registered sockets we
   * do not call select(2)
   */
  if(hfd == 0)
    return;
  
  FD_ZERO(&ibits);
  
  /* Adding file-descriptors to FD set */
  olsr_sockets = olsr_socket_entries;
  while(olsr_sockets)
    {
      FD_SET(olsr_sockets->fd, &ibits);
      olsr_sockets = olsr_sockets->next;
    }
      
  /* Runnig select on the FD set */
  n = olsr_select(hfd, &ibits, 0, 0, &tvp);
  
  if(n == 0)
    return;
  /* Did somethig go wrong? */
  if (n < 0) 
    {
      if(errno == EINTR)
	return;

      olsr_syslog(OLSR_LOG_ERR, "select: %m");
      OLSR_PRINTF(1, "Error select: %s", strerror(errno))
      return;
    }

  /* Update time since this is much used by the parsing functions */
  gettimeofday(&now, NULL);      
  now_times = times(&tms_buf);

  olsr_sockets = olsr_socket_entries;
  while(olsr_sockets)
    {
      if(FD_ISSET(olsr_sockets->fd, &ibits))
	{
	  olsr_sockets->process_function(olsr_sockets->fd);
	}
      olsr_sockets = olsr_sockets->next;
    }
  	
}

