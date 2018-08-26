dnl# -*- mode: sh; mode: fold -*-
dnl# 0.2.7-2: For the Makefile rules, use cd foo && bar instead of cd foo; bar
dnl# 0.2.7-1: Use "$ARCH"elfobjs instead of elf"$ARCH"objs for better flexibility
dnl# 0.2.7-0: Instead of expanding $ARCH at configure time, use \$ARCH for compile-time expansion
dnl# 0.2.6-2: Missing hyphen for cygwin ELFLIB_MAJOR (Marco Atzeri)
dnl# 0.2.6-1: Added optional second and third arguments to AC_DEFINE (Marco Atzeri)
dnl# 0.2.6-0: On cygwin, change libfooX_Y_Z.dll to cygfoo-X_Y_Z.dll (Marco Atzeri)
dnl# 0.2.5-3: Changed AC_DEFUN(foo...) to AC_DEFUN([foo]...)
dnl# 0.2.5-2: JD_CHECK_FOR_LIBRARY will alse output *_INC_DIR and *_LIB_DIR
dnl# 0.2.5-1: Updated using autoupdate
dnl# 0.2.5-0: M_LIB output variable created for haiku support (Scott McCreary)
dnl# 0.2.4-0: Added optional 3rd argument to JD_WITH_LIBRARY for a default path
dnl# 0.2.3-2: X was missing in a "test" statement (Joerg Sommer)
dnl# 0.2.3-1: AC_AIX needs to be called before running the compiler (Miroslav Lichvar)
dnl# 0.2.3: rewrote JD_CHECK_FOR_LIBRARY to loop over include/lib pairs
dnl# 0.2.2-1: JD_WITH_LIBRARY bug-fix
dnl# 0.2.2:  Use ncurses5-config to search for terminfo dirs.
dnl# 0.2.1:  Add .dll.a to list of extensions to when searching for libs (cygwin)
dnl# 0.2.0:  Added install target name and more fixes for cygwin
dnl# 0.1.12: Improved support for cygwin
dnl# 0.1.11: Fixed elf linking on freebsd (Renato Botelho (garga at freebsd, org)
dnl# Version 0.1.10: rpath support for netbsd
dnl# Version 0.1.9: When searching for libs, use dylib on darwin
dnl# Version 0.1.8: Add rpath support for OpenBSD
dnl# Version 0.1.7: removed "-K pic" from IRIX compiler lines
dnl# Version 0.1.6: Added cygwin module support
dnl# Version 0.1.5: Added gcc version-script support.

AC_DEFUN([JD_INIT],     dnl#{{{
[
#These variable are initialized by JD init function
CONFIG_DIR=`pwd`
cd $srcdir
if test "`pwd`" != "$CONFIG_DIR"
then
  AC_MSG_ERROR("This software does not support configuring from another directory.   See the INSTALL file")
fi
dnl# if test "X$PWD" != "X"
dnl# then
dnl#  CONFIG_DIR="$PWD"
dnl# fi
AC_SUBST(CONFIG_DIR)dnl
# Note: these will differ if one is a symbolic link
if test -f /usr/bin/dirname; then
  JD_Above_Dir=`dirname $CONFIG_DIR`
else
# system is a loser
  JD_Above_Dir=`cd ..;pwd`
fi
JD_Above_Dir2=`cd ..;pwd`
])
dnl#}}}

dnl# This function expand the "prefix variables.  For example, it will expand
dnl# values such as ${exec_prefix}/foo when ${exec_prefix} itself has a
dnl# of ${prefix}.  This function produces the shell variables:
dnl# jd_prefix_libdir, jd_prefix_incdir
AC_DEFUN([JD_EXPAND_PREFIX], dnl#{{{
[
  if test "X$jd_prefix" = "X"
  then
    jd_prefix=$ac_default_prefix
    if test "X$prefix" != "XNONE"
    then
      jd_prefix="$prefix"
    fi
    jd_exec_prefix="$jd_prefix"
    if test "X$exec_prefix" != "XNONE"
    then
      jd_exec_prefix="$exec_prefix"
    fi

    dnl#Unfortunately, exec_prefix may have a value like ${prefix}, etc.
    dnl#Let the shell expand those.  Yuk.
    eval `sh <<EOF
      prefix=$jd_prefix
      exec_prefix=$jd_exec_prefix
      libdir=$libdir
      includedir=$includedir
      echo jd_prefix_libdir="\$libdir" jd_prefix_incdir="\$includedir"
EOF
`
  fi
])
#}}}

AC_DEFUN([JD_SET_OBJ_SRC_DIR], dnl#{{{
[
#---------------------------------------------------------------------------
# Set the source directory and object directory.   The makefile assumes an
# absolute path name.  This is because src/Makefile cds to OBJDIR and compiles
# the src file which is in SRCDIR
#---------------------------------------------------------------------------
SRCDIR=$CONFIG_DIR
if test "$1" != "."
then
  if test -z "$1"
  then
    SRCDIR=$SRCDIR/src
  else
    SRCDIR=$SRCDIR/$1
  fi
fi

OBJDIR=$SRCDIR/"\$(ARCH)"objs
ELFDIR=$SRCDIR/"\$(ARCH)"elfobjs
AC_SUBST(SRCDIR)dnl
AC_SUBST(OBJDIR)dnl
AC_SUBST(ELFDIR)dnl
])
dnl#}}}

