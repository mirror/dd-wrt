libb64: Base64 Encoding/Decoding Routines
======================================

## Changelog ##

Version 2.0.0 Release
---------------------
* Introduce version macros for detection of incompatible API / version
* size_t as argument to allow longer base64 encoded strings
* re-introduce line break functionality this time with line with configurable
* add flags field for encoder to mage it configureable (currently unused)
* add functions to calculate required output buffer maximum lengths
* change in-/out-pointers to void* as we don't need to make assumptions about kind of data

Version 1.4.1 Release
---------------------
* Fix differing prototypes in cencode.h and cdecode.h
* Fix compiler errors due to C++ style "//" comments and `-pedantic` option on gcc

Version 1.4.0 Release
---------------------
* add ARM compatibility by Harry Rostovtsev
* Fix integer overflows in decoder by Jakub Wilk
* Make Visual studio project compile again, use Visual Studio 2013
* switch to warning level 4 and get rid of warnings
* init encoderstate on instantiation to make `encode()` work out of the box
* Make project compile with x64 compiler

Version 1.3.0 Release
---------------------
* Remove newlines in output because json doesn't allow them in string values.

Version 1.2.1 Release
---------------------
* Fixed a long-standing bug in `src/cdecode.c` where `value_in` was not correctly checked against the bounds [0..decoding_size) Thanks to both Mario Rugiero and Shlok Datye for pointing this out.
* Added some simple example code to answer some of the most common misconceptions people have about the library usage.

Version 1.2 Release
-------------------
* Removed the `b64dec`, `b64enc`, encoder and decoder programs in favour of a better example, called `base64`, which encodes and decodes depending on its arguments.
* Created a solution for Microsoft Visual Studio C++ Express 2010 edition, which simply builds the base64 example as a console application.

Version 1.1 Release
-------------------
* Modified `encode.h` to (correctly) read from the `iostream` argument, instead of `std::cin`. Thanks to Peter K. Lee for the heads-up.
* No API changes.

Version 1.0 Release
-------------------
* The current content is the changeset.
