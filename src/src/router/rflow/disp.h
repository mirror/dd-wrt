#ifndef	__IPCAD_DISPLAY_H__
#define	__IPCAD_DISPLAY_H__

void print_ip(FILE *f, struct in_addr ip);
void print_aligned_ip(FILE *f, struct in_addr ip);


/*
 * Display the storage contents.
 */
typedef enum {
	DS_ACTIVE,
	DS_CHECKPOINT,
	DS_NETFLOW,
} disp_storage_e;
int display(FILE *, disp_storage_e);

void rflow_show_stats(FILE *f);
int display_internal_averages(FILE *f, const char *ifname);
void show_version(FILE *f);
void ipcad_uptime(FILE *f);
void display_uptime(FILE *f, time_t uptime);



/*
 * Dump the table or cache contents to the provided file pointer.
 */
struct flow_s;
void dump_flow_table(FILE *f, struct flow_s *flow, int entries, disp_storage_e);


#endif	/* __IPCAD_DISPLAY_H__ */
