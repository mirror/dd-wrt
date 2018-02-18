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

#include "tc_set.h"
#include "link_set.h"
#include "lq_plugin.h"
#include "olsr_spf.h"
#include "lq_packet.h"
#include "packet.h"
#include "olsr.h"
#include "lq_plugin_default_ffeth.h"
#include "parser.h"
#include "fpm.h"
#include "mid_set.h"
#include "scheduler.h"
#include "log.h"

#define LQ_PLUGIN_LC_MULTIPLIER 1024
#define LQ_PLUGIN_RELEVANT_COSTCHANGE_FF 16

static void default_lq_initialize_ffeth(void);

static olsr_linkcost default_lq_calc_cost_ffeth(const void *lq);

static void default_lq_packet_loss_worker_ffeth(struct link_entry *link, void *lq, bool lost);
static void default_lq_memorize_foreign_hello_ffeth(void *local, void *foreign);

static int default_lq_serialize_hello_lq_pair_ffeth(unsigned char *buff, void *lq);
static void default_lq_deserialize_hello_lq_pair_ffeth(const uint8_t ** curr, void *lq);
static int default_lq_serialize_tc_lq_pair_ffeth(unsigned char *buff, void *lq);
static void default_lq_deserialize_tc_lq_pair_ffeth(const uint8_t ** curr, void *lq);

static void default_lq_copy_link2neigh_ffeth(void *t, void *s);
static void default_lq_copy_link2tc_ffeth(void *target, void *source);
static void default_lq_clear_ffeth(void *target);
static void default_lq_clear_ffeth_hello(void *target);

static const char *default_lq_print_ffeth(void *ptr, char separator, struct lqtextbuffer *buffer);
static double default_lq_get_cost_scaled(olsr_linkcost cost);

/* etx lq plugin (freifunk fpm version) settings */
struct lq_handler lq_etx_ffeth_handler = {
  &default_lq_initialize_ffeth,
  &default_lq_calc_cost_ffeth,
  &default_lq_calc_cost_ffeth,

  &default_lq_packet_loss_worker_ffeth,

  &default_lq_memorize_foreign_hello_ffeth,
  &default_lq_copy_link2neigh_ffeth,
  &default_lq_copy_link2tc_ffeth,
  &default_lq_clear_ffeth_hello,
  &default_lq_clear_ffeth,

  &default_lq_serialize_hello_lq_pair_ffeth,
  &default_lq_serialize_tc_lq_pair_ffeth,
  &default_lq_deserialize_hello_lq_pair_ffeth,
  &default_lq_deserialize_tc_lq_pair_ffeth,

  &default_lq_print_ffeth,
  &default_lq_print_ffeth,
  &default_lq_get_cost_scaled,

  sizeof(struct default_lq_ffeth_hello),
  sizeof(struct default_lq_ffeth),
  4,
  4
};

static void
default_lq_ffeth_handle_lqchange(void) {
  struct default_lq_ffeth_hello *lq;
  struct ipaddr_str buf;
  struct link_entry *link;

  bool triggered = false;

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    bool relevant = false;
    lq = (struct default_lq_ffeth_hello *)link->linkquality;

    if (lq->smoothed_lq.valueLq < lq->lq.valueLq) {
      if (lq->lq.valueLq >= 254 || lq->lq.valueLq - lq->smoothed_lq.valueLq > lq->smoothed_lq.valueLq/10) {
        relevant = true;
      }
    }
    else if (lq->smoothed_lq.valueLq > lq->lq.valueLq) {
      if (lq->smoothed_lq.valueLq - lq->lq.valueLq > lq->smoothed_lq.valueLq/10) {
        relevant = true;
      }
    }
    if (lq->smoothed_lq.valueNlq < lq->lq.valueNlq) {
      if (lq->lq.valueNlq >= 254 || lq->lq.valueNlq - lq->smoothed_lq.valueNlq > lq->smoothed_lq.valueNlq/10) {
        relevant = true;
      }
    }
    else if (lq->smoothed_lq.valueNlq > lq->lq.valueNlq) {
      if (lq->smoothed_lq.valueNlq - lq->lq.valueNlq > lq->smoothed_lq.valueNlq/10) {
        relevant = true;
      }
    }

    if (relevant) {
      memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct default_lq_ffeth));
      link->linkcost = default_lq_calc_cost_ffeth(&lq->smoothed_lq);
      triggered = true;
    }
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link)

  if (!triggered) {
    return;
  }

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    lq = (struct default_lq_ffeth_hello *)link->linkquality;

    if (lq->smoothed_lq.valueLq >= 254 && lq->smoothed_lq.valueNlq >= 254) {
      continue;
    }

    if (lq->smoothed_lq.valueLq == lq->lq.valueLq && lq->smoothed_lq.valueNlq == lq->lq.valueNlq) {
      continue;
    }

    memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct default_lq_ffeth));
    link->linkcost = default_lq_calc_cost_ffeth(&lq->smoothed_lq);
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link)

  olsr_relevant_linkcost_change();
}

