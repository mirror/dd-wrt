/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/sys.h"
#include "lwip/api.h"

static void 
tcpecho_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;

  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);

  /* Bind connection to well known port number 7. */
  netconn_bind(conn, NULL, 7);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while(1) {

    /* Grab new connection. */
    newconn = netconn_accept(conn);
    /* Process the new connection. */
    if(newconn != NULL) {
      struct netbuf *buf;
      void *data;
      u16_t len;
      
      while((buf = netconn_recv(newconn)) != NULL) {
	do {
	  netbuf_data(buf, &data, &len);
	  err = netconn_write(newconn, data, len, NETCONN_COPY);
	  if(err != ERR_OK) {
	  }
	} while(netbuf_next(buf) >= 0);
	netbuf_delete(buf);	
      }
      /* Close connection and discard connection identifier. */
      netconn_delete(newconn);
    }
  }
}

void
tmain(cyg_addrword_t p)
{
  lwip_init();	
  sys_thread_new(tcpecho_thread, (void*)"tcpecho",7);  
}

#define STACK_SIZE 0x1000
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

void
cyg_user_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      tmain,          // entry
                      0,                 // entry parameter
                      "TCP echo test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
}

