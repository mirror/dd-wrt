//#############################################################################
/** @file				can-proxy.c
 @author:		T.Motylewski@bfad.de
 Operating System:	LINUX
 Compiler:			gcc
 Description: 		gateway CAN-UDP.
receives all CAN packets, prints them and may save them to a file.
may send packets with ID, len, data entered from keyboard or from text file.
may send to CAN packets received over UDP
will forward packets from CAN to the most recent IP/UDP address.
*/
//#############################################################################


//-----------------------------------------------------------------------------
//                                  INCLUDES
//-----------------------------------------------------------------------------

#include <stdio.h>
#include "can.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

#define DEFAULT_CAN_SERVER_PORT 3007
#define DEFAULT_CAN_CLIENT_PORT 3008
#define MAX_MSG_SIZE 4000

extern int SendCanMsg(canmsg_t * msg);
extern int ReceiveCanMsg(canmsg_t * msg);
extern int ReadInput(int fd);


/** tm means time in units of 1/100 s since program started (tsStarted)
 * tm time stays positive for 24.8 days after system boot.
 * It can be redefined by changing TM_HZ value to something else than 1000.
 * Always use TM_HZ when comparing time with real seconds.
 * The code should work after "time wraparound" but if something happens you know why.
 * ts means time in sec */
#define TM_HZ 1000


#define MSG_CAN_UNKNOWN -1
#define MSG_CAN_NONE 0
#define TRUE 1
#define FALSE 0
#define CAN_MSG_SIZE sizeof(canmsg_t)
#define CAN_DEV "/dev/can0"

enum BusStateFlags {
	BusReset = 1,
	BusLocked = 1<<1
};

#define WARN(fmt...) (fprintf(stderr,"WARN:" fmt),fprintf(stderr,"\n"))
#define ERR(fmt...) (fprintf(stderr,"ERROR:" fmt),fprintf(stderr,"\n"))

#ifdef DEBUG
	//#define DMSG(fmt...) (fprintf(stderr,"DBG:" fmt),fprintf(stderr," %f\n",gettime()))
	//very usefull for debugging data change after some miliseconds combined with dumpnc.c

	#define DMSG(fmt...) (fprintf(stderr,"DBG:" fmt),fprintf(stderr,"\n"))
#else
	#define DMSG(fmt...)
#endif


/** global variables */

unsigned short int myport;
unsigned short int toport;
struct sockaddr_in myaddr, fromaddr, recvaddr, toaddr;


int fdCanDev;
int fdNet;
int fdError;
int fdIn;
FILE * fLog;
long iBusFlags;
time_t tsStarted;
long tmLastVerified=0;
long tmNow;  // current time, global, will be updated in many places
long tmLastSentReceived=0;
struct timeval tvNow;
struct timeval tvSelectTimeout;
double time0;
double SleepUntil=0;

int quiet = 0;
int use_select = 0;


long tmGet() {
	gettimeofday(&tvNow,NULL);
	return (tmNow=TM_HZ*(tvNow.tv_sec-tsStarted) + (TM_HZ*tvNow.tv_usec)/1000000);
#if TM_HZ>2147
#error when using 32 bit signed int TM_HZ can not be greater than 2147
#endif
}

double gettime() {
	gettimeofday(&tvNow,NULL);
	return ( tvNow.tv_sec + tvNow.tv_usec/1000000.0);
}

/**
	removes '\r\n' and all spaces and tabs from the end of string str
	@param str: input/output string
	@return final string length (without terminating \0) */
int RemoveNL(char *str)
{
	int iLength;
	for(iLength = strlen(str)-1; iLength >=0; iLength --) {
		//DMSG("RmNL LineLength:>%i<", iLength);
		if (isspace(str[iLength])) {
			str[iLength] = '\0';
		}
		else
			break;
	}
	return iLength+1;
}


/**
	sending data over tcp/ip
	@param fd       - file descriptor for connection
	@param msg      - pointer to message to be written
	@param size     - size of message
	@return result of sendto */
int WriteNet(int fd, void * msg, int size) {
	return sendto(fd, msg, size, 0, (struct sockaddr*)&toaddr, sizeof(toaddr));
}

/**
	reading data over tcp/ip
	@param fd       - file descriptor for connection
	@param msg      - pointer to message to be read
	@param size     - size of message
	@return result of recvfrom */
int ReadNet(int fd, void * msg, int size) {
	char recvbuf[MAX_MSG_SIZE];
	int ret;
	unsigned int recvaddrsize = sizeof(recvaddr);

	// setting recvaddr is required for non-connected sockets
	memcpy(&recvaddr, &fromaddr, sizeof(recvaddr));
	ret = recvfrom(fd, recvbuf, MAX_MSG_SIZE, 0, (struct sockaddr*)& recvaddr, &recvaddrsize);
//  DMSG("NET: %d", ret);
	if(ret>size)
		ret = size;
	if(ret>0)
		memcpy(msg, recvbuf, size);
	return ret;
}

