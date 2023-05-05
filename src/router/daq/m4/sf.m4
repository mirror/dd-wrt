dnl Enable visibility if we can
dnl modified from gnulib/m4/visibility.m4
AC_DEFUN([AC_ENABLE_VISIBILITY],
[
    AC_REQUIRE([AC_PROG_CC])
    AC_MSG_CHECKING([for visibility support])
    AC_CACHE_VAL(gl_cv_cc_visibility, [
        gl_save_CFLAGS="$CFLAGS"
        # Add -Werror flag since some compilers, e.g. icc 7.1, don't support it,
        # but only warn about it instead of compilation failing
        CFLAGS="$CFLAGS -Werror -fvisibility=hidden"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            extern __attribute__((__visibility__("hidden"))) int hiddenvar;
            extern __attribute__((__visibility__("default"))) int exportedvar;
            extern __attribute__((__visibility__("hidden"))) int hiddenfunc (void);
            extern __attribute__((__visibility__("default"))) int exportedfunc (void);]],
            [[]])],
            [gl_cv_cc_visibility="yes"],
            [gl_cv_cc_visibility="no"])
    ])
    AC_MSG_RESULT([$gl_cv_cc_visibility])
    if test "x$gl_cv_cc_visibility" = "xyes"; then
        CFLAGS="$gl_save_CFLAGS -fvisibility=hidden"
        AC_DEFINE([HAVE_VISIBILITY],[1],
            [Define if the compiler supports visibility declarations.])
    else
        CFLAGS="$gl_save_CFLAGS"
    fi
])

dnl Special compiler flags for ICC
dnl GCC strict CFLAGS
AC_DEFUN([AC_SF_COMPILER_SETUP],
   [AC_REQUIRE([AC_PROG_CC])
    ICC=no
    if eval "echo $CC | grep icc > /dev/null" ; then
        if eval "$CC -help | grep libcxa > /dev/null" ; then
            CFLAGS="$CFLAGS -static-libcxa"
            LDFLAGS="$LDFLAGS -static-libcxa"
            XCCFLAGS="-XCClinker -static-libcxa"
        else
            CFLAGS="$CFLAGS -static-intel"
            LDFLAGS="$LDFLAGS -static-intel"
            XCCFLAGS="-XCClinker -static-intel"
        fi
        CFLAGS=`echo $CFLAGS | sed 's/-O2/-O3/'`
        CFLAGS="$CFLAGS -ip -w1"
        ICC=yes
        GCC=no
    fi

    if test "$GCC" = yes ; then
        AX_CHECK_COMPILE_FLAG(-Wall, [AM_CFLAGS="${AM_CFLAGS} -Wall"])
        AX_CHECK_COMPILE_FLAG(-Wwrite-strings, [AM_CFLAGS="${AM_CFLAGS} -Wwrite-strings"])
        AX_CHECK_COMPILE_FLAG(-Wsign-compare, [AM_CFLAGS="${AM_CFLAGS} -Wsign-compare"])
        AX_CHECK_COMPILE_FLAG(-Wcast-align, [AM_CFLAGS="${AM_CFLAGS} -Wcast-align"])
        AX_CHECK_COMPILE_FLAG(-Wextra, [AM_CFLAGS="${AM_CFLAGS} -Wextra"])
        AX_CHECK_COMPILE_FLAG(-Wformat, [AM_CFLAGS="${AM_CFLAGS} -Wformat"])
        AX_CHECK_COMPILE_FLAG(-Wformat-security, [AM_CFLAGS="${AM_CFLAGS} -Wformat-security"])
        AX_CHECK_COMPILE_FLAG(-Wno-unused-parameter, [AM_CFLAGS="${AM_CFLAGS} -Wno-unused-parameter"])
        AX_CHECK_COMPILE_FLAG(-fno-strict-aliasing, [AM_CFLAGS="${AM_CFLAGS} -fno-strict-aliasing"])
        AX_CHECK_COMPILE_FLAG(-fdiagnostics-show-option, [AM_CFLAGS="${AM_CFLAGS} -fdiagnostics-show-option"])
        AX_CHECK_COMPILE_FLAG(-pedantic -std=c99 -D_GNU_SOURCE, [AM_CFLAGS="${AM_CFLAGS} --pedantic -std=c99 -D_GNU_SOURCE"])
    fi
])

