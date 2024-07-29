# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [2.2.0] - 2024-03-29
### Added
 - flashcp: Add write last option
 - flash_erase: Add an option for JFFS2 cleanmarker size
 - ubiattach: Add disable fastmap option
 - ubiattach: Add option to reserve peb pool for fastmap
 - support building without zlib
 - CHANGELOG & README files

### Fixed
 - jffs2dump: check return value of lseek
 - mkfs.ubifs: fix xattr scanning for builds with selinux support

### Changed
 - overhaul dependency handling in the build system

## [2.1.6] - 2023-08-30
### Added
 - flash_speed: Measure read while write latency
 - Support `mtd:<num>` syntax for several tools

### Fixed
 - flashcp: check for lseek errors
 - flashcp: fix buffer overflow
 - flashcp: verify data in `--partition`
 - flashcp: abort on `--partition` and `--erase-all`
 - flashcp: correct casting for percent display
 - mtdpart: document partition of size 0
 - mkfs.ubifs: Non-terminated string related failure with option selinux
 - nandtest: handle nand devices larger than 4G
 - Fix printf format specifiers for 64 bit integer types

### Changed
 - flashcp: merge duplicate write code paths
 - flashcp: merge duplicate MEMERASE code paths
 - flashcp: simplify logging

## [2.1.5] - 2022-10-07
### Fixed
 - mkfs.jffs2: spelling of `--compression-mode` parameter in help text
 - ubinfo: `--vol_id` return code for absent volume id
 - nandflipbits: fix corrupted oob
 - libmtd: do not ignore non-zero eraseblock size when `MTD_NO_ERASE` is set
 - jffs2reader: warning about unaligned pointer
 - tests: Remove unused linux/fs.h header from includes
 - fix test bashism
 - nanddump: fix writing big images on 32bit machines
 - nor-utils: fix memory leak

### Changed
 - flash_otp_dump: make offset optional
 - nandwrite: warn about writing 0xff blocks

## [2.1.4] - 2022-10-07
### Added
 - ubiscan debugging and statistics utility

### Fixed
 - Some mtd-tests erroneously using sub-pages instead of the full page size
 - Buffer overrun in fectest
 - Build failures due to missing jffs2 kernel header

## [2.1.3] - 2021-07-25
### Added
 - flashcp: Add new function that copy only different blocks
 - flash_erase: Add flash erase chip
 - Add flash_otp_erase
 - Add an ubifs mount helper
 - Add nandflipbits tool

### Fixed
 - mkfs.ubifs: Fix runtime assertions when running without crypto
 - Use AC_SYS_LARGEFILE
 - Fix test binary installation
 - libmtd: avoid divide by zero
 - ubihealthd: fix UBIFS build dependency
 - mkfs.ubifs: remove `OPENSSL_no_config()`
 - misc-utils: Add fectest to build system
 - mkfs.ubifs: Fix build with SELinux
 - Fix typos found by Debian's lintian tool
 - Fix jffs2 build if zlib or lzo headers are not in default paths

## [2.1.2] - 2020-07-13
### Added
 - flashcp: Add option `-A`, `--erase-all`
 - mtd-utils: add optional offset parameter to `flash_otp_dump`
 - ubi-utils: Implement a ubihealthd
 - mkfs.ubifs: Add authentication support

### Fixed
 - mtd-utils: Fix return value of ubiformat
 - ubiupdatevol: Prevent null pointer dereference
 - libubigen: remove unnecessary include
 - libubi: remove private kernel header from includes
 - mkfs.ubifs: fscrypt: bail from encrypt_block if gen_essiv_salt fails
 - mkfs.ubifs: abort add_directory if readdir fails
 - mkfs.ubifs: close file descriptor in add_file error path
 - mkfs.ubifs: don't leak copied command line arguments
 - mkfs.ubifs: free derived fscrypt context in add_directory error paths
 - mkfs.ubifs: don't leak hastable iterators
 - mkfs.ubifs: don't leak temporary buffers
 - mkfs.ubifs: propperly cleanup in ALL interpret_table_entry error paths
 - mkfs.jffs2: don't leak temporary buffer if readlink fails
 - libmtd: don't leak temporary buffers
 - ftl_check: don't leak temporary buffers
 - ftl_format: don't leak temporary buffers
 - ubiformat: don't leak file descriptors
 - nanddump: don't leak copied command line arguments
 - mtd_debug: cleanup error handling in flash_to_file
 - jittertest: fix error check for open system call
 - fs-tests: don't leak temporary buffers
 - mtd-utils: Fix printf format specifiers with the wrong type
 - mtd-utils: Fix potential negative arguments passed to close(2)
 - mtd-utils: Fix various TOCTOU issues
 - mtd-utils: Fix some simple cases of uninitialized value reads
 - mtd-utils: Fix wrong argument to sizeof in nanddump
 - mtd-utils: Fix "are we really at EOF" test logic in libubi read_data
 - mtd-utils: Fix potentially unterminated strings
 - mtd-utils: Add checks to code that copies strings into fixed sized buffers
 - mkfs.ubifs: fix broken build if fscrtyp is disabled

### Changed
 - ubifs-media: Update to Linux-5.3-rc3

## [2.1.1] - 2019-07-21
### Added
 - mkfs.ubifs: Add ZSTD compression

