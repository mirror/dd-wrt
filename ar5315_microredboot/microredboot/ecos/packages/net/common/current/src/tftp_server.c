//==========================================================================
//
//      lib/tftp_server.c
//
//      TFTP server support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Andrew Lunn
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
// Contributors: gthomas, hmt, andrew.lunn@ascom.ch (Andrew Lunn)
// Date:         2000-04-06
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// TFTP server support

#include <network.h>          // Basic networking support
#include <arpa/tftp.h>        // TFTP protocol definitions
#include <tftp_support.h>     // TFTPD structures
#include <cyg/kernel/kapi.h>
#include <stdlib.h>           // For malloc
#include <fcntl.h>

#define nCYGOPT_NET_TFTP_SERVER_INSTRUMENT
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT

struct subinfo {
    int rx;
    int rx_repeat;
    int rx_skip;
    int send;
    int resend;
};

struct info {
    struct subinfo ack, data;
    int err_send;
    int total_transactions;
};


static struct info tftp_server_instrument = {
    { 0,0,0,0,0 },
    { 0,0,0,0,0 },
    0, 0,
};

#endif // CYGOPT_NET_TFTP_SERVER_INSTRUMENT

#ifdef CYGSEM_NET_TFTPD_MULTITHREADED
struct tftpd_sem {
  int port;
  cyg_sem_t sem;
};

static struct tftpd_sem tftpd_sems[CYGNUM_NET_TFTPD_MULTITHREADED_PORTS];
#endif //CYGSEM_NET_TFTPD_MULTITHREADED

#define STACK_SIZE (((CYGPKG_NET_TFTPD_THREAD_STACK_SIZE) + CYGARC_ALIGNMENT-1) & ~(CYGARC_ALIGNMENT-1))
static char *TFTP_tag = "TFTPD";
#define CYGNUM_NET_MAX_INET_PROTOS 2
struct tftp_server {
    char                 *tag;
    char                  stack[STACK_SIZE];
    cyg_thread            thread_data;
    cyg_handle_t          thread_handle;
    int                   port;
    struct tftpd_fileops *ops;
    int s[CYGNUM_NET_MAX_INET_PROTOS], num_s;
    struct addrinfo      *res;
};

static char * errmsg[] = {
  "Undefined error code",               // 0 nothing defined
  "File not found",                     // 1 TFTP_ENOTFOUND 
  "Access violation",                   // 2 TFTP_EACCESS   
  "Disk full or allocation exceeded",   // 3 TFTP_ENOSPACE  
  "Illegal TFTP operation",             // 4 TFTP_EBADOP    
  "Unknown transfer ID",                // 5 TFTP_EBADID    
  "File already exists",                // 6 TFTP_EEXISTS   
  "No such user",                       // 7 TFTP_ENOUSER   
};

// Little helper function to set the port number in an address 
static void set_port(struct sockaddr * address, int port) {

  switch (address->sa_family) {
  case AF_INET:{
    struct sockaddr_in *addr = (struct sockaddr_in *)address;
    addr->sin_port = ntohs(port);
    break;
  }
#ifdef CYGPKG_NET_INET6
  case AF_INET6:
    {
      struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
      addr->sin6_port = ntohs(port);
      break;
    }
#endif
  default:
    break;
  }
}

#ifdef CYGSEM_NET_TFTPD_MULTITHREADED

// Allocate a semaphore for a given port number if one is not already
// allocated.
static void sem_alloc(int port) {
  int i;

  CYG_ASSERT(port != 0, "Invalid port number");

  cyg_scheduler_lock(); // Avoid race with other tftpd's
  for (i=0; i < CYGNUM_NET_TFTPD_MULTITHREADED_PORTS; i++) {
    if (tftpd_sems[i].port == port) {
      cyg_scheduler_unlock();
      return;
    }
    if (tftpd_sems[i].port == 0) {
      tftpd_sems[i].port = port;
      cyg_semaphore_init(&tftpd_sems[i].sem,1);
      cyg_scheduler_unlock();
      return ;
    }
  }
  cyg_scheduler_unlock();
  diag_printf("TFTPD: Unable to allocate a semaphore for port %d\n",port);
}
  
// Wait on the semaphore for a given port number.
static void sem_wait(int port) {
  int i;
  CYG_ASSERT(port != 0, "Invalid port number");
  
  for (i=0; i < CYGNUM_NET_TFTPD_MULTITHREADED_PORTS; i++) {
    if (tftpd_sems[i].port == port) {
      cyg_semaphore_wait(&tftpd_sems[i].sem);
      return;
    }
  }
  diag_printf("TFTPD: No semaphore for port %d\n",port);
}

