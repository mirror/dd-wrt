#ifndef IPLIST_H
#define IPLIST_H

#include "def.h"

#ifdef WINDOWS
#include <Winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

typedef struct _iplist{
  struct sockaddr_in address;
  struct _iplist *next;
}IPLIST;

class IpList
{
private:
  IPLIST *ipl_head;
  size_t count;
  void _Emtpy();
public:
  IpList() { ipl_head = (IPLIST*)0; count = 0;}
  ~IpList() { if(ipl_head) _Emtpy(); }
  int Add(const struct sockaddr_in* psin);
  int Pop(struct sockaddr_in* psin);
  int IsEmpty() { return count ? 0 : 1; }
};

extern IpList IPQUEUE;
#endif