void show_usage(void) {
	printf("can-proxy options:\n"
	"-i	: intercative (send CAN packets typed by user)\n"
	"-o file.log : log all traffic to a file\n"
	"-p port : send CAN packets arriving at UDP port\n"
	"-c	: use select() for CAN (driver can-0.7.1-pi3.4 or newer)\n"
	"-q	: quiet\n"
	"-h	: this help\n"
	"UDP arriving at specified port causes can-proxy to forward all CAN\n"
	"traffic back to the sender, establishing bi-directional communication.\n");
}

/**
	handling command line options, calling functions, main program loop 
	@param argc, char * argv[]	- command line options
	@return  0					- OK
			-1					- ERROR */
int main(int argc, char * argv[]) {
	struct timeval tvTimeoutTmp;
	int fdSelectMax;
//	int ret;
	int opt;
	fd_set readsel;
	canmsg_t canmsg;

	myport = DEFAULT_CAN_SERVER_PORT;
	toport = DEFAULT_CAN_CLIENT_PORT;
	fdNet = fdCanDev = fdIn = -1;
	fLog = NULL;
	
	iBusFlags = 0;
	time0 = gettime();
	tsStarted = tvNow.tv_sec;
	tvSelectTimeout.tv_sec = 0;
	tvSelectTimeout.tv_usec = 500000; // wake up every 0.5 s even without data

	while((opt=getopt(argc, argv, "io:p:ch"))>0) {
		switch(opt) {
		case 'i': // interactive or stdin
			fdIn = 0;
			break;
		case 'o': // log file
			fLog = fopen(optarg,"a");
			break;
		case 'p':
			sscanf(optarg,"%hi", &myport);
			break;
		case 'c':
			fdCanDev = open(optarg,O_RDWR/*|O_NONBLOCK - select supported*/);
			use_select++;
			break;
		case 'q':
			quiet ++;
			break;
		case 'h':
			show_usage();
			break;
		default:
			break;
		}
	}

	if(!quiet)
		fprintf(stderr, "can-proxy v0.7.1-pi3.5 (C) 2002 BFAD GmbH http://www.getembedded.de/ (GPL) \n");

	
	if(fdCanDev<0)
		fdCanDev = open(CAN_DEV,O_RDWR|O_NONBLOCK);
	fdNet = socket(AF_INET,SOCK_DGRAM,0);
	memset(&myaddr, 0, sizeof(myaddr));
	memset(&fromaddr, 0, sizeof(fromaddr));
	memset(&toaddr, 0, sizeof(toaddr));

	memset(& canmsg, 0, sizeof(canmsg));
	myaddr.sin_family=AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(myport) ;

	toaddr.sin_family=AF_INET;
	toaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);;
	toaddr.sin_port = htons(toport);

	bind(fdNet,(struct sockaddr*) &myaddr,sizeof(myaddr));

	if(fdCanDev< 0 ) {
		perror(CAN_DEV);
		ERR("1.CAN-PROXY main fdCanDev not valid:%d, exiting", fdCanDev);
		exit(1);
	}

	while(1) {
		tvTimeoutTmp = tvSelectTimeout;
		fdSelectMax=0;
		FD_ZERO(&readsel);
		if(fdCanDev>=0){
			FD_SET(fdCanDev, &readsel);
			if(fdCanDev+1>fdSelectMax)
				fdSelectMax = fdCanDev+1;
		}
		if(fdNet>=0) {
			 FD_SET(fdNet, &readsel);
			 if(fdNet+1>fdSelectMax)
				 fdSelectMax = fdNet+1;
		}
		if(fdIn>=0) {
			 FD_SET(fdIn, &readsel);
			 if(fdIn+1>fdSelectMax)
				 fdSelectMax = fdIn+1;
		}
		select(fdSelectMax, &readsel, NULL, NULL, &tvTimeoutTmp);
		if(fdCanDev>=0 && FD_ISSET(fdCanDev, &readsel)) {
			if(ReceiveCanMsg(&canmsg) == 0) {
				WriteNet(fdNet, &canmsg, sizeof(canmsg));
				continue; // reading CAN has priority
			}
		}
		if(gettime()-time0 < SleepUntil) {
			continue;
		}
		if(fdNet>=0 && FD_ISSET(fdNet,&readsel)) {
			ReadNet(fdNet, &canmsg, sizeof(canmsg));
			// TODO: this will work for a single client
			// multiple clients should probably use broadcast
			//  for now we just reply to the most recent address
			toaddr = recvaddr;
			SendCanMsg(&canmsg);
		}
		if(fdIn>=0 && FD_ISSET(fdIn, &readsel)) {
			ReadInput(fdIn);
		}
		if(!use_select)
			usleep(20000);
	}
	return 0;       
}

/**
	logging messages to/from CAN
	@param dir	- direction
	@param fout - file descriptor for output
	@param msg	- CanMsgPerma
	@return  0					- OK
			-1					- ERROR */
