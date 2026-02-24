/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _MORSE_OFFLOAD_H_
#define _MORSE_OFFLOAD_H_

#include "morse.h"
#include "command.h"

/**
 * Max size of filename for DHCP update script
 */
#define DHCPC_LEASE_UPDATE_SCRIPT_NAME_SIZE_MAX		(64)

/**
 * @brief Handle a lease update event from the in-chip DHCP client
 * This function calls the script defined by the mod_param `dhcpc_lease_update_script` to handle
 * updating the addresses. The default script is located at `/morse/scripts/dhcpc_update.sh`
 *
 * It is called with the following parameters:
 * <interface name> <ip> <netmask> <gateway> <dns server>
 * eg.
 * wlan0 192.168.1.2 255.255.255.0 192.168.1.1 192.168.1.1
 *
 * If the lease has expired or cleared, all IP addresses passed will be the null address (0.0.0.0)
 *
 * @param mors Morse structure
 * @param evt lease update event
 * @return int 0 on success else error number
 */
int morse_offload_dhcpc_set_address(struct morse *mors,
				    struct morse_cmd_evt_dhcp_lease_update *evt);

#endif /* !_MORSE_OFFLOAD_H_ */
