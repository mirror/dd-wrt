
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <linux/ip.h>


int sd;				    // socket
int msgsize;                        // tx msg size
int msgno =0;			    // tx msg number
int msgno_ul= -1;                      // msg number of uplink frame
int rmiss;
unsigned int txmsg[1024];	    // tx msg buffer
struct sockaddr dst, rcv;           // sock declarations
struct sockaddr_in target;
struct sockaddr_in from;
struct sockaddr_in local;
int fromlen;
char *dstnames[32];
int ndest;
char *dstname;                      // dst system name or ip
char *period;                       // codec period (if specified)
char *count;			    // output frame count
int nsent;
int nend;
double buzzinterval;
double buzzrate;
int buzzcount;
int buzzperiod;

void buzz();

#define PORT    12345               // port for sending/receiving
int port        = PORT;

#define EQ(a,b)     (strcmp(a,b)==0)

struct itimerval waitval_ap;        // codec and ap wait intervals
struct itimerval waitval_codec;
struct itimerval waitval_cancel;
struct timeval time_ul;
struct timeval time_ap;
struct timeval time_delta;
struct timeval time_start, time_stop;

int codec_sec = 0;                      // default values for codec period
int codec_usec = 10000;                 // 10 ms
int rcv_sleep_time = 10;


typedef enum {
    WAIT_NEXT_CODEC,
    WAIT_FOR_AP_RESPONSE,
} WAIT_MODE;

int wstate;                             // what event are we currently timing

char phoneflag;                         // operate as the phone
char apflag;                            // operate as ap, echo every frame back to phone
char upsdflag;                          // same as phone mode
char upsdstartflag;
unsigned char upsdval = 0xb8;
char pgenflag;                          // operate as packet generator only
char pgetflag;				// operate as packet reader only
char histflag;				// keep running histogram
char wmeloadflag;                       // generate packets for all 4 ACCs
char floodflag;				// flood the network
char qdataflag;                         // generate QOS_DATA frames at AP
char qdisflag;
char tspecflag;				// generate a tspec frame
char iphdrflag;                         // print ip header
unsigned char qdenable = 0xbc;
unsigned char qdisable = 0xad;
unsigned char tsenable = 0xbd;
char broadcastflag;			// generate broadcast packets

/*
 * wme
 */
//unsigned char dscpval[] ={ 0x28, 0x18, 0x08, 0x30 };
//unsigned char dscpval[] ={ 0x28, 0x18, 0x08, 0x30, 0x30, 0x30, 0x30 };
unsigned char dscpval[] ={ 0x18, 0x18, 0x18, 0x18, 0x08, 0x08, 0x08, 0x08,  0x30, 0x30, 0x30, 0x30, 0x28, 0x28, 0x28, 0x28 };
unsigned int dscpx[sizeof(dscpval)];

__inline__ 
int dscp_to_acc(int dscpval) {
int id;

	switch(dscpval) {
	    case 0x08:
		id = 0; break;
	    case 0x18:
		id = 1; break;
	    case 0x28:
		id = 2; break;
	    case 0x30:
		id = 3; break;
	    case 0:
	    default:
		id = 4;
	}
        return(id);
}

/*
 * timediff
 * return diff between two timevals
 */
struct timeval
timediff(t2, t1)
struct timeval *t2, *t1;
{
struct timeval del;

    del.tv_sec = t2->tv_sec - t1->tv_sec;
    del.tv_usec = t2->tv_usec - t1->tv_usec;
    if (del.tv_usec<0) {
	del.tv_sec--;
	del.tv_usec += 1000000;
    }
    return(del);
}

float
ftimediff(t2, t1)
struct timeval *t2, *t1;
{
struct timeval time_delta;
float time;

	time_delta = timediff(t2, t1);
	time = time_delta.tv_sec;
	if (time_delta.tv_usec) {
		time += time_delta.tv_usec * 1e-6;
	}
	return(time);
}


#define NSAMPLES    1000
#define	NBUCKETS    10
struct histogram {
    int buckets[NBUCKETS+1];
    float avg, sum;
    float min, max;
    int n;
    int nsamples;
};


float h_limits[NBUCKETS] = {10., 20., 30., 40., 50., 100., 500., 1000., 2000., 5000.,};

