#ifndef	__CFGVAR_H__
#define	__CFGVAR_H__

#include "rflow.h"

/*
 * IP networks aggregation list.
 * This list implies linear scan, but should be fine for
 * small number of aggregated networks.
 */
struct atable {
	struct in_addr ip;
	struct in_addr mask;
	struct in_addr strip;
	struct atable *next;
	int strip_bits;
};

/*
 * Ports aggregation table.
 */
extern int agr_portmap[65536];

typedef struct config_s {
	char *pidfile;

	unsigned memsize;	/* Memory usage size */
	unsigned bufsize;	/* Capture buffers size */

	packet_source_t *packet_sources_head;

	struct atable *atable;

	int capture_ports;	/* Whether to display UDP/TCP ports */

	int netflow_version;
	int netflow_timeout_active;	/* Flow timeout, in seconds */
	int netflow_timeout_inactive;	/* Flow timeout, in seconds */
	int netflow_packet_interval;	/* Sampling mode, in packets */
} config_t;

extern pthread_mutex_t packet_sources_list_lock;


/* Globally visible configuration */
extern config_t *conf;

/*
 * Run-time global variables
 */
extern size_t used_memory;


/*
 * Constructors used by the configuration reader.
 */

packet_source_t *cfg_add_iface(char *, int iflags, char *filter);
int cfg_add_rsh_host(char *ru, char *rh, int privlevel);
int cfg_check_rsh(char *ru, struct in_addr *);

int cfg_add_atable(char *ip, char *mask, char *strip);
int cfg_add_aggregate_ports_table(int from, int to, int into);


/*
 * Misc methods.
 */

int make_dump(char *, FILE *);
int import_table(char *, FILE *, int clear);

#endif	/* __CFGVAR_H__ */
