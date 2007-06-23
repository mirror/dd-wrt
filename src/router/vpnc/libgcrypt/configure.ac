# Configure.ac script for Libgcrypt
# Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003
#               2004, 2006  Free Software Foundation, Inc.
#
# This file is part of Libgcrypt.
#
# Libgcrypt is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser general Public License as
# published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# Libgcrypt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
# USA.
#
# (Process this file with autoconf to produce a configure script.)
AC_REVISION($Revision: 1206 $)
AC_PREREQ(2.59)
min_automake_version="1.9.3"

# Remember to change the version number immediately *after* a release.
# Set my_issvn to "yes" for non-released code.  Remember to run an
# "svn up" and "autogen.sh" right before creating a distribution.
m4_define([my_version], [1.2.4])
m4_define([my_issvn], [no])

m4_define([svn_revision], m4_esyscmd([echo -n $( (svn info 2>/dev/null \
            || echo 'Revision: 0')|sed -n '/^Revision:/ {s/[^0-9]//gp;q;}')]))
AC_INIT([libgcrypt], my_version[]m4_if(my_issvn,[yes],[-svn[]svn_revision]),
        [bug-libgcrypt@gnupg.org])
# LT Version numbers, remember to change them just *before* a release.
# NOTE: API changes must be coordinated with the development branch
#   (Interfaces removed:    CURRENT++, AGE=0, REVISION=0)
#   (Interfaces added:      CURRENT++, AGE++, REVISION=0)
#   (No interfaces changed:                   REVISION++)
LIBGCRYPT_LT_CURRENT=13
LIBGCRYPT_LT_AGE=2
LIBGCRYPT_LT_REVISION=3

# If the API is changed in an incompatible way: increment the next counter.
LIBGCRYPT_CONFIG_API_VERSION=1

NEED_GPG_ERROR_VERSION=0.5


PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION

AC_CONFIG_SRCDIR([src/libgcrypt.vers])
AC_CANONICAL_TARGET()
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AH_TOP([
/* need this, because some autoconf tests rely on this (e.g. stpcpy)
 * and it should be used for new programs  */
#define _GNU_SOURCE  1
])

AH_BOTTOM([
#define _GCRYPT_IN_LIBGCRYPT 1
])

AH_VERBATIM([_REENTRANT],
[/* To allow the use of Libgcrypt in multithreaded programs we have to use
    special features from the library. */
#ifndef _REENTRANT
# define _REENTRANT 1
#endif
])

AC_SUBST(LIBGCRYPT_LT_CURRENT)
AC_SUBST(LIBGCRYPT_LT_AGE)
AC_SUBST(LIBGCRYPT_LT_REVISION)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of this package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version of this package])


AC_PROG_MAKE_SET
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
# AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_PROG_CC
AC_PROG_CPP
AM_PROG_AS
AC_ISC_POSIX
AC_PROG_INSTALL
AC_PROG_AWK

AM_PROG_LIBTOOL

##########################
## General definitions. ##
##########################

# Used by libgcrypt-config
LIBGCRYPT_CONFIG_LIBS="-lgcrypt"
LIBGCRYPT_CONFIG_CFLAGS=""

# Definitions for symmetric ciphers.
available_ciphers="arcfour blowfish cast5 des aes twofish serpent rfc2268"
enabled_ciphers=""

# Definitions for public-key ciphers.
available_pubkey_ciphers="dsa elgamal rsa"
enabled_pubkey_ciphers=""

# Definitions for message digests.
available_digests="crc md4 md5 rmd160 sha1 sha256"
available_digests_64="sha512 tiger"
enabled_digests=""

# Definitions for random modules.
available_random_modules="linux egd unix"
auto_random_modules="$available_random_modules"

# Supported thread backends.
LIBGCRYPT_THREAD_MODULES=""

# Other definitions.
print_egd_notice=no

