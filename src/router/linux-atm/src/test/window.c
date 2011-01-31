/* window.c, M. Welsh (mdw24@cl.cam.ac.uk)
 * A simple bandwidth/latency benchmark for Linux/ATM. Usage:
 * window [send | recv] <addr-data> <addr-ack> <num> <msg_size> <window_size>
 *   where 
 * <addr-data> is the VPI/VCI to transmit data on
 * <addr-ack>  is the VPI/VCI to transmit ACKs on 
 * <num> is the number of iterations to run
 * <msg_size> is the size of each message to send
 * <window_size> is the number of messages to send between ACKs
 *
 * Two VC's are used so that the program can be run between two processes
 * on the same machine.
 *
 * Example:
 * apple%  window recv 0.32 0.33 1000 1024 10
 * banana% window send 0.32 0.33 1000 1024 10
 *
 * Copyright (c) 1996 University of Cambridge Computer Laboratory
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * M. Welsh, 6 July 1996
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <atm.h>

#undef MDW_DEBUG
#undef TERSE_OUTPUT
#define REPLY_SIZE 40
#define MAX_WINDOW_SIZE 100

static double get_seconds(void) {
  struct timeval t;
  gettimeofday(&t,NULL);
  return (double)t.tv_sec+((double)t.tv_usec/(double)1e6);
}

int main(int argc, char **argv) {
   struct sockaddr_atmpvc addr1, addr2;
   struct atm_qos qos1, qos2;
   char *buffer;
   char buffer2[REPLY_SIZE];
   int s1, s2;
   ssize_t size;

   char *theaddr1, *theaddr2;
   int NUM_WINDOWS;
   int PINGPONG_SIZE;
   int WINDOW_SIZE;

  
   int i, w;
   int sending = 0;
   double t1, t2;


   if (argc != 7) {
     fprintf(stderr,"Usage: window [send | recv] <addr-data> <addr-ack> <num_windows> <message_size> <window_size>\n");
     exit(-1);
   }
   if (!strcmp(argv[1],"send")) sending = 1;

   theaddr1 = argv[2];
   theaddr2 = argv[3];
   NUM_WINDOWS = atoi(argv[4]);
   PINGPONG_SIZE = atoi(argv[5]);
   WINDOW_SIZE = atoi(argv[6]);

   if (WINDOW_SIZE > MAX_WINDOW_SIZE) {
     fprintf(stderr,"Maximum window size is %d.\n",MAX_WINDOW_SIZE);
     exit(-1);
   }

   if ((s1 = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
     perror("socket");
     exit(-1);
   }
   if ((s2 = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
     perror("socket");
     exit(-1);
   }
   memset(&addr1, 0, sizeof(addr1));
   memset(&addr2, 0, sizeof(addr2));
   if (text2atm(theaddr1, (struct sockaddr *)&addr1, sizeof(addr1), T2A_PVC | T2A_UNSPEC | T2A_WILDCARD) < 0) {
     fprintf(stderr,"window: invalid address syntax\n");
     exit(-1);
   }
   if (text2atm(theaddr2, (struct sockaddr *)&addr2, sizeof(addr2), T2A_PVC | T2A_UNSPEC | T2A_WILDCARD) < 0) {
     fprintf(stderr,"window: invalid address syntax\n");
     exit(-1);
   }
   
   
   /* Do a lot of them */
   if (sending) {
     buffer = (char *)malloc(PINGPONG_SIZE);
     if (!buffer) {
       fprintf(stderr,"Can't malloc buffer\n");
       exit(-1);
     }
     for (i = 0; i < PINGPONG_SIZE; i++) {
       buffer[i] = i&0xff;
     }

     memset(&qos1,0,sizeof(qos1));
     memset(&qos2,0,sizeof(qos2));

     qos1.aal = ATM_AAL5;
     qos1.txtp.traffic_class = ATM_UBR;
     qos1.txtp.max_sdu = PINGPONG_SIZE;
     qos2.aal = ATM_AAL5;
     qos2.rxtp.traffic_class = ATM_UBR;
     qos2.rxtp.max_sdu = REPLY_SIZE;

     if (setsockopt(s1,SOL_ATM,SO_ATMQOS,&qos1,sizeof(qos1)) < 0) {
       perror("setsockopt SO_ATMQOS1");
       exit(-1);
     }
     if (setsockopt(s2,SOL_ATM,SO_ATMQOS,&qos2,sizeof(qos2)) < 0) {
       perror("setsockopt SO_ATMQOS2");
       exit(-1);
     }
     
     if (connect(s1, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
       perror("connect");
       exit(-1);
     }
     if (bind(s2, (struct sockaddr *)&addr2, sizeof(addr1)) < 0) {
       perror("bind");
       exit(-1);
     }

#ifndef TERSE_OUTPUT
     fprintf(stderr,"Sending %d %d-byte messages, window size %d.\n",
	     NUM_WINDOWS*WINDOW_SIZE,PINGPONG_SIZE,WINDOW_SIZE);
#endif

     t1 = get_seconds();
     for (i = 0; i < NUM_WINDOWS; i++) {
#ifdef MDW_DEBUG
       fprintf(stderr,"Sending %d...",i);
#endif
       for (w = 0; w < WINDOW_SIZE; w++) {
         (void)write(s1, buffer, PINGPONG_SIZE);
       }
       
#ifdef MDW_DEBUG
       fprintf(stderr,"sent!...");
#endif
       
#if 1 /* XXX mdw testing! XXX XXX */
       /* Get a reply */
       size = read(s2, buffer2, REPLY_SIZE);
       if (size != REPLY_SIZE) {
	 fprintf(stderr,"Received reply of length %d, should be %d.\n",
		 size,REPLY_SIZE);
       }
#endif
     }

     t2 = get_seconds();

#ifdef TERSE_OUTPUT
     fprintf(stderr,"%d %d %d %f %f %f\n",
     	NUM_WINDOWS, PINGPONG_SIZE, WINDOW_SIZE, 
	(t2-t1),
	((t2-t1)*1e6)/(NUM_WINDOWS*WINDOW_SIZE),
	(NUM_WINDOWS*WINDOW_SIZE*PINGPONG_SIZE*8)/(1e6*(t2-t1)));
#else
     fprintf(stderr,"Sent %d %d-byte messages (window size %d) in %f seconds.\n",
     	NUM_WINDOWS*WINDOW_SIZE,PINGPONG_SIZE,WINDOW_SIZE,(t2-t1));
     fprintf(stderr,"%f usec/message or %f Mbit/sec\n",
     	((t2-t1)*1e6)/(NUM_WINDOWS*WINDOW_SIZE),
	(NUM_WINDOWS*WINDOW_SIZE*PINGPONG_SIZE*8)/(1e6*(t2-t1)));
#endif
     	
   } else {

     buffer = (char *)malloc(PINGPONG_SIZE);
     if (!buffer) {
       fprintf(stderr,"Can't malloc buffer\n");
       exit(-1);
     }

     memset(&qos1,0,sizeof(qos1));
     memset(&qos2,0,sizeof(qos2));

     qos1.rxtp.traffic_class = ATM_UBR;
     qos1.rxtp.max_sdu = PINGPONG_SIZE;
     qos2.txtp.traffic_class = ATM_UBR;
     qos2.txtp.max_sdu = REPLY_SIZE;

     if (setsockopt(s1,SOL_ATM,SO_ATMQOS,&qos1,sizeof(qos1)) < 0) {
       perror("setsockopt SO_ATMQOS1");
       exit(-1);
     }
     if (setsockopt(s2,SOL_ATM,SO_ATMQOS,&qos2,sizeof(qos2)) < 0) {
       perror("setsockopt SO_ATMQOS2");
       exit(-1);
     }


     if (bind(s1, (struct sockaddr *)&addr1, sizeof(addr1)) < 0) {
       perror("bind");
       exit(-1);
     }
     if (connect(s2, (struct sockaddr *)&addr2, sizeof(addr2)) < 0) {
       perror("connect");
       exit(-1);
     }

     i = 0;
     while (i < NUM_WINDOWS*WINDOW_SIZE) {
#ifdef MDW_DEBUG
       fprintf(stderr,"Receiving %d... ",i);
#endif

       size = read(s1, buffer, PINGPONG_SIZE);

#ifdef MDW_DEBUG
       fprintf(stderr,"received!\n");
#endif


       if (size == PINGPONG_SIZE) {
	 i++;
	 if ((i % WINDOW_SIZE) == 0) {
	 /* Send reply */
#ifdef MDW_DEBUG
	   fprintf(stderr,"Sending reply...");
#endif
           (void)write(s2, buffer2, REPLY_SIZE);
#ifdef MDW_DEBUG
	   fprintf(stderr,"sent!\n");
#endif
	 }

       } else {
	 fprintf(stderr,"Received message of length %d, should be %d.\n",
		 size,PINGPONG_SIZE);
       }
     }
   }
   
   return 0;
}
