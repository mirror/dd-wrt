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
 * Author: Jani Monoses <jani@iv.ro>
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__
/*include the configuration made with configtool*/
#include <pkgconf/net_lwip.h>
/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT          CYGPKG_LWIP_MEM_ALIGNMENT

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE               CYGPKG_LWIP_MEM_SIZE

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           CYGPKG_LWIP_MEMP_NUM_PBUF
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        CYGPKG_LWIP_MEMP_NUM_UDP_PCB
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        CYGPKG_LWIP_MEMP_NUM_TCP_PCB
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN CYGPKG_LWIP_MEMP_NUM_TCP_PCB_LISTEN
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        CYGPKG_LWIP_MEMP_NUM_TCP_SEG
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    CYGPKG_LWIP_MEMP_NUM_SYS_TIMEOUT


/* The following four are used only with the sequential API and can be
   set to 0 if the application only will use the raw API. */
/* MEMP_NUM_NETBUF: the number of struct netbufs. */
#define MEMP_NUM_NETBUF         CYGPKG_LWIP_MEMP_NUM_NETBUF
/* MEMP_NUM_NETCONN: the number of struct netconns. */
#define MEMP_NUM_NETCONN        CYGPKG_LWIP_MEMP_NUM_NETCONN
/* MEMP_NUM_APIMSG: the number of struct api_msg, used for
   communication between the TCP/IP stack and the sequential
   programs. */
#define MEMP_NUM_API_MSG        CYGPKG_LWIP_MEMP_NUM_APIMSG
/* MEMP_NUM_TCPIPMSG: the number of struct tcpip_msg, which is used
   for sequential API communication and incoming packets. Used in
   src/api/tcpip.c. */
#define MEMP_NUM_TCPIP_MSG      CYGPKG_LWIP_MEMP_NUM_TCPIP_MSG

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          CYGPKG_LWIP_PBUF_POOL_SIZE

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       CYGPKG_LWIP_PBUF_POOL_BUFSIZE

/* PBUF_LINK_HLEN: the number of bytes that should be allocated for a
   link level header. */
#define PBUF_LINK_HLEN          CYGPKG_LWIP_PBUF_LINK_HLEN

/* ---------- TCP options ---------- */
#define LWIP_TCP                defined (CYGPKG_LWIP_TCP)
#define TCP_TTL                 CYGPKG_LWIP_TCP_TTL

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         CYGPKG_LWIP_TCP_QUEUE_OOSEQ

/* TCP Maximum segment size. */
#define TCP_MSS                 CYGPKG_LWIP_TCP_MSS

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF            CYGPKG_LWIP_TCP_SND_BUF
#define TCP_SNDLOWAT            TCP_SND_BUF/2

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        CYGPKG_LWIP_TCP_SND_QUEUELEN

/* TCP receive window. */
#define TCP_WND                 CYGPKG_LWIP_TCP_WND

/* Maximum number of retransmissions of data segments. */
#define TCP_MAXRTX              CYGPKG_LWIP_TCP_MAXRTX

/* Maximum number of retransmissions of SYN segments. */
#define TCP_SYNMAXRTX           CYGPKG_LWIP_TCP_SYNMAXRTX

/* ---------- ARP options ---------- */
#define ARP_TABLE_SIZE          CYGPKG_LWIP_ARP_TABLE_SIZE

/* ---------- IP options ---------- */
/* Define IP_FORWARD to 1 if you wish to have the ability to forward
   IP packets across network interfaces. If you are going to run lwIP
   on a device with only one network interface, define this to 0. */
#define IP_FORWARD              CYGPKG_LWIP_IP_FORWARD

/* If defined to 1, IP options are allowed (but not parsed). If
   defined to 0, all packets with IP options are dropped. */
#define IP_OPTIONS              CYGPKG_LWIP_IP_OPTIONS

