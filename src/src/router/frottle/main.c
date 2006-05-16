/***************************************************************************
    main.c     -  Main entry point and iptables IPQ handling
                             -------------------
    begin                : Sun May 4 2003
    copyright            : (C) 2003 by Chris King
    email                : frottle@wafreenet.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "main.h"

// Max packet message size
// I assume this should be sizeof(struct ipq_packet_msg) + MTU ?
#define MAXIPQMSG	2048

// Packet queues themselves
BYTE	*PacketQueue[PacketQueues] = {NULL, NULL, NULL};
int	PacketQueueHead[PacketQueues] = {0, 0, 0}, PacketQueueTail[PacketQueues] = {0, 0, 0};
pthread_mutex_t	PacketQueueMutex;

// Configuration (see main.h)
CONFIG	Config;

// Client -> server status flags (see main.h)
BYTE	ClientStatus = 0;

// IPQ handle
struct ipq_handle *hIpq;

// Structure and variable to track connections
#define MaxConTrack	100
#define CT_FlagValid	0x01
typedef struct _CONTRACK{
	struct timeval StartTime;
	struct timeval LastTime;
	unsigned short SourcePort;
	char SourceIp[20];
	float Data;
	BYTE Flags;
} CONTRACK, *LCONTRACK;
CONTRACK	ConTrack[MaxConTrack];

// Forward declarations of local functions
void SignalHandler(int SigNum);
int ReadConfig(int argc, char *argv[]);
int BackgroundProcessing(void);
int PacketPriority(ipq_packet_msg_t *pPacketMsg);
int ProcessPacketMessage(ipq_packet_msg_t *pPacketMsg);
int Daemon(void);

// Local variables
BYTE ExitMain = FALSE;
pthread_t idMasterThread = 0;
BYTE ExitMasterThread = 0;
pthread_t idControlThread = 0;
BYTE ExitControlThread = 0;


// Main entry point to application
int main(int argc, char *argv[])
{
	BYTE MsgBuff[MAXIPQMSG];
	struct sigaction SigAction;
	int i;

	// Read config & parameters
	if(!ReadConfig(argc, argv))
		return EXIT_FAILURE;

	// If we should act as neither a master nor a client (config error)
	if(!(Config.Flags & CONFIGFLAGS_MASTER) && !(Config.Flags & CONFIGFLAGS_CLIENT)){
		printf(APPTITLE": not configured to run as a master nor client\n");
		return EXIT_FAILURE;
	}

	// If we're to be a client, check we have an IP
	if(Config.Flags & CONFIGFLAGS_CLIENT && Config.MasterIp[0] == 0x00){
		printf(APPTITLE": Can not run as a client, no master IP specified 0x%02X\n", Config.Flags);
		return EXIT_FAILURE;
	}

	// Let the user know we're thinking of them
	if(Config.Flags & CONFIGFLAGS_MASTER)
		Log(LOG_NOTICE, "Acting as a master on port %d", Config.MasterPort);
	if(Config.Flags & CONFIGFLAGS_SELFCLIENT)
		Log(LOG_NOTICE, "Acting as a self-client with %d packet queues", Config.Queue);
	if(Config.Flags & CONFIGFLAGS_CLIENT)
		Log(LOG_NOTICE, "Acting as a client to %s:%d with %d packet queues", Config.MasterIp, Config.MasterPort, Config.Queue);
	else
		Log(LOG_NOTICE, "Acting stupid");

	// If we should daemonise
	if(Config.Flags & CONFIGFLAGS_DAEMON)
		// Daemonise
		Daemon();

	// If we should act as a master
	if(Config.Flags & CONFIGFLAGS_MASTER){

		// If we should act as _only_ a master
		if(!(Config.Flags & CONFIGFLAGS_CLIENT) && !(Config.Flags & CONFIGFLAGS_SELFCLIENT)){

			// Jump to main function in master.c which will perform this function
			MasterRun(NULL);

			return EXIT_SUCCESS;
		}
		// If we should act as a client as well
		else{

			// Start main thread in master.c which will perform this function
			pthread_create(&idMasterThread, NULL, &MasterRun, NULL);
		}
	}

	// Create an IPQ handle
	hIpq = ipq_create_handle(0, PF_INET);
	if(hIpq == NULL){

		// If we're running a master thread
		if(idMasterThread){

			// Set flag & wait for master thread to exit
			ExitMasterThread = 1;
			pthread_join(idMasterThread, NULL);
		}

		// Log it
		Log(LOG_ERR, "Failed to initialise IPQ (%s)", ipq_errstr());

		return EXIT_FAILURE;
	}

	// Set mode. Note: We set to packet mode so that we get to see the
	// number of bytes in the payload, the payload itself is ignored
	if (ipq_set_mode(hIpq, IPQ_COPY_PACKET, 0) == -1){

		// If we're running a master thread
		if(idMasterThread){

			// Set flag & wait for master thread to exit
			ExitMasterThread = 1;
			pthread_join(idMasterThread, NULL);
		}

		// Release IPQ
		ipq_destroy_handle(hIpq);

		// Tell the user
		Log(LOG_ERR, "Failed to configure IPQ (%s)", ipq_errstr());

		return EXIT_FAILURE;
	}

	// Allocate Packet queue memory
	for(i = 0; i < PacketQueues; i++)
		PacketQueue[i] = malloc(Config.Queue * sizeof(struct ipq_packet_msg));

	// Check allocations worked
	if(!PacketQueue){

		// Free anything that was allocated
		if(PacketQueue)
			free(PacketQueue);

		// If we're running a master thread
		if(idMasterThread){

			// Set flag & wait for master thread to exit
			ExitMasterThread = 1;
			pthread_join(idMasterThread, NULL);
		}

		// Release IPQ
		ipq_destroy_handle(hIpq);

		// Tell the user
		Log(LOG_ERR, "Failed to allocate sufficant memory");

		return EXIT_FAILURE;
	}

	// Inititalise the PacketQueue mutex
	pthread_mutex_init(&PacketQueueMutex, NULL);

	// If we're acting as a normal client
	if(Config.Flags & CONFIGFLAGS_CLIENT)
		// Start control thread
		pthread_create(&idControlThread, NULL, &ControlRun, NULL);

	// Install SIGTERM/etc handler to get out of 'while(!ExitMain)'
	memset(&SigAction, 0, sizeof(SigAction));
	SigAction.sa_handler = SignalHandler;
	sigaction(SIGINT, &SigAction, NULL);
	sigaction(SIGQUIT, &SigAction, NULL);
	sigaction(SIGTERM, &SigAction, NULL);
	// Install SIGHUP handler to reread config
	sigaction(SIGHUP, &SigAction, NULL);

	// Tell user
	Log(LOG_INFO, "Client up and running");

	// Enter main loop
	while(!ExitMain){

		// Wait up to 100mS to recieve a message
		int MsgSize = ipq_read(hIpq, MsgBuff, MAXIPQMSG, 100);

		// Error getting message
		if(MsgSize == -1){

			// TODO: This has started to happen occasionally. Don't know why.
			// ipq_errstr() = "Failed to receive netlink message"

			// Tell the user
			Log(LOG_ERR, "Error reading message from IPQ (%s)", ipq_errstr());
		}

		// Timeout getting message
		else if(MsgSize == 0){

			// Do background processing
			BackgroundProcessing();
		}

		// Got a message
		else{

			// Switch on message type
			switch (ipq_message_type(MsgBuff)){

			case NLMSG_ERROR:

				// Apparently we should call this (to clear the error?)
				ipq_get_msgerr(MsgBuff);

				// Tell the user
				Log(LOG_ERR, "Error reading message type from IPQ (%s)", ipq_errstr());
				break;

			case IPQM_PACKET:

				// Call a function to process the packet
				ProcessPacketMessage(ipq_get_packet(MsgBuff));

				// Do background processing
				BackgroundProcessing();
				break;

			default:

				// Tell the user
				Log(LOG_WARNING, "Undefined message type from IPQ");
				break;
			}
		}
	}

	// If we're running a control thread
	if(idControlThread){

		// Set flag & wait for control thread to exit
		ExitControlThread = 1;
		pthread_join(idControlThread, NULL);
	}

	// If we're running a master thread
	if(idMasterThread){

		// Set flag & wait for master thread to exit
		ExitMasterThread = 1;
		pthread_join(idMasterThread, NULL);
	}

	// Release IPQ
	ipq_destroy_handle(hIpq);

	// Release Packet queue memory
	for(i = 0; i < PacketQueues; i++)
		free(PacketQueue[i]);

	// Let the user know we're thinking of them
	Log(LOG_INFO, "Exited cleanly");

	return EXIT_SUCCESS;
}

// Signal handler, mainly for SIGTERM
void SignalHandler(int SigNum){

	if(SigNum == SIGINT || SigNum == SIGTERM || SigNum == SIGQUIT){

		// Exit the app
		ExitMain = TRUE;
	}

	else if(SigNum == SIGHUP){

		// Re-read config file
		ReadConfig(0, NULL);
	}

	return;
}

// Setup the configuration, returning 0 exits the app
int ReadConfig(int argc, char *argv[]){

	FILE	*hFile;
	char	LineBuf[128], Arg[32], Val[128];

	// If we are being passed a command line to process
	// (ie we are setting parameters for the first time)
	if(argc > 0){

		int NextOption;

		// Command line parameteres
		const char * const ShortOptions = "hvc:Vd";
		const struct option LongOptions[] = {
			{"help", 0, NULL, 'h'},
			{"version", 0, NULL, 'v'},
			{"conf", 1, NULL, 'c'},
			{"verbose", 0, NULL, 'V'},
			{"deamon", 0, NULL, 'd'},
			{NULL, 0, NULL, 0} };

		// Set config to default
		Config.Flags = 0;
		Config.MasterPort = 999;
		Config.Queue = 100;
		Config.Timeout = 100;
		Config.PollParams[0] = 60000;
		Config.PollParams[1] = 10;
		Config.PollParams[2] = 6000;
		Config.PollParams[3] = 7;
		Config.PollParams[4] = 5000;
		Config.PollParams[5] = 5;
		Config.PollParams[6] = 4000;
		strcpy(Config.winterface, "eth1");
		Config.HiPorts[0] = 22;
		Config.HiPorts[1] = 53;
		Config.HiPortsCnt = 2;
		strcpy(Config.ConfFilename, "/etc/frottle.conf");
		strcpy(Config.InfoFilename, "/tmp/frottle.html");
		Config.StatsFilename[0] = 0x00;

		// Loop through each option
		while((NextOption = getopt_long(argc, argv, ShortOptions, LongOptions, NULL)) != -1){

			switch(NextOption){

				case 'h':
					// Show some basic command line help
					printf("Usage: frottle <args>\n");
					printf("\t-h --help\tPrint this help\n");
					printf("\t-v --version\tPrint the version\n");
					printf("\t-c --conf\tfrottle.conf file path\\name\n");
					printf("\t-V --verbose\tVerbose messages\n");
					printf("\t-d --daemon\tRun as a daemon\n");
					return 0;
					break;

				case 'v':
					// Show version and exit
					printf("Frottle version %s\n", FROTVERSION);
					return 0;
					break;

				case 'c':
					strncpy(Config.ConfFilename, optarg, sizeof(Config.ConfFilename) - 1);
					Config.ConfFilename[sizeof(Config.ConfFilename) - 1] = 0x00;
					break;

				case 'V':
					Config.Flags |= CONFIGFLAGS_VERBOSE;
					break;

				case 'd':
					Config.Flags |= CONFIGFLAGS_DAEMON;
					break;

				default:
					Log(LOG_ERR, "Unknown command line parameter");
					break;
			}
		}
	}

	// Let the user know
	if(argc == 0)
		Log(LOG_INFO, "Rereading frottle.conf");

	// Open config file
	hFile = fopen(Config.ConfFilename, "r");
	if (!hFile)
		return 1;

	// Loop for each line in the file
	while (fgets(LineBuf, sizeof(LineBuf), hFile)){

		// Skip blank/comment lines
		if (!LineBuf[0] || LineBuf[0] == '#')
			continue;

		// Get arg & value
		Arg[0] = 0x00;
		Val[0] = 0x00;
		if(sscanf(LineBuf, "%31s %127s", Arg, Val) == 2){

			// If master mode
			if(strcasecmp(Arg, "mastermode") == 0 && argc != 0){
				if(atoi(Val))	Config.Flags |= CONFIGFLAGS_MASTER;
				else			Config.Flags &= ~CONFIGFLAGS_MASTER;
			}

			// If client mode
			else if(strcasecmp(Arg, "clientmode") == 0 && argc != 0){
				if(atoi(Val))	Config.Flags |= CONFIGFLAGS_CLIENT;
				else			Config.Flags &= ~CONFIGFLAGS_CLIENT;
			}

			// If self-client mode
			else if(strcasecmp(Arg, "selfclient") == 0 && argc != 0){
				if(atoi(Val))	Config.Flags |= CONFIGFLAGS_SELFCLIENT;
				else			Config.Flags &= ~CONFIGFLAGS_SELFCLIENT;
			}

			// Get master IP string
			else if(strcasecmp(Arg, "masterip") == 0 && argc != 0){
				strncpy(Config.MasterIp, Val, sizeof(Config.MasterIp));
				Config.MasterIp[sizeof(Config.MasterIp) - 1] = 0x00;
			}

			// Master port
			else if(strcasecmp(Arg, "masterport") == 0 && argc != 0){
				Config.MasterPort = (short)atoi(Val);
			}

			// Timeout
			else if(strcasecmp(Arg, "timeout") == 0){
				Config.Timeout = (short)atoi(Val);
			}

			// Polling parameters
			else if(strcasecmp(Arg, "pollparams") == 0){
				// Scan for 7 parameters
				int i, Params[7];
				i = sscanf(Val, "%d,%d,%d,%d,%d,%d,%d",
					&Params[0], &Params[1], &Params[2], &Params[3],
					&Params[4], &Params[5], &Params[6]);
				// If we got 7, use them
				if(i == 7){
					for(i = 0; i < 7; i++)
						Config.PollParams[i] = Params[i];
				}
			}

			// Queue size
			else if(strcasecmp(Arg, "queuesize") == 0 && argc != 0){
				Config.Queue = (short)atoi(Val);
			}

			// High priority ports
			else if(strcasecmp(Arg, "hiports") == 0){
				Config.HiPortsCnt = sscanf(Val, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
					&Config.HiPorts[0], &Config.HiPorts[1], &Config.HiPorts[2],
					&Config.HiPorts[3], &Config.HiPorts[4], &Config.HiPorts[5],
					&Config.HiPorts[6], &Config.HiPorts[7], &Config.HiPorts[8],
					&Config.HiPorts[9]);
			}

			// Daemonise
			else if(strcasecmp(Arg, "daemon") == 0 && argc != 0){
				if(atoi(Val))	Config.Flags |= CONFIGFLAGS_DAEMON;
				else			Config.Flags &= ~CONFIGFLAGS_DAEMON;
			}

			// Verbose
			else if(strcasecmp(Arg, "verbose") == 0){
				if(atoi(Val))	Config.Flags |= CONFIGFLAGS_VERBOSE;
				else			Config.Flags &= ~CONFIGFLAGS_VERBOSE;
			}

			// Wireless interface
			else if(strcasecmp(Arg, "winterface") == 0){
				strncpy(Config.winterface, Val, sizeof(Config.winterface));
				Config.winterface[sizeof(Config.winterface) - 1] = 0x00;
			}

			// Info file
			else if(strcasecmp(Arg, "infofile") == 0){
				strncpy(Config.InfoFilename, Val, sizeof(Config.InfoFilename));
				Config.InfoFilename[sizeof(Config.InfoFilename) - 1] = 0x00;
			}
			// Stats file
			else if(strcasecmp(Arg, "statsfile") == 0){
				strncpy(Config.StatsFilename, Val, sizeof(Config.StatsFilename));
				Config.StatsFilename[sizeof(Config.StatsFilename) - 1] = 0x00;
			}
		}
	}

	// Close the file
	fclose(hFile);
	
	// Do sanity checks
	if(Config.Flags & CONFIGFLAGS_SELFCLIENT){
		if(Config.Flags & CONFIGFLAGS_MASTER)
			Config.Flags &= ~CONFIGFLAGS_CLIENT;
		else
			Config.Flags &= ~CONFIGFLAGS_SELFCLIENT;
	}
	if(Config.Flags & CONFIGFLAGS_DAEMON)
		Config.Flags &= ~CONFIGFLAGS_VERBOSE;

	return 1;
}

// This is called randomly, atleast once every 100mS
int BackgroundProcessing(void){

	long PacketAge;
	int Queue, i;
	
	// Get current time
	struct timeval NowTime;
	gettimeofday(&NowTime, NULL);

	// Wait for access to the packet queue
	pthread_mutex_lock(&PacketQueueMutex);

	for(Queue = 0; Queue < PacketQueues; Queue++){

		// Loop
		while(PacketQueueHead[Queue] != PacketQueueTail[Queue]){

			// Work out age in mS
			PacketAge = (NowTime.tv_sec - ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->timestamp_sec) * 1000;
			if(NowTime.tv_usec < ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->timestamp_usec)
				 PacketAge += (1000000 - ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->timestamp_usec + NowTime.tv_usec) / 1000;
			else
				 PacketAge += (NowTime.tv_usec - ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->timestamp_usec) / 1000;

			// If the packet is old
			if(PacketAge > STALETIME){

				// Drop the packet
				ipq_set_verdict(hIpq, ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->packet_id, NF_DROP, 0, NULL);

				// Move the Tail offset
				PacketQueueTail[Queue]++;
				if(PacketQueueTail[Queue] >= Config.Queue)
					PacketQueueTail[Queue] = 0;
			}
			else{
				// As the queue should be chronological, we can assume if this
				// packet is not too old, later ones won't be either
				break;
			}
		}
	}

	// Release access to the packet queue
	pthread_mutex_unlock(&PacketQueueMutex);

	// Delete old connections from contrack
	for(i = 0; i < MaxConTrack; i++){

		// If more than 30s since last packet
		if(Age(ConTrack[i].LastTime) > 30000 && ConTrack[i].Flags & CT_FlagValid){
			// Clear the valid flag
			ConTrack[i].Flags &= ~CT_FlagValid;
		}
	}

	return 1;
}

// This is called to work out a packets priority
int PacketPriority(ipq_packet_msg_t *pPacketMsg){

#define PriHigh	0
#define PriDef	1
#define PriLow	2

	int i;
	unsigned char Protocol;
	char SourceIp[20], DestIp[20];
	unsigned short SourcePort, DestPort;

	// Store the protocol
	Protocol = ((struct iphdr *)(pPacketMsg->payload))->protocol;
	
	// If it is ICMP it is high priority automatically
	if(Protocol == IPPROTO_ICMP){

		return PriHigh;
	}

	// If TCP/UDP protocol
	else if(Protocol == IPPROTO_TCP || Protocol == IPPROTO_UDP){

		// If TCP protocol
		if(Protocol == IPPROTO_TCP){

			// Get pointer to TCP header
			struct tcphdr *pTcpHdr;
			pTcpHdr = (struct tcphdr *) (pPacketMsg->payload + sizeof(struct iphdr));

			// Store the source/dest ports
			SourcePort = ntohs(pTcpHdr->source);
			DestPort = ntohs(pTcpHdr->dest);
		}

		// else UDP protocol
		else{

			// Get pointer to UDP header
			struct udphdr *pUdpHdr;
			pUdpHdr = (struct udphdr *) (pPacketMsg->payload + sizeof(struct iphdr));

			// Store the source/dest ports
			SourcePort = ntohs(pUdpHdr->source);
			DestPort = ntohs(pUdpHdr->dest);
		}

		// Look for source or dest port being high priority
		for(i = 0; i < Config.HiPortsCnt; i++){

			if(SourcePort == Config.HiPorts[i] || DestPort == Config.HiPorts[i]){

				return PriHigh;
			}
		}

		// Store the source/dest IP
		strcpy(SourceIp, inet_ntoa(*(struct in_addr *) &((struct iphdr *)(pPacketMsg->payload))->saddr));
		strcpy(DestIp, inet_ntoa(*(struct in_addr *) &((struct iphdr *)(pPacketMsg->payload))->daddr));

		// Look for source or dest port being high priority
		for(i = 0; i < MaxConTrack; i++){

			// If this slot has valid data
			if(ConTrack[i].Flags & CT_FlagValid){

				// If this connection matches
				if(ConTrack[i].SourcePort == SourcePort &&
					strcmp(ConTrack[i].SourceIp, SourceIp) == 0){

					// Update info
					gettimeofday(&ConTrack[i].LastTime, NULL);
					ConTrack[i].Data += (float)pPacketMsg->data_len;

					// If done less than ~2 MB
					if(ConTrack[i].Data < 2000000){

						return PriDef;
					}

					// else has done significant data
					else{

						// If doing more than 5 kB/s
						if((ConTrack[i].Data / max((Age(ConTrack[i].StartTime) / 1000), 1)) > 5120){

							return PriLow;
						}
					}

					return PriDef;
				}
			}
		}

		// Look for an empty slot to use to track this connection
		for(i = 0; i < MaxConTrack; i++){

			if(!(ConTrack[i].Flags & CT_FlagValid)){

				// Set initial info
				gettimeofday(&ConTrack[i].StartTime, NULL);
				gettimeofday(&ConTrack[i].LastTime, NULL);
				ConTrack[i].SourcePort = SourcePort;
				strcpy(ConTrack[i].SourceIp, SourceIp);
				ConTrack[i].Data = (float)pPacketMsg->data_len;
				ConTrack[i].Flags = CT_FlagValid;

				break;
			}
		}
	}
	/* Non-ICMP/TCP/UDP protocol
	else{

	}
	*/

	// Retun the default
	return PriDef;
}

