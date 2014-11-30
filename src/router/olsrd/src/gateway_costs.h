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

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /**
   * Structure to keep weighing factors for the gw_costs_weigh function
   */
  struct costs_weights {
      uint8_t WexitU;
      uint8_t WexitD;
      uint8_t Wetx;
      uint32_t Detx;
  };

  /**
   * Weigh the path costs and the gateway bandwidth.
   *
   * If the ETX divider is zero, then no weighing is performed and only the path
   * costs are considered (classic behaviour), but scaled to a 64 bit number.
   *
   * @param weights the weights for the calculation
   * @param path_cost the (ETX) path cost to the gateway
   * @param exitUk the gateway exit link uplink bandwidth (in kbps)
   * @param exitDk the gateway exit link downlink bandwidth (in kbps)
   * @return the weighed path cost, INT64_MAX when exitUk and/or exitDk are zero
   */
  int64_t gw_costs_weigh(const struct costs_weights weights, uint32_t path_cost, uint32_t exitUk, uint32_t exitDk);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GATEWAY_COSTS_H_ */
