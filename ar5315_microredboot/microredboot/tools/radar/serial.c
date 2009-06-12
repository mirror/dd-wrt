#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>

#define	Mhz	1000000
#define	Usec	.000001
double samplerate = 80.;
double chirpspan = 5.0; 
double pulselen = 50.0; /*duration of pulse*/
int pulse_interval = 1000;
int npulses = 8;
double freq = 10.0;

//char *serial_device = "/dev/ttyUSB0";
char *serial_device = "/dev/ttyS0";
FILE *SD;
int sd;
struct termios terminal;

unsigned char debugflag = 0;
FILE *DB;

#define	T(FLAG)	if (flags& FLAG ) {strcat(pbuf," +");strcat(pbuf,#FLAG);}

p_iflags(struct termios *t)
{
int flags;
char pbuf[1024];

	flags = t->c_iflag;
	pbuf[0] = 0;
	T(IGNBRK); T(BRKINT); T(IGNPAR); T(PARMRK);
	T(INPCK); T(ISTRIP); T(INLCR); T(IXON);
	T(IXANY); T(IXOFF); T(IMAXBEL);
	printf("iflags: %s\n", pbuf);
}

p_oflags(struct termios *t)
{
int flags;
char pbuf[1024];
	flags = t->c_oflag;
	pbuf[0] = 0;
	T(OPOST); T(OLCUC); T(ONLCR); T(OCRNL); T(ONOCR); T(ONLRET);
	T(OFILL); T(OFDEL); T(NLDLY); T(CRDLY); T(TABDLY); T(BSDLY);
	T(VTDLY); T(FFDLY);
	printf("oflags: %s\n", pbuf);
}

p_cflags(struct termios *t)
{
int flags;
char pbuf[1024];
	flags = t->c_cflag;
	pbuf[0] = 0;
	T(CSIZE); T(CSTOPB); T(CREAD); T(PARENB);
	T(PARODD); T(HUPCL); T(CLOCAL); 
	T(CRTSCTS);
	printf("cflags: %s\n", pbuf);
}

serial_connect()
{
struct termios *tp = &terminal;
int r;

    sd = open(serial_device, O_RDWR);
    if (sd<0) { perror("serial open"); exit(1); }

    SD = fdopen(sd, "r+");
    if (!SD) { perror("serial fdopen"); exit(1); }

    r = tcgetattr(sd, tp);
    if (r<0) { perror("tcgetattr"); exit(1); }
    //tp->c_iflag = IMAXBEL+IXANY+IGNBRK+INLCR+IXON;
    tp->c_iflag = IGNBRK;
    tp->c_oflag = 0;
    tp->c_cflag = CREAD+CLOCAL+CRTSCTS+CS8;
    tp->c_cflag = 0x80000cbd;
    tp->c_lflag = 0;
    cfsetospeed(tp, B115200);
    cfsetispeed(tp, B115200);
    r = tcsetattr(sd, TCSANOW,  tp);

    r = tcgetattr(sd, tp);
    if (r<0) { perror("tcgetattr"); exit(1); }

    printf("speed\t%d\n", tp->c_ispeed);
    printf("imodes\t%x\n", tp->c_iflag);
    p_iflags(tp);
    printf("omodes\t%x\n", tp->c_oflag);
    p_oflags(tp);
    printf("cntl\t%x\n", tp->c_cflag);
    p_cflags(tp);
    printf("local\t%x\n", tp->c_lflag);
    printf("line\t%x\n", tp->c_line);
}




#define NSAMP	4000000
unsigned short b1[NSAMP];

int
getnum(char *s)
{
char c;
char isneg = 1;

	while (1) {
		c = *s;
		if (c=='-') {
			isneg = -1;
			s++;
			continue;
		}
		if (isdigit(c))
			break;
		if (c==0)
			return(0);
		s++;
	}
	return(isneg * atoi(s));
}
	
	 

/*
 * convert a double array to Rohde&Schwarz binary format
 */
