#include <stdlib.h>
#include "cvmx.h"
#include "cvmx-config-parse.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "string.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-pko-internal-ports-range.h"
#include "cvmx-pko.h"
#include "cvmx-fpa.h"
#include "cvmx-ipd.h"
#include "cvmx-dma-engine.h"
#include "cvmx-zip.h"
#include "cvmx-tim.h"
#include "cvmx-dfa.h"
#include "cvmx-dma-engine.h"
#include "cvmx-raid.h"
#include "cvmx-helper-spi.h"
#include "cvmx-llm.h"
#include "cvmx-ilk.h"
#include "cvmx-bch.h"
#include "octeon-feature.h"

#define SC  strcmp
#define ST  strtol
#define TK  list->tokens

extern int parse_pki_config_command(token_list_t *list);
static char *cvmx_app_config_str = 0;
static int dbg_parse = 0;

void *parse_alloc(size_t sz)
{
	return malloc(sz);

}

void parse_free(void *ptr)
{
	free(ptr);
}

void destroy_token_list(token_list_t *list)
{
	int i;

	for(i=0; i< list->max_tokens; i++)
	{
		if (TK[i] != 0) parse_free(TK[i]);
	}
}

token_list_t *init_token_list(int max_tokens, int max_token_size)
{
	int i;
	token_list_t *list = parse_alloc(sizeof(token_list_t));

	//bzero
	if (list == 0) return 0;
	list->max_tokens = max_tokens;
	list->tokens = (char **) parse_alloc(sizeof(char*) * max_tokens);
	list->max_token_size = max_token_size;
	for(i=0; i < max_tokens; i++)
	{
		TK[i] = (char *) parse_alloc(max_token_size);
		if (TK[i] == 0)  {
			destroy_token_list(list);
			return 0;
		}
	}
	list->token_index = 0;
	list->max_line_length = 512;
	return list;

}

void show_token_list(token_list_t *list)
{
	int cnt = list->token_index;
	int i;

	cvmx_dprintf("cnt=%02d : ",cnt);

	for (i=0; i<cnt; i++)
	{
		cvmx_dprintf("%s##", TK[i]);
	}

	cvmx_dprintf("\n");
}

void reset_token_index(token_list_t *list)
{
	list->token_index = 0;
}

int add_token(token_list_t *list, char *str, int offset, int sz)
{

	char *dst;

	//cvmx_dprintf("sz=%d str=%s \n",sz, str);
	if (str == 0)
	{
		cvmx_dprintf("%s: token is null\n", __FUNCTION__);
		return -1;
	}
	if ((sz+1) > list->max_token_size)
	{
		cvmx_dprintf("token size too big sz=%d\n",sz);
		return -2;
	}

	if (list->token_index >= list->max_tokens)
	{
		cvmx_dprintf("ERROR: max_tokens=%d reached \n", list->max_tokens);
		return -3;
	}
	dst = TK[list->token_index];
	memcpy(dst, str + offset, sz);
	*(dst + sz) = '\0';
	list->token_index++;
	return 0;

}

int get_token_list(token_list_t *list, char *str, int start_index)
{
	int i=start_index;

	while (1) {
		int start_of_token, len_of_token;

		if ( (str[i] == '\0') || (str[i] == '\n'))
			return i;
		while ((str[i] == ' ') || (str[i] == '\t') ) {
			i++;
			if ( (str[i] == '\0') || (str[i] == '\n') )
				return i;
		}
		start_of_token = i;
		//printf("start of token=%d is %d ", token_cnt, start_of_token);
		while ((str[i] != ' ') && (str[i] != '\t') && (str[i] != '=') ) {
			i++;
			if (str[i] == '\0' || str[i] == '\n') {
				len_of_token = i - start_of_token;
				if (len_of_token > 0) {
					//printf("-end of token=%d is %d \n", token_cnt, i-1);
					add_token(list, str, start_of_token, len_of_token);
				}
				return i;
			}
		}
		//printf("end of token=%d is %d \n", token_cnt, i-1);
		len_of_token = i - start_of_token;
		add_token(list, str, start_of_token, len_of_token);
		if (str[i] == '=') add_token(list, str, i, 1);
		i++;
	}
}

void cvmx_set_app_config_str(char *str)
{
	if (str != 0)
		cvmx_app_config_str = str;
}

int  is_app_config_string_set(void)
{
	if (cvmx_app_config_str)
		return 1;
	return 0;
}

#define CVMX_PARSE_INF_TYPE_CNT  5
#define CVMX_PARSE_INF_INDEX_MAX 5
static struct interface_type{
	char *name;
	int max_cnt;
} cvmx_interface_types[CVMX_PARSE_INF_TYPE_CNT] =
	{ {"gmx",5}, {"ilk", 2 }, { "loop", 1 }, { "srio", 4}, { "npi", 1 } };

