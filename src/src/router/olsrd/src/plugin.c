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
 * $Id: plugin.c,v 1.19 2005/03/06 19:33:35 kattemat Exp $
 */


#include "olsr_plugin_io.h"
#include <stdio.h>
#include "olsr.h"
#include "defs.h"
#include "parser.h"
#include "scheduler.h"
#include "duplicate_set.h"
#include "link_set.h"
#include "mantissa.h"
#include "local_hna_set.h"
#include "socket_parser.h"
#include "neighbor_table.h"
#include "link_set.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "hna_set.h"
#include "apm.h"
#include "routing_table.h"
#include "mid_set.h"
#include "mpr_selector_set.h"
#include "plugin_loader.h"

/**
 * Multi-purpose function for plugins
 * Syntax much the same as the ioctl(2) call 
 *
 *@param cmd the command
 *@param data pointer to memory to put/get data
 *@param size size of the memory pointed to
 *
 *@return negative if unknown command
 */

int
olsr_plugin_io(int cmd, void *data, size_t size)
{
  void *ptr;

  OLSR_PRINTF(3, "olsr_plugin_io(%d)\n", cmd)

  switch(cmd)
    {

      /* Data fetching */
    case(GETD__IFNET):
      *((struct interface **)data) = ifnet;
      break;
    case(GETD__NOW):
      *((struct timeval **)data) = &now;
      break;
    case(GETD__PARSER_ENTRIES):
      *((struct parse_function_entry **)data) = parse_functions;
      break;
    case(GETD__OLSR_SOCKET_ENTRIES):
      *((struct olsr_socket_entry **)data) = olsr_socket_entries;
      break;
    case(GETD__NEIGHBORTABLE):
      *((struct neighbor_entry **)data) = neighbortable;
      break;
    case(GETD__TWO_HOP_NEIGHBORTABLE):
      *((struct neighbor_2_entry **)data) = two_hop_neighbortable;
      break;
     case(GETD__TC_TABLE):
      *((struct tc_entry **)data) = tc_table;
      break;
     case(GETD__HNA_SET):
      *((struct hna_entry **)data) = hna_set;
      break;
     case(GETD__OLSR_CNF):
      *((struct olsrd_config **)data) = olsr_cnf;
      break;
      case(GETD__ROUTINGTABLE):
      *((struct rt_entry **)data) = routingtable;
      break;
      case(GETD__HNA_ROUTES):
      *((struct rt_entry **)data) = hna_routes;
      break; 
     case(GETD__MID_SET):
      *((struct mid_entry **)data) = mid_set;
      break;
     case(GETD__LINK_SET):
      *((struct link_entry **)data) = get_link_set();
      break;
      
      /* Function fetching */
      
    case(GETF__OLSR_PRINTF):
      ptr = &olsr_printf;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_MALLOC):
      ptr = &olsr_malloc;
      memcpy(data, &ptr, size);
      break;
    case(GETF__DOUBLE_TO_ME):
      ptr = &double_to_me;
      memcpy(data, &ptr, size);
      break;
    case(GETF__ME_TO_DOUBLE):
      ptr = &me_to_double;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_REGISTER_SCHEDULER_EVENT):
      ptr = &olsr_register_scheduler_event;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_REMOVE_SCHEDULER_EVENT):
      ptr = &olsr_remove_scheduler_event;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_PARSER_ADD_FUNCTION):
      ptr = &olsr_parser_add_function;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_PARSER_REMOVE_FUNCTION):
      ptr = &olsr_parser_remove_function;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_REGISTER_TIMEOUT_FUNCTION):
      ptr = &olsr_register_timeout_function;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_REMOVE_TIMEOUT_FUNCTION):
      ptr = &olsr_remove_timeout_function;
      memcpy(data, &ptr, size);
      break;
    case(GETF__GET_MSG_SEQNO):
      ptr = &get_msg_seqno;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_CHECK_DUP_TABLE_PROC):
      ptr = &olsr_check_dup_table_proc;
      memcpy(data, &ptr, size);
      break;
    case(GETF__NET_OUTPUT):
      ptr =  &net_output;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_FORWARD_MESSAGE):
      ptr = &olsr_forward_message;
      memcpy(data, &ptr, size);
      break;
    case(GETF__ADD_OLSR_SOCKET):
      ptr = &add_olsr_socket;
      memcpy(data, &ptr, size);
      break;
    case(GETF__REMOVE_OLSR_SOCKET):
      ptr = &remove_olsr_socket;
      memcpy(data, &ptr, size);
      break;
    case(GETF__CHECK_NEIGHBOR_LINK):
      ptr = &check_neighbor_link;
      memcpy(data, &ptr, size);
      break;
    case(GETF__ADD_LOCAL_HNA4_ENTRY):
      ptr = &add_local_hna4_entry;
      memcpy(data, &ptr, size);
      break;
    case(GETF__REMOVE_LOCAL_HNA4_ENTRY):
      ptr = &remove_local_hna4_entry;
      memcpy(data, &ptr, size);
      break;
    case(GETF__ADD_LOCAL_HNA6_ENTRY):
      ptr = &add_local_hna6_entry;
      memcpy(data, &ptr, size);
      break;
    case(GETF__REMOVE_LOCAL_HNA6_ENTRY):
      ptr = &remove_local_hna6_entry;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_INPUT):
      ptr = &olsr_input;
      memcpy(data, &ptr, size);
      break;
    case(GETF__ADD_PTF):
      ptr = &add_ptf;
      memcpy(data, &ptr, size);
      break;
    case(GETF__DEL_PTF):
      ptr = &del_ptf;
      memcpy(data, &ptr, size);
      break;
    case(GETF__IF_IFWITHSOCK):
      ptr = &if_ifwithsock;
      memcpy(data, &ptr, size);
      break;
    case(GETF__IF_IFWITHADDR):
      ptr = &if_ifwithaddr;
      memcpy(data, &ptr, size);
      break;
    case(GETF__PARSE_PACKET):
      ptr = &parse_packet;
      memcpy(data, &ptr, size);
      break;
    case(GETF__REGISTER_PCF):
      ptr = &register_pcf;
      memcpy(data, &ptr, size);
      break;
    case(GETF__OLSR_HASHING):
      ptr = &olsr_hashing;
      memcpy(data, &ptr, size);
      break;
    case(GETF__ADD_IFCHGF):
      ptr = &add_ifchgf;
      memcpy(data, &ptr, size);
      break;
    case(GETF__DEL_IFCHGF):
      ptr = &del_ifchgf;
      memcpy(data, &ptr, size);
      break;
    case(GETF__APM_READ):
      ptr = &apm_read;
      memcpy(data, &ptr, size);
      break;
    case(GETF__NET_OUTBUFFER_PUSH):
      ptr = &net_outbuffer_push;
      memcpy(data, &ptr, size);
      break;
    case(GETF__NET_RESERVE_BUFSPACE):
      ptr = &net_reserve_bufspace;
      memcpy(data, &ptr, size);
      break;
    case(GETF__NET_OUTBUFFER_PUSH_RESERVED):
      ptr = &net_outbuffer_push_reserved;
      memcpy(data, &ptr, size);
      break;

    case(GETF__OLSR_LOOKUP_MPRS_SET):
      ptr = &olsr_lookup_mprs_set;
      memcpy(data, &ptr, size);
      break;

    default:
      return -1;
    }

  return 1;
}
