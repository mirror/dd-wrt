#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <asm/io.h>

#define PP_BASE 0x378
#define PP_BUSY 0x80
#define PP_DATA_OFF 0
#define PP_STATUS_OFF 1
#define PP_CONTROL_OFF  2

struct timeval
timediff(t2, t1)
struct timeval *t2, *t1;
{
struct timeval delta;

    delta.tv_sec = t2->tv_sec - t1->tv_sec;
    delta.tv_usec = t2->tv_usec - t1->tv_usec;
    if (delta.tv_usec<0) {
        delta.tv_sec--;
        delta.tv_usec += 1000000;
    }
    return (delta);
}

float
ftimediff(t2, t1)
struct timeval *t2, *t1;
{
struct timeval delta;
float rval;

    delta = timediff(t2, t1);
    rval = delta.tv_sec;
    if (delta.tv_usec) {
        rval += delta.tv_usec * 1e-6;
    }
    return (rval);
}

int 
delay(int usecs)
{
struct timeval now, stop, start;
float diff;
static first;

    gettimeofday(&stop, 0);
    start = stop;

    stop.tv_usec += usecs;
    while (stop.tv_usec > 1000000) {
        stop.tv_usec -= 1000000;
        stop.tv_sec += 1;
    }
    do {
        gettimeofday(&now, 0);
        diff = ftimediff(&stop, &now);
    } while (diff>0.0);
}

void
initpp()
{
    if (ioperm(PP_BASE, 4, 1)) {
        fprintf(stderr, "can't open parport\n");
        exit(1);
    }
}

run_pulses(int pnext[], int plen[], int repeatflag)
{
int pn;

    initpp();
    outb(0, PP_BASE);

    do {
        for(pn=0; pnext[pn]; pn++) {
            outb(1, PP_BASE);
            delay(plen[pn]);
            outb(0, PP_BASE);
            delay(pnext[pn]);
        }
    } while (repeatflag);
}

run_burst(int plen, int pps, int npulses)
{
int s = 1000000;
int period;
int pnext;

    if (pps<=0) {
	pps = 1000000;
    }
    period = s/pps;
    pnext = period - plen;

    while (npulses--) {
	outb(1, PP_BASE);
	delay(pnext);
    }
}

#ifdef BURST
main(int argc, char **argv)
{
int plen, npulses, pps;

    if (strcmp(argv[0], "burst")==0) {
	if (argc<4) {
	    fprintf(stderr, "needs 3 args (plen, npulses, pps)\n");
	    exit(1);
	}
	if (ioperm(PP_BASE, 4, 1)) {
	    fprintf(stderr, "can't open parport (you must be root)\n");
	    exit(1);
	}
	plen = atoi(argv[1]);
	pps = atoi(argv[2]);
	npulses = atoi(argv[3]);
	run_burst(plen, pps, npulses);
    }

}
#endif
#ifdef TEST_PP
main(int argc, char **argv)
{

    if (ioperm(PP_BASE, 4, 1)) {
        fprintf(stderr, "can't open parport\n");
        exit(1);
    }

    while (1) {
        outb (1, PP_BASE);
        delay(125);
        outb (0, PP_BASE);
        delay(300);
    }

}
#endif
