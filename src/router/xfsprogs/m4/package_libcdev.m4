# 
# Check if we have a working fadvise system call
#
AC_DEFUN([AC_HAVE_FADVISE],
  [ AC_MSG_CHECKING([for fadvise ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <fcntl.h>
    ], [
	posix_fadvise(0, 1, 0, POSIX_FADV_NORMAL);
    ],	have_fadvise=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_fadvise)
  ])

#
# Check if we have a working madvise system call
#
AC_DEFUN([AC_HAVE_MADVISE],
  [ AC_MSG_CHECKING([for madvise ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <sys/mman.h>
    ], [
	posix_madvise(0, 0, MADV_NORMAL);
    ],	have_madvise=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_madvise)
  ])

#
# Check if we have a working mincore system call
#
AC_DEFUN([AC_HAVE_MINCORE],
  [ AC_MSG_CHECKING([for mincore ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <sys/mman.h>
    ], [
	mincore(0, 0, 0);
    ],	have_mincore=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_mincore)
  ])

#
# Check if we have a working sendfile system call
#
AC_DEFUN([AC_HAVE_SENDFILE],
  [ AC_MSG_CHECKING([for sendfile ])
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <sys/sendfile.h>
    ], [
         sendfile(0, 0, 0, 0);
    ],	have_sendfile=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_sendfile)
  ])

#
# Check if we have a getmntent libc call (Linux)
#
AC_DEFUN([AC_HAVE_GETMNTENT],
  [ AC_MSG_CHECKING([for getmntent ])
    AC_TRY_COMPILE([
#include <stdio.h>
#include <mntent.h>
    ], [
         getmntent(0);
    ], have_getmntent=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_getmntent)
  ])

#
# Check if we have a fallocate libc call (Linux)
#
AC_DEFUN([AC_HAVE_FALLOCATE],
  [ AC_MSG_CHECKING([for fallocate])
    AC_TRY_LINK([
#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/falloc.h>
    ], [
         fallocate(0, 0, 0, 0);
    ], have_fallocate=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_fallocate)
  ])

#
# Check if we have the fiemap ioctl (Linux)
#
AC_DEFUN([AC_HAVE_FIEMAP],
  [ AC_MSG_CHECKING([for fiemap])
    AC_TRY_LINK([
#define _GNU_SOURCE
#include <linux/fs.h>
#include <linux/fiemap.h>
    ], [
         struct fiemap *fiemap;
         ioctl(0, FS_IOC_FIEMAP, (unsigned long)fiemap);
    ], have_fiemap=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_fiemap)
  ])

#
# Check if we have a preadv libc call (Linux)
#
AC_DEFUN([AC_HAVE_PREADV],
  [ AC_MSG_CHECKING([for preadv])
    AC_TRY_LINK([
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <sys/uio.h>
    ], [
         preadv(0, 0, 0, 0);
    ], have_preadv=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_preadv)
  ])

#
# Check if we have a pwritev2 libc call (Linux)
#
AC_DEFUN([AC_HAVE_PWRITEV2],
  [ AC_MSG_CHECKING([for pwritev2])
    AC_TRY_LINK([
#define _BSD_SOURCE
#include <sys/uio.h>
    ], [
         pwritev2(0, 0, 0, 0, 0);
    ], have_pwritev2=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_pwritev2)
  ])

#
# Check if we have a copy_file_range system call (Linux)
#
AC_DEFUN([AC_HAVE_COPY_FILE_RANGE],
  [ AC_MSG_CHECKING([for copy_file_range])
    AC_TRY_LINK([
#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
    ], [
         syscall(__NR_copy_file_range, 0, 0, 0, 0, 0, 0);
    ], have_copy_file_range=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_copy_file_range)
  ])

#
# Check if we have a sync_file_range libc call (Linux)
#
AC_DEFUN([AC_HAVE_SYNC_FILE_RANGE],
  [ AC_MSG_CHECKING([for sync_file_range])
    AC_TRY_LINK([
#define _GNU_SOURCE
#include <fcntl.h>
    ], [
         sync_file_range(0, 0, 0, 0);
    ], have_sync_file_range=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_sync_file_range)
  ])

