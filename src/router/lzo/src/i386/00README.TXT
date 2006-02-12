
Directory overview:
===================
  src           assembler sources for gcc/gas
  d_asm1        sources converted for masm/tasm/wasm
  d_asm2        sources converted for masm/tasm/wasm (in a `db' format)
  d_asm3        sources converted for nasm (in a `db' format)


Notes:
======

- The assembler sources are designed for a flat 32 bit memory model
  running in protected mode - they should work with most i386
  32-bit compilers.

- All functions expect a `cdecl' (C stack based) calling convention.
  The function return value will be placed into `eax'.
  All other registers are preserved.

- There are no prototypes for the assembler functions - copy them
  from ltest/asm.h if you need some.

- For reasons of speed all fast assembler decompressors (having `_fast'
  in their name) can access (write to) up to 3 bytes past the end of
  the decompressed (output) block. Data past the end of the compressed
  (input) block is never accessed (read from).
  See also LZO.FAQ

- The assembler functions are not available in a Windows or OS/2 DLL because
  I don't know how to generate the necessary DLL export information.

- You should prefer the sources in `d_asm2' over those in `d_asm1' - many
  assemblers insert their own alignment instructions or perform some
  other kinds of "optimizations".

- Finally you should test if the assembler versions are actually faster
  than the C version on your machine - some compilers can do a very good
  optimization job, and they also can optimize the code for a specific
  processor type.