static void
default_lq_parser_ffeth(struct olsr *olsr, struct interface_olsr *in_if, union olsr_ip_addr *from_addr)
{
  const union olsr_ip_addr *main_addr;
  struct link_entry *lnk;
  struct default_lq_ffeth_hello *lq;
  uint32_t seq_diff;

  /* Find main address */
  main_addr = mid_lookup_main_addr(from_addr);

  /* Loopup link entry */
  lnk = lookup_link_entry(from_addr, main_addr, in_if);
  if (lnk == NULL) {
    return;
  }

  lq = (struct default_lq_ffeth_hello *)lnk->linkquality;

  /* ignore double package */
  if (lq->last_seq_nr == olsr->olsr_seqno) {
    struct ipaddr_str buf;
    olsr_syslog(OLSR_LOG_INFO, "detected duplicate packet with seqnr %d from %s on %s (%d Bytes)",
		olsr->olsr_seqno,olsr_ip_to_string(&buf, from_addr),in_if->int_name,ntohs(olsr->olsr_packlen));
    return;
  }

  if (lq->last_seq_nr > olsr->olsr_seqno) {
    seq_diff = (uint32_t) olsr->olsr_seqno + 65536 - lq->last_seq_nr;
  } else {
    seq_diff = olsr->olsr_seqno - lq->last_seq_nr;
  }

  /* Jump in sequence numbers ? */
  if (seq_diff > 256) {
    seq_diff = 1;
  }

  lq->received[lq->activePtr]++;
  lq->total[lq->activePtr] += seq_diff;

  lq->last_seq_nr = olsr->olsr_seqno;
  lq->missed_hellos = 0;
}

static void
default_lq_ffeth_timer(void __attribute__ ((unused)) * context)
{
  struct link_entry *link;

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    struct default_lq_ffeth_hello *tlq = (struct default_lq_ffeth_hello *)link->linkquality;
    fpm ratio;
    int i, received, total;

    received = 0;
    total = 0;

    /* enlarge window if still in quickstart phase */
    if (tlq->windowSize < LQ_FFETH_WINDOW) {
      tlq->windowSize++;
    }
    for (i = 0; i < tlq->windowSize; i++) {
      received += tlq->received[i];
      total += tlq->total[i];
    }

    /* calculate link quality */
    if (total == 0) {
      tlq->lq.valueLq = 0;
    } else {
      // start with link-loss-factor
      ratio = fpmidiv(itofpm(link->loss_link_multiplier), LINK_LOSS_MULTIPLIER);

      /* don't forget missing hellos */
      if (tlq->missed_hellos > 1) {
        uint32_t interval;

        interval = tlq->missed_hellos * link->loss_helloint / 1000;
        if (interval > LQ_FFETH_WINDOW) {
          received = 0;
        }
        else {
          received = (received * (LQ_FFETH_WINDOW - interval)) / LQ_FFETH_WINDOW;
        }
      }

      // calculate received/total factor
      ratio = fpmmuli(ratio, received);
      ratio = fpmidiv(ratio, total);
      ratio = fpmmuli(ratio, 255);

      tlq->lq.valueLq = (uint8_t) (fpmtoi(ratio));
    }

    /* ethernet booster */
    if (link->inter->mode == IF_MODE_ETHER) {
      if (tlq->lq.valueLq > (uint8_t)(0.95 * 255)) {
        tlq->perfect_eth = true;
      }
      else if (tlq->lq.valueLq > (uint8_t)(0.90 * 255)) {
        tlq->perfect_eth = false;
      }

      if (tlq->perfect_eth) {
        tlq->lq.valueLq = 255;
      }
    }
    else if (link->inter->mode != IF_MODE_ETHER && tlq->lq.valueLq > 0) {
      tlq->lq.valueLq--;
    }

    // shift buffer
    tlq->activePtr = (tlq->activePtr + 1) % LQ_FFETH_WINDOW;
    tlq->total[tlq->activePtr] = 0;
    tlq->received[tlq->activePtr] = 0;
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link);

  default_lq_ffeth_handle_lqchange();
}

static void
default_lq_initialize_ffeth(void)
{
  if (olsr_cnf->lq_nat_thresh < 1.0f) {
    fprintf(stderr, "Warning, nat_treshold < 1.0 is more likely to produce loops with etx_ffeth\n");
  }
  olsr_packetparser_add_function(&default_lq_parser_ffeth);
  olsr_start_timer(1000, 0, OLSR_TIMER_PERIODIC, &default_lq_ffeth_timer, NULL, 0);
}

