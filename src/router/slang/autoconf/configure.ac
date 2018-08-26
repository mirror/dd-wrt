dnl# -*- sh -*-
dnl#
dnl#   This file requires JD_* m4 macros that are defined in 
dnl# the aclocal.m4 file.
dnl#

AC_INIT(src/slang.c)
#AC_PREFIX_DEFAULT($MY_PREFIX_DEAULT)

# Installation location
AC_PREFIX_DEFAULT("/usr/local"
dnl# if test -f "/usr/include/slang.h"; then
dnl# ac_default_prefix="/usr"
dnl# fi
)

AC_CONFIG_AUX_DIR(autoconf)
AC_CANONICAL_HOST

JD_INIT
JD_ANSI_CC

AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_INSTALL

AC_ARG_WITH(pkgconfigdir,
  AC_HELP_STRING([--with-pkgconfigdir=DIR],
  [pkgconfig dir (libdir/pkgconfig)]),
  [pkgconfigdir=${withval}],
  [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST(pkgconfigdir)

dnl # Terminfo binary format
AC_ARG_WITH(terminfo,
   [  --with-terminfo=xxx  Terminfo layout: default, hpux11, osf1r5, aix4, uwin ],
   [ jd_with_terminfo_arg=$withval ], [jd_with_terminfo_arg=default])
AC_MSG_CHECKING(terminfo layout)
if test "x$jd_with_terminfo_arg" = "x"
then
  jd_with_terminfo_arg="default"
fi
if test -r src/terminfo/$jd_with_terminfo_arg.inc
then
  AC_MSG_RESULT($jd_with_terminfo_arg)
  TERMINFO_LAYOUT="$jd_with_terminfo_arg"
else
  AC_MSG_ERROR(-"-with-terminfo=$jd_with_terminfo_arg is invalid.  src/terminfo/$jd_with_terminfo_arg.inc does not exist")
fi
AC_SUBST(TERMINFO_LAYOUT)

dnl # Readline support for slsh
AC_ARG_WITH(readline,
   [  --with-readline=xxx       use slang(default) or gnu readline for slsh ],
   [ jd_with_readline_arg=$withval ], [jd_with_readline_arg=slang])
AC_MSG_CHECKING(type of readline support for slsh)
case "x$jd_with_readline_arg" in
  xgnu|xGNU )
    GNU_READLINE=1
    LIB_READLINE=-lreadline
    AC_MSG_RESULT(gnu);
    ;;
  *)
    GNU_READLINE=0
    LIB_READLINE="# -lreadline"
    AC_MSG_RESULT(slang);
    ;;
esac
AC_SUBST(GNU_READLINE)
AC_SUBST(LIB_READLINE)

# For the socket module
AC_PATH_XTRA

#Check these header since they cause trouble
AC_CHECK_HEADERS( \
stdlib.h \
unistd.h \
memory.h \
termios.h \
malloc.h \
locale.h \
langinfo.h \
fcntl.h \
sys/fcntl.h \
sys/types.h \
sys/wait.h \
sys/utsname.h \
sys/times.h \
sys/time.h \
utime.h \
floatingpoint.h \
ieeefp.h \
nan.h \
fenv.h \
sys/mman.h \
sys/socket.h \
netinet/in.h \
arpa/inet.h \
sys/un.h \
sys/resource.h \
)

AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_UID_T
dnl# AC_TYPE_OFF_T

AC_MSG_CHECKING([for socklen_t])
AC_EGREP_HEADER(socklen_t, sys/socket.h,
                AC_MSG_RESULT([yes]),
                AC_MSG_RESULT([no])
                   AC_DEFINE(socklen_t,int,
                             [ Define to 'int' if not in <sys/socket.h>]))

AC_HEADER_DIRENT

AC_CHECK_FUNCS(\
memset \
memcpy \
putenv \
getcwd \
setlocale \
tcgetattr \
tcsetattr \
cfgetospeed \
sigaction \
sigemptyset \
sigprocmask \
sigaddset \
alarm \
pause \
vfscanf \
lstat readlink symlink link \
kill killpg \
snprintf vsnprintf \
getppid getegid geteuid getuid getgid setgid setuid \
setpgid getpgid setpgrp getpgrp setsid getsid \
mmap \
chown popen mkfifo \
atexit on_exit umask uname \
times gmtime mktime gettimeofday \
gmtime_r ctime_r localtime_r \
strtod atoll strtoll \
issetugid \
isnan \
finite \
isinf \
round \
siglongjmp \
fork waitpid \
socket socketpair \
confstr pathconf \
getpriority setpriority \
setvbuf \
utime utimes \
ttyname ttyname_r \
)