static int interface_index[CVMX_PARSE_INF_TYPE_CNT][CVMX_PARSE_INF_INDEX_MAX] =
	{ { 0,  1, -1, -1 , -1 },  /*gmx*/
	  { 5,  6, -1, -1 , -1 },  /*ilk*/
	  { 3, -1, -1, -1 , -1 },  /*loop*/
	  { 4,  5,  6,  7 , -1 },  /*srio*/
	  { 2, -1, -1, -1 , -1 } };/*npi*/

static int interface_index_pknd[5][5] =
	{ { 0,  1,  2,  3 ,  4 },    /*gmx*/
	  { 5,  6, -1, -1 , -1 },    /*ilk*/
	  { 8, -1, -1, -1 , -1 },    /*loop*/
	  {-1, -1, -1, -1 , -1 },    /*srio*/
	  { 7, -1, -1, -1 , -1 } };  /*npi*/


static int cvmx_get_interface_number(int type, int index)
{
	if (type > CVMX_PARSE_INF_TYPE_CNT) {
		cvmx_dprintf("ERROR: interface type=%d \n",type);
		return -1;
	}
	if (index > CVMX_PARSE_INF_INDEX_MAX) {
		cvmx_dprintf("ERROR: interface index=%d \n",index);
		return -1;
	}
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return interface_index_pknd[type][index];
	return interface_index[type][index];
}

static int cvmx_get_interface_type_index(char *type) {
	int len = sizeof(cvmx_interface_types)/sizeof(struct interface_type) ;
	int i;

	for(i=0; i<len; i++) {
		if (SC(type, cvmx_interface_types[i].name) == 0)
			return i;
	}
	return -1;
}

void get_interface_name_token_list(token_list_t *list, char *str)
{
	int start_of_token, i=0;

	while(1) {
		if (str[i] == '\0')
			return;
		start_of_token=i;
		while ( (str[i] != ':') && (str[i] != '[')  && str[i] != ']' &&
			(str[i] != '.') && (str[i] != ',') ) {
			i++;
			if (str[i] == '\0') {
				add_token(list, str, start_of_token,
					  i - start_of_token);
				return;
			}
		}
		if ((i-start_of_token) > 0 ) {
			add_token(list, str, start_of_token, i - start_of_token);
			add_token(list, str, i, 1);
		} else {
			add_token(list, str, i, 1);
		}
		i++;
	}

}

int parse_interface_name(char *name, int interfaces[], int max_interface_cnt)
{

	token_list_t *list;
	list = init_token_list(30, 128);

	get_interface_name_token_list(list, name);
	//show_token_list(list);
	if (list->token_index < 3 ) {
		cvmx_dprintf("invalid interface name :%s token_cnt=%d \n",
			     name, list->token_index);
		show_token_list(list);
		return -1;
	}

	/* Looking for something like type:n */
	if ( (list->token_index == 3 ) && (SC(TK[1],":") == 0) ) {
		int interface_num = ST(TK[2], 0, 10);
		int type_index = cvmx_get_interface_type_index(TK[0]);

		//cvmx_dprintf("type_index=%d num=%d \n", type_index, interface_num);
		if (type_index < 0) {
			cvmx_dprintf("invalid interface type:%s\n", TK[0]);
			return -1;
		}

		if (interface_num >= cvmx_interface_types[type_index].max_cnt ) {
			cvmx_dprintf("invalid interface index=%d for type=%s\n",
				     interface_num, TK[0]);
			return -1;
		}
		interfaces[0] = cvmx_get_interface_number(type_index, interface_num);
		//cvmx_dprintf("interface=%d \n", interfaces[0]);
		return 1;
	}
	/* Looking for something like type:[n..n] */
	if ( (list->token_index == 8 ) && (SC(TK[2],"[") == 0)  &&
	     (SC(TK[4],".") == 0) && (SC(TK[5],".") == 0) &&
	     (SC(TK[7],"]") == 0) ) {
		int intf_start = ST(TK[3], 0, 10);
		int intf_end   = ST(TK[6], 0, 10);
		int type_index = cvmx_get_interface_type_index(TK[0]);
		int i,j;

		if (type_index < 0) {
			cvmx_dprintf("invalid interface type:%s\n", TK[0]);
			return -1;
		}
		if ((intf_end <= intf_start) || (intf_start < 0) ||
		    intf_end >= cvmx_interface_types[type_index].max_cnt) {
			cvmx_dprintf("invalid interface range %d..%d max=%d\n",
				     intf_start, intf_start,
				     cvmx_interface_types[type_index].max_cnt);
			return -1;
		}
		j=0;
		for(i=intf_start; i<=intf_end; i++) {
			int inum = cvmx_get_interface_number(type_index, i);
			if (inum != -1)
				interfaces[j++] = inum;
		}
		return j;
	}
	//cvmx_dprintf("%s:%d Here... %s %s \n", __FUNCTION__, __LINE__,
	//	     TK[2], TK[list->token_index -1]);
	/* Looking for something like type:[n,*,n] * is repetitions of ,n*/
	if ( (SC(TK[2],"[") == 0) && (SC(TK[list->token_index-1], "]") == 0) ) {
		int type_index = cvmx_get_interface_type_index(TK[0]);
		int i,j;

		//cvmx_dprintf("%s:%d Here..\n", __FUNCTION__, __LINE__);
		if (type_index < 0) {
			cvmx_dprintf("invalid interface type:%s\n", TK[0]);
			return -1;
		}
		for(i=4; i < (list->token_index-1); i+=2) {
			if (SC(TK[i],",") != 0) {
				cvmx_dprintf("invalid seperator i=%d\n",i);
				return -1;
			}
		}
		j = 0;
		for(i=3; i < (list->token_index-1); i++) {
			int num = ST(TK[i], 0, 10);
			int inum;

			if (num >  cvmx_interface_types[type_index].max_cnt) {
				cvmx_dprintf("invalid index %d for type=%s\n", num,
					     cvmx_interface_types[type_index].name);
			}
			inum = cvmx_get_interface_number(type_index, i);
			if (inum != -1)
				interfaces[j++] = inum;
			if (j >= max_interface_cnt) {
				cvmx_dprintf("too many interface numbers=%d\n", j);
				return -1;
			}
		}
		return j;
	}
	return -1;

}

