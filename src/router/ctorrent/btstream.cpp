#include <arpa/inet.h>
#include "btstream.h"
#include "msgencode.h"
#include "btconfig.h"

ssize_t btStream::Flush()
{
  return out_buffer.FlushOut(sock);
}

ssize_t btStream::Send_State(unsigned char state)
{
  char msg[H_BASE_LEN + 4];
  *(size_t*)msg = htonl(H_BASE_LEN);
  msg[4] = (char)state;
  return out_buffer.PutFlush(sock,msg,H_BASE_LEN + 4);
}

ssize_t btStream::Send_Have(size_t idx)
{
  char msg[H_HAVE_LEN + 4];
  size_t *p = (size_t*)msg;

  *p = htonl(H_HAVE_LEN);
  msg[4] = (char)M_HAVE;
  p = (size_t*)(msg + 5);
  *p = htonl(idx);

  return out_buffer.PutFlush(sock,msg,H_HAVE_LEN + 4);
}

ssize_t btStream::Send_Bitfield(char *bit_buf,size_t len)
{
  size_t q = htonl(len + 1);
  unsigned char t = M_BITFIELD;
  ssize_t r = out_buffer.Put(sock,(char*)&q,4);
  if(r < 0) return r;
  r = out_buffer.Put(sock,(char*)&t,1);
  if(r < 0) return r;
  return out_buffer.PutFlush(sock,bit_buf,len);
}

ssize_t btStream::Send_Cancel(size_t idx,size_t off,size_t len)
{
  char msg[H_CANCEL_LEN + 4];
  size_t *p = (size_t*)msg;

  *p = htonl(H_CANCEL_LEN);
  msg[4] = M_CANCEL;
  p = (size_t*)(msg + 5);
  *p = htonl(idx); p++;
  *p = htonl(off); p++;
  *p = htonl(len);
  return out_buffer.Put(sock,msg,H_CANCEL_LEN + 4);
}

ssize_t btStream::Send_Piece(size_t idx,size_t off,char *piece_buf,size_t len)
{
  size_t q = htonl(len + H_PIECE_LEN);
  unsigned char t = M_PIECE;
  ssize_t r;

  idx = htonl(idx);
  off = htonl(off);
  if( (r = out_buffer.Put(sock,(char*)&q,4)) < 0 ) return r;
  if( (r = out_buffer.Put(sock,(char*)&t,1)) < 0 ) return r;
  if( (r = out_buffer.Put(sock,(char*)&idx,4)) < 0) return r;
  if( (r = out_buffer.Put(sock,(char*)&off,4)) < 0) return r;
  return out_buffer.PutFlush(sock,piece_buf,len);
}

ssize_t btStream::Send_Request(size_t idx, size_t off,size_t len)
{
  char msg[H_REQUEST_LEN + 4];
  size_t *p = (size_t*) msg;

  *p = htonl(H_REQUEST_LEN);
  msg[4] = (char)M_REQUEST;
  p = (size_t*)(msg + 5);
  *p = htonl(idx); p++;
  *p = htonl(off); p++;
  *p = htonl(len);
  return out_buffer.Put(sock,msg,H_REQUEST_LEN + 4);
}

ssize_t btStream::Send_Keepalive()
{
  size_t i = 0;
  return out_buffer.PutFlush(sock,(char*)&i,4);
}

int btStream::HaveMessage()
{
  // if message arrived.
  size_t r;
  if( 4 <= in_buffer.Count() ){
    r = ntohl(*(size_t*)in_buffer.BasePointer());
    if( (cfg_max_slice_size + H_PIECE_LEN + 4) < r) return -1; //message too long
    if( (r + 4) <= in_buffer.Count() ) return 1;
  }
  return 0; //no message arrived
}

ssize_t btStream::Feed()
{
  return in_buffer.FeedIn(sock);
}

ssize_t btStream::PickMessage()
{
  return in_buffer.PickUp( ntohl((*(int*)in_buffer.BasePointer())) + 4);
}

ssize_t btStream::Send_Buffer(char *buf, size_t len)
{
  return out_buffer.PutFlush(sock,buf,len);
}
