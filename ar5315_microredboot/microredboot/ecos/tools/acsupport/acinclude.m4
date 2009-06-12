dnl Process this file with aclocal to get an aclocal.m4 file. Then
dnl process that with autoconf.
dnl ====================================================================
dnl
dnl     acinclude.m4
dnl
dnl     Various autoconf macros that are shared between different
dnl     eCos packages.
dnl
dnl ====================================================================
dnl ####ECOSHOSTGPLCOPYRIGHTBEGIN####
dnl ----------------------------------------------------------------------------
dnl Copyright (C) 2002, 2003 Bart Veer
dnl Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
dnl
dnl This file is part of the eCos host tools.
dnl
dnl This program is free software; you can redistribute it and/or modify it 
dnl under the terms of the GNU General Public License as published by the Free 
dnl Software Foundation; either version 2 of the License, or (at your option) 
dnl any later version.
dnl 
dnl This program is distributed in the hope that it will be useful, but WITHOUT 
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
dnl FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
dnl more details.
dnl 
dnl You should have received a copy of the GNU General Public License along with
dnl this program; if not, write to the Free Software Foundation, Inc., 
dnl 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
dnl
dnl ----------------------------------------------------------------------------
dnl ####ECOSHOSTGPLCOPYRIGHTEND####
dnl ====================================================================
dnl#####DESCRIPTIONBEGIN####
dnl
dnl Author(s):  bartv
dnl Contact(s): bartv
dnl Date:       1998/12/16
dnl Version:    0.01
dnl
dnl####DESCRIPTIONEND####
dnl ====================================================================

dnl ====================================================================
dnl Ensure that configure is not being run in the source tree, i.e.
dnl that a separate build tree has been created. This is not absolutely
dnl necessary at the time of writing but may become so in future, and
dnl is good practice.

AC_DEFUN(ECOS_CHECK_BUILD_ne_SRC,[
    AC_MSG_CHECKING([that a separate build tree is being used])
    ecos_cwd=`/bin/pwd`
    if test "${srcdir}" = "." ; then
        srcdir=${ecos_cwd}
    fi
    if test "${ecos_cwd}" = "${srcdir}" ; then
        AC_MSG_RESULT([no])
        AC_MSG_ERROR([This configure script should not be run inside the source tree. Instead please use a separate build tree])
    else
        AC_MSG_RESULT(yes)
    fi
])

dnl ====================================================================
dnl The AM_INIT_AUTOMAKE() will define a symbol VERSION for the
dnl package's version number. Unfortunately this symbol is rather
dnl hard to share if several different packages are involved, so this
dnl macro is used to define an alternative symbol

