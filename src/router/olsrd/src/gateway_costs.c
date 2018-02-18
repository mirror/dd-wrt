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

#include "gateway_costs.h"
#include "olsr_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#define SCALING_SHIFT_CLASSIC 31
#define SCALING_SHIFT 23
#define MAX_SMARTGW_SPEED 320000000

int64_t gw_costs_weigh(bool up, uint32_t path_cost, uint32_t exitUk, uint32_t exitDk) {
  int64_t costU;
  int64_t costD;
  int64_t costE;

  if (!up) {
    /* interface is down */
    return INT64_MAX;
  }

  if (!olsr_cnf->smart_gw_divider_etx) {
    /* only consider path costs (classic behaviour) (but scale to 64 bit) */
    return ((int64_t) path_cost) << SCALING_SHIFT_CLASSIC;
  }

  if (!exitUk || !exitDk) {
    /* zero bandwidth */
    return INT64_MAX;
  }

  if ((exitUk >= MAX_SMARTGW_SPEED) //
      && (exitDk >= MAX_SMARTGW_SPEED) //
      && (path_cost < olsr_cnf->smart_gw_path_max_cost_etx_max)) {
    /* maximum bandwidth AND below max_cost_etx_max: only consider path costs */
    return path_cost;
  }

  costU = (((int64_t) (1000      * olsr_cnf->smart_gw_weight_exitlink_up  )) << SCALING_SHIFT) / exitUk;
  costD = (((int64_t) (1000      * olsr_cnf->smart_gw_weight_exitlink_down)) << SCALING_SHIFT) / exitDk;
  costE = (((int64_t) (path_cost * olsr_cnf->smart_gw_weight_etx          )) << SCALING_SHIFT) / olsr_cnf->smart_gw_divider_etx;

  return (costU + costD + costE);
}

double get_gwcost_scaled(int64_t cost) {
  if (cost != INT64_MAX) {
    unsigned int shift = !olsr_cnf->smart_gw_divider_etx ? SCALING_SHIFT_CLASSIC : SCALING_SHIFT;
    return ((double) cost) / (1u << shift);
  }

  return (double)cost;
}

const char * get_gwcost_text(int64_t cost, struct gwtextbuffer *buffer) {
  if (cost == INT64_MAX) {
    return "INFINITE";
  }

  snprintf(buffer->buf, sizeof(buffer->buf), "%.3f", get_gwcost_scaled(cost));
  buffer->buf[sizeof(buffer->buf) - 1] = '\0';
  return buffer->buf;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