int LogMsg(char * dir, FILE *fout, canmsg_t * msg) {
	int i;
	double t;

	t = gettime()-time0;
	if(msg->length < 0)
		msg->length = 0;
	if(msg->length > 8)
		msg->length = 8;

	fprintf(stdout,"%s %8.3f  id=%08lx n=%d", 
		dir, gettime()-time0, msg->id, msg->length);
	for(i=0; i< msg->length; i++) {
		fprintf(stdout, " %02x",  msg->data[i]);
	}
	fprintf(stdout, "\n");
	fflush(stdout);

	if(!fout) 
		return 0;

	fprintf(fout,"%s %8.3f  id=%08lx n=%d", 
		dir, gettime()-time0, msg->id, msg->length);
	for(i=0; i< msg->length; i++) {
		fprintf(fout, " %02x",  msg->data[i]);
	}
	fprintf(fout, "\n");
	fflush(fout);

	return 0;
}


/** PERMA CAN utility functions */

/**
	sending messages to CAN
	@param msg	- CanMsgPerma
	@return  0					- OK
			-1					- ERROR */
int SendCanMsg(canmsg_t * msg) {
	int ret;

	msg->flags = MSG_EXT;
	msg->cob = 0;
	tmGet();
	ret=write(fdCanDev, msg, CAN_MSG_SIZE);
	LogMsg("SEND", fLog, msg);
	tmLastSentReceived = tmNow;
	if( ret != CAN_MSG_SIZE) {
		perror("sending to CAN");
//TODO: send to error_ico
		return ret;
	}
	return 0;
}
	
/**
	receiving messages from CAN
	@param msg	- CanMsgPerma
	@return  0					- OK
			-1					- ERROR */
int ReceiveCanMsg(canmsg_t * msg) {
	int ret;
//	double t;
//	fdCanDev = open(CAN_DEV,O_RDWR | O_NONBLOCK);

	msg->flags = 0;
	msg->cob = 0;
	ret = read(fdCanDev,msg, CAN_MSG_SIZE);
	if(ret == CAN_MSG_SIZE) {	
		LogMsg("RECV", fLog, msg);
		tmLastSentReceived = tmNow;
		return 0;
	}
	// we can receive 0 here
	if(ret == 0 || (ret == -1 && errno == EAGAIN)) {
		return -EAGAIN;
	}
	DMSG("received %d bytes",ret);
	return ret;
}

/** @name GetNumber
	aliasing: deassign
	@param buf	- string buffer
	@param val	- alias value
	@return  0					- OK
			-1					- ERROR */
int GetNumber(char * buf, int * val) {
	if(!buf) {
		return -1;
	}
	if(sscanf(buf,"%i", val)==1) {
		return 0;
	}
	return -1;
}

/** @name BuildCanMsg
	building the can message from buf string to msg
	@param buf	- 
	@param msg	- CanMsgPerma
	@return  0					- OK
			-1					- ERROR */
int BuildCanMsg(char * buf, canmsg_t *msg) {
	int val;
	int i;

	buf = strtok(buf, " \t");

	val = msg->id;
	buf = strtok(NULL, " \t");
	GetNumber(buf, &val);
	msg->id = val;

	val = msg->length;
	buf = strtok(NULL, " \t");
	GetNumber(buf, &val);
	msg->length = val;

	for(i=0;(buf = strtok(NULL, " \t")); i++) {
		val = msg->data[i];
		GetNumber(buf, &val);
		msg->data[i] = val;
	}
	return 0;
}


#define LINE_L 66000
char buf[LINE_L];

extern int ReadCommand(char *buf);


/**
	processing a script or from stdin
	@param fd	- file descriptor
	@return  0					- OK
			-1					- ERROR */
int ReadInput(int fd) {
	int ret;
	int i,j;
	ret = read(fd, buf, LINE_L);
	for(i=0; i< ret; ) {
		for(j=i; (j<ret) && (buf[j] != '\n'); j++); // find NL
		if(j<ret) {
			buf[j] = '\0';
			ReadCommand(buf+i);
			i=j+1;
		} else {
			fprintf(stderr, "too big input file\n");
			i=j;
		}
	}
	return 0;
}

/**
	reading and handling commands from buf string
	@param buf	- string buffer, 1.char tells about what is to be done:	
								 w - write 
								 s - sleep
								 a - assign (simple aliasing mechanism)
								 r - reset timer
								 l - lock the bus
								 q - quit application
	@return  0					- OK
			-1					- ERROR */
int ReadCommand(char *buf) {
	static int usSleep=20000;
	char * ptr;
	static canmsg_t msg;

	buf[LINE_L-1] = '\0';
	if(RemoveNL(buf) == 0)
		return 0; // empty line

	switch(tolower(buf[0])) {
	case 'q':
		DMSG("1.CAN-PROXY ReadCommand received QUIT command, exiting");
		exit(0);
		break;
	case 'r':
		time0 = gettime();
		break;
	case 't':
		printf("TIME %8.3f\n", gettime()-time0);
		break;
	case 's':
		strtok(buf, " \t");
		ptr = strtok(NULL, " \t");
		if(ptr)
			sscanf(ptr,"%i", &usSleep);
		SleepUntil = gettime()-time0 + usSleep/1000000.0;
		break;
	case 'w':
		if(BuildCanMsg(buf, &msg) == 0) {
			SendCanMsg(&msg);
		} else {
			fprintf(stderr,"wrong message syntax: %s\n", buf);
		}
		break;
	}
	return 0;
}