RPATH=""
AC_DEFUN([JD_INIT_RPATH], dnl#{{{
[
dnl# determine whether or not -R or -rpath can be used
case "$host_os" in
  *linux*|*solaris* )
    if test "X$GCC" = Xyes
    then
      if test "X$ac_R_nospace" = "Xno"
      then
        RPATH="-Wl,-R,"
      else
        RPATH="-Wl,-R"
      fi
    else
      if test "X$ac_R_nospace" = "Xno"
      then
        RPATH="-R "
      else
	RPATH="-R"
      fi
    fi
  ;;
  *osf*|*openbsd*)
    if test "X$GCC" = Xyes
    then
      RPATH="-Wl,-rpath,"
    else
      RPATH="-rpath "
    fi
  ;;
  *netbsd*)
    if test "X$GCC" = Xyes
    then
      RPATH="-Wl,-R"
    fi
  ;;
esac
])

dnl#}}}

AC_DEFUN([JD_SET_RPATH], dnl#{{{
[
if test "X$1" != "X"
then
  if test "X$RPATH" = "X"
  then
    JD_INIT_RPATH
    if test "X$RPATH" != "X"
    then
      RPATH="$RPATH$1"
    fi
  else
    RPATH="$RPATH:$1"
  fi
fi
])
AC_SUBST(RPATH)dnl

dnl#}}}

AC_DEFUN([JD_UPPERCASE], dnl#{{{
[
changequote(<<, >>)dnl
define(<<$2>>, translit($1, [a-z], [A-Z]))dnl
changequote([, ])dnl
])
#}}}

AC_DEFUN([JD_SIMPLE_LIB_DIR], dnl#{{{
[
JD_UPPERCASE($1,JD_UP_NAME)
JD_UP_NAME[]_LIB_DIR=$JD_Above_Dir/$1/libsrc/"$ARCH"objs
JD_UP_NAME[]_INCLUDE=$JD_Above_Dir/$1/libsrc

if test ! -d "[$]JD_UP_NAME[]_INCLUDE"
then
   JD_UP_NAME[]_LIB_DIR=$JD_Above_Dir/$1/src/"$ARCH"objs
   JD_UP_NAME[]_INCLUDE=$JD_Above_Dir/$1/src
   if test ! -d "[$]JD_UP_NAME[]_INCLUDE"
   then
     echo ""
     echo WARNING------Unable to find the JD_UP_NAME directory
     echo You may have to edit $CONFIG_DIR/src/Makefile.
     echo ""
   fi
fi

AC_SUBST(JD_UP_NAME[]_LIB_DIR)dnl
AC_SUBST(JD_UP_NAME[]_INCLUDE)dnl
undefine([JD_UP_NAME])dnl
])

dnl#}}}

AC_DEFUN([JD_FIND_GENERIC], dnl#{{{
[
  AC_REQUIRE([JD_EXPAND_PREFIX])dnl

  changequote(<<, >>)dnl
  define(<<JD_UP_NAME>>, translit($1, [a-z], [A-Z]))dnl
  changequote([, ])dnl
# Look for the JD_UP_NAME package
#JD_UP_NAME[]_INCLUDE=""
#JD_UP_NAME[]_LIB_DIR=""

# This list consists of "include,lib include,lib ..."
JD_Search_Dirs="$JD_Above_Dir2/$1/libsrc,$JD_Above_Dir2/$1/libsrc/"$ARCH"objs \
                $JD_Above_Dir/$1/libsrc,$JD_Above_Dir/$1/libsrc/"$ARCH"objs \
		$JD_Above_Dir2/$1/src,$JD_Above_Dir2/$1/src/"$ARCH"objs \
                $JD_Above_Dir/$1/src,$JD_Above_Dir/$1/src/"$ARCH"objs"

JD_Search_Dirs="$JD_Search_Dirs \
                $jd_prefix_incdir,$jd_prefix_libdir \
		$HOME/include,$HOME/lib"

if test -n "$ARCH"
then
 JD_Search_Dirs="$JD_Search_Dirs $HOME/include,$HOME/$ARCH/lib"
 JD_Search_Dirs="$JD_Search_Dirs $HOME/include,$HOME/sys/$ARCH/lib"
fi

# Now add the standard system includes.  The reason for doing this is that
# the other directories may have a better chance of containing a more recent
# version.

JD_Search_Dirs="$JD_Search_Dirs \
                /usr/local/include,/usr/local/lib \
		/usr/include,/usr/lib \
		/usr/include/$1,/usr/lib \
		/usr/include/$1,/usr/lib/$1"

echo looking for the JD_UP_NAME library

for include_and_lib in $JD_Search_Dirs
do
  # Yuk.  Is there a better way to set these variables??
  generic_include=`echo $include_and_lib | tr ',' ' ' | awk '{print [$]1}'`
  generic_lib=`echo $include_and_lib | tr ',' ' ' | awk '{print [$]2}'`
  echo Looking for $1.h in $generic_include
  echo and lib$1.a in $generic_lib
  if test -r $generic_include/$1.h && test -r $generic_lib/lib$1.a
  then
    echo Found it.
    JD_UP_NAME[]_LIB_DIR="$generic_lib"
    JD_UP_NAME[]_INCLUDE="$generic_include"
    break
  else
    if test -r $generic_include/$1.h && test -r $generic_lib/lib$1.so
    then
      echo Found it.
      JD_UP_NAME[]_LIB_DIR="$generic_lib"
      JD_UP_NAME[]_INCLUDE="$generic_include"
      break
    fi
  fi
done

if test -n "[$]JD_UP_NAME[]_LIB_DIR"
then
    jd_have_$1="yes"
else
    echo Unable to find the $JD_UP_NAME library.
    echo You may have to edit $CONFIG_DIR/src/Makefile.
    JD_UP_NAME[]_INCLUDE=$JD_Above_Dir/$1/src
    JD_UP_NAME[]_LIB_DIR=$JD_Above_Dir/$1/src/"$ARCH"objs
    jd_have_$1="no"
fi

JD_UP_NAME[]_INC="-I[$]JD_UP_NAME[]_INCLUDE"
JD_UP_NAME[]_LIB="-L[$]JD_UP_NAME[]_LIB_DIR"
JD_SET_RPATH([$]JD_UP_NAME[]_LIB_DIR)
dnl if test "X$GCC" = Xyes
dnl then
dnl    RPATH_[]JD_UP_NAME="-Wl,-R[$]JD_UP_NAME[]_LIB_DIR"
dnl else
dnl    RPATH_[]JD_UP_NAME="-R[$]JD_UP_NAME[]_LIB_DIR"
dnl fi

# gcc under solaris is often not installed correctly.  Avoid specifying
# -I/usr/include.
if test "[$]JD_UP_NAME[]_INC" = "-I/usr/include"
then
    JD_UP_NAME[]_INC=""
fi

if test "[$]JD_UP_NAME[]_LIB" = "-L/usr/lib"
then
    JD_UP_NAME[]_LIB=""
    RPATH_[]JD_UP_NAME=""
fi

AC_SUBST(JD_UP_NAME[]_LIB)dnl
AC_SUBST(JD_UP_NAME[]_INC)dnl
AC_SUBST(JD_UP_NAME[]_LIB_DIR)dnl
AC_SUBST(JD_UP_NAME[]_INCLUDE)dnl
dnl AC_SUBST(RPATH_[]JD_UP_NAME)dnl
undefine([JD_UP_NAME])dnl
])