#
# Check if we have a syncfs libc call (Linux)
#
AC_DEFUN([AC_HAVE_SYNCFS],
  [ AC_MSG_CHECKING([for syncfs])
    AC_TRY_LINK([
#define _GNU_SOURCE
#include <unistd.h>
    ], [
         syncfs(0);
    ], have_syncfs=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_syncfs)
  ])

#
# Check if we have a readdir libc call
#
AC_DEFUN([AC_HAVE_READDIR],
  [ AC_MSG_CHECKING([for readdir])
    AC_TRY_LINK([
#include <dirent.h>
    ], [
         readdir(0);
    ], have_readdir=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_readdir)
  ])

#
# Check if we have a flc call (Mac OS X)
#
AC_DEFUN([AC_HAVE_FLS],
  [ AC_CHECK_DECL([fls],
       have_fls=yes,
       [],
       [#include <string.h>]
       )
    AC_SUBST(have_fls)
  ])

#
# Check if we have a fsetxattr call
#
AC_DEFUN([AC_HAVE_FSETXATTR],
  [ AC_CHECK_DECL([fsetxattr],
       have_fsetxattr=yes,
       [],
       [#include <sys/types.h>
        #include <sys/xattr.h>]
       )
    AC_SUBST(have_fsetxattr)
  ])

#
# Check if there is mntent.h
#
AC_DEFUN([AC_HAVE_MNTENT],
  [ AC_CHECK_HEADERS(mntent.h,
    have_mntent=yes)
    AC_SUBST(have_mntent)
  ])

#
# Check if we have a mremap call (not on Mac OS X)
#
AC_DEFUN([AC_HAVE_MREMAP],
  [ AC_CHECK_DECL([mremap],
       have_mremap=yes,
       [],
       [#define _GNU_SOURCE
        #include <sys/mman.h>]
       )
    AC_SUBST(have_mremap)
  ])

#
# Check if we need to override the system struct fsxattr with
# the internal definition.  This /only/ happens if the system
# actually defines struct fsxattr /and/ the system definition
# is missing certain fields.
#
AC_DEFUN([AC_NEED_INTERNAL_FSXATTR],
  [
    AC_CHECK_TYPE(struct fsxattr,
      [
        AC_CHECK_MEMBER(struct fsxattr.fsx_cowextsize,
          ,
          need_internal_fsxattr=yes,
          [#include <linux/fs.h>]
        )
      ],,
      [#include <linux/fs.h>]
    )
    AC_SUBST(need_internal_fsxattr)
  ])

#
# Check if we need to override the system struct fscrypt_add_key_arg
# with the internal definition.  This /only/ happens if the system
# actually defines struct fscrypt_add_key_arg /and/ the system
# definition is missing certain fields.
#
AC_DEFUN([AC_NEED_INTERNAL_FSCRYPT_ADD_KEY_ARG],
  [
    AC_CHECK_TYPE(struct fscrypt_add_key_arg,
      [
        AC_CHECK_MEMBER(struct fscrypt_add_key_arg.key_id,
          ,
          need_internal_fscrypt_add_key_arg=yes,
          [#include <linux/fs.h>]
        )
      ],,
      [#include <linux/fs.h>]
    )
    AC_SUBST(need_internal_fscrypt_add_key_arg)
  ])

#
# Check if we have a FS_IOC_GETFSMAP ioctl (Linux)
#
AC_DEFUN([AC_HAVE_GETFSMAP],
  [ AC_MSG_CHECKING([for GETFSMAP])
    AC_TRY_LINK([
#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/fsmap.h>
    ], [
         unsigned long x = FS_IOC_GETFSMAP;
         struct fsmap_head fh;
    ], have_getfsmap=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_getfsmap)
  ])

AC_DEFUN([AC_HAVE_STATFS_FLAGS],
  [
    AC_CHECK_TYPE(struct statfs,
      [
        AC_CHECK_MEMBER(struct statfs.f_flags,
          have_statfs_flags=yes,,
          [#include <sys/vfs.h>]
        )
      ],,
      [#include <sys/vfs.h>]
    )
    AC_SUBST(have_statfs_flags)
  ])

#
# Check if we have MAP_SYNC defines (Linux)
#
AC_DEFUN([AC_HAVE_MAP_SYNC],
  [ AC_MSG_CHECKING([for MAP_SYNC])
    AC_TRY_COMPILE([
#include <asm-generic/mman.h>
#include <asm-generic/mman-common.h>
    ], [
        int flags = MAP_SYNC | MAP_SHARED_VALIDATE;
    ], have_map_sync=yes
	AC_MSG_RESULT(yes),
	AC_MSG_RESULT(no))
    AC_SUBST(have_map_sync)
  ])

#
# Check if we have a mallinfo libc call
#
AC_DEFUN([AC_HAVE_MALLINFO],
  [ AC_MSG_CHECKING([for mallinfo ])
    AC_TRY_COMPILE([
#include <malloc.h>
    ], [
         struct mallinfo test;

         test.arena = 0; test.hblkhd = 0; test.uordblks = 0; test.fordblks = 0;
         test = mallinfo();
    ], have_mallinfo=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_mallinfo)
  ])

#
# Check if we have a openat call
#
AC_DEFUN([AC_HAVE_OPENAT],
  [ AC_CHECK_DECL([openat],
       have_openat=yes,
       [],
       [#include <sys/types.h>
        #include <sys/stat.h>
        #include <fcntl.h>]
       )
    AC_SUBST(have_openat)
  ])

#
# Check if we have a fstatat call
#
AC_DEFUN([AC_HAVE_FSTATAT],
  [ AC_CHECK_DECL([fstatat],
       have_fstatat=yes,
       [],
       [#define _GNU_SOURCE
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <unistd.h>])
    AC_SUBST(have_fstatat)
  ])

#
# Check if we have the SG_IO ioctl
#
AC_DEFUN([AC_HAVE_SG_IO],
  [ AC_MSG_CHECKING([for struct sg_io_hdr ])
    AC_TRY_COMPILE([#include <scsi/sg.h>],
    [
         struct sg_io_hdr hdr;
         ioctl(0, SG_IO, &hdr);
    ], have_sg_io=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_sg_io)
  ])

#
# Check if we have the HDIO_GETGEO ioctl
#
AC_DEFUN([AC_HAVE_HDIO_GETGEO],
  [ AC_MSG_CHECKING([for struct hd_geometry ])
    AC_TRY_COMPILE([#include <linux/hdreg.h>],
    [
         struct hd_geometry hdr;
         ioctl(0, HDIO_GETGEO, &hdr);
    ], have_hdio_getgeo=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_hdio_getgeo)
  ])

AC_DEFUN([AC_PACKAGE_CHECK_LTO],
  [ AC_MSG_CHECKING([if C compiler supports LTO])
    OLD_CFLAGS="$CFLAGS"
    OLD_LDFLAGS="$LDFLAGS"
    LTO_FLAGS="-flto -ffat-lto-objects"
    CFLAGS="$CFLAGS $LTO_FLAGS"
    LDFLAGS="$LDFLAGS $LTO_FLAGS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
        [AC_MSG_RESULT([yes])]
        [lto_cflags=$LTO_FLAGS]
        [lto_ldflags=$LTO_FLAGS]
        [AC_PATH_PROG(gcc_ar, gcc-ar,,)]
        [AC_PATH_PROG(gcc_ranlib, gcc-ranlib,,)],
        [AC_MSG_RESULT([no])])
    if test -x "$gcc_ar" && test -x "$gcc_ranlib"; then
        have_lto=yes
    fi
    CFLAGS="${OLD_CFLAGS}"
    LDFLAGS="${OLD_LDFLAGS}"
    AC_SUBST(gcc_ar)
    AC_SUBST(gcc_ranlib)
    AC_SUBST(have_lto)
    AC_SUBST(lto_cflags)
    AC_SUBST(lto_ldflags)
  ])