// Release the semaphore for a given port number.
static void sem_post(int port) {
  int i;
  CYG_ASSERT(port != 0, "Invalid port number");
  
  for (i=0; i < CYGNUM_NET_TFTPD_MULTITHREADED_PORTS; i++) {
    if (tftpd_sems[i].port == port) {
      cyg_semaphore_post(&tftpd_sems[i].sem);
      return;
    }
  }
  diag_printf("TFTPD: No semaphore for port %d\n",port);
}
#endif

/* Send an error packet to the client */
static void 
tftpd_send_error(int s, struct tftphdr * reply, int err,
		 struct sockaddr *from_addr, int from_len)
{
    CYG_ASSERT( 0 <= err, "err underflow" );
    CYG_ASSERT( sizeof(errmsg)/sizeof(errmsg[0]) > err, "err overflow" );

    reply->th_opcode = htons(ERROR);
    reply->th_code = htons(err);
    if ( (0 > err) || (sizeof(errmsg)/sizeof(errmsg[0]) <= err) )
        err = 0; // Do not copy a random string from hyperspace
    strcpy(reply->th_msg, errmsg[err]);
    sendto(s, reply, 4+strlen(reply->th_msg)+1, 0, 
	   from_addr, from_len);
}

//
// Receive a file from the client
//
static void
tftpd_write_file(struct tftp_server *server,
                 struct tftphdr *hdr, 
                 struct sockaddr *from_addr, int from_len)
{
    char data_out[SEGSIZE+sizeof(struct tftphdr)];
    char data_in[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *reply = (struct tftphdr *)data_out;
    struct tftphdr *response = (struct tftphdr *)data_in;
    int fd, len, ok, tries, closed, data_len, s;
    unsigned short block;
    struct timeval timeout;
    fd_set fds;
    int total_timeouts = 0;
    struct sockaddr client_addr;
    struct addrinfo hints;
    struct addrinfo *res;
    int client_len;
    int error;
    
    memset(&hints,0,sizeof(hints));
    hints.ai_family = from_addr->sa_family;
    hints.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL,"tftp",&hints, &res);
    if (0 != error) {
      diag_printf("TFTPD: can't get a suitable local address: %s\n",
		  gai_strerror(error));
      return;
    }
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s < 0) {
        diag_printf("TFTPD: can't open socket for 'write_file'\n");
	freeaddrinfo(res);
        return;
    }

    // We want the stack to pick a free local port number
    set_port(res->ai_addr,0);

    if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
        // Problem setting up my end
        diag_printf("TFTPD: can't bind to reply port for 'write_file'\n");
        close(s);
	freeaddrinfo(res);
        return;
    }
    if ((fd = (server->ops->open)(hdr->th_stuff, O_WRONLY|O_TRUNC|O_CREAT)) < 0) {
        tftpd_send_error(s,reply,TFTP_ENOTFOUND,from_addr, from_len);
        close(s);
	freeaddrinfo(res);
        return;
    }
    ok = true;
    closed = false;
    block = 0;
    while (ok) {
        // Send ACK telling client he can send data
        reply->th_opcode = htons(ACK);
        reply->th_block = htons(block++); // postincrement
        for (tries = 0;  tries < TFTP_RETRIES_MAX;  tries++) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
            tftp_server_instrument.ack.send++;
#endif
            sendto(s, reply, 4, 0, from_addr, from_len);
        repeat_select:
            timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
            timeout.tv_usec = 0;
            FD_ZERO(&fds);
            FD_SET(s, &fds);
            if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
                if (++total_timeouts > TFTP_TIMEOUT_MAX) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                    tftp_server_instrument.err_send++;
#endif
                    tftpd_send_error(s,reply,TFTP_EBADOP,from_addr, from_len);
                    ok = false;
                    break;
                }
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.ack.resend++;
#endif
                continue; // retry the send, using up one retry.
            }
            // Some data has arrived
            data_len = sizeof(data_in);
            client_len = sizeof(client_addr);
            if ((data_len = recvfrom(s, data_in, data_len, 0, 
                      &client_addr, &client_len)) < 0) {
                // What happened?  No data here!
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.ack.resend++;
#endif
                continue; // retry the send, using up one retry.
            }
            if (ntohs(response->th_opcode) == DATA &&
                ntohs(response->th_block) < block) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.data.rx_repeat++;
