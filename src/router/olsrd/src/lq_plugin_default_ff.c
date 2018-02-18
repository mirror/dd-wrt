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
#include "lq_plugin_default_ff.h"
#include "parser.h"
#include "fpm.h"
#include "mid_set.h"
#include "scheduler.h"
#include "log.h"
#include <time.h>

static void default_lq_initialize_ff(void);

static olsr_linkcost default_lq_calc_cost_ff(const void *lq);

static void default_lq_packet_loss_worker_ff(struct link_entry *link, void *lq, bool lost);
static void default_lq_memorize_foreign_hello_ff(void *local, void *foreign);

static int default_lq_serialize_hello_lq_pair_ff(unsigned char *buff, void *lq);
static void default_lq_deserialize_hello_lq_pair_ff(const uint8_t ** curr, void *lq);
static int default_lq_serialize_tc_lq_pair_ff(unsigned char *buff, void *lq);
static void default_lq_deserialize_tc_lq_pair_ff(const uint8_t ** curr, void *lq);

static void default_lq_copy_link2neigh_ff(void *t, void *s);
static void default_lq_copy_link2tc_ff(void *target, void *source);
static void default_lq_clear_ff(void *target);
static void default_lq_clear_ff_hello(void *target);

static const char *default_lq_print_ff(void *ptr, char separator, struct lqtextbuffer *buffer);
static double default_lq_get_cost_scaled(olsr_linkcost cost);

/* etx lq plugin (freifunk fpm version) settings */
struct lq_handler lq_etx_ff_handler = {
  &default_lq_initialize_ff,
  &default_lq_calc_cost_ff,
  &default_lq_calc_cost_ff,

  &default_lq_packet_loss_worker_ff,

  &default_lq_memorize_foreign_hello_ff,
  &default_lq_copy_link2neigh_ff,
  &default_lq_copy_link2tc_ff,
  &default_lq_clear_ff_hello,
  &default_lq_clear_ff,

  &default_lq_serialize_hello_lq_pair_ff,
  &default_lq_serialize_tc_lq_pair_ff,
  &default_lq_deserialize_hello_lq_pair_ff,
  &default_lq_deserialize_tc_lq_pair_ff,

  &default_lq_print_ff,
  &default_lq_print_ff,
  &default_lq_get_cost_scaled,

  sizeof(struct default_lq_ff_hello),
  sizeof(struct default_lq_ff),
  4,
  4
};

static void
default_lq_ff_handle_lqchange(void) {
  struct default_lq_ff_hello *lq;
  struct ipaddr_str buf;
  struct link_entry *link;

  bool triggered = false;

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    bool relevant = false;
    lq = (struct default_lq_ff_hello *)link->linkquality;

    if (lq->smoothed_lq.valueLq < lq->lq.valueLq) {
      if (lq->lq.valueLq == 255 || lq->lq.valueLq - lq->smoothed_lq.valueLq > lq->smoothed_lq.valueLq/10) {
        relevant = true;
      }
    }
    else if (lq->smoothed_lq.valueLq > lq->lq.valueLq) {
      if (lq->smoothed_lq.valueLq - lq->lq.valueLq > lq->smoothed_lq.valueLq/10) {
        relevant = true;
      }
    }
    if (lq->smoothed_lq.valueNlq < lq->lq.valueNlq) {
      if (lq->lq.valueNlq == 255 || lq->lq.valueNlq - lq->smoothed_lq.valueNlq > lq->smoothed_lq.valueNlq/10) {
        relevant = true;
      }
    }
    else if (lq->smoothed_lq.valueNlq > lq->lq.valueNlq) {
      if (lq->smoothed_lq.valueNlq - lq->lq.valueNlq > lq->smoothed_lq.valueNlq/10) {
        relevant = true;
      }
    }

    if (relevant) {
      memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct default_lq_ff));
      link->linkcost = default_lq_calc_cost_ff(&lq->smoothed_lq);
      triggered = true;
    }
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link)

  if (!triggered) {
    return;
  }

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    lq = (struct default_lq_ff_hello *)link->linkquality;

    if (lq->smoothed_lq.valueLq == 255 && lq->smoothed_lq.valueNlq == 255) {
      continue;
    }

    if (lq->smoothed_lq.valueLq == lq->lq.valueLq && lq->smoothed_lq.valueNlq == lq->lq.valueNlq) {
      continue;
    }

    memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct default_lq_ff));
    link->linkcost = default_lq_calc_cost_ff(&lq->smoothed_lq);
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link)

  olsr_relevant_linkcost_change();
}

