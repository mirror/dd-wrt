#include <string.h>
#include "iplist.h"

IpList IPQUEUE;

void IpList::_Emtpy()
{
  IPLIST *node = ipl_head;
  for(; ipl_head;){
    node = ipl_head;
    delete ipl_head;
    ipl_head = node->next;
  }
  count = 0;
}

int IpList::Add(const struct sockaddr_in *psin)
{
  IPLIST *node = ipl_head;
  for(; node; node = node->next)
    if(memcmp(psin, &node->address, sizeof(struct sockaddr_in)) == 0) break;

  if( node ) return -1;
  // if not exist;
  node = new IPLIST;
#ifndef WINDOWS
  if( !node ) return -1;
#endif
  count++;
  memcpy(&node->address,psin,sizeof(struct sockaddr_in));
  node->next = ipl_head;
  ipl_head = node;
  return 0;
}

int IpList::Pop(struct sockaddr_in *psin)
{
  IPLIST *node = (IPLIST*) 0;
  if(!ipl_head) return -1;
  node = ipl_head;
  ipl_head = ipl_head->next;

  count--;
  memcpy(psin, &node->address, sizeof(struct sockaddr_in));
  delete node;
  return 0;
}
