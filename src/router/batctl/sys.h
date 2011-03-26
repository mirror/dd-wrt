/*
 * Copyright (C) 2009-2011 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <lindner_marek@yahoo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#define SYS_BATIF_PATH_FMT "/sys/class/net/%s/mesh/"
#define SYS_LOG_LEVEL "log_level"
#define SYS_LOG "log"
#define SYS_AGGR "aggregated_ogms"
#define SYS_BONDING "bonding"
#define SYS_GW_MODE "gw_mode"
#define SYS_GW_SEL "gw_sel_class"
#define SYS_GW_BW "gw_bandwidth"
#define SYS_VIS_MODE "vis_mode"
#define SYS_ORIG_INTERVAL "orig_interval"
#define SYS_IFACE_PATH "/sys/class/net"
#define SYS_MESH_IFACE_FMT SYS_IFACE_PATH"/%s/batman_adv/mesh_iface"
#define SYS_IFACE_STATUS_FMT SYS_IFACE_PATH"/%s/batman_adv/iface_status"
#define SYS_FRAG "fragmentation"

enum gw_modes {
	GW_MODE_OFF,
	GW_MODE_CLIENT,
	GW_MODE_SERVER,
};

extern const char *sysfs_param_enable[];
extern const char *sysfs_param_server[];

void aggregation_usage(void);
void bonding_usage(void);
void fragmentation_usage(void);
void gw_mode_usage(void);
void vis_mode_usage(void);
void orig_interval_usage(void);
int interface(char *mesh_iface, int argc, char **argv);
int handle_loglevel(char *mesh_iface, int argc, char **argv);
int handle_sys_setting(char *mesh_iface, int argc, char **argv,
		       char *file_path, void setting_usage(void),
		       const char *sysfs_param[]);
int handle_gw_setting(char *mesh_iface, int argc, char **argv);
