/*
 * Copyright (c) 2012
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: qos.h,v 1.4 2012/05/21 21:39:17 karls Exp $ */

#ifndef _QOS_H_
#define _QOS_H_

/*
 * DSCP definitions (bits 2-7)
 *
 * Symbolic DSCP values for setting IP_TOS (ip_tos.dscp) via setsockopt().
 * Defined in RFC 2597.
 *
 * The values correspond to the standard DSCP values and need to be
 * shifted before being used with setsockopt() due to the two lowest
 * IP_TOS bits values having a different purpose (this should be handled
 * automatically by the ip_tos.dscp definition).
 *
 * Some platforms provide similar defines but these have been preshifted
 * and are not directly usable with ip_tos.dscp.
 */

/* numeric values */

#define SOCKS_IP_TOS_DSCP_DEFAULT                  0

#define SOCKS_IP_TOS_DSCP_AF11                           10
#define SOCKS_IP_TOS_DSCP_AF12                           12
#define SOCKS_IP_TOS_DSCP_AF13                           14
#define SOCKS_IP_TOS_DSCP_AF21                           18
#define SOCKS_IP_TOS_DSCP_AF22                           20
#define SOCKS_IP_TOS_DSCP_AF23                           22
#define SOCKS_IP_TOS_DSCP_AF31                           26
#define SOCKS_IP_TOS_DSCP_AF32                           28
#define SOCKS_IP_TOS_DSCP_AF33                           30
#define SOCKS_IP_TOS_DSCP_AF41                           34
#define SOCKS_IP_TOS_DSCP_AF42                           36
#define SOCKS_IP_TOS_DSCP_AF43                           38

#define SOCKS_IP_TOS_DSCP_CS0                             0
#define SOCKS_IP_TOS_DSCP_CS1                             8
#define SOCKS_IP_TOS_DSCP_CS2                            16
#define SOCKS_IP_TOS_DSCP_CS3                            24
#define SOCKS_IP_TOS_DSCP_CS4                            32
#define SOCKS_IP_TOS_DSCP_CS5                            40
#define SOCKS_IP_TOS_DSCP_CS6                            48
#define SOCKS_IP_TOS_DSCP_CS7                            56

#define SOCKS_IP_TOS_DSCP_EF                             46

/* symbolic names */

#define SOCKS_IP_TOS_DSCP_DEFAULT_SYMNAME         "default"

#define SOCKS_IP_TOS_DSCP_AF11_SYMNAME                   "af11"
#define SOCKS_IP_TOS_DSCP_AF12_SYMNAME                   "af12"
#define SOCKS_IP_TOS_DSCP_AF13_SYMNAME                   "af13"
#define SOCKS_IP_TOS_DSCP_AF21_SYMNAME                   "af21"
#define SOCKS_IP_TOS_DSCP_AF22_SYMNAME                   "af22"
#define SOCKS_IP_TOS_DSCP_AF23_SYMNAME                   "af23"
#define SOCKS_IP_TOS_DSCP_AF31_SYMNAME                   "af31"
#define SOCKS_IP_TOS_DSCP_AF32_SYMNAME                   "af32"
#define SOCKS_IP_TOS_DSCP_AF33_SYMNAME                   "af33"
#define SOCKS_IP_TOS_DSCP_AF41_SYMNAME                   "af41"
#define SOCKS_IP_TOS_DSCP_AF42_SYMNAME                   "af42"
#define SOCKS_IP_TOS_DSCP_AF43_SYMNAME                   "af43"

#define SOCKS_IP_TOS_DSCP_CS0_SYMNAME                    "cs0"
#define SOCKS_IP_TOS_DSCP_CS1_SYMNAME                    "cs1"
#define SOCKS_IP_TOS_DSCP_CS2_SYMNAME                    "cs2"
#define SOCKS_IP_TOS_DSCP_CS3_SYMNAME                    "cs3"
#define SOCKS_IP_TOS_DSCP_CS4_SYMNAME                    "cs4"
#define SOCKS_IP_TOS_DSCP_CS5_SYMNAME                    "cs5"
#define SOCKS_IP_TOS_DSCP_CS6_SYMNAME                    "cs6"
#define SOCKS_IP_TOS_DSCP_CS7_SYMNAME                    "cs7"

#define SOCKS_IP_TOS_DSCP_EF_SYMNAME                     "ef"

/*
 * IP Precedence definitions (bits 5-7)
 *
 * RFC 791.
 */

#define SOCKS_IP_TOS_PREC_NETCONTROL              0x07
#define SOCKS_IP_TOS_PREC_INTERNETCONTROL         0x06
#define SOCKS_IP_TOS_PREC_CRITIC_ECP              0x05
#define SOCKS_IP_TOS_PREC_FLASHOVERRIDE           0x04
#define SOCKS_IP_TOS_PREC_FLASH                   0x03
#define SOCKS_IP_TOS_PREC_IMMEDIATE               0x02
#define SOCKS_IP_TOS_PREC_PRIORITY                0x01
#define SOCKS_IP_TOS_PREC_ROUTINE                 0x00

#define SOCKS_IP_TOS_PREC_NETCONTROL_SYMNAME      "netcontrol"
#define SOCKS_IP_TOS_PREC_INTERNETCONTROL_SYMNAME "internetcontrol"
#define SOCKS_IP_TOS_PREC_CRITIC_ECP_SYMNAME      "critic_ecp"
#define SOCKS_IP_TOS_PREC_FLASHOVERRIDE_SYMNAME   "flashoverride"
#define SOCKS_IP_TOS_PREC_FLASH_SYMNAME           "flash"
#define SOCKS_IP_TOS_PREC_IMMEDIATE_SYMNAME       "immediate"
#define SOCKS_IP_TOS_PREC_PRIORITY_SYMNAME        "priority"
#define SOCKS_IP_TOS_PREC_ROUTINE_SYMNAME         "routine"

/*
 * IP TOS definitions (bits 1-4)
 *
 * RFC 791/1349.
 */

#define SOCKS_IP_TOS_TOS_LOWDELAY                 0x08
#define SOCKS_IP_TOS_TOS_THROUGHPUT               0x04
#define SOCKS_IP_TOS_TOS_RELIABILITY              0x02
/*#define SOCKS_IP_TOS_TOS_LOWCOST                0x01*/

#define SOCKS_IP_TOS_TOS_LOWDELAY_SYMNAME         "lowdelay"
#define SOCKS_IP_TOS_TOS_THROUGHPUT_SYMNAME       "throughput"
#define SOCKS_IP_TOS_TOS_RELIABILITY_SYMNAME      "reliability"

#endif /* !_QOS_H_ */
