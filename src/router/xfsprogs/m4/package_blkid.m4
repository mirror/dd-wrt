#
# See if blkid has the topology bits
#

AC_DEFUN([AC_HAVE_BLKID_TOPO],
[
  enable_blkid="$1"
  if test "$enable_blkid" = "yes"; then
    AC_SEARCH_LIBS([blkid_probe_all], [blkid])
    AC_CHECK_FUNCS(blkid_probe_get_topology)
    if test $ac_cv_func_blkid_probe_get_topology = yes; then
      libblkid="-lblkid"
    else
      libblkd=""
      enable_blkid="no"
      AC_SUBST(enable_blkid)
    fi
  fi
  AC_SUBST(libblkid)
])
