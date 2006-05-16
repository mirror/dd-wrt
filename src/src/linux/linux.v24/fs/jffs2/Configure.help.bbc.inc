ARMLIB compression support for BBC (EXPERIMENTAL)
CONFIG_JFFS2_BBC_ARMLIB
  This enables ARMLIB support for BBC.

LZO1X-* compression support for BBC (EXPERIMENTAL)
CONFIG_JFFS2_BBC_LZO
  This enables LZO1X-1 and LZO1X-999 support for BBC. (fast & good
  compressor, beats ZLIB in everything)

LZARI compression support for BBC (EXPERIMENTAL)
CONFIG_JFFS2_BBC_LZARI
  This enables LempelZiv-Storer-Szymanski compression for BBC with
  additional arithmetic coding (damn slow, but best compresor).

LZHD compression support for BBC (EXPERIMENTAL)
CONFIG_JFFS2_BBC_LZHD
  This enables LempelZiv-Storer-Szymanski compression for BBC with
  additional dynamic Huffman coding (a little faster than LZARI, and
  it's compression ratio is a little worse than LZARI's)

LZSS compression support for BBC (EXPERIMENTAL)
CONFIG_JFFS2_BBC_LZSS
  This enables simple LempelZiv-Storer-Szymanski compression for BBC
  (faster than LZHD, and, and has a not-so-good compression ratio,
  was included just for testing)