# Setup some stuff depending on host/target.
case "${target}" in
    *-*-mingw32*)
      available_random_modules="w32"
      ac_cv_have_dev_random=no
      AC_DEFINE(USE_ONLY_8DOT3,1,
                [set this to limit filenames to the 8.3 format])
      AC_DEFINE(HAVE_DRIVE_LETTERS,1,
                [defined if we must run on a stupid file system])
      AC_DEFINE(HAVE_DOSISH_SYSTEM,1,
                [defined if we run on some of the PCDOS like systems 
                 (DOS, Windoze. OS/2) with special properties like
                  no file modes])
      ;;

    i?86-emx-os2 | i?86-*-os2*emx)
        # OS/2 with the EMX environment
        ac_cv_have_dev_random=no
        AC_DEFINE(HAVE_DRIVE_LETTERS)
        AC_DEFINE(HAVE_DOSISH_SYSTEM)
        ;;

    i?86-*-msdosdjgpp*)
        # DOS with the DJGPP environment
        ac_cv_have_dev_random=no
        AC_DEFINE(HAVE_DRIVE_LETTERS)
        AC_DEFINE(HAVE_DOSISH_SYSTEM)
        ;;

    *-*-freebsd*)
       # FreeBSD
       CPPFLAGS="$CPPFLAGS -I/usr/local/include"
       LDFLAGS="$LDFLAGS -L/usr/local/lib"
       ;;

    *-*-hpux*)
        if test -z "$GCC" ; then
            CFLAGS="$CFLAGS -Ae -D_HPUX_SOURCE"
        fi
        ;;
    *-dec-osf4*)
        if test -z "$GCC" ; then
            # Suppress all warnings
            # to get rid of the unsigned/signed char mismatch warnings.
            CFLAGS="$CFLAGS -w"
        fi
        ;;
    m68k-atari-mint)
        ;;
    *)
      ;;
esac

# A printable OS Name is sometime useful.
case "${target}" in
    *-*-mingw32*)
        PRINTABLE_OS_NAME="MingW32"
        ;;

    i?86-emx-os2 | i?86-*-os2*emx )
        PRINTABLE_OS_NAME="OS/2"
        ;;

    i?86-*-msdosdjgpp*)
        PRINTABLE_OS_NAME="MSDOS/DJGPP"
        ;;

    *-linux*)
        PRINTABLE_OS_NAME="GNU/Linux"
        ;;

    *)
        PRINTABLE_OS_NAME=`uname -s || echo "Unknown"`
        ;;
esac


case "${target}" in
    *-openbsd*)
        # FIXME: Are these the best flags for OpenBSD?
        NAME_OF_DEV_RANDOM="/dev/srandom"
        NAME_OF_DEV_URANDOM="/dev/urandom"
#       DYNLINK_MOD_CFLAGS="-shared -rdynamic $CFLAGS_PIC -Wl,-Bshareable -Wl,-x"
        ;;

    *-solaris* | *-irix* | *-dec-osf* | *-netbsd* )
        NAME_OF_DEV_RANDOM="/dev/random"
        NAME_OF_DEV_URANDOM="/dev/random"
#       DYNLINK_MOD_CFLAGS="-shared $CFLAGS_PIC"
        ;;

    *)
        NAME_OF_DEV_RANDOM="/dev/random"
        NAME_OF_DEV_URANDOM="/dev/urandom"
        # -shared is a gcc-ism.  Find pic flags from GNUPG_CHECK_PIC.
#        if test -n "$GCC" ; then
#           DYNLINK_MOD_CFLAGS="-shared $CFLAGS_PIC"                            
#        else
#           DYNLINK_MOD_CFLAGS="$CFLAGS_PIC"
#        fi
        ;;
esac


AC_C_BIGENDIAN

AC_CHECK_SIZEOF(unsigned short, 2)
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned long, 4)
AC_CHECK_SIZEOF(unsigned long long, 0)

if test "$ac_cv_sizeof_unsigned_short" = "0" \
   || test "$ac_cv_sizeof_unsigned_int" = "0" \
   || test "$ac_cv_sizeof_unsigned_long" = "0"; then
    AC_MSG_WARN([Hmmm, something is wrong with the sizes - using defaults]);
fi