### Fixed
 - ubiformat: Dont ignore sequence number CLI option
 - mkfs.ubifs: fix build without openssl
 - mkfs.ubifs: fix regression when trying to store device special files
 - mkfs.ubifs: fix description of favor_lzo
 - unittests/test_lib: Include proper header for `_IOC_SIZE`
 - unittests/libmtd_test: Include fcntl header
 - unittests: Define the use of `_GNU_SOURCE`
 - ubinize: Exit with non-zero exit code on error.
 - mtd-tests: nandbiterrs: Fix issue that just insert error at bit 7
 - ubi-tests: ubi_mkvol_request: Fully initialize `struct ubi_mkvol_request req`
 - ubi-tests: io_read: Filter invalid offset before `lseek` in `io_read` test
 - ubi-tests: mkvol test: Checks return value `ENOSPC` for `ubi_mkvol`
 - ubi-tests: fm_param: Replace `fm_auto` with `fm_autoconvert`

## [2.1.0] - 2019-03-19
### Added
 - mkfs.ubifs: Implement support for file system encryption
 - mkfs.ubifs: Implement selinux labelling support
 - ubinize: add support for skipping CRC check of a static volume when opening
 - ubimkvol: add support for skipping CRC check of a static volume when opening
 - Add lsmtd program

### Fixed
 - update various kernel headers
 - Instead of doing preprocessor magic, just output off_t as long long
 - fix verification percent display in flashcp
 - mkfs.ubifs: fix double free
 - mkfs.ubifs: Fix xattr nlink value
 - ubinize: avoid to create two `UBI_LAYOUT_VOLUME_ID` volume
 - common.h: fix prompt function
 - libmtd: don't print an error message for devices without ecc support
 - io_paral: Fix error handling of update_volume()
 - ubimkvol: Initialize req to zero to make sure no flags are set by default
 - libubi: add volume flags to `ubi_mkvol_request`
 - mkfs.ubifs: add_xattr is not depending on host XATTR support
 - Revert "Return correct error number in ubi_get_vol_info1" which
   introduced a regression.
 - make sure pkg-config is installed in configure script
 - ubiformat: process command line arguments before handling file arguments

### Changed
 - ubiformat: remove no-volume-table option

## [2.0.2] - 2018-04-16
### Added
 - libmtd: Add support to access OOB available size
 - mkfs.ubifs: Allow root entry in device table

### Fixed
 - Fix unit-test header and file paths for out of tree builds
 - Fix unit test mockup for oobavail sysfs file
 - misc-utils: flash_erase: Fix Jffs2 type flash erase problem
 - libmtd_legacy: Fix some function description mismatches
 - mtd-utils: ubifs: fix typo in without_lzo definition
 - mtd: tests: check erase block count in page test
 - mtd: unittests: Stop testing stat() calls
 - mtd: unittests: Decode arg size from ioctl request
 - mtd: unittests: Use proper unsigned long type for ioctl requests
 - mtd: tests: Fix check on ebcnt in nandpagetest
 - ubi-utils: ubicrc32: process command line arguments first
 - nandbiterrs: Fix erroneous counter increment in for loop body
 - jittertest: Use the appropriate versions of abs()
 - Mark or fix switch cases that fall through
 - mkfs.ubifs: ignore EOPNOTSUPP when listing extended attributes
 - misc-utils: initialize "ip" in docfdisk to NULL
 - mkfs.ubifs: Apply squash-uids to the root node

### Changed
 - ubi-utils: ubiformat.c: convert to integer arithmetic
 - mtd-utils: common.c: convert to integer arithmetic
 - Run unit test programs through "make check"
 - Enable more compiler warning flags, fix warnings
 - Add no-return attribute to usage() style functions
 - Remove self-assignments of unused paramters
 - tests: checkfs: Remove unused source file from makefiles
 - ubi-tests: io_update: fix missleading indentation
 - Add ctags files to .gitignore
 - libscan: fix a comment typo in libscan.h
 - libmtd: fix a comment typo in dev_node2num

## [2.0.1] - 2017-08-24
### Added
 - nandbiterrs: Add Erased Pages Bit Flip Test
 - mkfs.ubifs: Add support for symlinks in device table
 - nanddump: Add `--skip-bad-blocks-to-start` option
 - nandwrite: Add `--skip-bad-blocks-to-start` option

### Fixed
 - common: Always terminate with failure status if command line options
   are unknown or missing
 - common: Fix format specifier definitions for `off_t` and `loff_t`
 - common: More consistent exit codes
 - libmtd: Fix error status if MTD is not present on the system
 - libubi: Add klibc specific fixes for `ioctl`
 - libubi: Fix error status in `ubi_get_vol_info1` for non-existing volumes
 - misc-utils: Support jffs2 flash-erase for large OOB (>32b)
 - mkfs.jffs2: Add missing header inclusions required for build with musl
 - mkfs.ubifs: Fix alignment trap triggered by NEON instructions
 - mkfs.ubifs: Fix uuid.h path
 - mkfs.ubifs: Replace broken ubifs_assert with libc assert
 - nandbiterrs: Actually get the new ECC bit flip count before comparing stats
 - nandpagetest: Improved argument sanity checking
 - nandwrite: Fix bad block skipping
 - nandwrite: Improved argument sanity checking
 - ubinfo: Improved argument sanity checking
 - ubi-tests: Replace variable-length array with `malloc`
 - ubi-tests: Support up to 64k NAND page size

