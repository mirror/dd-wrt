#ifndef _SGWDYNSPEED_SGWDYNSPEED_H
#define _SGWDYNSPEED_SGWDYNSPEED_H

/* Plugin includes */

/* OLSR includes */

/* System includes */
#include <stdbool.h>

/*
 * Global
 */

/** The long plugin name */
#define SGWDYNSPEED_PLUGIN_NAME_LONG			"OLSRD Smart Gateway Dynamic Speed plugin"

/** The short plugin name / abbreviation */
#define SGWDYNSPEED_PLUGIN_ABBR					"SGWDYNSPEED"

/*
 *  Interface
 */

bool initSgwDynSpeed(void);
void stopSgwDynSpeed(void);

void sgwDynSpeedError(bool useErrno, const char *format, ...) __attribute__ ((format(printf, 2, 3)));

#endif /* _SGWDYNSPEED_SGWDYNSPEED_H */
