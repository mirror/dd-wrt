/* nutscan-snmp.h - fully generated during build of NUT
 *  Copyright (C) 2011-2019 EATON
 *  	Authors: Frederic Bohe <FredericBohe@Eaton.com>
 *               Arnaud Quette <ArnaudQuette@Eaton.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DEVSCAN_SNMP_H
#define DEVSCAN_SNMP_H

typedef struct {
	char *	oid;
	char *	mib;
	char *	sysoid;
} snmp_device_id_t;

/* SNMP IDs device table */
static snmp_device_id_t snmp_device_table[] = {
	{ ".1.3.6.1.4.1.318.1.1.8.1.5.0", "apc_ats", ".1.3.6.1.4.1.318.1.3.11" },
	{ NULL, "apc", ".1.3.6.1.4.1.318.1.3.4.9" },
	{ ".1.3.6.1.4.1.318.1.1.1.1.1.1.0", "apcc", ".1.3.6.1.4.1.318.1.1.1.1.1.1.0" },
	{ ".1.3.6.1.4.1.318.1.1.4.1.4.0", "apc_pdu", ".1.3.6.1.4.1.318.1.3.4.4.1.3.6.1.4.1.318.1.3.4.5" },
	{ ".1.3.6.1.4.1.318.1.1.4.1.4.0", "apc_pdu", ".1.3.6.1.4.1.318.1.3.4.5" },
	{ ".1.3.6.1.4.1.318.1.1.4.1.4.0", "apc_pdu", ".1.3.6.1.4.1.318.1.3.4.6" },
	{ ".1.3.6.1.4.1.4779.1.3.5.2.1.24.1", "baytech", ".1.3.6.1.4.1.4779" },
	{ ".1.3.6.1.4.1.2947.1.1.2.0", "bestpower", ".1.3.6.1.4.1.2947.1.1.2.0" },
	{ ".1.3.6.1.4.1.232.165.3.1.1.0", "cpqpower", ".1.3.6.1.4.1.232.165.3" },
	{ ".1.3.6.1.4.1.3808.1.1.1.1.1.1.0", "cyberpower", ".1.3.6.1.4.1.3808.1.1.1.1.3.6.1.4.1.3808" },
	{ ".1.3.6.1.4.1.3808.1.1.1.1.1.1.0", "cyberpower", ".1.3.6.1.4.1.3808" },
	{ NULL, "delta_ups", ".1.3.6.1.4.1.2254.2.4" },
	{ ".1.3.6.1.4.1.534.10.2.1.2.0", "eaton_ats16_nm2", ".1.3.6.1.4.1.534.10.2" },
	{ ".1.3.6.1.4.1.534.10.2.1.2.0", "eaton_ats16_nmc", ".1.3.6.1.4.1.705.1" },
	{ ".1.3.6.1.4.1.534.10.1.2.1.0", "eaton_ats30", ".1.3.6.1.4.1.534.10.1" },
	{ ".1.3.6.1.4.1.17373.3.1.1.0", "aphel_genesisII", ".1.3.6.1.4.1.17373" },
	{ ".1.3.6.1.4.1.534.6.6.7.1.2.1.2.0", "eaton_epdu", ".1.3.6.1.4.1.534.6.6.7" },
	{ NULL, "eaton_pdu_nlogic", ".1.3.6.1.4.1.534.7.1" },
	{ ".1.3.6.1.4.1.20677.1", "pulizzi_switched1", ".1.3.6.1.4.1.20677.1" },
	{ ".1.3.6.1.4.1.20677.1", "pulizzi_switched2", ".1.3.6.1.4.1.20677.2" },
	{ ".1.3.6.1.4.1.534.6.6.6.1.1.12.0", "aphel_revelation", ".1.3.6.1.4.1.534.6.6.6" },
	{ "1.3.6.1.4.1.534.1.1.2.0", "eaton_pw_nm2", ".1.3.6.1.4.1.534.1" },
	{ "1.3.6.1.4.1.534.1.1.2.0", "eaton_pxg_ups", ".1.3.6.1.4.1.534.2.12" },
	{ ".1.3.6.1.4.1.10418.17.2.1.2.0", "emerson_avocent_pdu", ".1.3.6.1.4.1.10418.17.1.7" },
	{ ".1.3.6.1.4.1.232.165.7.1.2.1.3.0", "hpe_epdu", ".1.3.6.1.4.1.232.165.7" },
	{ ".1.3.6.1.4.1.232.165.11.1.2.1.3.1", "hpe_pdu3_cis", ".1.3.6.1.4.1.232.165.11" },
	{ ".1.3.6.1.4.1.2011.6.174.1.2.100.1.2.1", "huawei", ".1.3.6.1.4.1.8072.3.2.10" },
	{ NULL, "tripplite", ".1.3.6.1.4.1.850.1" },
	{ "1.3.6.1.2.1.33.1.1.1.0", "ietf", ".1.3.6.1.2.1.33" },
	{ ".1.3.6.1.4.1.705.1.1.1.0", "mge", ".1.3.6.1.4.1.705.1" },
	{ ".1.3.6.1.4.1.4555.1.1.1.1.1.1.0", "netvision", ".1.3.6.1.4.1.4555.1.1.1" },
	{ ".1.3.6.1.4.1.13742.1.1.12.0", "raritan", ".1.3.6.1.4.1.13742" },
	{ ".1.3.6.1.4.1.13742.6.3.2.1.1.3.1", "raritan-px2", ".1.3.6.1.4.1.13742.6" },
	{ NULL, "xppc", ".1.3.6.1.4.1.935" },
	/* Terminating entry */
	{ NULL, NULL, NULL }
};
#endif /* DEVSCAN_SNMP_H */