struct cvmx_pko_config
{
	int internal_port_cnt;
	int queues_per_internal_port;
};

int is_interface_disabled(int interface_num)
{
	int mode = cvmx_helper_interface_get_mode(interface_num);
	if (mode == CVMX_HELPER_INTERFACE_MODE_DISABLED)
		return 1;
	return 0;
}

struct cvmx_pko_config __cvmx_pko_config[CVMX_HELPER_MAX_IFACE]
[CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] =
    {[0 ... CVMX_HELPER_MAX_IFACE - 1] =
     {[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
      {CVMX_HELPER_CFG_INVALID_VALUE,CVMX_HELPER_CFG_INVALID_VALUE}}};

int parse_fpa_config_command(token_list_t *list, cvmx_fpa_pool_config_t* fpa_config,
			     const char *pool_name)
{
	int64_t pool_num;

	if((SC(TK[2], "pool_number") == 0) && (SC(TK[3], "=") == 0)){
		pool_num = ST(TK[4], (char**)NULL, 10);
		if(pool_num >= CVMX_FPA_NUM_POOLS) {
			cvmx_dprintf("ERROR: Invalid pool number %d in %s command\n",
				     (int)pool_num, pool_name);
			return -1;
		}
		fpa_config->pool_num = pool_num;
	}
	else if((SC(TK[2], "buffer_size") == 0) && (SC(TK[3], "=")) == 0)
		fpa_config->buffer_size = ST(TK[4], (char**)NULL, 10);
	else if((SC(TK[2], "buffer_count") == 0) && (SC(TK[3], "=")) == 0)
		fpa_config->buffer_count = ST(TK[4], (char**)NULL, 10);
	else {
		cvmx_dprintf("ERROR: Invalid %s config = %s \n", pool_name, TK[2]);
		return -1;
	}

	if(dbg_parse)
		cvmx_dprintf("%s pool_number=%d buffer_size=%d buffer_count=%d \n",
			     pool_name, (int)fpa_config->pool_num,
			     (int)fpa_config->buffer_size,(int)fpa_config->buffer_count);
	return 0;
}


cvmx_fpa_pool_config_t pko_fpa_cfg;

void show_pko_queue_config()
{
	int i, index;

	cvmx_dprintf("==== pko queue config ====\n");
	for(i=0; i<CVMX_HELPER_MAX_IFACE; i++) {
		int port_cnt = __cvmx_helper_early_ports_on_interface(i);
		if (is_interface_disabled(i))
			continue;
		cvmx_dprintf("interface:%d ",i);
		for (index=0; index<port_cnt; index++) {
			cvmx_dprintf("%02d ", __cvmx_pko_config[i][index].queues_per_internal_port);
		}
		cvmx_dprintf("\n");
	}
}

void show_pko_port_config()
{
	int i, index;

	cvmx_dprintf("==== pko port config ====\n");
	for(i=0; i<CVMX_HELPER_MAX_IFACE; i++) {
		int port_cnt = __cvmx_helper_early_ports_on_interface(i);
		if (is_interface_disabled(i))
			continue;
		cvmx_dprintf("interface:%d ",i);
		for (index=0; index<port_cnt; index++) {
			cvmx_dprintf("%02d ", __cvmx_pko_config[i][index].internal_port_cnt);
		}
		cvmx_dprintf("\n");
	}
}

int parse_pko_config_command(token_list_t *list)
{
	int intf_index = -1;
	int queue_cnt = -1;
	int port_cnt = -1;
	int tok_offset = 0;
	int interfaces[CVMX_HELPER_MAX_IFACE];
	int intf_cnt, i;

	if (SC(TK[1],"command_queue") == 0)
		return (parse_fpa_config_command(list, &pko_fpa_cfg, "pko command queue pool"));
	if  ( (SC(TK[0],"pko") != 0) || (SC(TK[1],"interface") != 0) ||
	      (SC(TK[2],"=") != 0) || (list->token_index < 9) ) {
		cvmx_dprintf("ERROR: invalid pko command :");
		return -1;
		//show_token_list(list);
	}
	intf_cnt = parse_interface_name(TK[3], interfaces,
					CVMX_HELPER_MAX_IFACE);

	if (intf_cnt < 0) {
		cvmx_dprintf("ERROR: invalid interface name = %s \n", TK[3]);
		return -1;
	}

	if (SC(TK[4],"interface_index") == 0)
	{
		if (intf_cnt != 1) {
			cvmx_dprintf("ERROR: cannot specify interface_index"
				     " when using wild card for specifying"
				     " interface name\n");
			return -1;
		}
		if (SC(TK[5],"=") != 0) {
			cvmx_dprintf("ERROR: invalid pko config command : no "
				     "equal sign after interface_index \n");
			return -1;
		}
		tok_offset = 3;
		intf_index = ST(TK[6], 0, 10);
		if ((intf_index >= CVMX_HELPER_CFG_MAX_PORT_PER_IFACE) ||
			(intf_index < 0)) {
			cvmx_dprintf("ERROR: invalid intf_index=%d max=%d\n",
				     intf_index,
				     CVMX_HELPER_CFG_MAX_PORT_PER_IFACE-1);
			return -1;
		}
	}

	if ( (SC(TK[tok_offset + 4],"internal_port_cnt") != 0) ||
	     (SC(TK[tok_offset + 5],"=") != 0) ) {
		cvmx_dprintf("ERROR: invalid pko config command  while looking"
			     "for internal_port_cnt \n");
		return -1;
	}
	if ( (SC(TK[tok_offset+7],"queues_per_internal_port") != 0) ||
	     (SC(TK[tok_offset+8],"=") != 0) )  {
		cvmx_dprintf("ERROR: invalid pko config command while looking"
			     "for queues_per_internal_port\n");
		return -1;
	}
#define PKO_CFG __cvmx_pko_config

	port_cnt = (int) ST(TK[tok_offset+6], 0, 10);
	queue_cnt  = (int) ST(TK[tok_offset+9], 0, 10);

	for(i=0; i<intf_cnt; i++) {
		int intf_num = interfaces[i];
		if (intf_num == -1)
			continue;
		if ( (is_interface_disabled(intf_num) == 1)  && dbg_parse) {
			cvmx_dprintf("skipping config interace_num=%d :",
				     intf_num);
			show_token_list(list);
		}
		if (intf_index != -1) {
			//cvmx_dprintf("%d:%d %d %d \n", intf_num, intf_index, port_cnt, queue_cnt);
			PKO_CFG[intf_num][intf_index].internal_port_cnt = port_cnt;
			PKO_CFG[intf_num][intf_index].queues_per_internal_port = queue_cnt;
		} else {
			int index;
			int port_max = __cvmx_helper_early_ports_on_interface(intf_num);
			for(index=0; index<port_max; index++) {
				PKO_CFG[intf_num][index].internal_port_cnt = port_cnt;
				PKO_CFG[intf_num][index].queues_per_internal_port = queue_cnt;
			}
		}
		if(dbg_parse)
			cvmx_dprintf("interface=%d interface_index=%d queue_cnt"
				     "=%d port_cnt=%d\n", intf_num, intf_index,
				     queue_cnt, port_cnt);
	}
#undef PKO_CFG
	return 0;
}

int cvmx_apply_pko_config(void)
{
	int i,j,n;
#define PKO_CFG __cvmx_pko_config

	//show_pko_queue_config();
	//show_pko_port_config();

	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		n = __cvmx_helper_early_ports_on_interface(i);
		for (j = 0; j < n; j++) {
			int port_cnt = PKO_CFG[i][j].internal_port_cnt ;
			int rv;
			int queues_cnt = PKO_CFG[i][j].queues_per_internal_port;
			if (queues_cnt == CVMX_HELPER_CFG_INVALID_VALUE)
				queues_cnt = 1;
			if (port_cnt == CVMX_HELPER_CFG_INVALID_VALUE)
				port_cnt = 1;
			if (!octeon_has_feature(OCTEON_FEATURE_PKND))
				port_cnt = 1;
			if(dbg_parse)
				cvmx_dprintf("alloc internal ports for interface=%d"
				     "port=%d cnt=%d\n", i, j, port_cnt);
			rv = cvmx_pko_alloc_iport_and_queues(i, j, port_cnt,
							     queues_cnt);
			if (rv < 0)
				return -1;
		}
	}
	return 0;
#undef PKO_CFG

}