dnl#}}}

AC_DEFUN([JD_FIND_SLANG], dnl#{{{
[
JD_FIND_GENERIC(slang)
])

dnl#}}}

AC_DEFUN([JD_GCC_WARNINGS], dnl#{{{
[
AC_ARG_ENABLE(warnings,
	      AC_HELP_STRING([--enable-warnings],[turn on GCC compiler warnings]),
	      [gcc_warnings=$enableval])
if test -n "$GCC"
then
  #CFLAGS="$CFLAGS -fno-strength-reduce"
  if test -n "$gcc_warnings"
  then
    CFLAGS="$CFLAGS -Wall -W -pedantic -Winline -Wmissing-prototypes \
 -Wnested-externs -Wpointer-arith -Wcast-align -Wshadow -Wstrict-prototypes \
 -Wformat=2"
    # Now trim excess whitespace
    CFLAGS=`echo $CFLAGS`
  fi
fi
])

dnl#}}}

IEEE_CFLAGS=""
AC_DEFUN([JD_IEEE_CFLAGS], dnl#{{{
[
case "$host_cpu" in
  *alpha* )
    if test "$GCC" = yes
    then
      IEEE_CFLAGS="-mieee"
    else
      IEEE_CFLAGS="-ieee_with_no_inexact"
    fi
    ;;
  * )
    IEEE_CFLAGS=""
esac
])

dnl#}}}

AC_DEFUN([JD_CREATE_ORULE], dnl#{{{
[
PROGRAM_OBJECT_RULES="$PROGRAM_OBJECT_RULES
\$(OBJDIR)/$1.o : \$(SRCDIR)/$1.c \$(DOT_O_DEPS) \$("$1"_O_DEP)
	cd \$(OBJDIR) && \$(COMPILE_CMD) \$("$1"_C_FLAGS) \$(SRCDIR)/$1.c
"
])

dnl#}}}

AC_DEFUN([JD_CREATE_ELFORULE], dnl#{{{
[
PROGRAM_ELF_ORULES="$PROGRAM_ELF_ORULES
\$(ELFDIR)/$1.o : \$(SRCDIR)/$1.c \$(DOT_O_DEPS) \$("$1"_O_DEP)
	cd \$(ELFDIR) && \$(ELFCOMPILE_CMD) \$("$1"_C_FLAGS) \$(SRCDIR)/$1.c
"
])

dnl#}}}

AC_DEFUN([JD_CREATE_EXEC_RULE], dnl#{{{
[
PROGRAM_OBJECT_RULES="$PROGRAM_OBJECT_RULES
$1 : \$(OBJDIR)/$1
	@echo $1 created in \$(OBJDIR)
\$(OBJDIR)/$1 : \$(OBJDIR)/$1.o \$("$1"_DEPS) \$(EXECDEPS)
	\$(CC) -o \$(OBJDIR)/$1 \$(LDFLAGS) \$(OBJDIR)/$1.o \$("$1"_LIBS) \$(EXECLIBS)
\$(OBJDIR)/$1.o : \$(SRCDIR)/$1.c \$(DOT_O_DEPS) \$("$1"_O_DEP)
	cd \$(OBJDIR) && \$(COMPILE_CMD) \$("$1"_INC) \$(EXECINC) \$(SRCDIR)/$1.c
"
])

dnl#}}}

AC_DEFUN([JD_CREATE_MODULE_ORULES], dnl#{{{
[
 for program_module in $Program_Modules; do
   JD_CREATE_ORULE($program_module)
   JD_CREATE_ELFORULE($program_module)
 done
])

dnl#}}}