static olsr_linkcost
default_lq_calc_cost_ffeth(const void *ptr)
{
  const struct default_lq_ffeth *lq = ptr;
  olsr_linkcost cost;
  bool ether;
  int lq_int, nlq_int;

  if (lq->valueLq < (unsigned int)(255 * MINIMAL_USEFUL_LQ) || lq->valueNlq < (unsigned int)(255 * MINIMAL_USEFUL_LQ)) {
    return LINK_COST_BROKEN;
  }

  ether = lq->valueLq == 255 && lq->valueNlq == 255;

  lq_int = (int)lq->valueLq;
  if (lq_int > 0 && lq_int < 255) {
    lq_int++;
  }

  nlq_int = (int)lq->valueNlq;
  if (nlq_int > 0 && nlq_int < 255) {
    nlq_int++;
  }
  cost = fpmidiv(itofpm(255 * 255), lq_int * nlq_int);
  if (ether) {
    /* ethernet boost */
    cost /= 10;
  }

  if (cost >= LINK_COST_BROKEN)
    return LINK_COST_BROKEN;
  if (cost == 0)
    return 1;
  return cost;
}

static int
default_lq_serialize_hello_lq_pair_ffeth(unsigned char *buff, void *ptr)
{
  struct default_lq_ffeth *lq = ptr;

  buff[0] = (unsigned char)(0);
  buff[1] = (unsigned char)(0);
  buff[2] = (unsigned char)lq->valueLq;
  buff[3] = (unsigned char)lq->valueNlq;

  return 4;
}

static void
default_lq_deserialize_hello_lq_pair_ffeth(const uint8_t ** curr, void *ptr)
{
  struct default_lq_ffeth *lq = ptr;

  pkt_ignore_u16(curr);
  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
}

static int
default_lq_serialize_tc_lq_pair_ffeth(unsigned char *buff, void *ptr)
{
  struct default_lq_ffeth *lq = ptr;

  buff[0] = (unsigned char)(0);
  buff[1] = (unsigned char)(0);
  buff[2] = (unsigned char)lq->valueLq;
  buff[3] = (unsigned char)lq->valueNlq;

  return 4;
}

static void
default_lq_deserialize_tc_lq_pair_ffeth(const uint8_t ** curr, void *ptr)
{
  struct default_lq_ffeth *lq = ptr;

  pkt_ignore_u16(curr);
  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
}

static void
default_lq_packet_loss_worker_ffeth(struct link_entry *link,
    void __attribute__ ((unused)) *ptr, bool lost)
{
  struct default_lq_ffeth_hello *tlq = (struct default_lq_ffeth_hello *)link->linkquality;

  if (lost) {
    tlq->missed_hellos++;
  }
  return;
}

static void
default_lq_memorize_foreign_hello_ffeth(void *ptrLocal, void *ptrForeign)
{
  struct default_lq_ffeth_hello *local = ptrLocal;
  struct default_lq_ffeth *foreign = ptrForeign;

  if (foreign) {
    local->lq.valueNlq = foreign->valueLq;
  } else {
    local->lq.valueNlq = 0;
  }
}

static void
default_lq_copy_link2neigh_ffeth(void *t, void *s)
{
  struct default_lq_ffeth *target = t;
  struct default_lq_ffeth_hello *source = s;
  *target = source->smoothed_lq;
}

static void
default_lq_copy_link2tc_ffeth(void *t, void *s)
{
  struct default_lq_ffeth *target = t;
  struct default_lq_ffeth_hello *source = s;
  *target = source->smoothed_lq;
}

static void
default_lq_clear_ffeth(void *target)
{
  memset(target, 0, sizeof(struct default_lq_ffeth));
}

static void
default_lq_clear_ffeth_hello(void *target)
{
  struct default_lq_ffeth_hello *local = target;
  int i;

  default_lq_clear_ffeth(&local->lq);
  default_lq_clear_ffeth(&local->smoothed_lq);
  local->windowSize = LQ_FFETH_QUICKSTART_INIT;
  for (i = 0; i < LQ_FFETH_WINDOW; i++) {
    local->total[i] = 3;
  }
}

static const char *
default_lq_print_ffeth(void *ptr, char separator, struct lqtextbuffer *buffer)
{
  struct default_lq_ffeth *lq = ptr;
  int lq_int, nlq_int;

  lq_int = (int)lq->valueLq;
  if (lq_int > 0 && lq_int < 255) {
    lq_int++;
  }

  nlq_int = (int)lq->valueNlq;
  if (nlq_int > 0 && nlq_int < 255) {
    nlq_int++;
  }

  snprintf(buffer->buf, sizeof(buffer->buf), "%s%c%s", fpmtoa(fpmidiv(itofpm(lq_int), 255)), separator,
           fpmtoa(fpmidiv(itofpm(nlq_int), 255)));
  return buffer->buf;
}

static double
default_lq_get_cost_scaled(olsr_linkcost cost)
{
  return fpmtod(cost);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
