#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <features.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include <asm/semaphore.h>
#include <sys/ioctl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <ejtag.h>

#include <signal.h>
#include <fcntl.h>
#include <assert.h>

#include "ejtag_primitives.h"
#include "remote-mips-ejtag.h"
#include "tm.h"
#include <strings.h>

#define VERBOSE 0
static int verbose = VERBOSE;

unsigned long pp_base = PP_BASE;

extern int jtagbrk;
extern unsigned int big_probe_mem[];

#define read_cycle_counter(x) \
        __asm__(".byte 0x0f,0x31" \
                        :"=a" (((unsigned long *) &x)[0]), \
                        "=d" (((unsigned long *) &x)[1]));

#define diff_cycle_counter(time_low, time_high, s, e) \
        __asm__("subl %2,%0\n\t" \
                        "sbbl %3,%1" \
                        :"=r" (time_low), "=r" (time_high) \
                        :"m" (*(0+(long *)&s)), \
                        "m" (*(1+(long *)&s)), \
                        "0" (*(0+(long *)&e)), \
                        "1" (*(1+(long *)&e)));

void usage()
{
    fprintf(stderr, "Usage:\n     profsoc [-p <parport_base_addr>] <number_samples> <usec_sample_period>\n");
}

/* for getopt usage */
#define OPTARG  "p:"
extern int optind, opterr, optopt;
extern char *optarg;

int main(int argc, char *argv[]) 
{
	unsigned long reg;
	int           delay;
	int           num;
	unsigned long long start;
	unsigned long long end;
	unsigned long time_high, time_low;
	int           c;

    while ((c = getopt(argc, argv, OPTARG)) != EOF) {
	    switch (c) {
            case 'p':  /* parport port number to override default */
				pp_base = strtoul(optarg, 0, 0);
		        break;
    		default:
    		    usage();
    			exit(0);
		}
	}

	if ( (argc - optind) != 2) {
		usage();
		exit(1);
	}

	num   = atoi(argv[optind]);
	delay = atoi(argv[optind + 1]);

	if(ioperm(pp_base, 4 /* num addr */, 1 /* perm */)){
		printf("No permission for parport!\n");
		return 1;
	}

	for ( ; (num > 0); num--) {

		mips_ejtag_jtagbrk();   
		reg = mips_ejtag_getreg(PC_REGNUM);
		mips_ejtag_release();   
		printf("0x%x\n", reg);
		fflush(stdout);
		if (delay)
			usleep(delay);
	}
}
