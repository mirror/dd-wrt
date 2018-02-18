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
#include "lq_plugin_default_fpm.h"

static void default_lq_initialize_fpm(void);
static olsr_linkcost default_lq_calc_cost_fpm(const void *lq);
static void default_lq_packet_loss_worker_fpm(struct link_entry *link, void *lq, bool lost);
static void default_lq_memorize_foreign_hello_fpm(void *local, void *foreign);
static int default_lq_serialize_hello_lq_pair_fpm(unsigned char *buff, void *lq);
static void default_lq_deserialize_hello_lq_pair_fpm(const uint8_t ** curr, void *lq);
static int default_lq_serialize_tc_lq_pair_fpm(unsigned char *buff, void *lq);
static void default_lq_deserialize_tc_lq_pair_fpm(const uint8_t ** curr, void *lq);
static void default_lq_copy_link2tc_fpm(void *target, void *source);
static void default_lq_clear_fpm(void *target);
static const char *default_lq_print_fpm(void *ptr, char separator, struct lqtextbuffer *buffer);
static double default_lq_get_cost_scaled(olsr_linkcost cost);


/* etx lq plugin (fpm version) settings */
struct lq_handler lq_etx_fpm_handler = {
  &default_lq_initialize_fpm,

  &default_lq_calc_cost_fpm,
  &default_lq_calc_cost_fpm,

  &default_lq_packet_loss_worker_fpm,
  &default_lq_memorize_foreign_hello_fpm,
  &default_lq_copy_link2tc_fpm,
  &default_lq_copy_link2tc_fpm,
  &default_lq_clear_fpm,
  &default_lq_clear_fpm,

  &default_lq_serialize_hello_lq_pair_fpm,
  &default_lq_serialize_tc_lq_pair_fpm,
  &default_lq_deserialize_hello_lq_pair_fpm,
  &default_lq_deserialize_tc_lq_pair_fpm,

  &default_lq_print_fpm,
  &default_lq_print_fpm,
  &default_lq_get_cost_scaled,

  sizeof(struct default_lq_fpm),
  sizeof(struct default_lq_fpm),
  4,
  4
};

uint32_t aging_factor_new, aging_factor_old;
uint32_t aging_quickstart_new, aging_quickstart_old;

static void
default_lq_initialize_fpm(void)
{
  aging_factor_new = (uint32_t) (olsr_cnf->lq_aging * LQ_FPM_INTERNAL_MULTIPLIER);
  aging_factor_old = LQ_FPM_INTERNAL_MULTIPLIER - aging_factor_new;

  aging_quickstart_new = (uint32_t) (LQ_QUICKSTART_AGING * LQ_FPM_INTERNAL_MULTIPLIER);
  aging_quickstart_old = LQ_FPM_INTERNAL_MULTIPLIER - aging_quickstart_new;
}

static olsr_linkcost
default_lq_calc_cost_fpm(const void *ptr)
{
  const struct default_lq_fpm *lq = ptr;
  olsr_linkcost cost;

  if (lq->valueLq < (unsigned int)(255 * MINIMAL_USEFUL_LQ) || lq->valueNlq < (unsigned int)(255 * MINIMAL_USEFUL_LQ)) {
    return LINK_COST_BROKEN;
  }

  cost = LQ_FPM_LINKCOST_MULTIPLIER * 255 / (int)lq->valueLq * 255 / (int)lq->valueNlq;

  if (cost >= LINK_COST_BROKEN)
    return LINK_COST_BROKEN;
  if (cost == 0)
    return 1;
  return cost;
}

static int
default_lq_serialize_hello_lq_pair_fpm(unsigned char *buff, void *ptr)
{
  struct default_lq_fpm *lq = ptr;

  buff[0] = (unsigned char)lq->valueLq;
  buff[1] = (unsigned char)lq->valueNlq;
  buff[2] = (unsigned char)(0);
  buff[3] = (unsigned char)(0);

  return 4;
}

static void
default_lq_deserialize_hello_lq_pair_fpm(const uint8_t ** curr, void *ptr)
{
  struct default_lq_fpm *lq = ptr;

  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
  pkt_ignore_u16(curr);
}

static int
default_lq_serialize_tc_lq_pair_fpm(unsigned char *buff, void *ptr)
{
  struct default_lq_fpm *lq = ptr;

  buff[0] = (unsigned char)lq->valueLq;
  buff[1] = (unsigned char)lq->valueNlq;
  buff[2] = (unsigned char)(0);
  buff[3] = (unsigned char)(0);

  return 4;
}

static void
default_lq_deserialize_tc_lq_pair_fpm(const uint8_t ** curr, void *ptr)
{
  struct default_lq_fpm *lq = ptr;

  pkt_get_u8(curr, &lq->valueLq);
  pkt_get_u8(curr, &lq->valueNlq);
  pkt_ignore_u16(curr);
}

static void
default_lq_packet_loss_worker_fpm(struct link_entry *link __attribute__ ((unused)), void *ptr, bool lost)
{
  struct default_lq_fpm *tlq = ptr;
  uint32_t alpha_old = aging_factor_old;
  uint32_t alpha_new = aging_factor_new;

  uint32_t value;

  if (tlq->quickstart < LQ_QUICKSTART_STEPS) {
    alpha_new = aging_quickstart_new;
    alpha_old = aging_quickstart_old;
    tlq->quickstart++;
  }
  // exponential moving average
  value = (uint32_t) (tlq->valueLq) * LQ_FPM_INTERNAL_MULTIPLIER / 255;

  value = (value * alpha_old + LQ_FPM_INTERNAL_MULTIPLIER - 1) / LQ_FPM_INTERNAL_MULTIPLIER;

  if (!lost) {
    uint32_t ratio;

    ratio = (alpha_new * link->loss_link_multiplier + LINK_LOSS_MULTIPLIER - 1) / LINK_LOSS_MULTIPLIER;
    value += ratio;
  }
  tlq->valueLq = (value * 255 + LQ_FPM_INTERNAL_MULTIPLIER - 1) / LQ_FPM_INTERNAL_MULTIPLIER;

  link->linkcost = default_lq_calc_cost_fpm(ptr);
  olsr_relevant_linkcost_change();
}

static void
default_lq_memorize_foreign_hello_fpm(void *ptrLocal, void *ptrForeign)
{
  struct default_lq_fpm *local = ptrLocal;
  struct default_lq_fpm *foreign = ptrForeign;

  if (foreign) {
    local->valueNlq = foreign->valueLq;
  } else {
    local->valueNlq = 0;
  }
}

static void
default_lq_copy_link2tc_fpm(void *target, void *source)
{
  memcpy(target, source, sizeof(struct default_lq_fpm));
}

static void
default_lq_clear_fpm(void *target)
{
  memset(target, 0, sizeof(struct default_lq_fpm));
}

static const char *
default_lq_print_fpm(void *ptr, char separator, struct lqtextbuffer *buffer)
{
  struct default_lq_fpm *lq = ptr;

  snprintf(buffer->buf, sizeof(buffer->buf), "%0.3f%c%0.3f", (double)(lq->valueLq) / (double)255.0, separator,
		  (double)(lq->valueNlq) / (double)255.0);
  return buffer->buf;
}

static double
default_lq_get_cost_scaled(olsr_linkcost cost)
{
  return ((double) cost) / LQ_FPM_LINKCOST_MULTIPLIER;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
