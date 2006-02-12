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
 * $Id: hysteresis.c,v 1.16 2005/02/27 10:43:38 kattemat Exp $
 */


#include <time.h>

#include "olsr_protocol.h"
#include "hysteresis.h"
#include "defs.h"
#include "olsr.h"

#define hscaling olsr_cnf->hysteresis_param.scaling
#define hhigh    olsr_cnf->hysteresis_param.thr_high
#define hlow     olsr_cnf->hysteresis_param.thr_low

inline float
olsr_hyst_calc_stability(float old_quality)
{
  return (((1 - hscaling) * old_quality) + hscaling);
}



inline float
olsr_hyst_calc_instability(float old_quality)
{
  return ((1 - hscaling) * old_quality);
}



int
olsr_process_hysteresis(struct link_entry *entry)
{
  clock_t tmp_timer;

  //printf("PROCESSING QUALITY: %f\n", entry->L_link_quality);
  if(entry->L_link_quality > hhigh)
    {
      if(entry->L_link_pending == 1)
	{
	  OLSR_PRINTF(1, "HYST[%s] link set to NOT pending!\n", 
		      olsr_ip_to_string(&entry->neighbor_iface_addr))
	  changes_neighborhood = OLSR_TRUE;
	}

      /* Pending = false */
      entry->L_link_pending = 0;

      if(!TIMED_OUT(entry->L_LOST_LINK_time))
	changes_neighborhood = OLSR_TRUE;

      /* time = now -1 */
      entry->L_LOST_LINK_time = now_times - 1;

      return 1;
    }

  if(entry->L_link_quality < hlow)
    {
      if(entry->L_link_pending == 0)
	{
	  OLSR_PRINTF(1, "HYST[%s] link set to pending!\n", 
		      olsr_ip_to_string(&entry->neighbor_iface_addr))
	  changes_neighborhood = OLSR_TRUE;
	}
      
      /* Pending = true */
      entry->L_link_pending = 1;

      if(TIMED_OUT(entry->L_LOST_LINK_time))
	changes_neighborhood = OLSR_TRUE;

      /* Timer = min (L_time, current time + NEIGHB_HOLD_TIME) */
      //tmp_timer = now;
      //tmp_timer.tv_sec += NEIGHB_HOLD_TIME; /* Takafumi fix */
      tmp_timer = now_times + get_hold_time_neighbor();

	entry->L_LOST_LINK_time = 
	  entry->time > tmp_timer ? tmp_timer : entry->time;

      /* (the link is then considered as lost according to section
	 8.5 and this may produce a neighbor loss).
	 WTF?
      */
      return -1;
    }

  /*
   *If we get here then:
   *(HYST_THRESHOLD_LOW <= entry->L_link_quality <= HYST_THRESHOLD_HIGH)
   */

  /* L_link_pending and L_LOST_LINK_time remain unchanged. */
  return 0;


}

/**
 *Update the hello timeout of a hysteresis link
 *entry
 *
 *@param entry the link entry to update
 *@param htime the hello interval to use
 *
 *@return nada
 */
void
olsr_update_hysteresis_hello(struct link_entry *entry, double htime)
{
#ifdef DEBUG
  OLSR_PRINTF(3, "HYST[%s]: HELLO update vtime %f\n", olsr_ip_to_string(&entry->neighbor_iface_addr), htime*1.5)
#endif
  /* hello timeout = current time + hint time */
  /* SET TIMER TO 1.5 TIMES THE INTERVAL */
  /* Update timer */

  entry->hello_timeout = GET_TIMESTAMP(htime*1500);

  return;
}



void
update_hysteresis_incoming(union olsr_ip_addr *remote, union olsr_ip_addr *local, olsr_u16_t seqno)
{
  struct link_entry *link;

  link = lookup_link_entry(remote, local);

  /* Calculate new quality */      
  if(link != NULL)
    {
      link->L_link_quality = olsr_hyst_calc_stability(link->L_link_quality);
#ifdef DEBUG
      OLSR_PRINTF(3, "HYST[%s]: %0.3f\n", olsr_ip_to_string(remote), link->L_link_quality)
#endif

      /* 
       * see how many packets we have missed and update the link quality
       * for each missed packet; HELLOs have already been accounted for by
       * the timeout function and the number of missed HELLOs has already
       * been added to olsr_seqno there
       */

      if (link->olsr_seqno_valid && 
          (unsigned short)(seqno - link->olsr_seqno) < 100)
	  while (link->olsr_seqno != seqno)
	    {
	      link->L_link_quality = olsr_hyst_calc_instability(link->L_link_quality);
#ifdef DEBUG
	      OLSR_PRINTF(5, "HYST[%s] PACKET LOSS! %0.3f\n",
			  olsr_ip_to_string(remote), link->L_link_quality)
#endif
	      if(link->L_link_quality < olsr_cnf->hysteresis_param.thr_low)
		break;

	      link->olsr_seqno++;
	    }


      link->olsr_seqno = seqno + 1;
      link->olsr_seqno_valid = OLSR_TRUE;

      //printf("Updating seqno to: %d\n", link->olsr_seqno);
    }
  return;
}
