#ifndef TRACKER_H
#define TRACKER_H

#include <sys/types.h>

#include "./def.h"
#include "./bufio.h"

#ifdef WINDOWS
#include <Winsock2.h>

#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/param.h>
#endif

#define T_FREE 		0
#define T_CONNECTING	1
#define T_READY		2

class btTracker
{
 private:
  char m_host[MAXHOSTNAMELEN];
  char m_path[MAXPATHLEN];
  int m_port;

  struct sockaddr_in m_sin;

  unsigned char m_status:2;
  unsigned char m_f_started:1;
  unsigned char m_f_stoped:1;

  unsigned char m_f_pause:1;
  unsigned char m_f_reserved:3;


  time_t m_interval;		// 与Tracker通信的时间间隔
  time_t m_last_timestamp;	// 最后一次成功与Tracker通信的时间
  size_t m_connect_refuse_click;

  SOCKET m_sock;
  BufIo m_reponse_buffer;
  
  int _IPsin(char *h, int p, struct sockaddr_in *psin);
  int _s2sin(char *h,int p,struct sockaddr_in *psin);
  int _UpdatePeerList(char *buf,size_t bufsiz);

 public:
  btTracker();
  ~btTracker();

  int Initial();

  void Reset(time_t new_interval);

  unsigned char GetStatus() { return m_status;}
  void SetStatus(unsigned char s) { m_status = s; }

  SOCKET GetSocket() { return m_sock; }

  void SetPause() { m_f_pause = 1; }
  void ClearPause() { m_f_pause = 0; }

  int Connect();
  int SendRequest();
  int CheckReponse();
  int IntervalCheck(const time_t *pnow,fd_set* rfdp, fd_set *wfdp);
  int SocketReady(fd_set *rfdp, fd_set *wfdp, int *nfds);

  size_t GetRefuseClick() const { return m_connect_refuse_click; }
};

extern btTracker Tracker;

#endif
