#
# See if blkid has the topology bits
#

AC_DEFUN([AC_HAVE_BLKID_TOPO],
[
  AC_SEARCH_LIBS([blkid_probe_all], [blkid])
  AC_CHECK_FUNCS(blkid_probe_get_topology)
  if test $ac_cv_func_blkid_probe_get_topology = yes; then
    libblkid="-lblkid"
  else
    echo
    echo 'FATAL ERROR: could not find a valid BLKID header.'
    echo 'Install the Block device ID development package.'
    exit 1
  fi
  AC_SUBST(libblkid)
])
