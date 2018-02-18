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

#ifdef __linux__
#ifdef LINUX_NL80211 /* Optional - not supported on all platforms */

#include "lq_plugin_ffeth_nl80211.h"
#include "tc_set.h"
#include "link_set.h"
#include "lq_plugin.h"
#include "olsr_spf.h"
#include "lq_packet.h"
#include "packet.h"
#include "olsr.h"
#include "parser.h"
#include "fpm.h"
#include "mid_set.h"
#include "scheduler.h"
#include "log.h"

#ifdef LINUX_NL80211
#include "nl80211_link_info.h"
#define WEIGHT_ETX			50
#define WEIGHT_BANDWIDTH	50
#endif

#define LQ_PLUGIN_LC_MULTIPLIER 1024
#define LQ_PLUGIN_RELEVANT_COSTCHANGE_FF 16

static void lq_initialize_ffeth_nl80211(void);

static olsr_linkcost lq_calc_cost_ffeth_nl80211(const void *lq);

static void lq_packet_loss_worker_ffeth_nl80211(struct link_entry *link, void *lq, bool lost);
static void lq_memorize_foreign_hello_ffeth_nl80211(void *local, void *foreign);

static int lq_serialize_hello_lq_pair_ffeth_nl80211(unsigned char *buff, void *lq);
static void lq_deserialize_hello_lq_pair_ffeth_nl80211(const uint8_t ** curr, void *lq);
static int lq_serialize_tc_lq_pair_ffeth_nl80211(unsigned char *buff, void *lq);
static void lq_deserialize_tc_lq_pair_ffeth_nl80211(const uint8_t ** curr, void *lq);

static void lq_copy_link2neigh_ffeth_nl80211(void *t, void *s);
static void lq_copy_link2tc_ffeth_nl80211(void *target, void *source);
static void lq_clear_ffeth_nl80211(void *target);
static void lq_clear_ffeth_nl80211_hello(void *target);

static const char *lq_print_ffeth_nl80211(void *ptr, char separator, struct lqtextbuffer *buffer);
static const char *lq_print_cost_ffeth_nl80211(olsr_linkcost cost, struct lqtextbuffer *buffer);

/* etx lq plugin (freifunk fpm version) settings */
struct lq_handler lq_etx_ffeth_nl80211_handler = {
  &lq_initialize_ffeth_nl80211,
  &lq_calc_cost_ffeth_nl80211,
  &lq_calc_cost_ffeth_nl80211,

  &lq_packet_loss_worker_ffeth_nl80211,

  &lq_memorize_foreign_hello_ffeth_nl80211,
  &lq_copy_link2neigh_ffeth_nl80211,
  &lq_copy_link2tc_ffeth_nl80211,
  &lq_clear_ffeth_nl80211_hello,
  &lq_clear_ffeth_nl80211,

  &lq_serialize_hello_lq_pair_ffeth_nl80211,
  &lq_serialize_tc_lq_pair_ffeth_nl80211,
  &lq_deserialize_hello_lq_pair_ffeth_nl80211,
  &lq_deserialize_tc_lq_pair_ffeth_nl80211,

  &lq_print_ffeth_nl80211,
  &lq_print_ffeth_nl80211,
  &lq_print_cost_ffeth_nl80211,

  sizeof(struct lq_ffeth_hello),
  sizeof(struct lq_ffeth),
  4,
  4
};