/* ---------- ICMP options ---------- */
#define ICMP_TTL                CYGPKG_LWIP_ICMP_TTL


/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces.*/

#ifdef CYGPKG_LWIP_DHCP
#define LWIP_DHCP               CYGPKG_LWIP_DHCP

/* 1 if you want to do an ARP check on the offered address
   (recommended). */
#define DHCP_DOES_ARP_CHECK     CYGPKG_LWIP_DHCP_DOES_ARP_CHECK
#endif

/* ---------- UDP options ---------- */
#define LWIP_UDP                CYGPKG_LWIP_UDP
#define UDP_TTL                 CYGPKG_LWIP_UDP_TTL

/* ---------- RAW socket support ---------- */
#define LWIP_RAW                CYGPKG_LWIP_RAW

/* ---------- SLIP options --------- */ 
#define LWIP_SLIP               defined(CYGPKG_LWIP_SLIP)
#define SLIP_DEV                CYGPKG_LWIP_SLIP_DEV

#define LWIP_HAVE_LOOPIF	defined (CYGPKG_LWIP_LOOPIF)
/* ---------- PPP options --------- */  
#define PPP_SUPPORT             defined(CYGPKG_LWIP_PPP)
#define PPP_DEV                 CYGPKG_LWIP_PPP_DEV
#define MD5_SUPPORT             1

#if defined(CYGPKG_LWIP_PPP_PAP_AUTH)
#define PAP_SUPPORT	1
#else
#define PAP_SUPPORT	0
#endif

#if defined(CYGPKG_LWIP_PPP_CHAP_AUTH)
#define CHAP_SUPPORT	1
#else
#define CHAP_SUPPORT	0
#endif

/* ------- Thread priorities ---------------*/
#define TCPIP_THREAD_PRIO       CYGPKG_LWIP_TCPIP_THREAD_PRIORITY
#define SLIPIF_THREAD_PRIO      CYGPKG_LWIP_SLIPIF_THREAD_PRIORITY
#define PPP_THREAD_PRIO         CYGPKG_LWIP_PPP_THREAD_PRIORITY
/* ---------- Statistics options ---------- */
#define LWIP_STATS		defined(CYGPKG_LWIP_STATS)

/* ---------- Debug options ---------- */
#if !defined(CYGPKG_LWIP_ASSERTS)
#define LWIP_NOASSERT
#endif

#if defined(CYGPKG_LWIP_DEBUG)
#define LWIP_DEBUG
#define MEM_DEBUG               DBG_ON
#define MEMP_DEBUG              DBG_ON
#define PBUF_DEBUG              DBG_ON
#define API_LIB_DEBUG   DBG_ON
#define API_MSG_DEBUG   DBG_ON 
#define TCPIP_DEBUG             DBG_ON
#define NETIF_DEBUG             DBG_ON
#define SOCKETS_DEBUG   DBG_ON
#define DEMO_DEBUG              DBG_ON
#define IP_DEBUG                DBG_ON
#define IP_REASS_DEBUG  DBG_ON
#define RAW_DEBUG               DBG_ON
#define ICMP_DEBUG              DBG_ON
#define UDP_DEBUG               DBG_ON
#define TCP_DEBUG               DBG_ON
#define TCP_INPUT_DEBUG         DBG_ON
#define TCP_OUTPUT_DEBUG        DBG_ON
#define TCP_RTO_DEBUG   DBG_ON
#define TCP_CWND_DEBUG  DBG_ON
#define TCP_WND_DEBUG   DBG_ON
#define TCP_FR_DEBUG    DBG_ON
#define TCP_QLEN_DEBUG  DBG_ON
#define TCP_RST_DEBUG   DBG_ON
#define PPP_DEBUG   DBG_ON

#define DBG_TYPES_ON    (DBG_ON|DBG_TRACE|DBG_STATE|DBG_FRESH|DBG_HALT)
#endif



#endif /* __LWIPOPTS_H__ */
