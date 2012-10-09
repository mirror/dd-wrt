# Copyright (c) 2004 Damien Miller
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

AC_INIT
AC_CONFIG_SRCDIR([softflowd.c])

AC_CONFIG_HEADER(config.h)
AC_PROG_CC
AC_PROG_INSTALL

# Optional verbose warnings for gcc, see below
WFLAGS="-Wall -Waggregate-return -Wcast-align -Wcast-qual"
WFLAGS="$WFLAGS -Wmissing-declarations -Wmissing-prototypes"
WFLAGS="$WFLAGS -Wno-conversion -Wpointer-arith -Wshadow"
WFLAGS="$WFLAGS -Wuninitialized -Wcast-align -Wcast-qual"
WFLAGS="$WFLAGS -Wformat=2 -Wformat-nonliteral -Wwrite-strings" 

# Process flag arguments early, so they are available for tests later
AC_ARG_ENABLE(gcc-warnings,
	[  --enable-gcc-warnings   Enable verbose warnings (only for gcc)],
	[ if test "x$enableval" = "xyes" ; then CFLAGS="$CFLAGS $WFLAGS"; fi ]
)
AC_ARG_WITH(cflags,
	[  --with-cflags           Specify additional compiler flags],
	[ if test "x$withval" != "xno" ; then CFLAGS="$CFLAGS $withval"; fi ]	
)
AC_ARG_WITH(cppflags,
	[  --with-cppflags         Specify additional preprocessor flags] ,
	[ if test "x$withval" != "xno"; then CPPFLAGS="$CPPFLAGS $withval"; fi ]
)
AC_ARG_WITH(ldflags,
	[  --with-ldflags          Specify additional linker flags],
	[ if test "x$withval" != "xno" ; then LDFLAGS="$LDFLAGS $withval"; fi ]	
)
AC_ARG_WITH(libs,
	[  --with-libs             Specify additional libraries to link with],
	[ if test "x$withval" != "xno" ; then LIBS="$LIBS $withval"; fi ]	
)
AC_ARG_WITH(chrootdir,
	[  --with-chrootdir        Specify chroot directory],
	[ AC_DEFINE_UNQUOTED([PRIVDROP_CHROOT_DIR], ["${withval}"], [privdrop chroot directory]) ]	
)

AC_CHECK_HEADERS(net/bpf.h pcap.h pcap-bpf.h)

dnl AC_CHECK_HEADERS(netinet/in_systm.h netinet/tcp.h netinet/udp.h)
dnl 
dnl # This ugliness is because of autoconf's stupid default include list
dnl AC_CHECK_HEADERS([netinet/ip.h], 
dnl 	[AC_DEFINE([HAVE_HAVE_NETINET_IP_H], 1, [has netinet/ip.h])], [], 
dnl 	[
dnl #include <sys/types.h>
dnl #include <netinet/in.h>
dnl #if HAVE_NETINET_IN_SYSTM_H
dnl #include <netinet/in_systm.h>
dnl #endif
dnl 	])

AC_CHECK_MEMBER([struct sockaddr.sa_len], 
	[AC_DEFINE([SOCK_HAS_LEN], 1, [struct sockaddr contains length])], ,
	[#include <sys/types.h>
	 #include <sys/socket.h>])

AC_CHECK_MEMBER(struct ip6_ext.ip6e_nxt,
	[AC_DEFINE([HAVE_STRUCT_IP6_EXT], 1, [struct ip6_ext.ip6e_nxt exists])],
	[],
	[
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <netinet/ip6.h>
	])

AC_SEARCH_LIBS(daemon, bsd)
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket, socket)
AC_CHECK_LIB(pcap, pcap_open_live)

AC_CHECK_FUNCS(closefrom daemon setresuid setreuid setresgid setgid strlcpy strlcat)

AC_CHECK_TYPES([u_int64_t, int64_t, uint64_t, u_int32_t, int32_t, uint32_t])
AC_CHECK_TYPES([u_int16_t, int16_t, uint16_t, u_int8_t, int8_t, uint8_t])
AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short int, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long int, 4)
AC_CHECK_SIZEOF(long long int, 8)

