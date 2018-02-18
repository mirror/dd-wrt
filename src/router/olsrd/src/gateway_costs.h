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

#ifndef GATEWAY_COSTS_H_
#define GATEWAY_COSTS_H_

/*
 * Weighing of the path costs:
 *
 * exitUm = the gateway exit link uplink   bandwidth, in Mbps
 * exitDm = the gateway exit link downlink bandwidth, in Mbps
 * WexitU = the gateway exit link uplink   bandwidth weight   (configured)
 * WexitD = the gateway exit link downlink bandwidth weight   (configured)
 * Wetx   = the ETX path cost weight                          (configured)
 * Detx   = the ETX path cost divider                         (configured)
 *
 *                     WexitU   WexitD   Wetx
 * path_cost_weight =  ------ + ------ + ---- * path_cost
 *                     exitUm   exitDm   Detx
 *
 * Since the gateway exit link bandwidths are in Kbps, the following formula
 * is used to convert them to the desired Mbps:
 *
 *       bwK
 * bwM = ----       bwK = bandwidth in Kbps
 *       1000       bwM = bandwidth in Mbps
 *
 * exitUk = the gateway exit link uplink   bandwidth, in Kbps
 * exitDk = the gateway exit link downlink bandwidth, in Kbps
 *
 *                     1000 * WexitU   1000 * WexitD   Wetx
 * path_cost_weight =  ------------- + ------------- + ---- * path_cost
 *                         exitUk          exitDk      Detx
 *
 *
 * Analysis of the required bit width of the result:
 *
 * exitUk    = [1,   320,000,000] = 29 bits
 * exitDk    = [1,   320,000,000] = 29 bits
 * WexitU    = [1,           255] =  8 bits
 * WexitD    = [1,           255] =  8 bits
 * Wetx      = [1,           255] =  8 bits
 * Detx      = [1, 4,294,967,295] = 32 bits
 * path_cost = [1, 4,294,967,295] = 32 bits
 *
 *                         1000 * 255   1000 * 255   255
 * path_cost_weight(max) = ---------- + ---------- + --- * 4,294,967,295
 *                              1             1       1
 *
 * path_cost_weight(max) = 0x3E418    + 0x3E418    + 0xFEFFFFFF01
 * path_cost_weight(max) = 0xFF0007C731
 *
 * Because we can multiply 0xFF0007C731 by 2^24 without overflowing an
 * unsigned 64 bits number, we could do this to increase accuracy.
 *
 * However, since we also want to implement this in Java, which doesn't support
 * unsigned types, we multiply 0xFF0007C731 by 2^23 without overflowing a
 * signed 64 bits number.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  struct gwtextbuffer {
    char buf[16];
  };

  /**
   * Weigh the path costs and the gateway bandwidth.
   *
   * If the ETX divider is zero, then no weighing is performed and only the path
   * costs are considered (classic behaviour), but scaled to a 64 bit number.
   *
   * If path_cost is the maximum AND path_cost < max_cost_etx_max then
   * the gateway costs are equal to path_cost.
   *
   * @param up true when the relevant interface is up
   * @param path_cost the (ETX) path cost to the gateway
   * to take the calculation shortcut.
   * @param exitUk the gateway exit link uplink bandwidth (in kbps)
   * @param exitDk the gateway exit link downlink bandwidth (in kbps)
   * @return the weighed path cost, INT64_MAX when up is false or when exitUk and/or exitDk are zero
   */
  int64_t gw_costs_weigh(bool up, uint32_t path_cost, uint32_t exitUk, uint32_t exitDk);

  double get_gwcost_scaled(int64_t cost);

  const char * get_gwcost_text(int64_t cost, struct gwtextbuffer *buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GATEWAY_COSTS_H_ */
