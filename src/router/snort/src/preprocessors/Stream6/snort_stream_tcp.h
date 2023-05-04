/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef _STREAM_TCP_H_
#define _STREAM_TCP_H_

#include "session_common.h"
#include "stream_common.h"
#include "sfPolicy.h"

extern uint32_t xtradata_func_count;
extern LogFunction xtradata_map[LOG_FUNC_MAX];
extern LogExtraData extra_data_log;
extern void *extra_data_config;

void StreamCleanTcp(void);
void StreamResetTcp(void);
void StreamInitTcp(void);
void StreamTcpRegisterPreprocProfiles(void);
void StreamTcpRegisterRuleOptions(struct _SnortConfig *);
void StreamTcpInitFlushPoints(void);
int StreamVerifyTcpConfig(struct _SnortConfig *, StreamTcpConfig *, tSfPolicyId);
void StreamTcpPolicyInit(struct _SnortConfig *, StreamTcpConfig *, char *);
int StreamProcessTcp(Packet *, SessionControlBlock *,
        StreamTcpPolicy *, SessionKey *);
int StreamFlushListener(Packet *p, SessionControlBlock *scb);
int StreamFlushTalker(Packet *p, SessionControlBlock *scb);
int StreamFlushClient(Packet *p, SessionControlBlock *scb);
int StreamFlushServer(Packet *p, SessionControlBlock *scb);
void TcpUpdateDirection(SessionControlBlock *ssn, char dir,
        sfaddr_t* ip, uint16_t port);
void StreamTcpSessionClear(Packet *p);
SessionControlBlock *GetLWTcpSession(const SessionKey *key);
int GetTcpRebuiltPackets(Packet *p, SessionControlBlock *ssn,
        PacketIterator callback, void *userdata);
int GetTcpStreamSegments(Packet *p, SessionControlBlock *ssn,
        StreamSegmentIterator callback, void *userdata);
int StreamAddSessionAlertTcp(SessionControlBlock *scb, Packet *p, uint32_t gid, uint32_t sid);
int StreamCheckSessionAlertTcp(SessionControlBlock *scb, Packet *p, uint32_t gid, uint32_t sid);
int StreamUpdateSessionAlertTcp(SessionControlBlock *scb, Packet *p, uint32_t gid, uint32_t sid, uint32_t event_id, uint32_t event_second);
void StreamSetExtraDataTcp(SessionControlBlock*, Packet*, uint32_t flag);
void StreamClearExtraDataTcp(SessionControlBlock*, Packet*, uint32_t flag);
char StreamGetReassemblyDirectionTcp(SessionControlBlock *scb);
uint32_t StreamGetFlushPointTcp(SessionControlBlock *scb, char dir);
void StreamSetFlushPointTcp(SessionControlBlock *scb, char dir, uint32_t flush_point);
char StreamSetReassemblyTcp(SessionControlBlock *scb, uint8_t flush_policy, char dir, char flags);
char StreamGetReassemblyFlushPolicyTcp(SessionControlBlock *scb, char dir);
char StreamIsStreamSequencedTcp(SessionControlBlock *scb, char dir);
int StreamMissingInReassembledTcp(SessionControlBlock *scb, char dir);
char StreamPacketsMissingTcp(SessionControlBlock *scb, char dir);
void s5TcpSetPortFilterStatus(struct _SnortConfig *sc,
        unsigned short port, uint16_t status, tSfPolicyId policyId, int parsing );
void s5TcpUnsetPortFilterStatus( struct _SnortConfig *sc, unsigned short port, uint16_t status,
        tSfPolicyId policyId, int parsing );
int s5TcpGetPortFilterStatus( struct _SnortConfig *sc, unsigned short port, tSfPolicyId policyId, int parsing );
void s5TcpSetSynSessionStatus(struct _SnortConfig *sc, uint16_t status, tSfPolicyId policyId, int parsing);
void s5TcpUnsetSynSessionStatus(struct _SnortConfig *sc, uint16_t status, tSfPolicyId policyId, int parsing);
void StreamTcpConfigFree(StreamTcpConfig *);
void **StreamGetPAFUserDataTcp(SessionControlBlock*, bool to_server, uint8_t id);
bool StreamIsPafActiveTcp(SessionControlBlock*, bool to_server);
bool StreamActivatePafTcp (SessionControlBlock *scb, int dir, int16_t service_port, uint8_t type);
void StreamResetPolicyTcp(SessionControlBlock*, int dir, uint16_t policy, uint16_t mss);
void StreamSetSessionDecryptedTcp( SessionControlBlock *scb, bool enable);
bool StreamIsSessionDecryptedTcp( SessionControlBlock *scb );

uint32_t StreamGetTcpPrunes(void);
void StreamResetTcpPrunes(void);
void enableRegisteredPortsForReassembly( struct _SnortConfig *sc );
uint32_t StreamGetPreprocFlagsTcp(SessionControlBlock *scb);

#ifdef NORMALIZER
void Stream_PrintNormalizationStats(void);
void Stream_ResetNormalizationStats(void);
#endif

void StreamPostConfigTcp(struct _SnortConfig *sc, void*);

void registerPortForReassembly( char *network, uint16_t port, int reassembly_direction );
void unregisterPortForReassembly( char *network, uint16_t port, int reassembly_direction );
Packet* getWirePacketTcp();
uint8_t getFlushPolicyDirTcp();
bool StreamIsSessionHttp2Tcp( SessionControlBlock *scb );
void StreamSetSessionHttp2Tcp( SessionControlBlock *scb );
bool StreamIsSessionHttp2UpgTcp( SessionControlBlock *scb );
void StreamSetSessionHttp2UpgTcp( SessionControlBlock *scb );

void SessionTCPReload(uint32_t max_sessions, uint16_t pruningTimeout, uint16_t nominalTimeout);
unsigned SessionTCPReloadAdjust(unsigned maxWork);

void SetFTPFileLocation(void *scbptr ,bool flush);

void set_service_based_flush_policy(SessionControlBlock *scb);

#ifdef HAVE_DAQ_DECRYPTED_SSL
int StreamSimulatePeerTcpAckp( SessionControlBlock *scb, uint8_t dir, uint32_t tcp_payload_len );
#endif

size_t get_tcp_used_mempool();

#endif /* STREAM_TCP_H_ */