### Changed
 - build-system: Enable compiler warnings
 - build-system: Restructure autoconf dependency checking
 - common: Add const modifier to read only strings and string constants
 - common: Eliminate warnings about missing prototypes
 - common: Get rid of rpmatch usage
 - common: Remove README.udev from ubi-tests extra dist
 - common: Remove unused variables and functions
 - common: Silence warnings about unused arguments
 - flashcp: Drop custom defines for `EXIT_FAILURE` and `EXIT_SUCCESS`
 - libiniparser: remove unused function needing float
 - libmissing: Use autoconf header detection directly
 - libubi: Remove `UDEV_SETTLE_HACK`
 - misc-utils: Move libfec to common public header & library directory
 - nandwrite: replace erase loop with mtd_erase_multi
 - serve_image: Use PRIdoff_t as format specifier.
 - ubi-tests: Speedup io_paral by using rand_r()
 - ubirename: Fix spelling

## [2.0.0] - 2016-12-22
### Added
 - libmissing with stubs for functions not present in libraries like musl
 - unittests for libmtd and libubi
 - port most kernel space mtd test modules to userspace
 - mkfs.ubifs: extended attribute support
 - ubinize: Move lengthy help text to a man page
 - nandwrite: Add skip-all-ff-pages option
 - flash_{un,}lock: support for MEMISLOCKED
 - nandtest: support hex/dec/oct for `--offset` and `--length`

### Fixed
 - common: Fix 'unchecked return code' warnings
 - common: Fix PRI{x,d}off definitions for x86_64 platform
 - common: include sys/sysmacros.h for major/minor/makedev
 - common: fix wrong format specifiers on mips32
 - libmtd: Fix uninitialized buffers
 - libmtd: Eliminate warnings about implicit non-const casting
 - libmtd: Fix return status in mtd_torture test function
 - libmtd: mtd_read: Take the buffer offset into account when reading
 - mkfs.ubifs: use gid from table instead 2x uid
 - mkfs.ubifs: fix compiler warning for WITHOUT_LZO
 - mkfs.ubifs: fix build when WITHOUT_LZO is set
 - mkfs.ubifs: correct the size of nnode in memset
 - mkfs.jffs2: initialize lzo decompression buffer size
 - mkfs.jffs2: Fix scanf() formatstring for modern C version
 - nanddump: check write function result for errors
 - nanddump: write requested length only
 - flash_{un,}lock: don't allow "last byte + 1"
 - flash_{un,}lock: improve strtol() error handling
 - ubinize: Always return error code (at least -1) in case of an error
 - recv_image: fix build warnings w/newer glibc & _BSD_SOURCE
 - serve_image: use proper POSIX_C_SOURCE value
 - flashcp: Use %llu to print filestat.st_size
 - mtd_debug: check amount of data read.
 - fs-tests: integrity: don't include header <bits/stdio_lim.h>
 - tests: Fix endian issue with CRC generation algorithm
 - make_a_release.sh: fix MTD spelling
 - Fix packaging of unit test files
 - Correct casting for final status report in flashcp

### Changed
 - autotools based build system
 - complete restructuring of the source tree
 - cleanup of some utilities
 - removal of some very old, unused or duplicated files from the source tree
 - libmtd: removal of very old, completely unused and broken functions
 - nandwrite: Factor out buffer checking code

## [1.5.2] - 2015-07-24
### Added
 - mtdpart utility to add/delete partition
 - jffs2dump: XATTR and XREF support for content dump & endianess conversion

### Fixed
 - ubifs-media.h: include "byteorder.h"
 - ubiformat: fix the subpage size hint on the error path
 - include/common.h: fix build against current uClibc
 - include/common.h: fix build against recent 0.9.33 uClibc
 - libmtd: fix comment typo
 - mkfs.ubifs: Fix build with gcc 5.1
 - libmtd: don't ignore "region index" parameter in `mtd_regioninfo()`
 - .gitignore: add new mtdpart utility
 - ubi-tests: fix a some overflows
 - formating issues (trailing whitespaces, space-before-tab, blank line at EOF)
 - fs-tests: pass `TEST_DIR` to integck in run_all.sh
 - nandtest: fix `--reads` argument
 - libmtd: fix `mtd_dev_present` return value on legacy systems
 - libfec: use standard C type instead of `u_long`
 - serve_image: do not include error.h
 - recv_image: do not include error.h
 - include/common.h: fix build against musl
 - integck: Use `$(CC)` and `$(AR)` instead of `gcc` and `ar` consistently

### Changed
 - mkfs.ubifs: start using common code, move some macros to common header
 - Move mkfs.ubifs/ubifs-media.h to include/mtd
 - make_a_release.sh: suggest announcement e-mail
 - nandtest: Remove redundant check
 - nandtest: Move the "read and compare" code to a function
 - nandtest: Introduce multiple reads & check iterations

## [1.5.1] - 2014-04-07
### Added
 - tests: ubi: add stress-test.sh
 - ubi-utils: Add ubiblock tool
 - flash_erase, flash_otp_write: check the nand type
 - ubiupdatevol: add a `--skip` option
 - nandwrite: add `--input-{skip,size}` options
 - mkfs.ubifs: allow reformatting of devices
 - new prompt() helper for talking to the user

### Fixed
 - Makefile: add LDFLAGS_mkfs.ubifs, fix libuuid issue
 - runtests.sh: print more consistent messages
 - integck: fix identation
 - nandwrite: apply corrent version of `--input-*` patch series
 - ubi-tests: argument number mismatch for `ubi_leb_change_start`
 - ubi-tests: fix pthreads linking
 - ubiformat: correct "non-ubifs" warning message
 - mkfs.ubifs: correct and improve LEB size error prints
 - ubi-utils: Fix file descriptor leaks in libubi
 - integck.c: Fix buffer overflow in save_file
 - integck.c: Only verify the operation after datastructures have been updated
 - mkfs.ubifs: Improve error handling of is_contained()
 - nandwrite: clean up length types
 - nand{dump, test, write}: clean up `--help` handling
 - fix build errors w/newer kernel headers & glibc
 - ubinize: fix usage text
 - consistency between `u_int32_t` / `off_t` / `off64_t`
 - tests: io_update correct lseek parameters order
 - ubiformat: fix failure on big partitions (>4Gio)
 - Check mtdoffset is not larger than mtd.size in case of a bad block.
 - flash_otp_write: fix writing to NAND in presence of partial reads
 - flash_otp_write: fix a buffer overflow on NAND with write size > 2048
 - ubiformat: really skip some messages when quiet
 - ubiformat: fix error path

