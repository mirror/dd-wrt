#include "PipedInOutStreams.h"

HRESULT PipedInStream::Read(void *aData, UINT32 aSize, UINT32* aProcessedSize) {
	FILE* in = stdin;
	*aProcessedSize = fread(aData, 1, aSize, in);
	return S_OK;
}

HRESULT PipedOutStream::Write(const void *aData, UINT32 aSize, UINT32* aProcessedSize) {
	FILE* out = stdout;
	*aProcessedSize = fwrite(aData, 1, aSize, out);
	total += *aProcessedSize;
	return S_OK;
}
