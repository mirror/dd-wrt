//  
//  May-2005 
//    Greg Chesson           greg@atheros.com
//    Stephen [kiwin] PALM   wmm@kiwin.com
//////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005  Greg Chesson,  Stephen [kiwin] PALM
//
// License is granted to Wi-Fi Alliance members and designated
// contractors for exclusive use in testing of Wi-Fi equipment.
// This license is not transferable and does not extend to non-Wi-Fi
// applications.
//
// Derivative works are authorized and are limited by the
// same restrictions.
//
// Derivatives in binary form must reproduce the
// above copyright notice, the name of the authors "Greg Chesson" and
// "Stephen [kiwin] Palm",
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// The name of the authors may not be used to endorse or promote
// products derived from this software without specific prior
// written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#//////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#ifdef __CYGWIN__
#include <rpc.h>
#include <netinet/ip.h>
#else
#include <rpc/rpc.h>
#include <linux/ip.h>
#endif
#include <netdb.h>
#include <errno.h>
#include <string.h>


/*
 * Modes of operation
 */
#define	STAUT	1		// Station Under Test
#define	APTS	2		// APSD Test System
#define	AP	3		// AP proxy
#define	PHONE	4		// Phone emulator
#define	UPSD	5		// UPSD phone
#define	PGEN	6		// Generate packets
#define	PGET	7		// Get packets

int sd;				    // socket descriptor
int msgsize;                        // tx msg size
int msgno =0;			    // tx msg number
int msgno_ul= -1;                   // uplink message number
int rmiss;			    // number of missed messages
unsigned long txmsg[2048];	    // tx msg buffer
struct sockaddr dst, rcv;           // sock declarations
struct sockaddr_in target;
struct sockaddr_in from;
struct sockaddr_in local;
int sockflags;			    // socket call flags
int fromlen;			    // sizeof socket struct
char *period;                       // codec period (if specified)
char *count;			    // output frame count
int nsent=0;                        // Number sent 
int nrecv=0;                        // Number received 
int nend;                           // Number of exchanges
int ndown;                          // Number of Downlink frames per exchange
int nup;                            // Number of Uplink frames per exchange

double buzzinterval;
double buzzrate;
int buzzcount;
int buzzperiod;
void buzz();

#define	NTARG	32		    // number of target names
int ntarg;
char *targetnames[NTARG];
char *targetname;                   // dst system name or ip address

/*
 * APTS messages/tests
 */
#define	APTS_DEFAULT	1		// message codes
#define	APTS_HELLO	2
#define	APTS_HELLO_RESP	3
#define	APTS_CONFIRM	4
#define	APTS_STOP	5
#define	APTS_TESTS	10		// test codes begin after APTS_TESTS
#define B_D		11
#define B_2		12
#define	B_H		13
#define	B_4		14
#define	B_5		15
#define	B_6		16
#define B_B             17
#define B_E             18
#define B_G             19
#define B_I             20
#define	APTS_LAST	21		// reminder: update APTS_LAST when adding tests

/*
 * internal table
 */
struct apts_msg {			// 
	char *name;			// name of test
	int cmd;			// msg num
	int param0;			// number of packet exchanges
	int param1;			// number of uplink frames 
	int param2;			// number of downlink frames
	int param3;
};


struct apts_msg apts_msgs[] ={
	{0, -1, 0, 0, 0, },
	{"APTS TX         ", APTS_DEFAULT, 0, 0, 0, },
	{"APTS Hello      ", APTS_HELLO, 0, 0, 0, 0},
	{"APTS Hello Resp ", APTS_HELLO_RESP, 0, 0, 0, 0},
	{"APTS Confirm    ", APTS_CONFIRM, 0, 0, 0, 0},
	{"APTS STOP       ", APTS_STOP, 0, 0, 0, 0},
	{0, 6, 0, 0, 0, },
	{0, 7, 0, 0, 0, },
	{0, 8, 0, 0, 0, },
	{0, 9, 0, 0, 0, },
	{0, 10, 0, 0, 0, },		// APTS_TESTS
	{"B.D", B_D, 4, 1, 1, 0},	// 4 single packet exchanges
	{"B.2", B_2, -1, 1, 1, 0},	// continuous single packet exchanges
	{"B.H", B_H, 4, 1, 2, 0},	// 4 exchanges: 1 uplink, 2 downlink frames
	{"B.4", B_4, 4, 2, 1, 0},	// 4 exchanges: 2 uplink (trigger 2nd), 1 downlink frames
	{ 0,    B_5, 4, 2, 1, 0},	// placeholder
	{ 0,    B_6, 4, 2, 1, 0},	// placeholder
	{"B.B", B_B, 4, 1, 0, 0},	// 4 exchanges: 1 uplink, 0 downlink
	{"B.E", B_E, 4, 2, 0, 0},	// 4 exchanges: 2 uplink, 0 downlink
	{"B.G", B_G, 4, 2, 1, 0},	// 4 exchanges: 2 uplink, 1 downlink
	{"B.I", B_I, 4, 2, 2, 0},	// 4 exchanges: 2 uplink, 2 downlink
	{0, 0, 0, 0, 0, }		// APTS_LAST
};

