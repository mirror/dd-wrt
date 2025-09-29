#ifndef __NDPI_STATIC_BITMASK_H__
#define __NDPI_STATIC_BITMASK_H__

//
// "NDPI_MAX_NUM_STATIC_BITMAP" MUST BE >= "NDPI_NUM_FDS_DISSECTORS"
//

#define NDPI_MAX_NUM_STATIC_BITMAP 640
#define NDPI_NUM_FDS_STATIC ((NDPI_MAX_NUM_STATIC_BITMAP + 1)/sizeof(u_int32_t))

struct ndpi_static_bitmask {
  u_int32_t fds[NDPI_NUM_FDS_STATIC];
};

static int NDPI_BITMASK_IS_EMPTY(const struct ndpi_static_bitmask *a);
static int NDPI_COMPARE_PROTOCOL_TO_BITMASK(const struct ndpi_static_bitmask *a,u_int16_t val);
static void NDPI_ADD_PROTOCOL_TO_BITMASK(struct ndpi_static_bitmask *a,u_int16_t val);
static void NDPI_DEL_PROTOCOL_FROM_BITMASK(struct ndpi_static_bitmask *a,u_int16_t val);

#endif