static void
lq_ffeth_nl80211_handle_lqchange(void) {
  struct lq_ffeth_hello *lq;
  struct ipaddr_str buf;
  struct link_entry *link;

  bool triggered = false;

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    bool relevant = false;
    lq = (struct lq_ffeth_hello *)link->linkquality;

#if 0
  fprintf(stderr, "%s: old = %u/%u   new = %u/%u\n", olsr_ip_to_string(&buf, &link->neighbor_iface_addr),
      lq->smoothed_lq.valueLq, lq->smoothed_lq.valueNlq,
      lq->lq.valueLq, lq->lq.valueNlq);
#endif

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
      memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct lq_ffeth));
      link->linkcost = lq_calc_cost_ffeth_nl80211(&lq->smoothed_lq);
      triggered = true;
    }
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link)

  if (!triggered) {
    return;
  }

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    lq = (struct lq_ffeth_hello *)link->linkquality;

    if (lq->smoothed_lq.valueLq >= 254 && lq->smoothed_lq.valueNlq >= 254) {
      continue;
    }

    if (lq->smoothed_lq.valueLq == lq->lq.valueLq && lq->smoothed_lq.valueNlq == lq->lq.valueNlq) {
      continue;
    }

    memcpy(&lq->smoothed_lq, &lq->lq, sizeof(struct lq_ffeth));
    link->linkcost = lq_calc_cost_ffeth_nl80211(&lq->smoothed_lq);
  } OLSR_FOR_ALL_LINK_ENTRIES_END(link)

  olsr_relevant_linkcost_change();
}