#endif
                // Then it is repeat DATA with an old block; listen again,
                // but do not repeat sending the current ack, and do not
                // use up a retry count.  (we do re-send the ack if
                // subsequently we time out)
                goto repeat_select;
            }
            if (ntohs(response->th_opcode) == DATA &&
                ntohs(response->th_block) == block) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.data.rx++;
#endif
                // Good data - write to file
                len = (server->ops->write)(fd, response->th_data, data_len-4);
                if (len < (data_len-4)) {
                    // File is "full"
                    tftpd_send_error(s,reply,TFTP_ENOSPACE,
                                     from_addr, from_len);     
                    ok = false;  // Give up
                    break; // out of the retries loop
                }
                if (data_len < (SEGSIZE+4)) {
                    // End of file
                    closed = true;
                    ok = false;
                    if ((server->ops->close)(fd) == -1) {
                        tftpd_send_error(s,reply,TFTP_EACCESS,
                                         from_addr, from_len);
                        break;  // out of the retries loop
                    }
                    // Exception to the loop structure: we must ACK the last
                    // packet, the one that implied EOF:
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                    tftp_server_instrument.ack.send++;
#endif
                    reply->th_opcode = htons(ACK);
                    reply->th_block = htons(block++); // postincrement
                    sendto(s, reply, 4, 0, from_addr, from_len);
                    break; // out of the retries loop
                }
                // Happy!  Break out of the retries loop.
                break;
            }
            // Otherwise, we got something we do not understand!  So repeat
            // sending the current ACK, and use up a retry count.
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
            if ( (ntohs(response->th_opcode) == DATA) )
                tftp_server_instrument.data.rx_skip++;
            tftp_server_instrument.ack.resend++;
#endif
        } // End of the retries loop.
        if (TFTP_RETRIES_MAX <= tries) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
            tftp_server_instrument.err_send++;
#endif
            tftpd_send_error(s,reply,TFTP_EBADOP,from_addr, from_len);
            ok = false;
        }
    }
    close(s);
    freeaddrinfo(res);
    if (!closed) {
      (server->ops->close)(fd);
    }
}

//
// Send a file to the client
//
static void
tftpd_read_file(struct tftp_server *server,
                struct tftphdr *hdr, 
                struct sockaddr *from_addr, int from_len)
{
    char data_out[SEGSIZE+sizeof(struct tftphdr)];
    char data_in[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *reply = (struct tftphdr *)data_out;
    struct tftphdr *response = (struct tftphdr *)data_in;
    int fd, len, tries, ok, data_len, s;
    unsigned short block;
    struct timeval timeout;
    fd_set fds;
    int total_timeouts = 0;
    struct sockaddr client_addr;
    struct addrinfo hints;
    struct addrinfo *res;
    int client_len;
    int error;

    memset(&hints,0,sizeof(hints));
    hints.ai_family = from_addr->sa_family;
    hints.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL,"tftp",&hints, &res);
    if (0 != error) {
      diag_printf("TFTPD: can't get a suitable local address: %s\n",
		  gai_strerror(error));
      return;
    }
    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s < 0) {
        diag_printf("TFTPD: can't open socket for 'write_file'\n");
	freeaddrinfo(res);
        return;
    }

    // We want the stack to pick a free local port number
    set_port(res->ai_addr,0);

    if (bind(s, res->ai_addr, res->ai_addrlen) < 0) {
        // Problem setting up my end
        diag_printf("TFTPD: can't bind to reply port for 'write_file'\n");
        close(s);
	freeaddrinfo(res);
        return;
    }

    if ((fd = (server->ops->open)(hdr->th_stuff, O_RDONLY)) < 0) {
        tftpd_send_error(s,reply,TFTP_ENOTFOUND,from_addr, from_len);
        close(s);
	freeaddrinfo(res);
        return;
    }
    block = 0;
    ok = true;
    while (ok) {
        // Read next chunk of file
        len = (server->ops->read)(fd, reply->th_data, SEGSIZE);
        reply->th_block = htons(++block); // preincrement
        reply->th_opcode = htons(DATA);
        for (tries = 0;  tries < TFTP_RETRIES_MAX;  tries++) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
            tftp_server_instrument.data.send++;
#endif
            if (sendto(s, reply, 4+len, 0,
                       from_addr, from_len) < 0) {
                // Something went wrong with the network!
                ok = false;
                break;
            }
        repeat_select:
            timeout.tv_sec = TFTP_TIMEOUT_PERIOD;
            timeout.tv_usec = 0;
            FD_ZERO(&fds);
            FD_SET(s, &fds);
            if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
                if (++total_timeouts > TFTP_TIMEOUT_MAX) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                    tftp_server_instrument.err_send++;
#endif
 		    tftpd_send_error(s,reply,TFTP_EBADOP,from_addr, from_len);
                    ok = false;
                    break;
                }
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.data.resend++;
#endif
                continue; // retry the send, using up one retry.
            }
            data_len = sizeof(data_in);
            client_len = sizeof(client_addr);
            if ((data_len = recvfrom(s, data_in, data_len, 0, 
                                     &client_addr,
                                     &client_len)) < 0) {
                // What happened?  Maybe someone lied to us...
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.data.resend++;
#endif
                continue; // retry the send, using up one retry.
            }
            if ((ntohs(response->th_opcode) == ACK) &&
                (ntohs(response->th_block) < block)) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.ack.rx_repeat++;
