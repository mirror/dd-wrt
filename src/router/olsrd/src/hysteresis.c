/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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
 */

#include <time.h>
#include <stdlib.h>

#include "olsr_protocol.h"
#include "hysteresis.h"
#include "defs.h"
#include "olsr.h"
#include "net_olsr.h"
#include "ipcalc.h"
#include "scheduler.h"

#define hscaling olsr_cnf->hysteresis_param.scaling
#define hhigh    olsr_cnf->hysteresis_param.thr_high
#define hlow     olsr_cnf->hysteresis_param.thr_low

float
olsr_hyst_calc_stability(float old_quality)
{
  return (((1 - hscaling) * old_quality) + hscaling);
}

float
olsr_hyst_calc_instability(float old_quality)
{
  return ((1 - hscaling) * old_quality);
}

int
olsr_process_hysteresis(struct link_entry *entry)
{
  //printf("PROCESSING QUALITY: %f\n", entry->L_link_quality);
  if (entry->L_link_quality > hhigh) {
    if (entry->L_link_pending == 1) {
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "HYST[%s] link set to NOT pending!\n", olsr_ip_to_string(&buf, &entry->neighbor_iface_addr));
      changes_neighborhood = true;
    }

    /* Pending = false */
    entry->L_link_pending = 0;

    if (!TIMED_OUT(entry->L_LOST_LINK_time))
      changes_neighborhood = true;

    /* time = now -1 */
    entry->L_LOST_LINK_time = now_times - 1;

    return 1;
  }

  if (entry->L_link_quality < hlow) {
    if (entry->L_link_pending == 0) {
      struct ipaddr_str buf;
      OLSR_PRINTF(1, "HYST[%s] link set to pending!\n", olsr_ip_to_string(&buf, &entry->neighbor_iface_addr));
      changes_neighborhood = true;
    }

    /* Pending = true */
    entry->L_link_pending = 1;

    if (TIMED_OUT(entry->L_LOST_LINK_time))
      changes_neighborhood = true;

    /* Timer = min (L_time, current time + NEIGHB_HOLD_TIME) */
    entry->L_LOST_LINK_time = MIN(GET_TIMESTAMP(NEIGHB_HOLD_TIME * MSEC_PER_SEC), entry->link_timer->timer_clock);

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
olsr_update_hysteresis_hello(struct link_entry *entry, olsr_reltime htime)
{
  struct ipaddr_str buf;
  OLSR_PRINTF(3, "HYST[%s]: HELLO update vtime %u ms\n", olsr_ip_to_string(&buf, &entry->neighbor_iface_addr), htime + htime / 2);

  olsr_set_timer(&entry->link_hello_timer, htime + htime / 2, OLSR_LINK_HELLO_JITTER, OLSR_TIMER_PERIODIC,
                 &olsr_expire_link_hello_timer, entry, 0);

  return;
}

void
update_hysteresis_incoming(union olsr_ip_addr *remote, struct interface_olsr *local, uint16_t seqno)
{
  struct link_entry *lnk = lookup_link_entry(remote, NULL, local);

  /* Calculate new quality */
  if (lnk != NULL) {
#ifdef DEBUG
    struct ipaddr_str buf;
#endif /* DEBUG */
    lnk->L_link_quality = olsr_hyst_calc_stability(lnk->L_link_quality);
#ifdef DEBUG
    OLSR_PRINTF(3, "HYST[%s]: %f\n", olsr_ip_to_string(&buf, remote), (double)lnk->L_link_quality);
#endif /* DEBUG */

    /*
     * see how many packets we have missed and update the link quality
     * for each missed packet; HELLOs have already been accounted for by
     * the timeout function and the number of missed HELLOs has already
     * been added to olsr_seqno there
     */

    if (lnk->olsr_seqno_valid && (unsigned short)(seqno - lnk->olsr_seqno) < 100)
      while (lnk->olsr_seqno != seqno) {
        lnk->L_link_quality = olsr_hyst_calc_instability(lnk->L_link_quality);
#ifdef DEBUG
        OLSR_PRINTF(5, "HYST[%s] PACKET LOSS! %f\n", olsr_ip_to_string(&buf, remote), (double)lnk->L_link_quality);
#endif /* DEBUG */
        if (lnk->L_link_quality < olsr_cnf->hysteresis_param.thr_low)
          break;

        lnk->olsr_seqno++;
      }

    lnk->olsr_seqno = seqno + 1;
    lnk->olsr_seqno_valid = true;

    //printf("Updating seqno to: %d\n", lnk->olsr_seqno);
  }
  return;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
