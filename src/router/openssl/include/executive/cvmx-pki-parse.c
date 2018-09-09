/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * PKI COnfig Parsing Support.
 */
#include <stdlib.h>
#include "cvmx.h"
#include "cvmx-config-parse.h"
#include "cvmx-pki.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "string.h"

#define CVMX_CONFIG_LOCAL_NODE  (1)
#define CVMX_CONFIG_ALL_NODES   (3)

static struct cvmx_pki_cluster_profile cl_profile;
static struct cvmx_pki_pool_profile pool_profile;
static struct cvmx_pki_aura_profile aura_profile;
static struct cvmx_pki_style_profile style_profile;
static struct cvmx_pki_style_config style_config;
static struct cvmx_pki_qpg_profile qpg_profile;
static struct cvmx_pki_qpg_config qpg_config;
static struct cvmx_pki_pcam_config pcam_entry;
static struct cvmx_pki_sso_grp_profile sso_grp_profile;
static int pcam_index;
static int node;
static int dbg_pki_parse=0;

int parse_pki_node(token_list_t *list)
{
	if((strcmp(list->tokens[2], "node") == 0) &&
		    (strcmp(list->tokens[3], "=") == 0)) {
		if((strcmp(list->tokens[4], "local") == 0)) {
			node = cvmx_get_node_num();
		}
		else if((strcmp(list->tokens[4], "all") == 0))
			node = CVMX_CONFIG_ALL_NODES;
		else {
			node = strtol(list->tokens[3], (char**)NULL, 10);
			if(node > CVMX_CONFIG_ALL_NODES) {
				cvmx_dprintf("ERROR:Parsing PKI command node=%d",node);
				return -1;
			}
			//vinita_to_do implement [0..1]
		}
	}
	else {
		cvmx_dprintf("ERROR: invalid node format in pki parse cmds\n");
		return -1;
	}
	return 0;
}

void cvmx_pki_reset_pool_profile(struct cvmx_pki_pool_profile *pool_profile)
{
	pool_profile->pool_name[0] = '\0';
	pool_profile->buffer_size = CVMX_PKI_NOT_ASSIGNED;
	pool_profile->buffer_count = CVMX_PKI_NOT_ASSIGNED;
}

int cvmx_pki_check_pool_profile(struct cvmx_pki_pool_profile *pool_profile)
{
	if(pool_profile->pool_name[0] == '\0') {
		cvmx_dprintf("ERROR: pool_name not specified in pool_config cmd\n");
		return -1;
	}
	if(pool_profile->buffer_size == (uint64_t)CVMX_PKI_NOT_ASSIGNED) {
		cvmx_dprintf("ERROR: buffer_size not specified in pool_config cmd\n");
		return -1;
	}
	if(pool_profile->buffer_count == (uint64_t)CVMX_PKI_NOT_ASSIGNED) {
		cvmx_dprintf("ERROR: number_buffers not specified in pool_config cmd\n");
		return -1;
	}
	return 0;
}

int parse_pki_pool_command(token_list_t *list)
{

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki pool_config\n");

	if(strcmp(list->tokens[2], "start_config") == 0) {
		//cvmx_pki_get_default_pool_profile(&pool_profile);
		cvmx_pki_reset_pool_profile(&pool_profile);
		if((strcmp(list->tokens[3], "pool_name") == 0) &&
				  (strcmp(list->tokens[4], "=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: pool_name > %d char in pool config cmd\n",
					     CVMX_PKI_MAX_NAME-1);
				return -1;
			}
			strcpy(pool_profile.pool_name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: Missing pool_name in pool_config cmd\n");
			return -1;
		}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if((strcmp(list->tokens[2],"pool_number") == 0) &&
		    (strcmp(list->tokens[3],"=") == 0)) {
		if((strcmp(list->tokens[4],"auto") == 0))
			pool_profile.pool_num = CVMX_PKI_FIND_AVAL_ENTRY;
		else
			pool_profile.pool_num = strtol(list->tokens[4], (char**)NULL, 10);
		if(pool_profile.pool_num > CVMX_FPA3_NUM_POOLS) {
			cvmx_dprintf("ERROR: pool_number is > %d in pool_config cmd\n"
					,CVMX_FPA3_NUM_POOLS);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"buffer_size") == 0) &&
		    (strcmp(list->tokens[3],"=") == 0)) {
		pool_profile.buffer_size = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2],"number_of_buffers") == 0) &&
		   (strcmp(list->tokens[3],"=") == 0)) {
		pool_profile.buffer_count = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if(strcmp(list->tokens[2],"end_config")== 0) {
		//if(cvmx_pki_check_pool_profile(&pool_profile))
		//	return -1;
		cvmx_pki_set_pool_config(node, pool_profile.pool_name, pool_profile.pool_num,
					 pool_profile.buffer_size, pool_profile.buffer_count);
	}
	else {
		cvmx_dprintf("ERROR: Unrecognized pool_config cmd %s", list->tokens[2]);
		return -1;
	}
	return 0;
}

void cvmx_pki_reset_aura_profile(struct cvmx_pki_aura_profile *aura_profile)
{
	aura_profile->aura_name[0] = '\0';
	aura_profile->pool_num = CVMX_PKI_NOT_ASSIGNED;
	aura_profile->buffer_count = CVMX_PKI_NOT_ASSIGNED;
}

int cvmx_pki_check_aura_profile(struct cvmx_pki_aura_profile *aura_profile)
{
	if(aura_profile->aura_name[0] == '\0') {
		cvmx_dprintf("ERROR: aura_name not specified in aura_config cmd\n");
		return -1;
	}
	if(aura_profile->pool_num == CVMX_PKI_NOT_ASSIGNED) {
		cvmx_dprintf("ERROR: pool_num not specified in aura_config cmd\n");
		return -1;
	}
	if(aura_profile->buffer_count == CVMX_PKI_NOT_ASSIGNED) {
		cvmx_dprintf("ERROR: number_buffers not specified in aura_config cmd\n");
		return -1;
	}
	return 0;
}

