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
 * $Id: olsr.c,v 1.47 2005/11/17 04:25:44 tlopatic Exp $
 */

/**
 * All these functions are global
 */

#include "defs.h"
#include "olsr.h"
#include "link_set.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "duplicate_set.h"
#include "mpr_selector_set.h"
#include "mid_set.h"
#include "mpr.h"
#include "lq_mpr.h"
#include "lq_route.h"
#include "scheduler.h"
#include "apm.h"
#include "misc.h"
#include "neighbor_table.h"
#include "log.h"
#include "lq_packet.h"

#include <stdarg.h>
#include <signal.h>


olsr_bool changes_topology;
olsr_bool changes_neighborhood;
olsr_bool changes_hna;

/**
 * Process changes functions
 */

struct pcf
{
  int (*function)(int, int, int);
  struct pcf *next;
};

static struct pcf *pcf_list;

static olsr_u16_t message_seqno;

/**
 *Initialize the message sequence number as a random value
 */
void
init_msg_seqno()
{
  message_seqno = random() & 0xFFFF;
}

/**
 * Get and increment the message sequence number
 *
 *@return the seqno
 */
inline olsr_u16_t
get_msg_seqno()
{
  return message_seqno++;
}


void
register_pcf(int (*f)(int, int, int))
{
  struct pcf *new_pcf;

  OLSR_PRINTF(1, "Registering pcf function\n")

  new_pcf = olsr_malloc(sizeof(struct pcf), "New PCF");

  new_pcf->function = f;
  new_pcf->next = pcf_list;
  pcf_list = new_pcf;

}


/**
 *Process changes in neighborhood or/and topology.
 *Re-calculates the neighborhooh/topology if there
 *are any updates - then calls the right functions to
 *update the routing table.
 *@return 0
 */
void
olsr_process_changes()
{

  struct pcf *tmp_pc_list;

#ifdef DEBUG
  if(changes_neighborhood)
    OLSR_PRINTF(3, "CHANGES IN NEIGHBORHOOD\n")
  if(changes_topology)
    OLSR_PRINTF(3, "CHANGES IN TOPOLOGY\n")
  if(changes_hna)
    OLSR_PRINTF(3, "CHANGES IN HNA\n")
#endif
  
  if(!changes_neighborhood &&
     !changes_topology &&
     !changes_hna)
    return;

  if (olsr_cnf->debug_level > 0 && olsr_cnf->clear_screen && isatty(1))
  {
      clear_console();
      printf("%s", OLSRD_VERSION_DATE);
  }

  if (changes_neighborhood)
    {
      /* Calculate new mprs, HNA and routing table */
      if (olsr_cnf->lq_level < 1)
        {
          olsr_calculate_mpr();
        }

      else
        {
          olsr_calculate_lq_mpr();
        }

      if (olsr_cnf->lq_level < 2)
        {
          olsr_calculate_routing_table();
          olsr_calculate_hna_routes();
        }

      else
        {
          olsr_calculate_lq_routing_table();
        }
    }
  
  else if (changes_topology)
    {
      /* calculate the routing table and HNA */

      if (olsr_cnf->lq_level < 2)
        {
          olsr_calculate_routing_table();
          olsr_calculate_hna_routes();
        }

      else
        {
          olsr_calculate_lq_routing_table();
        }
    }

  else if (changes_hna)
    {
      /* update HNA routes */

      if (olsr_cnf->lq_level < 2)
        {
          olsr_calculate_hna_routes();
        }

      else
        {
          olsr_calculate_lq_routing_table();
        }
    }
  
  if (olsr_cnf->debug_level > 0)
    {      
      if (olsr_cnf->debug_level > 2) 
        {
          olsr_print_mid_set();
	  
          if (olsr_cnf->debug_level > 3)
            {
              olsr_print_duplicate_table();
              olsr_print_hna_set();
            }
        }
      
      olsr_print_link_set();
      olsr_print_neighbor_table();
      olsr_print_tc_table();
    }

  for(tmp_pc_list = pcf_list; 
      tmp_pc_list != NULL;
      tmp_pc_list = tmp_pc_list->next)
    {
      tmp_pc_list->function(changes_neighborhood,
			    changes_topology,
			    changes_hna);
    }

  changes_neighborhood = OLSR_FALSE;
  changes_topology = OLSR_FALSE;
  changes_hna = OLSR_FALSE;


  return;
}





/**
 *Initialize all the tables used(neighbor,
 *topology, MID,  HNA, MPR, dup).
 *Also initalizes other variables
 */
