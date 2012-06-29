#ifndef _SGWDYNSPEED_CONFIGURATION_H_
#define _SGWDYNSPEED_CONFIGURATION_H_

/* Plugin includes */

/* OLSR includes */
#include "olsrd_plugin.h"

/* System includes */

/*
 * speedFile
 */

/** The name of the speedFile plugin parameter */
#define SGWDYNSPEED_SPEEDFILE_NAME				"speedFile"

char * getSpeedFile(void);
int setSpeedFile(const char *value, void *data, set_plugin_parameter_addon addon);

/*
 * speedFilePeriod
 */

/** The name of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_NAME		"speedFilePeriod"

/** The default value of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_DEFAULT		((unsigned long long)10000)

/** The minimal value of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_MIN			((unsigned long long)1000)

/** The maximal value of the speedFilePeriod plugin parameter */
#define SGWDYNSPEED_SPEEDFILEPERIOD_MAX			((unsigned long long)320000000)

unsigned long long getSpeedFilePeriod(void);
int setSpeedFilePeriod(const char *value, void *data, set_plugin_parameter_addon addon);

#endif /* _SGWDYNSPEED_CONFIGURATION_H_ */
