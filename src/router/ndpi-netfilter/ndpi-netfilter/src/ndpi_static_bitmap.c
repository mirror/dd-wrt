#include <asm/byteorder.h>
#include <linux/types.h>

#include "ndpi_static_bitmap.h"

int NDPI_BITMASK_IS_EMPTY(const struct ndpi_static_bitmask *a) {
  int i;

  for(i=0; i<NDPI_NUM_FDS_STATIC; i++)
    if(a->fds[i] != 0)
      return(0);

  return(1);
}

int NDPI_COMPARE_PROTOCOL_TO_BITMASK(const struct ndpi_static_bitmask *a,u_int16_t val) {
  u_int16_t i = val;
  if(val >= NDPI_MAX_NUM_STATIC_BITMAP) return 0;
  i = val/sizeof(a->fds[0]);
  val &= sizeof(a->fds[0])-1;
  return (a->fds[i] & (1ul << val)) ? 1:0;
}

void NDPI_ADD_PROTOCOL_TO_BITMASK(struct ndpi_static_bitmask *a,u_int16_t val) {
  u_int16_t i = val;
  if(val >= NDPI_MAX_NUM_STATIC_BITMAP) return;
  i = val/sizeof(a->fds[0]);
  val &= sizeof(a->fds[0])-1;
  a->fds[i] |= 1ul << val;
}

void NDPI_DEL_PROTOCOL_FROM_BITMASK(struct ndpi_static_bitmask *a,u_int16_t val) {
  u_int16_t i = val;
  if(val >= NDPI_MAX_NUM_STATIC_BITMAP) return;
  i = val/sizeof(a->fds[0]);
  val &= sizeof(a->fds[0])-1;
  a->fds[i] &= ~(1ul << val);
}