int parse_pki_aura_command(token_list_t *list)
{
	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki aura_config\n");

	if(strcmp(list->tokens[2], "start_config") == 0) {
		//cvmx_pki_get_default_aura_profile(&aura_profile);
		cvmx_pki_reset_aura_profile(&aura_profile);
		if((strcmp(list->tokens[3], "aura_name") == 0) &&
					(strcmp(list->tokens[4], "=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: aura_name > %d char in aura_config cmd\n",
					     CVMX_PKI_MAX_NAME-1);
				return -1;
			}
			strcpy(aura_profile.aura_name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: Missing aura_name in aura_config cmd\n");
			return -1;
		}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if((strcmp(list->tokens[2],"aura_number") == 0) &&
		    (strcmp(list->tokens[3],"=") == 0)) {
		if((strcmp(list->tokens[4],"auto") == 0))
			aura_profile.aura_num = CVMX_PKI_FIND_AVAL_ENTRY;
		else
			aura_profile.aura_num = strtol(list->tokens[4], (char**)NULL, 10);
		if(aura_profile.aura_num > CVMX_FPA_AURA_NUM) {
			cvmx_dprintf("ERROR: aura_number is > %d in aura_config cmd\n"
					,CVMX_FPA_AURA_NUM);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"pool_to_use") == 0) &&
		    (strcmp(list->tokens[3],"=") == 0)) {
		if(strlen(list->tokens[4]) >= CVMX_PKI_MAX_NAME) {
			cvmx_dprintf("ERROR: pool_to_use name > %d in aura config command",
				     CVMX_PKI_MAX_NAME-1);
			return -1;
		}
		if((aura_profile.pool_num = cvmx_pki_find_pool(node,list->tokens[4])) == -1) {
			cvmx_dprintf("ERROR:can't find pool %s in aura_conifg cmd\n",
				     list->tokens[4]);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"number_of_buffers") == 0) &&
		   (strcmp(list->tokens[3],"=") == 0)) {
		aura_profile.buffer_count = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if(strcmp(list->tokens[2],"end_config")== 0) {
		if(cvmx_pki_check_aura_profile(&aura_profile))
			return -1;
		cvmx_pki_set_aura_config(node, aura_profile.aura_name, aura_profile.aura_num,
					 aura_profile.pool_num, aura_profile.buffer_count);
	}
	else {
		cvmx_dprintf("ERROR: Unrecognized aura_config cmd %s", list->tokens[2]);
		return -1;
	}
	return 0;
}

int parse_interface_command(char *name, int *intf_s, int* intf_e)
{
	token_list_t *list;
	list = init_token_list(30, 128);

	get_interface_name_token_list(list, name);

	if((strcmp(list->tokens[1], ":") == 0) &&
		   (strcmp(list->tokens[2],"[")== 0))
		*intf_s = strtol(list->tokens[3], (char**)NULL, 10);
	else {
		cvmx_dprintf("ERROR: invalid format for interface command\n");
		return -1;
	}
	if(strcmp(list->tokens[4],"]")== 0)
		*intf_e = *intf_s;
	else if((strcmp(list->tokens[4],".") == 0) &&
			(strcmp(list->tokens[5],".") == 0))
		*intf_e = strtol(list->tokens[6], (char**)NULL, 10);
	else {
		cvmx_dprintf("ERROR: invalid format for interface command\n");
		return -1;
	}
	if(strcmp(list->tokens[0], "ilk") == 0) {
		*intf_s += 6;
		*intf_e += 6;
		if(*intf_s < 6 || *intf_s > 7 || *intf_e < 6 || *intf_e > 7) { //vinita_do_to later replace with #define
			cvmx_dprintf("ERROR: invalid ilk interface in pki parse cmd\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[0], "gmx") == 0) {
		if(*intf_s < 0 || *intf_s > 5 || *intf_e < 0 || *intf_e > 5) { //vinita_do_to later replace with #define
			cvmx_dprintf("ERROR: invalid gmx interface in pki parse cmd\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[0], "npi") == 0) {
		//vinita_to_do find out which interface is npi in 78xx
	}
	else if(strcmp(list->tokens[0], "loop") == 0) {
		*intf_s += 9;
		*intf_e += 9;
		if(*intf_s != 9 || *intf_e != 9) { //vinita_do_to later replace with #define
			cvmx_dprintf("ERROR: invalid loop interface in pki parse cmd\n");
			return -1;
		}
	}
	else {
		cvmx_dprintf("ERROR: Invalid interface type %s in parse cmd\n",list->tokens[0]);
		return -1;
	}

	return 0;
}

int parse_interface_index(char *name, int *port_start, int* port_end)
{
	token_list_t *list;
	list = init_token_list(30, 128);
	get_interface_name_token_list(list, name);

	if(strcmp(list->tokens[0],"[")== 0) {
		*port_start = strtol(list->tokens[1], (char**)NULL, 10);
		if(strcmp(list->tokens[2],"]")== 0)
			*port_end = *port_start;
		else if(strcmp(list->tokens[2],".")== 0 &&
				 strcmp(list->tokens[3],".")==0) {
			*port_end = strtol(list->tokens[4], (char**)NULL, 10);
		}
		else {
			 cvmx_dprintf("ERROR: invalid format for interface_index\n");
			 return -1;
		}
	}
	else {
		cvmx_dprintf("ERROR: invalid interface_index format\n");
		return -1;
	}
	return 0;
}

int parse_pki_port_command(token_list_t *list)
{
	int index;
	int port_start, port_end;
	int intf_start, intf_end;
	//uint64_t pkind;

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki pkind_config\n");

	if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	if((strcmp(list->tokens[5],"interface")== 0) &&
			strcmp(list->tokens[6], "=") == 0) {
		if((parse_interface_command(list->tokens[7],
		    		&intf_start, &intf_end))== -1) {
			cvmx_dprintf("ERROR: invalid interface in pki pkind cmd\n");
			return -1;
		}
		if((strcmp(list->tokens[8],"interface_index")== 0) &&
				strcmp(list->tokens[9],"=")==0) {
			parse_interface_index(list->tokens[10],&port_start,&port_end);
		}
		else {
			cvmx_dprintf("ERROR: No interface_index %s specified\n",list->tokens[9]);
			return -1;
		}
	}
	else {
		cvmx_dprintf("ERROR: Invalid interface command in pki parse\n");
		return -1;
	}
	if((strcmp(list->tokens[11],"initial_style_to_use")== 0) &&
		   (strcmp(list->tokens[12],"=")== 0)) {
		int style;
		if(strlen(list->tokens[13]) >= CVMX_PKI_MAX_NAME) {
			cvmx_dprintf("ERROR: parsing pkind style cmd");
			return -1;
		}
		if((style = cvmx_pki_find_style(node,list->tokens[13])) == -1)
		{
			cvmx_dprintf("ERROR:cant find style_profile %s\n",list->tokens[13]);
			return -1;
		}
		while(intf_start <= intf_end) {
			for(index=port_start; index<=port_end; index++)
				cvmx_pki_set_port_style_cfg(node, intf_start, index, style);
			intf_start++;
		}
	}
	else if((strcmp(list->tokens[11],"initial_parse_mode")== 0) &&
		   (strcmp(list->tokens[12],"=")== 0)) {
		enum cvmx_pki_pkind_parse_mode	parsing_mode;
		if(strcmp(list->tokens[13],"la_to_lg")) {
			parsing_mode = CVMX_PKI_PARSE_LA_TO_LG;
		}
		else if(strcmp(list->tokens[13],"lb_to_lg")) {
			parsing_mode = CVMX_PKI_PARSE_LB_TO_LG;
		}
		else if(strcmp(list->tokens[13],"lc_to_lg")) {
			parsing_mode = CVMX_PKI_PARSE_LC_TO_LG;
		}
		else if(strcmp(list->tokens[13],"only_lg")) {
			parsing_mode = CVMX_PKI_PARSE_LG;
		}
		else if(strcmp(list->tokens[13],"nothing")) {
			parsing_mode = CVMX_PKI_PARSE_NOTHING;
		}
		else {
			cvmx_dprintf("ERROR:invalid format for initial_parse_mode\n");
			return -1;
		}
		while(intf_start <= intf_end) {
			for(index=port_start; index<=port_end; index++)
				cvmx_pki_set_port_parse_mode_cfg(node,intf_start,index,parsing_mode);
			intf_start++;
		}
	}
	else if((strcmp(list->tokens[11],"enable_specific_parsing")== 0) &&
			(strcmp(list->tokens[12],"=")== 0)) {
		//vinita_to_do
	}
	else if((strcmp(list->tokens[11],"cluster_group_to_service_traffic") == 0) &&
				(strcmp(list->tokens[12],"=")== 0)) {
		int cluster_group,cluster_mask;
		if(strlen(list->tokens[13]) >= CVMX_PKI_MAX_NAME) {
			cvmx_dprintf("ERROR: parsing pkind cluster command\n");
			return -1;
		}
		cluster_group = cvmx_pki_find_cluster_group(node,list->tokens[13]);
		cluster_mask = cvmx_pki_find_cluster_mask(node, list->tokens[13]);
		if(cluster_group < 0 || cluster_mask < 0) {
			cvmx_dprintf("ERROR: finding cluster group/mask for group %s\n",list->tokens[13]);
			return -1;
		}
		while(intf_start <= intf_end) {
			for(index=port_start; index<=port_end; index++)
				cvmx_pki_set_port_cluster_config(node,intf_start,index,cluster_group,cluster_mask);
			intf_start++;
		};
	}
	else {
		cvmx_dprintf("ERROR:unrecognized pki interface cmd %s\n",list->tokens[11]);
		return -1;
	}
	return 0;
}

void cvmx_pki_reset_cluster_profile(struct cvmx_pki_cluster_profile *cl_profile)
{
	cl_profile->name[0] = '\0';
	cl_profile->num_clusters = CVMX_PKI_NOT_ASSIGNED;
}

int cvmx_pki_check_cluster_profile(struct cvmx_pki_cluster_profile *cl_profile)
{
	if(cl_profile->name[0] == '\0') {
		cvmx_dprintf("ERROR: cluster_group_name not provided\n");
		return -1;
	}
	if(cl_profile->num_clusters == CVMX_PKI_NOT_ASSIGNED) {
		cvmx_dprintf("ERROR: clusters_in_group not provided\n");
		return -1;
	}
	return 0;
}

int parse_pki_cluster_command(token_list_t *list)
{
	int token_index;

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki cluster_config\n");

	if(strcmp(list->tokens[2], "start_config") == 0) {
		//cvmx_pki_get_default_cluster_profile(&cl_profile);
		cvmx_pki_reset_cluster_profile(&cl_profile);
		if((strcmp(list->tokens[3], "cluster_group_name") == 0) &&
				  (strcmp(list->tokens[4], "=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: cluster_group_name > %d in cluster_config cmd\n",
					     CVMX_PKI_MAX_NAME-1);
				return -1;
			}
			strcpy(cl_profile.name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: Missing cluster_group_name\n");
			return -1;
		}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if((strcmp(list->tokens[2],"group_number") == 0) &&
			 (strcmp(list->tokens[3],"=") == 0)) {
		if((strcmp(list->tokens[4],"auto") == 0))
			cl_profile.cluster_group = CVMX_PKI_FIND_AVAL_ENTRY;
		else
			cl_profile.cluster_group = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2], "clusters_in_group") == 0) &&
			   (strcmp(list->tokens[3], "=") == 0)) {
		cl_profile.num_clusters = strtol(list->tokens[4], (char**)NULL, 10);
		if(cl_profile.num_clusters > CVMX_PKI_NUM_CLUSTERS) {
			cvmx_dprintf("ERROR: pki cluster config cluster %d is > CVMX_PKI_NUM_CLUSTERS\n",
				     cl_profile.num_clusters);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"enable_parsing")== 0) &&
			 list->token_index >= 3) {
		token_index = 3;
		while(token_index < list->token_index) {
			if(strcmp(list->tokens[token_index],"l3") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_L3;
			else if(strcmp(list->tokens[token_index],"l4") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_L4;
			else if(strcmp(list->tokens[token_index],"il3") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_IL3;
			else if(strcmp(list->tokens[token_index],"custom_l2") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_CUSTOM_L2;
			else if(strcmp(list->tokens[token_index],"custom_lg") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_CUSTOM_LG;
			else if(strcmp(list->tokens[token_index],"virt") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_VIRTUALIZATION;
			else if(strcmp(list->tokens[token_index],"mpls") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_MPLS;
			else if(strcmp(list->tokens[token_index],"fulcrum") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_FULCRUM;
			else if(strcmp(list->tokens[token_index],"dsp") == 0)
				cl_profile.parsing_mask |= CVMX_PKI_PARSE_DSP;
			else {
				cvmx_dprintf("ERROR:Invalid parsing type in cluster_group_profile cmd");
				return -1;
			}
			token_index++;
		}
	}
	else if(strcmp(list->tokens[2],"end_config")== 0) {
		if(cvmx_pki_check_cluster_profile(&cl_profile))
			return -1;
		cvmx_pki_set_cluster_config(node,cl_profile);
	}
	else {
		cvmx_dprintf("ERROR: Unrecognized cluster_config cmd %s",list->tokens[2]);
		return -1;
	}
	return 0;
}

int parse_pki_qpg_command(token_list_t *list)
{
	uint64_t entry_start=0;
	uint64_t entry_end=0;
	int group;

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki qpg_table\n");

	if(strcmp(list->tokens[2], "start_config") == 0) {
		//cvmx_pki_get_default_qpg_profile(&qpg_profile);
		//cvmx_pki_reset_qpg_profile(&qpg_profile);//vinita_to_do
		if((strcmp(list->tokens[3], "table_name") == 0) &&
				  (strcmp(list->tokens[4], "=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: qpg table_name > %d in qpg_config cmd\n",
					     CVMX_PKI_MAX_NAME-1);
				return -1;
			}
			strcpy(qpg_profile.qpg_name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: Missing qpg table_name\n");
			return -1;
		}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if((strcmp(list->tokens[2],"base_offset") == 0) &&
		   (strcmp(list->tokens[3],"=") == 0)) {
		if((strcmp(list->tokens[4],"auto") == 0))
			qpg_profile.base_offset = CVMX_PKI_FIND_AVAL_ENTRY;
		else
			qpg_profile.base_offset = strtol(list->tokens[11], (char**)NULL, 10);
		if(qpg_profile.base_offset > CVMX_PKI_NUM_QPG_ENTRY) {
			cvmx_dprintf("ERROR: qpg base-offset > %d",CVMX_PKI_NUM_QPG_ENTRY);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"num_entries")==0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		qpg_profile.num_entries = strtol(list->tokens[4],(char**)NULL,10);
		cvmx_pki_set_qpg_profile(node, qpg_profile. qpg_name, qpg_profile.base_offset,
					 qpg_profile.num_entries);
	}
	else if((strcmp(list->tokens[2],"entry")==0) &&
			(strcmp(list->tokens[3],"=") == 0)) { //vinita_to_do malloc and pointer
		token_list_t *entry_list;
		entry_list = init_token_list(30, 128);
		get_interface_name_token_list(entry_list, list->tokens[4]);
		if(strcmp(entry_list->tokens[0],"[") == 0)
			entry_start = strtol(entry_list->tokens[1],(char**)NULL,10);
		else {
			cvmx_dprintf("ERROR: Invalid entry format in qpg_table cmd\n");
			return -1;
		}
		if(strcmp(entry_list->tokens[2],".")==0 &&
				 strcmp(entry_list->tokens[3],".")==0) {
			entry_end = strtol(entry_list->tokens[4],(char**)NULL,10);
		}
		else
			entry_end = entry_start;

		if(strcmp(list->tokens[5],"port_addr") == 0 &&
				 strcmp(list->tokens[6],"=")==0) {
			qpg_config.port_add = strtol(list->tokens[7],(char**)NULL,10);

		}
		else if(strcmp(list->tokens[5],"group_ok") == 0 &&
				 strcmp(list->tokens[6],"=")==0) {
			if(strlen(list->tokens[7]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: parsing group_ok name qpg_config cmd\n");
				return -1;
			}
			if((group = cvmx_pki_find_group(node,list->tokens[7])) == -1)
			{
				cvmx_dprintf("ERROR: couldn't find group %s\n",list->tokens[7]);
				return -1;
			}
			qpg_config.group_ok = group;
		}
		else if(strcmp(list->tokens[5],"group_bad") == 0 &&
				 strcmp(list->tokens[6],"=")==0) {
			if(strlen(list->tokens[7]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: parsing group_bad name qpg_config cmd\n");
				return -1;
			}
			if((group = cvmx_pki_find_group(node,list->tokens[7])) == -1)
			{
				cvmx_dprintf("ERROR: couldn't find group %s\n",list->tokens[7]);
				return -1;
			}
			qpg_config.group_bad = group;
		}
		else if(strcmp(list->tokens[5],"aura") == 0 &&
				 strcmp(list->tokens[6],"=")==0) {
			if(strlen(list->tokens[7]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: parsing group_ok name qpg_config cmd\n");
				return -1;
			}
			if((group = cvmx_pki_find_aura(node,list->tokens[7])) == -1)
			{
				cvmx_dprintf("ERROR: couldn't find aura %s\n",list->tokens[7]);
				return -1;
			}
			qpg_config.aura = group;
		}
		cvmx_pki_set_qpg_config(node, qpg_profile.qpg_name, entry_start,
					entry_end, qpg_config);

	}
	else if(strcmp(list->tokens[2],"end_config")==0) {
		//if(cvmx_pki_check_qpg_profile(&qpg_profile))//vinita_to_do
		//	return -1;

	}
        else {
		cvmx_dprintf("ERROR:unrecognized command %s in parse pki qpg_config\n",
			     list->tokens[5]);
	        return -1;
        }
	return 0;
}

int parse_pki_style_command(token_list_t *list)
{
	int token_index;

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki style_config\n");

	if(strcmp(list->tokens[2],"start_config") == 0) {
		//cvmx_pki_get_default_style_config(style_config);//vinita_to_do uncomment when imp
		if((strcmp(list->tokens[3],"style_name") == 0) &&
				  (strcmp(list->tokens[4],"=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: parsing pki style config command");
				return -1;
			}
			strcpy(style_profile.name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: no name specified in style start config\n");
			return -1;
		}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if((strcmp(list->tokens[2],"style_number") == 0) &&
				  (strcmp(list->tokens[3],"=") == 0)) {
			if((strcmp(list->tokens[4],"auto") == 0))
				style_profile.style_num = CVMX_PKI_FIND_AVAL_ENTRY;
			else
				style_profile.style_num = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2],"packet_little_endian") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_pkt_le_mode = 1;
		else
			style_config.en_pkt_le_mode = 0;
			}
	else if((strcmp(list->tokens[2],"l2_length_check") == 0) &&
		   (strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_l2_lenchk = 1;
		else
			style_config.en_l2_lenchk = 0;
	}
	else if((strcmp(list->tokens[2],"l2_length_check_mode") == 0) &&
		   (strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"equal")==0)
			style_config.l2_lenchk_mode = 1;
		else
			style_config.l2_lenchk_mode = 0;
	}
	else if((strcmp(list->tokens[2],"max_frame_len_check") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_maxframe_errchk = 1;
		else
			style_config.en_maxframe_errchk = 0;
	}
	else if((strcmp(list->tokens[2],"min_frame_len_check") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_minframe_errchk = 1;
		else
			style_config.en_minframe_errchk = 0;
	}
	else if((strcmp(list->tokens[2],"strip_l2_fcs") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_strip_l2_fcs = 1;
		else
			style_config.en_strip_l2_fcs = 0;
	}
	else if((strcmp(list->tokens[2],"enable_fcs_checks") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_fcs_check = 1;
		else
			style_config.en_fcs_check = 0;
	}
	else if((strcmp(list->tokens[2],"wqe_header_size") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.wqe_header_size = strtol(list->tokens[4],(char**)NULL,10);
		if(style_config.wqe_header_size > 8) {//vinita_to_do
			cvmx_dprintf("ERROR: invalid wqe header size in style_config\n");
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"wqe_start_offset") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.wqe_start_offset = strtol(list->tokens[4],(char**)NULL,10);
	}
	else if((strcmp(list->tokens[2],"first_mbuf_skip") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.first_mbuf_skip = strtol(list->tokens[4],(char**)NULL,10);
	}
	else if((strcmp(list->tokens[2],"later_mbuf_skip") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.later_mbuf_skip = strtol(list->tokens[4],(char**)NULL,10);
	}
	else if((strcmp(list->tokens[2],"cache_mode") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.cache_mode = strtol(list->tokens[4],(char**)NULL,10);
	}
	else if((strcmp(list->tokens[2],"separate_1st_data_buffer_from_wqe") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"yes")==0)
			style_config.en_data_wqe_buf_diff = 1;
		else
			style_config.en_data_wqe_buf_diff = 0;
	}
	else if((strcmp(list->tokens[2],"tag_type") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"ordered")==0)
			style_config.tag_type = CVMX_SSO_TAG_TYPE_ORDERED;
		else if(strcmp(list->tokens[4],"atomic")==0)
			style_config.tag_type = CVMX_SSO_TAG_TYPE_ATOMIC;
		else if(strcmp(list->tokens[4],"untagged")==0)
			style_config.tag_type = CVMX_SSO_TAG_TYPE_UNTAGGED;
		else if(strcmp(list->tokens[4],"empty")==0)
			style_config.tag_type = CVMX_SSO_TAG_TYPE_EMPTY;
		else {
			cvmx_dprintf("ERROR: Invalid tag_type %s in pki config\n",
				     list->tokens[4]);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2], "include_in_tag_tuple") == 0) &&
			list->token_index >= 3 ) {
		token_index = 3;
		while (token_index < list->token_index) {
			if(strcmp(list->tokens[token_index], "layer_B_src") == 0)
				style_config.tag_fields.layer_B_src = 1;
			else if(strcmp(list->tokens[token_index], "layer_B_dst") == 0)
				style_config.tag_fields.layer_B_dst = 1;
			else if(strcmp(list->tokens[token_index], "layer_C_src") == 0)
				style_config.tag_fields.layer_C_src = 1;
			else if(strcmp(list->tokens[token_index], "layer_C_dst") == 0)
				style_config.tag_fields.layer_C_dst = 1;
			else if(strcmp(list->tokens[token_index], "layer_D_src") == 0)
				style_config.tag_fields.layer_D_src = 1;
			else if(strcmp(list->tokens[token_index], "layer_D_dst") == 0)
				style_config.tag_fields.layer_D_dst = 1;
			else if(strcmp(list->tokens[token_index], "layer_E_src") == 0)
				style_config.tag_fields.layer_E_src = 1;
			else if(strcmp(list->tokens[token_index], "layer_E_dst") == 0)
				style_config.tag_fields.layer_E_dst = 1;
			else if(strcmp(list->tokens[token_index], "layer_F_src") == 0)
				style_config.tag_fields.layer_F_src = 1;
			else if(strcmp(list->tokens[token_index], "layer_F_dst") == 0)
				style_config.tag_fields.layer_F_dst = 1;
			else if(strcmp(list->tokens[token_index], "layer_G_src") == 0)
				style_config.tag_fields.layer_G_src = 1;
			else if(strcmp(list->tokens[token_index], "layer_G_dst") == 0)
				style_config.tag_fields.layer_G_dst = 1;
			else if((strcmp(list->tokens[token_index], "ipv4_protocol") == 0) ||
					(strcmp(list->tokens[token_index], "ipv6_next_header")==0))
				style_config.tag_fields.ip_prot_nexthdr = 1;
			else if (strcmp(list->tokens[token_index], "exclude_tcp_sync_src") == 0)
				style_config.tag_fields.tag_sync = 1;
			else if((strcmp(list->tokens[token_index], "vxlan_vni") == 0) ||
					(strcmp(list->tokens[token_index], "nvgre_tni")==0))
				style_config.tag_fields.tag_vni = 1;
			else if (strcmp(list->tokens[token_index], "mpls_label") == 0)
				style_config.tag_fields.mpls_label = 1;
			else if (strcmp(list->tokens[token_index], "gtp_teid") == 0)
				style_config.tag_fields.tag_gtp = 1;
			else if (strcmp(list->tokens[token_index], "first_vlan") == 0)
				style_config.tag_fields.first_vlan = 1;
			else if (strcmp(list->tokens[token_index], "second_vlan") == 0)
				style_config.tag_fields.second_vlan = 1;
			else if((strcmp(list->tokens[token_index], "ipsec_ah") == 0) ||
					(strcmp(list->tokens[token_index], "gre_call_number")==0))
				style_config.tag_fields.tag_spi = 1;
			else if (strcmp(list->tokens[token_index], "input_port") == 0)
				style_config.tag_fields.input_port = 1;

			else {
				cvmx_dprintf("ERROR: Invalid tag_tuple_include config %s\n",
					     list->tokens[token_index]);
				return -1;
			}
			token_index += 1;
		}
	}
	else if((strcmp(list->tokens[2],"qpg_table_to_use") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strlen(list->tokens[4]) >= CVMX_PKI_MAX_NAME ||
				 strlen(list->tokens[4]) == 0) {
			cvmx_dprintf("ERROR: invalid qpg_table_to_use in style_config\n");
			return -1;
		}
		style_config.qpg_base_offset = cvmx_pki_find_qpg_base_offset(node,list->tokens[4]);
	}
	else if((strcmp(list->tokens[2],"qpg_qos") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		if(strcmp(list->tokens[4],"none")==0)
			style_config.qpg_qos = CVMX_PKI_QPG_QOS_NONE;
		else if(strcmp(list->tokens[4],"vlan")==0)
			style_config.qpg_qos = CVMX_PKI_QPG_QOS_VLAN;
		else if(strcmp(list->tokens[4],"mpls")==0)
			style_config.qpg_qos = CVMX_PKI_QPG_QOS_MPLS;
		else if(strcmp(list->tokens[4],"dsa_src")==0)
			style_config.qpg_qos = CVMX_PKI_QPG_QOS_DSA_SRC;
		else if(strcmp(list->tokens[4],"diffserv")==0)
			style_config.qpg_qos = CVMX_PKI_QPG_QOS_DIFFSERV;
		else if(strcmp(list->tokens[4],"higig")==0)
			style_config.qpg_qos = CVMX_PKI_QPG_QOS_HIGIG;

		else {
			cvmx_dprintf("ERROR: Invalid qpg qos %s in pki config\n",list->tokens[7]);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2],"qpg_port_msb") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.qpg_port_msb = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2],"qpg_port_shift") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
		style_config.qpg_port_shift = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2],"use_qpg_algo_to_calculate_channel")==0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		if(strcmp(list->tokens[4],"yes") == 0)
			style_config.en_qpg_calc_port_addr = 1;
		else
			style_config.en_qpg_calc_port_addr = 0;
	}
	else if((strcmp(list->tokens[2],"use_qpg_algo_to_calculate_aura")==0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		if(strcmp(list->tokens[4],"yes") == 0)
			style_config.en_qpg_calc_aura = 1;
		else
			style_config.en_qpg_calc_aura = 0;
	}
	else if((strcmp(list->tokens[2],"use_qpg_algo_to_calculate_group")==0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		if(strcmp(list->tokens[4],"yes") == 0)
			style_config.en_qpg_calc_group = 1;
		else
			style_config.en_qpg_calc_group = 0;
	}
	else if(strcmp(list->tokens[2],"end_config") == 0) {
		cvmx_pki_set_style_config(node, style_profile.name,
					  style_profile.style_num, &style_config);
	}
	else if(strcmp(list->tokens[2],"clone")==0) {
		int style;
		if((strcmp(list->tokens[3],"style_name") == 0) &&
				  (strcmp(list->tokens[4],"=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: parsing pki style config command");
				return -1;
			}
			strcpy(style_profile.name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: no name specified in clone style_config\n");
			return -1;
		}
		if((strcmp(list->tokens[6],"from_style_name") == 0) &&
				  (strcmp(list->tokens[7],"=") == 0)) {
			if(strlen(list->tokens[8]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: parsing pki style config command");
				return -1;
			}
			strcpy(style_profile.name, list->tokens[5]);
			if((style = cvmx_pki_find_style(node,list->tokens[8])) == -1){
				cvmx_dprintf("ERROR: clone_style profile %s does not exist",
					     list->tokens[5]);
				return -1;
			}
			cvmx_pki_get_style_config(node,style,&style_config);
			style_profile.style_num = CVMX_PKI_FIND_AVAL_ENTRY;
		}
		else {
			cvmx_dprintf("ERROR: no from_name specified in clone style_config\n");
			return -1;
        	}
	}
	else {
		cvmx_dprintf("ERROR: Unrecognized style command %s",list->tokens[5]);
		return -1;
	}

	return 0;
}

int parse_pki_pcam_command(token_list_t *list)
{
	int style;

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki pcam entry\n");

	if(strcmp(list->tokens[2],"start_config") == 0) {
		//cvmx_pki_reset_pcam_config(pcam_entry);//vinita_to_do uncomment when imp
		if((strcmp(list->tokens[3],"index") == 0) &&
				  (strcmp(list->tokens[4],"=") == 0)) {
			if(strcmp(list->tokens[5],"auto") == 0)
				pcam_index = CVMX_PKI_FIND_AVAL_ENTRY;
			else
				pcam_index = strtol(list->tokens[5], (char**)NULL, 10);
			if(pcam_index >= CVMX_PKI_NUM_PCAM_ENTRY) {
				cvmx_dprintf("ERROR: pcam_entry index % is > than %d",
						pcam_index, CVMX_PKI_NUM_PCAM_ENTRY);
				return -1;
			}

		}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if ((strcmp(list->tokens[2], "cluster_group_for_entry") == 0) &&
			 (strcmp(list->tokens[3],"=") == 0)) {
		int cluster_mask;
		if(strlen(list->tokens[4]) >= CVMX_PKI_MAX_NAME) {
			cvmx_dprintf("ERROR: parsing pcam cluster command\n");
			return -1;
		}
		cluster_mask = cvmx_pki_find_cluster_mask(node, list->tokens[4]);
		if(cluster_mask < 0) {
			cvmx_dprintf("ERROR: finding cluster mask for group %s\n",list->tokens[4]);
			return -1;
		}
		pcam_entry.cluster_mask = cluster_mask;
	}
	else if((strcmp(list->tokens[2], "style_to_match") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		if(strlen(list->tokens[4]) >= CVMX_PKI_MAX_NAME) {
			cvmx_dprintf("ERROR: style_to_match name in pcam_entry is > %d",
				     CVMX_PKI_MAX_NAME);
			return -1;
		}
		if((style = cvmx_pki_find_style(node,list->tokens[4])) == -1)
		{
			cvmx_dprintf("ERROR:cant find style_profile %s\n",list->tokens[4]);
			return -1;
		}
		pcam_entry.pcam_input.style = style;
	}
	else if((strcmp(list->tokens[2], "style_mask") == 0) &&
		       (strcmp(list->tokens[3],"=") == 0)) {
		pcam_entry.pcam_input.style_mask = strtol(list->tokens[7], (char**)NULL, 16);
	}
	else if((strcmp(list->tokens[2], "field_to_match") == 0) &&
			(strcmp(list->tokens[3], "=") == 0)) {
			if(strcmp(list->tokens[4],"higig") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_HIGIG_M;
			else if(strcmp(list->tokens[4],"l2_custom") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_L2_CUSTOM_M;
			else if(strcmp(list->tokens[4],"dest_mac") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_DMACL_M;
			else if(strcmp(list->tokens[4],"fulcrum_routing_addr") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_GLORT_M;
			else if(strcmp(list->tokens[4],"dsa_header") == 0 )
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_DSA_M;
			else if(strcmp(list->tokens[4],"ethertype0") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_ETHTYPE0_M;
			else if(strcmp(list->tokens[4],"ethertype1") == 0 )
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_ETHTYPE1_M;
			else if(strcmp(list->tokens[4],"ethertype2") == 0 )
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_ETHTYPE2_M;
			else if(strcmp(list->tokens[4],"ethertype3") == 0 )
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_ETHTYPE3_M;
			else if(strcmp(list->tokens[4],"mpls_label") == 0 )
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_MPLS0_M;
			else if(strcmp(list->tokens[4],"l3_flags") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_L3_FLAGS_M;
			else if(strcmp(list->tokens[4],"virt_tunnel_id") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_LD_VNI_M;
			else if(strcmp(list->tokens[4],"inner_l3_flags") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_IL3_FLAGS_M;
			else if(strcmp(list->tokens[4],"l4_dest_port_flags") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_L4_PORT_M;
			else if(strcmp(list->tokens[4],"lg_custom_match") == 0)
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_LG_CUSTOM_M;
			else {
				cvmx_dprintf("ERROR: unknown field_to_match %s in pcam_entry cmd",
					     list->tokens[4]);
				return -1;
			}
	}
	else if((strcmp(list->tokens[2], "field_mask") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		pcam_entry.pcam_input.field_mask = strtol(list->tokens[4], (char**)NULL, 16);
	}
	else if((strcmp(list->tokens[2], "data_to_match") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		pcam_entry.pcam_input.data = strtol(list->tokens[4], (char**)NULL, 16);
	}
	else if((strcmp(list->tokens[2], "data_mask") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		pcam_entry.pcam_input.data_mask = strtol(list->tokens[4], (char**)NULL, 16);
	}
	else if(strcmp(list->tokens[2], "action_on_match") == 0) {
		if((strcmp(list->tokens[3], "switch_to_style") == 0) &&
				  (strcmp(list->tokens[4], "=") == 0 )) {
			if(strcmp(list->tokens[5], "no_change") == 0)
				pcam_entry.pcam_action.style_add = 0;
			else {
				if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
					cvmx_dprintf("ERROR: change_to_style name in pcam_entry is > %d",
							CVMX_PKI_MAX_NAME);
					return -1;
				}
				if((style = cvmx_pki_find_style(node,list->tokens[5])) == -1)
				{
					cvmx_dprintf("ERROR:cant find change_to_style %s in pcam entry\n",list->tokens[5]);
					return -1;
				}
				pcam_entry.pcam_action.style_add = (int)(style - pcam_entry.pcam_input.style);
			}
		}
		else if((strcmp(list->tokens[3], "parse_mode_change") == 0) &&
				       (strcmp(list->tokens[4], "=") == 0 )) {
			if(strcmp(list->tokens[5], "none") == 0)
				pcam_entry.pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
			else if(strcmp(list->tokens[5], "skip_to_lb") == 0)
				pcam_entry.pcam_action.parse_mode_chg = CVMX_PKI_PARSE_SKIP_TO_LB;
			else if(strcmp(list->tokens[5], "skip_to_lc") == 0)
				pcam_entry.pcam_action.parse_mode_chg = CVMX_PKI_PARSE_SKIP_TO_LC;
			else if(strcmp(list->tokens[5], "skip_to_ld") == 0)
				pcam_entry.pcam_action.parse_mode_chg = CVMX_PKI_PARSE_SKIP_TO_LD;
			else if(strcmp(list->tokens[5], "skip_to_lg") == 0)
				pcam_entry.pcam_action.parse_mode_chg = CVMX_PKI_PARSE_SKIP_TO_LG;
			else if(strcmp(list->tokens[5], "skip_all") == 0)
				pcam_entry.pcam_action.parse_mode_chg = CVMX_PKI_PARSE_SKIP_ALL;
			else {
				cvmx_dprintf("ERROR: unknown parse_mode_change %s in pcam_entry\n",
					     list->tokens[5]);
				return -1;
			}
		}
		else if((strcmp(list->tokens[3], "set_layer_type") == 0) &&
				       (strcmp(list->tokens[4], "=") == 0 )) {
			if(strcmp(list->tokens[5], "none") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_NONE;
			else if(strcmp(list->tokens[5], "vlan") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_VLAN;
			else if(strcmp(list->tokens[5], "snap_payload") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_SNAP_PAYLD;
			else if(strcmp(list->tokens[5], "arp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_ARP;
			else if(strcmp(list->tokens[5], "rarp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_RARP;
			else if(strcmp(list->tokens[5], "ipv4") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IP4;
			else if(strcmp(list->tokens[5], "ipv4_option") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IP4_OPT;
			else if(strcmp(list->tokens[5], "ipv6") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IP6;
			else if(strcmp(list->tokens[5], "ipv6_option") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IP6_OPT;
			else if(strcmp(list->tokens[5], "ipsec_esp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IPSEC_ESP;
			else if(strcmp(list->tokens[5], "ip_fragment") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IPFRAG;
			else if(strcmp(list->tokens[5], "ip_compression") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_IPCOMP;
			else if(strcmp(list->tokens[5], "tcp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_TCP;
			else if(strcmp(list->tokens[5], "udp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_UDP;
			else if(strcmp(list->tokens[5], "sctp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_SCTP;
			else if(strcmp(list->tokens[5], "udp_vxlan") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_UDP_VXLAN;
			else if(strcmp(list->tokens[5], "gre") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_GRE;
			else if(strcmp(list->tokens[5], "gtp") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_GTP;
			else if(strcmp(list->tokens[5], "software_defined_28") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_SW28;
			else if(strcmp(list->tokens[5], "software_defined_29") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_SW29;
			else if(strcmp(list->tokens[5], "software_defined_30") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_SW30;
			else if(strcmp(list->tokens[5], "software_defined_31") == 0)
				pcam_entry.pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_SW31;
			else {
				cvmx_dprintf("ERROR: unknown set_layer_type %s in pcam_entry action\n",
					     list->tokens[5]);
				return -1;
			}
		}
		else if((strcmp(list->tokens[3], "parse_flag_set") == 0) &&
				  (strcmp(list->tokens[4], "=") == 0 )) {
			if(strcmp(list->tokens[5], "none") == 0)
				pcam_entry.pcam_action.parse_flag_set = 0;
			else if (strcmp(list->tokens[5], "wqe_pf1") == 0)
				pcam_entry.pcam_action.parse_flag_set = 1;
			else if (strcmp(list->tokens[5], "wqe_pf2") == 0)
				pcam_entry.pcam_action.parse_flag_set = 2;
			else if (strcmp(list->tokens[5], "wqe_pf3") == 0)
				pcam_entry.pcam_action.parse_flag_set = 3;
			else if (strcmp(list->tokens[5], "wqe_pf4") == 0)
				pcam_entry.pcam_action.parse_flag_set = 4;
			else {
				cvmx_dprintf("ERROR: unknown parse_flag_set %s in pcam_entry action\n",
					     list->tokens[5]);
				return -1;
			}
		}
		else if((strcmp(list->tokens[3], "pointer_advance") == 0) &&
				       (strcmp(list->tokens[4], "=") == 0 )) {
			pcam_entry.pcam_action.pointer_advance = strtol(list->tokens[5], (char**)NULL, 10);
			if(pcam_entry.pcam_action.pointer_advance > 256) {
				cvmx_dprintf("ERROR: pointer_advance %d is > 256\n",
					     pcam_entry.pcam_action.pointer_advance);
				return -1;
			}
		}
		else {
			cvmx_dprintf("ERROR:unknown action cmd %s in pcam_cfg",list->tokens[3]);
			return -1;
		}
	}
	else if((strcmp(list->tokens[2], "end_config") == 0)) {
		//if(cvmx_pki_check_pcam_entry(&pcam_entry)) //vinita_to_do
		//	return -1;
		if(pcam_entry.pcam_input.field == CVMX_PKI_PCAM_TERM_E_DMACL_M){
			uint64_t dest_mac_h = (pcam_entry.pcam_input.data >> 32) & 0xffff;
			uint64_t dest_mac_h_mask = (pcam_entry.pcam_input.data_mask >> 32) & 0xffff;
			if(pcam_entry.pcam_input.data_mask & 0xffffffff) {
				pcam_entry.pcam_input.data &= 0xffffffff;
				pcam_entry.pcam_input.data_mask &= 0xffffffff;
				cvmx_pki_set_pcam_entry(node, pcam_index, pcam_entry.cluster_mask,
						pcam_entry.pcam_input, pcam_entry.pcam_action);
			}
			if(pcam_entry.pcam_input.data_mask & 0xffff00000000) {
				pcam_entry.pcam_input.field = CVMX_PKI_PCAM_TERM_E_DMACH_M;
				pcam_entry.pcam_input.data = dest_mac_h;
				pcam_entry.pcam_input.data_mask = dest_mac_h_mask;
				cvmx_pki_set_pcam_entry(node, pcam_index, pcam_entry.cluster_mask,
						pcam_entry.pcam_input, pcam_entry.pcam_action);
			}
		}
		else
			cvmx_pki_set_pcam_entry(node, pcam_index, pcam_entry.cluster_mask,
					       pcam_entry.pcam_input, pcam_entry.pcam_action);
	}
	else {
		cvmx_dprintf("ERROR: Invalid pcam_entry command %s",list->tokens[2]);
		return -1;
	}
	return 0;
}

void cvmx_pki_reset_sso_grp_profile(struct cvmx_pki_sso_grp_profile *grp_profile)
{
	grp_profile->grp_name[0] = '\0';
	grp_profile->grp_num = CVMX_PKI_NOT_ASSIGNED;
}

int cvmx_pki_check_sso_grp_profile(struct cvmx_pki_sso_grp_profile *grp_profile)
{
	if(grp_profile->grp_name[0] == '\0') {
		cvmx_dprintf("ERROR: group_name not specified in group_config cmd\n");
		return -1;
	}
	if(grp_profile->grp_num == CVMX_PKI_NOT_ASSIGNED) {
		cvmx_dprintf("ERROR: group_num not specified in group_config cmd\n");
		return -1;
	}
	return 0;
}

int parse_pki_sso_command(token_list_t *list)
{

	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki sso comand\n");

	if(strcmp(list->tokens[2], "start_config") == 0) {
		//cvmx_pki_get_default_sso_grp_profile(&sso_grp_profile);
		cvmx_pki_reset_sso_grp_profile(&sso_grp_profile);//vinita_to_do
		if((strcmp(list->tokens[3], "group_name") == 0) &&
				  (strcmp(list->tokens[4], "=") == 0)) {
			if(strlen(list->tokens[5]) >= CVMX_PKI_MAX_NAME) {
				cvmx_dprintf("ERROR: sso grp name > %d in pki sso cmd\n",
					     CVMX_PKI_MAX_NAME-1);
				return -1;
			}
			strcpy(sso_grp_profile.grp_name, list->tokens[5]);
		}
		else {
			cvmx_dprintf("ERROR: Missing sso grp name\n");
			return -1;
         	}
	}
	else if (strcmp(list->tokens[2], "node") == 0) {
		if(parse_pki_node(list))
			return -1;
	}
	else if((strcmp(list->tokens[2], "group_priority") == 0) &&
				(strcmp(list->tokens[3],"=") == 0)) {
		sso_grp_profile.priority = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2], "group_weight") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		sso_grp_profile.weight = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2], "group_affinity") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		sso_grp_profile.affinity = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if((strcmp(list->tokens[2], "group_core_affinity") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		sso_grp_profile.core_affinity_mask = strtol(list->tokens[4], (char**)NULL, 16);
	}
	else if((strcmp(list->tokens[2], "group_core_affinity_mask_set") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		sso_grp_profile.core_affinity_mask_set = strtol(list->tokens[4], (char**)NULL, 16);
	}
	else if((strcmp(list->tokens[2], "group_number") == 0) &&
			(strcmp(list->tokens[3],"=") == 0)) {
		if(strcmp(list->tokens[4], "auto") == 0)
			sso_grp_profile.grp_num = CVMX_PKI_FIND_AVAL_ENTRY;
		else
			sso_grp_profile.grp_num = strtol(list->tokens[4], (char**)NULL, 10);
	}
	else if(strcmp(list->tokens[2], "end_config") == 0) {
		if(cvmx_pki_check_sso_grp_profile(&sso_grp_profile))//vinita_to_do
			return -1;
		cvmx_pki_set_sso_group_config(node, sso_grp_profile);
	}
	else {
		cvmx_dprintf("ERROR: Unrecognized sso_group_config cmd %s", list->tokens[2]);
		return -1;
	}
	return 0;
}


int parse_pki_config_command(token_list_t *list)
{
	if(dbg_pki_parse)
		cvmx_dprintf("parsing pki config\n");

	/* node is local if not configured*/
	node = cvmx_get_node_num();

	if(strcmp(list->tokens[1], "cluster_group_config") == 0) {
		if(parse_pki_cluster_command(list)) {
			cvmx_dprintf("ERROR: parsing pki cluster profile commands\n");
			show_token_list(list);
		}
	}
	else if(strcmp(list->tokens[1],"pool_config")== 0) {
		if(parse_pki_pool_command(list)){
			cvmx_dprintf("ERROR: parsing pki pool commands\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[1],"aura_config")== 0) {
		if(parse_pki_aura_command(list)){
			cvmx_dprintf("ERROR: parsing pki aura commands\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[1],"sso_group_config")== 0) {
		if(parse_pki_sso_command(list)){
			cvmx_dprintf("ERROR: parsing pki sso commands\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[1],"qpg_config")== 0) {
		if(parse_pki_qpg_command(list)){
			cvmx_dprintf("ERROR: parsing pki qpg commands\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[1],"style_config")== 0) {
		if(parse_pki_style_command(list)) {
			cvmx_dprintf("ERROR: parsing pki style config commands\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[1],"port_config")== 0) {
		if(parse_pki_port_command(list)){
			cvmx_dprintf("ERROR: parsing pki port commands\n");
			return -1;
		}
	}
	else if(strcmp(list->tokens[1],"pcam_entry")== 0) {
		if(parse_pki_pcam_command(list)){
			cvmx_dprintf("ERROR: parsing pki pcam commands\n");
			return -1;
		}
	}
	else {
		cvmx_dprintf("ERROR: Unrecognized pki command %s",list->tokens[4]);
		return -1;
	}
	return 0;
}