### Changed
 - nandwrite: minor cleanups
 - tests: ubi: clean-up the runtests.sh script
 - libubi: Remove `ubi_attach_mtd`
 - load_nandsim.sh: switch to sh, remove bashisms, cleanup code
 - load_nandsim.sh: introduce a usage function
 - load_nandsim.sh: intruduce fatal function
 - load_nandsim.sh: use dochere for help output
 - mkfs.ubifs: rewrite path checking
 - ubi-tests: switch to using common.h from the top level
 - ubi-tests: rename common.[ch] to helpers.[ch]
 - integck.c: immediately dump the buffer containing the errors
 - use xstrdup in a few more places
 - move `_GNU_SOURCE` to the main makefile
 - ftl_check/ftl_format/nftldump: use existing mtd_swab.h header
 - flash_erase: use `pwrite()` rather than `lseek()` and `write()`
 - UBI: sync ubi-user.h with kernel v3.6-rc1
 - introduce `PRIxoff_t` and `PRIdoff_t` printf helpers
 - ubiattach: use `max_beb_per1024` in `UBI_IOCATT` ioctl
 - ubiattach: fail if kernel ignores max_beb_per1024
 - libubi: factorize `ubi_attach` and `ubi_attach_mtd` code
 - Makefile: Build and install `flash_otp_lock`, `flash_otp_write`
 - ubiformat: clean up synopsis of command-line parameters

## [1.5.0] - 2012-05-07
### Added
 - limbtd: implement `mtd_dev_present` for older kernels
 - libubi: make `ubi_dev_present()` a library function
 - libmtd: add `mtd_dev_present()` library function

### Fixed
 - mkfs.ubifs: do not ignore `--max-leb-cnt` when formatting an UBI volume
 - ubinfo: fix `--all` for non-consecutive device numbers
 - mtdinfo: fix `--all` for non-consecutive device numbers
 - libmtd: fix segmentation fault on lib->mtd
 - mtdinfo: correct grammar on error message
 - libmtd: fix `mtd_write()` issues for large data-only writes
 - libmtd: check if the device is present before accessing it
 - libmtd_legacy: don't open device in R/W
 - Makefile: fix "make clean" for old GNU find
 - libmtd: Variable name same as function name causing compile to fail (Android)

### Changed
 - make_a_release.sh: remind about pushing the master branch
 - docfsdisk: minor cosmetic cleanup
 - mkfs.jffs2: improve documentation of `--pagesize` parameter

## [1.4.9] - 2011-12-17
### Added
 - Add a script to make releases
 - nandtest: add seed argument

### Fixed
 - nandtest: seed random generator with time
 - libmtd: allow write operations when MEMWRITE is not supported
 - nandtest: set oldstats.failed
 - Makefile: fix install target with out-of-tree builds

### Changed
 - Makefile: separate man page install and compression steps

## [1.4.8] - 2011-11-19
### Fixed
 - Makefile: Version number accedientally left at 1.4.6 in last release

## [1.4.7] - 2011-11-18
### Added
 - libmtd: support MEMWRITE ioctl

### Fixed
 - jffs2reader: get rid of linker error
 - jffs2reader: eliminate compiler errors
 - ubiformat: handle write errors correctly
 - mtd-utils: add jffs2reader to .gitignore

### Changed
 - jffs2reader: use major() and minor() helpers
 - jffs2reader: print ctime only by user's request
 - jffs2reader: use const char * for path variables
 - jffs2reader: introduce ADD_BYTES macro
 - jffs2reader: update the header inclusion block
 - nandtest: print number of bits corrected during test
 - mkfs.ubifs/ubinize: increase PEB size limit to 2MiB
 - mtdinfo: provide info when used without arguments
 - nandwrite: use common.h "errmsg" functions
 - nandwrite: re-implement `--autoplace` option
 - move OOB auto-layout into libmtd's mtd_write
 - nandwrite: merge `mtd_write_oob` and `mtd_write` calls
 - libmtd: modify `mtd_write` to cover OOB writes
 - update mtd-abi.h
 - nandwrite: consolidate buffer usage
 - nanddump: kill usages of MEMSETOOBSEL ioctl
 - nandwrite: refactor "old_oobinfo" code
 - nandwrite: cleanup "oobinfochanged" leftovers
 - nandwrite: remove C99 comment style
 - use `__func__` instead of `__FUNCTION__`
 - mtd_debug: replace #defines with enum
 - mtd_debug: fixup style

### Removed
 - tests: checkfs: remove unused code
 - nandwrite: kill `--raw` option
 - nandwrite: kill -j, -y, and -f options
 - nandwrite: remove `autoplace` features

