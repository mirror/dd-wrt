#ifndef __IINOUTSTREAMS_H
#define __IINOUTSTREAMS_H

#include "Portable.h"

// same buffer sizes as gzip
const int cOutputBufferSize = 0x4000;
const int cInputBufferSize = 0x800;

// 7zip default: buffers are far too big for linux, it can easily handle
// much smaller buffers without performance penality.
/*
const int cInputBufferSize = 0x100000;
const int cOutputBufferSize = 1<<20;
*/

class ISequentialInStream
{
	const char* data;
	unsigned size;
public:
	ISequentialInStream(const char* Adata, unsigned Asize) : data(Adata), size(Asize) { }

	virtual HRESULT Read(void *aData, UINT32 aSize, UINT32 *aProcessedSize);
};

class ISequentialOutStream
{
	char* data;
	unsigned size;
	bool overflow;
	unsigned total;
public:
	ISequentialOutStream(char* Adata, unsigned Asize) : data(Adata), size(Asize), overflow(false), total(0) { }

	virtual bool overflow_get() const { return overflow; }
	virtual unsigned size_get() const { return total; }

	virtual HRESULT Write(const void *aData, UINT32 aSize, UINT32 *aProcessedSize);
};

#endif
