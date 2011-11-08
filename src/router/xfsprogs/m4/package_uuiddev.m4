AC_DEFUN([AC_PACKAGE_NEED_UUID_H],
  [ AC_CHECK_HEADERS([uuid.h sys/uuid.h uuid/uuid.h])
    if test $ac_cv_header_uuid_h = no -a \
	    $ac_cv_header_sys_uuid_h = no -a \
	    $ac_cv_header_uuid_uuid_h = no; then
	echo
	echo 'FATAL ERROR: could not find a valid UUID header.'
	echo 'Install the Universally Unique Identifiers development package.'
	exit 1
    fi
  ])

AC_DEFUN([AC_PACKAGE_NEED_UUIDCOMPARE],
  [ AC_CHECK_FUNCS(uuid_compare)
    if test $ac_cv_func_uuid_compare = yes; then
	libuuid=""
    else
	AC_CHECK_LIB(uuid, uuid_compare,, [
	    echo
	    echo 'FATAL ERROR: could not find a valid UUID library.'
	    echo 'Install the Universally Unique Identifiers library package.'
	    exit 1])
	libuuid="-luuid"
    fi
    AC_SUBST(libuuid)
  ])
