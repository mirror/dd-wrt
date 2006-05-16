#ifndef __MEMORYINOUTSTREAMS_H
#define __MEMORYINOUTSTREAMS_H

#include "Portable.h"
#include "IInOutStreams.h"
#include <cstdio>
#include <cassert>

class MemoryInStream: public ISequentialInStream
{
public:
	MemoryInStream(const char *data, unsigned size)
		: ISequentialInStream(0, 0)
	{in_size = size;in_data = data;in_data_ptr = data;}
	
	HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);
const char *in_data;
const char *in_data_ptr;
unsigned in_size;
};


class MemoryOutStream: public ISequentialOutStream
{
	unsigned total;
public:
	MemoryOutStream(char *data, unsigned *size)
		: ISequentialOutStream(0, 0), total(0)
	{out_size = size;out_data = data; out_ptr = data;}
	
	bool overflow_get() const { return false; }
	unsigned size_get() const { assert(0); }

	HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);
char *out_data;
char *out_ptr;
unsigned *out_size;
};

#endif