# Do we have any 64-bit data types?
if test "$ac_cv_sizeof_unsigned_int" != "8" \
   && test "$ac_cv_sizeof_unsigned_long" != "8" \
   && test "$ac_cv_sizeof_unsigned_long_long" != "8" \
   && test "$ac_cv_sizeof_uint64_t" != "8"; then
    AC_MSG_WARN([No 64-bit types.  Disabling TIGER/192, SHA-384, and SHA-512])
else
  available_digests="$available_digests $available_digests_64"
fi

# If not specified otherwise, all available algorithms will be
# included.
default_ciphers="$available_ciphers"
default_pubkey_ciphers="$available_pubkey_ciphers"
default_digests="$available_digests"

############################
## Command line switches. ##
############################

# Implementation of the --enable-ciphers switch.
AC_ARG_ENABLE(ciphers,
	      AC_HELP_STRING([--enable-ciphers=ciphers],
			     [select the symmetric ciphers to include]),
	      [enabled_ciphers=`echo $enableval | tr , ' ' | tr '[A-Z]' '[a-z]'`],
	      [enabled_ciphers=""])
if test "x$enabled_ciphers" = "x" \
   -o "$enabled_ciphers" = "yes"  \
   -o "$enabled_ciphers" = "no"; then
   enabled_ciphers=$default_ciphers
fi
AC_MSG_CHECKING([which symmetric ciphers to include])
for cipher in $enabled_ciphers; do
    LIST_MEMBER($cipher, $available_ciphers)
    if test "$found" = "0"; then
       AC_MSG_ERROR([unsupported cipher specified])
    fi
done
AC_MSG_RESULT([$enabled_ciphers])

# Implementation of the --enable-pubkey-ciphers switch.
AC_ARG_ENABLE(pubkey-ciphers,
	      AC_HELP_STRING([--enable-pubkey-ciphers=ciphers],
			     [select the public-key ciphers to include]),
	      [enabled_pubkey_ciphers=`echo $enableval | tr , ' ' | tr '[A-Z]' '[a-z]'`],
	      [enabled_pubkey_ciphers=""])
if test "x$enabled_pubkey_ciphers" = "x" \
   -o "$enabled_pubkey_ciphers" = "yes"  \
   -o "$enabled_pubkey_ciphers" = "no"; then
   enabled_pubkey_ciphers=$default_pubkey_ciphers
fi
AC_MSG_CHECKING([which public-key ciphers to include])
for cipher in $enabled_pubkey_ciphers; do
    LIST_MEMBER($cipher, $available_pubkey_ciphers)
    if test "$found" = "0"; then
       AC_MSG_ERROR([unsupported public-key cipher specified])
    fi
done
AC_MSG_RESULT([$enabled_pubkey_ciphers])

# Implementation of the --enable-digests switch.
AC_ARG_ENABLE(digests,
	      AC_HELP_STRING([--enable-digests=digests],
			     [select the message digests to include]),
	      [enabled_digests=`echo $enableval | tr , ' ' | tr '[A-Z]' '[a-z]'`],
	      [enabled_digests=""])
if test "x$enabled_digests" = "x" \
   -o "$enabled_digests" = "yes"  \
   -o "$enabled_digests" = "no"; then
   enabled_digests=$default_digests
fi
AC_MSG_CHECKING([which message digests to include])
for digest in $enabled_digests; do
    LIST_MEMBER($digest, $available_digests)
    if test "$found" = "0"; then
       AC_MSG_ERROR([unsupported message digest specified])
    fi
done
AC_MSG_RESULT([$enabled_digests])

# Implementation of the --enable-random switch.
AC_ARG_ENABLE(random,
	      AC_HELP_STRING([--enable-random=name],
	                     [select which random number generator to use]),
	      [random=`echo $enableval | tr '[A-Z]' '[a-z]'`],
	      [])
if test "x$random" = "x" -o "$random" = "yes" -o "$random" = "no"; then
    random=default
fi
AC_MSG_CHECKING([which random module to use])
if test "$random" != "default" -a "$random" != "auto"; then
    LIST_MEMBER($random, $available_random_modules)
    if test "$found" = "0"; then
       AC_MSG_ERROR([unsupported random module specified])
    fi
fi
AC_MSG_RESULT($random)