void
olsr_init_tables()
{
  
  changes_topology = OLSR_FALSE;
  changes_neighborhood = OLSR_FALSE;
  changes_hna = OLSR_FALSE;

  /* Initialize link set */
  olsr_init_link_set();

  /* Initialize duplicate table */
  olsr_init_duplicate_table();

  /* Initialize neighbor table */
  olsr_init_neighbor_table();

  /* Initialize routing table */
  olsr_init_routing_table();

  /* Initialize two hop table */
  olsr_init_two_hop_table();

  /* Initialize old route table */
  olsr_init_old_table();

  /* Initialize topology */
  olsr_init_tc();

  /* Initialize mpr selector table */
  olsr_init_mprs_set();

  /* Initialize MID set */
  olsr_init_mid_set();

  /* Initialize HNA set */
  olsr_init_hna_set();
  
}






/**
 *Check if a message is to be forwarded and forward
 *it if necessary.
 *
 *@param m the OLSR message recieved
 *@param originator the originator of this message
 *@param seqno the seqno of the message
 *
 *@returns positive if forwarded
 */
int
olsr_forward_message(union olsr_message *m, 
		     union olsr_ip_addr *originator, 
		     olsr_u16_t seqno,
		     struct interface *in_if, 
		     union olsr_ip_addr *from_addr)
{
  union olsr_ip_addr *src;
  struct neighbor_entry *neighbor;
  int msgsize;
  struct interface *ifn;


  if(!olsr_check_dup_table_fwd(originator, seqno, &in_if->ip_addr))
    {
#ifdef DEBUG
      OLSR_PRINTF(3, "Message already forwarded!\n")
#endif
      return 0;
    }

  /* Lookup sender address */
  if(!(src = mid_lookup_main_addr(from_addr)))
    src = from_addr;


  if(NULL == (neighbor=olsr_lookup_neighbor_table(src)))
    return 0;

  if(neighbor->status != SYM)
    return 0;

  /* Update duplicate table interface */
  olsr_update_dup_entry(originator, seqno, &in_if->ip_addr);

  
  /* Check MPR */
  if(olsr_lookup_mprs_set(src) == NULL)
    {
#ifdef DEBUG
      OLSR_PRINTF(5, "Forward - sender %s not MPR selector\n", olsr_ip_to_string(src))
#endif
      return 0;
    }


  /* Treat TTL hopcnt */
  if(olsr_cnf->ip_version == AF_INET)
    {
      /* IPv4 */
      m->v4.hopcnt++;
      m->v4.ttl--; 
    }
  else
    {
      /* IPv6 */
      m->v6.hopcnt++;
      m->v6.ttl--; 
    }



  /* Update dup forwarded */
  olsr_set_dup_forward(originator, seqno);

  /* Update packet data */


  msgsize = ntohs(m->v4.olsr_msgsize);

  /* looping trough interfaces */
  for (ifn = ifnet; ifn ; ifn = ifn->int_next) 
    { 
      if(net_output_pending(ifn))
	{
	  /*
	   * Check if message is to big to be piggybacked
	   */
	  if(net_outbuffer_push(ifn, (olsr_u8_t *)m, msgsize) != msgsize)
	    {
	      /* Send */
	      net_output(ifn);
	      /* Buffer message */
	      set_buffer_timer(ifn);
	      
	      if(net_outbuffer_push(ifn, (olsr_u8_t *)m, msgsize) != msgsize)
		{
		  OLSR_PRINTF(1, "Received message to big to be forwarded in %s(%d bytes)!", ifn->int_name, msgsize)
		  olsr_syslog(OLSR_LOG_ERR, "Received message to big to be forwarded on %s(%d bytes)!", ifn->int_name, msgsize);
		}

	    }
	}
      
      else
	{
	  /* No forwarding pending */
	  set_buffer_timer(ifn);
	  
	  if(net_outbuffer_push(ifn, (olsr_u8_t *)m, msgsize) != msgsize)
	    {
	      OLSR_PRINTF(1, "Received message to big to be forwarded in %s(%d bytes)!", ifn->int_name, msgsize)
	      olsr_syslog(OLSR_LOG_ERR, "Received message to big to be forwarded on %s(%d bytes)!", ifn->int_name, msgsize);
	    }
	}
    }

  return 1;

}


void
set_buffer_timer(struct interface *ifn)
{
  float jitter;
      
  /* Set timer */
  jitter = (float) random()/RAND_MAX;
  jitter *= max_jitter;

  fwdtimer[ifn->if_nr] = GET_TIMESTAMP(jitter*1000);

}