if test "x$ac_cv_type_uint8_t" = "xyes" ; then
	AC_DEFINE([OUR_CFG_U_INT8_T], [uint8_t], [8-bit unsigned int])
elif test "x$ac_cv_sizeof_char" = "x1" ; then
	AC_DEFINE([OUR_CFG_U_INT8_T], [unsigned char], [8-bit unsigned int])
else
	AC_MSG_ERROR([No 8-bit unsigned int type found])
fi
if test "x$ac_cv_sizeof_char" = "x1" ; then
	AC_DEFINE([OUR_CFG_INT8_T], [signed char], [8-bit signed int])
else
	AC_MSG_ERROR([No 8-bit signed int type found])
fi

if test "x$ac_cv_type_uint16_t" = "xyes" ; then
	AC_DEFINE([OUR_CFG_U_INT16_T], [uint16_t], [16-bit unsigned int])
elif test "x$ac_cv_sizeof_short_int" = "x2" ; then
	AC_DEFINE([OUR_CFG_U_INT16_T], [unsigned short int], [16-bit unsigned int])
else
	AC_MSG_ERROR([No 16-bit unsigned int type found])
fi
if test "x$ac_cv_sizeof_short_int" = "x2" ; then
	AC_DEFINE([OUR_CFG_INT16_T], [short int], [16-bit signed int])
else
	AC_MSG_ERROR([No 16-bit signed int type found])
fi

if test "x$ac_cv_type_uint32_t" = "xyes" ; then
	AC_DEFINE([OUR_CFG_U_INT32_T], [uint32_t], [32-bit unsigned int])
elif test "x$ac_cv_sizeof_int" = "x4" ; then
	AC_DEFINE([OUR_CFG_U_INT32_T], [unsigned int], [32-bit unsigned int])
else
	AC_MSG_ERROR([No 32-bit unsigned int type found])
fi
if test "x$ac_cv_sizeof_int" = "x4" ; then
	AC_DEFINE([OUR_CFG_INT32_T], [int], [32-bit signed int])
else
	AC_MSG_ERROR([No 32-bit signed int type found])
fi

if test "x$ac_cv_type_uint64_t" = "xyes" ; then
	AC_DEFINE([OUR_CFG_U_INT64_T], [uint64_t], [64-bit unsigned int])
elif test "x$ac_cv_sizeof_long_int" = "x8" ; then
	AC_DEFINE([OUR_CFG_U_INT64_T], [unsigned long int], [64-bit unsigned int])
elif test "x$ac_cv_sizeof_long_long_int" = "x8" ; then
	AC_DEFINE([OUR_CFG_U_INT64_T], [unsigned long long int], [64-bit unsigned int])
else
	AC_MSG_ERROR([No 64-bit unsigned int type found])
fi
if test "x$ac_cv_sizeof_long_int" = "x8" ; then
	AC_DEFINE([OUR_CFG_INT64_T], [long int], [64-bit signed int])
elif test "x$ac_cv_sizeof_long_long_int" = "x8" ; then
	AC_DEFINE([OUR_CFG_INT64_T], [long long int], [64-bit signed int])
else
	AC_MSG_ERROR([No 64-bit signed int type found])
fi

if test "x$ac_cv_header_pcap_bpf_h" != "xyes" && \
   test "x$ac_cv_header_net_bpf_h" != "xyes" ; then
	AC_MSG_ERROR([No BPF header found])
fi
if test "x$ac_cv_header_pcap_h" != "xyes" ; then
	AC_MSG_ERROR([No pcap.h header found])
fi
if test "x$ac_cv_lib_pcap_pcap_open_live" != "xyes" ; then
	AC_MSG_ERROR([libpcap not found])
fi



AC_EXEEXT
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