int parse_raid_config_command(token_list_t *list, cvmx_raid_config_t* raid_config)
{
	if(dbg_parse)
		cvmx_dprintf("parsing raid_config\n");
	if(SC(TK[1], "command_queue") == 0)
		return (parse_fpa_config_command(list,&raid_config->command_queue_pool,
						 "raid command_queue pool"));
	else {
		cvmx_dprintf("ERROR: Invalid raid config command = %s\n", TK[1]);
		return -1;
	}
}

int parse_bch_config_command(token_list_t *list,
			     cvmx_bch_app_config_t* bch_config)
{
	if (dbg_parse)
		cvmx_dprintf("parsing bch_config\n");
	if (SC(TK[1], "bch_pool") == 0) {
		return (parse_fpa_config_command(list,
						 &bch_config->command_queue_pool,
						 "bch command_queue pool"));
	} else {
		cvmx_dprintf("ERROR: Invalid bch config command= %s\n", TK[1]);
		return -1;
	}
}

int parse_dma_config_command(token_list_t *list, cvmx_dma_config_t* dma_config)
{

	if(dbg_parse)
		cvmx_dprintf("parsing dma_config\n");

	if(SC(TK[1], "command_queue") == 0)
		return (parse_fpa_config_command(list, &dma_config->command_queue_pool,
						 "dma command_queue pool"));
	else {
		cvmx_dprintf("ERROR: Invalid dma config command = %s\n", TK[1]);
		return -1;
	}
}