static void
lq_parser_ffeth_nl80211(struct olsr *olsr, struct interface_olsr *in_if, union olsr_ip_addr *from_addr)
{
  const union olsr_ip_addr *main_addr;
  struct link_entry *lnk;
  struct lq_ffeth_hello *lq;
  uint32_t seq_diff;

  /* Find main address */
  main_addr = mid_lookup_main_addr(from_addr);

  /* Loopup link entry */
  lnk = lookup_link_entry(from_addr, main_addr, in_if);
  if (lnk == NULL) {
    return;
  }

  lq = (struct lq_ffeth_hello *)lnk->linkquality;

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
lq_ffeth_nl80211_timer(void __attribute__ ((unused)) * context)
{
  struct link_entry *link;

#ifdef LINUX_NL80211
	nl80211_link_info_get();
#endif

  OLSR_FOR_ALL_LINK_ENTRIES(link) {
    struct lq_ffeth_hello *tlq = (struct lq_ffeth_hello *)link->linkquality;
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

      /* keep missed hello periods in mind (round up hello interval to seconds) */
      if (tlq->missed_hellos > 1) {
        received = received - received * tlq->missed_hellos * link->inter->hello_etime/1000 / LQ_FFETH_WINDOW;
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

  lq_ffeth_nl80211_handle_lqchange();
}

static void
lq_initialize_ffeth_nl80211(void)
{
  if (olsr_cnf->lq_nat_thresh < 1.0f) {
    fprintf(stderr, "Warning, nat_treshold < 1.0 is more likely to produce loops with etx_ffeth\n");
  }
  olsr_packetparser_add_function(&lq_parser_ffeth_nl80211);
  olsr_start_timer(1000, 0, OLSR_TIMER_PERIODIC, &lq_ffeth_nl80211_timer, NULL, 0);

#ifdef LINUX_NL80211
  nl80211_link_info_init();
#endif
}

static olsr_linkcost
lq_calc_cost_ffeth_nl80211(const void *ptr)
{
  const struct lq_ffeth *lq = ptr;
  olsr_linkcost cost;
  bool ether;
  int lq_int, nlq_int;
#ifdef LINUX_NL80211
  fpm nl80211 = itofpm((int) lq->valueBandwidth + lq->valueRSSI);
#endif

  // MINIMAL_USEFUL_LQ is a float, multiplying by 255 converts it to uint8_t
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

#ifdef LINUX_NL80211
  nl80211 = fpmidiv(nl80211, 255);
  cost = fpmidiv(itofpm(255 * 255), lq_int * nlq_int); // 1 / (LQ * NLQ)
  cost = fpmadd(cost, nl80211);
#else
  cost = fpmidiv(itofpm(255 * 255), lq_int * nlq_int); // 1 / (LQ * NLQ)
#endif
  if (ether) {
    /* ethernet boost */
    cost /= 10;
  }

  if (cost > LINK_COST_BROKEN)
    return LINK_COST_BROKEN;
  if (cost == 0)
    return 1;
  return cost;
}

static int
lq_serialize_hello_lq_pair_ffeth_nl80211(unsigned char *buff, void *ptr)
{
  struct lq_ffeth *lq = ptr;

#ifdef LINUX_NL80211
  buff[0] = lq->valueBandwidth;
  buff[1] = lq->valueRSSI;
#else
  buff[0] = (unsigned char)(0);
  buff[1] = (unsigned char)(0);
#endif
  buff[2] = (unsigned char)lq->valueLq;
  buff[3] = (unsigned char)lq->valueNlq;

  return 4;
}

static void
lq_deserialize_hello_lq_pair_ffeth_nl80211(const uint8_t ** curr, void *ptr)
{
  struct lq_ffeth *lq = ptr;

#ifdef LINUX_NL80211
  pkt_get_u8(curr, &lq->valueBandwidth);
  pkt_get_u8(curr, &lq->valueRSSI);
#else
  pkt_ignore_u16(curr);
#endif
  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
}

static int
lq_serialize_tc_lq_pair_ffeth_nl80211(unsigned char *buff, void *ptr)
{
  struct lq_ffeth *lq = ptr;

#ifdef LINUX_NL80211
  buff[0] = lq->valueBandwidth;
  buff[1] = lq->valueRSSI;
#else
  buff[0] = (unsigned char)(0);
  buff[1] = (unsigned char)(0);
#endif
  buff[2] = (unsigned char)lq->valueLq;
  buff[3] = (unsigned char)lq->valueNlq;

  return 4;
}

static void
lq_deserialize_tc_lq_pair_ffeth_nl80211(const uint8_t ** curr, void *ptr)
{
  struct lq_ffeth *lq = ptr;

#ifdef LINUX_NL80211
  pkt_get_u8(curr, &lq->valueBandwidth);
  pkt_get_u8(curr, &lq->valueRSSI);
#else
  pkt_ignore_u16(curr);
#endif
  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
}

static void
lq_packet_loss_worker_ffeth_nl80211(struct link_entry *link,
    void __attribute__ ((unused)) *ptr, bool lost)
{
  struct lq_ffeth_hello *tlq = (struct lq_ffeth_hello *)link->linkquality;

  if (lost) {
    tlq->missed_hellos++;
  }
  return;
}

static void
lq_memorize_foreign_hello_ffeth_nl80211(void *ptrLocal, void *ptrForeign)
{
  struct lq_ffeth_hello *local = ptrLocal;
  struct lq_ffeth *foreign = ptrForeign;

  if (foreign) {
    local->lq.valueNlq = foreign->valueLq;
  } else {
    local->lq.valueNlq = 0;
  }
}

static void
lq_copy_link2neigh_ffeth_nl80211(void *t, void *s)
{
  struct lq_ffeth *target = t;
  struct lq_ffeth_hello *source = s;
  *target = source->smoothed_lq;
}

static void
lq_copy_link2tc_ffeth_nl80211(void *t, void *s)
{
  struct lq_ffeth *target = t;
  struct lq_ffeth_hello *source = s;
  *target = source->smoothed_lq;
}

static void
lq_clear_ffeth_nl80211(void *target)
{
  memset(target, 0, sizeof(struct lq_ffeth));
}

static void
lq_clear_ffeth_nl80211_hello(void *target)
{
  struct lq_ffeth_hello *local = target;
  int i;

  lq_clear_ffeth_nl80211(&local->lq);
  lq_clear_ffeth_nl80211(&local->smoothed_lq);
  local->windowSize = LQ_FFETH_QUICKSTART_INIT;
  for (i = 0; i < LQ_FFETH_WINDOW; i++) {
    local->total[i] = 3;
  }
}

static const char *
lq_print_ffeth_nl80211(void *ptr, char separator, struct lqtextbuffer *buffer)
{
  struct lq_ffeth *lq = ptr;
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

static const char *
lq_print_cost_ffeth_nl80211(olsr_linkcost cost, struct lqtextbuffer *buffer)
{
  snprintf(buffer->buf, sizeof(buffer->buf), "%s", fpmtoa(cost));
  return buffer->buf;
}

#endif /* LINUX_NL80211 */
#endif /* __linux__ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