# Implementation of the --disable-dev-random switch.
AC_MSG_CHECKING([whether use of /dev/random is requested])
AC_ARG_ENABLE(dev-random,
[  --disable-dev-random    disable the use of dev random],
    try_dev_random=$enableval, try_dev_random=yes)
AC_MSG_RESULT($try_dev_random)

# Implementation of the --with-egd-socket switch.
AC_ARG_WITH(egd-socket,
    [  --with-egd-socket=NAME  Use NAME for the EGD socket)],
            egd_socket_name="$withval", egd_socket_name="" )
AC_DEFINE_UNQUOTED(EGD_SOCKET_NAME, "$egd_socket_name",
                   [Define if you don't want the default EGD socket name.
                    For details see cipher/rndegd.c])

# Implementation of --disable-asm.
AC_MSG_CHECKING([whether assembler modules are requested])
AC_ARG_ENABLE([asm],
              AC_HELP_STRING([--disable-asm],
	                     [Disable assembler modules]),
              [try_asm_modules=$enableval],
              [try_asm_modules=yes])
AC_MSG_RESULT($try_asm_modules)

# Implementation of the --enable-m-guard switch.
AC_MSG_CHECKING([whether memory guard is requested])
AC_ARG_ENABLE(m-guard,
    [  --enable-m-guard        enable memory guard facility],
    use_m_guard=$enableval, use_m_guard=no)
    AC_MSG_RESULT($use_m_guard)
if test "$use_m_guard" = yes ; then
    AC_DEFINE(M_GUARD,1,[Define to use the (obsolete) malloc guarding feature])
fi

# Implementation of the --with-capabilities switch.
# Check whether we want to use Linux capabilities
AC_MSG_CHECKING([whether use of capabilities is requested])
AC_ARG_WITH(capabilities,
    [  --with-capabilities     use linux capabilities [default=no]],
[use_capabilities="$withval"],[use_capabilities=no])
AC_MSG_RESULT($use_capabilities)

# We don't have a test to check whether as(1) knows about the
# non executable stack option.  Thus we provide an option to enable it.
AC_MSG_CHECKING([whether non excutable stack support is requested])
AC_ARG_ENABLE(noexecstack,
              AC_HELP_STRING([--enable-noexecstack],
                             [enable non executable stack support (gcc only)]),
              noexecstack_support=$enableval, noexecstack_support=no)
AC_MSG_RESULT($noexecstack_support)



AC_DEFINE_UNQUOTED(PRINTABLE_OS_NAME, "$PRINTABLE_OS_NAME",
                   [A human readable text with the name of the OS])

# For some systems we know that we have ld_version scripts.  
# Use it then as default.
have_ld_version_script=no
case "${host}" in
    *-*-linux*)
	have_ld_version_script=yes
        ;;
    *-*-gnu*)
	have_ld_version_script=yes
        ;;
esac
AC_ARG_ENABLE([ld-version-script],
              AC_HELP_STRING([--enable-ld-version-script],
                             [enable/disable use of linker version script.
                              (default is system dependent)]),
              [have_ld_version_script=$enableval],
              [ : ] )
AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, test "$have_ld_version_script" = "yes")

AC_DEFINE_UNQUOTED(NAME_OF_DEV_RANDOM, "$NAME_OF_DEV_RANDOM",
                   [defined to the name of the strong random device])
AC_DEFINE_UNQUOTED(NAME_OF_DEV_URANDOM, "$NAME_OF_DEV_URANDOM",
                   [defined to the name of the weaker random device])

###############################
#### Checks for libraries. ####
###############################

AM_PATH_GPG_ERROR("$NEED_GPG_ERROR_VERSION")
if test "x$GPG_ERROR_LIBS" = "x"; then
  AC_MSG_ERROR([libgpg-error is needed.
                See ftp://ftp.gnupg.org/gcrypt/libgpg-error/ .])
fi

AC_DEFINE(GPG_ERR_SOURCE_DEFAULT, GPG_ERR_SOURCE_GCRYPT,
          [The default error source for libgcrypt.])

# Solaris needs -lsocket and -lnsl. Unisys system includes
# gethostbyname in libsocket but needs libnsl for socket.
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, socket, ac_need_libsocket=1, ac_try_nsl=1)
if test x$ac_need_libsocket = x1; then
    LIBS="$LIBS -lsocket"
fi
if test x$ac_try_nsl = x1; then
    AC_CHECK_LIB(nsl, gethostbyname, ac_need_libnsl=1)
    if test x$ac_need_libnsl = x1
    then
        LIBS="$LIBS -lnsl"
    fi
fi

##################################
#### Checks for header files. ####
##################################

AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/select.h)

