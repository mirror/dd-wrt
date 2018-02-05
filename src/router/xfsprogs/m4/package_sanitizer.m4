AC_DEFUN([AC_PACKAGE_CHECK_UBSAN],
  [ AC_MSG_CHECKING([if C compiler supports UBSAN])
    OLD_CFLAGS="$CFLAGS"
    OLD_LDFLAGS="$LDFLAGS"
    UBSAN_FLAGS="-fsanitize=undefined"
    CFLAGS="$CFLAGS $UBSAN_FLAGS"
    LDFLAGS="$LDFLAGS $UBSAN_FLAGS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
        [AC_MSG_RESULT([yes])]
        [ubsan_cflags=$UBSAN_FLAGS]
        [ubsan_ldflags=$UBSAN_FLAGS]
        [have_ubsan=yes],
        [AC_MSG_RESULT([no])])
    CFLAGS="${OLD_CFLAGS}"
    LDFLAGS="${OLD_LDFLAGS}"
    AC_SUBST(have_ubsan)
    AC_SUBST(ubsan_cflags)
    AC_SUBST(ubsan_ldflags)
  ])

AC_DEFUN([AC_PACKAGE_CHECK_ADDRSAN],
  [ AC_MSG_CHECKING([if C compiler supports ADDRSAN])
    OLD_CFLAGS="$CFLAGS"
    OLD_LDFLAGS="$LDFLAGS"
    ADDRSAN_FLAGS="-fsanitize=address"
    CFLAGS="$CFLAGS $ADDRSAN_FLAGS"
    LDFLAGS="$LDFLAGS $ADDRSAN_FLAGS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
        [AC_MSG_RESULT([yes])]
        [addrsan_cflags=$ADDRSAN_FLAGS]
        [addrsan_ldflags=$ADDRSAN_FLAGS]
        [have_addrsan=yes],
        [AC_MSG_RESULT([no])])
    CFLAGS="${OLD_CFLAGS}"
    LDFLAGS="${OLD_LDFLAGS}"
    AC_SUBST(have_addrsan)
    AC_SUBST(addrsan_cflags)
    AC_SUBST(addrsan_ldflags)
  ])

AC_DEFUN([AC_PACKAGE_CHECK_THREADSAN],
  [ AC_MSG_CHECKING([if C compiler supports THREADSAN])
    OLD_CFLAGS="$CFLAGS"
    OLD_LDFLAGS="$LDFLAGS"
    THREADSAN_FLAGS="-fsanitize=thread"
    CFLAGS="$CFLAGS $THREADSAN_FLAGS"
    LDFLAGS="$LDFLAGS $ADRSAN_FLAGS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
        [AC_MSG_RESULT([yes])]
        [threadsan_cflags=$THREADSAN_FLAGS]
        [threadsan_ldflags=$THREADSAN_FLAGS]
        [have_threadsan=yes],
        [AC_MSG_RESULT([no])])
    CFLAGS="${OLD_CFLAGS}"
    LDFLAGS="${OLD_LDFLAGS}"
    AC_SUBST(have_threadsan)
    AC_SUBST(threadsan_cflags)
    AC_SUBST(threadsan_ldflags)
  ])