f_to_bin(double from[], unsigned short to[], int count)
{
int i;
unsigned short bin;

	for(i=0; i<count; i++) {
		bin = (unsigned short)(32768.0 + (from[i]*32000.0)+.5);
		bin &= 0xfffc;
		to[i] = bin;
	}
}

int
c_signal()
{
int u, d, i, val, count;

    u = NSAMP;
    u = 4096;
    d = 1024;

    val = 0;
    count = 0;
    for(i=0; i<NSAMP; i++, count++) {
	if (count==d) {
		val = (val)? 0 : 61000;
		count = 0;
	}
	b1[i] = val;
    }
    return(u);
	
}

char wave_header[128];
char mem_header[128];

int
ndigits(int n)
{
char buf[32];

	buf[0] = 0;
	sprintf(buf, "%d", n);
	return(strlen(buf));
}


int
old_chirp(unsigned short wbuf[], int ipos, double span, double duration, double samplerate)
{
double omega, omega0, omega_carrier, alpha;
double omega_samplerate = (samplerate*2*M_PI);
int  pos, epos, rad;

    omega0 = span*2*M_PI*Mhz/2;				// radians per usec
    alpha = span*2*M_PI*Mhz/(duration*Usec);		// Mhz per us

    epos = ipos + duration*samplerate;
    pos = ipos;
    rad = 0;

	printf("gen chirp\n");
	printf("span %f omega0 %f srate %f alpha %f duration %f\n", 
		span, omega0, samplerate, alpha, duration);
	printf("ipos %d epos %d\n", ipos, epos);
	if (debugflag) {
		fprintf(DB, "span %f omega0 %f srate %f\nalpha %f duration %f\n", 
			span, omega0, samplerate, alpha, duration);
		fprintf(DB, "ipos %d epos %d\n", ipos, epos);
	}

    while (pos < epos) {
	double t, f, z;
	unsigned short w, tstep;

	tstep = pos-ipos;
	t = tstep;
	z = tstep*Usec/samplerate;
	//f = alpha*z*z - omega0*z;
	f = alpha*z*z;
	t = sin(omega*f);
	w = (unsigned short)(32768.0 + (t*32000.0)+.5);
	b1[ipos + 2*tstep] = w & 0xfffc;
	if (debugflag) {
		//fprintf(DB, "[%d] %e \t", tstep, z);
		//fprintf(DB, " %f %f %d ", f, t, w & 0xfffc);
		//fprintf(DB, "\n");
		fprintf(DB, "%d %f %f %d \n", tstep, f, t, w & 0xfffc);
	}
	t = cos(omega*f);
	w = (unsigned short)(32768.0 + (t*32000.0)+.5);
	b1[ipos + 2*tstep+1] = w & 0xfffc;
	pos += 1;
    }
    for (pos=ipos; pos<epos; pos++) {
    }

    return((epos-ipos)*2);
}

int
new_chirp(unsigned short wbuf[], double span, double duration, double samplerate)
{
double omega, f, t, s;
int samples, pos, i;
int ntones, niq;

	samples = duration*samplerate;		
	samples = (samples+3)& ~3;
	ntones = duration*2;

	t = span/ntones;
	f = t;
	s = duration/ntones;
	for(pos=i=0; i<ntones; i++) {
		printf("pos %d f %f\n", pos, f);
		niq = gen_tone(&wbuf[pos], f, s, samplerate);
		pos += niq;
		f += t;
	}
	return(pos);
}

int
gen_null(unsigned short wbuf[], double span, double duration, double samplerate)
{
int samples, i;

	samples = duration*samplerate;		
	samples = (samples+3)& ~3;

	for(i=0; i<samples*2; i++) {
		wbuf[i] = 32768;
	}
	return (i);
}

