#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include "radar.h"

#define EQ(a,b)     (strcmp(a,b)==0)


/* Bin-5 Test Parameters */
#define MINBURST    8
#define	MAXBURST    20
#define	MINPULSES   1
#define MAXPULSES   3
#define	MINLEN	    50
#define	MAXLEN	    100
#define BIN5_RUNLEN    12*1000000

/* selected parameter values */
int nburst;			    // number of bursts in test
int runlen;                         // time span (in microseconds) for a test run
int burstwindow;		    // time window for burst[i]
int nbpulses[MAXBURST];		    // number of pulses in burst[i]
int bpulselen[MAXBURST][MAXPULSES];  // duration for the pulses in each burst
int spacing[MAXBURST][MAXPULSES];   // pulse spacing
int burstlen[MAXBURST];		    // total time for burst[i]
int randomstart[MAXBURST];	    // start time for each burst[i]
int pstart[MAXBURST*MAXPULSES+1];   // pulse start times
int plen[MAXBURST*MAXPULSES+1];     // pulse lens
int pnext[MAXBURST*MAXPULSES+1];    // time to next pulse
int npulses;                        // total number of pulses


main(int argc, char **argv)
{
    long int r, i, pn, bn, b, np;
    int l, s;
    int rflag = 0;

    if (EQ("rand", argv[0])) {          // test random numbers and exit
        set_random_seed(0);
        history();
        exit(1);
    }

    for(i=1; i<argc; i++) {
        if (EQ(argv[i], "-t"))
            rflag = 1;
    }
            

    set_random_seed(rflag);

    // pick number of bursts for test
    runlen = BIN5_RUNLEN;
    nburst = pickanumber(MINBURST, MAXBURST);
    burstwindow = runlen/nburst;
    npulses = 0;

    // pick the random test parameters for each burst 
    for (bn=0; bn<nburst; bn++) {

	np = nbpulses[bn] = pickanumber(MINPULSES, MAXPULSES);
        npulses += np;
        for(i=0; i<np; i++) {
	    l = bpulselen[bn][i] = pickanumber(MINLEN, MAXLEN);
            s = (np>1 && i<np-1) ? pickanumber(1000,2000) : 0;
            spacing[bn][i] = s;
            burstlen[bn] += l + s;
        }

        randomstart[bn] = pickanumber(0, burstwindow - burstlen[bn]);
    }

    // convert to flat pulse schedule
    for (pn=bn=0; bn<nburst; bn++) {
        int burststart;

        np = nbpulses[bn];
        burststart = burstwindow*bn + randomstart[bn];
        for (s=i=0; i<np; i++) {
            plen[pn] = bpulselen[bn][i];
            pstart[pn] = burststart +s;
            s += plen[pn] + spacing[bn][i];
            pn++;
        }
    }
    for(pn=0; pn<npulses-1; pn++) {
        pnext[pn] = pstart[pn+1] - (pstart[pn] + plen[pn]);
    }
    pnext[pn] = 0;
    //printf("npulses %d pn %d\n", npulses, pn+1);

    if (EQ(argv[0], "gen")) {
        run_pulses(pnext, plen, 0);
    }
    if (EQ(argv[0], "sim")) {
        print_test();
    }
}

print_test()
{
int i, j, k, time;
int bn, pn;

    printf("n bursts %d burstwindow %d check %d\n", nburst, burstwindow, nburst*burstwindow);
    for (bn=0; bn<nburst; bn++) {
        printf("nbpulses %d ", nbpulses[bn]);
        printf("start %d ", randomstart[bn]);
        printf("len ");
        for(k=0; k<nbpulses[bn]; k++) {
            printf("%d:%d ", bpulselen[bn][k], spacing[bn][k]);
        }
        printf("(%d) %d\n", burstlen[bn], randomstart[bn]+bn*burstwindow);
    }

    printf("pulse schedule\n");
    for (pn=0; pn<npulses; pn++) {
        printf("%d %d %d\n", pstart[pn], plen[pn], pnext[pn]);
    }
}
