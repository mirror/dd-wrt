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


#define DEBUG_BATIF_PATH_FMT "%s/batman_adv/%s"
#define DEBUG_ORIGINATORS "originators"
#define DEBUG_TRANSTABLE_LOCAL "transtable_local"
#define DEBUG_TRANSTABLE_GLOBAL "transtable_global"
#define DEBUG_SOFTIF_NEIGH "softif_neigh"
#define DEBUG_GATEWAYS "gateways"
#define DEBUG_VIS_DATA "vis_data"
#define DEBUG_LOG "log"

void originators_usage(void);
void trans_local_usage(void);
void trans_global_usage(void);
void softif_neigh_usage(void);
void gateways_usage(void);
int handle_debug_table(char *mesh_iface, int argc, char **argv,
		       char *file_path, void table_usage(void));
int log_print(char *mesh_iface, int argc, char **argv);
