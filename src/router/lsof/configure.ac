AC_INIT([lsof],[4.99.0])
AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT([disable-fast-install]) # avoid lt-lsof naming
# Locate custom m4 macros
AC_CONFIG_MACRO_DIR([m4])

# Require C compiler
AC_PROG_CC
# For legacy automake on e.g. CentOS 7
AM_PROG_CC_C_O

# Detect OS
AC_CANONICAL_HOST

LSOF_VSTR=$(uname -r)
LSOF_DIALECT=unknown
LSOF_DIALECT_DIR=unknown
LSOF_TGT=unknown
AS_CASE([${host_os}],
[linux*], [
	LSOF_DIALECT=linux
	LSOF_DIALECT_DIR=linux
	LSOF_TGT=linux

	# Compute version
	LSOF_VERS=$(echo $LSOF_VSTR | sed 's/\./ /g' | awk '{printf "%d%d%03d",$1,$2,$3}')

	# Always use large file
	CFLAGS="$CFLAGS -D_FILE_OFFSET_BITS=64"

	# Enable LTbigf test
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_BIGF -D_FILE_OFFSET_BITS=64"
], [darwin*], [
	LSOF_DIALECT=darwin
	LSOF_DIALECT_DIR=darwin
	LSOF_TGT=darwin

	AS_CASE([$LSOF_VSTR],
	[7.*],	[LSOF_VERS=700],		# Mac OS X 10.3 (Panther)
	[8.*],	[LSOF_VERS=800],		# Mac OS X 10.4 (Tiger)
	[9.*],	[LSOF_VERS=900],		# Mac OS X 10.5 (Leopard)
	[10.*],	[LSOF_VERS=1000],		# Mac OS X 10.6 (SnowLeopard)
	[11.*],	[LSOF_VERS=1100],		# Mac OS X 10.7 (Lion)
	[12.*],	[LSOF_VERS=1200],		# Mac OS X 10.8 (Mountain Lion)
	[13.*],	[LSOF_VERS=1300],		# Mac OS X 10.9 (Mavericks)
	[14.*],	[LSOF_VERS=1400],		# Mac OS X 10.10 (Yosemite)
	[15.*],	[LSOF_VERS=1500],		# Mac OS X 10.11 (El Capitan)
	[16.*],	[LSOF_VERS=1600],		# macOS 10.12 (Sierra)
	[17.*],	[LSOF_VERS=1700],		# macOS 10.13 (High Sierra)
	[18.*],	[LSOF_VERS=1800],		# macOS 10.14 (Mojave)
	[19.*],	[LSOF_VERS=1900],		# macOS 10.15 (Catalina)
	[
		AC_MSG_NOTICE([Unknown Darwin release: $(uname -r)])
		AC_MSG_NOTICE([Assuming Darwin 19.0])
		LSOF_VERS=1900
	])

	# Add DARWINV to cflags
	CFLAGS="$CFLAGS -DDARWINV=$LSOF_VERS"

	# Enable LTbigf test
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_BIGF"
], [freebsd*], [
	LSOF_DIALECT=freebsd
	LSOF_DIALECT_DIR=freebsd
	LSOF_TGT=freebsd

	AS_CASE([$LSOF_VSTR],
	[10*],	[LSOF_VERS=10000],
	[11*],	[LSOF_VERS=11000],
	[12*],	[LSOF_VERS=12000],
	[13*],	[LSOF_VERS=13000],
	[14*],	[LSOF_VERS=14000],
	[15*],	[LSOF_VERS=15000], [
		AC_MSG_ERROR([Unknown FreeBSD release: $(uname -r)])
	])

	# Add FREEBSDV to cflags
	CFLAGS="$CFLAGS -DFREEBSDV=$LSOF_VERS"

	# Always define HASPROCFS
	CFLAGS="$CFLAGS -DHASPROCFS"

	# Always link libutil
	LDFLAGS="$LDFLAGS -lutil"

	# Detect FreeBSD source code
	AS_IF([test -d /usr/src/sys], [
		FREEBSD_SYS=/usr/src/sys
	], [test -d /sys], [
		FREEBSD_SYS=/sys
	], [
		AC_MSG_WARN([No kernel sources in /usr/src/sys or /sys])
	])

	# Enable LTbigf test
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_BIGF"
], [netbsd*],  [
	LSOF_DIALECT=netbsd
	LSOF_DIALECT_DIR=netbsd
	LSOF_TGT=netbsd

	case $LSOF_VSTR in
		8.[[0123]]*)
			LSOF_VERS="8000000"
			;;
		8.99.*)
			LSOF_VERS="8099000"
			;;
		8.*)
			LSOF_VERS="8000000"
			AC_MSG_WARN([Unsupported NetBSD release: $(uname -r)])
			AC_MSG_WARN([Configuring for NetBSD 8.0])
			;;
		9.[[0123]]*)
			LSOF_VERS="9000000"
			;;
		9.99.10[[45678]])
			LSOF_VERS="9099104"
			;;
		9.99.*)
			LSOF_VERS="9099000"
			;;
		9.*)
			LSOF_VERS="9000000"
			AC_MSG_WARN([Unsupported NetBSD release: $(uname -r)])
			AC_MSG_WARN([Configuring for NetBSD 9.0])
			;;
		10.99.*)
			LSOF_VERS="10099000"
			;;
		10.*)
			LSOF_VERS="10000000"
			AC_MSG_WARN([Unsupported NetBSD release: $(uname -r)])
			AC_MSG_WARN([Configuring for NetBSD 10.0])
			;;
		*)
			AC_MSG_ERROR([Unknown NetBSD release: $(uname -r)])
			;;
	esac

	# Add NETBSDV to cflags
	CFLAGS="$CFLAGS -DNETBSDV=$LSOF_VERS"

	# Define _KMEMUSER for paddr_t and __NAMECACHE_PRIVATE for struct namecache
	CFLAGS="$CFLAGS -D_KMEMUSER -D__NAMECACHE_PRIVATE"

	# Define HASNFSVATTRP because the relevant change in kernel src is in 1997
	# We can safely assume that n_vattr is a pointer
	CFLAGS="$CFLAGS -DHASNFSVATTRP"

	# Check /dev/kmem
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_KMEM"

	# Detect NetBSD source code
	AS_IF([test -d /usr/src/sys], [
		NETBSD_SYS=/usr/src/sys
		CFLAGS="$CFLAGS -I$NETBSD_SYS"
	], [
		AC_MSG_WARN([No kernel sources in /usr/src/sys])
	])
], [openbsd*], [
	LSOF_DIALECT=openbsd
	LSOF_DIALECT_DIR=openbsd
	LSOF_TGT=openbsd

	AS_CASE([$LSOF_VSTR],
	[7.2],	[LSOF_VERS=7020],
	[7.3],	[LSOF_VERS=7030],
	[
		AC_MSG_NOTICE([Unknown OpenBSD release: $(uname -r)])
		AC_MSG_NOTICE([Assuming OpenBSD 7.3])
		LSOF_VERS=7030
	])
], [solaris*], [
	LSOF_DIALECT=solaris
	LSOF_DIALECT_DIR=sun
	LSOF_TGT=solaris

	AS_CASE([$LSOF_VSTR],
	[5.11],	[LSOF_VERS=110000],
	[
		AC_MSG_NOTICE([Unknown Solaris release: $(uname -r)])
		AC_MSG_NOTICE([Assuming Solaris 5.11])
		LSOF_VERS=110000
	])

	# Add solaris to cflags
	CFLAGS="$CFLAGS -Dsolaris=$LSOF_VERS"

	# Link system libraries
	LIBS="$LIBS -lelf -lsocket -lnsl"

	# Handle missing header
	mkdir -p ./solaris11/sys
	touch ./solaris11/sys/extdirent.h
	CFLAGS="$CFLAGS -Isolaris11"
], [aix*], [
	LSOF_DIALECT=aix
	LSOF_DIALECT_DIR=aix
	LSOF_TGT=aix

	LSOF_TMP1=/usr/bin/oslevel
	AS_IF([test -x $LSOF_TMP1], [
		echo "Determining AIX version with $LSOF_TMP1."
		echo "This may take a while, depending on your maintenance level."
		LSOF_VSTR=`$LSOF_TMP1 | sed 's/[^0-9]*\([0-9\.]*\).*/\1/'`
		echo "$LSOF_TMP1 reports the version is $LSOF_VSTR."
	], [
		# If oslevel can't be used, build the version string with
		# `uname -rv` and issue a warning.

		LSOF_VSTR=`uname -rv | awk '{printf "%d.%d.0.0\n",\$2,\$1}'`
		echo "WARNING: can't execute $LSOF_TMP1; uname -rv reports"
		echo "         the version is $LSOF_VSTR;"
	])

	LSOF_VERS=`echo $LSOF_VSTR | sed 's/\.//g'`

	# Add AIXV to cflags
	CFLAGS="$CFLAGS -DAIXV=$LSOF_VERS"

	# Assuming recent AIX on PowerPC64
	CFLAGS="$CFLAGS -maix64 -DAIXA=1 -DAIX_KERNBITS=64 -DHASSTAT64"
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_AIXA=1"

	# JFS2 support
	# Add -fms-extensions to handle use of i_dev from the wInode anonymous
	# structure reference in the JFS2 inode structure of <j2/j2_inode.h>.
	CFLAGS="$CFLAGS -DHAS_JFS2 -I./lib/dialects/aix/aix5 -fms-extensions"
], [
	AC_MSG_ERROR(["Host $host_os not supported"])
])

