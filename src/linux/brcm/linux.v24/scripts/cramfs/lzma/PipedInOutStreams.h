#ifndef __PIPEDINOUTSTREAMS_H
#define __PIPEDINOUTSTREAMS_H

#include "Portable.h"
#include "IInOutStreams.h"
#include <cstdio>
#include <cassert>

class PipedInStream: public ISequentialInStream
{
public:
	PipedInStream()
		: ISequentialInStream(0, 0)
	{ }
	
	HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);
};


class PipedOutStream: public ISequentialOutStream
{
	unsigned total;
public:
	PipedOutStream()
		: ISequentialOutStream(0,0), total(0)
	{ }
	
	bool overflow_get() const { return false; }
	unsigned size_get() const { assert(0); }

	HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);
};

#endif