##########################################
#### Checks for typedefs, structures, ####
####  and compiler characteristics.   ####
##########################################

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_DECL_SYS_SIGLIST

GNUPG_CHECK_TYPEDEF(byte, HAVE_BYTE_TYPEDEF)
GNUPG_CHECK_TYPEDEF(ushort, HAVE_USHORT_TYPEDEF)
GNUPG_CHECK_TYPEDEF(ulong, HAVE_ULONG_TYPEDEF)
GNUPG_CHECK_TYPEDEF(u16, HAVE_U16_TYPEDEF)
GNUPG_CHECK_TYPEDEF(u32, HAVE_U32_TYPEDEF)

gl_TYPE_SOCKLEN_T
if test ".$gl_cv_socklen_t_equiv" = "."; then
  FALLBACK_SOCKLEN_T="typedef socklen_t gcry_socklen_t;"
else
  FALLBACK_SOCKLEN_T="typedef ${gl_cv_socklen_t_equiv} gcry_socklen_t;"
fi
AC_SUBST(FALLBACK_SOCKLEN_T)


#######################################
#### Checks for library functions. ####
#######################################

AC_FUNC_VPRINTF
# We have replacements for these in src/missing-string.c
AC_CHECK_FUNCS(stpcpy strcasecmp)
# We have replacements for these in src/g10lib.h
AC_CHECK_FUNCS(strtoul memmove stricmp atexit raise)
# Other checks
AC_CHECK_FUNCS(strerror rand mmap getpagesize waitpid wait4)
AC_CHECK_FUNCS(gettimeofday getrusage gethrtime clock_gettime)

GNUPG_CHECK_MLOCK

# Check whether we can use Linux capabilities as requested.
if test "$use_capabilities" = "yes" ; then
use_capabilities=no
AC_CHECK_HEADERS(sys/capability.h)
if test "$ac_cv_header_sys_capability_h" = "yes" ; then
  AC_CHECK_LIB(cap, cap_init, ac_need_libcap=1)
  if test "$ac_cv_lib_cap_cap_init" = "yes"; then
     AC_DEFINE(USE_CAPABILITIES,1,
               [define if capabilities should be used])
     LIBS="$LIBS -lcap"
     use_capabilities=yes
  fi
fi
if test "$use_capabilities" = "no" ; then
    AC_MSG_WARN([[
***
*** The use of capabilities on this system is not possible.
*** You need a recent Linux kernel and some patches:
***   fcaps-2.2.9-990610.patch      (kernel patch for 2.2.9)
***   fcap-module-990613.tar.gz     (kernel module)
***   libcap-1.92.tar.gz            (user mode library and utilities)
*** And you have to configure the kernel with CONFIG_VFS_CAP_PLUGIN
*** set (filesystems menu). Be warned: This code is *really* ALPHA.
***]])
fi
fi

# Check whether a random device is available.
if test "$try_dev_random" = yes ; then
    AC_CACHE_CHECK(for random device, ac_cv_have_dev_random,
    [if test -r "$NAME_OF_DEV_RANDOM" && test -r "$NAME_OF_DEV_URANDOM" ; then
      ac_cv_have_dev_random=yes; else ac_cv_have_dev_random=no; fi])
    if test "$ac_cv_have_dev_random" = yes; then
        AC_DEFINE(HAVE_DEV_RANDOM,1,
                 [defined if the system supports a random device] )
    fi
else
    AC_MSG_CHECKING(for random device)
    ac_cv_have_dev_random=no
    AC_MSG_RESULT(has been disabled)
fi

