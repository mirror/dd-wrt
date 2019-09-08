
#include "../lib/third_party/include/ndpi_patricia.h"

struct ndpi_port_range {
	uint16_t	start, end, // port range
			proto,	    // ndpi proto
			l4_proto;   // 0 - udp, 1 - tcp
};
typedef struct ndpi_port_range ndpi_port_range_t;

struct ndpi_port_def {
	int		  count[2]; // counter udp and tcp ranges
	ndpi_port_range_t p[0];     // udp and tcp ranges
};

uint16_t ndpi_check_ipport(patricia_node_t *node,uint16_t port,int l4);
int ndpi_print_port_range(ndpi_port_range_t *pt,
		int count,char *buf,size_t bufsize,
                ndpi_mod_str_t *ndpi_str);
int parse_n_proto(char *pr,ndpi_port_range_t *np,ndpi_mod_str_t *ndpi_str);
int parse_l4_proto(char *pr,ndpi_port_range_t *np);
int parse_port_range(char *pr,ndpi_port_range_t *np);
int parse_ndpi_ipdef_cmd(struct ndpi_net *n, int f_op, prefix_t *prefix, char *arg);

int parse_ndpi_ipdef(struct ndpi_net *n,char *cmd);
int parse_ndpi_hostdef(struct ndpi_net *n,char *cmd);
int parse_ndpi_proto(struct ndpi_net *n,char *cmd);
