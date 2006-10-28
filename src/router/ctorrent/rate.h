#ifndef RATE_H
#define RATE_H

#include <sys/types.h>
#include <time.h>
#include "def.h"

class Rate{
 private:
  time_t m_last_timestamp;
  time_t m_total_timeused;
  u_int64_t m_count_bytes;
 public:
  Rate(){ m_last_timestamp = m_total_timeused = (time_t)0; m_count_bytes = 0; }
  void Reset(){ m_last_timestamp = m_total_timeused = (time_t)0; m_count_bytes = 0;}
  void StartTimer();
  void StopTimer();
  void CountAdd(size_t nbytes);
  void operator=(const Rate &ra);
  u_int64_t Count() const { return m_count_bytes; }
  size_t RateMeasure() const;
  size_t RateMeasure(const Rate &ra) const;
  time_t TimeUsed(const time_t *pnow) const;
};

#endif
