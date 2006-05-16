#ifndef __7Z_H
#define __7Z_H
extern "C" {
int compress_lzma_7z(
	unsigned algo,
	unsigned dictionary_size,
	unsigned num_fast_bytesi,
	char *data_in,
	unsigned size_in,
	char *data_out,
	unsigned *size_out) throw ();
bool decompress_lzma_7z() throw ();
};

#endif

