#include "gateway_costs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SCALING_SHIFT_CLASSIC 31
#define SCALING_SHIFT 23

int64_t gw_costs_weigh(const struct costs_weights weights, uint32_t path_cost, uint32_t exitUk, uint32_t exitDk) {
  int64_t costU;
  int64_t costD;
  int64_t costE;

  if (!weights.Detx) {
    /* only consider path costs (classic behaviour) (but scale to 64 bit) */
    return ((int64_t) path_cost) << SCALING_SHIFT_CLASSIC;
  }

  if (!exitUk || !exitDk) {
    /* zero bandwidth */
    return INT64_MAX;
  }

  if ((exitUk == UINT32_MAX) && (exitDk == UINT32_MAX)) {
    /* maximum bandwidth: only consider path costs */
    return path_cost;
  }

  costU = (((int64_t) (1000 * weights.WexitU))    << SCALING_SHIFT) / exitUk;
  costD = (((int64_t) (1000 * weights.WexitD))    << SCALING_SHIFT) / exitDk;
  costE = (((int64_t) (weights.Wetx * path_cost)) << SCALING_SHIFT) / weights.Detx;

  return (costU + costD + costE);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