int
gen_chirp(unsigned short wbuf[], double span, double duration, double samplerate)
{
double omega, omega0, omega_carrier, alpha;
double omega_samplerate = (samplerate*2*M_PI);
int  pos, ipos, epos, samples, tstep;

    omega = omega0 = span*M_PI*Mhz;			// radians per usec
    alpha = span*M_PI*Mhz/(duration*Usec);		// Mhz per us

    ipos = 0;
    samples = duration*samplerate;
    samples = (samples+3) & ~3;
    pos = ipos;

	printf("gen chirp span %f duration %f\n", span, duration);
	if (debugflag) {
		fprintf(DB, "span %f omega0 %f alpha %f\nduration %f samples %d\n", 
			span, omega0, alpha, duration, samples);
	}

   for(tstep=0; tstep<samples; tstep++) {
	double t, f, z;
	unsigned short w;

	z = tstep*Usec/samplerate;
	//f = alpha*z*z - omega0*z;
	f = alpha*z*z;
	//t = cos(omega*f);
	t = cos(f);
	w = (unsigned short)(32768.0 + (t*32000.0)+.5);
	wbuf[2*tstep] = w & 0xfffc;
	if (debugflag) {
		fprintf(DB, "%4d %8f %8f %d \n", tstep, f, t, w & 0xfffc);
	}
	//t = sin(omega*f);
	t = sin(f);
	w = (unsigned short)(32768.0 + (t*32000.0)+.5);
	wbuf[(2*tstep)+1] = w & 0xfffc;
    }
    return(samples*2);
}

pulse()
{
char *arm = ":ARM\n";
char *trig = ":TRIG\n";
char abuf[128];

	abuf[0] = 0;
	strcat(abuf, ":OUTP:I FIX\n:OUTP:Q FIX\n:CLOCK 80e6,FAST\n:TRIG:MODE SING\n:ARM\n");
	write(sd, abuf, strlen(abuf));
	write(1, abuf, strlen(abuf));
	while (npulses) {
		write(sd, trig, strlen(trig));
		//write(1, trig, strlen(trig));
		usleep(pulse_interval);
		write(sd, arm, strlen(arm));
		//write(1, arm, strlen(arm));

		if (npulses>0) {
			npulses--;
		}
	}
}


int
gen_tone(unsigned short wbuf[], double chirp, double duration, double samplerate)
{
double omega, omega0;
double omega_samplerate = (samplerate*2*M_PI);
int  ipos, pos, epos, rad;

    omega0 = freq*2*M_PI;		// radians per usec
    omega = omega0/samplerate;    	// radians per sample

    ipos = 0;
    epos = ipos + duration*samplerate;
    pos = ipos;
    rad = 0;
//printf("gen tone\n");
//printf("omega0 %f srate %f omega %f duration %f\n", omega0, samplerate, omega, duration);
    if (debugflag) {
	fprintf(DB, "omega0 %f srate %f omega %f duration %f\n", omega0, samplerate, omega, duration);
	fprintf(DB, "ipos %d epos %d\n", ipos, epos);
    }

    while (pos < epos) {
	double t;
	unsigned short w;

	t = sin(omega*rad);
	w = (unsigned short)(32768.0 + (t*32000.0)+.5);
	wbuf[ipos + 2*rad] = w & 0xfffc;
	if (debugflag)
		fprintf(DB, "%d %f %f %d \n", pos, pos*omega, sin(omega*pos), w & 0xfffc);
	t = cos(omega*rad);
	w = (unsigned short)(32768.0 + (t*32000.0)+.5);
	wbuf[ipos + 2*pos+1] = w & 0xfffc;
	pos += 1;
	rad += 1;
    }

    return((epos-ipos)*2);
}