// This is called when a packet message is received
int ProcessPacketMessage(ipq_packet_msg_t *pPacketMsg){

	struct timeval NowTime;
	int Queue;

	// If we're acting as a client
	if(Config.Flags & CONFIGFLAGS_CLIENT){

		// If we don't have a server connection
		if(ClientStatus == CS_NOSERVER){

			// Just let the packet straight through (fail safe)
			ipq_set_verdict(hIpq, pPacketMsg->packet_id, NF_ACCEPT, 0, NULL);
			return 1;
		}
	}

	// If we're acting as a self client
	if(Config.Flags & CONFIGFLAGS_SELFCLIENT){

		// If we don't have any clients
		if(ClientCount == 0){

			// Just let the packet straight through (fail safe)
			ipq_set_verdict(hIpq, pPacketMsg->packet_id, NF_ACCEPT, 0, NULL);
			return 1;
		}
	}

	// Note: The timestamp_sec value in struct ipq_packet_msg always appears to be 0,
	// so we will insert our own time stamp for when we got it (not as good, but oh well).

	// Get current time
	gettimeofday(&NowTime, NULL);
	pPacketMsg->timestamp_sec = NowTime.tv_sec;
	pPacketMsg->timestamp_usec = NowTime.tv_usec;

	// Workout queue that packet should go in
	Queue = PacketPriority(pPacketMsg);

	// Wait for access to the packet queue
	pthread_mutex_lock(&PacketQueueMutex);

	// Store packet meta data (including payload size, but not payload
	// itself!) at the head of the queue
	memcpy(&PacketQueue[Queue][PacketQueueHead[Queue] * sizeof(struct ipq_packet_msg)], pPacketMsg, sizeof(struct ipq_packet_msg));

	// Move the head offset
	PacketQueueHead[Queue]++;
	if(PacketQueueHead[Queue] >= Config.Queue)
		PacketQueueHead[Queue] = 0;

	// If we just looped around
	if(PacketQueueHead[Queue] == PacketQueueTail[Queue]){

		// Drop the oldest (overwritten) packet
		ipq_set_verdict(hIpq, ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->packet_id, NF_DROP, 0, NULL);

		// Move the Tail offset
		PacketQueueTail[Queue]++;
		if(PacketQueueTail[Queue] >= Config.Queue)
			PacketQueueTail[Queue] = 0;
	}

	// Release access to the packet queue
	pthread_mutex_unlock(&PacketQueueMutex);

	return 1;
}