void
dohist(struct histogram *h, float val)
{
static nout =0;
int i;

    if (h->n==h->nsamples) {
	h->avg = h->sum/h->n;
	h->n = 0;
	h->sum = 0.0;
	printf("avg %f\tmin(%f)\tmax(%f)\n", h->avg, h->min, h->max);
	if (nout==0) {
	    for (i=0; i<=NBUCKETS; i++) {
		printf("%6.0f ", h_limits[i]);
	    }
	    printf("\n");
	}
	for (i=0; i<=NBUCKETS; i++) {
	    printf("%5d ", h->buckets[i]);
	    h->buckets[i] = 0;
	}
	printf("\n");
	h->max = 0.0;
	h->min = 10000000.0;
	nout++;
    } else {
	h->sum += val;
	h->n++;
	if (nout==10)
		 nout = 0;
    }
    if ((val < h->min) && val>0.0)
	     h->min = val;
    if (val > h->max)
	     h->max = val;
    if (val>0.0) {
	for(i=0; i<NBUCKETS; i++) {
	    if (val < h_limits[i]) {
		h->buckets[i]++;
		goto hdone;
	    }
	}
	h->buckets[NBUCKETS]++;
    }
hdone:
	return;


}

void
send_uplink()
{
int r, n;
unsigned int dscp, ldscp;
static int totalsent;

    msgno_ul = txmsg[0] = msgno++;
    txmsg[1] = 0;
    txmsg[2] = 0;
    txmsg[3] = 0;
    txmsg[4] = 0;


    if (wmeloadflag || upsdflag) {
        n = msgno % sizeof(dscpval);
        if (qdataflag) {
            dscp = qdenable;
            qdataflag = 0;
        } else
	if (qdisflag) {
	    dscp = qdisable;
	    qdisflag = 0;
	} else
	if (tspecflag) {
	    dscp = tsenable;
	    //tspecflag = 0;
	} else
	if (upsdstartflag) {
	    dscp = upsdval;
	    upsdstartflag = 0;
	} else
	if (upsdflag) {
	   dscp = upsdval;
	} else {
       	    dscp = dscpval[n];
	}
	if (waitval_codec.it_value.tv_sec && floodflag==0) {
        	printf("msgno %d n %d dscp(%x)\n", msgno, n, dscp);
	}
	if (dscp != ldscp)
        if ((r=setsockopt(sd, SOL_IP, IP_TOS, &dscp, sizeof(dscp)))<0) {
	    perror("can't set tos field");
	    exit(-1);
        }
	ldscp = dscp;
        txmsg[1] = dscp;
        txmsg[2] = dscpx[dscp_to_acc(dscp)]++;
    }

    /* record start time */
    if (time_start.tv_sec==0) {
	gettimeofday(&time_start, 0);
	nsent = 0;
    }
    /* initialize nend if sending <count> frames */
    if (count && nend==0) {
	    nend = atoi(count);
    }

resend:
    r = sendto(sd, txmsg, msgsize, MSG_DONTWAIT, &dst, sizeof(dst));
    gettimeofday(&time_ul, 0);

    /* wait for 1 ms and resend when we're saturating output queues */
    if (r<0) {
	buzz(100);
	goto resend;
    }
    nsent++;

    /* print each msg only if slow mode */
    if (waitval_codec.it_value.tv_sec && floodflag==0) {
	printf("s%d M(%d)\n", msgsize, msgno);
	//pb("sendto", &dst, sizeof(dst));
	//mptimeval("ul", &time_ul);
    }

    if (count && nsent>=nend) {
	float rate, time;

	gettimeofday(&time_stop, 0);
	time = ftimediff(&time_stop, &time_start);
	if (time) {
		rate = nsent/time;
	} else {
		rate = 0.0;
	}
	printf("sent %d\t", nsent);
	printf("elapsed time %f\t", time);
	printf("rate %.0f/s\n", rate);
	exit(0);
    }
}


/*
 * use a buzz loop when interpacket time is smaller
 * than resolution provided by real-time interval timer.
 */
void
buzz(int delay)
{
struct timeval now, stop;
float diff;
#ifdef TESTBUZZ
struct timeval start;
static first;
#endif

	gettimeofday(&stop, 0);
#ifdef TESTBUZZ
	start = stop;
#endif
	stop.tv_usec += delay;
	if (stop.tv_usec > 1000000) {
		stop.tv_usec -= 1000000;
		stop.tv_sec += 1;
	}
	do {
		gettimeofday(&now, 0);
		diff = ftimediff(&stop, &now);
	} while (diff>0.0);
#ifdef TESTBUZZ
if (first==0) {
	mptimeval("start\t", &start);
	mptimeval("stop \t", &now);
	printf("diff %f\n", ftimediff(&now, &start));
}
	first = 1;
#endif
}