# Figure out the random modules for this configuration.
if test "$random" = "default"; then

    # Select default value.
    if test "$ac_cv_have_dev_random" = yes; then
        # Try Linuxish random device.
        random_modules="linux"
    else
        case "${target}" in
        *-*-mingw32*|*-*-cygwin*)
          # Windows random device.
          random_modules="w32"
          ;;
        *)
          # Build everything, allow to select at runtime.
          random_modules="$auto_random_modules"
          ;;
        esac    
    fi
else
    if test "$random" = "auto"; then
        # Build everything, allow to select at runtime.
        random_modules="$auto_random_modules"
    fi
fi


#
# Setup assembler stuff.
#
GNUPG_SYS_SYMBOL_UNDERSCORE()
AC_MSG_CHECKING(for mpi assembler functions)
if test -f $srcdir/mpi/config.links ; then
    . $srcdir/mpi/config.links
    AC_CONFIG_LINKS("$mpi_ln_list")
    ac_cv_mpi_mod_list="$mpi_mod_list"
    ac_cv_mpi_sflags="$mpi_sflags"
    ac_cv_mpi_config_done="yes"
    AC_MSG_RESULT(done)
else
    AC_MSG_RESULT(failed)
    AC_MSG_ERROR([mpi/config.links missing!])
fi
MPI_MOD_LIST_LO=""
MPI_MOD_LIST_O=""
if test "$ac_cv_mpi_mod_list" != ""; then
  for i in $ac_cv_mpi_mod_list; do
      MPI_MOD_LIST_LO="$MPI_MOD_LIST_LO $i.lo"
      MPI_MOD_LIST_O="$MPI_MOD_LIST_O $i.o"
  done
fi
AC_SUBST(MPI_MOD_LIST_LO)
AC_SUBST(MPI_MOD_LIST_O)
MPI_SFLAGS="$ac_cv_mpi_sflags"
AC_SUBST(MPI_SFLAGS)

# Allow users to append something to the version string without
# flagging it as development version.  The user version part is
# considered everything after a dash.
changequote(,)#
tmp_pat='[a-zA-Z]'
changequote([,])#
if echo "$VERSION" | sed 's/-.*//' | grep "$tmp_pat" >/dev/null ; then
    AC_DEFINE(IS_DEVELOPMENT_VERSION,1,
              [Defined if this is not a regular release])
fi

AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)


# This is handy for debugging so the compiler doesn't rearrange
# things and eliminate variables.
AC_ARG_ENABLE(optimization,
       AC_HELP_STRING([--disable-optimization],
		      [disable compiler optimization]),
                      [if test $enableval = no ; then
                         CFLAGS=`echo $CFLAGS | sed 's/-O[[0-9]]//'`
                       fi])

AC_ARG_ENABLE(gcc-warnings,
	      AC_HELP_STRING([--enable-gcc-warnings],
			     [enable more verbose gcc warnings]),
	      [more_gcc_warnings="$enableval"],
	      [more_gcc_warnings="no"])

if test "$GCC" = yes; then
    if test "$USE_MAINTAINER_MODE" = "yes" ||
       test "$more_gcc_warnings" = "yes"; then
        CFLAGS="$CFLAGS -Wall -Wcast-align -Wshadow -Wstrict-prototypes"
        if test "$more_gcc_warnings" = "yes"; then
            CFLAGS="$CFLAGS -W -Wpointer-arith -Wbad-function-cast"
            CFLAGS="$CFLAGS -Wwrite-strings -Wunreachable-code"
            CFLAGS="$CFLAGS -Wno-sign-compare"
        fi
    else
        CFLAGS="$CFLAGS -Wall"
    fi

    # Non exec stack hack.  Fixme: Write a test to check whether as
    # can cope with it and use the enable-noexecstack option only to
    # disable it in case it is required on some platforms.
    if test "$noexecstack_support" = yes; then
        NOEXECSTACK_FLAGS="-Wa,--noexecstack"
    fi
fi
AC_SUBST(NOEXECSTACK_FLAGS)


AC_SUBST(LIBGCRYPT_CONFIG_API_VERSION)
AC_SUBST(LIBGCRYPT_CONFIG_LIBS)
AC_SUBST(LIBGCRYPT_CONFIG_CFLAGS)
AC_SUBST(LIBGCRYPT_THREAD_MODULES)