AC_DEFUN(ECOS_SUBST_VERSION,[
    AC_REQUIRE([AM_INIT_AUTOMAKE])
    ifelse($#,1,,AC_MSG_ERROR([Invalid number of arguments passed to ECOS SUBST_VERSION]))
    AC_DEFINE_UNQUOTED($1, "$VERSION")
])

dnl --------------------------------------------------------------------
dnl Convert a cygwin pathname to something acceptable to VC++ (but
dnl still invoked from bash and cygwin's make). This means using
dnl the cygpath utility and then translating any backslashes into
dnl forward slashes to avoid confusing make.

AC_DEFUN(ECOS_MSVC_PATH, [
    AC_REQUIRE([ECOS_PROG_MSVC])
    ifelse($#, 1, , AC_MSG_ERROR("Invalid number of arguments passed to ECOS MSVC_PATH"))
    if test "${MSVC}" = "yes" ; then
      $1=`cygpath -w ${$1} | tr \\\\\\\\ /`
    fi
])

dnl ====================================================================
dnl An internal utility to define eCos variants of various compilation
dnl related flags. The aim is to avoid messing with CFLAGS, LIBS, and
dnl so on because those are used for feature tests as well as for
dnl passing on to the application.
AC_DEFUN(ECOS_PROG_DEFINE_COMPILER_FLAGS,[
    ecos_CFLAGS=""
    ecos_CXXFLAGS=""
    ecos_LDADD=""
    ecos_INCLUDES=""
    ecos_LIBS=""
    AC_SUBST(ecos_CFLAGS)
    AC_SUBST(ecos_CXXFLAGS)
    AC_SUBST(ecos_LDADD)
    AC_SUBST(ecos_INCLUDES)
    AC_SUBST(ecos_LIBS)
])

dnl For historical reasons some of the eCos host-side software can be
dnl built with Visual C++ as well as g++. The user can specify this
dnl at configure time using CC=cl, where cl.exe is the compiler driver.
dnl This macro will set the variable MSVC to "yes" or to "no" depending
dnl on whether or not VC++ is being used, analogous to the variable
dnl GCC set by AC_PROG_CC. It provides support for an automake
dnl conditional thus allowing the makefile to adapt somewhat to the
dnl compiler being used. Finally it fills in the ECOS_INCLUDES,
dnl ECOS_LIBS and ECOS_LDADD variables with suitable initial values.

AC_DEFUN(ECOS_PROG_MSVC,[
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([ECOS_PROG_DEFINE_COMPILER_FLAGS])

    AC_MSG_CHECKING("for Visual C++")
    MSVC="no";
    if test "${CC}" = "cl" ; then
       MSVC="yes"
       CXX="cl"
       MSVC_SRCDIR=${srcdir}
       ECOS_MSVC_PATH(MSVC_SRCDIR)
       AC_SUBST(MSVC_SRCDIR)
       ecos_INCLUDES="${ecos_INCLUDES} \"-I${MSVC_SRCDIR}\""
       ecos_LDADD="-link"
       ecos_LIBS="advapi32.lib"
    fi
    AM_CONDITIONAL(MSVC, test "${MSVC}" = "yes")
    if test "${MSVC}" = "yes" ; then
        AC_MSG_RESULT([unfortunately yes])
    else
        AC_MSG_RESULT([no])
    fi
])

dnl ====================================================================
dnl Set up sensible flags for the various different compilers. This
dnl is achieved by manipulating AM-CFLAGS and AM-CXXFLAGS via a subst,
dnl plus undoing the setting of CFLAGS and CXXFLAGS done by
dnl the AC_PROC_CC and AC_PROG_CXX macros (e.g. setting the default
dnl compilation flags to -O2). Note that this relies
dnl on knowing about the internals of those macros.
dnl
dnl There is little point in checking the cache: this macro does
dnl not do any feature tests so checking the cache would probably
dnl be more expensive than doing the work here.
dnl
dnl For now the only supported compilers are gcc/g++ and VC++. Attempts
dnl to use another compiler will result in an error at configure-time.
AC_DEFUN(ECOS_PROG_STANDARD_COMPILER_FLAGS, [
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])
    AC_REQUIRE([ECOS_PROG_DEFINE_COMPILER_FLAGS])
    AC_REQUIRE([ECOS_PROG_MSVC])

    AC_MSG_CHECKING("the default compiler flags")

    dnl Add a user-settable flag to control whether or debugging info is
    dnl incorporated at compile-time.
    ecosflags_enable_debug="no"
    AC_ARG_ENABLE(debug,[ --enable-debug           do a debug rather than a release build],
    [case "${enableval}" in
       yes) ecosflags_enable_debug="yes" ;;
       *)   ecosflags_enable_debug="no" ;;
    esac])

    dnl For VC++ builds also provide a flag for ANSI vs. unicode builds.
    dnl For now this does not actually affect the compiler flags.
    dnl NOTE: there may also have to be a flag to control whether or
    dnl the VC++ multi-threading flags are enabled.
    ecosflags_enable_ansi="no"
    if test "${MSVC}" = "yes" ; then
      AC_ARG_ENABLE(ansi,[ --enable-ansi            do an ANSI rather than a unicode build],
      [case "${enableval}" in
         yes) ecosflags_enable_ansi="yes" ;;
         *)   ecosflags_enable_ansi="no" ;;
      esac])
    fi

    dnl Now we know what the user is after.
    if test "${GCC}" = "yes" ; then
        ecos_CFLAGS="${ecos_CFLAGS} -pipe -Wall -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs"
        ecos_CXXFLAGS="${ecos_CXXFLAGS} -pipe -Wall -Wpointer-arith -Wbad-function-cast -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Woverloaded-virtual"
    elif test "${MSVC}" = "yes" ; then
        ecos_CFLAGS="${ecos_CFLAGS} -nologo -W3"
        ecos_CXXFLAGS="${ecos_CXXFLAGS} -nologo -W3 -GR -GX"
    else
        AC_MSG_ERROR("default flags for ${CC} are not known")
    fi

    dnl Choose between debugging and optimization.
    if test "${ecosflags_enable_debug}" = "yes" ; then
        if test "${GCC}" = "yes" ; then
            ecos_CFLAGS="${ecos_CFLAGS} -g -O0"
            ecos_CXXFLAGS="${ecos_CXXFLAGS} -g -O0"
        elif test "${MSVC}" = "yes" ; then
            ecos_CFLAGS="${ecos_CFLAGS} -MDd -Zi"
            ecos_CXXFLAGS="${ecos_CXXFLAGS} -MDd -Zi"
        fi
    else
        dnl For now building with g++ implies -O0 rather than -O2. The
        dnl compile-time performance of g++ at -O2 has been disappointing
        dnl for quite some time, and the eCos host-side code is not
        dnl sufficiently cpu-intensive to require -O2.
        if test "${GCC}" = "yes" ; then
            ecos_CFLAGS="${ecos_CFLAGS} -O0"
            ecos_CXXFLAGS="${ecos_CXXFLAGS} -O0"
        elif test "${MSVC}" = "yes" ; then
            ecos_CFLAGS="${ecos_CFLAGS} -MD -O2"
            ecos_CXXFLAGS="${ecos_CXXFLAGS} -MD -O2"
        fi
    fi

    CFLAGS="${ac_save_CFLAGS}"
    CXXFLAGS="${ac_save_CXXFLAGS}"

    AC_MSG_RESULT(done)
])

dnl --------------------------------------------------------------------
dnl User-settable options for assertions and tracing.
dnl
dnl The settable options are:
dnl   --disable-asserts
dnl   --disable-preconditions
dnl   --disable-postconditions
dnl   --disable-invariants
dnl   --disable-loopinvariants
dnl   --disable-tracing
dnl   --disable-fntracing

