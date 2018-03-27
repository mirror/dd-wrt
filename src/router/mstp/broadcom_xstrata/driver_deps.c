/*
 * driver_deps.c	Driver-specific code.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Authors: Vitalii Demianets <dvitasgs@gmail.com>
 * Authors: Vladimir Cotfas <unix_router@yahoo.com> -- Broadcom Xstrata support
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/byteorder.h>
#include <linux/if_bridge.h>

#include "log.h"
#include "mstp.h"

static const char* port_states_bcm[] =
{
    [BR_STATE_DISABLED]   = "Disable",
    [BR_STATE_LISTENING]  = "LIsten",
    [BR_STATE_LEARNING]   = "LEarn",
    [BR_STATE_FORWARDING] = "Forward",
    [BR_STATE_BLOCKING]   = "Disable",
};

static char* if_linux2bcm(const char* name)
{
    if((NULL == name) || ('\0' == name[0]))
        return "???";

    if(!strcmp(name, "GbE1"))
        return "ge3";
    if(!strcmp(name, "GbE2"))
        return "ge4";

    return "???";
}

/*
 * Set new state (BR_STATE_xxx) for the given port and MSTI.
 * Return new actual state (BR_STATE_xxx) from driver.
 */
int driver_set_new_state(per_tree_port_t *ptp, int new_state)
{
    unsigned int port;
    port_t *ifc = ptp->port;

    char cmd[257] = {0};
    snprintf(cmd, 256, "bcmexp port %s stp=%s &>/dev/null",
             if_linux2bcm(ifc->sysdeps.name), port_states_bcm[new_state]);
    Dprintf(2, "CMD: %s\n", cmd);
    system(cmd);

    return new_state;
}

bool driver_create_msti(bridge_t *br, __u16 mstid)
{
    /* TODO: send "create msti" command to driver */
    return true;
}

bool driver_delete_msti(bridge_t *br, __u16 mstid)
{
    /* TODO: send "delete msti" command to driver */
    return true;
}

/*
 * Flush L2 forwarding table for a given port
 */
void driver_flush_all_fids(per_tree_port_t *ptp)
{
    unsigned int port;
    port_t *ifc = ptp->port;

    char cmd[257] = {0};
    snprintf(cmd, 256, "bcmexp l2 clear port=%s &>/dev/null",
             if_linux2bcm(ifc->sysdeps.name));
    Dprintf(2, "CMD: %s\n", cmd);
    system(cmd);

    /*
     * TODO: Confirm that L2 forwarding table is _really_ flushed
     *   when bcmexp returns. If that is not true and after return from
     *   bcmexp the flushing is still in process, then we should wait for the
     *   end of that process (preferably in asynchronous manner) and only than
     *   call MSTP_IN_all_fids_flushed().
     */
    MSTP_IN_all_fids_flushed(ptp);
}

/*
 * Set new ageing time (in seconds) for the bridge.
 * Return new actual ageing time from driver (the ageing timer granularity
 *  in the hardware can be more than 1 sec)
 */
unsigned int driver_set_ageing_time(bridge_t *br, unsigned int ageingTime)
{
    /* TODO: do set new ageing time */
    return ageingTime;
}
