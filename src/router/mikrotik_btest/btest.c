/* Program to emulate the Mikrotik Bandwidth test protocol */
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>

#ifdef __MACH__
#include "timing_mach.h"
#endif

#define BTEST_PORT 2000
#define BTEST_PORT_CLIENT_OFFSET 256
#define CMD_PROTO_UDP 0
#define CMD_PROTO_TCP 1
#define CMD_DIR_RX 0x01
#define CMD_DIR_TX 0x02
#define CMD_RANDOM 0

unsigned char helloStr[] = { 0x01, 0x00, 0x00, 0x00 };
//unsigned char cmdStr[16];
unsigned int udpport=BTEST_PORT;

struct cmdStruct {
	int proto;
	int direction;
	int random;
	int tcp_conn_count;
	int tx_size;
	int client_buf_size;
	unsigned long remote_tx_speed;
	unsigned long local_tx_speed;
};

struct statStruct {
	unsigned long seq;
	unsigned char unknown[3];
	unsigned long recvBytes;
	unsigned long maxInterval; // In us, Not sent over the wire
	unsigned long minInterval; // In us, Not sent over the wire
	unsigned long lostPackets; // Not sent over the wire
};

void usage();
void usage_long();
int server();
int client();
int server_conn(int cmdsock, char *);
struct cmdStruct unpackCmdStr(unsigned char *);
void packCmdStr(struct cmdStruct *, unsigned char *);
struct statStruct unpackStatStr(unsigned char *);
void packStatStr(struct statStruct *, unsigned char *);
void printStatStruct(char *, struct statStruct *);
int test_udp(struct cmdStruct, int, char *); 
int test_tcp(struct cmdStruct, int);
void timespec_diff(struct timespec *, struct timespec *,
                   struct timespec *);
void timespec_add(struct timespec *, struct timespec *);
int timespec_cmp(struct timespec *t1, struct timespec *t2);
void timespec_dump(char *, struct timespec *);
void dumpBuffer(const char *msg, unsigned char *buffer, int len);
void packShortLE (unsigned char *, unsigned int);
void packLongLE (unsigned char *, unsigned long);
void packLongBE (unsigned char *, unsigned long);
void unpackShortLE (unsigned char *, unsigned int *);
void unpackLongLE (unsigned char *, unsigned long *);
void unpackLongBE (unsigned char *, unsigned long *);

char *opt_bandwidth=NULL;
int opt_udpmode=0;
int opt_server=0;
int opt_interval=0;
int opt_nat=0;
int opt_transmit=0;
int opt_receive=0;
char *opt_connect=NULL;

int main(int argc, char **argv){
	int opt;
	static struct option long_options[] =
        {
          {"udp", no_argument,       &opt_udpmode, 1},
          {"transmit", no_argument,       &opt_transmit, 1},
          {"receive", no_argument,       &opt_receive, 1},
          {"server", no_argument,       &opt_server, 1},
          {"nat", no_argument,       &opt_nat, 1},
          {"help", no_argument,       0, 'h'},
          {"client",     required_argument,       0, 'c'},
          {"interval",  required_argument,       0, 'i'},
          {"bandwidth",  required_argument, 0, 'b'},
          {0, 0, 0, 0}
    };
    int option_index = 0;

    if (argc < 2)
	{
		usage();
		exit(1);
	}

	while ((opt = getopt_long(argc, argv, "utrsnhc:i:b:",long_options,&option_index)) != -1) {
		switch (opt) {
		case 'u':
			opt_udpmode=1;
			break;
		case 't':
			opt_transmit=1;
			break;
		case 'r':
			opt_receive=1;
			break;
		case 's':
			opt_server=1;
			break;
		case 'n':
			opt_nat=1;
			break;
		case 'c':
			opt_connect=strdup(optarg);
			break;
		case 'i':
			opt_interval=atoi(optarg);
			break;
		case 'b':
			opt_bandwidth=strdup(optarg);
			break;
		case 'h':
			usage_long();
			exit(1);
		default:
			usage();
            exit(EXIT_FAILURE);
		}
	}

	if (opt_server) {
		server();
	} else {
		if (!opt_transmit && !opt_receive) {
			printf("You must specify transmit(-t) or receive(-r)\n");
            exit(EXIT_FAILURE);
		}
		client();
	}
}

