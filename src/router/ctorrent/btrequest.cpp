#include <sys/types.h>
#include <stdlib.h>

#include "btrequest.h"
#include "btcontent.h"
#include "btconfig.h"


static void _empty_slice_list(PSLICE *ps_head)
{
  PSLICE p;
  for(; *ps_head;){
    p = (*ps_head)->next;
    delete (*ps_head);
    *ps_head = p;
  }
}

RequestQueue::~RequestQueue()
{
  if( rq_head ) _empty_slice_list(&rq_head);
}

RequestQueue::RequestQueue()
{
  rq_head = (PSLICE) 0;
}

void RequestQueue::Empty()
{
  if(rq_head) _empty_slice_list(&rq_head);
}

void RequestQueue::SetHead(PSLICE ps)
{
  if( rq_head ) _empty_slice_list(&rq_head);
  rq_head = ps;
}

void RequestQueue::operator=(RequestQueue &rq)
{
  if( rq_head ) _empty_slice_list(&rq_head);
  rq_head = rq.rq_head;
  rq.rq_head = (PSLICE) 0;
}

int RequestQueue::Add(size_t idx,size_t off,size_t len)
{
  size_t cnt = 0;
  PSLICE n = rq_head;
  PSLICE u = (PSLICE) 0;

  for( ; n ; u = n,n = u->next) cnt++; // move to end

  if( cnt >= cfg_req_queue_length ) return -1;	// already full

  n = new SLICE;

#ifndef WINDOWS
  if( !n ) return -1;
#endif

  n->next = (PSLICE) 0;
  n->index = idx;
  n->offset = off;
  n->length = len;

  if( u ) u->next = n; else rq_head = n;

  return 0;
}

int RequestQueue::Remove(size_t idx,size_t off,size_t len)
{
  PSLICE n = rq_head;
  PSLICE u = (PSLICE) 0;

  for( ; n ; u = n, n = u->next){
    if(n->index == idx && n->offset == off && n->length == len ) break;
  }

  if( !n ) return -1;	/* not found */

  if( u ) u->next = n->next; else rq_head = n->next;
  delete n;

  return 0;
}

int RequestQueue::Pop(size_t *pidx,size_t *poff,size_t *plen)
{
  PSLICE n;

  if( !rq_head ) return -1;

  n = rq_head->next;

  if(pidx) *pidx = rq_head->index;
  if(poff) *poff = rq_head->offset;
  if(plen) *plen = rq_head->length;

  delete rq_head;

  rq_head = n;

  return 0;
}

int RequestQueue::Peek(size_t *pidx,size_t *poff,size_t *plen) const
{
  if( !rq_head ) return -1;

  if(pidx) *pidx = rq_head->index;
  if(poff) *poff = rq_head->offset;
  if(plen) *plen = rq_head->length;

  return 0;
}

int RequestQueue::CreateWithIdx(size_t idx)
{
  size_t i,off,len,ns;
  
  if( rq_head ) _empty_slice_list(&rq_head);

  ns = NSlices(idx);

  for( i = off = 0; i < ns; i++){
    len = Slice_Length(idx,i);
    if( Add(idx,off,len) < 0) return -1;
    off += len;
  }

  return 0;
}

size_t RequestQueue::Slice_Length(size_t idx,size_t sidx) const
{
  size_t plen = BTCONTENT.GetPieceLength(idx);

  return (sidx == ( plen / cfg_req_slice_size)) ?
    (plen % cfg_req_slice_size) : 
    cfg_req_slice_size;
}

size_t RequestQueue::NSlices(size_t idx) const
{
  size_t r,n;
  r = BTCONTENT.GetPieceLength(idx);
  n = r / cfg_req_slice_size;
  return ( r % cfg_req_slice_size ) ? n + 1 : n;
}

int RequestQueue::IsValidRequest(size_t idx,size_t off,size_t len)
{
  return ( idx < BTCONTENT.GetNPieces() &&
	   len &&
	   (off + len) <= BTCONTENT.GetPieceLength(idx) &&
	   len <= cfg_max_slice_size) ?
    1 : 0;
}

// ****************************** PendingQueue ******************************

PendingQueue PENDINGQUEUE;

PendingQueue::PendingQueue()
{
  int i = 0;
  for(; i < PENDING_QUEUE_SIZE; i++) pending_array[i] = (PSLICE) 0;
  pq_count = 0;
}

PendingQueue::~PendingQueue()
{
  if(pq_count) Empty();
}

void PendingQueue::Empty()
{
  int i = 0;
  for ( ; i < PENDING_QUEUE_SIZE && pq_count; i++)
    if( pending_array[i] != (PSLICE) 0 ){ 
      _empty_slice_list(&(pending_array[i])); 
      pq_count--; 
    }
}

int PendingQueue::Exist(size_t idx)
{
   int i = 0;
  for ( ; i < PENDING_QUEUE_SIZE && pq_count; i++)
    if( (PSLICE) 0 != pending_array[i] && idx == pending_array[i]->index) return 1;
  return 0;
}

int PendingQueue::Pending(RequestQueue *prq)
{
   int i = 0;

  if( pq_count >= PENDING_QUEUE_SIZE ){
    prq->Empty();
    return -1;
  }

  for( ; i < PENDING_QUEUE_SIZE; i++)
    if(pending_array[i] == (PSLICE) 0){
      pending_array[i] = prq->GetHead();
      prq->Release();
      pq_count++;
      break;
    }

  return 0;
}

int PendingQueue::ReAssign(RequestQueue *prq, BitField &bf)
{
   int i = 0;
  size_t sc = pq_count;
  for( ; i < PENDING_QUEUE_SIZE && sc; i++){
    if( pending_array[i] != (PSLICE) 0){
      sc--;
      if( bf.IsSet(pending_array[i]->index) ){
	prq->SetHead(pending_array[i]);
	pending_array[i] = (PSLICE) 0;
	pq_count--;
	break;
      }
    }
  }
  return 0;
}