trytwo(int f())
{
int niq;
int h;
char buf[4096];
int i, n;

	printf("try n\n");
	for(i=0; i<NSAMP; i++) {
		b1[i] = 32768;
	}
	niq = 0;

	niq += (f(b1, 5., 10., samplerate));
	niq += gen_null(&b1[niq], 0.0, 10., samplerate);
	niq += (f(b1, 5., 20., samplerate));
	niq += gen_null(&b1[niq], 0.0, 10., samplerate);

	sprintf(wave_header, "{TYPE: WV, 0}{IDLE SIGNAL: 32768,32768}{WAVEFORM-%d: 0,#", 3+niq*2);
	h = strlen(wave_header) + niq*2;
	sprintf(mem_header, ":MEM:DATA RAM,#%d%d", ndigits(h+1), h+1);

	sprintf(buf, "%s%s", mem_header,wave_header);
	printf("wave header(%d) niq(%d)\n", strlen(wave_header), niq);
	printf("%s\n", buf);
	if (debugflag) {
		fprintf(DB, "%s%s\n", buf);
		fprintf(DB, "niq %d\n", niq);
		return;
	}

	write(sd, buf, strlen(buf));

	strcat(buf, ":SYS:LANG FAST\n");

	n = write(sd, b1, niq*2);
	if (n!= niq*2) {
		printf("botched write %d instead of %d\n", n, niq*2);
	} else {
		printf("sent %d binary bytes\n", niq*2);
	}
	write(sd, "}\n", 2);
	printf("done\n");
}

try(int f())
{
int niq;
int h;
char buf[4096];
int i, n;

	for(i=0; i<NSAMP; i++) {
		b1[i] = 32768;
	}
	niq = 0;

	niq += (f(b1, chirpspan, pulselen, samplerate));

	//sprintf(wave_header, "{TYPE: WV, 0}{CLOCK: 40MHZ}{IDLE SIGNAL: 32768,32768}{WAVEFORM-%d: 0,#", 3+niq*2);
	sprintf(wave_header, "{TYPE: WV, 0}{IDLE SIGNAL: 32768,32768}{WAVEFORM-%d: 0,#", 3+niq*2);
	h = strlen(wave_header) + niq*2;
	sprintf(mem_header, ":MEM:DATA RAM,#%d%d", ndigits(h+1), h+1);

	sprintf(buf, "%s%s", mem_header,wave_header);
	printf("wave header(%d) niq(%d)\n", strlen(wave_header), niq);
	printf("%s\n", buf);
	if (debugflag) {
		fprintf(DB, "%s%s\n", buf);
		fprintf(DB, "niq %d\n", niq);
		return;
	}

	write(sd, buf, strlen(buf));

	strcat(buf, ":SYS:LANG FAST\n");

	n = write(sd, b1, niq*2);
	if (n!= niq*2) {
		printf("botched write %d instead of %d\n", n, niq*2);
	} else {
		printf("sent %d binary bytes\n", niq*2);
	}
	write(sd, "}\n", 2);
	printf("done\n");
}



#define	FORK 1