AC_DEFUN(ECOS_ARG_INFRASTRUCTURE, [

    AC_REQUIRE([ECOS_PROG_STANDARD_COMPILER_FLAGS])

    if test "${ecosflags_enable_debug}" = "yes" ; then
        ecosinfra_asserts="yes"
        ecosinfra_preconditions="yes"
        ecosinfra_postconditions="yes"
        ecosinfra_invariants="yes"
        ecosinfra_loopinvariants="yes"
        ecosinfra_tracing="yes"
        ecosinfra_fntracing="yes"
    else
        ecosinfra_asserts="no"
        ecosinfra_preconditions="no"
        ecosinfra_postconditions="no"
        ecosinfra_invariants="no"
        ecosinfra_loopinvariants="no"
        ecosinfra_tracing="no"
        ecosinfra_fntracing="no"
    fi

    AC_ARG_ENABLE(asserts,[ --disable-asserts        disable all assertions],
        [case "${enableval}" in
            yes) ecosinfra_asserts="yes" ;;
            no)  ecosinfra_asserts="no"  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-asserts option]) ;;
        esac])
    if test "${ecosinfra_asserts}" = "yes"; then
        AC_DEFINE(CYGDBG_USE_ASSERTS)
    fi

    AC_ARG_ENABLE(preconditions, [ --disable-preconditions  disable a subset of the assertions],
        [case "${enableval}" in
            yes) ecosinfra_preconditions="yes" ;;
            no)  ecosinfra_preconditions="no"  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-preconditions option]) ;;
        esac])
    if test "${ecosinfra_preconditions}" = "yes"; then
        AC_DEFINE(CYGDBG_INFRA_DEBUG_PRECONDITIONS)
    fi

    AC_ARG_ENABLE(postconditions, [ --disable-postconditions disable a subset of the assertions],
        [case "${enableval}" in
            yes) ecosinfra_postconditions="yes" ;;
            no)  ecosinfra_postconditions="no"  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-postconditions option]) ;;
        esac])
    if test "${ecosinfra_postconditions}" = "yes"; then
        AC_DEFINE(CYGDBG_INFRA_DEBUG_POSTCONDITIONS)
    fi

    AC_ARG_ENABLE(invariants, [ --disable-invariants     disable a subset of the assertions],
        [case "${enableval}" in
            yes) ecosinfra_invariants="yes" ;;
            no)  ecosinfra_invariants="no"  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-invariants option]) ;;
        esac])
    if test "${ecosinfra_invariants}" = "yes"; then
        AC_DEFINE(CYGDBG_INFRA_DEBUG_INVARIANTS)
    fi

    AC_ARG_ENABLE(loopinvariants, [ --disable-loopinvariants disable a subset of the assertions],
        [case "${enableval}" in
            yes) ecosinfra_loopinvariants="yes" ;;
            no)  ecosinfra_loopinvariants="no"  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-loopinvariants option]) ;;
        esac])
    if test "${ecosinfra_loopinvariants}" = "yes"; then
        AC_DEFINE(CYGDBG_INFRA_DEBUG_LOOP_INVARIANTS)
    fi

    AC_ARG_ENABLE(tracing,[ --disable-tracing        disable tracing],
        [case "${enableval}" in
            yes) ecosinfra_tracing="yes" ;;
            no)  ecosinfra_tracing="no"  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-tracing option]) ;;
        esac])
    if test "${ecosinfra_tracing}" = "yes"; then
        AC_DEFINE(CYGDBG_USE_TRACING)
    fi

    AC_ARG_ENABLE(fntracing,[ --disable-fntracing      disable function entry/exit tracing],
        [case "${enableval}" in
            yes) ecosinfra_fntracing="yes" ;;
            no)  ecosinfra_fntracing=no  ;;
            *)   AC_MSG_ERROR([bad value ${enableval} for disable-fntracing option]) ;;
    esac])
    if test "${ecosinfra_fntracing}" = "yes"; then
        AC_DEFINE(CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS)
    fi
])

dnl ====================================================================
dnl Inspired by KDE's autoconfig
dnl This macro takes three Arguments like this:
dnl AC_FIND_FILE(foo.h, $incdirs, incdir)
dnl the filename to look for, the list of paths to check and
dnl the variable with the result.

AC_DEFUN(AC_FIND_FILE,[
    $3=""
    for i in $2; do
        if test -r "$i/$1"; then
            $3=$i
            break
        fi
    done
])

dnl ====================================================================
dnl Variation of the above.
dnl This macro takes three Arguments like this:
dnl AC_FIND_DIR(infra, $incdirs, incdir)
dnl the directory name to look for, the list of paths to check and
dnl the variable with the result.

AC_DEFUN(AC_FIND_DIR,[
    $3=""
    for i in $2; do
        if test -d "$i/$1"; then
            $3=$i
            break
        fi
    done
])