## [1.4.6] - 2011-08-20
### Fixed
 - mkfs.ubifs: fix a gcc warning
 - integck: fix build error (MS_DIRSYNC, MS_RELATIME)
 - mtd-tests: checkfs: fix size_t related warning
 - mtd-tests: io_paral: build error, "variable length strings"
 - flash_erase: fix incorrect help message
 - Makefile: fix "version.h" build for cross-compiling
 - Makefile: fix "make clean" for cross-compile
 - mtdinfo: don't open NULL pointer when getting region_info with `-a'
 - mtdinfo: refactor code to remove "args.all" dependency
 - mtdinfo: fixup "example usage" help section
 - nandwrite: invalid erase after page write failure

### Changed
 - nanddump: change default to `--bb=skipbad`
 - nanddump: change `-o` to mean `--oob`, not `--omitoob`
 - nanddump: default to NOT dumping OOB data
 - mkfs.ubifs: use common.h
 - switch more utils to unified versioning
 - add common version printing function
 - mtdinfo: restructure help message
 - mtdinfo: consolidate help as display_help()
 - rewrite build system to avoid recursion
 - autogenerate version.h from build system

### Removed
 - mtdinfo: remove -m leftovers
 - mkfs.ubifs: remove root inode squash feature
 - nanddump: kill `--omitbad`, `--noskipbad`
 - kill flash_info
 - mtdinfo: kill `-m` option
 - build: remove old SYMLINKS variable

## [1.4.5] - 2011-07-25
### Added
 - Makefile: introduce new target tests in Makefile
 - Makefile: introduce cscope target
 - nanddump: add `--oob` option
 - nanddump: add `--bb=METHOD` option
 - mkfs.ubifs: add `-F` option for `free-space fixup`
 - libmtd: add helper funcs for getting regioninfo and locked info
 - mtdinfo: add regioninfo/eraseblock map display
 - flash_info: allow people to get info on multiple devices

### Fixed
 - serve_image: adjust classifier and type for printf
 - libmtd: use PRIu64 classifier for uint64_t printf arguments
 - mkfs.jffs2: fix casting of __off64_t
 - tests: ubi-tests: clean libubi.a and *.o
 - tests: fs-tests: ssize_t return type in read() wrapper
 - mkfs.jffs2: fix casting of printf argument
 - tests: jittertest: fix set of compiler warnings
 - tests: ubi-tests: seed_random_generator() was used w/o prototype
 - tests: fs-tests: check return value of functions
 - tests: checkfs: fix linker warnings
 - tests: checkfs: fix compiler warnings
 - tests: make jittertest buildable
 - mkfs.ubifs: check output first
 - flash_{lock, unlock}: fix off-by-one error for "entire device" length
 - fs-tests: use independent random generators for ops and data
 - ubi-utils: ubimkvol: fix parameters parsing regression

### Changed
 - tests: checkfs: integerate with common Makefile
 - fs-tests: integck: major cleanup/rewrite
 - mtdinfo: deprecate the -m option
 - libmtd: improve mtd_islocked interface
 - switch ubi and ubifs tools to use common strtoX funcs
 - mtdinfo: separate out ubi information printing
 - fs_tests: make the test-suite finish faster
 - nanddump: document, warn about future default `--omitoob`
 - feature-removal-schedule: describe nanddump changes
 - nanddump: warn about new default BB handling
 - nanddump: update help message for BB method changes
 - nanddump: sort options in help message alphabetically by shortname
 - jffs2: make lzo optional at build time
 - flash_info: deprecate
 - ubi-utils: send help/version info to stdout
 - libmtd: use O_CLOEXEC
 - include/mtd: sync with kernel
 - flash_info: convert to common.h
 - ignore (FLAT) gdb files
 - flash_{lock,unlock}: convert to common code
 - flash_{lock,unlock}: merge into one util
 - flash_{lock,unlock}: merge functionality
 - flash_lock/flash_unlock/flash_info: clean up style

### Removed
 - punt redundant libcrc32
 - mtdinfo: remove now unused ubigen info

## [1.4.4] - 2011-04-01
### Added
 - libmtd: fix OOB read and write interface

### Fixed
 - fs-tests: test_1: fix compilation warnings
 - fs-tests: perf: fix compilation warning
 - fs-tests: integck: improve re-mounting test coverage

### Changed
 - mkfs.ubifs: deprecate `squash-rino-perm` options
 - nanddump: fail if `-s` parameter is unaligned

## [1.4.3] - 2011-03-18
### Added
 - nandwrite: add only write oob option

### Fixed
 - common.h: simple_strtoll type

### Changed
 - flash_erase: start position should be in bytes

## [1.4.2] - 2010-12-31
### Added
 - nandwrite: Large page+oob support
 - nanddump/nandwrite: 64-bit support utilizing libmtd
 - introduce xzalloc() helper
 - add xasprintf() helper
 - new memory wrappers
 - new strtoX helpers
 - new bareverbose() helper
 - libmtd: add lock/unlock helpers
 - nandwrite: add `--skipbad` to write bad blocks

### Fixed
 - nanddump: fix initialization of bad blocks oob data buffer
 - nanddump: always check the first erase block
 - flash_erase: Fix output of offsets
 - nanddump/nandwrite: Style fixups
 - nanddump/nandwrite: style, signed-ness, printing fixups
 - nandwrite: type consistency
 - nandwrite: prevent 32-bit overflow
 - nandwrite: avoid NULL buffer pointers
 - libmtd: fix OOB size initialization in legacy code
 - Makefile: Use $(CURDIR) in place of $(PWD)
 - mkfs.ubifs: Fix heap corruption on LEB overrun
 - mkfs.ubifs: Fix --squash-rino-perm / --nosquash-rino-perm
 - Fix make install errors
 - libmtd: fix "fount" typo
 - sys_errmsg: fix indentation
 - recv_image: fix `__USE_GNU` hack
 - ubi-utils: tweak const strings decls
 - compr_rtime: fix unused warning
 - libfec: fix up pointer warnings in fec magic computation
 - mkfs.jffs2: fix devtable count as mkfs.ubifs does
 - mkfs.jffs2: fix repeated dev nodes
 - mkfs.ubifs: Fix typo in short options of mkfs.ubifs