int parse_timer_config_command(token_list_t *list, cvmx_tim_config_t* timer_config)
{
	if(dbg_parse)
		cvmx_dprintf("parsing timer_config\n");

	if(SC(TK[1], "timer_pool") == 0)
		return (parse_fpa_config_command(list,&timer_config->timer_pool, "timer_pool"));
	else {
		cvmx_dprintf("ERROR: Invalid timer config command = %s\n", TK[1]);
		return -1;
	}
}

int parse_dfa_config_command(token_list_t *list, cvmx_dfa_app_config_t* dfa_config)
{
	if(dbg_parse)
		cvmx_dprintf("parsing dfa_config\n");

	if(SC(TK[1], "dfa_pool") == 0)
		return (parse_fpa_config_command(list,&dfa_config->dfa_pool, "dfa_pool"));
	else {
		cvmx_dprintf("ERROR: Invalid dfa config command= %s\n", TK[1]);
		return -1;
	}
}

int parse_zip_config_command(token_list_t *list, cvmx_zip_config_t* zip_config)
{
	if(dbg_parse)
		cvmx_dprintf("parsing zip_config\n");
	if(SC(TK[1], "zip_pool") == 0) {
		return (parse_fpa_config_command(list,&zip_config->zip_pool, "zip_pool"));
	}
	else {
		cvmx_dprintf("ERROR: Invalid zip config command= %s\n", TK[1]);
		return -1;
	}
}

int parse_rgmii_config_command(token_list_t *list)
{
	uint64_t dis_bp;
	if(dbg_parse)
		cvmx_dprintf("parsing rgmii_config\n");
	if (list->token_index < 10) {
		cvmx_dprintf("ERROR: Invalid rgmii config command\n");
		return -1;
	}
	if((SC(TK[1], "rgmii") == 0) &&
		   (SC(TK[7], "disable_backpressure") == 0) &&
		   	(SC(TK[8], "=") == 0)) {
		if(SC(TK[9], "yes") == 0)
			dis_bp = 1;
		else
			dis_bp = 0;
		cvmx_rgmii_set_back_pressure(dis_bp);
	}
	return 0;
}

