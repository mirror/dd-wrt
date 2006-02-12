/***************************************************************************
    main.h     -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Standard linux headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <linux/netfilter.h>
              
// Headers for polling wireless tools in GetWirelessState()
#include "wireless.h"

// IPQ headers (only present if installed)
#include "../iptables/include/libipq/libipq.h"

// Current version  TODO: make sure this is correct
#define FROTVERSION	"0.2.1"

// Useful defines
#define	APPTITLE	"Frottle"
#define	TRUE		1
#define	FALSE		0
#define	BYTE		unsigned char
#define	USHORT		unsigned short
#define	min(x,y)	((x<y)?x:y)
#define	max(x,y)	((x>y)?x:y)

// Time in mS after which to drop packets as stale
#define STALETIME	5000

// IPQ handle
extern struct ipq_handle *hIpq;

// Packet queue itself
#define PacketQueues	3
extern BYTE	*PacketQueue[PacketQueues];
extern int	PacketQueueHead[PacketQueues], PacketQueueTail[PacketQueues];
extern pthread_mutex_t	PacketQueueMutex;

// Client -> server status flags
#define	CS_NOSERVER		0x00
#define	CS_REGISTER		0x01
#define	CS_CONNECTED	0x02
extern BYTE	ClientStatus;

// Thread control variables defined in main.c
extern BYTE	ExitMasterThread;
extern BYTE	ExitControlThread;

// Configuration parameters
#define	CONFIGFLAGS_MASTER		0x01
#define	CONFIGFLAGS_CLIENT		0x02
#define CONFIGFLAGS_SELFCLIENT	0x04
#define	CONFIGFLAGS_VERBOSE		0x08
#define	CONFIGFLAGS_DAEMON		0x10
typedef struct _CONFIG{
	BYTE	Flags;
	char	MasterIp[32];
	short	MasterPort;
	short	Timeout;
	int		PollParams[7];
	short	Queue;
	int		HiPortsCnt;
	int		HiPorts[10];
	char	winterface[12];
	char	InfoFilename[256];
	char	StatsFilename[256];
	char	ConfFilename[256];
} CONFIG, *LCONFIG;
extern CONFIG	Config;

// Packet format sent from master to client
#define	MASTERID_POLL		1
typedef struct _MASTERPACKET{
	BYTE		TypeId;
	BYTE		Flags;
	USHORT		MaxPacketsToSend;
	USHORT		MaxBytesToSend;
	BYTE		FutureUse[4];
} MASTERPACKET, *LCMASTERPACKET;

// Packet format sent from client to master
#define	CLIENTID_REGISTER	1
#define	CLIENTID_END		2
#define	CLIENTFLAGS_LATENCY	0x01
typedef struct _CLIENTPACKET{
	BYTE		TypeId;
	BYTE		Flags;
	USHORT		PacketsInQueue;
	USHORT		BytesInQueue;
	BYTE		Rate;
	char		Signal;
	char		Noise;
	BYTE		FutureUse[4];
} CLIENTPACKET, *LCCLIENTPACKET;

// From master.c
extern int ClientCount;
void *MasterRun(void *pParam);
void Log(int Level, char *Format, ...);
int Age(struct timeval OldTime);
char *GetTimeString(char *Format);


// From client.c
void *ControlRun(void *pParam);
int SendPackets(int MaxPackets, int MaxBytes);
void *WriteStatsFile(void *pParam);