void
olsr_init_willingness()
{
  if(olsr_cnf->willingness_auto)
    olsr_register_scheduler_event(&olsr_update_willingness, 
				  NULL, will_int, will_int, NULL);
}

void
olsr_update_willingness(void *foo)
{
  int tmp_will;

  tmp_will = olsr_cnf->willingness;

  /* Re-calculate willingness */
  olsr_cnf->willingness = olsr_calculate_willingness();

  if(tmp_will != olsr_cnf->willingness)
    {
      OLSR_PRINTF(1, "Local willingness updated: old %d new %d\n", tmp_will, olsr_cnf->willingness)
    }
}


/**
 *Calculate this nodes willingness to act as a MPR
 *based on either a fixed value or the power status
 *of the node using APM
 *
 *@return a 8bit value from 0-7 representing the willingness
 */

olsr_u8_t
olsr_calculate_willingness()
{
  struct olsr_apm_info ainfo;

  /* If fixed willingness */
  if(!olsr_cnf->willingness_auto)
    return olsr_cnf->willingness;

  if(apm_read(&ainfo) < 1)
    return WILL_DEFAULT;

  apm_printinfo(&ainfo);

  /* If AC powered */
  if(ainfo.ac_line_status == OLSR_AC_POWERED)
    return 6;

  /* If battery powered 
   *
   * juice > 78% will: 3
   * 78% > juice > 26% will: 2
   * 26% > juice will: 1
   */
  return (ainfo.battery_percentage / 26);
}

const char *
olsr_msgtype_to_string(olsr_u8_t msgtype)
{
  static char type[20];

  switch(msgtype)
    {
    case(HELLO_MESSAGE):
      return "HELLO";
    case(TC_MESSAGE):
      return "TC";
    case(MID_MESSAGE):
      return "MID";
    case(HNA_MESSAGE):
      return "HNA";
    case(LQ_HELLO_MESSAGE):
      return("LQ-HELLO");
    case(LQ_TC_MESSAGE):
      return("LQ-TC");
    default:
      break;
    }

  snprintf(type, 20, "UNKNOWN(%d)", msgtype);
  return type;
}


const char *
olsr_link_to_string(olsr_u8_t linktype)
{
  static char type[20];

  switch(linktype)
    {
    case(UNSPEC_LINK):
      return "UNSPEC";
    case(ASYM_LINK):
      return "ASYM";
    case(SYM_LINK):
      return "SYM";
    case(LOST_LINK):
      return "LOST";
    case(HIDE_LINK):
      return "HIDE";
    default:
      break;
    }

  snprintf(type, 20, "UNKNOWN(%d)", linktype);
  return type;
}


const char *
olsr_status_to_string(olsr_u8_t status)
{
  static char type[20];

  switch(status)
    {
    case(NOT_NEIGH):
      return "NOT NEIGH";
    case(SYM_NEIGH):
      return "NEIGHBOR";
    case(MPR_NEIGH):
      return "MPR";
    default:
      break;
    }

  snprintf(type, 20, "UNKNOWN(%d)", status);
  return type;
}


/**
 *Termination function to be called whenever a error occures
 *that requires the daemon to terminate
 *
 *@param msg the message to write to the syslog and possibly stdout
 */

void
olsr_exit(const char *msg, int val)
{
  OLSR_PRINTF(1, "OLSR EXIT: %s\n", msg)
  olsr_syslog(OLSR_LOG_ERR, "olsrd exit: %s\n", msg);
  fflush(stdout);
  exit_value = val;

  raise(SIGTERM);
}


/**
 *Wrapper for malloc(3) that does error-checking
 *
 *@param size the number of bytes to allocalte
 *@param caller a string identifying the caller for
 *use in error messaging
 *
 *@return a void pointer to the memory allocated
 */
void *
olsr_malloc(size_t size, const char *id)
{
  void *ptr;

  if((ptr = malloc(size)) == 0) 
    {
      OLSR_PRINTF(1, "OUT OF MEMORY: %s\n", strerror(errno))
      olsr_syslog(OLSR_LOG_ERR, "olsrd: out of memory!: %m\n");
      olsr_exit((char *)id, EXIT_FAILURE);
    }
  return ptr;
}


/**
 *Wrapper for printf that prints to a specific
 *debuglevel upper limit
 *
 */

int
olsr_printf(int loglevel, char *format, ...)
{
  va_list arglist;

  if((loglevel <= olsr_cnf->debug_level) && debug_handle)
    {
      va_start(arglist, format);
      
      vfprintf(debug_handle, format, arglist);
      
      va_end(arglist);
    }


  return 0;
}