void
timeout()
{

    switch(wstate) {
    case WAIT_NEXT_CODEC:
	send_uplink();
	//wstate = WAIT_FOR_AP_RESPONSE;
	//setitimer(ITIMER_REAL, &waitval_ap, NULL);
	wstate = WAIT_NEXT_CODEC;
	setitimer(ITIMER_REAL, &waitval_codec, NULL);
	return;
    case WAIT_FOR_AP_RESPONSE:
	wstate = WAIT_NEXT_CODEC;
	gettimeofday(&time_ap, 0);
	time_delta = timediff(&time_ap, &time_ul);
	mptimeval("timeout ", &time_delta);
	setitimer(ITIMER_REAL, &waitval_codec, NULL);
	return;
    default:
	return;
    }
}

ptimeval(struct timeval *tv)                        // print routines for timevals
{
    printf("%d:%d\n", tv->tv_sec, tv->tv_usec);
}
mptimeval(char *s, struct timeval *tv)
{
    printf("%s\t ", s);
    ptimeval(tv);
}
pitimes(struct itimerval *itv)
{
    ptimeval(&itv->it_interval);
    ptimeval(&itv->it_value);
}
mpitimes(char *s, struct itimerval *itv)
{
    printf("%s\n", s); pitimes(itv);
}

setup_timers()
{
int r;

    waitval_codec.it_value.tv_sec = codec_sec;		// sec codec sample period
    waitval_codec.it_value.tv_usec = codec_usec;
    waitval_ap.it_value.tv_sec = 1;			// set AP timeout value
    waitval_ap.it_value.tv_usec = 0;

    signal(SIGALRM, &timeout);                          // enable alarm signal
}

setup_period(char *period)
{
    int n;

    if (!period)
	return;
    n = atoi(period);
    printf("period %d usec\n", n);
    codec_sec = 0;
    codec_usec = n;
    if (n<10000) {
	buzzperiod = n;
    }
}


paddr(unsigned char *s)
{
    printf("%d.%d.%d.%d\n", s[0], s[1], s[2], s[3]);
}
pbaddr(s, in)
char *s;
unsigned long int in;
{
unsigned char a, b, c, d;

	a = in&0xff;
	b = (in>>8)&0xff;
	c = (in>>16)&0xff;
	d = (in>>24)&0xff;
	printf("%s %d.%d.%d.%d\n", s, a, b, c, d);
}

pb(char *s, unsigned char *b, int cc)
{
    printf("%s: ", s);
    while (cc--) {
	printf("%x|%d ", *b, *b);
        b++;
    }
    printf("\n");
}

is_ipdotformat(char *s)
{
int d;

    for(d=0; *s; s++) {
	if (*s=='.')
	    d++;
    }
    return(d==3);
}


int
strvec_sep(s, array, n, sep)
char *s, *array[], *sep;
int n;
{
char *p;
static char buf[2048];
int i;

        strncpy(buf, s, sizeof(buf));

        p = strtok(buf, sep);

	for (i=0; p && i<n; ) {
                array[i++] = p;
                if (i==n) {
                        i--;
                        break;
                }
                p = strtok(0, sep);
        }
        array[i] = 0;
        return(i);
}



/*
 * dst address
 * may be host name or w.x.y.z format ip address
 */
int
setup_addr(char *name, struct sockaddr *dst)
{
struct hostent *h;
char *array[5];
char c;
char *s;
int d, r;
unsigned long in =0;
unsigned char b;


    if (broadcastflag) {
	name = "192.168.171.255";
	printf("dst %s\n", name);
    }

    if ((apflag||pgetflag) || (name==NULL)) {
        goto rcv_setup;
    }

    if (is_ipdotformat(name)) {                 // check for dot format addr
        strvec_sep(name, array, 5, ".");
        for(d=0; d<4; d++) {
            b = atoi(array[d]);
            in |= (b<<(d*8));
        }
        target.sin_addr.s_addr = in;
    } else {
	h = gethostbyname(name);                // try name lookup
	if (h) {
	    memcpy((caddr_t)&target.sin_addr.s_addr, h->h_addr, h->h_length);
	    //paddr(h->h_addr_list[0]);
	} else {
            fprintf(stderr, "name lookup failed for (%s)\n", name);
            exit(-1);
	}
    }
    pbaddr("dst:", target.sin_addr.s_addr);
    target.sin_family = AF_INET;
    target.sin_port = port;
    memcpy((caddr_t)dst, (caddr_t)&target, sizeof(target));

    if (broadcastflag) {
	int one = 1;
	r = setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
	printf("multicast mode r %d\n", r);
    }

rcv_setup:
    from.sin_family = AF_INET;
    from.sin_port = port;
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    r = bind(sd, (struct sockaddr *)&from, sizeof(from));
    if (r<0) {
        perror("bind call failed");
        exit(-1);
    }
    return(r);
}

    

