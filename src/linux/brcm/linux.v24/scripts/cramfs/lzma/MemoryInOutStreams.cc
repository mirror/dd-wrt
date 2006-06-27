#include "MemoryInOutStreams.h"
#include <stdlib.h>

HRESULT MemoryInStream::Read(void *aData, UINT32 aSize, UINT32* aProcessedSize) {
	unsigned size_to_move;
	if (in_size <= 0)
		{
		*aProcessedSize = 0;
		return S_OK;
		}
	size_to_move = aSize > in_size ? in_size : aSize;
	memcpy(aData, in_data_ptr, size_to_move);
	in_data_ptr += size_to_move;
	in_size -= size_to_move;
	*aProcessedSize = size_to_move;
	return S_OK;
}

HRESULT MemoryOutStream::Write(const void *aData, UINT32 aSize, UINT32* aProcessedSize) {
	memcpy(out_ptr, aData, aSize);
	*aProcessedSize = aSize;
	total += *aProcessedSize;
	*out_size = total;
	out_ptr += *aProcessedSize;
	return S_OK;
}