### Changed
 - mkfs.ubifs: do not override root inode permissions
 - Makefile: remove old ubi-utils
 - Add --squash-rino-perm removal plan
 - nanddump: warn when the start address is not page aligned
 - nanddump: Dynamic buffer, increase pagesize/oobsize
 - nandwrite: Use libmtd to get correct mtd parameters
 - nandwrite: switch "oobsize" for "writesize"
 - nandwrite: Clarify usage of aligned "erasesize"
 - nandwrite: add check for negative blockalign
 - nandwrite: use common.h "errmsg_die"
 - nanddump: choose correct "printf" format-specifier
 - nanddump: check for negative inputs
 - nanddump: change "unsigned" to "signed"
 - nanddump/nandwrite: use "simple_" str functions
 - common.h: Add MAX() macro, fix MIN()
 - nanddump: Refactor pretty print code into an sprintf()
 - jffs2reader: convert to common.h helpers
 - libmtd: make malloc failures fatal
 - sumtool/libfec: convert "()" to "(void)" in func defs
 - sumtool: convert to common.h helpers
 - mkfs.jffs2: convert to common.h and xalloc.h helpers
 - libmtd: unify some error messages
 - xalloc: simplify/unify error messages
 - enable garbage collection of unused function/data sections
 - mkfs.jffs2: use new xasprintf() helper
 - unify flash_erase and flash_eraseall
 - common.h: clean up PROGRAM_NAME usage
 - standardize PROGRAM_NAME
 - mkfs.ubifs: use common ARRAY_SIZE
 - punt duplicate normsg_cont define
 - convert to common.h/min
 - libmtd: unify erase block argument checking
 - compr: drop unused model argument
 - compr_zlib: mark local functions as static
 - rbtree: avoid redefining offsetof

### Removed
 - ftl_check: drop unused verbose flag
 - nandwrite: Remove redundant 'autoplace' check

## [1.4.1] - 2010-10-19
### Changed
 - Hide zlib's crc32 in compr_zlib.c and mkfs.ubifs/compr.c

## [1.4.0] - 2010-09-13
### Added
 - libmtd: add OOB read and write interfaces
 - libmtd: support MEMERASE64
 - lib: Add forgotten Makefile
 - libmtd: add mtd_write_img
 - ubi-tests: add normsg
 - ubinfo: document the new -N option
 - nanddump: add canonical (hex+ascii) flag
 - nanddump: add "forcebinary" flag
 - nanddump: add `--nobad` to read bad blocks
 - libubi: add support to attach/detach by MTD device path
 - ubiattach/ubidetach: add support to attach/detach by path
 - nanddump: Support 4096+218 and 4096+224 page sizes
 - ubinfo: add -N option to get info by name of ubi volume
 - support 4096+64 page sizes

### Fixed
 - mkfs.jffs2: fix `--enable-compressor`
 - lib: fix libcrc32 generation
 - rename crc32 to mtd_crc32
 - mkfs.ubifs: fix compilation warning
 - mkfs.jffs2: fixed warnings
 - nandtest: Fixed indentation
 - fix spelling error
 - fix compiler warnings
 - nandwrite: fix the bug of writing a yaffs2 image to NAND
 - nandwrite: check if the start address is page-aligned
 - nandtest: fix `--keep` argument
 - fix parallel build between ubi-utils and mkfs.ubifs
 - clean up compile warnings
 - ubi-utils: reformat help text to fit in 80 columns
 - nanddump: Fix hexdump nybble ordering
 - mtd_debug: fix creation mode parameter
 - mkfs.jffs2: fix integer underflow in jffs2_rtime_compress()

### Changed
 - libs: remove ubiutils-specific stuff from common.h
 - libs: make crc32 and fec to be libraries
 - libscan: rename hdr to ech
 - ubi-test: seed the random genrator in tests
 - ubi-tests: use rand instead of random in io_paral
 - ubi-utils: harmonize mtd device node variables
 - ubi-utils: harmonize libmtd interface a bit
 - ubi-tests: remove some junk from the integ test
 - libubigen: make init_vid_hdr externally visible
 - libubigen: move header comments
 - libubigen: do not create huge arrays on stack
 - ubi-tests: teach errmsg and failed return error code
 - ubi-tests: rename err_msg to errmsg
 - libubi: provide mtd number in UBI device information
 - libubi: remove few fields from volume info
 - nanddump: increase max OOB size
 - nanddump: rename `--nobad` to `--noskipbad`
 - build: add option to not force largefile support
 - ubi-utils: drop -Werror in old utils
 - flash_eraseall: tweaks to make binary size smaller
 - mtd: change flash_eraseall to use libmtd-wrapped ioctls
 - mtd-utils: update to latest mtd-abi.h from kernel.org
 - move libmtd source files to lib/ subdirectory
 - ubi-utils: provide default value for /dev/ubi_ctrl

### Removed
 - nanddump: drop unused --ignoreerrors option

## [1.3.1] - 2010-01-15
### Fixed
 - ubiformat: always initialize seq number
 - ubiformat: be consistent with sequence numbers