dnl ====================================================================
dnl Work out details of the Tcl/tk installation that should be used.
dnl In theory this is simple: when Tcl is installed in <tcl_prefix>
dnl (usually /usr) there should be a file <tcl_prefix>/lib/tclConfig.sh
dnl which defines exactly how to build Tcl-based applications. Of course
dnl Tcl may be installed anywhere, not just in /usr, so it is necessary
dnl to do some searching. There is a command-line argument
dnl --with-tcl=<path> to specify the Tcl installation, so the macro
dnl can search for <with_tcl>/lib/tclConfig.sh, <prefix>/lib/tclConfig.sh
dnl and /usr/lib/tclConfig.sh
dnl
dnl Unfortunately not all systems use this convention. For example,
dnl at the time of writing Debian installs tclConfig.sh in a versioned
dnl subdirectory /usr/lib/tcl8.3/tclConfig.sh. Hence there is an
dnl additional argument --with-tcl-version=<vsn> which is used to
dnl extend the search path.
dnl
dnl For VC++ builds the situation is different again. Tcl may be
dnl installed anywhere, so the data in tclConfig.sh is not useful
dnl (and that file may not be provided at all). Instead --with-tcl
dnl must be used to specify the path. Alternatively separate paths
dnl for headers and libraries can be specified using --with-tcl-header
dnl and --with-tcl-lib. Usually it will also be necessary to specify
dnl the library version number using --with-tcl-version.

dnl This adds two main command-line options, --with-tcl=<prefix> to
dnl specify the Tcl install directory, and --with-tcl-version=<vsn>
dnl to control which version of Tcl should be used. For finer-grained
dnl control there are additional options --with-tcl-header and
dnl --with-tcl-version. It is assumed that Tcl and Tk are installed
dnl in the same place.
dnl
dnl On Unix systems and under cygwin there should be a file
dnl $(tcl_prefix)/lib/tclConfig.sh containing all the information
dnl needed for Tcl. This file is consulted and the appropriate
dnl variables extracted. Similar information for Tk lives in
dnl tkConfig.sh. As a useful side effect all variables defined
dnl in those scripts can be accessed.
dnl
dnl To confuse matters, subtly different naming conventions are used
dnl under Unix and NT. Under Unix the Tcl library will be called
dnl libtcl8.0.a, libtcl8.1.a, etc. with a dot between the major and
dnl minor version. Under NT (including cygwin) the library will be
dnl called tcl80.lib, tcl81.lib, libtcl80.a, libtcl81.a, etc.
dnl without a dot.
dnl
dnl Currently this macro assumes that Tcl is preinstalled, and not
dnl built alongside eCos. Specifically the macro checks that
dnl tcl.h  can be found, plus on Unix systems tclConfig.sh and
dnl tkConfig.sh as well.
dnl
dnl This macro updates the build-related variables ecos_INCLUDES,
dnl ecos_LDADD, and ecos_LIBS. The latter assumes the application
dnl only needs Tcl. If Tk is needed as well then the variable
dnl ecos_tklibs should be used in addition.

