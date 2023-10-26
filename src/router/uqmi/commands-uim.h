/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#define __uqmi_uim_commands												\
	__uqmi_command(uim_slot, uim-slot, required, CMD_TYPE_OPTION), \
	__uqmi_command(uim_verify_pin1, uim-verify-pin1, required, QMI_SERVICE_UIM), \
	__uqmi_command(uim_verify_pin2, uim-verify-pin2, required, QMI_SERVICE_UIM), \
	__uqmi_command(uim_get_sim_state, uim-get-sim-state, no, QMI_SERVICE_UIM), \
	__uqmi_command(uim_power_off, uim-power-off, no, QMI_SERVICE_UIM), \
	__uqmi_command(uim_power_on, uim-power-on, no, QMI_SERVICE_UIM) \


#define uim_helptext \
		"  --uim-verify-pin1 <pin>:          Verify PIN1 (new devices)\n" \
		"  --uim-verify-pin2 <pin>:          Verify PIN2 (new devices)\n" \
		"  --uim-get-sim-state:              Get current SIM state\n" \
		"  --uim-power-off:                  Power off SIM card\n" \
		"    --uim-slot:                     SIM slot [1-2]\n" \
		"  --uim-power-on:                   Power on SIM card\n" \
		"    --uim-slot:                     SIM slot [1-2]\n" \

