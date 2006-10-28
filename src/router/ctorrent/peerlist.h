#ifndef PEERLIST_H
#define PEERLIST_H

#include <sys/types.h>
#include "./def.h"
#include "./peer.h"
#include "./rate.h"

typedef struct _peernode{
  btPeer *peer;
  size_t click;
  struct _peernode *next;
}PEERNODE;

class PeerList
{
 private:
  SOCKET m_listen_sock;
  PEERNODE *m_head;
  size_t m_peers_count;
  time_t m_unchoke_check_timestamp, m_keepalive_check_timestamp, m_last_progress_timestamp;

  unsigned char m_live_idx:2;
  unsigned char m_reserved:6;
  
  Rate m_pre_dlrate, m_pre_ulrate;
  
  int Accepter();
  void Sort();
  void UnChokeCheck(btPeer* peer,btPeer *peer_array[]);
  
 public:
  PeerList();
  ~PeerList();

  size_t TotalPeers() const { return m_peers_count; }
  int Initial_ListenPort();

  int IsEmpty() const;

  void PrintOut();

  int NewPeer(struct sockaddr_in addr, SOCKET sk);
  
  void CloseAllConnectionToSeed();
  void CloseAll();
  
  int FillFDSET(const time_t *pnow, fd_set *rfd, fd_set *wfd);
  void AnyPeerReady(fd_set *rfdp,fd_set *wfdp,int *nready);
  
  void Tell_World_I_Have(size_t idx);
  btPeer* Who_Can_Abandon(btPeer *proposer);
  void CheckBitField(BitField &bf);
  int AlreadyRequested(size_t idx);
  size_t Pieces_I_Can_Get();
};

extern PeerList WORLD;

#endif