AC_DEFUN(ECOS_PATH_TCL, [

    AC_REQUIRE([ECOS_PROG_MSVC])
    AC_REQUIRE([AC_CYGWIN])

    ecos_tk_libs=""
    ecos_tk_libdir=""

    dnl Where is the Tcl installation, and what version should be used?
    AC_MSG_CHECKING(for Tcl installation)
    AC_ARG_WITH(tcl,[ --with-tcl=<path>        location of Tcl header and libraries])
    AC_ARG_WITH(tcl-version,[ --with-tcl-version=<vsn> version of Tcl to be used])

    dnl If using VC++ then there are no sensible default directories
    dnl to search for a Tcl installation. Instead the user must
    dnl supply either --with-tcl, or both --with-tcl-header and
    dnl --with-tcl-lib.
    dnl
    dnl Also when using VC++ there is no tclConfig.sh file to
    dnl consult about which libraries are needed. Instead that
    dnl information is hard-wired here.
    if test "${MSVC}" = "yes" ; then
        AC_ARG_WITH(tcl-header,[ --with-tcl-header=<path> location of Tcl header])
        AC_ARG_WITH(tcl-lib,[ --with-tcl-lib=<path>    location of Tcl libraries])
        ecos_tcl_incdir=""
	ecos_tcl_libdir=""
        if test "${with_tcl_version+set}" != set ; then
            AC_MSG_ERROR(You must specify a Tcl version using --with-tcl-version=<vsn>)
        fi
        if test "${with_tcl_header+set}" = set ; then
            ecos_tcl_incdir=${with_tcl_header}
        elif test "${with_tcl+set}" = set ; then
            ecos_tcl_incdir="${with_tcl}/include"
        else
            AC_MSG_ERROR(You must specify a Tcl installation with either --with-tcl=<path> or --with-tcl-header=<path>)
        fi
        if test "${with_tcl_lib+set}" = set; then
            ecos_tcl_libdir=${with_tcl_lib}
        elif test "${with_tcl+set}" = set; then
            ecos_tcl_libdir="${with_tcl}/lib"
        else
            AC_MSG_ERROR(You must specify a Tcl installation with either --with-tcl=<path> or --with-tcl-lib=<path>)
        fi
  
        dnl Sanity check, make sure that there is a tcl.h header file.
        dnl If not then there is no point in proceeding.
        if test \! -r "${ecos_tcl_incdir}/tcl.h" ; then
            AC_MSG_ERROR([unable to locate Tcl header file tcl.h])
        fi

        ECOS_MSVC_PATH(ecos_tcl_incdir)
        ECOS_MSVC_PATH(ecos_tcl_libdir)
        ecos_INCLUDES="${ecos_INCLUDES} \"-I${ecos_tcl_incdir}\""
        ecos_LIBS="${ecos_LIBS} tcl${with_tcl_version}.lib"
        ecos_LDADD="${ecos_LDADD} \"-libpath=${ecos_tcl_libdir}\""

        dnl FIXME: what libraries are needed for a tk application under VC++?
        dnl        and can the version be determined more accurately?
        ecos_tk_libs=""

    else
	dnl Try to find tclConfig.sh
	possibles=""
	if test "${with_tcl+set}" = set ; then
	    possibles="${with_tcl}/lib"
            if test "${with_tcl_version+set}" = set ; then
		possibles="${possibles} ${with_tcl}/lib/tcl${with_tcl_version}"
	    fi
        fi
	possibles="${possibles} ${prefix}/lib"
	if test "${with_tcl_version+set}" = set ; then
	    possibles="${possibles} ${prefix}/lib/tcl${with_tcl_version}"
	fi
	possibles="${possibles} /usr/lib"
	if test "${with_tcl_version+set}" = set ; then
	    possibles="${possibles} /usr/lib/tcl${with_tcl_version}"
	fi
	AC_FIND_FILE("tclConfig.sh", ${possibles}, tclconfig)
	if test \! -r "${tclconfig}/tclConfig.sh" ; then
	    AC_MSG_ERROR(unable to locate Tcl configuration file tclConfig.sh)
	else
	    . ${tclconfig}/tclConfig.sh

	    dnl Now we need to figure out where to find the Tcl header files.
	    dnl tclConfig.sh may define a variable TCL_INC_DIR, otherwise
	    dnl use TCL_PREFIX/include
	    if test -z "${TCL_INC_DIR}" ; then
		ecos_tcl_incdir="${TCL_PREFIX}/include"
	    else
		ecos_tcl_incdir="${TCL_INC_DIR}"
	    fi
            if test \! -r "${ecos_tcl_incdir}/tcl.h" ; then
	        AC_MSG_ERROR(unable to locate Tcl header file tcl.h)
	    else
		dnl On Unix systems -I/usr/include is unnecessary, and can
        	dnl cause problems on hosts where gcc is not the platform's
		dnl default compiler because of the use of unfixed headers.
		dnl Hence it is explicitly removed here.
		if test "${ecos_tcl_incdir}" != "/usr/include" ; then
		    ecos_INCLUDES="${ecos_INCLUDES} -I${ecos_tcl_incdir}"
		fi
	    fi

	    dnl There should be a variable TCL_LIB_SPEC which defines
	    dnl exactly how to link with Tcl. Unfortunately this is not
	    dnl 100% guaranteed, so a backup solution is still needed.
	    dnl NOTE: there is also TCL_LIBS defining additional libraries
	    dnl such as -ldl. That may have to be added to ecos_LIBS.
	    if test -z "${TCL_LIB_SPEC}" -a "${with_tcl_version+set}" = set ; then
		AC_FIND_FILE("libtcl${with_tcl_version}.a", ${possibles}, libtcl)
		if test -r "${libtcl}/libtcl${with_tcl_version}.a" ; then
		    TCL_LIB_SPEC="-L${libtcl} -ltcl${with_tcl_version}"
		fi
	    fi
	    if test -z "${TCL_LIB_SPEC}" ; then
		AC_FIND_FILE("libtcl.a", ${possibles}, libtcl)
		if test -r "${libtcl}/libtcl.a" ; then
		    TCL_LIB_SPEC="-L${libtcl} -ltcl"
		fi
	    fi
	    if test -z "${TCL_LIB_SPEC}" ; then
		AC_MSG_ERROR(${tclconfig}/tclConfig.sh does not define TCL_LIB_SPEC, and unable to find libtcl.a)
	    fi
	    ecos_LIBS="${ecos_LIBS} ${TCL_LIB_SPEC}"

	    dnl Next, look for tkConfig.sh
	    possibles=`echo ${possibles} | sed -e 's,tcl,tk,g'`
	    AC_FIND_FILE("tkConfig.sh", ${possibles}, tkconfig)
	    if test \! -r "${tkconfig}/tkConfig.sh" ; then
		AC_MSG_ERROR(unable to locate Tk config file tkConfig.sh)
	    else
		. ${tkconfig}/tkConfig.sh
		if test -z "${TK_INC_DIR}" ; then
		    if test "${TK_PREFIX}" = "/usr" ; then
			ecos_tk_includes="${TK_XINCLUDES}"
		    else
			ecos_tk_includes="-I${TK_PREFIX}/include ${TK_XINCLUDES}"
		    fi
		else
		    ecos_tk_includes="-I${TK_INC_DIR} ${TK_XINCLUDES}"
		fi

		dnl As with TCL_LIB_SPEC, TK_LIB_SPEC may be empty
		if test -z "${TK_LIB_SPEC}" -a "${with_tcl_version+set}" = set ; then
		    AC_FIND_FILE("libtk${with_tcl_version}.a", ${possibles}, libtk)
		    if test -r "${libtk}/libtk${with_tcl_version}.a" ; then
			TK_LIB_SPEC="-L${libtk} -ltk${with_tcl_version}"
		    fi
		fi
		if test -z "${TK_LIB_SPEC}" ; then
		    AC_FIND_FILE("libtk.a", ${possibles}, libtk)
		    if test -r "${libtk}/libtk.a" ; then
			TK_LIB_SPEC="-L${libtk} -ltk"
		    fi
		fi
		if test -z "${TK_LIB_SPEC}" ; then
		    AC_MSG_ERROR(${tkconfig}/tkConfig.sh does not define TK_LIB_SPEC, and unable to find libtk.a)
		fi
		ecos_tk_libs="${TK_LIB_SPEC} ${TK_LIBS}"
	    fi
	fi
    fi

    AC_MSG_RESULT([-I${ecos_tcl_incdir} ${TCL_LIB_SPEC}])
    AC_SUBST(ecos_tk_includes)
    AC_SUBST(ecos_tk_libs)
])