static void
default_lq_parser_ff(struct olsr *olsr, struct interface_olsr *in_if, union olsr_ip_addr *from_addr)
{
  const union olsr_ip_addr *main_addr;
  struct link_entry *lnk;
  struct default_lq_ff_hello *lq;
  uint32_t seq_diff;

  /* Find main address */
  main_addr = mid_lookup_main_addr(from_addr);

  /* Loopup link entry */
  lnk = lookup_link_entry(from_addr, main_addr, in_if);
  if (lnk == NULL) {
    return;
  }

  lq = (struct default_lq_ff_hello *)lnk->linkquality;

  /* ignore double package */
  if (lq->last_seq_nr == olsr->olsr_seqno) {
    struct ipaddr_str buf;
    olsr_syslog(OLSR_LOG_INFO, "detected duplicate packet with seqnr 0x%x from %s on %s (%d Bytes)",
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
default_lq_ff_timer(void __attribute__ ((unused)) * context)
{
  struct link_entry *link;

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    struct default_lq_ff_hello *tlq = (struct default_lq_ff_hello *)link->linkquality;
    fpm ratio;
    int i, received, total;

    received = 0;
    total = 0;

    /* enlarge window if still in quickstart phase */
    if (tlq->windowSize < LQ_FF_WINDOW) {
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
        if (interval > LQ_FF_WINDOW) {
          received = 0;
        }
        else {
          received = (received * (LQ_FF_WINDOW - interval)) / LQ_FF_WINDOW;
        }
      }

      // calculate received/total factor
      ratio = fpmmuli(ratio, received);
      ratio = fpmidiv(ratio, total);
      ratio = fpmmuli(ratio, 255);

      tlq->lq.valueLq = (uint8_t) (fpmtoi(ratio));
    }

    // shift buffer
    tlq->activePtr = (tlq->activePtr + 1) % LQ_FF_WINDOW;
    tlq->total[tlq->activePtr] = 0;
    tlq->received[tlq->activePtr] = 0;
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link);

  default_lq_ff_handle_lqchange();
}

static void
default_lq_initialize_ff(void)
{
  olsr_packetparser_add_function(&default_lq_parser_ff);
  olsr_start_timer(1000, 0, OLSR_TIMER_PERIODIC, &default_lq_ff_timer, NULL, 0);
}

static olsr_linkcost
default_lq_calc_cost_ff(const void *ptr)
{
  const struct default_lq_ff *lq = ptr;
  olsr_linkcost cost;

  if (lq->valueLq < (unsigned int)(255 * MINIMAL_USEFUL_LQ) || lq->valueNlq < (unsigned int)(255 * MINIMAL_USEFUL_LQ)) {
    return LINK_COST_BROKEN;
  }

  cost = fpmidiv(itofpm(255 * 255), (int)lq->valueLq * (int)lq->valueNlq);

  if (cost >= LINK_COST_BROKEN)
    return LINK_COST_BROKEN;
  if (cost == 0)
    return 1;
  return cost;
}

static int
default_lq_serialize_hello_lq_pair_ff(unsigned char *buff, void *ptr)
{
  struct default_lq_ff *lq = ptr;

  buff[0] = (unsigned char)lq->valueLq;
  buff[1] = (unsigned char)lq->valueNlq;
  buff[2] = (unsigned char)(0);
  buff[3] = (unsigned char)(0);

  return 4;
}

static void
default_lq_deserialize_hello_lq_pair_ff(const uint8_t ** curr, void *ptr)
{
  struct default_lq_ff *lq = ptr;

  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
  pkt_ignore_u16(curr);
}

static int
default_lq_serialize_tc_lq_pair_ff(unsigned char *buff, void *ptr)
{
  struct default_lq_ff *lq = ptr;

  buff[0] = (unsigned char)lq->valueLq;
  buff[1] = (unsigned char)lq->valueNlq;
  buff[2] = (unsigned char)(0);
  buff[3] = (unsigned char)(0);

  return 4;
}

static void
default_lq_deserialize_tc_lq_pair_ff(const uint8_t ** curr, void *ptr)
{
  struct default_lq_ff *lq = ptr;

  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
  pkt_ignore_u16(curr);
}

static void
default_lq_packet_loss_worker_ff(struct link_entry *link,
    void __attribute__ ((unused)) *ptr, bool lost)
{
  struct default_lq_ff_hello *tlq = (struct default_lq_ff_hello *)link->linkquality;

  if (lost) {
    tlq->missed_hellos++;
  }
  return;
}

static void
default_lq_memorize_foreign_hello_ff(void *ptrLocal, void *ptrForeign)
{
  struct default_lq_ff_hello *local = ptrLocal;
  struct default_lq_ff *foreign = ptrForeign;

  if (foreign) {
    local->lq.valueNlq = foreign->valueLq;
  } else {
    local->lq.valueNlq = 0;
  }
}

static void
default_lq_copy_link2neigh_ff(void *t, void *s)
{
  struct default_lq_ff *target = t;
  struct default_lq_ff_hello *source = s;
  *target = source->smoothed_lq;
}

static void
default_lq_copy_link2tc_ff(void *t, void *s)
{
  struct default_lq_ff *target = t;
  struct default_lq_ff_hello *source = s;
  *target = source->smoothed_lq;
}

static void
default_lq_clear_ff(void *target)
{
  memset(target, 0, sizeof(struct default_lq_ff));
}

static void
default_lq_clear_ff_hello(void *target)
{
  struct default_lq_ff_hello *local = target;
  int i;

  default_lq_clear_ff(&local->lq);
  default_lq_clear_ff(&local->smoothed_lq);
  local->windowSize = LQ_FF_QUICKSTART_INIT;
  for (i = 0; i < LQ_FF_WINDOW; i++) {
    local->total[i] = 3;
  }
}

static const char *
default_lq_print_ff(void *ptr, char separator, struct lqtextbuffer *buffer)
{
  struct default_lq_ff *lq = ptr;

  snprintf(buffer->buf, sizeof(buffer->buf), "%s%c%s", fpmtoa(fpmidiv(itofpm((int)lq->valueLq), 255)), separator,
           fpmtoa(fpmidiv(itofpm((int)lq->valueNlq), 255)));
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