AC_DEFUN([JD_GET_MODULES], dnl#{{{
[
 PROGRAM_HFILES=""
 PROGRAM_OFILES=""
 PROGRAM_CFILES=""
 PROGRAM_OBJECTS=""
 PROGRAM_ELFOBJECTS=""
 PROGRAM_OBJECT_RULES=""
 PROGRAM_ELF_ORULES=""
 if test -z "$1"
 then
   Program_Modules=""
 else
   comment_re="^#"
   Program_Modules=`grep -v '$comment_re' $1 | awk '{print [$]1}'`
   Program_H_Modules=`grep -v '$comment_re' $1 | awk '{print [$]2}'`
   for program_module in $Program_H_Modules; do
     PROGRAM_HFILES="$PROGRAM_HFILES $program_module"
   done
 fi
 for program_module in $Program_Modules; do
   PROGRAM_OFILES="$PROGRAM_OFILES $program_module.o"
   PROGRAM_CFILES="$PROGRAM_CFILES $program_module.c"
   PROGRAM_OBJECTS="$PROGRAM_OBJECTS \$(OBJDIR)/$program_module.o"
   PROGRAM_ELFOBJECTS="$PROGRAM_ELFOBJECTS \$(ELFDIR)/$program_module.o"
 done
dnl echo $PROGRAM_OFILES
dnl echo $PROGRAM_HFILES
AC_SUBST(PROGRAM_OFILES)dnl
AC_SUBST(PROGRAM_CFILES)dnl
AC_SUBST(PROGRAM_HFILES)dnl
AC_SUBST(PROGRAM_OBJECTS)dnl
AC_SUBST(PROGRAM_ELFOBJECTS)dnl
])

dnl#}}}

AC_DEFUN([JD_APPEND_RULES], dnl#{{{
[
 echo "$PROGRAM_OBJECT_RULES" >> $1
])

dnl#}}}

AC_DEFUN([JD_APPEND_ELFRULES], dnl#{{{
[
 echo "$PROGRAM_ELF_ORULES" >> $1
])

dnl#}}}

AC_DEFUN([JD_CREATE_MODULE_EXEC_RULES], dnl#{{{
[
 for program_module in $Program_Modules; do
   JD_CREATE_EXEC_RULE($program_module)
 done
])

dnl#}}}

AC_DEFUN([JD_TERMCAP], dnl#{{{
[
AC_PATH_PROG(nc5config, ncurses5-config, no)
if test "$nc5config" = "no"
then
  AC_PATH_PROG(nc5config, ncurses5w-config, no)
fi
AC_MSG_CHECKING(for terminfo)
if test "$nc5config" != "no"
then
   MISC_TERMINFO_DIRS=`$nc5config --terminfo`
else
   MISC_TERMINFO_DIRS=""
fi
JD_Terminfo_Dirs="$MISC_TERMINFO_DIRS \
                  /usr/lib/terminfo \
                  /usr/share/terminfo \
                  /usr/share/lib/terminfo \
		  /usr/local/lib/terminfo"
TERMCAP=-ltermcap

for terminfo_dir in $JD_Terminfo_Dirs
do
   if test -d $terminfo_dir
   then
      AC_MSG_RESULT(yes)
      TERMCAP=""
      break
   fi
done
if test "$TERMCAP"; then
  AC_MSG_RESULT(no)
  AC_DEFINE(USE_TERMCAP,1,[Define to use termcap])
fi
AC_SUBST(TERMCAP)dnl
AC_SUBST(MISC_TERMINFO_DIRS)dnl
])

dnl#}}}

AC_DEFUN([JD_ANSI_CC], dnl#{{{
[
AC_AIX
AC_REQUIRE([AC_PROG_CC])
AC_REQUIRE([AC_PROG_CPP])
AC_REQUIRE([AC_PROG_GCC_TRADITIONAL])
AC_ISC_POSIX

dnl #This stuff came from Yorick config script
dnl
dnl # HPUX needs special stuff
dnl
AC_EGREP_CPP(yes,
[#ifdef hpux
  yes
#endif
], [
AC_DEFINE(_HPUX_SOURCE,1,[Special define needed for HPUX])
if test "$CC" = cc; then CC="cc -Ae"; fi
])dnl
dnl
dnl #Be sure we've found compiler that understands prototypes
dnl
AC_MSG_CHECKING(C compiler that understands ANSI prototypes)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ ]], [[
 extern int silly (int);]])],[
 AC_MSG_RESULT($CC looks ok.  Good.)],[
 AC_MSG_RESULT($CC is not a good enough compiler)
 AC_MSG_ERROR(Set env variable CC to your ANSI compiler and rerun configure.)
 ])dnl
])dnl

dnl#}}}