#define PORT    12345               // port for sending/receiving
int port        = PORT;

#define EQ(a,b)     (strcmp(a,b)==0)

struct itimerval waitval_ap;        // codec and ap wait intervals
struct itimerval waitval_codec;
struct itimerval waitval_cancel;
struct timeval time_ul;		    // uplink timestamp
struct timeval time_ap;		    // downlink timestamp
struct timeval time_delta;	    // time difference
struct timeval time_start, time_stop;

int codec_sec = 0;                      // default values for codec period
int codec_usec = 10000;                 // 10 ms
int rcv_sleep_time = 10;


/*
 * Wait/Timer states
 */
int wstate;                             // what event are we currently timing
typedef enum {
    WAIT_NEXT_CODEC,			
    WAIT_FOR_AP_RESPONSE,
    WAIT_STUAT_00,
    WAIT_STUAT_01,
    WAIT_STUAT_02,
    WAIT_STUAT_03,
    WAIT_STUAT_04,
    WAIT_STUAT_0E,
} WAIT_MODE;

/*
 * power management
 */
#define	P_ON	1
#define	P_OFF	0

/*
 * global flags and variables
 */
unsigned char dscp, ldscp;		// new/last dscp output values
char phoneflag = 0;                     // operate as the phone
char apflag = 0;                        // operate as ap, echo every frame back to phone
char stautflag= 0;                      // operate as staut, wait for command packet
int sta_state = 0;
char aptsflag = 0;                      // operate as AP Test software (TS) mode
int apts_state = 0;
char upsdflag = 0;                      // same as phone mode
char upsdstartflag;
unsigned char upsdval = 0xb8;		// dscp value for AC_VO
char pgenflag;                          // operate as packet generator only
char pgetflag;				// operate as packet reader only
char histflag;				// keep running histogram
char wmeloadflag;                       // generate packets for all 4 ACCs
char floodflag;				// flood/saturate the network
char broadcastflag;			// generate broadcast packets
char qdataflag;                         // generate QOS_DATA frames at AP
char qdisflag;
char tspecflag;				// generate a tspec frame
char iphdrflag;                         // print ip header
char traceflag;				// enable debug packet tracing
char prcvflag = 1;			// enable rcv packet printing in do_staut
char brcmflag = 0;			// is BRCM device
char athrflag = 0;			// is ATHR device
unsigned char qdenable = 0xbc;		// special dscp values for driver debugging only
unsigned char qdisable = 0xad;
unsigned char tsenable = 0xbd;

/*
 * wme
 */
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

#define	TOS_VO	    0x30		// tos/dscp values
#define	TOS_VI	    0x28
#define	TOS_BE	    0x18
#define	TOS_LE	    0x08

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


/*
 * Histograms
 */
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
set_dscp(int dval)
{
int r;
	if (dscp == ldscp)
	    return;

        if ((r=setsockopt(sd, SOL_IP, IP_TOS, &dscp, sizeof(dscp)))<0) {
	    perror("can't set dscp/tos field");
	    exit(-1);
        }
}