dnl
dnl Check for flex, default to lex
dnl Require flex 2.4 or higher
dnl Check for bison, default to yacc
dnl Default to lex/yacc if both flex and bison are not available
dnl Define the yy prefix string if using flex and bison
dnl
dnl usage:
dnl
dnl AC_LBL_LEX_AND_YACC(lex, yacc, yyprefix)
dnl
dnl results:
dnl
dnl $1 (lex set)
dnl $2 (yacc appended)
dnl $3 (optional flex and bison -P prefix)
dnl
AC_DEFUN([AC_LBL_LEX_AND_YACC],
    [AC_ARG_WITH(flex, [  --without-flex          don't use flex])
    AC_ARG_WITH(bison, [  --without-bison         don't use bison])
    if test "$with_flex" = no ; then
        $1=lex
    else
        AC_CHECK_PROGS($1, flex, lex)
    fi
    if test "$$1" = flex ; then
        # The -V flag was added in 2.4
        AC_MSG_CHECKING(for flex 2.4 or higher)
        AC_CACHE_VAL(ac_cv_lbl_flex_v24,
        if flex -V >/dev/null 2>&1; then
            ac_cv_lbl_flex_v24=yes
        else
            ac_cv_lbl_flex_v24=no
        fi)
        AC_MSG_RESULT($ac_cv_lbl_flex_v24)
        if test $ac_cv_lbl_flex_v24 = no ; then
            s="2.4 or higher required"
            AC_MSG_WARN(ignoring obsolete flex executable ($s))
            $1=lex
        fi
    fi
    if test "$with_bison" = no ; then
        $2=yacc
    else
        AC_CHECK_PROGS($2, bison, yacc)
    fi
    if test "$$2" = bison ; then
        $2="$$2 -y"
    fi
    if test "$$1" != lex -a "$$2" = yacc -o "$$1" = lex -a "$$2" != yacc ; then
        AC_MSG_WARN(don't have both flex and bison; reverting to lex/yacc)
        $1=lex
        $2=yacc
    fi
    if test "$$1" = flex -a -n "$3" ; then
        $1="$$1 -P$3"
        $2="$$2 -p $3"
    fi])

AC_DEFUN([AC_CHECK_PCAP_VER],
[
    AC_REQUIRE([AC_PROG_CC])
    AC_MSG_CHECKING([for pcap_lib_version])
    AC_CHECK_LIB([pcap],[pcap_lib_version],[LIBS="-lpcap ${LIBS}"],[have_pcap_lib_version="no"],[])
    if test "x$have_pcap_lib_version" = "xno"; then
        echo
        echo "    ERROR!  Libpcap library version >= $1 not found."
        echo "    Get it from http://www.tcpdump.org"
        echo
        exit 1
    fi
    AC_CACHE_CHECK([for libpcap version >= $1], [daq_cv_libpcap_version_1x], [
    AC_RUN_IFELSE(
    [AC_LANG_PROGRAM(
    [[
    #include <pcap.h>
    #include <string.h>
    extern char pcap_version[];
    ]],
    [[
        if (strcmp(pcap_version, $1) < 0)
            return 1;
    ]])],
    [daq_cv_libpcap_version_1x="yes"],
    [daq_cv_libpcap_version_1x="no"])])
    if test "x$daq_cv_libpcap_version_1x" = "xno"; then
        echo
        echo "    ERROR!  Libpcap library version >= $1  not found."
        echo "    Get it from http://www.tcpdump.org"
        echo
        exit 1
    fi
])