AC_DEFUN([JD_ELF_COMPILER], dnl#{{{
[
dnl #-------------------------------------------------------------------------
dnl # Check for dynamic linker
dnl #-------------------------------------------------------------------------
DYNAMIC_LINK_LIB=""

dnl# AH_TEMPLATE([HAVE_DLOPEN],1,[Define if you have dlopen])

AC_CHECK_HEADER(dlfcn.h,[
  AC_DEFINE(HAVE_DLFCN_H,1,[Define if you have the dlfcn.h header])
  AC_CHECK_LIB(dl,dlopen,[
    DYNAMIC_LINK_LIB="-ldl"
    AC_DEFINE(HAVE_DLOPEN,1,[Define if you have dlopen])
   ],[
    AC_CHECK_FUNC(dlopen,AC_DEFINE(HAVE_DLOPEN,[Define if you have dlopen]))
    if test "$ac_cv_func_dlopen" != yes
    then
      AC_MSG_WARN(cannot perform dynamic linking)
    fi
   ])])
AC_SUBST(DYNAMIC_LINK_LIB)

if test "$GCC" = yes
then
  if test X"$CFLAGS" = X
  then
     CFLAGS="-O2"
  fi
fi

dnl #Some defaults
ELFLIB="lib\$(THIS_LIB).so"
ELFLIB_MAJOR="\$(ELFLIB).\$(ELF_MAJOR_VERSION)"
ELFLIB_MAJOR_MINOR="\$(ELFLIB_MAJOR).\$(ELF_MINOR_VERSION)"
ELFLIB_MAJOR_MINOR_MICRO="\$(ELFLIB_MAJOR_MINOR).\$(ELF_MICRO_VERSION)"

dnl# This specifies the target to use in the makefile to install the shared library
INSTALL_ELFLIB_TARGET="install-elf-and-links"
ELFLIB_BUILD_NAME="\$(ELFLIB_MAJOR_MINOR_MICRO)"
INSTALL_MODULE="\$(INSTALL_DATA)"
SLANG_DLL_CFLAGS=""
M_LIB="-lm"

case "$host_os" in
  *linux*|*gnu*|k*bsd*-gnu )
    DYNAMIC_LINK_FLAGS="-Wl,-export-dynamic"
    ELF_CC="\$(CC)"
    ELF_CFLAGS="\$(CFLAGS) -fPIC"
    ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-O1 -Wl,--version-script,\$(VERSION_SCRIPT) -Wl,-soname,\$(ELFLIB_MAJOR)"
    ELF_DEP_LIBS="\$(DL_LIB) -lm -lc"
    CC_SHARED="\$(CC) \$(CFLAGS) -shared -fPIC"
    ;;
  *solaris* )
    if test "$GCC" = yes
    then
      DYNAMIC_LINK_FLAGS=""
      ELF_CC="\$(CC)"
      ELF_CFLAGS="\$(CFLAGS) -fPIC"
      ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-ztext -Wl,-h,\$(ELFLIB_MAJOR)"
      ELF_DEP_LIBS="\$(DL_LIB) -lm -lc"
      CC_SHARED="\$(CC) \$(CFLAGS) -G -fPIC"
    else
      DYNAMIC_LINK_FLAGS=""
      ELF_CC="\$(CC)"
      ELF_CFLAGS="\$(CFLAGS) -K PIC"
      ELF_LINK="\$(CC) \$(LDFLAGS) -G -h\$(ELFLIB_MAJOR)"
      ELF_DEP_LIBS="\$(DL_LIB) -lm -lc"
      CC_SHARED="\$(CC) \$(CFLAGS) -G -K PIC"
    fi
    ;;
   # osr5 or unixware7 with current or late autoconf
  *sco3.2v5* | *unixware-5* | *sco-sysv5uw7*)
     if test "$GCC" = yes
     then
       DYNAMIC_LINK_FLAGS=""
       ELF_CC="\$(CC)"
       ELF_CFLAGS="\$(CFLAGS) -fPIC"
       ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-h,\$(ELFLIB_MAJOR)"
       ELF_DEP_LIBS=
       CC_SHARED="\$(CC) \$(CFLAGS) -G -fPIC"
     else
       DYNAMIC_LINK_FLAGS=""
       ELF_CC="\$(CC)"
       ELF_CFLAGS="\$(CFLAGS) -K pic"
       # ELF_LINK="ld -G -z text -h#"
       ELF_LINK="\$(CC) \$(LDFLAGS) -G -z text -h\$(ELFLIB_MAJOR)"
       ELF_DEP_LIBS=
       CC_SHARED="\$(CC) \$(CFLAGS) -G -K pic"
     fi
     ;;
  *irix6.5* )
     echo "Note: ELF compiler for host_os=$host_os may not be correct"
     echo "double-check: 'mode_t', 'pid_t' may be wrong!"
     if test "$GCC" = yes
     then
       # not tested
       DYNAMIC_LINK_FLAGS=""
       ELF_CC="\$(CC)"
       ELF_CFLAGS="\$(CFLAGS) -fPIC"
       ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-h,\$(ELFLIB_MAJOR)"
       ELF_DEP_LIBS=
       CC_SHARED="\$(CC) \$(CFLAGS) -shared -fPIC"
     else
       DYNAMIC_LINK_FLAGS=""
       ELF_CC="\$(CC)"
       ELF_CFLAGS="\$(CFLAGS)"     # default anyhow
       ELF_LINK="\$(CC) \$(LDFLAGS) -shared -o \$(ELFLIB_MAJOR)"
       ELF_DEP_LIBS=
       CC_SHARED="\$(CC) \$(CFLAGS) -shared"
     fi
     ;;
  *darwin* )
     DYNAMIC_LINK_FLAGS=""
     ELF_CC="\$(CC)"
     ELF_CFLAGS="\$(CFLAGS) -fno-common"
     ELF_LINK="\$(CC) \$(LDFLAGS) -dynamiclib -install_name \$(install_lib_dir)/\$(ELFLIB_MAJOR) -compatibility_version \$(ELF_MAJOR_VERSION) -current_version \$(ELF_MAJOR_VERSION).\$(ELF_MINOR_VERSION)"
     ELF_DEP_LIBS="\$(LDFLAGS) \$(DL_LIB)"
     CC_SHARED="\$(CC) -bundle -flat_namespace -undefined suppress \$(CFLAGS) -fno-common"
     ELFLIB="lib\$(THIS_LIB).dylib"
     ELFLIB_MAJOR="lib\$(THIS_LIB).\$(ELF_MAJOR_VERSION).dylib"
     ELFLIB_MAJOR_MINOR="lib\$(THIS_LIB).\$(ELF_MAJOR_VERSION).\$(ELF_MINOR_VERSION).dylib"
     ELFLIB_MAJOR_MINOR_MICRO="lib\$(THIS_LIB).\$(ELF_MAJOR_VERSION).\$(ELF_MINOR_VERSION).\$(ELF_MICRO_VERSION).dylib"
     ;;
  *freebsd* )
    ELF_CC="\$(CC)"
    ELF_CFLAGS="\$(CFLAGS) -fPIC"
    #if test "X$PORTOBJFORMAT" = "Xelf" ; then
    #  ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-soname,\$(ELFLIB_MAJOR)"
    #else
    #  ELF_LINK="ld -Bshareable -x"
    #fi
    ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-soname,\$(ELFLIB_MAJOR)"
    ELF_DEP_LIBS="\$(DL_LIB) -lm"
    CC_SHARED="\$(CC) \$(CFLAGS) -shared -fPIC"
    ;;
  *cygwin* )
    DYNAMIC_LINK_FLAGS=""
    ELF_CC="\$(CC)"
    SLANG_DLL_CFLAGS="-DSLANG_DLL=1"
    ELF_CFLAGS="\$(CFLAGS) -DBUILD_DLL=1"
    DLL_IMPLIB_NAME="lib\$(THIS_LIB)\$(ELFLIB_MAJOR_VERSION).dll.a"
    #ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-O1 -Wl,--version-script,\$(VERSION_SCRIPT) -Wl,-soname,\$(ELFLIB_MAJOR) -Wl,--out-implib=\$(DLL_IMPLIB_NAME) -Wl,-export-all-symbols -Wl,-enable-auto-import"
    ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-O1 -Wl,--version-script,\$(VERSION_SCRIPT) -Wl,-soname,\$(ELFLIB_MAJOR) -Wl,--out-implib=\$(DLL_IMPLIB_NAME)"
    ELF_DEP_LIBS="\$(DL_LIB) -lm"
    CC_SHARED="\$(CC) \$(CFLAGS) -shared -DSLANG_DLL=1"
    dnl# CYGWIN prohibits undefined symbols when linking shared libs
    SLANG_LIB_FOR_MODULES="-L\$(ELFDIR) -lslang"
    INSTALL_MODULE="\$(INSTALL)"
    INSTALL_ELFLIB_TARGET="install-elf-cygwin"
    ELFLIB="lib\$(THIS_LIB).dll"
    ELFLIB_MAJOR="cyg\$(THIS_LIB)-\$(ELF_MAJOR_VERSION).dll"
    ELFLIB_MAJOR_MINOR="cyg\$(THIS_LIB)-\$(ELF_MAJOR_VERSION)_\$(ELF_MINOR_VERSION).dll"
    ELFLIB_MAJOR_MINOR_MICRO="cyg\$(THIS_LIB)-\$(ELF_MAJOR_VERSION)_\$(ELF_MINOR_VERSION)_\$(ELF_MICRO_VERSION).dll"
    ELFLIB_BUILD_NAME="\$(ELFLIB_MAJOR)"
    ;;
  *haiku* )
    M_LIB=""
    DYNAMIC_LINK_FLAGS="-Wl,-export-dynamic"
    ELF_CC="\$(CC)"
    ELF_CFLAGS="\$(CFLAGS) -fPIC"
    ELF_LINK="\$(CC) \$(LDFLAGS) -shared -Wl,-O1 -Wl,--version-script,\$(VERSION_SCRIPT) -Wl,-soname,\$(ELFLIB_MAJOR)"
    ELF_DEP_LIBS="\$(DL_LIB)"
    CC_SHARED="\$(CC) \$(CFLAGS) -shared -fPIC"
    ;;
  * )
    echo "Note: ELF compiler for host_os=$host_os may be wrong"
    ELF_CC="\$(CC)"
    ELF_CFLAGS="\$(CFLAGS) -fPIC"
    ELF_LINK="\$(CC) \$(LDFLAGS) -shared"
    ELF_DEP_LIBS="\$(DL_LIB) -lm -lc"
    CC_SHARED="\$(CC) \$(CFLAGS) -shared -fPIC"
