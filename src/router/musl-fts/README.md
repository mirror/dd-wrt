### musl-fts

The musl-fts package implements the `fts(3)` functions
`fts_open`, `fts_read`, `fts_children`, `fts_set` and `fts_close`,
which are missing in musl libc.

It uses the NetBSD implementation of `fts(3)` to build a static
library `/usr/lib/libfts.a` and the `/usr/include/fts.h` header file.