AC_CONFIG_COMMANDS([gcrypt-conf],[[
chmod +x src/libgcrypt-config
]],[[
prefix=$prefix
exec_prefix=$exec_prefix
libdir=$libdir
datadir=$datadir
DATADIRNAME=$DATADIRNAME
]])

#####################
#### Conclusion. ####
#####################

# Define conditional sources and config.h symbols depending on the
# selected ciphers, pubkey-ciphers, digests and random modules.

LIST_MEMBER(arcfour, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS arcfour.lo"
AC_DEFINE_UNQUOTED(USE_ARCFOUR, $found,
                                 [Defined if this module should be included])

LIST_MEMBER(blowfish, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS blowfish.lo"
AC_DEFINE_UNQUOTED(USE_BLOWFISH, $found,
                                  [Defined if this module should be included])

LIST_MEMBER(cast5, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS cast5.lo"
AC_DEFINE_UNQUOTED(USE_CAST5, $found,
                               [Defined if this module should be included])

LIST_MEMBER(des, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS des.lo"
AC_DEFINE_UNQUOTED(USE_DES, $found,
                             [Defined if this module should be included])

LIST_MEMBER(aes, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS rijndael.lo"
AC_DEFINE_UNQUOTED(USE_AES, $found,
                             [Defined if this module should be included])

LIST_MEMBER(twofish, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS twofish.lo"
AC_DEFINE_UNQUOTED(USE_TWOFISH, $found,
                                 [Defined if this module should be included])

LIST_MEMBER(serpent, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS serpent.lo"
AC_DEFINE_UNQUOTED(USE_SERPENT, $found,
                                 [Defined if this module should be included])

LIST_MEMBER(rfc2268, $enabled_ciphers)
test "$found" = "1" && GCRYPT_CIPHERS="$GCRYPT_CIPHERS rfc2268.lo"
AC_DEFINE_UNQUOTED(USE_RFC2268, $found,
                                 [Defined if this module should be included])

LIST_MEMBER(dsa, $enabled_pubkey_ciphers)
test "$found" = "1" && GCRYPT_PUBKEY_CIPHERS="$GCRYPT_PUBKEY_CIPHERS dsa.lo"
AC_DEFINE_UNQUOTED(USE_DSA, $found,
                             [Defined if this module should be included])

LIST_MEMBER(rsa, $enabled_pubkey_ciphers)
test "$found" = "1" && GCRYPT_PUBKEY_CIPHERS="$GCRYPT_PUBKEY_CIPHERS rsa.lo"
AC_DEFINE_UNQUOTED(USE_RSA, $found,
                            [Defined if this module should be included])

LIST_MEMBER(elgamal, $enabled_pubkey_ciphers)
test "$found" = "1" \ 
     && GCRYPT_PUBKEY_CIPHERS="$GCRYPT_PUBKEY_CIPHERS elgamal.lo"
AC_DEFINE_UNQUOTED(USE_ELGAMAL, $found,
                                [Defined if this module should be included])

LIST_MEMBER(crc, $enabled_digests)
test "$found" = "1" && GCRYPT_DIGESTS="$GCRYPT_DIGESTS crc.lo"
AC_DEFINE_UNQUOTED(USE_CRC, $found,
                            [Defined if this module should be included])

LIST_MEMBER(md4, $enabled_digests)
test "$found" = "1" && GCRYPT_DIGESTS="$GCRYPT_DIGESTS md4.lo"
AC_DEFINE_UNQUOTED(USE_MD4, $found,
                             [Defined if this module should be included])

LIST_MEMBER(md5, $enabled_digests)
test "$found" = "1" && GCRYPT_DIGESTS="$GCRYPT_DIGESTS md5.lo"
AC_DEFINE_UNQUOTED(USE_MD5, $found,
                             [Defined if this module should be included])

LIST_MEMBER(sha256, $enabled_digests)
test "$found" = "1" && GCRYPT_DIGESTS="$GCRYPT_DIGESTS sha256.lo"
AC_DEFINE_UNQUOTED(USE_SHA256, $found,
                                [Defined if this module should be included])

LIST_MEMBER(sha512, $enabled_digests)
test "$found" = "1" && GCRYPT_DIGESTS="$GCRYPT_DIGESTS sha512.lo"
AC_DEFINE_UNQUOTED(USE_SHA512, $found,
                                [Defined if this module should be included])

LIST_MEMBER(tiger, $enabled_digests)
test "$found" = "1" && GCRYPT_DIGESTS="$GCRYPT_DIGESTS tiger.lo"
AC_DEFINE_UNQUOTED(USE_TIGER, $found,
                               [Defined if this module should be included])

# rmd160 and sha1 should be included always.
GCRYPT_DIGESTS="$GCRYPT_DIGESTS rmd160.lo sha1.lo"
AC_DEFINE_UNQUOTED(USE_RMD160, 1, [Defined if this module should be included])
AC_DEFINE_UNQUOTED(USE_SHA1, 1,   [Defined if this module should be included])

LIST_MEMBER(linux, $random_modules)
test "$found" = "1" && GCRYPT_RANDOM="$GCRYPT_RANDOM rndlinux.lo"
AC_DEFINE_UNQUOTED(USE_RNDLINUX, $found,
                   [Defined if the /dev/random based RNG should be used.])


LIST_MEMBER(unix, $random_modules)
test "$found" = "1" && GCRYPT_RANDOM="$GCRYPT_RANDOM rndunix.lo"
AC_DEFINE_UNQUOTED(USE_RNDUNIX, $found,
                  [Defined if the default Unix RNG should be used.])
if test "$found" = "1"; then
   print_egd_notice=yes
fi

LIST_MEMBER(egd, $random_modules)
test "$found" = "1" && GCRYPT_RANDOM="$GCRYPT_RANDOM rndegd.lo"
AC_DEFINE_UNQUOTED(USE_RNDEGD, $found,
                  [Defined if the EGD based RNG should be used.])

LIST_MEMBER(w32, $random_modules)
test "$found" = "1" && GCRYPT_RANDOM="$GCRYPT_RANDOM rndw32.lo"
AC_DEFINE_UNQUOTED(USE_RNDW32, $found,
                  [Defined if the Windows specific RNG should be used.])

AC_SUBST([GCRYPT_CIPHERS])
AC_SUBST([GCRYPT_PUBKEY_CIPHERS])
AC_SUBST([GCRYPT_DIGESTS])
AC_SUBST([GCRYPT_RANDOM])

AC_SUBST(LIBGCRYPT_CIPHERS, $enabled_ciphers)
AC_SUBST(LIBGCRYPT_PUBKEY_CIPHERS, $enabled_pubkey_ciphers)
AC_SUBST(LIBGCRYPT_DIGESTS, $enabled_digests)

AC_CONFIG_FILES([
Makefile
m4/Makefile
mpi/Makefile
cipher/Makefile
doc/Makefile
src/Makefile
src/gcrypt.h
src/libgcrypt-config
tests/Makefile
w32-dll/Makefile
])
AC_OUTPUT

# Give some feedback
echo

if test "$print_egd_notice" = "yes"; then
cat <<G10EOF
  
   The performance of the Unix random gatherer module (rndunix) is not
   very good and it does not keep the entropy pool over multiple
   invocations of GnuPG.  The suggested way to overcome this problem is
   to use the
  
                 Entropy Gathering Daemon (EGD)
  
   which provides a entropy source for the whole system.  It is written
   in Perl and available at the GnuPG FTP servers.  To enable EGD you
   should rerun configure with the option "--enable-static-rnd=egd".
   For more information consult the GnuPG webpages:
  
             http://www.gnupg.org/download.html#egd

G10EOF
fi

warn=""
for file in "random_modules"; do
  case "$file" in rndunix | rndw32) warn="$warn $file";; esac
done
if test -n "$warn"; then
  echo "Please note that you are building a version of Libgcrypt with"
  echo "  $warn"
  echo "included.  These parts are licensed under the GPL and thus the"
  echo "use of this library has to comply with the conditions of the GPL."
fi

# Give some feedback
echo   "                Configured for: $PRINTABLE_OS_NAME ($target)"
echo