## [1.3.0] - 2009-12-09
### Added
 - Makefile: Add Optional ZLIB and LZO CPPFLAGS and LDFLAGS
 - Add mkfs.ubifs
 - Add support for 4k pages
 - ubirmvol: remove volume by name
 - nandwrite: Support reading from standard input
 - nanddump: Add Support for Quiet Option
 - ubinize: allow an absent 'image' in the ubinize configuration
 - handle non-power-of-2 erase size
 - add ubirename utility
 - ubiformat: mark faulty blocks as bad
 - ubi-utils: add sysfs interface support and new tool
 - libubi: add `ubi_set_property`, `ubi_leb_unmap` interfaces
 - libubi: add `ubi_is_mapped()` function
 - ubinize: add sequence number support
 - ubiformat: add image sequence support
 - ubi-utils: add ubirsvol tool to resize UBI volumes
 - flash_unlock: enhancing for unlocking of specified number of blocks

### Fixed
 - ubi-tests: fix makefile
 - ubi-tests: fix run script
 - ubinize: validate number of sections
 - ubinize: add more ini-file validation
 - ubinize: fix static volumes generation
 - fs-tests: fix symlink bug in integrity test
 - fs-tests: fix max file name length in integrity test
 - fs-tests: allow for symlink name too long in integrity test
 - ubi-utils: fixed and enhanced `--flash-image` option
 - ubimkvol: check for free LEBs
 - ubiupdatevol: remove non-existing option from help message
 - fs-tests: also preserve mount options when mounting again
 - nandwrite/nanddump: Pass Real Names as Arguments to perror
 - ubinize: correct subpage_size print and initialise vol_info to zero
 - flash_eraseall reports incorrect percentage
 - ubi-utils: fix CFLAGS handling wrt cross compilation
 - Makefile: Separate '-m' and the mode with a space when invoking 'install'
 - ubi-utils: various fixes
 - ubiformat.c: fix printf(%d, size_t) warning
 - ubi-utils: fix up build system
 - nandwrite: correct data reading
 - mkfs.jffs2: fix dir creation in /
 - ubi-utils: fix warning in fprintf() code
 - common.mk: tweak rules to workaround make-3.80 bugs
 - fs-tests: fix remounting in integck
 - ubi-utils: minor printing fix
 - ubi-utils: fix compilation warnings
 - ubi-tests: fix build and some warnings
 - ubiupdatevol: fix -t parameter
 - mkfs.jffs2: fix lzo usage on 64bit systems
 - make sure compiler supports warning flags
 - libmtd: fix mtd_is_bad return code
 - libubigen: don't define large array on stack
 - ubiformat: fix segfault and messages
 - Prevent git-clean from removing cscope files
 - ubinfo: handle -d correctly
 - ubiformat: fix build error
 - ubi-tests: make tests compile again
 - libubigen: add missing include
 - libubi: fix multiple memory corruptions
 - ubi-utils: fix memory corruptions
 - libmtd: recognize pre-MTD-sysfs kernels better
 - nandwrite: fix loop condition
 - nandwrite: return error if failure when reading from standard input
 - nandwrite: fix error handling
 - libubi: fix wrong size calculation with sizes > 4GiB
 - ubiformat: allow zero erase counter
 - ubi-utils: fix compilation errors when using CPPFLAGS
 - nandwrite: fix incorrect use of errno
 - ubiformat: fix typo in the help output
 - ubiformat: fix `--erase-counter` handling
 - flash_lock: fix length being passed
 - ubiformat: check that min IO size is power of 2
 - ubiformat: fix error message

### Changed
 - enable parallel build process
 - ubi-utils: use 'stat(2)' instead of 'lstat(2)'
 - ubi-utils: allow ubiformat to read from stdin
 - ubi-utils: minor rename
 - nandwrite/nanddump: cleanup qualifiers, exit mnemonics, booleans, usage
 - fec.c: replace bzero, bcopy, bcmp wiuth memset, memcpy, memcmp
 - libubi: be more verbose about errors
 - libubi: remove some too verbose messages
 - Unify all common build system parts
 - ubiformat: nicify error messages
 - libmtd: Add `_FILE_OFFSET_BITS=64`, fix `lseek` overflow
 - update ubi-user.h, ubi-header.h,  ubi-media.h
 - Add generated binaries to gitignore
 - ubi-utils: re-arrange directory layout
 - ubi-utils: tweak vpath handling
 - libmtd: move comments to headers
 - libubi: amend included header files
 - libubi: remove unnecessary header files
 - libmtd: amend interface
 - ubi-utils: rename `ubi_node_type`, `mtd_get_info`, `mtd_info`
 - libubi: improve `libubi_open` interface, error handling
 - libubi: do not use udevsettle
 - libmtd: rename `allows_bb`, `rdonly`, `num` fields
 - libmtd: make type_str to be an array
 - ubi-tests: improve io_paral test
 - ubiformat: torture eraseblocks on write errors
 - ubiformat: minor printing clean-up
 - ubiformat: nicify error output
 - ubiformat: make badblocks output less confusing
 - common: remove depricated KB,MB,GB support
 - ubi-utils: update ubi-media.h
 - ubiformat: clean up help output a little
 - jffs2dump: rewrite `--help` output to be more suitable for `help2man`
 - nandwrite: unified reading from standard input and from file

### Removed
 - ubi-utils: remove debugging leftovers
 - ubi-utils: remove depricated -d option