#endif
                // Then it is a repeat ACK for an old block; listen again,
                // but do not repeat sending the current block, and do not
                // use up a retry count.  (we do re-send the data if
                // subsequently we time out)
                goto repeat_select;
            }
            if ((ntohs(response->th_opcode) == ACK) &&
                (ntohs(response->th_block) == block)) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
                tftp_server_instrument.ack.rx++;
#endif
                // Happy!  Break out of the retries loop.
                break;
            }
            // Otherwise, we got something we do not understand!  So repeat
            // sending the current block, and use up a retry count.
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
            if ( (ntohs(response->th_opcode) == ACK) )
                tftp_server_instrument.ack.rx_skip++;
            tftp_server_instrument.data.resend++;
#endif
        } // End of the retries loop.
        if (TFTP_RETRIES_MAX <= tries) {
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
            tftp_server_instrument.err_send++;
#endif
            tftpd_send_error(s,reply,TFTP_EBADOP,from_addr, from_len);
            ok = false;
        }
        if (len < SEGSIZE) {
            break; // That's end of file then.
        }
    }
    close(s);
    freeaddrinfo(res);
    (server->ops->close)(fd);
}

//
// Actual TFTP server
//

static void
tftpd_server(cyg_addrword_t p)
{
    struct tftp_server *server = (struct tftp_server *)p;
    int max_s = 0;
    int data_len, recv_len, from_len;
    struct sockaddr from_addr;
    char data[SEGSIZE+sizeof(struct tftphdr)];
    struct tftphdr *hdr = (struct tftphdr *)data;
    struct addrinfo hints;
    struct addrinfo *ai;
    fd_set readfds;
    char name[64];
    int error;
    int i;
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
    struct info o = tftp_server_instrument;
#endif
    
#ifndef CYGPKG_NET_TESTS_USE_RT_TEST_HARNESS
    // Otherwise routine printfs fail the test - interrupts disabled too long.
    diag_printf("TFTPD [%x]: port %d\n", p, server->port);
#endif

    memset(&hints,0,sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;

    error = getaddrinfo(NULL,"tftp",&hints, &server->res);
    if (0 != error) {
      diag_printf("TFTPD [%p] : can't get a local server address to bind to: %s\n",
		  p, gai_strerror(error));
      return;
    }
 
    // If the port is 0, we need to use the default TFTP port. Extrace
    // the port number from one of the addresses returned by
    // getaddrinfo.
    if (server->port == 0) {
      switch (server->res->ai_family) {
      case AF_INET: 
	{
	  struct sockaddr_in *addr = (struct sockaddr_in *)server->res->ai_addr;
	  server->port = ntohs(addr->sin_port);
	  break;
	}
#ifdef CYGPKG_NET_INET6
      case AF_INET6:
	{
	  struct sockaddr_in6 *addr = (struct sockaddr_in6 *)server->res->ai_addr;
	  server->port = ntohs(addr->sin6_port);
	  break;
	}
#endif
      default:
	break;
      }
    }

#ifdef CYGSEM_NET_TFTPD_MULTITHREADED   
    sem_alloc(server->port);
    while (true) {
#endif
      // Iterate over the addresses and create a local port to listen for requests 
      ai = server->res;
      memset(server->s,0,sizeof(server->s));
      server->num_s = 0;
#ifdef CYGSEM_NET_TFTPD_MULTITHREADED   
      sem_wait(server->port);
#endif
      while (ai && (server->num_s < CYGNUM_NET_MAX_INET_PROTOS)) {
	server->s[server->num_s] = socket(ai->ai_family, 
					  ai->ai_socktype, 
					  ai->ai_protocol);
	if (server->s[server->num_s] < 0 ) {
	  diag_printf("TFTPD [%x]: can't open socket\n", p);
	  freeaddrinfo(server->res);
	  server->res = NULL;
	  return;
	}
	
	set_port(ai->ai_addr, server->port);
	
	if (bind(server->s[server->num_s],ai->ai_addr, ai->ai_addrlen) < 0) {
	  // Problem setting up my end
	  diag_printf("TFTPD [%x]: can't bind to server port\n",p);
	  close(server->s[server->num_s]);
	  server->s[server->num_s] = 0;
	  ai = ai->ai_next;
	  continue;
	}
	// We need to know the highest socket number for select.
	if (server->s[server->num_s] > max_s)
	  max_s = server->s[server->num_s];
	server->num_s++;
	ai = ai->ai_next;
      }
      
#ifndef CYGSEM_NET_TFTPD_MULTITHREADED   
      while (true) {
#endif
	FD_ZERO(&readfds);
	for (i=0; i < CYGNUM_NET_MAX_INET_PROTOS; i++) {
	  if (server->s[i]) {
	    FD_SET(server->s[i],&readfds);
	  }
	}
	error = select(max_s+1,&readfds,NULL,NULL,NULL);
	if ( -1 == error) {
	  diag_printf("TFTPD [%x]: error in select\n",p);
	}
	for (i=0; i < CYGNUM_NET_MAX_INET_PROTOS; i++) {
	  if (server->s[i] && FD_ISSET(server->s[i],&readfds)) {
	    recv_len = sizeof(data);
	    from_len = sizeof(from_addr);
	    data_len = recvfrom(server->s[i], hdr, recv_len, 0,
				&from_addr, &from_len);
	    if ( data_len < 0) {
	      diag_printf("TFTPD [%x]: can't read request\n", p);
	    } else {
#ifdef CYGSEM_NET_TFTPD_MULTITHREADED   
	      // Close the socket and post on the semaphore some
	      // another thread can start listening for requests. This
	      // is not quite right. select could of returned with more than
	      // one socket with data to read. Here we only deal with one of them 
	      for (i=0; i < CYGNUM_NET_MAX_INET_PROTOS; i++) {
		if (server->s[i]) {
		  close (server->s[i]);
		  server->s[i] = 0;
		}
	      }
	      sem_post(server->port);
#endif
#ifndef CYGPKG_NET_TESTS_USE_RT_TEST_HARNESS
	      getnameinfo(&from_addr,sizeof(from_addr), name, sizeof(name),0,0,0);
	      diag_printf("TFTPD [%x]: received %x from %s\n", p,
			  ntohs(hdr->th_opcode), name);
#endif
	      switch (ntohs(hdr->th_opcode)) {
	      case WRQ:
		tftpd_write_file(server, hdr, &from_addr, from_len);
		break;
	      case RRQ:
		tftpd_read_file(server, hdr, &from_addr, from_len);
		break;
	      case ACK:
	      case DATA:
	      case ERROR:
		// Ignore
		break;
	      default:
		getnameinfo(&from_addr,sizeof(from_addr), name, sizeof(name),0,0,0);
		diag_printf("TFTPD [%x]: bogus request %x from %s\n", p,
			    ntohs(hdr->th_opcode),
			    name);
		tftpd_send_error(server->s[i],hdr,TFTP_EBADOP,&from_addr,from_len);
	      }
	      
#ifdef CYGOPT_NET_TFTP_SERVER_INSTRUMENT
	      tftp_server_instrument.total_transactions++;
	      
	      o.data.rx        -= tftp_server_instrument.data.rx       ;
	      o.data.rx_repeat -= tftp_server_instrument.data.rx_repeat;
	      o.data.rx_skip   -= tftp_server_instrument.data.rx_skip  ;
	      o.data.send      -= tftp_server_instrument.data.send     ;
	      o.data.resend    -= tftp_server_instrument.data.resend   ;
	      
	      o.ack.rx         -= tftp_server_instrument.ack.rx        ;
	      o.ack.rx_repeat  -= tftp_server_instrument.ack.rx_repeat ;
	      o.ack.rx_skip    -= tftp_server_instrument.ack.rx_skip   ;
	      o.ack.send       -= tftp_server_instrument.ack.send      ;
	      o.ack.resend     -= tftp_server_instrument.ack.resend    ;
	      
	      o.err_send       -= tftp_server_instrument.err_send      ;
	      
#ifndef CYGPKG_NET_TESTS_USE_RT_TEST_HARNESS
	      if ( o.data.rx        ) diag_printf( "data rx       %4d\n", -o.data.rx        );
	      if ( o.data.rx_repeat ) diag_printf( "data rx_repeat%4d\n", -o.data.rx_repeat );
	      if ( o.data.rx_skip   ) diag_printf( "data rx_skip  %4d\n", -o.data.rx_skip   );
	      if ( o.data.send      ) diag_printf( "data send     %4d\n", -o.data.send      );
	      if ( o.data.resend    ) diag_printf( "data resend   %4d\n", -o.data.resend    );
	      
	      if ( o.ack.rx         ) diag_printf( " ack rx       %4d\n", -o.ack.rx         );
	      if ( o.ack.rx_repeat )  diag_printf( " ack rx_repeat%4d\n", -o.ack.rx_repeat  );
	      if ( o.ack.rx_skip   )  diag_printf( " ack rx_skip  %4d\n", -o.ack.rx_skip    );
	      if ( o.ack.send      )  diag_printf( " ack send     %4d\n", -o.ack.send       );
	      if ( o.ack.resend    )  diag_printf( " ack resend   %4d\n", -o.ack.resend     );
	      
	      if ( o.err_send      )  diag_printf( "*error sends  %4d\n", -o.err_send      );
#endif // CYGPKG_NET_TESTS_USE_RT_TEST_HARNESS
#endif // CYGOPT_NET_TFTP_SERVER_INSTRUMENT
#ifdef CYGSEM_NET_TFTPD_MULTITHREADED
	      break;
#endif
	    }
	  }
	}
	// The following looks a little strange, but it keeps emacs's
	// auto indention happy.
#ifndef CYGSEM_NET_TFTPD_MULTITHREADED   
      }
#endif
#ifdef CYGSEM_NET_TFTPD_MULTITHREADED   
    }
#endif
}


//
// This function is used to create a new server [thread] which supports
// the TFTP protocol on the given port.  A server 'id' will be returned
// which can later be used to destroy the server.  
//
// Note: all [memory] resources for the server thread will be allocated
// dynamically.  If there are insufficient resources, an error will be
// returned.
//
int 
tftpd_start(int port, struct tftpd_fileops *ops)
{
    struct tftp_server *server;
#ifdef CYGSEM_TFTP_SERVER_MULTITHREADED
    static char init = 0;
    if ( 0 == init ) {
        init++;
        cyg_semaphore_init( &tftp_server_sem, 0 );
    }
#endif

    if ((server = malloc(sizeof(struct tftp_server)))) {
        server->tag = TFTP_tag;
        server->port = port;
        server->ops = ops;
        cyg_thread_create(CYGPKG_NET_TFTPD_THREAD_PRIORITY, // Priority
                          tftpd_server,              // entry
                          (cyg_addrword_t)server,    // entry parameter
                          "TFTP server",             // Name
                          &server->stack[0],         // Stack
                          STACK_SIZE,                // Size
                          &server->thread_handle,    // Handle
                          &server->thread_data       // Thread data structure
            );
        cyg_thread_resume(server->thread_handle);  // Start it

    }
    return (int)server;
}

//
// Destroy a TFTP server, using a previously created server 'id'.
//
int 
tftpd_stop(int p)
{
    struct tftp_server *server = (struct tftp_server *)p;
    // Simple sanity check
    if (server->tag == TFTP_tag) {
        cyg_thread_kill(server->thread_handle);
        cyg_thread_set_priority(server->thread_handle, 0);
        cyg_thread_delay(1);  // Make sure it gets to die...
        if (cyg_thread_delete(server->thread_handle)) {
            // Success shutting down the thread. Close all its sockets.
	    int i;
	    for (i = 0 ; i < CYGNUM_NET_MAX_INET_PROTOS; i++) {
	      if (server->s[i]) {
		close (server->s[i]);
	      }
	    }
	    freeaddrinfo(server->res);
            free(server);  // Give up memory
            return 1;
        }
    }
    return 0;
}

// EOF tftp_server.c