esac

AC_SUBST(ELF_CC)
AC_SUBST(ELF_CFLAGS)
AC_SUBST(ELF_LINK)
AC_SUBST(ELF_LINK_CMD)
AC_SUBST(ELF_DEP_LIBS)
AC_SUBST(DYNAMIC_LINK_FLAGS)
AC_SUBST(CC_SHARED)
AC_SUBST(ELFLIB)
AC_SUBST(ELFLIB_MAJOR)
AC_SUBST(ELFLIB_MAJOR_MINOR)
AC_SUBST(ELFLIB_MAJOR_MINOR_MICRO)
AC_SUBST(SLANG_LIB_FOR_MODULES)
AC_SUBST(DLL_IMPLIB_NAME)
AC_SUBST(INSTALL_MODULE)
AC_SUBST(INSTALL_ELFLIB_TARGET)
AC_SUBST(ELFLIB_BUILD_NAME)
AC_SUBST(SLANG_DLL_CFLAGS)
AC_SUBST(M_LIB)
])

dnl#}}}

AC_DEFUN([JD_F77_COMPILER], dnl#{{{
[
case "$host_os" in
 *linux* )
   F77="g77"
   F77_LIBS="-lg2c"
 ;;
 *solaris*)
   F77=f77
   #F77_LIBS="-lF77 -lM77 -L/opt/SUNWspro/SC4.0/lib -lsunmath"
   F77_LIBS="-lF77 -lM77 -lsunmath"
   ;;
 *)
   echo ""
   echo "WARNING: Assuming f77 as your FORTRAN compiler"
   echo ""
   F77=f77
   F77_LIBS=""
