#ifndef NDPI_PROC_PARSERS_H
#define NDPI_PROC_PARSERS_H

#include "../lib/third_party/include/ndpi_patricia.h"

struct ndpi_port_range {
	uint16_t	start, end, // port range
			proto;	    // ndpi proto
	uint8_t		l4_proto,   // 0 - udp, 1 - tcp
			no_dpi;     // 1 - set proto without DPI
};
typedef struct ndpi_port_range ndpi_port_range_t;

struct ndpi_port_def {
	int		  count[2]; // counter udp and tcp ranges
	ndpi_port_range_t p[0];     // udp and tcp ranges
};

NDPI_STATIC uint16_t ndpi_check_ipport(ndpi_patricia_node_t *node,uint16_t port,int l4);
NDPI_STATIC int ndpi_print_port_range(ndpi_port_range_t *pt,
		int count,char *buf,size_t bufsize,
                ndpi_mod_str_t *ndpi_str);
NDPI_STATIC int parse_n_proto(char *pr,ndpi_port_range_t *np,ndpi_mod_str_t *ndpi_str);
NDPI_STATIC int parse_l4_proto(char *pr,ndpi_port_range_t *np);
NDPI_STATIC int parse_port_range(char *pr,ndpi_port_range_t *np);
NDPI_STATIC int parse_ndpi_ipdef_cmd(struct ndpi_net *n, int f_op, ndpi_prefix_t *prefix, char *arg);

NDPI_STATIC int parse_ndpi_ipdef(struct ndpi_net *n,char *cmd);
NDPI_STATIC int parse_ndpi_hostdef(struct ndpi_net *n,char *cmd);
NDPI_STATIC int parse_ndpi_proto(struct ndpi_net *n,char *cmd);
NDPI_STATIC int parse_ndpi_debug(struct ndpi_net *n,char *cmd);
NDPI_STATIC int parse_ndpi_risk(struct ndpi_net *n,char *cmd);
NDPI_STATIC int parse_ndpi_cfg(struct ndpi_net *n,char *cmd);
#endif