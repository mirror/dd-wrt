#include "defs.h"

FILE *debug_handle;

struct olsrd_config *olsr_cnf;

olsr_u16_t system_tick_divider;

int exit_value;


/* Timer data */
clock_t now_times;              /* current idea of times(2) reported uptime */
struct timeval now;		/* current idea of time */
struct tm *nowtm;		/* current idea of time (in tm) */

olsr_bool disp_pack_in;         /* display incoming packet content? */
olsr_bool disp_pack_out;        /* display outgoing packet content? */

olsr_bool del_gws;

float will_int;
float max_jitter;

size_t ipsize;

union olsr_ip_addr main_addr;

int olsr_udp_port;

int ioctl_s;

#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
int rts;
#endif

float max_tc_vtime;

clock_t fwdtimer[MAX_IFS];	/* forwarding timer */

int minsize;

olsr_bool changes;                /* is set if changes occur in MPRS set */ 

/* TC empty message sending */
clock_t send_empty_tc;