# Pass dialect info to Makefile.am
AM_CONDITIONAL([LINUX], [test x$LSOF_DIALECT_DIR = xlinux])
AM_CONDITIONAL([DARWIN], [test x$LSOF_DIALECT_DIR = xdarwin])
AM_CONDITIONAL([FREEBSD], [test x$LSOF_DIALECT_DIR = xfreebsd])
AM_CONDITIONAL([NETBSD], [test x$LSOF_DIALECT_DIR = xnetbsd])
AM_CONDITIONAL([OPENBSD], [test x$LSOF_DIALECT_DIR = xopenbsd])
AM_CONDITIONAL([SOLARIS], [test x$LSOF_DIALECT_DIR = xsun])
AM_CONDITIONAL([AIX], [test x$LSOF_DIALECT_DIR = xaix])

# Pass OS version
LSOF_TMP=$(echo $LSOF_VSTR | sed 's/(/\\\\(/g' | sed 's/)/\\\\)/g')
CFLAGS="$CFLAGS -DLSOF_VSTR=\\\"$LSOF_TMP\\\""

# Pass LSOF_DIALECT/LSOF_DIALECT_DIR to Makefile.am
AC_SUBST([LSOF_DIALECT])
AC_SUBST([LSOF_DIALECT_DIR])

# Export public function with default visibility
AC_DEFINE([API_EXPORT], [__attribute__ ((visibility ("default")))],
	[Set visibility to default for exported API functions.])

