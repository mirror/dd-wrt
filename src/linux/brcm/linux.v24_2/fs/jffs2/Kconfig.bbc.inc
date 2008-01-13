config JFFS2_BBC_ARMLIB
	bool "ARMLIB compression support for BBC (EXPERIMENTAL)"
	depends on JFFS2_FS && EXPERIMENTAL
	default y
	help
	  This enables ARMLIB support for BBC.

config JFFS2_BBC_LZO
	bool "LZO1X-* compression support for BBC (EXPERIMENTAL)"
	depends on JFFS2_FS && EXPERIMENTAL
	default y
	help
	  This enables LZO1X-1 and LZO1X-999 support for BBC. (fast & good
	  compressor)

config JFFS2_BBC_LZARI
	bool "LZARI compression support for BBC (EXPERIMENTAL)"
	depends on JFFS2_FS && EXPERIMENTAL
	default y
	help
	  This enables LempelZiv-Storer-Szymanski compression for BBC with
	  additional arithmetic coding (damn slow, but best compresor).

config JFFS2_BBC_LZHD
	bool "LZHD compression support for BBC (EXPERIMENTAL)"
	depends on JFFS2_FS && EXPERIMENTAL
	default y
	help
	  This enables LempelZiv-Storer-Szymanski compression for BBC with
	  additional dynamic Huffman coding (a little faster than LZARI, and
	  it's compression ratio is a little worse than LZARI's)

config JFFS2_BBC_LZSS
	bool "LZSS compression support for BBC (EXPERIMENTAL)"
	depends on JFFS2_FS && EXPERIMENTAL
	default y
	help
	  This enables simple LempelZiv-Storer-Szymanski compression for BBC
	  (faster than LZHD, and, and has a not-so-good compression ratio,
	  was included just for testing)