void
send_txmsg(int set_dscp)
{
int r, n;
static int totalsent;

    msgno_ul = txmsg[0] = msgno++;
    txmsg[1] = 0;
    txmsg[2] = 0;
    txmsg[3] = 0;
    txmsg[4] = 0;


    if (wmeloadflag || upsdflag || set_dscp>0) {
        n = msgno % sizeof(dscpval);
        if (qdataflag) {		// send special dscp value for debugging
            dscp = qdenable;
            qdataflag = 0;
        } else
	if (qdisflag) {			// send special dscp value for debugging
	    dscp = qdisable;
	    qdisflag = 0;
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
	if (set_dscp>0) {
		dscp = set_dscp;
	}
	if (dscp != ldscp)
        if ((r=setsockopt(sd, SOL_IP, IP_TOS, &dscp, sizeof(dscp)))<0) {
	    perror("can't set dscp/tos field");
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

resend:
    r = sendto(sd, txmsg, msgsize, sockflags, &dst, sizeof(dst));
    gettimeofday(&time_ul, 0);

    /* 
     * When we're flooding output queues with back-to-back packets,
     * (a saturation/overflow test)
     * wait for 1 ms and then resend, keeping output queue backlogged.
     */
    if (r<0) {
	buzz(100);
	goto resend;
    }
    nsent++;

    /* print each msg only if slow mode */
    if (waitval_codec.it_value.tv_sec && floodflag==0) {
	printf("send_txmsg: size(%d) msgno(%d) cmd(%2d)\n", msgsize, msgno, txmsg[10]);
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
	printf("msgs sent %d\t", nsent);
	printf("elapsed time %f\t", time);
	printf("rate %.0f/s\n", rate);
	exit(0);
    }
}



void
create_apts_msg(int msg, unsigned long txbuf[])
{
struct apts_msg *t;

    t = &apts_msgs[msg];
    msgno_ul = txbuf[0] = msgno++;
    txbuf[ 1] = 0;
    txbuf[ 2] = 0;
    txbuf[ 3] = 0;
    txbuf[ 4] = 0;
    txbuf[ 5] = 0; 
    txbuf[ 6] = t->param0; 
    txbuf[ 7] = t->param1; 
    txbuf[ 8] = t->param2; 
    txbuf[ 9] = t->param3; 
    txbuf[ 10] = t->cmd; 
    strcpy((char *)&txbuf[11], t->name);
    printf("create_apts_msg (%s)\n", t->name);
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
int r, i, n, t;
unsigned long rmsg[2048];
int flags =0;

    //printf( "wstate: %d\n", wstate );
    switch(wstate) {
    case WAIT_NEXT_CODEC:
	send_txmsg(-1);
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
    case WAIT_STUAT_00:
	alarm(2);
    	r = sendto(sd, txmsg, msgsize, sockflags, &dst, sizeof(dst));
	return;
    case WAIT_STUAT_02:
	send_txmsg(-1);
	setitimer(ITIMER_REAL, &waitval_codec, NULL);
	return;
    case WAIT_STUAT_04:
	send_txmsg(TOS_BE);		// send a best effort frame
	send_txmsg(TOS_VO);		// send a VO (trigger) frame
	setitimer(ITIMER_REAL, &waitval_codec, NULL);
	return;
    case WAIT_STUAT_0E:
	send_txmsg(TOS_VO);		// send a VO (trigger) frame
	send_txmsg(TOS_VO);		// send a VO (trigger) frame
	setitimer(ITIMER_REAL, &waitval_codec, NULL);
	return;
    default:
	return;
    }
}

ptimeval(struct timeval *tv)                        // print routines for timevals
{
    printf("%2d:%06d\n", tv->tv_sec, tv->tv_usec);
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


    //printf( "setup_addr: entering\n" );
    if (broadcastflag) {
	name = "192.168.171.255";
	printf("dst %s\n", name);
    }

    if ((apflag||aptsflag||pgetflag) || (name==NULL)) {
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

    //printf( "setup_socket: entering\n");

    if (phoneflag) {
	dscp = 0x30;
    }
    if (upsdflag || apflag || aptsflag || stautflag) {
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

/*
 * Determine name of running program.
 * Set global variables and defaults accordingly.
 */
int
parse_program(char *s)
{
int n;

    msgsize = 1024;                             // default msg size
#ifdef __CYGWIN__
    sockflags = 0;				// no sockflags if compiled for cygwin
#else
    sockflags = MSG_DONTWAIT;
#endif
    for(n=strlen(s); n>0; n--) {		// strip any leading pathname components
	if (s[n] == '/') {
	    s = &s[n+1];
	    break;
	}
    }

    if (strcmp(s, "staut")==0) {		// Station Under Test
        printf("STAUT mode\n");
        msgsize = 200;
        stautflag = 1;
        codec_sec = 2;
        codec_usec = 0;
	dscp = TOS_VO;
	return(STAUT);
    } 
    if (strcmp(s, "apts")==0) {			// AP Test Station
	aptsflag = 1;
	return(APTS);
    }
    if (strcmp(s, "ap")==0) {			// ap echo mode
        printf("ap mode\n");
        apflag = 1;
	return(AP);
    }
    if (strcmp(s, "phone") == 0) {		// emulate phone
        printf("phone mode\n");
        phoneflag = 1;
        msgsize = 200;
	codec_sec = 0;
	codec_usec = 10000;
	return(PHONE);
    }
    if (strcmp(s, "upsd") == 0) {		// emulate upsd phone
        printf("upsd phone\n");
        upsdflag = 1;
	upsdstartflag = 1;
        msgsize = 200;
	return(UPSD);
    }
    if (strcmp(s, "pgen") == 0) {		// basic pgen
        printf("pgen mode port(%d)\n", port);
        pgenflag = 1;
        codec_sec = 2;
        codec_usec = 0;
        port++;
	return(PGEN);
    }
    if (strcmp(s, "pget" ) == 0) {		// basic pget
	printf("pget mode on port(%d)\n", port);
	pgetflag = 1;
	port++;
	return(PGET);
    } 

    printf("Error: Could not match executable name (%s)\n", s);
    exit(-1);

}

print_iphdr(struct iphdr *iph)
{
#ifndef __CYGWIN__
    printf("tos %x\n", iph->tos);
#endif
    mpx("", iph, 32);
}

void
do_pget()
{
unsigned long rmsg[2048];
int r, flags, n, i, id;
float elapsed;
#define	NCOUNTS 5
int counts[NCOUNTS];


    n = 0;
    gettimeofday(&time_start, 0);

    while (1) {
	fromlen = sizeof(from);
	r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
	gettimeofday(&time_stop, 0);
	

        id = dscp_to_acc(rmsg[1]);

	if (histflag==0) {
	    printf("r %d\t%x\t%d\t%d\n", rmsg[0], rmsg[1], id, rmsg[2]);
            if (iphdrflag) {
		print_iphdr((struct iphdr *)rmsg);
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
#undef NCOUNTS
}

void
do_pgen()
{
    if (pgenflag) {

        printf("PGEN: msgsize(%d) period(%s)\n", msgsize, period);
        wstate = WAIT_NEXT_CODEC;
        setup_period(period);

	while (floodflag || buzzperiod) {
		send_txmsg(-1);
		buzz(buzzperiod);
	}

        setup_timers();				// enable timers
        timeout();				// stimulate first transmission
        while (1) {				// repetitive sleep, interrupted by timeouts
            sleep(10);
        }
    }
}
void
do_ap()
{
unsigned long rmsg[2048];
int r, flags, n, i, id;

    while (1) {

	fromlen = sizeof(from);
        r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
        if (r<0) {
            perror("rcv error:");
        }
        if (iphdrflag) {
	    print_iphdr((struct iphdr *)rmsg);
        }
        //pb("from", (caddr_t)&from.sin_addr.s_addr, 4);
        //pb("sock:", (caddr_t)&from, fromlen);
        //printf("port %d\n", from.sin_addr.s_port);
        if (from.sin_addr.s_addr==0 || from.sin_addr.s_addr==local.sin_addr.s_addr) {
            continue;
        }
	r = sendto(sd, rmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
        //printf("AP: snd %d\n", r);
    }
}


struct apts_msg *
apts_lookup(char *s)
{
struct apts_msg *t;

	for (t=&apts_msgs[APTS_TESTS]; s && t->cmd; t++) {
		if (t->name && strcmp(t->name, s)==0) {
			return(t);
		}
	}
	fprintf(stderr, "APTS Test(%s) unknown\n", s);
	fprintf(stderr, "available tests are:\n");
	for (t=&apts_msgs[APTS_TESTS]; t->cmd; t++) {
		if (t->name) {
			fprintf(stderr, "\t%s\n", t->name);
		}
	}
	exit(-1);
}


/*
 * AP TS mode
 *	Do initial handshake with STAUT:
 *		recv HELLO
 *		send TEST parameters
 *		recv HELLO_CONFIRM
 *	run APTS side of the test
 */
void
do_apts()
{
unsigned long rmsg[2048];
int r, flags, n, i, id;
struct apts_msg *t;

    t = apts_lookup(targetnames[0]);
    flags = apts_state = 0;
    fromlen = sizeof(from);
    nsent = nrecv = 0;
    nend  = t->param0;
    nup   = t->param1;
    ndown = t->param2;
    printf("APTS(%s) Exchanges: %d  Up/exch: %d  Down/exch: %d \n", t->name, nend, nup, ndown);

    while (aptsflag) {
        r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
        if (r<0) {
            perror("rcv error:");
	    exit(1);
        }
        nrecv++;
        if (traceflag) printf( "Received # %d   length:%d\n", nrecv, r );
        if (from.sin_addr.s_addr==0 || from.sin_addr.s_addr==local.sin_addr.s_addr) {
            continue;
        }

	switch(apts_state) {
	case 0:							// expecting HELLO
		if (rmsg[10] == APTS_HELLO) {
		    create_apts_msg(t->cmd, txmsg);
		    r = sendto(sd, txmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
		    apts_state = 1;
		}
		continue;
	case 1:							// expecting CONFIRM
		if (rmsg[10]==APTS_CONFIRM) {
	    	    apts_state = 2;
		}
		continue;
	case 2:							// expecting data to be echoed
                if (traceflag) printf( "nsent %d %d  nrecv %d %d\n", nsent, nend*ndown, nrecv-3, nend*nup );
                if ( (nsent >= (nend*ndown)) && ((nrecv-3) >= (nend*nup)) ) {
			create_apts_msg(APTS_STOP, txmsg);
           		r = sendto(sd, txmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
			if (traceflag) mpx("APTS send stop\n", txmsg, 64);
                        exit(0);
                }
		switch(t->cmd) {
		case B_4:
                        nrecv++;
                        // B.4 receives 2 frames before loading next buffer frame
		case B_D:
		case B_2:
		case B_I:
			nsent++;
			r = sendto(sd, rmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
			if (traceflag) mpx("APTS send\n", txmsg, 64);
			continue;
		case B_G:
			if ( nrecv%2 ) {
                          nsent++;
			  r = sendto(sd, rmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
			  if (traceflag) mpx("APTS send\n", txmsg, 64);
                        }
			continue;
		case B_H:
			nsent++;
			r = sendto(sd, rmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
			if (traceflag) mpx("APTS send\n ", rmsg, 64);
			/* change the received message somewhat and send it as a second packet */
			rmsg[3] = nsent++;
			rmsg[10] = 99;
			r = sendto(sd, rmsg, r, sockflags, (struct sockaddr *)&from, sizeof(from));
			if (traceflag) mpx("APTS send\n ", rmsg, 64);
		    	continue;
		case B_B:
		case B_E:
		        continue;
		}
        }
    }
}

/*
 * STAUT mode
 */
void
do_staut()
{
unsigned long rmsg[2048];
int r, flags, n, i, id;
int sta_test;

start:
    sta_state = 0;
    setup_timers();
    fromlen = sizeof(from);
    set_pwr_mgmt("STAUT", P_OFF);

    while (stautflag) {

        while ( sta_state==0 ) { 			// do startup exchange with APTS

	    create_apts_msg(APTS_HELLO, txmsg);		// send HELLO
            wstate = WAIT_STUAT_00;
	    timeout();

            r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);
            if (r<0) {
                perror("rcv error:");
		exit(1);
            }
	    sta_test = rmsg[10];
	    if (traceflag) mpx("STA recv\n", rmsg, 64);

	    switch(sta_test) {				// respond with CONFIRM and start test
		case APTS_HELLO_RESP:
		case B_D:
		case B_2:
		case B_H:
		case B_4:
		case B_B:
		case B_E:
		case B_G:
		case B_I:
		create_apts_msg(APTS_CONFIRM, txmsg);
    	    	r = sendto(sd, txmsg, msgsize, sockflags, &dst, sizeof(dst));
		sta_state = 2;
	        break;
	    default:
		continue;
            }
	}

	printf("STAUT: starting test %d\n", sta_test);
	set_pwr_mgmt("STAUT", P_ON);

	switch (sta_test) {
	case B_D:
	case B_2:				// Example: STAUT Test B_2
	case B_H:				// 
	case B_4:				// 
	case B_B:				// 
	case B_E:				// 
	case B_G:				// 
	case B_I:				// 
            wstate = WAIT_STUAT_02;
            if (sta_test == B_E) wstate=WAIT_STUAT_0E; // send two uplink frame
            if (sta_test == B_G) wstate=WAIT_STUAT_0E; // send two uplink frame
            if (sta_test == B_I) wstate=WAIT_STUAT_0E; // send two uplink frame
            setup_period(period);
	    create_apts_msg(APTS_DEFAULT, txmsg);
            setup_timers();
            timeout();
	    fromlen = sizeof(from);

	    while (1) {

		r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);

		if (traceflag) mpx("STA recv\n", rmsg, 64);

	        if ( rmsg[10] == APTS_STOP ) { 
		    printf("STAUT: STOP\n");
		    goto start;
		}

		if (prcvflag) {
		    gettimeofday(&time_ap, 0);
		    time_delta = timediff(&time_ap, &time_ul);
		    printf("rcv(%d) msgno(%d) cmd(%2d) ", r, rmsg[0], rmsg[10]);
		    mptimeval("delta ", &time_delta);
		}
	    }	    // while(1)
	}	    // switch( sta_test)
     }		    // while (stautflag)
}		    // do_staut

void
do_phone()
{
unsigned long rmsg[2048];
int r, flags, rmsgno;
struct histogram h;

    if (phoneflag||upsdflag) {

        setup_period(period);
        setup_timers();

	if (histflag) {
	    h.nsamples = 1000;
	}
	wstate = WAIT_NEXT_CODEC;

	timeout();                  // kick off first uplink packet 

        while (1) {

	    fromlen = sizeof(from);
	    r = recvfrom(sd, rmsg, sizeof(rmsg), flags, (struct sockaddr *)&from, &fromlen);

	    rmsgno = rmsg[0];
            if (rmsgno != msgno_ul) {
		rmiss++;
                continue;
	    }

	    gettimeofday(&time_ap, 0);
	    time_delta = timediff(&time_ap, &time_ul);

	    if (histflag) {
		dohist(&h, time_delta.tv_usec);
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
}


main(int argc, char **argv)
{
int i;
int which_program;

    
    which_program = parse_program(argv[0]);	// which program is running

    for(i=1; i<argc; i++) {			// parse command line options
        if (argv[i][0] != '-') {
            if (EQ(argv[i], "?") || EQ(argv[i], "help") || EQ(argv[i], "-h")) {
                goto helpme;
            }

            targetname = argv[i];			// gather non-option args here
	    if (ntarg < NTARG) {
		targetnames[ntarg++] = targetname;
	    }
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
	    nend = atoi(count);
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
	if (EQ(argv[i], "-t") || EQ(argv[i], "-trace")) {
	    traceflag = 1;
	    continue;
	}
	if (EQ(argv[i], "-brcm") || EQ(argv[i], "-BRCM")) {
	    brcmflag = 1;
	    codec_sec = 0;
	    codec_usec = 500000;
	    continue;
	}
	if (EQ(argv[i], "-athr") || EQ(argv[i], "-ATHR")) {
	    athrflag = 1;
	    codec_sec = 2;
	    codec_usec = 0;
	    continue;
	}
        if (EQ(argv[i], "-h")) {
        helpme:
            printf("-codec\n-period\n-hist\n-wmeload\n-broadcast\n-count\n-qdata\n-qdis\n-tspec\n");
            exit(0);
        }
    }

    if (ntarg==0 && !apflag && !pgetflag) {	// check arg count
	if (which_program == APTS) {
		apts_lookup(0);
	}
        fprintf(stderr, "need at least one target address or test name\n");
        exit(-1);
    }


    setup_socket(targetname);

    switch(which_program) {
    case APTS:
	    do_apts(); break;
    case STAUT:
	    do_staut(); break;
    case PHONE:
    case UPSD:
	    do_phone(); break;
    case AP:
	    do_ap(); break;
    case PGEN:
	    do_pgen(); break;
    case PGET:
	    do_pget(); break;
    }
}

set_pwr_mgmt(char *msg, int mode)
{

    if (brcmflag) {			// BRCM specific
	if (mode==P_OFF) {
	    if ( system("./wl.exe PM 0") < 0 ) {
	        printf("wl power-save call error\n");
	    } else {
		printf("wl power-save OFF\n", msg);
	    }
	}
	
	if (mode==P_ON) {
	   if ( system("./wl.exe PM 1") < 0 ) {
		printf("wl power-save call error\n");
	    } else {
		printf("wl power-save ON\n");
	    }
	}
    }

    if (athrflag) {			// ATHR specific
	if (mode==P_OFF) {
	    if ( system("iwpriv ath0 sleep 0") < 0) {
		printf("powersave botch\n");
	    } else {
		printf("power save OFF\n");
	    }
	}
	if (mode==P_ON) {
	    if ( system("iwpriv ath0 sleep 1") < 0) {
		printf("powersave botch\n");
	    } else {
		printf("power save on\n");
	    }
	}
    }
	
}