void usage() {
    const char usage_shortstr[] = "Usage: btest [-s|-r -c host] [options]\n"
                           "Try `btest --help' for more information.\n";
    printf(usage_shortstr);
}

void usage_long() {
	const char usage_longstr[] = "Usage: btest [-s|-c host] [options]\n"
	                           "       btest [-h|--help]\n\n"
	                           "Server or Client:\n"
	                           "  -i, --interval  #         seconds between periodic bandwidth reports\n"
	                           "  -h, --help                show this message and quit\n"
	                           "Server specific:\n"
	                           "  -s, --server              run in server mode\n"
	                           "Client specific:\n"
	                           "  -c, --client    <host>    run in client mode, connecting to <host>\n"
	                           "  -t, --transmit 			transmit data\n"
	                           "  -r, --receive 			receive data\n"
	                           "  -u, --udp                 use UDP\n"
	                           "  -b, --bandwidth #[KMG][/#] target bandwidth in bits/sec (0 for unlimited)\n"
	                           "                            (default %d Mbit/sec for UDP, unlimited for TCP)\n"
	                           "                            (optional slash and packet count for burst mode)\n"

								;
    printf(usage_longstr);
}

int client() {
	int cmdsock;
	struct cmdStruct cmd;
	unsigned char cmdStr[16];
	struct sockaddr_in serverAddr;
	struct hostent *he;
	char *remoteIP;
	char bwmult;
	int ret;

	cmd.proto=CMD_PROTO_UDP;
	cmd.direction=opt_transmit ? CMD_DIR_RX : 0;
	cmd.direction+=opt_receive ? CMD_DIR_TX : 0;
	cmd.random=0;
	cmd.tcp_conn_count=0;
	cmd.tx_size=1500;
	cmd.client_buf_size=0;

	if (opt_bandwidth) {
		if ((ret=sscanf(opt_bandwidth, "%lu%c", &cmd.remote_tx_speed, &bwmult))<1) {
			fprintf(stderr, "Cannot parse bandwidth string\n");
			return(-1);
		}
		if (ret==2) {
			/* Apply multiplier */
			if (bwmult=='k' || bwmult=='K') {
				cmd.remote_tx_speed *= 1000;
			} else if (bwmult=='m' || bwmult=='M') {
				cmd.remote_tx_speed *= 1000000;
			} else {
				fprintf(stderr, "Cannot parse bandwidth string\n");
			}
		}
	} else {
		cmd.remote_tx_speed=0; // Unlimited
	}
	cmd.local_tx_speed=cmd.remote_tx_speed;

	packCmdStr(&cmd, cmdStr);

	if ( (he = gethostbyname( opt_connect ) ) == NULL) {
		// get the host info
		perror("gethostbyname");
		exit(1);
	}
 
        //Return the first one;
	bcopy((char *)he->h_addr, (char *)&serverAddr.sin_addr.s_addr, he->h_length);

	remoteIP=strdup(inet_ntoa(serverAddr.sin_addr));

	cmdsock = socket(PF_INET, SOCK_STREAM, 0);
	int enable = 1;
	if (setsockopt(cmdsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(BTEST_PORT);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	if (connect(cmdsock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		perror("connect");
		exit(1);
	}

	/* Look for hello string */
	if (recv(cmdsock,helloStr,sizeof(helloStr),0)<sizeof(helloStr)) {
		fprintf(stderr, "Remote did not return hello response\n");
		return(-1);
	}

        send(cmdsock,cmdStr,sizeof(cmdStr),0);

	/* Look for hello string */
	if (recv(cmdsock,helloStr,sizeof(helloStr),0)<sizeof(helloStr)) {
		fprintf(stderr, "Remote did not return hello response\n");
		return(-1);
	}

	if (cmd.proto == CMD_PROTO_UDP) {
		test_udp(cmd, cmdsock, remoteIP);
	} else {
		test_tcp(cmd, cmdsock);
	}
	return 0;
}

int server() {
	int controlSocket;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;
	socklen_t addr_size;
	int newSocket;

	printf("Running in server mode\n");
	controlSocket = socket(PF_INET, SOCK_STREAM, 0);
	int enable = 1;
	if (setsockopt(controlSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(BTEST_PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	bind(controlSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if(listen(controlSocket,5)==0)
		printf("Listening\n");
	else
		printf("Error\n");
	
	addr_size = sizeof(clientAddr);
	while(1) {
		newSocket = accept(controlSocket, (struct sockaddr *) &clientAddr, &addr_size);
		/* fork a child process to handle the new connection */
		if (!fork()) {
			server_conn(newSocket, strdup(inet_ntoa(clientAddr.sin_addr)));
      			close(newSocket);
			printf("Complete\n");
      			exit(0);
		} else {
			/*if parent, close the socket and go back to listening new requests*/
			close(newSocket);
		}
	}
	return 0;
}

int server_conn(int cmdsock, char *remoteIP) {
	int nBytes = 1;
	int i;
	struct cmdStruct cmd;
	unsigned char cmdStr[16];

	int flag = 1; 
	setsockopt(cmdsock, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

	/* Send hello message */
        send(cmdsock,helloStr,sizeof(helloStr),0);

	/* Recieve command */
	nBytes=recv(cmdsock,cmdStr,sizeof(cmdStr),0);
	if (nBytes < sizeof(cmdStr)) {
		return(-1);
	}

	cmd=unpackCmdStr(cmdStr);
	printf("proto=%d\n" , cmd.proto);
	printf("direction=%d\n" , cmd.direction);
	printf("random=%d\n" , cmd.random);
	printf("tcp_conn_count=%d\n" , cmd.tcp_conn_count);
	printf("tx_size=%d\n" , cmd.tx_size);
	printf("client_buf_size=%d\n" , cmd.client_buf_size);
	printf("remote_tx_speed=%lu\n" , cmd.remote_tx_speed);
	printf("local_tx_speed=%lu\n" , cmd.local_tx_speed);
	printf("remoteIP=%s\n" , remoteIP);

	/* Do auth here */

	/* Send all OK message */
        send(cmdsock,helloStr,sizeof(helloStr),0);
	
	if (cmd.proto == CMD_PROTO_UDP) {
		test_udp(cmd, cmdsock, remoteIP);
	} else {
		test_tcp(cmd, cmdsock);
	}
	/*
	/*loop while connection is live*/
}

void packShortLE (unsigned char *buf, unsigned int res) {
	int i;
	for (i=0; i<2; i++) {
		buf[i]=res & 0xff;
		res >>= 8;
	}
}

void packLongLE (unsigned char *buf, unsigned long res) {
	int i;
	for (i=0; i<4; i++) {
		buf[i]=res & 0xff;
		res >>= 8;
	}
}

void packLongBE (unsigned char *buf, unsigned long res) {
	int i;
	for (i=3; i>=0; i--) {
		buf[i]=res & 0xff;
		res >>= 8;
	}
}

void unpackShortLE (unsigned char *buf, unsigned int *pres) {
	int i;
	*pres=0;
	for (i=2; i>=0; i--) {
		*pres <<= 8;
		*pres += buf[i];
	}
}

void unpackLongLE (unsigned char *buf, unsigned long *pres) {
	int i;
	*pres=0;
	for (i=3; i>=0; i--) {
		*pres <<= 8;
		*pres += buf[i];
	}
}

void unpackLongBE (unsigned char *buf, unsigned long *pres) {
	int i;
	*pres=0;
	for (i=0; i<4; i++) {
		*pres <<= 8;
		*pres += buf[i];
	}
}

struct cmdStruct unpackCmdStr(unsigned char *cmdStr) {
	struct cmdStruct cmd;
	int i;

	dumpBuffer("Cmd buffer: ", cmdStr, 16);

	cmd.proto=cmdStr[0];
	cmd.direction=cmdStr[1];
	cmd.random=cmdStr[2];
	cmd.tcp_conn_count=cmdStr[3];

	unpackShortLE(&cmdStr[4], &cmd.tx_size);

	/* Assume little endian */
	unpackShortLE(&cmdStr[6], &cmd.client_buf_size);
	unpackLongLE(&cmdStr[8], &cmd.remote_tx_speed);
	unpackLongLE(&cmdStr[12], &cmd.local_tx_speed);

	return(cmd);
}

void packCmdStr(struct cmdStruct *pcmd, unsigned char *buf) {

	buf[0]=(pcmd->proto==CMD_PROTO_TCP) ? CMD_PROTO_TCP : CMD_PROTO_UDP;
	buf[1]=pcmd->direction & 0x03;
	buf[2]=pcmd->random ? 1 : 0;
	buf[3]=pcmd->tcp_conn_count;

	/* Little endian */
	packShortLE(&buf[4], pcmd->tx_size);

	packShortLE(&buf[6], pcmd->client_buf_size);
	
	packLongLE(&buf[8], pcmd->remote_tx_speed);
	packLongLE(&buf[12], pcmd->local_tx_speed);

	dumpBuffer("Packed Buffer: ", buf, 16);

	return;
}

struct statStruct unpackStatStr(unsigned char *buf) {
	struct statStruct stat;

	// dumpBuffer("Stat buffer: ", buf, 12);

	unpackLongBE(&buf[1], &stat.seq);

	memcpy(stat.unknown, buf+5, 3);

	unpackLongLE(&buf[8], &stat.recvBytes);

	/* These three are not used */
	stat.maxInterval=0;
	stat.minInterval=0;
	stat.lostPackets=0;
	return(stat);
}

void printStatStruct(char *msg, struct statStruct *pstat) {
	/* Work out the bit rate - assume status every second */
	double bitRate;
	char bitRateStr[20];

	bitRate=pstat->recvBytes*8;
	if (bitRate > 10000000) {
		sprintf(bitRateStr, "%.1fMbps", bitRate/1000000);
	} else {
		sprintf(bitRateStr, "%.1fkbps", bitRate/1000);
	}
	printf("%sSeq: %lu, Rate: %s",
		msg,
		pstat->seq,
		bitRateStr
	);
	if (pstat->maxInterval>0) {
		printf(", Lost: %lu, Min: %.4lfms, Max: %.4lfms, Diff %0.4lfms\n",
			pstat->lostPackets,
			((double) pstat->minInterval)/1000,
			((double) pstat->maxInterval)/1000,
			((double) pstat->maxInterval)/1000 - ((double) pstat->minInterval)/1000
		);
	} else {
		printf("\n");
	}
}

void packStatStr(struct statStruct *pstat, unsigned char *buf) {

	buf[0]=0x07;
	packLongBE(&buf[1], pstat->seq);
	buf[5]=0x00;
	buf[6]=0x00;
	buf[7]=0x00;
	packLongLE(&buf[8], pstat->recvBytes);

	// dumpBuffer("Stat buffer: ", buf, 12);

	return;
}

int udpSocket;

void *test_udp_tx(void *arg) {
	struct cmdStruct *pcmd;
	unsigned char *buf;
	int seq=1;
	int tmp, i;
	struct timespec interval; /* Interval between packets in nano seconds */
	clockid_t clock_id;
	struct timespec nextPacketTime;
	unsigned long tx_speed;

	printf("Calling test_udp_tx()\n");
	// sleep(1);
	pcmd = (struct cmdStruct *)arg;
	printf("Calling test_udp_tx(%d)\n", pcmd->tx_size);
	if (opt_server) {
		tx_speed=pcmd->remote_tx_speed;
	} else {
		tx_speed=pcmd->local_tx_speed;
	}
	printf("Tx speed: %lu\n", tx_speed);
	if (tx_speed > 0) {
		// pthread_getcpuclockid(pthread_self(), &clock_id);
		interval.tv_sec=0;
		interval.tv_nsec = 1000000000/tx_speed;
		interval.tv_nsec *= pcmd->tx_size*8;
		/* Duplicate bug? in MT where anything less than 2 packets per second gets converted to 1 packet second */
		if (interval.tv_nsec > 500000000) {
			interval.tv_nsec=0;
			interval.tv_sec=1;
		}
	} else {
		interval.tv_nsec=0;
		interval.tv_sec=0;
	}
	timespec_dump("Interval: ", &interval);
	buf=(unsigned char *) malloc(pcmd->tx_size-28);
	printf("Calling test_udp_tx(more)\n");
	bzero(buf, pcmd->tx_size-28);

	/* Get current time and add the interval to it */
	clock_gettime(CLOCK_REALTIME, &nextPacketTime);
	timespec_dump("gettime: ", &nextPacketTime);
	while(1) {
		nextPacketTime.tv_sec += interval.tv_sec;
		nextPacketTime.tv_nsec += interval.tv_nsec;
		if (nextPacketTime.tv_nsec >= 1000000000) {
			nextPacketTime.tv_sec += nextPacketTime.tv_nsec/1000000000;
			nextPacketTime.tv_nsec %= 1000000000;
		}
		int tmp=seq++;
		int nBytes;
	
		//printf("seq=%d\n", seq);
		/* Put in sequence number */
		for (i=3; i>=0; i--) {
			buf[i]=tmp % 256;
			tmp = tmp >> 8;
		}
		// printf("Sleep until %lu:%lu\n", nextPacketTime.tv_sec, nextPacketTime.tv_nsec);
		//timespec_dump("sleep until: ", &nextPacketTime);
#ifdef __MACH__
		clock_nanosleep_abstime(&nextPacketTime);
#else
		if (clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &nextPacketTime, NULL) < 0) {
			perror("clock_nanosleep: ");
		}
#endif

		send(udpSocket, buf, pcmd->tx_size-28,0);
		/*
		if (send(udpSocket, buf, pcmd->tx_size-28,0)<0) {
			perror("send udp: ");
			exit(-1);
		}
		*/
	}
}

struct statStruct recvStats;

void *test_udp_rx(void *arg) {
	struct cmdStruct *pcmd;
	unsigned char *buf;
	int nBytes;
	struct timespec last;
	struct timespec now;
	struct timespec interval;
	unsigned long intervalUs;
	unsigned long lastSeq=0;
	unsigned long thisSeq=0;

	recvStats.recvBytes=0;
	recvStats.maxInterval=0;
	recvStats.minInterval=0;
	recvStats.lostPackets=0;
	pcmd = (struct cmdStruct *)arg;
	printf("Calling test_udp_rx(tx_size=%d)\n", pcmd->tx_size);
	buf=(unsigned char *) malloc(pcmd->tx_size);
	last.tv_sec=0;
	last.tv_nsec=0;
	while(1) {
		if ((nBytes=recv(udpSocket, buf, pcmd->tx_size,0))<0) {
			/* Ignore connection refused - the other end probably wasn't ready */
			if (errno!=ECONNREFUSED) {
				perror("test_udp_rx: recv udp: ");
				exit(-1);
			}
		}
		if (nBytes>0) {
			clock_gettime(CLOCK_REALTIME, &now);

			thisSeq=buf[0];
			thisSeq=thisSeq * 256 + buf[1];
			thisSeq=thisSeq * 256 + buf[2];
			thisSeq=thisSeq * 256 + buf[3];
			if (lastSeq>0) {
				/* Ignore the first one - we often lose packets initially */
				recvStats.lostPackets += thisSeq-lastSeq-1;
			}

			if (last.tv_sec!=0 && last.tv_nsec!=0) {
				/* Work out time difference */
				timespec_diff(&last, &now, &interval);
				intervalUs=interval.tv_nsec/1000;
				intervalUs+=interval.tv_sec*1000000;

				/* Divide by the difference in the sequence
				** number so that we don't get a bump in the
				** interval due to a bunch of missed packets
				*/
				intervalUs /= thisSeq-lastSeq;

				if (recvStats.maxInterval==0) {
					/* First result */
					// printf("First: %lu\n", intervalUs);
					recvStats.maxInterval=intervalUs;
					recvStats.minInterval=intervalUs;
				} else {
					if (intervalUs > recvStats.maxInterval) {
						recvStats.maxInterval=intervalUs;
					}
					if (intervalUs < recvStats.minInterval) {
						recvStats.minInterval=intervalUs;
					}
				}
			}
			last=now;
			recvStats.recvBytes+=nBytes+28;
			lastSeq=thisSeq;
		}
	}
}

int test_udp(struct cmdStruct cmd, int cmdsock, char *remoteIP) {
	unsigned char socknumbuf[2];	
	pthread_t pth_tx;
	pthread_t pth_rx;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addr_size;
	int nBytes, i;
	unsigned char buffer[1024];
	struct statStruct remoteStats;
	struct timespec interval; /* Interval between status messages */
	struct timespec nextStatusTime;
	struct timespec timeout;
	struct timespec now;

	printf("Calling test_udp()\n");
	if (opt_server) {
		/* Send server socket number */
		udpport++;
		socknumbuf[0]=udpport / 256;
		socknumbuf[1]=udpport % 256;
        	send(cmdsock,socknumbuf,sizeof(socknumbuf),0);
	} else {
        	if (recv(cmdsock,socknumbuf,sizeof(socknumbuf),0)<sizeof(socknumbuf)) {
			fprintf(stderr, "Did not recieve remote port number\n");
			return(-1);
		}
		dumpBuffer("Socket number buffer: ", socknumbuf, 2);
		udpport=(socknumbuf[0] << 8) + socknumbuf[1];
	}
	printf("Calling test_udp(udpport=%d)\n", udpport);

	addr_size = sizeof(clientAddr);

	/* Create a UDP socket to transmit/recieve the data */
	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
	serverAddr.sin_family = AF_INET;
	if (opt_server) {
		serverAddr.sin_port = htons(udpport);
	} else {
		serverAddr.sin_port = htons(udpport+BTEST_PORT_CLIENT_OFFSET);
	}
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	clientAddr.sin_family = AF_INET;
	if (opt_server) {
		clientAddr.sin_port = htons(udpport+BTEST_PORT_CLIENT_OFFSET);
	} else {
		clientAddr.sin_port = htons(udpport);
	}
	clientAddr.sin_addr.s_addr = inet_addr(remoteIP);
	memset(clientAddr.sin_zero, '\0', sizeof clientAddr.sin_zero);  

	/* Connect it to the remote end */
	connect(udpSocket, (struct sockaddr *) &clientAddr, sizeof(clientAddr));

	/* We have to swap the command round between server and client */
	if (((cmd.direction & CMD_DIR_TX) && opt_server) || ((cmd.direction & CMD_DIR_RX) && !opt_server)) {
		pthread_create(&pth_tx,NULL,test_udp_tx,(void *)&cmd);
	}
	if (((cmd.direction & CMD_DIR_RX) && opt_server) || ((cmd.direction & CMD_DIR_TX) && !opt_server)) {
		if (opt_nat) {
			/* Send a zero length packet to open any firewall */
			send(udpSocket, buffer, 0, 0);
		}
		pthread_create(&pth_rx,NULL,test_udp_rx,(void *)&cmd);
	}


	/* Interval between status messages */
	interval.tv_nsec=0;
	interval.tv_sec=1;

	/* Get current time and add the interval to it */
	clock_gettime(CLOCK_REALTIME, &nextStatusTime);
	timespec_add(&nextStatusTime, &interval);
	recvStats.seq=0;
	while(1) {
		int ready;
		fd_set readfds;

		clock_gettime(CLOCK_REALTIME, &now);
		timespec_diff(&now, &nextStatusTime, &timeout);

		FD_ZERO(&readfds);
		FD_SET(cmdsock, &readfds);
		ready=pselect(cmdsock+1, &readfds, NULL, NULL, &timeout, NULL);
		if (ready) {
			nBytes=recv(cmdsock,buffer,1024,0);
			if (nBytes<=0) {
				exit(0);
			}
			remoteStats=unpackStatStr(buffer);
			if (((cmd.direction & CMD_DIR_TX) && opt_server) || ((cmd.direction & CMD_DIR_RX) && !opt_server)) {
				/* Only print this if we are transmitting */
				printStatStruct("Remote: ", &remoteStats);
			}
		}

		clock_gettime(CLOCK_REALTIME, &now);
		if (timespec_cmp(&now, &nextStatusTime) > 0) {
			recvStats.seq++;
			packStatStr(&recvStats, buffer);
			if (((cmd.direction & CMD_DIR_RX) && opt_server) || ((cmd.direction & CMD_DIR_TX) && !opt_server)) {
				/* Only print this if we are recieving */
				printStatStruct("Local : ", &recvStats);
			}
			
			send(cmdsock, buffer, 12, 0);
			timespec_add(&nextStatusTime, &interval);
			recvStats.recvBytes=0;
			recvStats.maxInterval=0;
			recvStats.minInterval=0;
			recvStats.lostPackets=0;
		}
	}
}

int tcpSocket;

void *test_tcp_tx(void *arg) {
	struct cmdStruct *pcmd;
	unsigned char *buf;
	int ret;

	printf("Calling test_tcp_tx()\n");
	sleep(1);
	pcmd = (struct cmdStruct *)arg;
	buf=(unsigned char *) malloc(pcmd->tx_size);
	bzero(buf, pcmd->tx_size);

	buf[0]=0x07;
	buf[4]=0x01;

	while(send(tcpSocket, buf, pcmd->tx_size,0)>0);
}

int test_tcp(struct cmdStruct cmd, int cmdsock) {
	pthread_t pth_tx;
	pthread_t pth_rx;
	int nBytes, i;
	unsigned char buffer[1024];

	printf("Calling test_tcp()\n");
	tcpSocket=cmdsock;
	if (cmd.direction == CMD_DIR_TX) {
		pthread_create(&pth_tx,NULL,test_tcp_tx,(void *)&cmd);
	}

	printf("Listening on TCP cmdsock\n");
	while(nBytes=recv(cmdsock,buffer,1024,0)){
		for (i=0;i<nBytes-1;i++){
			printf("%02x", buffer[i]);
		}
		printf("\n");
        }
}

/* Calculate the difference between two timespec's */
void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}

	return;
}

/* Add timespec t2 onto timespec t1 onto an existing one  */
void timespec_add(struct timespec *t1, struct timespec *t2)
{
	t1->tv_sec += t2->tv_sec;
	t1->tv_nsec += t2->tv_nsec;
	if (t1->tv_nsec >= 1000000000) {
		t1->tv_sec += t1->tv_nsec/1000000000;
		t1->tv_nsec %= 1000000000;
	}
}

/* Return -1, 0, or +1 if t1 less than, equal to or greater than t2 */
int timespec_cmp(struct timespec *t1, struct timespec *t2)
{
	if (t1->tv_sec < t2->tv_sec) {
		return(-1);
	} else if (t1->tv_sec > t2->tv_sec) {
		return(1);
	} else {
		if (t1->tv_nsec < t2->tv_nsec) {
			return(-1);
		} else if (t1->tv_nsec > t2->tv_nsec) {
			return(1);
		} else {
			return(0);
		}
	}
}

void timespec_dump(char *msg, struct timespec *t1)
{
	printf("%s %lu:%lu\n", msg, t1->tv_sec, t1->tv_nsec);
}

void dumpBuffer(const char *msg, unsigned char *buffer, int len)
{
	int i;
	printf("%s", msg);
	for (i=0;i<len;i++){
		printf("%02x", buffer[i]);
	}
	printf("\n");
}
