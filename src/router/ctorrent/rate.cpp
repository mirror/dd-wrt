#include "rate.h"

void Rate::StartTimer()
{
  if( !m_last_timestamp ) time(&m_last_timestamp);
}

void Rate::StopTimer()
{
  if( !m_last_timestamp ){
    m_total_timeused += (time((time_t*) 0) - m_last_timestamp);
    m_last_timestamp = 0;
  }
}

void Rate::CountAdd(size_t nbytes)
{
  m_count_bytes += nbytes;
}

void Rate::operator=(const Rate &ra)
{
  m_last_timestamp = time((time_t*) 0);
  m_count_bytes = ra.m_count_bytes;
}

size_t Rate::RateMeasure() const
{
  time_t timeused = m_total_timeused;
  if( m_last_timestamp ) timeused += (time((time_t*) 0) - m_last_timestamp);
  if( timeused < 1 ) timeused = 1;
  return (size_t)(m_count_bytes / timeused);
}

size_t Rate::RateMeasure(const Rate &ra_to) const
{
  time_t timeused = time((time_t*) 0) - m_last_timestamp;
  if( timeused < 1 ) timeused = 1;
  return (size_t)((ra_to.m_count_bytes - m_count_bytes) / timeused);
}

time_t Rate::TimeUsed(const time_t *pnow) const
{
  return (*pnow - m_last_timestamp);
}