# --enable-security to define HASSECURITY
AC_ARG_ENABLE(security, AS_HELP_STRING([--enable-security],
	[allow only the root user to list all open files @<:@default=no@:>@]), [], [enable_security=no])
AS_IF([test "x$enable_security" = xyes], [
	CFLAGS="$CFLAGS -DHASSECURITY"
])

# --enable-no-sock-security to define HASNOSOCKSECURITY
AC_ARG_ENABLE(no_sock_security, AS_HELP_STRING([--enable-no-sock-security],
	[combined with --enable-security, allow anyone to list anyone else's socket files @<:@default=no@:>@]), [], [enable_no_sock_security=no])
AS_IF([test "x$enable_no_sock_security" = xyes], [
	AS_IF([test "x$enable_security" = xyes], [
		CFLAGS="$CFLAGS -DHASNOSOCKSECURITY"
	], [
		AC_MSG_ERROR([--enable-no-sock-security must be used with --enable-security])
	])
])

# Check rpc/rpc.h or libtirpc
# with_libtirpc=yes/no/auto
AC_ARG_WITH(libtirpc, AS_HELP_STRING([--with-libtirpc],
	[build with libtirpc support @<:@default=auto@:>@]), [], [with_libtirpc=auto])
AS_IF([test "x$with_libtirpc" != xno], [
	PKG_CHECK_MODULES([LIBTIRPC], [libtirpc], [
		CFLAGS="$CFLAGS $LIBTIRPC_CFLAGS"
		LIBS="$LIBS $LIBTIRPC_LIBS"
		with_libtirpc=yes
	], [
		AS_IF([test "x$with_libtirpc" = xyes], [
			AC_MSG_ERROR([--with-libtirpc specified, but libtirpc could not be found])
		])
		with_libtirpc=no
	])
])

AS_IF([test "x$with_libtirpc" = "xyes"],
        [AC_DEFINE(HAVE_LIBTIRPC, 1, [Enable libtirpc support])])

AC_CHECK_HEADERS([rpc/rpc.h])

# Detect glibc for GLIBCV
AC_CHECK_DECLS([__GLIBC__])

# Detect ipv6 headers and definitions for HASIPv6
AC_CHECK_DECLS([AF_INET6], [], [], [[#include <sys/socket.h>]])
AC_CHECK_HEADERS([netinet6/in6.h netinet/in6.h netinet/ip6.h])

# Detect tcp definitions for NEEDS_NETINET_TCPH
AC_CHECK_DECLS([TCP_ESTABLISHED], [], [], [[#include <netinet/tcp.h>]])

# Detect unix socket endpoint headers for HASUXSOCKEPT
AC_CHECK_HEADERS([linux/sock_diag.h linux/unix_diag.h])

# Detect pty endpoint definition for HASPTYEPT
AC_CHECK_DECLS([TTYAUX_MAJOR], [], [], [[#include <linux/major.h>]])

# Detect socket state definition for HASSOSTATE and HASSOOPT
AC_CHECK_DECLS([SS_CONNECTED], [], [], [[#include <linux/net.h>]])

# Define _FILE_OFFSET_BITS if necessary
AC_SYS_LARGEFILE

# Detect strftime function for HAS_STRFTIME
AC_CHECK_FUNCS([strftime])

# Detect selinux headers for HASSELINUX
# with_selinux=yes/no/auto
AC_ARG_WITH(selinux, AS_HELP_STRING([--with-selinux],
	[build with selinux support @<:@default=auto@:>@]), [], [with_selinux=auto])
AS_IF([test "x$with_selinux" != xno], [
	AC_CHECK_HEADERS([selinux/selinux.h], [LIBS="$LIBS -lselinux"], [
		AS_IF([test "x$with_selinux" = xyes], [
			AC_MSG_ERROR([--with-selinux specified, but libselinux could not be found])
		])
	])
])

# Detect utmpx headers for HASUTMPX
AC_CHECK_HEADERS([utmpx.h])

# Detect vm_memattr_t definition for HAS_VM_MEMATTR_T
AC_CHECK_TYPES([vm_memattr_t], [], [], [[#include <sys/types.h>
		#include <vm/vm.h>]])

# Detect doadump function for NEEDS_BOOLEAN_T
AC_CHECK_DECLS([doadump], [], [], [[#define _KERNEL 1
		#include <sys/types.h>
		#include <sys/param.h>
		#include <sys/conf.h>]])

# Detect struct vnode definition for HAS_KVM_VNODE
# Linking libkvm is not required in AIX
AS_IF([test x$LSOF_DIALECT_DIR != xaix], [
	AC_EGREP_CPP([struct vnode], [#define _KVM_VNODE 1
			#include <sys/vnode.h>], [AC_DEFINE([HAVE_KVM_VNODE], [1], ["Define to 1 if struct vnode exists"])
			LIBS="$LIBS -lkvm"])
])

# Detect SBS_CANTSENDMORE definition for HASSBSTATE
AC_CHECK_DECLS([SBS_CANTSENDMORE], [], [], [[#include <sys/types.h>
		#include <sys/socketvar.h>]])

# Detect lockf_entry definition for HAS_LOCKF_ENTRY
AC_CHECK_TYPES([struct lockf_entry], [
	HEADER_GENERATE([lockf_owner.h], [${FREEBSD_SYS}/kern/kern_lockf.c], ["^struct lock_owner {"], ["^};"], [LOCKF_OWNER_H])
	AC_MSG_NOTICE([lockf_owner.h creation succeeded.])
	CFLAGS="$CFLAGS -DHAS_LOCKF_ENTRY"
], [], [#include <sys/types.h>
		#include <sys/lockf.h>])

# Detect ki_numthreads definition for HASTASKS
AC_CHECK_MEMBERS([struct kinfo_proc.ki_numthreads], [], [], [#include <sys/types.h>
		#include <sys/user.h>])

# Detect i_effnlink/i_ffs_effnlink for HASEFFNLINK
AC_CHECK_MEMBERS([struct inode.i_effnlink], [CFLAGS="$CFLAGS -DHASEFFNLINK=i_effnlink"], [], [#define __BSD_VISIBLE 1
		#include <sys/types.h>
		#include <ufs/ufs/quota.h>
		#include <ufs/ufs/inode.h>])

# Detect struct file.f_vnode for HASF_VNODE
AC_CHECK_MEMBERS([struct file.f_vnode], [], [], [#define __BSD_VISIBLE 1
		#define _KERNEL 1
		#define GENOFFSET
		#include <sys/types.h>
		#include <sys/file.h>])

# Detect struct filedescent for HAS_FILEDESCENT
AC_CHECK_TYPES([struct filedescent], [], [], [#define __BSD_VISIBLE 1
		#include <sys/types.h>
		#include <sys/filedesc.h>])

# Detect fs/tmpfs/tmpfs.h for HAS_TMPFS
AS_IF([test -r ${FREEBSD_SYS}/fs/tmpfs/tmpfs.h], [
	AC_DEFINE([HAS_TMPFS], [1], [Define if fs/tmpfs/tmpfs.h is found])
])

# Detect wctype.h for HASWCTYPE_H
AC_CHECK_HEADERS([wctype.h])

# Detect struct inode.i_din2 for HAS_UFS1_2
AC_CHECK_MEMBERS([struct inode.i_din2], [], [], [#define __BSD_VISIBLE 1
		#include <sys/types.h>
		#include <ufs/ufs/quota.h>
		#include <ufs/ufs/inode.h>])

# Detect struct vnode.v_lockf definition for HAS_V_LOCKF
AC_CHECK_MEMBERS([struct vnode.v_lockf], [], [], [#define _KVM_VNODE 1
		#define __BSD_VISIBLE 1
		#include <sys/types.h>
		#include <sys/vnode.h>])

# Detect fdescfs version for HASFDESCFS
AC_CHECK_HEADERS([fs/fdescfs/fdesc.h], [], [], [#define _KERNEL 1
		#define __BSD_VISIBLE 1
		#include <sys/types.h>
		#include <sys/mount.h>])

# Detect pseudofs for HASPSEUDOFS
AS_IF([test -d ${FREEBSD_SYS}/fs/pseudofs], [
	CFLAGS="$CFLAGS -DHASPSEUDOFS -I$FREEBSD_SYS"
])

# Detect nullfs for HASNULLFS
AC_CHECK_HEADERS([fs/nullfs/null.h], [], [], [])

# Detect struct xtcpcb.t_maxseg for HAS_XTCPCB_TMAXSEG
AC_CHECK_MEMBERS([struct xtcpcb.t_maxseg], [], [], [#include <sys/types.h>
	#include <sys/queue.h>
	#include <sys/socketvar.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/in_pcb.h>
	#include <netinet/tcp_var.h>])

# Detect struct kinfo_file.kf_un.kf_sock.kf_sock_sendq for HAS_KF_SOCK_SENDQ
AC_CHECK_MEMBERS([struct kinfo_file.kf_un.kf_sock.kf_sock_sendq], [], [], [#include <sys/types.h>
	#include <sys/user.h>])

# Detect struct kinfo_file.kf_un.kf_file.kf_file_nlink for HAS_KF_FILE_NLINK
AC_CHECK_MEMBERS([struct kinfo_file.kf_un.kf_file.kf_file_nlink], [], [], [#include <sys/types.h>
	#include <sys/user.h>])

# Detect nfs/nfsproto.h for HASNFSPROTO
AC_CHECK_HEADERS([nfs/nfsproto.h])

# Detect uvm/uvm.h for HAS_UVM_INCL
AC_CHECK_HEADERS([uvm/uvm.h])

# Detect struct statvfs for HASSTATVFS
AC_CHECK_TYPES([struct statvfs], [], [], [#include <sys/statvfs.h>])

# Detect struct inode.i_ffs1_size for HASI_FFS1
AC_CHECK_MEMBERS([struct inode.i_ffs1_size], [], [], [#include <ufs/ufs/inode.h>])

# Detect getbootfile function for HASGETBOOTFILE
AC_CHECK_DECLS([getbootfile()], [
	LDFLAGS="$LDFLAGS -lutil"
], [], [#include <util.h>])

# Detect kvm_getproc2 function for HASKVMGETPROC2
AC_CHECK_DECLS([kvm_getproc2], [], [], [#include <kvm.h>])

# Detect fs/ptyfs/ptyfs.h header for HASPTYFS
AC_CHECK_HEADERS([fs/ptyfs/ptyfs.h])

# Detect fs/ptyfs/ptyfs.h header for HASPROCFS
AC_CHECK_HEADERS([miscfs/procfs/procfs.h])

# Detect PFSroot enum for HASPROCFS_PFSROOT
AC_CHECK_DECLS([PFSroot], [], [], [#define _KERNEL
	#include <sys/types.h>
	#include <miscfs/procfs/procfs.h>])

# Detect fs/tmpfs/tmpfs.h header for HASTMPFS
AC_CHECK_HEADERS([fs/tmpfs/tmpfs.h])

# Detect sys/pipe.h header for HAS_SYS_PIPEH
AC_CHECK_HEADERS([sys/pipe.h])

# Copy struct lockf definition from kernel source code for HAS_LOCKF_H
AS_IF([test x$LSOF_DIALECT = xnetbsd], [
	# Generate lockf.h from kern/vfs_lockf.c
	HEADER_GENERATE([lockf.h], [${NETBSD_SYS}/kern/vfs_lockf.c], ["^TAILQ_HEAD"], ["^};"], [LOCKF_H])

	AC_MSG_NOTICE([lockf.h creation succeeded])
	CFLAGS="$CFLAGS -DHAS_LOCKF_H"
])

# Detect fdescfs version for HASFDESCFS
AC_CHECK_HEADERS([miscfs/fdesc/fdesc.h])

# Detect struct fdescnode.fd_link for HASFDLINK
AC_CHECK_MEMBERS([struct fdescnode.fd_link], [], [], [#define _KERNEL
	#include <sys/types.h>
	#include <miscfs/fdesc/fdesc.h>])

# Detect miscfs/nullfs/null.h header for HASNULLFS
AC_CHECK_HEADERS([miscfs/nullfs/null.h], [], [], [#include <sys/mount.h>])

# Detect dup2 for HAS_DUP2
AC_CHECK_DECLS([dup2], [], [], [[#include <unistd.h>]])

# Detect closefrom for HAS_CLOSEFROM
AC_CHECK_DECLS([closefrom], [], [], [[#include <unistd.h>]])

# Detect sizeof(dev_t) for LT_DEV64
AC_CHECK_SIZEOF([dev_t])
AS_IF([test "x$ac_cv_sizeof_dev_t" = x8], [
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_DEV64"
])

# Detect inet/ipclassifier.h header for HAS_IPCLASSIFIER_H
AC_CHECK_HEADERS([inet/ipclassifier.h], [], [], [#define _KERNEL
	#include <inet/ip.h>])

# Detect struct conn_s.conn_ixa header for HAS_CONN_NEW
AC_CHECK_MEMBERS([struct conn_s.conn_ixa], [], [], [#define _KERNEL
	#include <inet/ip.h>
	#include <inet/ipclassifier.h>])

# Detect sys/zone.h header for HASZONES
AC_CHECK_HEADERS([sys/zone.h])

# Detect pad_mutex_t for HAS_PAD_MUTEX
AC_CHECK_TYPES([pad_mutex_t], [], [], [#define _KERNEL
	#include <sys/mutex.h>])

# Detect libctf.h header for HAS_LIBCTF
AC_CHECK_HEADERS([libctf.h], [LIBS="$LIBS -lctf"])

# Detect pc_direntpersec macro for HAS_PC_DIRENTPERSEC
AC_CHECK_DECLS([pc_direntpersec((struct pcfs *)NULL)], [], [], [#include <sys/fs/pc_fs.h>])

# Detect sys/fs/zfs.h header for HAS_ZFS
AC_CHECK_HEADERS([sys/fs/zfs.h])

# Detect sys/cred_impl.h header for HAS_CRED_IMPL_H
AC_CHECK_HEADERS([sys/cred_impl.h])

# Detect struct vnode.v_path for HAS_V_PATH
AC_CHECK_MEMBERS([struct vnode.v_path], [
	LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_VPATH"
], [], [#include <sys/vnode.h>])

# Detect VSOCK for HAS_VSOCK
AC_CHECK_DECLS([VSOCK], [], [], [#include <sys/vnode.h>])

# Detect struct aio_req for HAS_AIO_REQ_STRUCT
AC_CHECK_TYPES([struct aio_req], [], [], [#define _KERNEL
	#include <sys/aio_req.h>])

# Detect sys/rgm.h header for HAS_SYS_RGM_H
AC_CHECK_HEADERS([sys/rgm.h])

# Detect rpc/rpc_tags.h header for HAS_RPC_RPC_TAGS_H
AC_CHECK_HEADERS([rpc/rpc_tags.h])

# Generate Makefile from Makefile.in/am
AC_CONFIG_FILES([Makefile])

# Pass build configurations to version.h.in
AC_SUBST(cc, $CC)
AC_SUBST(ccv, $($CC -v 2>&1 | sed -n 's/.*version \(.*\)/\1/p'))
AC_SUBST(ccflags, $CFLAGS)
AC_SUBST(ldflags, "$LDFLAGS$LIBS")
# Reproducible build
AS_IF([test "X$SOURCE_DATE_EPOCH" = "X"], [
	AC_SUBST(host, $(uname -n))
	AC_SUBST(logname, $LOGNAME)
	AC_SUBST(user, $USER)
	AC_SUBST(sysinfo, $(uname -a))
])

# Generate version.h/autotools.h
AC_CONFIG_FILES([autotools/version.h autotools/autotools.h])

# Generate version
AC_CONFIG_FILES([version])

# Generate config.h
AC_CONFIG_HEADERS([config.h])

# For VPATH building
AC_CONFIG_LINKS([tests/common.bash:tests/common.bash
	lib/dialects/linux/tests/util-open-flags.bash:lib/dialects/linux/tests/util-open-flags.bash
	Lsof.8:Lsof.8 00DIALECTS:00DIALECTS 00DIST:00DIST])

# Generate cflags for tests
LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_DIAL_$LSOF_TGT -DLT_VERS=$LSOF_VERS"
# Override default lsof path
LSOF_TEST_CFLAGS="$LSOF_TEST_CFLAGS -DLT_DEF_LSOF_PATH=\\\"$PWD/lsof\\\""
AC_SUBST([LSOF_TEST_CFLAGS])

# Finish
AC_OUTPUT
