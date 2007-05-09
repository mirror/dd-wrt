#ifndef __STREAM_INBYTE_H
#define __STREAM_INBYTE_H

#include "IInOutStreams.h"
#include <cstdlib>
#include <cassert>

namespace NStream {

class CInByte
{
  BYTE m_nextByte;
  UINT64 m_ProcessedSize;
  BYTE *m_BufferBase;
  UINT32 m_BufferSize;
  BYTE *m_Buffer;
  BYTE *m_BufferLimit;
  ISequentialInStream* m_Stream;
  bool m_StreamIsExhausted;

  bool ReadBlock();

public:
  CInByte(UINT32 aBufferSize = cInputBufferSize);
  ~CInByte();

  void Init(ISequentialInStream *aStream);

  bool ReadByte(BYTE &aByte) {
	  // should never be called
	  assert(0);

      if(m_Buffer >= m_BufferLimit)
        if(!ReadBlock())
          return false;
      aByte = *m_Buffer++;
      return true;
    }

  bool isStreamExhausted() const
  {
	  return m_StreamIsExhausted;
  }

  // changed behaviour: returns old byte, stores new byte.
  BYTE ReadByte()
  {
	  // This is the critical code. what to do if nothing can be read any more?

      if(m_Buffer >= m_BufferLimit)
	  {
		  if(!ReadBlock())
		  {
			  m_StreamIsExhausted = true;
			  // finally, return last byte
			  return m_nextByte;
		  }
	  }
	  BYTE tmp = m_nextByte;
	  m_nextByte = *m_Buffer++;
	  return tmp;
  }

  void ReadBytes(void *aData, UINT32 aSize, UINT32 &aProcessedSize)
    {
      for(aProcessedSize = 0; aProcessedSize < aSize; aProcessedSize++)
        if (!ReadByte(((BYTE *)aData)[aProcessedSize]))
          return;
    }
  bool ReadBytes(void *aData, UINT32 aSize)
    {
      UINT32 aProcessedSize;
      ReadBytes(aData, aSize, aProcessedSize);
      return (aProcessedSize == aSize);
    }
  UINT64 GetProcessedSize() const { return m_ProcessedSize + (m_Buffer - m_BufferBase); }
};

}

#endif