int parse_ipd_config_command(token_list_t *list, cvmx_ipd_config_t* ipd_config)
{
	int token_index;

	if(dbg_parse)
		cvmx_dprintf("parsing ipd_config\n");
	if (list->token_index < 4) {
		cvmx_dprintf("ERROR: Invalid index=%d IPD config command\n", list->token_index);
		return -1;
	}
	if((SC(TK[1], "packet_pool") == 0) && (list->token_index == 5))
		return (parse_fpa_config_command(list, &ipd_config->packet_pool, "packet pool"));
	else if((SC(TK[1], "wqe_pool")) == 0 && (list->token_index == 5))
		return (parse_fpa_config_command(list, &ipd_config->wqe_pool, "wqe pool"));
	else if((SC(TK[1], "first_mbuf_skip") == 0) && (SC(TK[2], "=") == 0))
		ipd_config->first_mbuf_skip = ST(TK[3], (char**)NULL, 10);
	else if((SC(TK[1], "not_first_mbuf_skip") == 0) && (SC(TK[2], "=") == 0))
		ipd_config->not_first_mbuf_skip = ST(TK[3], (char**)NULL, 10);
	else if((SC(TK[1], "cache_mode") == 0) && (SC(TK[2], "=") == 0))
		ipd_config->cache_mode = ST(TK[3], (char**)NULL, 10);
	else if((SC(TK[1], "enable_ipd_in_helper_init") == 0) && (SC(TK[2], "=") == 0))
		ipd_config->ipd_enable = ST(TK[3], (char**)NULL, 10);
	else if((SC(TK[1], "enable_len_M8_fix") == 0) && (SC(TK[2], "=") == 0))
		ipd_config->enable_len_M8_fix = ST(TK[3], (char**)NULL, 10);
	else if( list->token_index == 10) {
		if((SC(TK[7], "parse_mode") == 0) && (SC(TK[8], "=") == 0)) {
			if(SC(TK[9], "skip_L2") == 0)
				ipd_config->port_config.parse_mode = CVMX_PIP_PORT_CFG_MODE_SKIPL2;
			else if(SC(TK[9], "skip_IP") == 0)
				ipd_config->port_config.parse_mode = CVMX_PIP_PORT_CFG_MODE_SKIPIP;
			else
				ipd_config->port_config.parse_mode = CVMX_PIP_PORT_CFG_MODE_NONE;
		}
		else if((SC(TK[7], "tag_type") == 0) && (SC(TK[8], "=") == 0)) {
			if(SC(TK[9], "ordered") == 0)
				ipd_config->port_config.tag_type = CVMX_POW_TAG_TYPE_ORDERED;
			else if(SC(TK[9], "atomic") == 0)
				ipd_config->port_config.tag_type = CVMX_POW_TAG_TYPE_ATOMIC;
			else if(SC(TK[9], "null") == 0)
				ipd_config->port_config.tag_type = CVMX_POW_TAG_TYPE_NULL;
			else if(SC(TK[9], "null_null") == 0)
				ipd_config->port_config.tag_type = CVMX_POW_TAG_TYPE_NULL_NULL;
			else {
				cvmx_dprintf("ERROR: Invalid tag_type %s in ipd config\n",TK[9]);
				return -1;
			}

		}
		else if((SC(TK[7], "tag_mode") == 0) && (SC(TK[8], "=") == 0)) {
			if(SC(TK[9], "tuple_tag") == 0)
				ipd_config->port_config.tag_mode = CVMX_PIP_TAG_MODE_TUPLE;
			else if(SC(TK[9], "mask_tag") == 0)
				ipd_config->port_config.tag_mode = CVMX_PIP_TAG_MODE_MASK;
			else if(SC(TK[9], "ip_or_mask_tag") == 0)
				ipd_config->port_config.tag_mode = CVMX_PIP_TAG_MODE_IP_OR_MASK;
			else if(SC(TK[9], "tuple_xor_mask_tag") == 0)
				ipd_config->port_config.tag_mode = CVMX_PIP_TAG_MODE_TUPLE_XOR_MASK;
			else {
				cvmx_dprintf("ERROR: Invalid tag_mode %s in ipd config\n",TK[9]);
				return -1;
			}
		}
		else {
			cvmx_dprintf("ERROR: Invalid IPD config %s\n",TK[7] );
			return -1;
		}
	}
	else if((SC(TK[7], "include_in_tag_tuple") == 0) && list->token_index >= 8 ) {
		token_index = 8;
		while (token_index < list->token_index) {
			//cvmx_dprintf("\ntag tuple include %s",list->tokens[token_index]);
			if(SC(TK[token_index], "ipv6_src_ip") == 0)
				ipd_config->port_config.tag_fields.ipv6_src_ip = 1;
			else if(SC(TK[token_index], "ipv6_dst_ip") == 0)
				ipd_config->port_config.tag_fields.ipv6_dst_ip = 1;
			else if(SC(TK[token_index], "ipv6_src_port") == 0)
				ipd_config->port_config.tag_fields.ipv6_src_port = 1;
			else if(SC(TK[token_index], "ipv6_dst_port") == 0)
				ipd_config->port_config.tag_fields.ipv6_dst_port = 1;
			else if(SC(TK[token_index], "ipv6_next_header") == 0)
				ipd_config->port_config.tag_fields.ipv6_next_header = 1;
			else if(SC(TK[token_index], "ipv4_src_ip") == 0)
				ipd_config->port_config.tag_fields.ipv4_src_ip = 1;
			else if(SC(TK[token_index], "ipv4_dst_ip") == 0)
				ipd_config->port_config.tag_fields.ipv4_dst_ip = 1;
			else if(SC(TK[token_index], "ipv4_src_port") == 0)
				ipd_config->port_config.tag_fields.ipv4_src_port = 1;
			else if(SC(TK[token_index], "ipv4_dst_port") == 0)
				ipd_config->port_config.tag_fields.ipv4_dst_port = 1;
			else if(SC(TK[token_index], "ipv4_protocol") == 0)
				ipd_config->port_config.tag_fields.ipv4_protocol = 1;
			else if (SC(TK[token_index], "input_port") == 0)
				ipd_config->port_config.tag_fields.input_port = 1;
			else {
				cvmx_dprintf("ERROR: Invalid tag_tuple_include config %s\n",
					     TK[token_index]);
				return -1;
			}
			token_index += 1;
		}
	}
	else {
		cvmx_dprintf("ERROR: Invalid IPD config command\n");
		return -1;
	}
	return 0;
}

