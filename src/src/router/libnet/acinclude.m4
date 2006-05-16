dnl $Id: acinclude.m4,v 1.1 2004/04/27 01:35:00 dyang Exp $
dnl
dnl     Libnet specific autoconf macros
dnl     Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
dnl                              route|daemon9 <route@infonexus.com>
dnl     All rights reserved.
dnl

dnl
dnl Checks to see if this linux kernel has a working PF_PACKET
dnl
dnl usage:
dnl
dnl     AC_LIBNET_CHECK_PF_PACKET
dnl
dnl results:
dnl
dnl     HAVE_PF_PACKET (DEFINED)
dnl

AC_DEFUN(AC_LIBNET_CHECK_PF_PACKET,
[
    AC_MSG_CHECKING(for PF_PACKET)
    AC_CACHE_VAL(ac_libnet_have_pf_packet,

        [case "$target_os" in

        linux)
                ac_libnet_have_pf_packet = no
                ;;
        *)

    cat > pf_packet-test.c << EOF
#include <net/if.h>
#if (__GLIBC__)
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#else
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif
#if (PF_PACKET)
#ifndef SOL_PACKET
#define SOL_PACKET 263
#endif  /* SOL_PACKET */
#include <linux/if_packet.h>
#endif
#include "./include/libnet.h"

int
main()
{
#if (PF_PACKET)
    int fd;
    struct sockaddr_ll sa;
    struct ifreq ifr;
    struct packet_mreq mr;
    char *device ="lo";

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd == -1)
    {
        printf("choked");
        exit (EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    strcpy(ifr.ifr_name, device);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
    {
        printf("choked");
        exit (EXIT_FAILURE);
    }
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_protocol = htons(ETH_P_ALL);

    memset(&mr, 0, sizeof (mr));
    mr.mr_ifindex = sa.sll_ifindex;
    mr.mr_type = PACKET_MR_ALLMULTI;

    if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (char *)&mr,
            sizeof (mr)) < 0)
    {
        printf("choked\n");
        exit (EXIT_FAILURE);
    }
    /* yay.  we made it and it workz! */
    printf("yes");
#else   /* PF_PACKET */
    printf("no");
#endif
    exit (EXIT_SUCCESS);
}
EOF
    ${CC-cc} -o pf_packet-test $CFLAGS pf_packet-test.c >/dev/null 2>&1

    # Oopz 4.3 BSD doesn't have this.  Sorry.
    if test ! -x ./pf_packet-test ; then
        ac_libnet_have_pf_packet=choked
    else
        ac_libnet_have_pf_packet=`./pf_packet-test`;
    fi

    if test $ac_libnet_have_pf_packet = choked; then
        AC_MSG_RESULT(test program choked... assuming no)
    elif test $ac_libnet_have_pf_packet = yes; then
        AC_DEFINE(HAVE_PF_PACKET)
        LIBNET_CONFIG_DEFINES="$LIBNET_CONFIG_DEFINES -DHAVE_PF_PACKET"
    fi

    if test $ac_libnet_have_pf_packet != choked; then
        AC_MSG_RESULT($ac_libnet_have_pf_packet)
    fi
    rm -f pf_packet-test* core core.pf_packet-test
    ;;
    esac])
])

dnl
dnl Looks for a previous libnet version and attempts to determine which verion
dnl it is.  Version 0.8 was the first version that actually knew internally
dnl what version it was.
dnl
dnl usage:
dnl
dnl     AC_LIBNET_CHECK_LIBNET_VERSION
dnl
dnl results:
dnl
dnl
dnl