dnl ====================================================================
dnl Search for the infrastructure headers. Usually these can be picked
dnl up from host/infra in the build tree. This macro updates
dnl ecos_INCLUDES, ecos_LDADD and ecos_LIBS appropriately. In addition
dnl it defines new variables ecos_infra_incdir and ecos_infra_libdir,
dnl useful for listing explicit dependencies.
dnl
dnl This macro should only be used in configure scripts that run after
dnl the infrastructure has been configured because it relies on the
dnl infra directory already having been created in the build tree.

AC_DEFUN(ECOS_PATH_INFRA, [
    AC_MSG_CHECKING([for eCos host-side infrastructure])

    dnl Where are we in the build tree? First assume that we are in the host
    dnl tree, thus allowing configury of just the host-side. If that fails
    dnl assume we can be anywhere.
    infra_builddir=""
    possibles=".. ../.. ../../.. ../../../.. ../../../../.."
    AC_FIND_DIR("infra", ${possibles}, infra_builddir)
    if test "${infra_builddir}" = "" ; then
      possibles="../host ../../host ../../../host ../../../../host ../../../../../host ../../../../../../host"
      AC_FIND_DIR("infra", ${possibles}, infra_builddir)
    fi
    if test "${infra_builddir}" != "" ; then
        infra_builddir="${infra_builddir}/infra"
        infra_builddir=`cd ${infra_builddir} && /bin/pwd`
    fi

    ecos_infra_incdir=""
    ecos_infra_libdir=""
    ecos_infra_libs=""

    AC_ARG_WITH(infra-header,[ --with-infra-header=<path> location of eCos infrastructure headers])
    AC_ARG_WITH(infra-lib,[ --with-infra-lib=<path>    location of eCos infrastructure library])
    AC_ARG_WITH(infra,[ --with-infra=<path>        location of eCos infrastructure installation])

    if test "${with_infra_header+set}" = "set"; then
        ecos_infra_incdir="${with_infra_header}"
    elif test "${with_infra+set}" = "set"; then
        ecos_infra_incdir="${with_infra}/include"
    elif test "${infra_builddir}" != "" ; then
        ecos_infra_incdir="${infra_builddir}"
    else
        AC_MSG_ERROR([infrastructure headers not found])
    fi
    if test "${MSVC}" = "yes" ; then
        ecos_msvc_infra_incdir=${ecos_infra_incdir}
        ECOS_MSVC_PATH(ecos_msvc_infra_incdir)
        ecos_INCLUDES="${ecos_INCLUDES} \"-I${ecos_msvc_infra_incdir}\""
    else
        ecos_INCLUDES="${ecos_INCLUDES} -I${ecos_infra_incdir}"
    fi

    if test "${with_infra_lib+set}" = "set"; then
        ecos_infra_libdir="${with_infra_lib}"
    elif test "${with_infra+set}" = "set"; then
        ecos_infra_libdir="${with_infra}/lib"
    elif test "${infra_builddir}" != "" ; then
        ecos_infra_libdir="${infra_builddir}"
    else
        AC_MSG_ERROR([infrastructure library not found])
    fi
    if test "${MSVC}" = "yes" ; then
        ecos_msvc_infra_libdir=${ecos_infra_libdir}
        ECOS_MSVC_PATH(ecos_msvc_infra_libdir)
        ecos_LIBS="${ecos_LIBS} cyginfra.lib"
        ecos_LDADD="${ecos_LDADD} \"-libpath=${ecos_msvc_infra_libdir}\""
    else
        ecos_LIBS="${ecos_LIBS} -lcyginfra"
        ecos_LDADD="${ecos_LDADD} -L${ecos_infra_libdir}"
    fi

    AC_SUBST(ecos_infra_incdir)
    AC_SUBST(ecos_infra_libdir)
    AC_MSG_RESULT(-I[${ecos_infra_incdir} -L${ecos_infra_libdir}])
])

dnl ====================================================================
dnl And a very similar macro for libcdl, but note that the headers
dnl are in the source tree rather than the build tree.

