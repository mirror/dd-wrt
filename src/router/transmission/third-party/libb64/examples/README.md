libb64 examples
===============

This is a collection of simple examples describing how to use libb64.

At the moment, only the C interface is described.
The C++ interface is used in the `base64/base64.cc` code, if you need more help please read through that source.

Files
-----

- `c-example1.c`: shows how to encode/decode a single string
- `c-example2.c`: shows how to directly encode/decode a file

Note that the examples are very simple, but should illustrate the interface usage adequately.

Targets
-------

The example code compiles using 'make all', which builds the examples and runs the 'test' target.
The test for `c-example2.c` uses diff to compare the original and decoded data directly.

loremgibson.txt?
----------------

The `loremgibson.txt` file is a plain text containing content from here: http://loremgibson.com/
It is used in `c-example2.c` as the input data file.

