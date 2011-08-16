#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.62)
AC_INIT([daq], [0.5], [snort-team@sourcefire.com])
AM_INIT_AUTOMAKE([daq], [0.5])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([sfbpf/gencode.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Enable visibility if we can
AC_ENABLE_VISIBILITY()
# Special compiler flags for ICC, etc.
AC_SF_COMPILER_SETUP()

# IPv6 configuration
AC_ARG_ENABLE(ipv6, 
AC_HELP_STRING([--enable-ipv6],[build IPv6-capable version @<:@default=yes, if getaddrinfo available@:>@]),
    [],
    [enable_ipv6=ifavailable])
if test "$enable_ipv6" != "no"; then
    AC_CHECK_FUNC(getaddrinfo,
        [
        AC_DEFINE(INET6,1,[System IPv6 support])
        ],
        [
        if test "$enable_ipv6" != "ifavailable"; then
            AC_MSG_FAILURE([--enable-ipv6 was given, but getaddrinfo isn't available])
        fi
        ])
fi

# YACC/LEX configuration
AC_LBL_LEX_AND_YACC(V_LEX, V_YACC, sfbpf_)
if test "$V_LEX" = lex ; then
# Some versions of lex can't handle the definitions section of scanner.l .
# Try lexing it and complain if it can't deal.
    AC_CACHE_CHECK([for capable lex], sf_cv_capable_lex,
        if lex -t scanner.l > /dev/null 2>&1; then
            sf_cv_capable_lex=yes
        else
            sf_cv_capable_lex=insufficient
        fi)
    if test $sf_cv_capable_lex = insufficient ; then
        AC_MSG_ERROR(
        [Your operating system's lex is insufficient to compile
         libsfbpf.  flex is a lex replacement that has many advantages,
         including being able to compile libsfbpf.  For more
         information, see http://www.gnu.org/software/flex/flex.html .])
    fi
fi

# Checks for libraries.
AC_ARG_WITH(libpcap_includes,
            [  --with-libpcap-includes=DIR    libpcap include directory],
            [with_pcap_includes="$withval"], [with_pcap_includes="no"])

AC_ARG_WITH(libpcap_libraries,
            [  --with-libpcap-libraries=DIR   libpcap library directory],
            [with_pcap_libraries="$withval"], [with_pcap_libraries="no"])

if test "x$with_pcap_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_pcap_includes}"
fi

if test "x$with_pcap_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_pcap_libraries}"
fi

AC_ARG_WITH(dnet_includes,
            [  --with-dnet-includes=DIR    dnet include directory],
            [with_dnet_includes="$withval"], [with_dnet_includes="no"])

AC_ARG_WITH(dnet_libraries,
            [  --with-dnet-libraries=DIR   dnet library directory],
            [with_dnet_libraries="$withval"], [with_dnet_libraries="no"])

if test "x$with_dnet_includes" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -I${with_dnet_includes}"
fi

if test "x$with_dnet_libraries" != "xno"; then
    LDFLAGS="${LDFLAGS} -L${with_dnet_libraries}"
fi

DEFAULT_ENABLE=yes

# Modules configuration
AC_ARG_ENABLE(bundled-modules,
              AC_HELP_STRING([--disable-bundled-modules],[don't build any of the bundled DAQ modules by default]),
              [], [disable_bundled_modules=no])

if test "$disable_bundled_modules" = no; then
    DEFAULT_ENABLE=yes
else
    DEFAULT_ENABLE=no
fi

# AFPacket Module
AC_ARG_ENABLE(afpacket-module,
              AC_HELP_STRING([--disable-afpacket-module],[don't build the bundled AFPacket module]),
              [enable_afpacket_module="$enableval"], [enable_afpacket_module="$DEFAULT_ENABLE"])
if test "$enable_afpacket_module" = yes; then
    AC_CHECK_HEADERS([linux/if_ether.h linux/if_packet.h], [], [enable_afpacket_module=no])
fi
AM_CONDITIONAL([BUILD_AFPACKET_MODULE], [test "$enable_afpacket_module" = yes])

# Dump Module
AC_ARG_ENABLE(dump-module,
              AC_HELP_STRING([--disable-dump-module],[don't build the bundled Dump module]),
              [enable_dump_module="$enableval"], [enable_dump_module="$DEFAULT_ENABLE"])
if test "$enable_dump_module" = yes; then
    AC_CHECK_HEADER([pcap.h], [], [enable_dump_module=no])
fi
AM_CONDITIONAL([BUILD_DUMP_MODULE], [test "$enable_dump_module" = yes])

# IPFW Module
AC_ARG_ENABLE(ipfw-module,
              AC_HELP_STRING([--disable-ipfw-module],[don't build the bundled IPFW module]),
              [enable_ipfw_module="$enableval"], [enable_ipfw_module="$DEFAULT_ENABLE"])
AM_CONDITIONAL([BUILD_IPFW_MODULE], [test "$enable_ipfw_module" = yes])

# IPQ Module
AC_ARG_ENABLE(ipq-module,
              AC_HELP_STRING([--disable-ipq-module],[don't build the bundled IPQ module]),
              [enable_ipq_module="$enableval"], [enable_ipq_module="$DEFAULT_ENABLE"])
if test "$enable_ipq_module" = yes; then
    AC_CHECK_HEADERS([dnet.h netinet/in.h libipq.h], [], [enable_ipq_module=no])
    AC_CHECK_HEADER([linux/netfilter.h], [], [enable_ipq_module=no],
                    [
                        #include <netinet/in.h>
                    ])
fi
AM_CONDITIONAL([BUILD_IPQ_MODULE], [test "$enable_ipq_module" = yes])

# NFQ Module
AC_ARG_ENABLE(nfq-module,
              AC_HELP_STRING([--disable-nfq-module],[don't build the bundled NFQ module]),
              [enable_nfq_module="$enableval"], [enable_nfq_module="$DEFAULT_ENABLE"])
if test "$enable_nfq_module" = yes; then
    AC_CHECK_HEADERS([dnet.h netinet/in.h libnetfilter_queue/libnetfilter_queue.h], [], [enable_nfq_module=no])
    AC_CHECK_HEADER([linux/netfilter.h], [], [enable_nfq_module=no],
                    [
                        #include <netinet/in.h>
                    ])
fi
AM_CONDITIONAL([BUILD_NFQ_MODULE], [test "$enable_nfq_module" = yes])

# PCAP Module
AC_ARG_ENABLE(pcap-module,
              AC_HELP_STRING([--disable-pcap-module],[don't build the bundled PCAP module]),
              [enable_pcap_module="$enableval"], [enable_pcap_module="$DEFAULT_ENABLE"])
if test "$enable_pcap_module" = yes; then
    AC_CHECK_HEADER([pcap.h], [], [enable_pcap_module=no])
#    AC_CHECK_PCAP_VER("1.0.0")
fi
AM_CONDITIONAL([BUILD_PCAP_MODULE], [test "$enable_pcap_module" = yes])

AM_CONDITIONAL([BUILD_MODULES], [test "$enable_afpacket_module" = yes -o \
                                      "$enable_dump_module" = yes -o \
                                      "$enable_ipfw_module" = yes -o \
                                      "$enable_ipq_module" = yes -o \
                                      "$enable_nfq_module" = yes -o \
                                      "$enable_pcap_module" = yes])

AC_CHECK_LIB([dl], [dlopen], [LIBDL="-ldl"])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h memory.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([gethostbyname getpagesize memset munmap socket strchr strcspn strdup strerror strrchr strstr strtoul])

# Substitutions
AC_SUBST(V_LEX)
AC_SUBST(V_YACC)
AC_SUBST(XCCFLAGS)
AC_SUBST(LIBDL)

AC_CONFIG_FILES([Makefile
                 api/Makefile
                 os-daq-modules/Makefile
                 sfbpf/Makefile])
AC_OUTPUT

echo
echo "Build AFPacket DAQ module.. : $enable_afpacket_module"
echo "Build Dump DAQ module...... : $enable_dump_module"
echo "Build IPFW DAQ module...... : $enable_ipfw_module"
echo "Build IPQ DAQ module....... : $enable_ipq_module"
echo "Build NFQ DAQ module....... : $enable_nfq_module"
echo "Build PCAP DAQ module...... : $enable_pcap_module"
echo
