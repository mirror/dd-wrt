#ifndef SLICE_H
#define SLICE_H

#include <sys/types.h>
#include "btcontent.h"
#include "bitfield.h"

typedef struct _slice{
   size_t index;
   size_t offset;
   size_t length;
   struct _slice *next;
}SLICE,*PSLICE;

class RequestQueue
{
 private:
  PSLICE rq_head;
 public:

  RequestQueue();
  ~RequestQueue();

  void Empty();

  void SetHead(PSLICE ps);
  PSLICE GetHead() const { return rq_head; }
  size_t GetRequestIdx(){ return rq_head ? rq_head->index : BTCONTENT.GetNPieces(); }
  size_t GetRequestLen(){ return rq_head ? rq_head->length : 0; }
  void Release(){ rq_head = (PSLICE) 0; }
  int IsValidRequest(size_t idx,size_t off,size_t len);

  void operator=(RequestQueue &rq);

  int IsEmpty() const { return rq_head ? 0 : 1; }

  int Add(size_t idx,size_t off,size_t len);
  int Remove(size_t idx,size_t off,size_t len);

  int Pop(size_t *pidx,size_t *poff,size_t *plen);
  int Peek(size_t *pidx,size_t *poff,size_t *plen) const;

  int CreateWithIdx(size_t idx);
  size_t NSlices(size_t idx) const;
  size_t Slice_Length(size_t idx,size_t sidx) const;
};

#define PENDING_QUEUE_SIZE 100

class PendingQueue
{
 private:
  PSLICE pending_array[PENDING_QUEUE_SIZE];
  size_t pq_count;
  
 public:
  PendingQueue();
  ~PendingQueue();
  void Empty();
  int Pending(RequestQueue *prq);
  int ReAssign(RequestQueue *prq, BitField &bf);
  int Exist(size_t idx);
};

extern PendingQueue PENDINGQUEUE;

#endif
