AC_DEFUN([AC_PACKAGE_NEED_INI_H],
  [ AC_CHECK_HEADERS([ini.h])
    if test $ac_cv_header_ini_h = no; then
	echo
	echo 'FATAL ERROR: could not find a valid ini.h header.'
	echo 'Install the libinih development package.'
	exit 1
    fi
  ])

AC_DEFUN([AC_PACKAGE_NEED_LIBINIH],
  [ AC_CHECK_LIB(inih, ini_parse,, [
	echo
	echo 'FATAL ERROR: could not find a valid inih library.'
	echo 'Install the libinih library package.'
	exit 1
    ])
    libinih=-linih
    AC_SUBST(libinih)
  ])