main(int argc, char **argv)
{
char ibuf[4096];
char *in;
char *am;
char abuf[4096];
int r, n;

    serial_connect();
    bzero(ibuf, sizeof(ibuf));
    bzero(abuf, sizeof(abuf));
#ifdef FORK
    r = fork();
#else
    r = 1;
#endif


    while (r) {
	ibuf[0] = abuf[0] = 0;
	in = fgets(ibuf, sizeof(ibuf), stdin);
	if (strcmp(ibuf, "t\n")==0) {
		try(&gen_tone);
		continue;
	}
	if (strcmp(ibuf, "tt\n")==0) {
		trytwo(&gen_chirp);
		continue;
	}
	if (strcmp(ibuf, "?\n")==0 || strcmp(ibuf, "h\n")==0) {
		usage();
		continue;
	}
	if (strcmp(ibuf, "p\n")==0) {
		pulse();
		continue;
	}
	if (strcmp(ibuf, "c\n")==0) {
		try(&gen_chirp);
		continue;
	}
	if (ibuf[0]=='C') {
		if (ibuf[1]=='\n') {
			print_params();
			continue;
		}
		n = getnum(&ibuf[1]);
		chirpspan = (double)n;
		continue;
	}
	if (ibuf[0]=='D') {
		if (ibuf[1]=='\n') {
			print_params();
			continue;
		}
		n = getnum(&ibuf[1]);
		pulselen = (double)n;
		printf("duration %f\n", pulselen);
		continue;
	}
	if (ibuf[0]=='f') {
		if (ibuf[1]=='\n') {
			print_params();
			continue;
		}
		n = getnum(&ibuf[1]);
		freq = (double)n;
		printf("freq %d %f\n", n, freq);
		continue;
	}
	if (ibuf[0]=='n') {
		if (ibuf[1]=='\n') {
			print_params();
			continue;
		}
		npulses = getnum(&ibuf[1]);
		printf("npulses %d\n", npulses);
		continue;
	}

	if (strcmp(ibuf, "e\n")==0) {
		strcat(abuf, ":SYST:ERR?\n");
		write(sd, abuf, strlen(abuf));
		write(1, abuf, strlen(abuf));
		continue;
	}
	if (strcmp(ibuf, "i\n")==0) {
		strcat(abuf, "*IDN?\n");
		write(sd, abuf, strlen(abuf));
		write(1, abuf, strlen(abuf));
		continue;
	}
	if (ibuf[0] == 'I') {
		if (ibuf[1]=='\n') {
			print_params();
			continue;
		}
		pulse_interval = getnum(&ibuf[1]);
		printf("pulse interval %d\n", pulse_interval);
		continue;
	}
	if (strcmp(ibuf, "d\n")==0) {
	   if (debugflag==0) {
		debugflag = 1;
		printf("debug enabled\n");
		DB = fopen("logfile", "w");
		if (!DB) {
			printf("can't open logfile\n");
		}
		continue;
	    } else {
		debugflag = 0;
		printf("debug disabled\n");
	    }
	}
	if (strcmp(ibuf, "q\n")==0) {
		kill(r, 9);
		exit(0);
	}
	if (strcmp(ibuf, "g\n")==0) {
		strcat(abuf, ":OUTP:I FIX\n:OUTP:Q FIX\n:CLOCK 80e6,FAST\n:TRIG:MODE CONT\n:ARM\n:TRIG\n");
		write(sd, abuf, strlen(abuf));
		write(1, abuf, strlen(abuf));
		continue;
	}
	if (strcmp(ibuf, "r\n")==0) {
		strcat(abuf, "*RST\n");
		write(sd, abuf, strlen(abuf));
		write(1, abuf, strlen(abuf));
		continue;
	}
	if (strcmp(ibuf, "s\n")==0) {
		printf("span %f duration %f\n", chirpspan, pulselen);
		continue;
	}
	if (!in) {
		kill(r, 9);
		 exit(0);
	}
	printf("sending %s", in);
	fprintf(SD, "%s", ibuf);
#ifndef FORK
	am = fgets(abuf, sizeof(abuf), SD);
	printf(">%s\n", am);
#endif
    }
    while (r==0) {
	int cc, i;

	cc = read(sd, abuf, sizeof(abuf));
	for(i=0; i<cc; i++) {
		if (!isprint(abuf[i])) {
			abuf[i] = '#';
		}
	}
	write(1, abuf, cc);
	abuf[0] = 0;
    }
}

usage()
{
	printf("t\t\t- generate sine wave tone\n");
	printf("c\t\t- generate chirp\n");
	printf("C\t\t- set chirp frequency span (in Mhz)\n");
	printf("D\t\t- set chirp duration (in usecs)\n");
	printf("e\t\t- query error status from device\n");
	printf("f\t\t- set frequency (in Mhz)\n");
	printf("i\t\t- send IDENT command to AMIQ\n");
	printf("g\t\t- start device in CONT TRIGGER mode\n");
	printf("p\t\t- start device in SINGLE TRIGGER mode\n");
	printf("q\t\t- exit program\n");
}

print_params()
{
	printf("Duration: %f us\n", pulselen);
	printf("Frequency: %f Mhz\n", freq);
	printf("chirp span %f Mhz\n",  chirpspan);
	printf("npulses %d\n", npulses);
	printf("pulse interval %d\n", pulse_interval);
}