// Handle log messages.
// Send them to /var/log/messages with appropriate level
// If verbose also print to stdout (console)
void Log(int Level, char *Format, ...){

	va_list ArgList;
    char Buff[256];

	va_start(ArgList, Format);
	vsnprintf(Buff, sizeof(Buff)-2, Format, ArgList);
	va_end(ArgList);

    syslog(Level, Buff);

	if(Config.Flags & CONFIGFLAGS_VERBOSE && !(Config.Flags & CONFIGFLAGS_DAEMON))
		printf(APPTITLE": %s\n", Buff);

	return;
}

// Work out the age in mS of the time value
int Age(struct timeval  OldTime){

	struct timeval  NowTime;
	int AgeVal;

	gettimeofday(&NowTime, NULL);

	// Work out age in mS of the timeval
	AgeVal = (NowTime.tv_sec - OldTime.tv_sec) * 1000;
	if(NowTime.tv_usec < OldTime.tv_usec){
		AgeVal -= 1000;
		AgeVal += ((1000000 + NowTime.tv_usec) - OldTime.tv_usec) / 1000;
	}
	else{
		AgeVal += (NowTime.tv_usec - OldTime.tv_usec) / 1000;
	}

	return AgeVal;
}

// Call this to daemonise a process
int Daemon(void){

	int	i, fd;

	// Check if already a daemon
	if (getppid() == 1)
		return -1;

	i = fork();
	if (i > 0)
		exit(0);

	// New session process group
	if (i < 0 || setsid() == -1)
		exit(1);

	if ((fd = open("/dev/null", O_RDWR, 0)) != -1){

		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);

		if (fd > 2)
			close(fd);
	}

	chdir("/");

	return 0;
}