esac
AC_SUBST(F77)
AC_SUBST(F77_LIBS)
])

dnl#}}}

dnl# This macro process the --with-xxx, --with-xxxinc, and --with-xxxlib
dnl# command line arguments and returns the values as shell variables
dnl# jd_xxx_include_dir and jd_xxx_library_dir.  It does not perform any
dnl# substitutions, nor check for the existence of the supplied values.
AC_DEFUN([JD_WITH_LIBRARY_PATHS], dnl#{{{
[
 JD_UPPERCASE($1,JD_ARG1)
 jd_$1_include_dir=""
 jd_$1_library_dir=""
 if test X"$jd_with_$1_library" = X
 then
   jd_with_$1_library=""
 fi

 AC_ARG_WITH($1,
  [  --with-$1=DIR      Use DIR/lib and DIR/include for $1],
  [jd_with_$1_arg=$withval], [jd_with_$1_arg=unspecified])

 case "x$jd_with_$1_arg" in
   xno)
     jd_with_$1_library="no"
    ;;
   x)
    dnl# AC_MSG_ERROR(--with-$1 requires a value-- try yes or no)
    jd_with_$1_library="yes"
    ;;
   xunspecified)
    ;;
   xyes)
    jd_with_$1_library="yes"
    ;;
   *)
    jd_with_$1_library="yes"
    jd_$1_include_dir="$jd_with_$1_arg"/include
    jd_$1_library_dir="$jd_with_$1_arg"/lib
    ;;
 esac

 AC_ARG_WITH($1lib,
  [  --with-$1lib=DIR   $1 library in DIR],
  [jd_with_$1lib_arg=$withval], [jd_with_$1lib_arg=unspecified])
 case "x$jd_with_$1lib_arg" in
   xunspecified)
    ;;
   xno)
    ;;
   x)
    AC_MSG_ERROR(--with-$1lib requres a value)
    ;;
   *)
    jd_with_$1_library="yes"
    jd_$1_library_dir="$jd_with_$1lib_arg"
    ;;
 esac

 AC_ARG_WITH($1inc,
  [  --with-$1inc=DIR   $1 include files in DIR],
  [jd_with_$1inc_arg=$withval], [jd_with_$1inc_arg=unspecified])
 case "x$jd_with_$1inc_arg" in
   x)
     AC_MSG_ERROR(--with-$1inc requres a value)
     ;;
   xunspecified)
     ;;
   xno)
     ;;
   *)
    jd_with_$1_library="yes"
    jd_$1_include_dir="$jd_with_$1inc_arg"
   ;;
 esac
])
dnl#}}}

dnl# This function checks for the existence of the specified library $1 with
dnl# header file $2.  If the library exists, then the shell variables will
dnl# be created:
dnl#  jd_with_$1_library=yes/no,
dnl#  jd_$1_inc_file
dnl#  jd_$1_include_dir
dnl#  jd_$1_library_dir
dnl# If $3 is present, then also look in $3/include+$3/lib
AC_DEFUN([JD_CHECK_FOR_LIBRARY], dnl#{{{
[
  AC_REQUIRE([JD_EXPAND_PREFIX])dnl
  dnl JD_UPPERCASE($1,JD_ARG1)
  JD_WITH_LIBRARY_PATHS($1)
  AC_MSG_CHECKING(for the $1 library and header files $2)
  if test X"$jd_with_$1_library" != Xno
  then
    jd_$1_inc_file=$2
    dnl# jd_with_$1_library="yes"

    if test "X$jd_$1_inc_file" = "X"
    then
       jd_$1_inc_file=$1.h
    fi

    if test X"$jd_$1_include_dir" = X
    then
      inc_and_lib_dirs="\
         $jd_prefix_incdir,$jd_prefix_libdir \
	 /usr/local/$1/include,/usr/local/$1/lib \
	 /usr/local/include/$1,/usr/local/lib \
	 /usr/local/include,/usr/local/lib \
	 /usr/include/$1,/usr/lib \
	 /usr/$1/include,/usr/$1/lib \
	 /usr/include,/usr/lib \
	 /opt/include/$1,/opt/lib \
	 /opt/$1/include,/opt/$1/lib \
	 /opt/include,/opt/lib"

      if test X$3 != X
      then
        inc_and_lib_dirs="$3/include,$3/lib $inc_and_lib_dirs"
      fi

      case "$host_os" in
         *darwin* )
	   exts="dylib so a"
	   ;;
	 *cygwin* )
	   exts="dll.a so a"
	   ;;
	 * )
	   exts="so a"
      esac

      xincfile="$jd_$1_inc_file"
      xlibfile="lib$1"
      jd_with_$1_library="no"

      for include_and_lib in $inc_and_lib_dirs
      do
        # Yuk.  Is there a better way to set these variables??
        xincdir=`echo $include_and_lib | tr ',' ' ' | awk '{print [$]1}'`
	xlibdir=`echo $include_and_lib | tr ',' ' ' | awk '{print [$]2}'`
	found=0
	if test -r $xincdir/$xincfile
	then
	  for E in $exts
	  do
	    if test -r "$xlibdir/$xlibfile.$E"
	    then
	      jd_$1_include_dir="$xincdir"
	      jd_$1_library_dir="$xlibdir"
	      jd_with_$1_library="yes"
	      found=1
	      break
	    fi
	  done
	fi
	if test $found -eq 1
	then
	  break
	fi
      done
    fi
  fi

  if test X"$jd_$1_include_dir" != X -a X"$jd_$1_library_dir" != X
  then
    AC_MSG_RESULT(yes: $jd_$1_library_dir and $jd_$1_include_dir)
    jd_with_$1_library="yes"
    dnl#  Avoid using /usr/lib and /usr/include because of problems with
    dnl#  gcc on some solaris systems.
    JD_ARG1[]_LIB=-L$jd_$1_library_dir
    JD_ARG1[]_LIB_DIR=$jd_$1_library_dir
    if test "X$jd_$1_library_dir" = "X/usr/lib"
    then
      JD_ARG1[]_LIB=""
    else
      JD_SET_RPATH($jd_$1_library_dir)
    fi

    JD_ARG1[]_INC=-I$jd_$1_include_dir
    JD_ARG1[]_INC_DIR=$jd_$1_include_dir
    if test "X$jd_$1_include_dir" = "X/usr/include"
    then
      JD_ARG1[]_INC=""
    fi
  else
    AC_MSG_RESULT(no)
    jd_with_$1_library="no"
    JD_ARG1[]_INC=""
    JD_ARG1[]_LIB=""
    JD_ARG1[]_INC_DIR=""
    JD_ARG1[]_LIB_DIR=""
  fi
  AC_SUBST(JD_ARG1[]_LIB)
  AC_SUBST(JD_ARG1[]_INC)
  AC_SUBST(JD_ARG1[]_LIB_DIR)
  AC_SUBST(JD_ARG1[]_INC_DIR)
])
dnl#}}}

