#ifndef _SGWDYNSPEED_OLSRD_PLUGIN_H_
#define _SGWDYNSPEED_OLSRD_PLUGIN_H_

/* Plugin includes */
#include "configuration.h"

/* OLSRD includes */
#include "olsrd_plugin.h"

/* System includes */
#include <stddef.h>

/** The interface version supported by the plugin */
#define SGWDYNSPEED_PLUGIN_INTERFACE_VERSION	5

/**
 The plugin parameter configuration, containing the parameter names, pointers
 to their setters, and an optional data pointer that is given to the setter
 when it is called.
 */
static const struct olsrd_plugin_parameters plugin_parameters[] = {
	{	.name = SGWDYNSPEED_SPEEDFILE_NAME, .set_plugin_parameter = &setSpeedFile, .data = NULL},
	{	.name = SGWDYNSPEED_SPEEDFILEPERIOD_NAME, .set_plugin_parameter = &setSpeedFilePeriod, .data = NULL}
};

#endif /* _SGWDYNSPEED_OLSRD_PLUGIN_H_ */
