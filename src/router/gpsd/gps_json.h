/* gps_json.h - JSON handling for libgps and gpsd
 *
 * By Eric S. Raymond, 2009
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */

#include "json.h"

#define GPS_JSON_COMMAND_MAX	80
#define GPS_JSON_RESPONSE_MAX	4096

#ifdef __cplusplus
extern "C" {
#endif
void json_data_report(const gps_mask_t,
		      const struct gps_device_t *,
		      const struct policy_t *,
		      /*@out@*/char *, size_t);
char *json_stringify(/*@out@*/char *, size_t, /*@in@*/const char *);
void json_tpv_dump(const struct gps_device_t *,
		   const struct policy_t *, /*@out@*/char *, size_t);
void json_noise_dump(const struct gps_data_t *, /*@out@*/char *, size_t);
void json_sky_dump(const struct gps_data_t *, /*@out@*/char *, size_t);
void json_att_dump(const struct gps_data_t *, /*@out@*/char *, size_t);
void json_subframe_dump(const struct gps_data_t *, /*@out@*/ char buf[], size_t);
void json_device_dump(const struct gps_device_t *, /*@out@*/char *, size_t);
void json_watch_dump(const struct policy_t *, /*@out@*/char *, size_t);
int json_watch_read(const char *, /*@out@*/struct policy_t *,
		    /*@null@*/const char **);
int json_device_read(const char *, /*@out@*/struct devconfig_t *,
		     /*@null@*/const char **);
void json_version_dump(/*@out@*/char *, size_t);
void json_aivdm_dump(const struct ais_t *, /*@null@*/const char *, bool,
		     /*@out@*/char *, size_t);
int json_rtcm2_read(const char *, char *, size_t, struct rtcm2_t *,
		    /*@null@*/const char **);
int json_ais_read(const char *, char *, size_t, struct ais_t *,
		  /*@null@*/const char **);
int libgps_json_unpack(const char *, struct gps_data_t *,
		       /*@null@*/const char **);
#ifdef __cplusplus
}
#endif

/* these values don't matter in themselves, they just have to be out-of-band */
#define DEVDEFAULT_BPS  	0
#define DEVDEFAULT_PARITY	'X'
#define DEVDEFAULT_STOPBITS	3
#define DEVDEFAULT_NATIVE	-1

/* gps_json.h ends here */