int parse_llm_config_command(token_list_t *list)
{
	int num_ports;
	if(dbg_parse)
		cvmx_dprintf("parsing llm_config\n");
	if (list->token_index < 4) {
		cvmx_dprintf("ERROR: Invalid index=%d llm config command\n", list->token_index);
		return -1;
	}
	if((SC(TK[1], "number_of_ports") == 0) &&  (SC(TK[2], "=") == 0))  {
		num_ports = ST(TK[3], (char**)NULL, 10);
		cvmx_llm_config_set_num_ports(num_ports);
	}
	else {
		cvmx_dprintf("ERROR: Invalid llm config command %s\n",TK[1] );
		return -1;
	}
	return 0;
}

int parse_spi_config_command(token_list_t *list)
{
	int timeout;

	if(dbg_parse)
		cvmx_dprintf("parsing spi_config\n");
	if (list->token_index < 4) {
		cvmx_dprintf("ERROR: Invalid index=%d spi config command\n", list->token_index);
		return -1;
	}
	if((SC(TK[1], "timeout") == 0) &&  (SC(TK[2], "=") == 0))  {
		timeout = ST(TK[3], (char**)NULL, 10);
		cvmx_spi_config_set_timeout(timeout);
	}
	else {
		cvmx_dprintf("ERROR: Invalid spi config command %s\n",TK[1] );
		return -1;
	}
	return 0;
}

int parse_ilk_config_command(token_list_t *list)
{
	int interface;
	unsigned int mode;
	unsigned char channels;
	unsigned char mask;

	if(dbg_parse)
		cvmx_dprintf("parsing llm_config\n");
	if (list->token_index < 7) {
		cvmx_dprintf("ERROR: Invalid index=%d ilk config command\n", list->token_index);
		return -1;
	}
	if((SC(TK[1], "interface") == 0) &&  (SC(TK[2], "=") == 0))  {
		interface = ST(TK[3], (char**)NULL, 10);
		if(interface >= CVMX_NUM_ILK_INTF || interface < 0) {
			cvmx_dprintf("ERROR: Invalid interface=%d in ilk config command\n",
				     interface);
			return -1;
		}
		if((SC(TK[4], "enable_look_aside_mode") == 0) &&
			(SC(TK[5], "=") == 0)) {
			if(SC(TK[6],"yes") == 0)
				mode = 1;
			else
				mode = 0;
			cvmx_ilk_config_set_LA_mode(interface, mode);
		}
		else if((SC(TK[4], "enable_rx_calendar_in_look_aside_mode") == 0) &&
		       (SC(TK[5], "=") == 0)) {
			if(SC(TK[6],"yes") == 0)
				mode = 1;
			else
				mode = 0;
			cvmx_ilk_config_set_LA_mode_cal(interface, mode);
		}
		else if((SC(TK[4], "max_number_of_channels") == 0) &&
				       (SC(TK[5], "=") == 0)) {
			channels = ST(TK[6], (char**)NULL, 10);
			if(dbg_parse)
				cvmx_dprintf("ilk interface=%d channels=%d\n",(int)interface,
				     (int)channels);
			cvmx_ilk_config_set_max_channels(interface, channels);
		}
		else if((SC(TK[4], "lane_mask") == 0) &&
				       (SC(TK[5], "=") == 0)) {
			mask = ST(TK[6], (char**)NULL, 16);
			if(dbg_parse)
				cvmx_dprintf("ilk interface=%d lane_mask=0x%x\n",(int)interface,
				     (int)mask);
			cvmx_ilk_config_set_lane_mask(interface, mask);
		}
		else {
		       cvmx_dprintf("ERROR: Invalid ilk config command %s\n",TK[4] );
		       return -1;
	       }
	}
	else {
		cvmx_dprintf("ERROR: Invalid ilk config command %s\n",TK[1] );
		return -1;
	}
	return 0;
}

int parse_npi_config_command(token_list_t *list)
{
	int max_pipe;

	if(dbg_parse)
		cvmx_dprintf("parsing npi_config\n");
	if (list->token_index < 4) {
		cvmx_dprintf("ERROR: Invalid index=%d npi config command\n", list->token_index);
		return -1;
	}
	if((SC(TK[1], "max_tx_pipes") == 0) &&  (SC(TK[2], "=") == 0))  {
		if (strcmp(TK[3], "num_port_on_interface") == 0)
			max_pipe = -1;
		else
			max_pipe = ST(TK[3], (char**)NULL, 10);
		cvmx_npi_config_set_num_pipes(max_pipe);
	}
	else {
		cvmx_dprintf("ERROR: Invalid npi config command %s\n",TK[1] );
		return -1;
	}
	return 0;
}

