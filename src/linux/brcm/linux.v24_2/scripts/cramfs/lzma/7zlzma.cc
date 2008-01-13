#include "7z.h"
#include "MemoryInOutStreams.h"

#include "LZMAEncoder.h"
#include "LZMADecoder.h"

int compress_lzma_7z(
		      unsigned algo,
		      unsigned dictionary_size,
		      unsigned num_fast_bytes,
		      char *data_in,
		      unsigned size_in,
		      char *data_out,
		      unsigned *size_out) throw ()
{
	try {
		NCompress::NLZMA::CEncoder cc;
		
		if (cc.SetDictionarySize(dictionary_size) != S_OK)
			return false;

		if (cc.SetEncoderNumFastBytes(num_fast_bytes) != S_OK)
			return false;

		if (cc.SetEncoderAlgorithm(algo) != S_OK)
			return false;
		ISequentialInStream* in = new MemoryInStream(data_in, size_in);
		ISequentialOutStream* out = new MemoryOutStream(data_out, size_out);
		
		if (cc.WriteCoderProperties(out) != S_OK)
			return false;

		if (cc.Code(in, out) != S_OK)
			return false;

		return true;
	} catch (...) {
		return false;
	}
}
#if 0
bool decompress_lzma_7z() throw () {
	try {
		NCompress::NLZMA::CDecoder cc;

		ISequentialInStream* in = new MemoryInStream(0,0);
		ISequentialOutStream* out = new MemoryOutStream(0,0);

		if (cc.ReadCoderProperties(in) != S_OK)
			return false;

		if (cc.Code(in, out) != S_OK)
			return false;

		return true;
	} catch (...) {
		return false;
	}
}
#endif