void
setup_socket(char *name)
{
int dscp = 0;
int stype = SOCK_DGRAM;
int sproto = 0;
int r;


    if (phoneflag) {
	dscp = 0x30;
    }
    if (upsdflag || apflag) {
        dscp = 0xb8;
    }

    if (iphdrflag) {
        stype = SOCK_RAW;
        sproto = IPPROTO_UDP;
    }

    if ((sd=socket(AF_INET, stype, sproto)) < 0) {
	perror("socket");
	exit(-1);
    }
    if ((r=setsockopt(sd, SOL_IP, IP_TOS, &dscp, sizeof(dscp)))<0) {
	perror("can't set tos field");
	exit(-1);
    }
    if ((r=setup_addr(name, &dst))<0) {
        fprintf(stderr, "can't map address (%s)\n", name);
        exit(-1);
    }
}


main(int argc, char **argv)
{
int r, i, n, t;
char buf[8];
unsigned long rmsg[2048];
int flags =0;
long rmsgno;
long *pm;

    /*
     * set mode from program name (argv[0]).
     */
    msgsize = 1024;                             // default msg size
    if (strcmp(argv[0], "ap")==0) {
        printf("ap mode\n");
        apflag = 1;
    } else
    if (strcmp(argv[0], "phone") == 0) {
        printf("phone mode\n");
        phoneflag = 1;
        msgsize = 200;
	//codec_sec = 1;
	//codec_usec = 0;
    } else
    if (strcmp(argv[0], "upsd") == 0) {
        printf("upsd phone\n");
        upsdflag = 1;
	upsdstartflag = 1;
        msgsize = 200;
    } else
    if (strcmp(argv[0], "pgen") == 0) {
        printf("pgen mode port(%d)\n", port);
        pgenflag = 1;
        codec_sec = 2;
        codec_usec = 0;
        port++;
        msgsize = 1024;
    } else
    if (strcmp(argv[0], "pget") == 0) {
	printf("pget mode on port(%d)\n", port);
	pgetflag = 1;
	port++;
    }


    /*
     * determine destination
     */
    if (argc==1 && !apflag && !pgetflag) {
        fprintf(stderr, "need at least one arg (dst address)\n");
        exit(-1);
    }


    /*
     * parse options
     */
    for(i=1; i<argc; i++) {
        if (argv[i][0] != '-') {
            if (EQ(argv[i], "?") || EQ(argv[i], "help")) {
                goto helpme;
            }
            dstname = argv[i];
            dstnames[ndest++] = dstname;
            continue;
        }
        if (EQ(argv[i], "-codec") || EQ(argv[i], "-c") || EQ(argv[i], "-period")) {
            i++;
            period = argv[i];
            continue;
        }
	if (EQ(argv[i], "-flood")) {
	    floodflag = 1;
	    continue;
	}
	if (EQ(argv[i], "-count") || EQ(argv[i], "count")) {
	    i++;
	    count = argv[i];
	    continue;
	}
        if (EQ(argv[i], "-size") || EQ(argv[i], "size")) {
            i++;
            msgsize = atoi(argv[i]);
            continue;
        }
	if (EQ(argv[i], "-hist")) {
	    histflag = 1;
	    continue;
	}
        if (EQ(argv[i], "-wmeload")) {
            wmeloadflag = 1;
            continue;
        }
	if (EQ(argv[i], "-broadcast")) {
	    broadcastflag = 1;
	    continue;
	}
        if (EQ(argv[i], "-qdata")) {
            qdataflag = 1;
            continue;
        }
	if (EQ(argv[i], "-qdis")) {
	    qdisflag = 1;
	    continue;
	}
	if (EQ(argv[i], "-tspec")) {
	    tspecflag = 1;
	    continue;
	}
	if (EQ(argv[i], "-upsd")) {
	    upsdflag = 1;
	    continue;
	}
        if (EQ(argv[i], "-ip")) {
            iphdrflag = 1;
            continue;
        }
        if (EQ(argv[i], "-h")) {
        helpme:
            printf("-codec\n-period\n-hist\n-wmeload\n-broadcast\n-count\n-qdata\n-qdis\n-tspec\n");
            exit(0);
        }

    }


    for (i=0; i<ndest; i++) {
        printf("dst[%d]=%s\n", i, dstnames[i]);
    }
    setup_socket(dstname);

    pm = (long *)rmsg;

    /*
     * phone mode
     */
    if (phoneflag||upsdflag) {
	struct histogram h;

        setup_period(period);
        setup_timers();

	if (histflag) {
	    h.nsamples = 1000;
	}
	wstate = WAIT_NEXT_CODEC;

	timeout();                  // kick off first uplink packet 

        while (1) {

	    r = recvfrom(sd, rmsg, sizeof(rmsg), flags, &rcv, &fromlen);

	    rmsgno = rmsg[0];
            if ((rmsgno + 1) != msgno_ul) {
		rmiss++;
                continue;
	    }

	    gettimeofday(&time_ap, 0);
	    time_delta = timediff(&time_ap, &time_ul);

	    if (histflag) {
		//dohist(&h, time_delta.tv_usec);
		dohist(&h, ftimediff(&time_ap, &time_ul) );
	    } else {
		printf("r%d msg(%d) ", r, rmsgno);
		mptimeval("delta ", &time_delta);
	    }

	    //setitimer(ITIMER_REAL, &waitval_cancel, NULL);
	    //wstate = WAIT_NEXT_CODEC;
	    //setitimer(ITIMER_REAL, &waitval_codec, NULL);
	    sleep(rcv_sleep_time);
	}
    }

    /*
     * ap mode
     * echo anything received back to the sender.
     * don't echo to local addresses
     */
    while (apflag) {
        struct iphdr *iph;

        r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
        if (r<0) {
            perror("rcv error:");
        }
        if (iphdrflag) {
            iph = (struct iphdr *)rmsg;
	    rmsgno = *pm;
            printf("R%d %d dscp(%x)\n", rmsgno, r, iph->tos);
        }
        //pb("from", (caddr_t)&from.sin_addr.s_addr, 4);
        //pb("sock:", (caddr_t)&from, fromlen);
        //printf("port %d\n", from.sin_addr.s_port);
        if (from.sin_addr.s_addr==0 || from.sin_addr.s_addr==local.sin_addr.s_addr) {
            continue;
        }
        r = sendto(sd, rmsg, r, MSG_DONTWAIT, (struct sockaddr *)&from, sizeof(from));
        //printf("snd %d\n", r);
    }


    /*
     * pgen mode
     */
    if (pgenflag) {

        printf("pgen msgsize(%d) period(%s)\n", msgsize, period);
        wstate = WAIT_NEXT_CODEC;
        setup_period(period);
	while (floodflag || buzzperiod) {
		send_uplink();
		buzz(buzzperiod);
	}
        setup_timers();
        timeout();
        while (1) {
            sleep(10);
        }
    }

    /*
     * pget mode
     */
    if (pgetflag) printf("starting pget sd(%d)\n", sd);

    n = 0;
    t = 0;
    gettimeofday(&time_start, 0);

#define	NCOUNTS 5

    while (pgetflag) {
        struct iphdr *iph;
	float elapsed;
	int counts[NCOUNTS];
	int id;

	fromlen = sizeof(from);
	r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
	gettimeofday(&time_stop, 0);
	

        id = dscp_to_acc(rmsg[1]);

	if (histflag==0) {
	    printf("r %d\t%x\t%d\t%d\n", rmsg[0], rmsg[1], id, rmsg[2]);
            if (iphdrflag) {
                iph = (struct iphdr *)rmsg;
                printf("tos %x\n", iph->tos);
                mpx("", rmsg, 32);
            }
	    continue;
	}

	n++;

	elapsed = ftimediff(&time_stop, &time_start);

	counts[id]++;

	if (elapsed >= 2.0) {
	    time_start = time_stop;

	    for(i=0; i<NCOUNTS; i++) {
		     printf("%d\t", counts[i]);
		     counts[i] = 0;
	    }
	    printf("[%d]\n", n);
	    n = 0;
	}
    }
	
}