AC_DEFUN([JD_WITH_LIBRARY], dnl#{{{
[
  JD_CHECK_FOR_LIBRARY($1, $2, $3)
  if test "$jd_with_$1_library" = "no"
  then
    AC_MSG_ERROR(unable to find the $1 library and header file $jd_$1_inc_file)
  fi
])
dnl#}}}

AC_DEFUN([JD_SLANG_VERSION], dnl#{{{
[
 slang_h=$jd_slang_include_dir/slang.h
 AC_MSG_CHECKING(SLANG_VERSION in $slang_h)
slang_version=`grep "^#define  *SLANG_VERSION " $slang_h |
               awk '{ print [$]3 }'`
slang_major_version=`echo $slang_version |
 awk '{ print int([$]1/10000) }'`
slang_minor_version=`echo $slang_version $slang_major_version |
 awk '{ print int(([$]1 - [$]2*10000)/100) }'`
slang_patchlevel_version=`echo $slang_version $slang_major_version $slang_minor_version |
 awk '{ print ([$]1 - [$]2*10000 - [$]3*100) }'`

AC_MSG_RESULT($slang_major_version.$slang_minor_version.$slang_patchlevel_version)
AC_SUBST(slang_version)
AC_SUBST(slang_major_version)
AC_SUBST(slang_minor_version)
AC_SUBST(slang_patchlevel_version)
])
#}}}

AC_DEFUN([JD_SLANG_MODULE_INSTALL_DIR], dnl#{{{
[
  AC_REQUIRE([JD_SLANG_VERSION])
  if test "X$slang_major_version" = "X1"
  then
    MODULE_INSTALL_DIR="$libdir/slang/modules"
  else
    MODULE_INSTALL_DIR="$libdir/slang/v$slang_major_version/modules"
  fi
  SL_FILES_INSTALL_DIR=$datadir/slsh/local-packages
  AC_SUBST(MODULE_INSTALL_DIR)
  AC_SUBST(SL_FILES_INSTALL_DIR)
])
#}}}

AC_DEFUN([JD_CHECK_LONG_LONG], dnl#{{{
[
  AC_CHECK_TYPES(long long)
  AC_CHECK_SIZEOF(long long)
])
dnl#}}}

AC_DEFUN([JD_LARGE_FILE_SUPPORTXXX], dnl#{{{
[
  AC_REQUIRE([JD_CHECK_LONG_LONG])
  AC_MSG_CHECKING(whether to explicitly activate long file support)
  AC_DEFINE(_LARGEFILE_SOURCE, 1)
  AC_DEFINE(_FILE_OFFSET_BITS, 64)
  jd_large_file_support=no
  if test X$ac_cv_type_long_long = Xyes
  then
    if test $ac_cv_sizeof_long_long -ge 8
    then
      jd_large_file_support=yes
    fi
  fi

  if test $jd_large_file_support = yes
  then
    AC_DEFINE(HAVE_LARGEFILE_SUPPORT, 1)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
])
dnl#}}}

AC_DEFUN([JD_LARGE_FILE_SUPPORT], dnl#{{{
[
  AC_SYS_LARGEFILE
  AC_FUNC_FSEEKO
  AC_TYPE_OFF_T
  AC_CHECK_SIZEOF(off_t)
])
#}}}

AC_DEFUN([JD_HAVE_ISINF], dnl#{{{
[
  AC_MSG_CHECKING([for isinf])
  AC_LINK_IFELSE([AC_LANG_PROGRAM( [[#include <math.h>]], [[isinf (0.0);]])],
                 [AC_MSG_RESULT([yes])
                  AC_DEFINE(HAVE_ISINF, 1)])
])
#}}}