AC_DEFUN(ECOS_PATH_LIBCDL, [
    AC_MSG_CHECKING([for libcdl])

    dnl Where are we in the source tree?
    libcdl_srcdir=""
    possibles="${srcdir}/.. ${srcdir}/../.. ${srcdir}/../../.. ${srcdir}/../../../.. ${srcdir}/../../../../.."
    AC_FIND_DIR("libcdl", ${possibles}, libcdl_srcdir)
    if test "${libcdl_srcdir}" = "" ; then
      possibles="${srcdir}/../host ${srcdir}/../../host ${srcdir}/../../../host ${srcdir}/../../../../host ${srcdir}/../../../../../host ${srcdir}/../../../../../../host"
      AC_FIND_DIR("libcdl", ${possibles}, libcdl_srcdir)
    fi
    if test "${libcdl_srcdir}" != "" ; then
        libcdl_srcdir="${libcdl_srcdir}/libcdl"
        libcdl_srcdir=`cd ${libcdl_srcdir} && /bin/pwd`
    fi

    dnl And where are we in the build tree?
    libcdl_builddir=""
    possibles=".. ../.. ../../.. ../../../.. ../../../../.."
    AC_FIND_DIR("libcdl", ${possibles}, libcdl_builddir)
    if test "${libcdl_builddir}" = "" ; then
      possibles="../host ../../host ../../../host ../../../../host ../../../../../host ../../../../../../host"
      AC_FIND_DIR("libcdl", ${possibles}, libcdl_builddir)
    fi
    if test "${libcdl_builddir}" != "" ; then
        libcdl_builddir="${libcdl_builddir}/libcdl"
        libcdl_builddir=`cd ${libcdl_builddir} && /bin/pwd`
    fi

    ecos_libcdl_incdir=""
    ecos_libcdl_libdir=""
    ecos_libcdl_libs=""

    AC_ARG_WITH(libcdl-header,[ --with-libcdl-header=<path> location of eCos libcdl headers])
    AC_ARG_WITH(libcdl-lib,[ --with-libcdl-lib=<path>    location of eCos libcdl library])
    AC_ARG_WITH(libcdl,[ --with-libcdl=<path>        location of eCos libcdl installation])

    if test "${with_libcdl_header+set}" = "set"; then
        ecos_libcdl_incdir="${with_libcdl_header}"
    elif test "${with_libcdl+set}" = "set"; then
        ecos_libcdl_incdir="${with_libcdl}/include"
    elif test "${libcdl_srcdir}" != "" ; then
        ecos_libcdl_incdir="${libcdl_srcdir}"
    fi
    if test \! -r "${ecos_libcdl_incdir}/cdl.hxx" ; then
        AC_MSG_ERROR([libcdl headers not found])
    fi
    if test "${MSVC}" = "yes" ; then
        ecos_msvc_libcdl_incdir="${ecos_libcdl_incdir}"
        ECOS_MSVC_PATH(ecos_msvc_libcdl_incdir)
        ecos_INCLUDES="${ecos_INCLUDES} \"-I${ecos_msvc_libcdl_incdir}\""
    else
        ecos_INCLUDES="${ecos_INCLUDES} -I${ecos_libcdl_incdir}"
    fi

    if test "${with_libcdl_lib+set}" = "set"; then
        ecos_libcdl_libdir="${with_libcdl_lib}"
    elif test "${with_libcdl+set}" = "set"; then
        ecos_libcdl_libdir="${with_libcdl}/lib"
    elif test "${libcdl_builddir}" != "" ; then
        ecos_libcdl_libdir="${libcdl_builddir}"
    else
        AC_MSG_ERROR([libcdl library not found])
    fi
    if test "${MSVC}" = "yes" ; then
        ecos_msvc_libcdl_libdir=${ecos_libcdl_libdir}
        ECOS_MSVC_PATH(ecos_msvc_libcdl_libdir)
        ecos_LIBS="${ecos_LIBS} cdl.lib"
        ecos_LDADD="${ecos_LDADD} \"-libpath=${ecos_msvc_libcdl_libdir}\""
    else
        ecos_LIBS="${ecos_LIBS} -lcdl"
        ecos_LDADD="${ecos_LDADD} -L${ecos_libcdl_libdir}"
    fi

    AC_SUBST(ecos_libcdl_incdir)
    AC_SUBST(ecos_libcdl_libdir)
    AC_MSG_RESULT([-I${ecos_libcdl_incdir} -L${ecos_libcdl_libdir}])
])

dnl ====================================================================
dnl Look for a 64 bit data type. It is necessary to check both C and C++
dnl compilers.
dnl
dnl A better implementation would check whether or not AC_PROG_CC and
dnl AC_PROG_CXX have been invoked and only test the appropriate
dnl compiler.
dnl
dnl When cross-compiling, default to long long on the assumption that
dnl gcc/g++ must be used and long long is likely to be the 64 bit data
dnl type. This is not guaranteed, but sufficiently likely to meet
dnl the requirements for the time being. The CHECK_SIZEOF() macro
dnl might be another way to get the desired information.