AC_DEFUN(AC_LIBNET_CHECK_LIBNET_VER,
[
    AC_CHECK_LIB(net, libnet_build_ip, AC_MSG_CHECKING(version) \

changequote(<<, >>)dnl
    if [[ ! -f $LIB_PREFIX/libnet.a ]] ; then
changequote([, ])dnl
        AC_MSG_RESULT($LIB_PREFIX/libnet.a doesn't exist)
        AC_MSG_RESULT(previous libnet install lives elsewhere, you should probably find it)
    else
        __LIBNET_VERSION=`strings $LIB_PREFIX/libnet.a | grep "libnet version"\
                | cut -f3 -d" "`;\
        if test -z "$__LIBNET_VERSION"; then
            AC_MSG_RESULT(<0.8)
        else
            AC_MSG_RESULT($__LIBNET_VERSION)
        fi
    fi\
    )
])


dnl
dnl Checks to see if this linux kernel uses ip_sum or ip_csum
dnl (Pulled from queso)
dnl
dnl usage:
dnl
dnl     AC_LIBNET_CHECK_IP_CSUM
dnl
dnl results:
dnl
dnl     HAVE_STRUCT_IP_CSUM (DEFINED)
dnl

AC_DEFUN(AC_LIBNET_CHECK_IP_CSUM,
[
    AC_MSG_CHECKING([struct ip contains ip_csum])
    AC_TRY_COMPILE([
        #define __BSD_SOURCE
        #define _BSD_SOURCE
        #include <sys/types.h>
        #include <netinet/in.h>
        #include <netinet/in_systm.h>
        #include <netinet/ip.h>],
        [
            struct ip ip;
            ip.ip_csum = 0;
        ],
        [AC_MSG_RESULT(yes);
        AC_DEFINE(HAVE_STRUCT_IP_CSUM)],
        [AC_MSG_RESULT(no);
    ])
])

dnl
dnl Checks to see if unaligned memory accesses fail
dnl (Pulled from libpcap)
dnl
dnl usage:
dnl
dnl     AC_LBL_UNALIGNED_ACCESS
dnl
dnl results:
dnl
dnl     LBL_ALIGN (DEFINED)
dnl

AC_DEFUN(AC_LBL_UNALIGNED_ACCESS,
    [AC_MSG_CHECKING(if unaligned accesses fail)
    AC_CACHE_VAL(ac_cv_lbl_unaligned_fail,
        [case "$target_cpu" in

        alpha|hp*|mips|sparc)
                ac_cv_lbl_unaligned_fail=yes
                ;;

        *)
                cat >conftest.c <<EOF
#                   include <sys/types.h>
#                   include <sys/wait.h>
#                   include <stdio.h>
                    unsigned char a[[5]] = { 1, 2, 3, 4, 5 };
                    main()
                    {
                        unsigned int i;
                        pid_t pid;
                        int status;
                        /* avoid "core dumped" message */
                        pid = fork();
                        if (pid <  0)
                        {
                            exit(2);
                        }
                        if (pid > 0)
                        {
                            /* parent */
                            pid = waitpid(pid, &status, 0);
                            if (pid < 0)
                            {
                                exit(3);
                            }
                            exit(!WIFEXITED(status));
                        }
                        /* child */
                        i = *(unsigned int *)&a[[1]];
                        printf("%d\n", i);
                        exit(0);
                    }
EOF
                ${CC-cc} -o conftest $CFLAGS $CPPFLAGS $LDFLAGS \
                    conftest.c $LIBS > /dev/null 2>&1
                # Oopz 4.3 BSD doesn't have this.  Sorry.
                if test ! -x conftest ; then
                        dnl failed to compile for some reason
                        ac_cv_lbl_unaligned_fail=yes
                else
                        ./conftest > conftest.out
                        if test ! -s conftest.out ; then
                                ac_cv_lbl_unaligned_fail=yes
                        else
                                ac_cv_lbl_unaligned_fail=no
                        fi
                fi
                rm -f conftest* core core.conftest
                ;;
        esac])
    AC_MSG_RESULT($ac_cv_lbl_unaligned_fail)
    if test $ac_cv_lbl_unaligned_fail = yes ; then
            AC_DEFINE(LBL_ALIGN)
    fi
])


dnl
dnl Checks endianess
dnl
dnl usage:
dnl
dnl     AC_LIBNET_ENDIAN_CHECK
dnl
dnl results:
dnl
dnl     LIBNET_BIG_ENDIAN = 1   or
dnl     LIBNET_LIL_ENDIAN = 1
dnl

AC_DEFUN(AC_LIBNET_ENDIAN_CHECK,
    [AC_MSG_CHECKING(machine endianess)

    cat > conftest.c << EOF
#       include <stdio.h>
#       include <stdlib.h>

        int main()
        {
            union
            {
                short s;
                char c[[sizeof(short)]];
            } un;

            un.s = 0x0102;
            if (sizeof (short) == 2)
            {
                if (un.c [[0]] == 1 && un.c [[1]] == 2)
                {
                    printf("B\n");
                }
                else
                {
                    if (un.c [[0]] == 2 && un.c [[1]] == 1)
                    {
                        printf("L\n");
                    }
                }
            }
            else
            {
                printf("?\n");
            }
            return (EXIT_SUCCESS);
        }
EOF
        ${CC-cc} -o conftest $CFLAGS $CPPFLAGS $LDFLAGS conftest.c $LIBS > /dev/null 2>&1
        # Oopz 4.3 BSD doesn't have this.  Sorry.
        if test ! -x conftest ; then
dnl failed to compile for some reason
            ac_cv_libnet_endianess=unknown
        else
            ./conftest > conftest.out
            result=`cat conftest.out`
            if test $result = "B"; then
                ac_cv_libnet_endianess=big
            elif test $result = "L"; then
                ac_cv_libnet_endianess=lil
            else
                ac_cv_libnet_endianess=unknown
            fi                                
        fi
        rm -f conftest* core core.conftest

        AC_MSG_RESULT($ac_cv_libnet_endianess)

        if test $ac_cv_libnet_endianess = big ; then
            AC_DEFINE(LIBNET_BIG_ENDIAN)
        LIBNET_CONFIG_DEFINES="$LIBNET_CONFIG_DEFINES -DLIBNET_BIG_ENDIAN"
        elif test $ac_cv_libnet_endianess = lil ; then
            AC_DEFINE(LIBNET_LIL_ENDIAN)
        LIBNET_CONFIG_DEFINES="$LIBNET_CONFIG_DEFINES -DLIBNET_LIL_ENDIAN"
        fi
    ])
