/***************************************************************************
    control.c  -  Register with and listen for control packets from the master
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

// If we have no polls for this many seconds we assume the server
// forgot about us and we must re-register
#define SERVERREREGISTER	10

// If we have no polls for this many seconds we assume the server
// has died (we keep trying to register, but allow packets straight
// through without queuing)
#define SERVERTIMEOUT		60

// Local variables
int hControlPort;
struct sockaddr_in LocalAddr, MasterAddr;
struct timeval	LastTxActivityTime;
struct timeval	LastRxActivityTime;
struct timeval	LastCtrlBackgroundTime;
BYTE CurrentRate;
char CurrentSignal, CurrentNoise;

typedef struct _CLIENTSTATS{
	float		Packets[PacketQueues];
	float		Bytes[PacketQueues];
	float		BytesAvg[PacketQueues];
	long		Polls;
} CLIENTSTATS, *LPCLIENTSTATS;
CLIENTSTATS	ClientStats;

// Forward declarations of local functions
void *ControlCleanup(void *pParam);
void SendRegisterPacket(void);
void GetWirelessState(void);


// This is the main entry point in to the control code
// and will be run in it's own thread
void *ControlRun(void *pParam){

	struct sockaddr_in FromAddr;
	socklen_t SockaddrLen;
	struct timeval	NowTime;
	BYTE PacketBuffer[max(sizeof(MASTERPACKET), sizeof(CLIENTPACKET)) + 2];
	int PacketSize, i, j;
	//pthread_t idWriteStatsFileThread;

	// Clear stats
	memset(&ClientStats, 0, sizeof(ClientStats));
	
	// Open UDP connection to server
	memset(&MasterAddr, 0, sizeof(MasterAddr));
	MasterAddr.sin_family = AF_INET;
	MasterAddr.sin_addr.s_addr = inet_addr(Config.MasterIp);
	MasterAddr.sin_port = htons(Config.MasterPort);

	hControlPort = socket(PF_INET, SOCK_DGRAM, 0);
	if(hControlPort < 0){
		Log(LOG_ERR, "Failed to open client socket");
		return NULL;
	}

	// Bind it to a local port
	memset(&LocalAddr, 0, sizeof(LocalAddr));
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	LocalAddr.sin_port = htons(0);

	if(bind(hControlPort, (struct sockaddr *)&LocalAddr, sizeof(LocalAddr)) != 0){
		close(hControlPort);
		Log(LOG_ERR, "Failed to bind to client port");
		return NULL;
	}

	// Get initial wireless data
	GetWirelessState();

	// Store intial background processing time
	gettimeofday(&LastCtrlBackgroundTime, NULL);
	LastCtrlBackgroundTime.tv_sec -= 8;

	// Set the initial status
	ClientStatus = CS_REGISTER;

	// Send register packet
	SendRegisterPacket();

	// Set an initial last Rx time
	gettimeofday(&LastRxActivityTime, NULL);

	// Setup cleanup function (closes the port)
	pthread_cleanup_push((void*)ControlCleanup, NULL);

	// While we should run (helps exit a thread cleanly)
	while(!ExitControlThread){

		struct timeval Timeout;
		fd_set readfds;
		int SelRet;

		FD_ZERO(&readfds);
		FD_SET(hControlPort, &readfds);

		// Wait 2 sec for UDP data
		Timeout.tv_sec = 2;
		Timeout.tv_usec = 0;
		SelRet = select(hControlPort+1, &readfds, NULL, NULL, &Timeout);

		// Error in select
		if(SelRet < 0){

			// Select error
			Log(LOG_ERR, "Error waiting for master packet");
		}
		// Data pending
		else if(SelRet > 0){

			// Get packet from master
			SockaddrLen = sizeof(FromAddr);
			PacketSize = recvfrom(hControlPort, PacketBuffer, sizeof(PacketBuffer), 0, (struct sockaddr *)&FromAddr, &SockaddrLen);

			// Check packet size
			if(PacketSize != sizeof(MASTERPACKET) + 2){

				// Wrong sized packet
				Log(LOG_ERR, "Client got packet of incorrect size");
			}
			else{

				// Check the checksum
				for(i = 0, j = 0; i < sizeof(MASTERPACKET); i++){
					j <<= 8;
					j += PacketBuffer[i];
				}
				if(PacketBuffer[i] != (USHORT)j){

					// Corrupted packet
					Log(LOG_ERR, "Client got corrupted packet");
				}
				else{

					int Packets, Bytes, Queue, k;

					if(ClientStatus != CS_CONNECTED)
						Log(LOG_INFO, "Client registered");

					// Update the status and last activity time
					ClientStatus = CS_CONNECTED;
					gettimeofday(&LastRxActivityTime, NULL);

					// Incriment poll count
					ClientStats.Polls++;

					// Send Packets up to allowed amount
					SendPackets(((MASTERPACKET*)PacketBuffer)->MaxPacketsToSend, ((MASTERPACKET*)PacketBuffer)->MaxBytesToSend);

					// Wait for access to the packet queue
					pthread_mutex_lock(&PacketQueueMutex);

					// Work out packets/bytes left in queues
					Packets = 0;
					Bytes = 0;
					for(Queue = 0; Queue < PacketQueues; Queue++){
						k = PacketQueueTail[Queue];
						while(PacketQueueHead[Queue] != k){
							Packets++;
							Bytes += ((ipq_packet_msg_t *)&PacketQueue[Queue][k * sizeof(struct ipq_packet_msg)])->data_len;
							k++;
							if(k >= Config.Queue)
								k = 0;
						}
					}

					// Release access to the packet queue
					pthread_mutex_unlock(&PacketQueueMutex);

					// Send 'done' packet
					memset(PacketBuffer, 0, sizeof(PacketBuffer));
					((CLIENTPACKET*)PacketBuffer)->TypeId = CLIENTID_END;
					((CLIENTPACKET*)PacketBuffer)->Rate = CurrentRate;
					((CLIENTPACKET*)PacketBuffer)->Signal = CurrentSignal;
					((CLIENTPACKET*)PacketBuffer)->Noise = CurrentNoise;
					((CLIENTPACKET*)PacketBuffer)->PacketsInQueue = Packets;
					((CLIENTPACKET*)PacketBuffer)->BytesInQueue = Bytes;
					for(i = 0, j = 0; i < sizeof(CLIENTPACKET); i++){
						j <<= 8;
						j += PacketBuffer[i];
					}
					PacketBuffer[i] = (USHORT)j;
					sendto(hControlPort, &PacketBuffer, sizeof(CLIENTPACKET)+2, 0, (struct sockaddr *)&MasterAddr, sizeof(MasterAddr));

					// Update the last activity time
					gettimeofday(&LastTxActivityTime, NULL);
				}
			}
		}

		// Do background processing here, this should run at least every 2 sec

		// Get current time
		gettimeofday(&NowTime, NULL);

		// If no Tx or Rx recently
		if(NowTime.tv_sec > LastRxActivityTime.tv_sec + SERVERREREGISTER &&
			NowTime.tv_sec > LastTxActivityTime.tv_sec + SERVERREREGISTER){

			// Reregister
			SendRegisterPacket();
		}

		// If no Rx for ages
		if(NowTime.tv_sec > LastRxActivityTime.tv_sec + SERVERTIMEOUT && ClientStatus != CS_NOSERVER){

			Log(LOG_INFO, "Client lost master connection");

			// Wait for access to the packet queue
			pthread_mutex_lock(&PacketQueueMutex);

			for(i = 0; i < PacketQueues; i++){

				// Send any/all packets in queue
				while(PacketQueueHead[i] != PacketQueueTail[i]){

					// Send the packet
					ipq_set_verdict(hIpq, ((ipq_packet_msg_t *)&PacketQueue[i][PacketQueueTail[i] * sizeof(struct ipq_packet_msg)])->packet_id, NF_ACCEPT, 0, NULL);

					PacketQueueTail[i]++;
					if(PacketQueueTail[i] >= Config.Queue)
						PacketQueueTail[i] = 0;
				}
			}

			// Release access to the packet queue
			pthread_mutex_unlock(&PacketQueueMutex);

			// Update the status to allow packets straight through
			ClientStatus = CS_NOSERVER;
		}

		// If no background processing for 10 sec (approx.)
		if(NowTime.tv_sec >= LastCtrlBackgroundTime.tv_sec + 10){

			// Update wireless data
			GetWirelessState();

			// Store intial background processing time
			gettimeofday(&LastCtrlBackgroundTime, NULL);

			if(Config.StatsFilename[0] != 0x00){
				// Start file write function in own threa
				//idWriteStatsFileThread = 0;
				//pthread_create(&idWriteStatsFileThread, NULL, &WriteStatsFile, NULL);
				// TODO: Figure out why doing it in a thread dies sooner or later (no further updates)
				WriteStatsFile(NULL);
			}

			//Reset poll count
			ClientStats.Polls = 0;
		}
	}

	// Call and clear cleanup function
	pthread_cleanup_pop(1);

	return NULL;
}

// In here we should do any cleanup, such as closing ports,
// that might be needed when we exit
void *ControlCleanup(void *pParam){

	// Close the port
	close(hControlPort);

	// Delete the stats file
	unlink(Config.StatsFilename);
	return NULL;
}

void SendRegisterPacket(void){

	BYTE PacketBuffer[sizeof(CLIENTPACKET) + 2];
	int i, j;

	Log(LOG_INFO, "Client sent registration");

	// Reset the status
	ClientStatus = CS_REGISTER;

	// Send hello packet
	memset(PacketBuffer, 0, sizeof(PacketBuffer));
	((CLIENTPACKET*)PacketBuffer)->TypeId = CLIENTID_REGISTER;
	((CLIENTPACKET*)PacketBuffer)->Rate = CurrentRate;
	((CLIENTPACKET*)PacketBuffer)->Signal = CurrentSignal;
	((CLIENTPACKET*)PacketBuffer)->Noise = CurrentNoise;
	for(i = 0, j = 0; i < sizeof(CLIENTPACKET); i++){
		j <<= 8;
		j += PacketBuffer[i];
	}
	PacketBuffer[i] = (USHORT)j;
	sendto(hControlPort, &PacketBuffer, sizeof(CLIENTPACKET)+2, 0, (struct sockaddr *)&MasterAddr, sizeof(MasterAddr));

	// Store last activity time
	gettimeofday(&LastTxActivityTime, NULL);

 return;
}

int SendPackets(int MaxPackets, int MaxBytes){

	int Queue, Packets, Bytes, PacketBytes;

	// Clear counts
	Packets = 0;
	Bytes = 0;

	// Wait for access to the packet queue
	pthread_mutex_lock(&PacketQueueMutex);

	// Loop thorugh each queue
	for(Queue = 0; Queue < PacketQueues; Queue++){

		// Loop for as many packets/bytes as we're allowed to send
		while(PacketQueueHead[Queue] != PacketQueueTail[Queue]){

			// Check if we'd go over the packet limit
			if(Packets + 1 > MaxPackets)
				break;

			PacketBytes = ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->data_len;

			// Check if we'd go over the byte limit
			if(Bytes + PacketBytes > MaxBytes)
				break;

			// Send the oldest packet
			ipq_set_verdict(hIpq, ((ipq_packet_msg_t *)&PacketQueue[Queue][PacketQueueTail[Queue] * sizeof(struct ipq_packet_msg)])->packet_id, NF_ACCEPT, 0, NULL);

			// Add to running totals
			Packets++;
			Bytes += PacketBytes;

			// Store stats
			ClientStats.Packets[Queue]++;
			ClientStats.Bytes[Queue] += PacketBytes;
			ClientStats.BytesAvg[Queue] += PacketBytes;

			// Move the Tail offset
			PacketQueueTail[Queue]++;
			if(PacketQueueTail[Queue] >= Config.Queue)
				PacketQueueTail[Queue] = 0;
		}
	}

	// Release access to the packet queue
	pthread_mutex_unlock(&PacketQueueMutex);

	return 1;
}

void GetWirelessState(void){

	struct iwreq	WirelessReq;
	struct iw_statistics	Stats;
	int fdSock;
	
	// Set defaults
	CurrentRate = 5;
	CurrentSignal = -120;
	CurrentNoise = -120;

	fdSock = socket(AF_INET, SOCK_DGRAM, 0);
	if(fdSock < 0)
		return;

	// Get bit rate
	strcpy(WirelessReq.ifr_name, Config.winterface);
	if(ioctl(fdSock, SIOCGIWRATE, &WirelessReq) >= 0){

		CurrentRate = WirelessReq.u.bitrate.value / 1000000;
	}

	// Get stats (signal/noise)
	strcpy(WirelessReq.ifr_name, Config.winterface);
	WirelessReq.u.data.pointer = (caddr_t)&Stats;
	if(ioctl(fdSock, SIOCGIWSTATS, &WirelessReq) >= 0){

		CurrentSignal = Stats.qual.level - 0x100;
		CurrentNoise = Stats.qual.noise - 0x100;
	}

	// Close the socket
	close(fdSock);

	return;
}

// This writes stats to a html file (usually run in it's
// own thread to not slow down the client loop)
void *WriteStatsFile(void *pParam){

	int i;
	char *pFileName;
	char QueueName[32];

	// Write current client info to info file
	FILE *StatsFile = fopen(Config.StatsFilename, "w");
	if(StatsFile != NULL){

		// Workout the file name without path
		pFileName = strrchr(Config.StatsFilename, '/');
		if(pFileName == NULL)
			pFileName = Config.StatsFilename;
		else
			pFileName++;

		// Write the header
		fprintf(StatsFile, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n\n<html>\n\n");
		fprintf(StatsFile, "<head>\n<meta http-equiv=\"content-type\" content=\"text-html; charset=utf-8\">\n");
		fprintf(StatsFile, "<meta NAME=\"Description\" CONTENT=\"Frottle stats\">\n<title>Frottle stats</title>\n");
		fprintf(StatsFile, "<meta http-equiv=\"refresh\" content=\"10; URL=%s\">\n</head>\n\n<body>\n\n", pFileName);
		if(Config.Flags & CONFIGFLAGS_SELFCLIENT){
			fprintf(StatsFile, "<h2>Frottle self-client (v%s)</h2>\n", FROTVERSION);
			fprintf(StatsFile, "<p>\nLast updated: %s<br>\nHigh priority ports:", GetTimeString("%d/%m/%Y %H:%M:%S"));
			}
		else{
			fprintf(StatsFile, "<h2>Frottle client (v%s)</h2>\n", FROTVERSION);
			fprintf(StatsFile, "<p>\nLast updated: %s<br>\nMaster: %s:%d (%s)<br>\nPolls/Sec: %ld<br>\nHigh priority ports:",
				GetTimeString("%d/%m/%Y %H:%M:%S"),
				Config.MasterIp, Config.MasterPort, (ClientStatus & CS_CONNECTED) ? "connected" : "not connected",
				ClientStats.Polls/10);
			}
		for(i = 0; i < Config.HiPortsCnt; i++)
			fprintf(StatsFile, " %d", Config.HiPorts[i]);
		fprintf(StatsFile, "\n</p>\n\n");

		// Setup titles
		if(PacketQueues > 0)
			fprintf(StatsFile, "<table>\n<tr bgcolor=\"c0c0c0\"><td>Queue</td><td>Packets sent</td><td>MBytes sent</td><td>Rate kB/s</td></tr>\n");

		// Fill in any queue info
		for(i = 0; i < PacketQueues; i++){

			// This should probably need changing if PacketQueues is changed
			if(i == 0)
				strcpy(QueueName, "High");
			else if(i == 1)
				strcpy(QueueName, "Medium");
			else if(i == 2)
				strcpy(QueueName, "Low");
			else
				sprintf(QueueName, "%d", i+1);

			fprintf(StatsFile, "<tr><td>%s</td><td align=\"right\">%.0f</td><td align=\"right\">%.0f</td><td align=\"right\">%.1f</td></tr>\n",
				QueueName, ClientStats.Packets[i], ClientStats.Bytes[i] / 1048576, ClientStats.BytesAvg[i] / 10240);

			ClientStats.BytesAvg[i] = 0;
		}

		if(PacketQueues > 0)
			fprintf(StatsFile, "</table>\n\n");

		// Link to the home page
		fprintf(StatsFile, "</p><a href=\"http://frottle.sourceforge.net/\">Frottle home page</a></p>\n");

		fprintf(StatsFile, "</body>\n\n</html>\n");

		// Close file
		fclose(StatsFile);
	}

	return NULL;
}