AC_DEFUN(ECOS_TYPE_64bit, [
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])

    AC_CACHE_CHECK("for a 64 bit data type",ecos_cv_type_64bit,[
        for type in "long" "long long" "__int64"; do
            AC_LANG_SAVE
            AC_LANG_C
            AC_TRY_RUN([
                main() {
                    return 8 != sizeof($type);
                }
            ],ctype_64bit=$type,ctype_64bit="unknown",ctype_64bit="long long")
            AC_LANG_CPLUSPLUS
            AC_TRY_RUN([
                int main(int argc, char ** argv) {
                    return 8 != sizeof($type);
                }
            ],cxxtype_64bit=$type,cxxtype_64bit="unknown",cxxtype_64bit="long long")
            AC_LANG_RESTORE
            if test "${ctype_64bit}" = "${type}" -a "${cxxtype_64bit}" = "${type}"; then
                ecos_cv_type_64bit="${type}"
                break
            fi
        done
    ])
    if test "${ecos_cv_type_64bit}" = ""; then
        AC_MSG_ERROR(Unable to figure out how to do 64 bit arithmetic)
    else
        if test "${ecos_cv_type_64bit}" != "long long"; then
            AC_DEFINE_UNQUOTED(cyg_halint64,${ecos_cv_type_64bit})
            AC_DEFINE_UNQUOTED(cyg_halcount64,${ecos_cv_type_64bit})
        fi
    fi
])

dnl ====================================================================
dnl Check that both the C and C++ compilers support __PRETTY_FUNCTION__

AC_DEFUN(ECOS_C_PRETTY_FUNCTION,[
    AC_REQUIRE([AC_PROG_CC])
    AC_REQUIRE([AC_PROG_CXX])

    AC_CACHE_CHECK("for __PRETTY_FUNCTION__ support",ecos_cv_c_pretty_function,[
        AC_LANG_SAVE
        AC_LANG_C
        AC_TRY_LINK(
            [#include <stdio.h>],
            [puts(__PRETTY_FUNCTION__);],
            c_ok="yes",
            c_ok="no"
        )
        AC_LANG_CPLUSPLUS
        AC_TRY_LINK(
            [#include <cstdio>],
            [puts(__PRETTY_FUNCTION__);],
            cxx_ok="yes",
            cxx_ok="no"
        )
        AC_LANG_RESTORE
        if test "${c_ok}" = "yes" -a "${cxx_ok}" = "yes"; then
            ecos_cv_c_pretty_function="yes"
        fi
    ])
    if test "${ecos_cv_c_pretty_function}" = "yes"; then
        AC_DEFINE(CYGDBG_INFRA_DEBUG_FUNCTION_PSEUDOMACRO)
    fi
])

dnl ====================================================================
dnl During installation eCos package-specific host-side code should be
dnl versioned in the same way as the packages themselves, allowing
dnl different versions to coexist in one installation. This is analogous
dnl to having multiple versions of a shared library installed so that
dnl applications can load whichever one they were linked with.
dnl
dnl To support all this the host-side code needs access to a number
dnl of directory names:
dnl     ECOS_REPOSITORY    e.g. ~/ecc/ecc
dnl     PACKAGE_DIR        e.g. hal/synth/arch
dnl     PACKAGE_VERSION    e.g. current
dnl     PACKAGE_INSTALL    e.g. hal/synth/arch/current
dnl
dnl These, together with the standard variable libexecdir, allow
dnl the host-side code to navigate around both source and install
dnl trees.

AC_DEFUN(ECOS_PACKAGE_DIRS,[

    dnl srcdir will be something like <path>/packages/<package_path>/<version>/host
    package_dir=`cd ${srcdir} && /bin/pwd`
    PACKAGE_VERSION=`dirname ${package_dir}`
    PACKAGE_VERSION=`basename ${PACKAGE_VERSION}`

    dnl Now look for an "acsupport" directory as a good way of identifying
    dnl the root of the repository. Assume that this does not clash with
    dnl any real packages in the repository. Also assume that no silly games
    dnl are being played with symlinks.
    package_dir=`dirname ${package_dir}`
    package_dir=`dirname ${package_dir}`

    possibles="${package_dir}/.. ${package_dir}/../.. ${package_dir}/../../.. ${package_dir}/../../../.."
    possibles="${possibles} ${package_dir}/../../../../.. ${package_dir}/../../../../../.." 
    AC_FIND_DIR("acsupport", ${possibles}, repository_root)
    if test "${repository_root}" = "" ; then
        AC_MSG_ERROR([Failed to identify this package's position within the eCos repository])
    fi
    dnl repository_root will still contain the ..'s, instead of an absolute path
    ECOS_REPOSITORY=`cd "${repository_root}/packages/pkgconf/.." && /bin/pwd`

    dnl Now we have two absolute paths, so just remove one from the other
    PACKAGE_DIR=`echo ${package_dir} | sed -e "s:${ECOS_REPOSITORY}/::"`

    dnl To avoid creating too many subdirectories on the host-side, turn
    dnl e.g. hal/synth/arch into hal_synth_arch. Theoretically this could
    dnl go wrong because multiple packages could map onto the same string,
    dnl but in practice there should be a net reduction in complexity.
    dnl bartv: / to _ conversion disabled for now, 5 March 2002
    PACKAGE_INSTALL="${PACKAGE_DIR}/${PACKAGE_VERSION}"
    dnl PACKAGE_INSTALL=`echo ${PACKAGE_INSTALL} | sed -e "s:/:_:g"`

    AC_SUBST(ECOS_REPOSITORY)
    AC_SUBST(PACKAGE_DIR)
    AC_SUBST(PACKAGE_VERSION)
    AC_SUBST(PACKAGE_INSTALL)
])
