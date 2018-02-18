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
#include "olsr_spf.h"
#include "lq_packet.h"
#include "packet.h"
#include "olsr.h"
#include "lq_plugin_default_float.h"

static void default_lq_initialize_float(void);
static olsr_linkcost default_lq_calc_cost_float(const void *lq);
static void default_lq_packet_loss_worker_float(struct link_entry *link, void *lq, bool lost);
static void default_lq_memorize_foreign_hello_float(void *local, void *foreign);
static int default_lq_serialize_hello_lq_pair_float(unsigned char *buff, void *lq);
static void default_lq_deserialize_hello_lq_pair_float(const uint8_t ** curr, void *lq);
static int default_lq_serialize_tc_lq_pair_float(unsigned char *buff, void *lq);
static void default_lq_deserialize_tc_lq_pair_float(const uint8_t ** curr, void *lq);
static void default_lq_copy_link2tc_float(void *target, void *source);
static void default_lq_clear_float(void *target);
static const char *default_lq_print_float(void *ptr, char separator, struct lqtextbuffer *buffer);
static double default_lq_get_cost_scaled(olsr_linkcost cost);


/* Default lq plugin settings */
struct lq_handler lq_etx_float_handler = {
  &default_lq_initialize_float,

  &default_lq_calc_cost_float,
  &default_lq_calc_cost_float,

  &default_lq_packet_loss_worker_float,
  &default_lq_memorize_foreign_hello_float,
  &default_lq_copy_link2tc_float,
  &default_lq_copy_link2tc_float,
  &default_lq_clear_float,
  &default_lq_clear_float,

  &default_lq_serialize_hello_lq_pair_float,
  &default_lq_serialize_tc_lq_pair_float,
  &default_lq_deserialize_hello_lq_pair_float,
  &default_lq_deserialize_tc_lq_pair_float,

  &default_lq_print_float,
  &default_lq_print_float,
  &default_lq_get_cost_scaled,

  sizeof(struct default_lq_float),
  sizeof(struct default_lq_float),
  4,
  4
};

static void
default_lq_initialize_float(void)
{
  return;
}

static olsr_linkcost
default_lq_calc_cost_float(const void *ptr)
{
  const struct default_lq_float *lq = ptr;
  olsr_linkcost cost;

  if (lq->lq < (float)MINIMAL_USEFUL_LQ || lq->nlq < (float)MINIMAL_USEFUL_LQ) {
    return LINK_COST_BROKEN;
  }

  cost = (olsr_linkcost) (1.0f / (lq->lq * lq->nlq) * (float)LQ_PLUGIN_LC_MULTIPLIER);

  if (cost >= LINK_COST_BROKEN)
    return LINK_COST_BROKEN;
  if (cost == 0) {
    return 1;
  }
  return cost;
}

static int
default_lq_serialize_hello_lq_pair_float(unsigned char *buff, void *ptr)
{
  struct default_lq_float *lq = ptr;

  buff[0] = (unsigned char)(lq->lq * 255);
  buff[1] = (unsigned char)(lq->nlq * 255);
  buff[2] = 0;
  buff[3] = 0;

  return 4;
}

static void
default_lq_deserialize_hello_lq_pair_float(const uint8_t ** curr, void *ptr)
{
  struct default_lq_float *lq = ptr;
  uint8_t lq_value, nlq_value;

  pkt_get_u8(curr, &lq_value);
  pkt_get_u8(curr, &nlq_value);
  pkt_ignore_u16(curr);

  lq->lq = (float)lq_value / 255.0f;
  lq->nlq = (float)nlq_value / 255.0f;
}

static int
default_lq_serialize_tc_lq_pair_float(unsigned char *buff, void *ptr)
{
  struct default_lq_float *lq = ptr;

  buff[0] = (unsigned char)(lq->lq * 255);
  buff[1] = (unsigned char)(lq->nlq * 255);
  buff[2] = 0;
  buff[3] = 0;

  return 4;
}

static void
default_lq_deserialize_tc_lq_pair_float(const uint8_t ** curr, void *ptr)
{
  struct default_lq_float *lq = ptr;
  uint8_t lq_value, nlq_value;

  pkt_get_u8(curr, &lq_value);
  pkt_get_u8(curr, &nlq_value);
  pkt_ignore_u16(curr);

  lq->lq = (float)lq_value / 255.0f;
  lq->nlq = (float)nlq_value / 255.0f;
}

static void
default_lq_packet_loss_worker_float(struct link_entry *link, void *ptr, bool lost)
{
  struct default_lq_float *tlq = ptr;
  float alpha = olsr_cnf->lq_aging;

  if (tlq->quickstart < LQ_QUICKSTART_STEPS) {
    alpha = LQ_QUICKSTART_AGING;        /* fast enough to get the LQ value within 6 Hellos up to 0.9 */
    tlq->quickstart++;
  }
  // exponential moving average
  tlq->lq *= (1 - alpha);
  if (lost == 0) {
    tlq->lq += (alpha * link->loss_link_multiplier / 65536);
  }
  link->linkcost = default_lq_calc_cost_float(ptr);
  olsr_relevant_linkcost_change();
}

static void
default_lq_memorize_foreign_hello_float(void *ptrLocal, void *ptrForeign)
{
  struct default_lq_float *local = ptrLocal;
  struct default_lq_float *foreign = ptrForeign;

  if (foreign) {
    local->nlq = foreign->lq;
  } else {
    local->nlq = 0;
  }
}

static void
default_lq_copy_link2tc_float(void *target, void *source)
{
  memcpy(target, source, sizeof(struct default_lq_float));
}

static void
default_lq_clear_float(void *target)
{
  memset(target, 0, sizeof(struct default_lq_float));
}

static const char *
default_lq_print_float(void *ptr, char separator, struct lqtextbuffer *buffer)
{
  struct default_lq_float *lq = ptr;

  snprintf(buffer->buf, sizeof(struct lqtextbuffer), "%2.3f%c%2.3f", (double)lq->lq, separator, (double)lq->nlq);
  return buffer->buf;
}

static double
default_lq_get_cost_scaled(olsr_linkcost cost)
{
  return ((double) cost) / LQ_PLUGIN_LC_MULTIPLIER;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