## [1.2.0] - 2008-06-27
### Added
 - nandwrite: add 'markbad' option
 - Add nand integrity testing utility
 - Add utilities for multicast send/receive of MTD images
 - ubi-utils: add -m option to ubimkvol
 - ubi-utils: add -S option to ubimkvol
 - ubi-utils: Add rmvol test
 - Add test program orph.c
 - mkfs.jffs2.c: detect hardlinks
 - ubi-utils: add ubinfo, ubiupdate, ubiattach, ubidetach
 - ubi-utils: bin2nand, nand2bin support for different ecc layouts
 - ubi-utils: ubinize, ubiformat
 - Add libmtd, libscan
 - libubi: support atomic LEB change ioctl
 - mtd_debug: support new flags
 - Add load_nandsim.sh script
 - fs-tests: add simple performance test
 - mkfs.jffs2: Add Support for Symlinks to Device Table

### Fixed
 - Fix repeated warning about ECC correction, in nandtest
 - fs-tests: fix bug in simple test_2
 - ubi-utils: Fixup oob data generation
 - ubi-utils: various fixes in unubi
 - ubi-utils: nand2bin had ECC calculation problems
 - libubi: fix use of negative `errno` values
 - libubi: fail gracefully if ubi is not supported on the current system
 - ubi-utils: fix readdir error checking
 - ubi-utils: fix bytes output formating
 - libubi: fix sysfs direntries scanning
 - ubi-utils: pddcustomize fixup update volume
 - ubi-utils: fix warnings for gcc 4.2.3
 - ubi-utils: ubimkvol: fix wrong variable printout
 - Use LSB locations for manpages
 - compr_lzo.c: allocate enough memory for lzo compressor.
 - mkfs.jffs2.c: fix issue with crashing when using lzo compression
 - fs-tests: preserve mount options when mounting again
 - mkfs.jffs2: Remove Incorrect Find Optimization

### Changed
 - nandtest: Mark blocks bad on failed write; don't abort
 - Makefile: Make optflags easier for distros to override
 - Makefile: build ubi-utils from top dir, add to install target
 - Improve option handling in nandtest, add markbad and offset/length options
 - ubi-utils: rename `__unused` to `ubi_unused`
 - ubi-utils: remove useless build information
 - ubi-utils: Nand2bin add more information when bad blocks occur
 - ubi-utils: migrate to new libubi
 - ubi-utils: Update and bugfix unubi
 - ubi-utils: Test-case for unubi
 - ubi-utils: unubi: add physical erase block number for analysis
 - ubi-utils: overhaul README, tests
 - ubi-utils: move UBI tests to tests/ubi-tests/
 - ubi-utils: move jffs2_test.sh to ubi-tests
 - mkfs.jffs2: use separate ino field in struct filesystem_entry for jffs2 ino#
 - mkfs.jffs2: Set mkfs.jffs2 page size runtime instead of fixed
 - ubi-tools: major cleanups in ubimkvol, ubirmvol
 - libubi: consistently rename `eb` to `leb`
 - ubi-utils: cleanup ubicrc32
 - ubi-utils: more sanity checks
 - Move perl scripts to scritps directory
 - tests: move checkfs test, jittertest to tests directory
 - ubi-utils: update headers
 - ubi-utils: CLI handling for ubigen consistent with other tools
 - ubi-utils: scripts/mkdevs.pl: create control device node as well
 - ubi-utils: major overhaul/rewrite for pfi2ubi
 - ubi-tools: use uint32_t in userspace
 - ubi-utils: use macros not hardcoded constants
 - ubi-utils: use mtd_swab.h
 - ubi-tools: improve printing macros
 - ubi-tools: consistent option handling for ubimkvol, ubirmvol, ubiupdatevol
 - ubi-utils: don't use argp.h
 - Teach libubi_open not to prirnt error message
 - ubi-utils: use common.h in libubi
 - fs-tests: allow for ENOSPC in test fwrite00
 - Makefile: respect CFLAGS/CPPFLAGS from build environment

### Removed
 - Remove mkfs.jffs utility
 - ubi-utils: remove unused directory, testcases.txt

## [1.1.0] - 2007-08-03
### Added
 - Add GPL license text, .gitignore file
 - mkfs.jffs2: Add XATTR support
 - nanddump: Add `-n` argument (no-ecc)
 - Support for 1KB page & 32 bytes spare NAND
 - Add feature removal schedule file
 - Add fs-tests from Adrian Hunter
 - Add UBI utilities
 - Add lzo helper functions

### Fixed
 - nanddump.c: add missing `--file` long options
 - fix handling of ioctl return value in nand-utils
 - Correct integrity test calculations of free space

### Changed
 - nandwrite: Allow `-s` argument to accept hex and octal values
 - Add usage message for flash_erase.c
 - Consolidate the swab macros into one location
 - Switch to using standard types
 - Update mtd-abi.h from upstream kernel.
 - Update user headers from latest kernel tree
 - Update mtd-abi.h and use new NAND ECC functionality
 - Makefile: Support out-of-tree builds
 - mkfs.jffs2: Add xseqno into `jffs2_raw_xref`
 - Report compressed file sizes (including node headers) in mkfs.jffs2 stats
 - Consistently use getopt based option parsing
 - Amend tests not to insist that file system type is JFFS2
 - Remove CVS $Id:$ tags

### Removed
 - mkfs.ffs2
 - Remaining Automake files

## [1.0.0] - 2006-04-30
### Added
 - Import source from CVS
 - Add user space headers

### Fixed
 - memory leak in `write_regular_file`
 - Fix cbuf free properly
 - Fix mtd_debug after removal of some MTD types and flags
 - Fix `make install` target

### Changed
 - Build using in-tree user space headers
