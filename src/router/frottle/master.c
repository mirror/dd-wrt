/***************************************************************************
    master.c   -  Perform the master (polling and co-ordination) functionality
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

// Number of sequantial timeouts before we
// drop a client from the poll list
#define MAXTIMEOUTS		50

// Each registered client is polled sequentially in loop 0, then in
// additional loops only clients that had data remaining in their
// queue previously are polled. Then the process repeats.
#define POLLLOOPS		2

// Maximum number of clients we can handle
#define MAXCLIENTS		30

// Local variables
int hMasterPort;
short PollLoop = 0;
short PollLastIndex = -1;
struct timeval	LastPollTime, LastLoop0Time, LastBackgroundTime;
FILE	*InfoFile;

// Client parameters
typedef struct _CLIENT{
	struct sockaddr_in	Addr;
	CLIENTPACKET		LastPacket;
	struct timeval		LastRxTime;
	int					TimeoutCount;
	unsigned long		PollCount;
	unsigned long		DroppedPollCount;
	char				RegTime[30];
	int					RespTimes[100];
	char				Hostname[100];
} CLIENT, *LCLIENT;
CLIENT Client[MAXCLIENTS];
int ClientCount = 0;

// Forward declarations of local functions
void *MasterCleanup(void *pParam);
void HandleResistration(struct sockaddr_in FromAddr, CLIENTPACKET *lpClientPacket);
void HandleEndPacket(struct sockaddr_in FromAddr, CLIENTPACKET *lpClientPacket);
void SendNextPoll(void);
void WriteIpFile(void);
void *WriteInfoFile(void *pParam);

// This is the main entry point in to the master code
// and may or may not be run in it's own thread
void *MasterRun(void *pParam){

	struct sockaddr_in MasterAddr, FromAddr;
	socklen_t SockaddrLen;
	BYTE PacketBuffer[max(sizeof(MASTERPACKET), sizeof(CLIENTPACKET)) + 2];
	int PacketSize, i, j;
	//pthread_t idWriteInfoFileThread;

	// Open UDP port for connections
	hMasterPort = socket(PF_INET, SOCK_DGRAM, 0);
	if(hMasterPort < 0){
		Log(LOG_ERR, "Failed to open master socket");
		return NULL;
	}

	// Bind to server port
	memset(&MasterAddr, 0, sizeof(MasterAddr));
	MasterAddr.sin_family = AF_INET;
	MasterAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	MasterAddr.sin_port = htons(Config.MasterPort);

	if(bind(hMasterPort, (struct sockaddr *)&MasterAddr, sizeof(MasterAddr)) != 0){
		close(hMasterPort);
		Log(LOG_ERR, "Failed to bind to master port");
		return NULL;
	}

	// Reset time value
	gettimeofday(&LastLoop0Time, NULL);
	memset(&LastBackgroundTime, 0, sizeof(LastBackgroundTime));
	gettimeofday(&LastBackgroundTime, NULL);
	LastBackgroundTime.tv_sec -= 8;

	// Write IP data
	WriteIpFile();

	// Setup cleanup function
	pthread_cleanup_push((void*)MasterCleanup, NULL);

	// Tell user
	Log(LOG_INFO, "Master up and running");

	// While we should run (helps exit a thread cleanly)
	while(!ExitMasterThread){

		struct timeval Timeout;
		fd_set readfds;
		int SelRet;

		FD_ZERO(&readfds);
		FD_SET(hMasterPort, &readfds);

		// Wait up to 5 mSec for UDP data
		Timeout.tv_sec = 0;
		Timeout.tv_usec = 5000;
		SelRet = select(hMasterPort+1, &readfds, NULL, NULL, &Timeout);

		// Error in select
		if(SelRet < 0){

			// Select error
			Log(LOG_ERR, "Error waiting for client packet");
		}
		// Data pending
		else if(SelRet > 0){

			// Get packet from client
			SockaddrLen = sizeof(FromAddr);
			PacketSize = recvfrom(hMasterPort, PacketBuffer, sizeof(PacketBuffer), 0, (struct sockaddr *)&FromAddr, &SockaddrLen);

			// Check packet size
			if(PacketSize != sizeof(CLIENTPACKET) + 2){

				// Wrong sized packet
				Log(LOG_ERR, "Master got packet of incorrect size");
			}
			else{

				// Check the checksum
				for(i = 0, j = 0; i < sizeof(CLIENTPACKET); i++){
					j <<= 8;
					j += PacketBuffer[i];
				}
				if(PacketBuffer[i] != (USHORT)j){

					// Corrupted packet
					Log(LOG_ERR, "Master got corrupted packet");
				}
				else{

					// Switch on packet type ID
					switch(((CLIENTPACKET*)PacketBuffer)->TypeId){

					case CLIENTID_REGISTER:

						// Handle the packet
						HandleResistration(FromAddr, (CLIENTPACKET*)PacketBuffer);
						break;

					case CLIENTID_END:

						// Handle the packet
						HandleEndPacket(FromAddr, (CLIENTPACKET*)PacketBuffer);
						break;

					default:
						Log(LOG_ERR, "Master got unknown packet");
						break;
					}
				}
			}
		}

		// Delt with data in the port, now any back ground processing...

		// Special case for polling, see SendNextPoll()
		if(PollLoop == -1){

			SendNextPoll();
		 }
		// If last poll has timed out
		else if(ClientCount > 0 && PollLastIndex != -1  && Age(LastPollTime) > Config.Timeout){

			// Log(LOG_INFO, "Timeout for client at %s:%d", inet_ntoa(Client[PollLastIndex].Addr.sin_addr), Client[PollLastIndex].Addr.sin_port);

			// Wipe last packet data
			//memset(&Client[PollLastIndex].LastPacket, 0, sizeof(Client[PollLastIndex].LastPacket));
			Client[PollLastIndex].LastPacket.Rate = 0;
			Client[PollLastIndex].LastPacket.PacketsInQueue = 0;
			Client[PollLastIndex].LastPacket.BytesInQueue = 0;
			Client[PollLastIndex].DroppedPollCount++;
			
			// Count timeouts for each client
			Client[PollLastIndex].TimeoutCount++;
			if(Client[PollLastIndex].TimeoutCount > MAXTIMEOUTS){

				Log(LOG_NOTICE, "Dropping lost client at %s:%d", inet_ntoa(Client[PollLastIndex].Addr.sin_addr), Client[PollLastIndex].Addr.sin_port);

				// Delete the client
				for(i = PollLastIndex; i < ClientCount - 1; i++)
					memcpy(&Client[i], &Client[i+1], sizeof(Client[i]));
				ClientCount--;

				// Write IP data
				WriteIpFile();

				// If that was our only client
				if(ClientCount == 0)
					PollLastIndex = -1;
			}

			SendNextPoll();
		}

		// If no background processing for 10 sec (approx.)
		if(Age(LastBackgroundTime) >= 10000){

			// Start file write function in own threa
			//idWriteInfoFileThread = 0;
			//pthread_create(&idWriteInfoFileThread, NULL, &WriteInfoFile, NULL);
			// TODO: Figure out why doing it in a thread dies sooner or later (no further updates)
			WriteInfoFile(NULL);

			// If we are a self client, we can still write stats
			if(Config.Flags & CONFIGFLAGS_SELFCLIENT)
				WriteStatsFile(NULL);

			// Store updated background processing time
			gettimeofday(&LastBackgroundTime, NULL);
		}
	}

	// Call and clear cleanup function
	pthread_cleanup_pop(1);

	return NULL;
}

// In here we should do any cleanup, such as closing ports,
// that might be needed when we exit
void *MasterCleanup(void *pParam){

	// Close the port
	close(hMasterPort);

	// Delete the info & ip files
	unlink(Config.InfoFilename);
	unlink("/tmp/frottle.ip");
	
	return NULL;
}

void HandleResistration(struct sockaddr_in FromAddr, CLIENTPACKET *lpClientPacket){

	int i;
	struct hostent *HostEnt;

	// Loop through Poll list
	for(i = 0; i < ClientCount; i++){

		// If this IP is already registered
		if(memcmp(&FromAddr.sin_addr, &Client[i].Addr.sin_addr, sizeof(FromAddr.sin_addr)) == 0){

			// If it is the same source port
			if(FromAddr.sin_port == Client[i].Addr.sin_port){

				Log(LOG_NOTICE, "Got repeat registration from %s:%d", inet_ntoa(FromAddr.sin_addr), FromAddr.sin_port);

				// Already registered, nothing to do
				return;
			}
			// IP is registered, but from a different port (probably
			// the frottle client has been restarted)
			else{

				// Delete this entry
				while(i < ClientCount - 1){
					memcpy(&Client[i], &Client[i+1], sizeof(Client[i]));
					i++;
				}
				ClientCount--;

				// Fall through to re-add it
				break;
			}
		}
	}

	// Check cap
	if(ClientCount >= MAXCLIENTS){

		Log(LOG_WARNING, "Maximum client limit ceached");
		return;
	}

	// Add this client
	memcpy(&Client[ClientCount].Addr, &FromAddr, sizeof(Client[ClientCount].Addr));
	memcpy(&Client[ClientCount].LastPacket, lpClientPacket, sizeof(Client[ClientCount].LastPacket));
	gettimeofday(&Client[ClientCount].LastRxTime, NULL);
	HostEnt = gethostbyaddr(&FromAddr.sin_addr, sizeof(FromAddr.sin_addr), AF_INET);
	if(HostEnt == NULL)
		strcpy(Client[ClientCount].Hostname, inet_ntoa(FromAddr.sin_addr));
	else
		strcpy(Client[ClientCount].Hostname, HostEnt->h_name);
	Client[ClientCount].TimeoutCount = 0;
	Client[ClientCount].PollCount = 0;
	Client[ClientCount].DroppedPollCount = 0;
	for(i = 0; i < 100; i++)
		Client[ClientCount].RespTimes[i] = 0;
	strcpy(Client[ClientCount].RegTime, GetTimeString("%d/%m/%Y %H:%M:%S"));
	ClientCount++;

	Log(LOG_NOTICE, "Got registration from %s:%d", Client[ClientCount-1].Hostname, FromAddr.sin_port);

	// Write IP data
	WriteIpFile();

	// If this is our first client, poll it
	if(ClientCount == 1)
		SendNextPoll();

	return;
}

void HandleEndPacket(struct sockaddr_in FromAddr, CLIENTPACKET *lpClientPacket){

	int i;

	// If this is not from the client we last polled
	if(memcmp(&Client[PollLastIndex].Addr, &FromAddr, sizeof(Client[PollLastIndex].Addr)) != 0){
		//Log(LOG_NOTICE, "Got out of sync end from %s:%d", inet_ntoa(FromAddr.sin_addr), FromAddr.sin_port);
		return;
	}

	// Store client data
	memcpy(&Client[PollLastIndex].LastPacket, lpClientPacket, sizeof(Client[PollLastIndex].LastPacket));
	gettimeofday(&Client[PollLastIndex].LastRxTime, NULL);
	for(i = 98; i >= 0; i--)
		Client[PollLastIndex].RespTimes[i+1] = Client[PollLastIndex].RespTimes[i];
	Client[PollLastIndex].RespTimes[0] = Age(LastPollTime);

	// Clear timeout count
	Client[PollLastIndex].TimeoutCount = 0;

	// Send next poll
	SendNextPoll();

	return;
}

// This function provides the polling algorithm.
void SendNextPoll(void){

	BYTE PacketBuffer[sizeof(MASTERPACKET) + 2];
	int i, j;

	// If we don't have clients
	if(ClientCount == 0){
		PollLastIndex = -1;
		return;
	}

	// If we are a client to ourselves
	if((Config.Flags & CONFIGFLAGS_SELFCLIENT)){

		// Allow the master to send some amount of bytes per poll loop
		i = Config.PollParams[0] / ClientCount;

		// Send out the packets
		SendPackets(0xFFFF, i);
	}

	// If we are in a loop wrap around pause
	if(PollLoop == -1){

		// If less than 20mS, wait a bit longer
		if(Age(LastLoop0Time) < 20)
			return;

		// Store time for next time
		gettimeofday(&LastLoop0Time, NULL);

		PollLoop = 0;
		PollLastIndex = 0;
	}
	else{

		while(1){

			// Incriment poll index and loop
			PollLastIndex++;
			if(PollLastIndex >= ClientCount){
				PollLastIndex = 0;
				PollLoop++;
				// If the poll loop is wrappig around, don't poll yet
				if(PollLoop >= POLLLOOPS){
					PollLoop = -1;
					PollLastIndex = -1;
					return;
				}
			}

			// Break if we should poll this one
			if(PollLoop == 0 || Client[PollLastIndex].LastPacket.BytesInQueue > 0)
				break;
		}
	}

	// Send poll packet
	memset(PacketBuffer, 0, sizeof(PacketBuffer));
	((MASTERPACKET*)PacketBuffer)->TypeId = MASTERID_POLL;
	// If polling fast node
	if(Client[PollLastIndex].LastPacket.Rate >= 5 || Client[PollLastIndex].LastPacket.Rate == 0){
		((MASTERPACKET*)PacketBuffer)->MaxPacketsToSend = Config.PollParams[1];
		((MASTERPACKET*)PacketBuffer)->MaxBytesToSend = Config.PollParams[2];
	}
	// If polling slow node
	else if(Client[PollLastIndex].LastPacket.Rate == 2){
		((MASTERPACKET*)PacketBuffer)->MaxPacketsToSend = Config.PollParams[3];
		((MASTERPACKET*)PacketBuffer)->MaxBytesToSend = Config.PollParams[4];
	}
	// If painfully slow node
	else{
		((MASTERPACKET*)PacketBuffer)->MaxPacketsToSend = Config.PollParams[5];
		((MASTERPACKET*)PacketBuffer)->MaxBytesToSend = Config.PollParams[6];
	}
	for(i = 0, j = 0; i < sizeof(MASTERPACKET); i++){
		j <<= 8;
		j += PacketBuffer[i];
	}
	PacketBuffer[i] = (USHORT)j;
	sendto(hMasterPort, &PacketBuffer, sizeof(MASTERPACKET)+2, 0, (struct sockaddr *)&Client[PollLastIndex].Addr, sizeof(Client[PollLastIndex].Addr));

	// Incriment poll count
	Client[PollLastIndex].PollCount++;
	if(Client[PollLastIndex].PollCount >= 0xFFFFFFFF){
		Client[PollLastIndex].PollCount /= 2;
		Client[PollLastIndex].DroppedPollCount /= 2;
	}
		
	// Update last poll time
	gettimeofday(&LastPollTime, NULL);

	return;
}

// Write a list of client IPs to a text file, one per line.
// (This can be used by iptables to firewall non-frottle clients)
void WriteIpFile(void){

	FILE *IpFile;
	int i;
	
	// Write current client ips to file
	IpFile = fopen("/tmp/frottle.ip", "w");
	if(IpFile != NULL){

		// Write out each IP
		for(i = 0; i < ClientCount; i++)
			fprintf(IpFile, "%s\n", inet_ntoa(Client[i].Addr.sin_addr));

		// Close file
		fclose(IpFile);
	}

	return;
}

// This writes stats to a html file (usually run in it's
// own thread to not slow down the main polling loop)
void *WriteInfoFile(void *pParam){

	int i, j, RespMin, RespMax;
	long RespAvg;
	char *pFileName;

	// Write current client info to info file
	FILE *InfoFile = fopen(Config.InfoFilename, "w");
	if(InfoFile != NULL){

		// Workout the file name without path
		pFileName = strrchr(Config.InfoFilename, '/');
		if(pFileName == NULL)
			pFileName = Config.InfoFilename;
		else
			pFileName++;

		// Write the header
		fprintf(InfoFile, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n\n<html>\n\n");
		fprintf(InfoFile, "<head>\n<meta http-equiv=\"content-type\" content=\"text-html; charset=utf-8\">\n");
		fprintf(InfoFile, "<meta NAME=\"Description\" CONTENT=\"Frottle info\">\n<title>Frottle Info</title>\n");
		fprintf(InfoFile, "<meta http-equiv=\"refresh\" content=\"10; URL=%s\">\n</head>\n\n<body>\n\n", pFileName);
		fprintf(InfoFile, "<h2>Frottle master (v%s)</h2>\n", FROTVERSION);
		fprintf(InfoFile, "<p>\nLast updated: %s<br>\nClient count: %d<br>\nHigh priority ports:", GetTimeString("%d/%m/%Y %H:%M:%S"), ClientCount);
		for(i = 0; i < Config.HiPortsCnt; i++)
			fprintf(InfoFile, " %d", Config.HiPorts[i]);
		fprintf(InfoFile, "\n</p>\n\n");

		// Setup titles
		if(ClientCount > 0)
			fprintf(InfoFile, "<table>\n<tr bgcolor=\"c0c0c0\"><td>Host/IP</td><td>RF<br>rate, signal, noise</td><td>Queued<br>packets, bytes</td><td>Dropped %%</td><td>Poll time (ms)<br>min, avg, max</td><td>Registration<br>time</td></tr>\n");

		// Fill in any client info
		for(i = 0; i < ClientCount; i++){

			// Work out min, avg, max response times
			RespMin = Client[i].RespTimes[0];
			RespAvg = Client[i].RespTimes[0];
			RespMax = Client[i].RespTimes[0];
			for(j = 1; j < 100; j++){
				if(RespMin > Client[i].RespTimes[j])
					RespMin = Client[i].RespTimes[j];
				if(RespMax < Client[i].RespTimes[j])
					RespMax = Client[i].RespTimes[j];
				RespAvg += Client[i].RespTimes[j];
			}
			RespAvg /= 100;

			fprintf(InfoFile, "<tr><td><a href=\"http://%s\">%s</a></td><td>%d, %d, %d</td><td>%d, %d</td><td>%.1f</td><td>%d, %d, %d</td><td>%s</td></tr>\n",
				Client[i].Hostname, Client[i].Hostname,
				Client[i].LastPacket.Rate,
				Client[i].LastPacket.Signal,
				Client[i].LastPacket.Noise,
				Client[i].LastPacket.PacketsInQueue,
				Client[i].LastPacket.BytesInQueue,
				((float)Client[i].DroppedPollCount / (float)Client[i].PollCount) * 100,
				RespMin, (int)RespAvg, RespMax,
				Client[i].RegTime);
		}

		if(ClientCount > 0)
			fprintf(InfoFile, "</table>\n\n");

		// Show master parameters
		fprintf(InfoFile, "<p>\n<small>Polling parameters: %d %d %d:%d %d:%d %d:%d</small>\n</p>\n\n",
			Config.Timeout,
			Config.PollParams[0], Config.PollParams[1], Config.PollParams[2],
			Config.PollParams[3], Config.PollParams[4], Config.PollParams[5],
			Config.PollParams[6]);

		// Link to the home page
		fprintf(InfoFile, "<p><a href=\"http://frottle.sourceforge.net/\">Frottle home page</a></p>\n");

		fprintf(InfoFile, "</body>\n\n</html>\n");

		// Close file
		fclose(InfoFile);
	}

	return NULL;
}

// Return a pointer to a string showing the current
// time/date in the requested format
char *GetTimeString(char *Format){

	struct timeval	Now;
	struct tm	*Time;
	static char	TimeString[40];

	gettimeofday(&Now, NULL);
	Time = localtime(&Now.tv_sec);
	strftime(TimeString, sizeof(TimeString), Format, Time);

	return TimeString;
}
