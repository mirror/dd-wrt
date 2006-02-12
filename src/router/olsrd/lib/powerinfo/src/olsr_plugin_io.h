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
 * $Id: olsr_plugin_io.h,v 1.5 2004/12/03 20:54:33 kattemat Exp $
 */


/*
 * REVISIONS(starting from 0.4.6):
 * 0.4.5 - 0.4.6 : GETD_S removed. The socket entries now reside within the 
 *                 interface struct.
 *                 Added GETF__ADD_IFCHGF and GETF__DEL_IFCHGF.
 *                 - Andreas
 *         0.4.8 : GETF__APM_READ added
 *                 GETD__OLSR_CNF added
 *                 GETD_PACKET removed
 *                 GETD_MAXMESSAGESIZE removed
 *                 GETD_OUTPUTSIZE removed
 *                 GETF__NET_OUTBUFFER_PUSH added
 *                 GETD__ROUTINGTABLE added
 *                 GETD__HNA_ROUTES added
 *                 GETD__MID_SET added
 *                 GETF__NET_RESERVE_BUFSPACE added 
 *                 GETF__NET_OUTBUFFER_PUSH_RESERVED added
 *                 - Andreas
 */

/*
 * IO commands
 *
 * NAMING CONVENTION:
 * - DATAPOINTERS
 *   Commands to get datapointers MUST have the prefix
 *   GETD__ added to the full name of the variable/pointer
 *   in all upper cases.
 *   Example: A command to get a pointer to a variable called
 *   "myvar" in olsrd must be called GETD__MYVAR
 *
 * - FUNCTIONS
 *   Commands to get pointers to olsrd functions MUST have
 *   the prefix GETF__ added to the full name of the runction
 *   in uppercases.
 *   Example: A command to get a pointer to the function
 *   "my_function" must be named GETF__MY_FUNCTION
 *
 *
 *   New commands can be added - BUT EXISTING COMMANDS MUST
 *   _NEVER_ CHANGE VALUE!
 */

#ifndef _OLSR_PLUGIN_IO
#define _OLSR_PLUGIN_IO

/* Data fetching - starts at 100 (used to anyway) */
#define GETD__IFNET                                102
#define GETD__NOW                                  103
#define GETD__PARSER_ENTRIES                       104
#define GETD__OLSR_SOCKET_ENTRIES                  105
#define GETD__NEIGHBORTABLE                        108
#define GETD__TWO_HOP_NEIGHBORTABLE                109
#define GETD__TC_TABLE                             110
#define GETD__HNA_SET                              111
#define GETD__OLSR_CNF                             112
#define GETD__ROUTINGTABLE                         113
#define GETD__HNA_ROUTES                           114
#define GETD__MID_SET                              115

/* Function fetching - starts at 500 */
#define GETF__OLSR_REGISTER_SCHEDULER_EVENT        500
#define GETF__OLSR_REMOVE_SCHEDULER_EVENT          501
#define GETF__OLSR_PARSER_ADD_FUNCTION             502
#define GETF__OLSR_PARSER_REMOVE_FUNCTION          503
#define GETF__OLSR_REGISTER_TIMEOUT_FUNCTION       504
#define GETF__OLSR_REMOVE_TIMEOUT_FUNCTION         505
#define GETF__GET_MSG_SEQNO                        506
#define GETF__OLSR_CHECK_DUP_TABLE_PROC            507
#define GETF__NET_OUTPUT                           508
#define GETF__OLSR_FORWARD_MESSAGE                 509
#define GETF__ADD_OLSR_SOCKET                      510
#define GETF__REMOVE_OLSR_SOCKET                   511
#define GETF__CHECK_NEIGHBOR_LINK                  512
#define GETF__OLSR_PRINTF                          513
#define GETF__OLSR_MALLOC                          514
#define GETF__DOUBLE_TO_ME                         515
#define GETF__ME_TO_DOUBLE                         516
#define GETF__ADD_LOCAL_HNA4_ENTRY                 517
#define GETF__REMOVE_LOCAL_HNA4_ENTRY              518
#define GETF__ADD_LOCAL_HNA6_ENTRY                 519
#define GETF__REMOVE_LOCAL_HNA6_ENTRY              520
#define GETF__OLSR_INPUT                           521
#define GETF__ADD_PTF                              522
#define GETF__DEL_PTF                              523
#define GETF__IF_IFWITHSOCK                        524
#define GETF__IF_IFWITHADDR                        525
#define GETF__PARSE_PACKET                         526
#define GETF__REGISTER_PCF                         527
#define GETF__OLSR_HASHING                         528
#define GETF__ADD_IFCHGF                           529
#define GETF__DEL_IFCHGF                           530
#define GETF__APM_READ                             531
#define GETF__NET_OUTBUFFER_PUSH                   532
#define GETF__NET_RESERVE_BUFSPACE                 533
#define GETF__NET_OUTBUFFER_PUSH_RESERVED          534

#endif
