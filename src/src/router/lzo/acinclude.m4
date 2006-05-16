## --------------------------------------------------------- ##
## My own customization.                                     ##
## --------------------------------------------------------- ##

# serial 4

AC_DEFUN(mfx_CUSTOMIZE,
[
mfx_PROG_CC_BUG_SIGNED_TO_UNSIGNED_CASTING
mfx_PROG_GCC_BUG_SCHEDULE_INSNS
mfx_PROG_GCC_BUG_STRENGTH_REDUCE

dnl /***********************************************************************
dnl // Prepare some macros
dnl ************************************************************************/

CFLAGS_GCC_OMIT_FRAME_POINTER=""
if test "X$GCC" = Xyes; then
  CFLAGS_GCC_OMIT_FRAME_POINTER="-fomit-frame-pointer"
  if test "X$mfx_cv_prog_checkergcc" = Xyes; then
    CFLAGS_GCC_OMIT_FRAME_POINTER="-fno-omit-frame-pointer"
  fi
  if test "X$enable_debug" = Xyes; then
    CFLAGS_GCC_OMIT_FRAME_POINTER="-fno-omit-frame-pointer"
  fi
  if test "X$enable_profiling" = Xyes; then
    CFLAGS_GCC_OMIT_FRAME_POINTER="-fno-omit-frame-pointer"
  fi
  if test "X$enable_coverage" = Xyes; then
    CFLAGS_GCC_OMIT_FRAME_POINTER="-fno-omit-frame-pointer"
  fi
fi
AC_SUBST(CFLAGS_GCC_OMIT_FRAME_POINTER)dnl

if test "X$enable_debug" = Xyes; then
  if test "X$GCC" = Xyes -a "X$USE_MAINTAINER_MODE" = Xyes; then
    test "$ac_cv_prog_cc_g" = yes && CFLAGS="$CFLAGS -g"
    test "$ac_cv_prog_cxx_g" = yes && CXXFLAGS="$CXXFLAGS -g"
  else
    test "$ac_cv_prog_cc_g" = yes && CFLAGS="$CFLAGS -g"
    test "$ac_cv_prog_cxx_g" = yes && CXXFLAGS="$CXXFLAGS -g"
  fi
fi


dnl /***********************************************************************
dnl // Compiler and architecture for use in makefiles
dnl ************************************************************************/

AC_SUBST(MFX_CC)
AC_SUBST(MFX_ARCH)
AC_SUBST(MFX_CPU)

MFX_CC="unknown"
MFX_ARCH="unknown"
MFX_CPU="$host_cpu"
if test "X$cross_compiling" = Xyes; then
  if test "X$build" = "X$host"; then
    MFX_CPU="unknown"
  fi
fi
MFX_CPU=`echo "$MFX_CPU" | sed -e 's/[^a-zA-Z0-9]//g'`

if test "X$GCC" = Xyes; then
  MFX_CC="GCC"
  if test "X$enable_debug" = Xyes; then
    CFLAGS="$CFLAGS -O0"
  else
    CFLAGS="$CFLAGS -O2"
  fi
  CFLAGS="$CFLAGS -Wall -Wcast-align -Wcast-qual -Wwrite-strings"
  case $MFX_CPU in
    i[[3456789]]86)
      MFX_ARCH="i386"
      mfx_unaligned_ok_2="yes"
      mfx_unaligned_ok_4="yes"
      CFLAGS="$CFLAGS -fno-strength-reduce"
      ;;
    *)
      if test "X$mfx_cv_prog_gcc_bug_strength_reduce" = Xyes; then
        CFLAGS="$CFLAGS -fno-strength-reduce"
      fi
      ;;
  esac
  if test "X$mfx_cv_prog_gcc_bug_schedule_insns" = Xyes; then
    CFLAGS="$CFLAGS -fno-schedule-insns -fno-schedule-insns2"
  fi
fi


if test "X$GCC" = Xyes; then
AC_CACHE_CHECK([whether ${CC-cc} accepts -fstrict-aliasing],
mfx_cv_prog_gcc_f_strict_aliasing,
[echo 'extern int x; int x = 0;' > conftest.cc
if test -z "`${CC-cc} -fstrict-aliasing -c conftest.cc 2>&1`"; then
  mfx_cv_prog_gcc_f_strict_aliasing=yes
else
  mfx_cv_prog_gcc_f_strict_aliasing=no
fi
rm -f conftest*
])
if test "X$mfx_cv_prog_gcc_f_strict_aliasing" = Xyes; then
  CFLAGS="$CFLAGS -fstrict-aliasing"
fi
fi


MFX_ARCH=`echo "$MFX_ARCH" | sed -e 's/[^a-zA-Z0-9]//g'`

AC_DEFINE_UNQUOTED(MFX_ARCH,"$MFX_ARCH")
AC_DEFINE_UNQUOTED(MFX_CPU,"$MFX_CPU")
])

