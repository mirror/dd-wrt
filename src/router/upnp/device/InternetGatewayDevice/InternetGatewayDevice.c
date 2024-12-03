/*
 * Broadcom UPnP module, InternetGatewayDevice.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: InternetGatewayDevice.c,v 1.9 2008/08/25 08:16:14 Exp $
 */
#include <upnp.h>
#include <soap.h>
#include <InternetGatewayDevice.h>

void upnp_portmap_reload(UPNP_CONTEXT *context)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	UPNP_PORTMAP *map;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);
	for (map = portmap_ctrl->pmlist; map < portmap_ctrl->pmlist + portmap_ctrl->num; map++) {
		/* Set to NAT kernel */
		if (map->enable)
			upnp_osl_nat_config(map);
	}

	return;
}

/* Find a port mapping entry with index */
UPNP_PORTMAP *upnp_portmap_with_index(UPNP_CONTEXT *context, int index)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);
	if (index >= portmap_ctrl->num)
		return 0;

	return (portmap_ctrl->pmlist + index);
}

/* Get number of port mapping entries */
unsigned short upnp_portmap_num(UPNP_CONTEXT *context)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);
	return portmap_ctrl->num;
}

/* Find a port mapping entry */
UPNP_PORTMAP *upnp_portmap_find(UPNP_CONTEXT *context, char *remote_host, unsigned short external_port, char *protocol)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	UPNP_PORTMAP *map;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);
	for (map = portmap_ctrl->pmlist; map < portmap_ctrl->pmlist + portmap_ctrl->num; map++) {
		/* Find the entry fits for the required paramters */
		if (strcmp(map->remote_host, remote_host) == 0 && map->external_port == external_port &&
		    strcmp(map->protocol, protocol) == 0) {
			return map;
		}
	}

	return 0;
}

/* Description: Delete a port mapping entry */
static void upnp_portmap_purge(UPNP_CONTEXT *context, UPNP_PORTMAP *map)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	int index;
	int remainder;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);

	if (map->enable) {
		map->enable = 0;
		upnp_osl_nat_config(map);
	}

	/* Pull up remainder */
	index = map - portmap_ctrl->pmlist;
	remainder = portmap_ctrl->num - (index + 1);
	if (remainder)
		memcpy(map, map + 1, sizeof(*map) * remainder);

	portmap_ctrl->num--;
	return;
}

int upnp_portmap_del(UPNP_CONTEXT *context, char *remote_host, unsigned short external_port, char *protocol)
{
	UPNP_PORTMAP *map;

	map = upnp_portmap_find(context, remote_host, external_port, protocol);
	if (map == 0)
		return SOAP_NO_SUCH_ENTRY_IN_ARRAY;

	/* Purge this entry */
	upnp_portmap_purge(context, map);
	return 0;
}

/* Add a new port mapping entry */
int upnp_portmap_add(UPNP_CONTEXT *context, char *remote_host, unsigned short external_port, char *protocol,
		     unsigned short internal_port, char *internal_client, unsigned int enable, char *description,
		     unsigned long duration)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	UPNP_PORTMAP *map;

	/* Get control body */
	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);

	/* data validation */
	if (strcasecmp(protocol, "TCP") != 0 && strcasecmp(protocol, "UDP") != 0) {
		upnp_syslog(LOG_ERR, "add_portmap:: Invalid protocol");
		return SOAP_ARGUMENT_VALUE_INVALID;
	}

	/* check duplication */
	map = upnp_portmap_find(context, remote_host, external_port, protocol);
	if (map) {
		if (strcmp(internal_client, map->internal_client) != 0)
			return SOAP_CONFLICT_IN_MAPPING_ENTRY;

		/* Argus, make it looked like shutdown */
		if (enable != map->enable || internal_port != map->internal_port) {
			if (map->enable) {
				map->enable = 0;
				upnp_osl_nat_config(map);
			}
		}
	} else {
		if (portmap_ctrl->num == portmap_ctrl->limit) {
			UPNP_PORTMAP_CTRL *new_portmap_ctrl;

			int old_limit = portmap_ctrl->limit;
			int old_size = UPNP_PORTMAP_CTRL_SIZE + old_limit * sizeof(UPNP_PORTMAP);

			int new_limit = old_limit * 2;
			int new_size = UPNP_PORTMAP_CTRL_SIZE + new_limit * sizeof(UPNP_PORTMAP);

			/*
			 * malloc a new one for twice the size,
			 * the reason we don't use realloc is when realloc failed,
			 * the old memory will be gone!
			 */
			new_portmap_ctrl = (UPNP_PORTMAP_CTRL *)malloc(new_size);
			if (new_portmap_ctrl == 0)
				return SOAP_OUT_OF_MEMORY;

			/* Copy the old to the new one, and free it */
			memcpy(new_portmap_ctrl, portmap_ctrl, old_size);

			free(portmap_ctrl);

			/* Assign the new one as the portmap_ctrl */
			portmap_ctrl = new_portmap_ctrl;
			context->focus_ifp->focus_devchain->devctrl = new_portmap_ctrl;

			portmap_ctrl->limit = new_limit;
		}

		/* Locate the map and advance the total number */
		map = portmap_ctrl->pmlist + portmap_ctrl->num;
		portmap_ctrl->num++;
	}

	/* Update database */
	map->external_port = external_port;
	map->internal_port = internal_port;
	map->enable = enable;
	map->duration = duration;
	map->book_time = time(0);

	strlcpy(map->remote_host, remote_host, sizeof(map->remote_host));
	strlcpy(map->protocol, protocol, sizeof(map->protocol));
	strlcpy(map->internal_client, internal_client, sizeof(map->internal_client));
	strlcpy(map->description, description, sizeof(map->description));

	/* Set to NAT kernel */
	if (map->enable)
		upnp_osl_nat_config(map);

	return 0;
}

