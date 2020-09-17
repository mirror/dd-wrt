#
# See if libdevmapper is available on the system.
#
AC_DEFUN([AC_HAVE_DEVMAPPER],
[ AC_SEARCH_LIBS([dm_task_create], [devmapper],
        libdevmapper="-ldevmapper"
        have_devmapper=yes,
        have_devmapper=no,)
    AC_SUBST(have_devmapper)
    AC_SUBST(libdevmapper)
])
