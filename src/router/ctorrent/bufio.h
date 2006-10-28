#ifndef BUFIO_H
#define BUFIO_H

#include <sys/types.h>
#include "def.h"

#ifdef WINDOWS
#include <Winsock2.h>
#endif

class BufIo
{
 private:
  char *b;
  size_t p;
  size_t n;

  int f_socket_remote_closed;

  ssize_t _realloc_buffer();
  ssize_t _SEND(SOCKET socket,char *buf,size_t len);
  ssize_t _RECV(SOCKET socket,char *buf,size_t len);
  
 public:
  BufIo();
  ~BufIo() { if(b){ delete []b; b = (char*) 0;} }


  void Reset(){ p = 0; f_socket_remote_closed = 0;}

  void Close(){
    if( b ){ delete []b; b = (char*) 0; }
    p = n = 0;
  }

  size_t Count() const { return p; } //缓存中现有字节数
  size_t LeftSize() const { return (n - p); }

  ssize_t PickUp(size_t len); //移除缓存中前len个字节

  ssize_t FeedIn(SOCKET sk); //从sk读数据到缓存直到暂时无数据可读或缓冲区满
  ssize_t FlushOut(SOCKET sk); //将缓存中数据写到socket
  ssize_t Put(SOCKET sk,char *buf,size_t len); //将buf内容添加到缓存
  ssize_t PutFlush(SOCKET sk,char *buf,size_t len);

  char *BasePointer(){ return b; }
  char *CurrentPointer() { return ( b + p); }
};

#endif
