/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2008 Henning Rogge <rogge@fgan.de>
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

#ifndef LQ_ETX_FF_
#define LQ_ETX_FF_

#include "olsr_types.h"
#include "lq_plugin.h"

#define LQ_PLUGIN_LC_MULTIPLIER 1024
#define LQ_PLUGIN_RELEVANT_COSTCHANGE_FF 16

#define LQ_ALGORITHM_ETX_FF_NAME "etx_ff"

#define LQ_FF_WINDOW 32
#define LQ_FF_QUICKSTART_INIT 4

struct default_lq_ff {
  olsr_u8_t valueLq;
  olsr_u8_t valueNlq;
};

struct default_lq_ff_hello {
  struct default_lq_ff lq;
	olsr_u8_t windowSize, activePtr;
	olsr_u16_t last_seq_nr;
	olsr_u16_t received[LQ_FF_WINDOW], lost[LQ_FF_WINDOW];
};

void default_lq_initialize_ff(void);

olsr_linkcost default_lq_calc_cost_ff(const void *lq);

olsr_bool default_lq_is_relevant_costchange_ff(olsr_linkcost c1, olsr_linkcost c2);

olsr_linkcost default_lq_packet_loss_worker_ff(struct link_entry *link, void *lq, olsr_bool lost);
void default_lq_memorize_foreign_hello_ff(void *local, void *foreign);

int default_lq_serialize_hello_lq_pair_ff(unsigned char *buff, void *lq);
void default_lq_deserialize_hello_lq_pair_ff(const olsr_u8_t **curr, void *lq);
int default_lq_serialize_tc_lq_pair_ff(unsigned char *buff, void *lq);
void default_lq_deserialize_tc_lq_pair_ff(const olsr_u8_t **curr, void *lq);

void default_lq_copy_link2tc_ff(void *target, void *source);
void default_lq_clear_ff(void *target);
void default_lq_clear_ff_hello(void *target);

const char *default_lq_print_ff(void *ptr, char separator, struct lqtextbuffer *buffer);
const char *default_lq_print_cost_ff(olsr_linkcost cost, struct lqtextbuffer *buffer);

extern struct lq_handler lq_etx_ff_handler;

#endif /*LQ_ETX_FF_*/