int cvmx_parse_config()
{
	token_list_t *list;
	int start_index=0;
        int rv;
	cvmx_ipd_config_t ipd_config;
	cvmx_dma_config_t dma_config;
	cvmx_tim_config_t timer_config;
	cvmx_dfa_app_config_t dfa_config;
	cvmx_zip_config_t zip_config;
	cvmx_raid_config_t raid_config;
	cvmx_bch_app_config_t bch_config;

	if (cvmx_app_config_str == 0)
		cvmx_dprintf("ERROR: application config string is not set\n");
	if(dbg_parse)
		cvmx_dprintf("Parsing config string.\n");

	list = init_token_list(30, 128);
	cvmx_ipd_get_config(&ipd_config);
	cvmx_dma_get_cmd_que_pool_config(&dma_config.command_queue_pool);
	cvmx_pko_get_cmd_que_pool_config(&pko_fpa_cfg);
	cvmx_tim_get_fpa_pool_config(&timer_config.timer_pool);
	cvmx_dfa_get_fpa_pool_config(&dfa_config.dfa_pool);
	cvmx_zip_get_fpa_pool_config(&zip_config.zip_pool);
	cvmx_raid_get_cmd_que_pool_config(&raid_config.command_queue_pool);
	cvmx_bch_get_cmd_que_pool_config(&bch_config.command_queue_pool);

	while (1) {
		/* Parsing the config string line by line, each call to get_token_list()
		   returns the tokens on the next line */
		start_index = get_token_list(list, cvmx_app_config_str, start_index);
		//show_token_list(list);
		if (list->token_index >= 1) {
			if (SC(TK[0],"pko") == 0)  {
				rv = parse_pko_config_command(list);
				if (rv != 0) {
					cvmx_dprintf("ERROR: in parsing pko commands\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"ipd") == 0) {
				if((rv = parse_ipd_config_command(list, &ipd_config))) {
					cvmx_dprintf("ERROR: in parsing IPD command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"rgmii") == 0) {
				if((rv = parse_rgmii_config_command(list))) {
					cvmx_dprintf("ERROR: in parsing rgmii command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"dma") == 0) {
				if((rv = parse_dma_config_command(list, &dma_config ))) {
					cvmx_dprintf("ERROR: in parsing dma command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"timer") == 0) {
				if((rv = parse_timer_config_command(list, &timer_config ))) {
					cvmx_dprintf("ERROR: in parsing timer command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"dfa") == 0) {
				if((rv = parse_dfa_config_command(list, &dfa_config ))) {
					cvmx_dprintf("ERROR: in parsing dfa command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"zip") == 0) {
				if((rv = parse_zip_config_command(list, &zip_config ))) {
					cvmx_dprintf("ERROR: in parsing zip command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0], "bch") == 0) {
				if ((rv = parse_bch_config_command(list, &bch_config))) {
					cvmx_dprintf("ERROR: in parsing bch command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"raid") == 0) {
				if((rv = parse_raid_config_command(list, &raid_config ))) {
					cvmx_dprintf("ERROR: in parsing raid command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"llm") == 0) {
				if((rv = parse_llm_config_command(list))) {
					cvmx_dprintf("ERROR: in parsing llm command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"spi") == 0) {
				if((rv = parse_spi_config_command(list))) {
					cvmx_dprintf("ERROR: in parsing spi command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"ilk") == 0) {
				if((rv = parse_ilk_config_command(list))) {
					cvmx_dprintf("ERROR: in parsing ilk command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"npi") == 0) {
				if((rv = parse_npi_config_command(list))) {
					cvmx_dprintf("ERROR: in parsing npi command\n");
					show_token_list(list);
				}
			}
			else if (SC(TK[0],"pki") == 0) {
				if(OCTEON_IS_MODEL(OCTEON_CN78XX)) {
					if((rv = parse_pki_config_command(list))) {
						cvmx_dprintf("ERROR: in parsing pki command\n");
						show_token_list(list);
					}
				}
			}
		}
		if (cvmx_app_config_str[start_index] == '\0') {
			cvmx_ipd_set_config(ipd_config);
			cvmx_pko_set_cmd_que_pool_config(pko_fpa_cfg.pool_num,
					pko_fpa_cfg.buffer_size, pko_fpa_cfg.buffer_count);
			cvmx_zip_set_fpa_pool_config(zip_config.zip_pool.pool_num,
					zip_config.zip_pool.buffer_size, zip_config.zip_pool.buffer_count);
			cvmx_tim_set_fpa_pool_config(timer_config.timer_pool.pool_num,
					timer_config.timer_pool.buffer_size, timer_config.timer_pool.buffer_count);
			cvmx_dfa_set_fpa_pool_config(dfa_config.dfa_pool.pool_num,
					dfa_config.dfa_pool.buffer_size, dfa_config.dfa_pool.buffer_count);
			cvmx_dma_set_cmd_que_pool_config(dma_config.command_queue_pool.pool_num,
					dma_config.command_queue_pool.buffer_size, dma_config.command_queue_pool.buffer_count);
			cvmx_raid_set_cmd_que_pool_config(raid_config.command_queue_pool.pool_num,
					raid_config.command_queue_pool.buffer_size, raid_config.command_queue_pool.buffer_count);
			cvmx_bch_set_cmd_que_pool_config(bch_config.command_queue_pool.pool_num,
					bch_config.command_queue_pool.buffer_size,
					bch_config.command_queue_pool.buffer_count);
			break;
		}
		start_index++;
		reset_token_index(list);
	}
	return 0;
}

int cvmx_apply_config()
{
	cvmx_apply_pko_config();
	return 0;
}