if test x"$ac_cv_func_socketpair" != x"yes"; then
  AC_CHECK_LIB(socket, socketpair, AC_DEFINE(HAVE_SOCKETPAIR,1))
fi
if test x"$ac_cv_func_socket" != x"yes"; then
  AC_CHECK_LIB(socket, socket, AC_DEFINE(HAVE_SOCKET,1))
fi


dnl# Systems that have nl_langinfo may not have CODESET.  Test for both here
AC_CACHE_CHECK([for nl_langinfo and CODESET], jd_cv_nl_langinfo_codeset,
 [AC_TRY_LINK([#include <langinfo.h>], [char* cs = nl_langinfo(CODESET);],
  jd_cv_nl_langinfo_codeset=yes, jd_cv_nl_langinfo_codeset=no)
])
if test $jd_cv_nl_langinfo_codeset = yes
then
  AC_DEFINE(HAVE_NL_LANGINFO_CODESET, 1, [Set to 1 if you have the nl_langinfo(CODESET) function])
fi

AC_CHECK_LIB(m, acosh, [AC_DEFINE(HAVE_ACOSH,1,[Set to 1 if you have the acosh function])])
AC_CHECK_LIB(m, asinh, [AC_DEFINE(HAVE_ASINH,1,[Set to 1 if you have the asinh function])])
AC_CHECK_LIB(m, atanh, [AC_DEFINE(HAVE_ATANH,1,[Set to 1 if you have the atanh function])])
AC_CHECK_LIB(m, hypot, [AC_DEFINE(HAVE_HYPOT,1,[Set to 1 if you have the hypot function])])
AC_CHECK_LIB(m, atan2, [AC_DEFINE(HAVE_ATAN2,1,[Set to 1 if you have the atan2 function])])
AC_CHECK_LIB(m, frexp, [AC_DEFINE(HAVE_FREXP,1,[Set to 1 if you have the frexp function])])
AC_CHECK_LIB(m, frexpf, [AC_DEFINE(HAVE_FREXPF,1,[Set to 1 if you have the frexpf function])])
AC_CHECK_LIB(m, ldexp, [AC_DEFINE(HAVE_LDEXP,1,[Set to 1 if you have the ldexp function])])
AC_CHECK_LIB(m, ldexpf, [AC_DEFINE(HAVE_LDEXPF,1,[Set to 1 if you have the ldexpf function])])
AC_CHECK_LIB(m, feclearexcept, [AC_DEFINE(HAVE_FECLEAREXCEPT,1,[Set to 1 if you have the feclearexcept function])])
AC_CHECK_LIB(m, fpsetsticky, [AC_DEFINE(HAVE_FPSETSTICKY,1,[Set to 1 if you have the fpsetsticky function])])

JD_ELF_COMPILER
JD_IEEE_CFLAGS

ELF_CFLAGS="$ELF_CFLAGS $IEEE_CFLAGS"
CFLAGS="$CFLAGS $IEEE_CFLAGS"

JD_SET_RPATH($libdir)

AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(float, 4)
AC_CHECK_SIZEOF(double, 8)

AC_TYPE_SIZE_T
AC_CHECK_SIZEOF(size_t)

JD_CHECK_LONG_LONG
JD_LARGE_FILE_SUPPORT

JD_TERMCAP
JD_GCC_WARNINGS

JD_SET_OBJ_SRC_DIR(src)

missingmodules=""
extramodules=""
JD_CHECK_FOR_LIBRARY(pcre)
if test $jd_with_pcre_library = yes
then
  PCRE_MODULE="pcre-module.so"
  AC_DEFINE(HAVE_PCRE_H,1,[Set to 1 if you have the pcre.h header])
  extramodules="$extramodules pcre"
else
  PCRE_MODULE=""
  missingmodules="$missingmodules pcre"
fi
AC_SUBST(PCRE_MODULE)

JD_CHECK_FOR_LIBRARY(onig,oniguruma.h)
if test $jd_with_onig_library = yes
then
  ONIG_MODULE="onig-module.so"
  AC_DEFINE(HAVE_ONIGURUMA_H,1,[Set to 1 if you have the oniguruma.h header])
  extramodules="$extramodules oniguruma"
else
  ONIG_MODULE=""
  missingmodules="$missingmodules oniguruma"
fi
AC_SUBST(ONIG_MODULE)

JD_CHECK_FOR_LIBRARY(png)
if test $jd_with_png_library = yes
then
  PNG_MODULE="png-module.so"
  AC_DEFINE(HAVE_PNG_H,1,[Set to 1 if you have the png.h header])
  extramodules="$extramodules png"
else
  PNG_MODULE=""
  missingmodules="$missingmodules png"
fi
AC_SUBST(PNG_MODULE)

JD_CHECK_FOR_LIBRARY(z,zlib.h)
if test $jd_with_z_library = yes
then
  ZLIB_MODULE="zlib-module.so"
  AC_DEFINE(HAVE_ZLIB_H,1,[Set to 1 if you have the zlib.h header])
  extramodules="$extramodules z"
else
  ZLIB_MODULE=""
  missingmodules="$missingmodules z"
fi
AC_SUBST(ZLIB_MODULE)

m4_include([./iconv.m4])

ICONV_MODULE=""
JD_CHECK_FOR_LIBRARY(iconv)
if test $jd_with_iconv_library = yes
then
  ICONV_MODULE="iconv-module.so"
  AC_DEFINE(HAVE_ICONV_H,1,[Set to 1 if you have the iconv.h header])
  ICONV_LIB="$ICONV_LIB -liconv"
  extramodules="$extramodules iconv"
else
  SLANG_AM_ICONV
  if test "$am_cv_func_iconv" = yes
  then
    ICONV_MODULE="iconv-module.so"
    extramodules="$extramodules iconv"
    if test "X$am_cv_lib_iconv" = Xyes
    then
       ICONV_LIB=$LIBICONV
       ICONV_INC=""
    fi
  else
    missingmodules="$missingmodules iconv"
  fi
fi
AC_SUBST(ICONV_MODULE)
dnl # AC_SUBST(ICONV_LIB)
dnl # AC_SUBST(ICONV_INC)

AC_CHECK_FUNCS(sysconf, [
 SYSCONF_MODULE="sysconf-module.so"
 ], [SYSCONF_MODULE=""])
AC_SUBST(SYSCONF_MODULE)

JD_GET_MODULES(src/modules.unx)
JD_CREATE_MODULE_ORULES

AC_MSG_CHECKING(SLANG_VERSION)
slang_version=`grep "^#define  *SLANG_VERSION " $srcdir/src/slang.h |
               awk '{ print [$]3 }'`
slang_major_version=`echo $slang_version |
 awk '{ print int([$]1/10000) }'`
slang_minor_version=`echo $slang_version $slang_major_version |
 awk '{ print int(([$]1 - [$]2*10000)/100) }'`
slang_micro_version=`echo $slang_version $slang_major_version $slang_minor_version |
 awk '{ print ([$]1 - [$]2*10000 - [$]3*100) }'`

dnl# slang_minor_version="$slang_minor_version.$slang_mminor_version"
slang_version="$slang_major_version.$slang_minor_version.$slang_micro_version"
AC_MSG_RESULT($slang_version)
AC_SUBST(slang_major_version)
AC_SUBST(slang_minor_version)
AC_SUBST(slang_micro_version)
AC_SUBST(slang_version)

AC_CONFIG_HEADER(src/sysconf.h:src/config.hin)

dnl AC_CONFIG_SUBDIRS(demo)

AC_OUTPUT(Makefile:autoconf/Makefile.in \
  src/Makefile slsh/Makefile modules/Makefile demo/Makefile \
  slang.pc:autoconf/slangpc.in \
)

JD_CREATE_EXEC_RULE(calc)
JD_CREATE_EXEC_RULE(untic)
JD_APPEND_RULES(src/Makefile)
JD_APPEND_ELFRULES(src/Makefile)

echo ""
echo "Configuration complete.  You may need to edit src/Makefile."
echo "You are compiling SLANG with the following compiler configuration:"
echo "       CC =" "$CC"
echo "   CFLAGS =" "$CFLAGS"
echo "  LDFLAGS =" "$LDFLAGS $DYNAMIC_LINK_FLAGS"
echo ""
echo "   ELF_CC =" "$ELF_CC"
echo " ELF_LINK =" "$ELF_LINK"
echo "ELF_CFLAGS=" "$ELF_CFLAGS"
echo ""
echo "                  prefix:" "$prefix"
echo "             exec_prefix:" "$exec_prefix"
echo "    Installation Lib Dir:" "$libdir"
echo "Installation Include Dir:" "$includedir"
echo ""
echo "See also src/sl-feat.h for various features."

echo ""
echo "The following modules that depend upon external libraries will be built:"
echo "  $extramodules"
echo ""
if test "X$missingmodules" != "X"
then
  echo ""
  echo "While not required, it is recommended that you install the following"
  echo "libraries and reconfigure so that the corresponding modules can be built:"
  echo "  $missingmodules"
  echo ""
fi
echo "Type 'make' to build a shared version of the library."
echo "Type 'make help' for more options"
echo ""