/* Timed-out a port mapping entry */
void upnp_portmap_timeout(UPNP_CONTEXT *context, time_t now)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	UPNP_PORTMAP *map;
	unsigned int past;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);

	/* Make sure reach check point */
	past = now - portmap_ctrl->pm_seconds;
	if (past == 0)
		return;

	map = portmap_ctrl->pmlist;
	while (map < portmap_ctrl->pmlist + portmap_ctrl->num) {
		/* Purge the expired one */
		if (map->duration == 0 && now >= map->book_time + 604800) { // infinite entry requests should remain 7 days max
			upnp_portmap_purge(context, map);

			/*
			 * Keep the map pointer because after purging,
			 * the remainders will be pulled up
			 */
		} else if (map->duration != 0 && now >= map->book_time + map->duration) {
			upnp_portmap_purge(context, map);

			/*
			 * Keep the map pointer because after purging,
			 * the remainders will be pulled up
			 */
		} else {
			map++;
		}
	}

	portmap_ctrl->pm_seconds = now;
	return;
}

/* Free port mapping list */
void upnp_portmap_free(UPNP_CONTEXT *context)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	UPNP_PORTMAP *map;
	int i;

	portmap_ctrl = (UPNP_PORTMAP_CTRL *)(context->focus_ifp->focus_devchain->devctrl);
	if (portmap_ctrl == 0)
		return;

	for (i = 0, map = portmap_ctrl->pmlist; i < portmap_ctrl->num; i++, map++) {
		/* Delete this one from NAT kernel */
		if (map->enable) {
			map->enable = 0;
			upnp_osl_nat_config(map);
		}
	}

	/* Free the control table */
	context->focus_ifp->focus_devchain->devctrl = 0;
	free(portmap_ctrl);
	return;
}

/* Initialize port mapping list */
int upnp_portmap_init(UPNP_CONTEXT *context)
{
	UPNP_PORTMAP_CTRL *portmap_ctrl;
	int size = UPNP_PORTMAP_CTRL_SIZE + UPNP_PM_SIZE * sizeof(UPNP_PORTMAP);

	/* allocate memory */
	portmap_ctrl = (UPNP_PORTMAP_CTRL *)malloc(size);
	if (portmap_ctrl == 0) {
		upnp_syslog(LOG_ERR, "Cannot allocate port mapping buffer");
		return -1;
	}

	memset(portmap_ctrl, 0, size);

	/* Do initialization */
	portmap_ctrl->num = 0;
	portmap_ctrl->limit = UPNP_PM_SIZE;

	/* Hook to devchain */
	context->focus_ifp->focus_devchain->devctrl = portmap_ctrl;

	return 0;
}

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: InternetGatewayDevice_common_init() */
int InternetGatewayDevice_common_init(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: InternetGatewayDevice_open() */
int InternetGatewayDevice_open(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	/* Check whether the IGD is okay */
	if (upnp_osl_igd_status() == 0)
		return -1;

	/* NAT traversal initialization */
	if (upnp_portmap_init(context) != 0)
		return -1;

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: InternetGatewayDevice_close() */
int InternetGatewayDevice_close(UPNP_CONTEXT *context)
{
	/* << USER CODE START >> */
	/* cleanup NAT traversal structures */
	upnp_portmap_free(context);
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: InternetGatewayDevice_request() */
int InternetGatewayDevice_request(UPNP_CONTEXT *context, void *cmd)
{
	/* << USER CODE START >> */
	UPNP_REQUEST *request = (UPNP_REQUEST *)cmd;

	switch (request->cmd) {
	case UPNP_REQ_RELOAD_PORTMAP:
		upnp_portmap_reload(context);
		break;

	default:
		break;
	}

	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: InternetGatewayDevice_timeout() */
int InternetGatewayDevice_timeout(UPNP_CONTEXT *context, time_t now)
{
	/* << USER CODE START >> */
	upnp_portmap_timeout(context, now);
	return 0;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: InternetGatewayDevice_notify() */
int InternetGatewayDevice_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	/* << USER CODE START >> */
	return 0;
}
/* >> AUTO GENERATED FUNCTION */
