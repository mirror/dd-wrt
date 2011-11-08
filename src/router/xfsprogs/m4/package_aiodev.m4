#
# Check if we have a libaio.h installed
#
AC_DEFUN([AC_PACKAGE_WANT_AIO],
  [ AC_CHECK_HEADERS(libaio.h, [ have_aio=true ], [ have_aio=false ])
    AC_SUBST(have_aio)
  ])

#
# Check if we have an aio.h installed
#
AC_DEFUN([AC_PACKAGE_NEED_AIO_H],
  [ AC_CHECK_HEADERS(aio.h)
    if test $ac_cv_header_aio_h = no; then
	echo
	echo 'FATAL ERROR: could not find a valid <aio.h> header.'
	exit 1
    fi
  ])

#
# Check if we have the lio_listio routine in either libc/librt
#
AC_DEFUN([AC_PACKAGE_NEED_LIO_LISTIO],
  [ AC_CHECK_FUNCS(lio_listio)
    if test $ac_cv_func_lio_listio = yes; then
	librt=""
    else
	AC_CHECK_LIB(rt, lio_listio,, [
	    echo
	    echo 'FATAL ERROR: could not find a library with lio_listio.'
	    exit 1],[-lpthread])
	librt="-lrt"
    fi
    AC_SUBST(librt)
  ])

