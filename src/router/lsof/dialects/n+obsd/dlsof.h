/*
 * dlsof.h - NetBSD and OpenBSD header file for lsof
 */


/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */


/*
 * $Id: dlsof.h,v 1.38 2006/03/28 21:54:08 abe Exp $
 */


#if	!defined(NETBSD_LSOF_H)
#define	NETBSD_LSOF_H	1

#include <stdlib.h>
#include <dirent.h>
#include <nlist.h>
#include <paths.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

# if	defined(HASGETBOOTFILE)
#include <util.h>
# endif	/* defined(HASGETBOOTFILE) */

#include <sys/filedesc.h>
#include <sys/mbuf.h>
  
# if	defined(HAS_LWP_H)
#include <sys/lwp.h>
# endif	/* defined(HAS_LWP_H) */

# if	(defined(OPENBSDV) && OPENBSDV>=3030) \
  ||	(defined(NETBSDV) && __NetBSD_Version__>=106060000)

#  if	defined(OPENBSDV) ||  __NetBSD_Version__<399001100
#define	_KERNEL
#  endif	/* defined(OPENBSDV) ||  __NetBSD_Version__<399001100 */

#  if	defined(NETBSDV) && __NetBSD_Version__<399001100
struct buf;	/* dummy for function prototype in <sys/buf.h> */
struct uio;	/* dummy for function prototype in <sys/buf.h> */
#  endif	/* defined(NETBSDV && __NetBSD_Version__<399001100) */

#include <sys/ucred.h>
# endif	/* (defined(OPENBSDV) && OPENBSDV>=3030)
	   || (defined(NETBSDV) && __NetBSD_Version__>=106060000) */

# if	defined(NETBSDV) && __NetBSD_Version__<399001100
#include <sys/buf.h>
# endif	/* defined(NETBSDV) && __NetBSD_Version__<399001100 */

# if	(defined(OPENBSDV) && OPENBSDV>=3030) \
  ||	(defined(NETBSDV) && __NetBSD_Version__>=106060000 \
  &&	__NetBSD_Version__<399001100)
#undef	_KERNEL
# endif	/* (defined(OPENBSDV) && OPENBSDV>=3030) \
	   || (defined(NETBSDV) && __NetBSD_Version__>=106060000 \
	   &&  __NetBSD_Version__<399001100) */

#define	NFS
#define m_stat	mnt_stat

# if	(defined(OPENBSDV) && OPENBSDV>=3030) \
  ||	(defined(NETBSDV) && __NetBSD_Version__>=106060000)
#define	_KERNEL
# endif	/* (defined(OPENBSDV) && OPENBSDV<3030)
	   || (defined(NETBSDV) && __NetBSD_Version__>=106060000) */

#include <sys/mount.h>

# if	(defined(OPENBSDV) && OPENBSDV>=3030) \
  ||	(defined(NETBSDV) && __NetBSD_Version__>=106060000)
#undef	_KERNEL
# endif	/* (defined(OPENBSDV) && OPENBSDV>=3030)
	   || (defined(NETBSDV) && __NetBSD_Version__>=106060000) */

#include <rpc/types.h>
#include <sys/protosw.h>

# if	defined(NETBSDV) && NETBSDV>=1003000
#define	sockproto	NETBSD_sockproto
# endif	/* defined(NETBSDV) && NETBSDV>=1003000 */

#include <sys/socket.h>

# if	defined(HASMSDOSFS)
#  if	HASMSDOSFS==1
#include <msdosfs/bpb.h>
#include <msdosfs/fat.h>
#  else	/* HASMSDOSFS!=1 */
#include <fs/msdosfs/bpb.h>
#include <fs/msdosfs/fat.h>
#  endif	/* HASMSDOSFS==1 */

#  if	(defined(OPENBSDV) && OPENBSDV<3030) \
   ||	(defined(NETBSDV) && __NetBSD_Version__<106060000)
/*
 * The netcred and netexport structures may be needed in the msdosfsmount
 * structure, defined in <msdosfs/msdosfsmount.h>.  So as a terrible hack,
 * the lsof Configure script extracts the netcred and netexport structure
 * definitions from <sys/mount.h> and places them in "netexport.h".
 *
 * When needed, the netcred and netexport structures netcred should really
 * be obtained from <sys/mount.h>.  However they are hidden in <sys/mount.h>
 * under _KERNEL, and that sometimes can't be defined when including
 * <sys/mount.h> without causing other seemingly insurmountable #include
 * problems.
 *
 * THIS IS A TERRIBLE AND FRAGILE HACK!!!  It might break if the netexport or
 * netcred definitions change radically in <sys/mount.h>.
 *
 * It is no longer needed for NetBSD Versions 1.6F and above, or for OpenBSD
 * versions 3.3 and above.
 */

#include "netexport.h"
#  endif	/* (defined(OPENBSDV) && OPENBSDV<3030)
		   || (defined(NETBSDV) && __NetBSD_Version__<106060000) */

#define	_KERNEL
struct nameidata;	/* to satisfy a function prototype in msdosfsmount.h */
#include <msdosfs/msdosfsmount.h>
#undef	_KERNEL
#include <msdosfs/direntry.h>
#include <msdosfs/denode.h>
# endif	/* defined(HASMSDOSFS) */

# if	defined(NETBSDV) && NETBSDV>=1003000
#undef	sockproto
# endif	/* defined(NETBSDV) && NETBSDV>=1003000 */

#include <sys/socketvar.h>
#include <sys/un.h>
#include <sys/unpcb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

# if	defined(HASIPv6) && defined(NETBSDV) && !defined(HASINRIAIPv6)
#include <netinet/ip6.h>
#include <netinet6/in6_pcb.h>
# endif	/* defined(HASIPv6) && defined(NETBSDV) && !defined(HASINRIAIPv6) */

#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

# if	defined(OPENBSDV)
#  if	!defined(TF_ECN_PERMIT)
#define	TF_ECN_PERMIT	0x00008000	/* other side said I could ECN */
#  endif	/* !defined(TF_ECN_PERMIT) */

#  if	!defined(TF_RCVD_CE)
#define	TF_RCVD_CE	0x00010000	/* send ECE in subsequent segs */ 
#  endif	/* !defined(TF_RCVD_CE) */

#  if	!defined(TF_SEND_CWR)
#define	TF_SEND_CWR	0x00020000	/* send CWR in next seg */
#  endif	/* !defined(TF_SEND_CWR) */

#  if	!defined(TF_DISABLE_ECN)
#define	TF_DISABLE_ECN	0x00040000	/* disable ECN for this connection */
#  endif	/* !defined(TF_DISABLE_ECN) */

# endif	/* defined(OPENBSDV) */

#include <sys/ucred.h>

# if	defined(UVM)
/*
 * Avoid conflicts with definitions in <vm/vm_param.h>.
 */

#undef	FALSE
#undef	TRUE
# endif	/* defined(UVM) */

#include <sys/vnode.h>

# if	defined(NETBSDV) && NETBSDV>=1003000
/*
 * Because late in the 1.3I NetBSD development cycle the sockproto structure
 * was placed under _KERNEL in <sys/socket.h>, and because defining _KERNEL
 * before #include'ing <sys/socket.h> causes other #include problems, the
 * sockproto structure definition that might have been in <sys/socket.h> is
 * renamed NETBSD_sockproto, and the following definition is used instead.
 *
 * Ugly, isn't it?
 */

struct sockproto {
	u_short sp_family;
	u_short sp_protocol;
};
# endif	/* defined(NETBSDV) && NETBSDV>=1003000 */

#include <net/raw_cb.h>
#include <sys/domain.h>
#define	pmap	RPC_pmap
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#undef 	pmap
#define KERNEL
#include <ufs/ufs/quota.h>
# if	defined(DIRBLKSIZ)
#define	DIRENT_DIRBLKSIZ	DIRBLKSIZ
#undef	DIRBLKSIZ
# endif	/* defined(DIRBLKSIZ) */

# if	defined(HASI_FFS1)
#define	_KERNEL
#include <ufs/ufs/ufsmount.h>
#undef	_KERNEL
# endif	/* defined(HASI_FFS1) */

#include <ufs/ufs/inode.h>

# if	defined(DIRENT_BLKSIZ)
#define	DIRBLKSIZ	DIRENT_DIRBLKSIZ
#undef	DIRENT_DIRBLKSIZ
# endif	/*defined(DIRENT_BLKSIZ) */

# if	defined(HASBUFQ_H)
#  if	defined(NETBSDV) && NETBSDV>=2099010
#define	_KERNEL
#include <sys/bufq.h>
#undef	_KERNEL
#  endif	/* defined(NETBSDV) && NETBSDV>=2099010 */
#endif	/* defined(HASBUFQ_H) */

#undef KERNEL
#include <ufs/mfs/mfsnode.h>

# if	defined(HASNFSPROTO)
#include <nfs/rpcv2.h>
#include <nfs/nfsproto.h>
# else	/* !defined(HASNFSPROTO) */
#include <nfs/nfsv2.h>
# endif	/* defined(HASNFSPROTO) */

#include <nfs/nfs.h>
#include <nfs/nfsnode.h>
#include <sys/proc.h>
#include <kvm.h>
#include <sys/sysctl.h>
#  if	defined(HASKVMGETPROC2)
#define	P_ADDR		p_paddr
#define	P_COMM		p_comm
#define	P_CWDI		p_cwdi
#define	P_FD		p_fd
#define	P_PID		p_pid
#define	P_PGID		p__pgid
#define	P_PPID		p_ppid
#define	P_STAT		p_stat
#define	P_TRACEP	p_tracep
#define	P_UID		p_uid
#define	P_VMSPACE	p_vmspace
#  else	/* !defined(HASKVMGETPROC2) */
#define	P_ADDR		kp_eproc.e_paddr
#define	P_COMM		kp_proc.p_comm
#define	P_CWDI		kp_proc.p_cwdi
#define	P_FD		kp_proc.p_fd
#define	P_PID		kp_proc.p_pid
#define	P_PGID		kp_eproc.e_pgid
#define	P_PPID		kp_eproc.e_ppid
#define	P_STAT		kp_proc.p_stat
#define	P_TRACEP	kp_proc.p_tracep
#define	P_UID		kp_eproc.e_ucred.cr_uid
#define	P_VMSPACE	kp_proc.p_vmspace
#  endif	/* defined(HASKVMGETPROC2) */

# if	defined(HASFDESCFS)
#define	_KERNEL
#include <miscfs/fdesc/fdesc.h>
#undef	_KERNEL
# endif	/* defined(HASFDESCFS) */

# if	defined(HASKERNFS)
#define	_KERNEL
#define	Pkern		__Pkern
#define	Proot		__Proot
#define	Pnull		__Pnull
#define	Ptime		__Ptime
#define	Pint		__Pint
#define	Pstring		__Pstring
#define	Phostname	__Phostname
#define	Pavenrun	__Pavenrun
#define	Pdevice		__Pdevice
#define	Pmsgbuf		__Pmsgbuf
#define	Pipsecsadir	__Pipsecsadir
#define	Pipsecspdir	__Pipsecspdir
#define	Pipsecsa	__Pipseca
#define	Pipsecsp	__Pipsecsp
#include <miscfs/kernfs/kernfs.h>
#undef	_KERNEL
#undef	Pkern
#undef	Proot
#undef	Pnull
#undef	Ptime
#undef	Pint
#undef	Pstring
#undef	Phostname
#undef	Pavenrun
#undef	Pdevice
#undef	Pmsgbuf
#undef	Pipsecsadir
#undef	Pipsecspdir
#undef	Pipseca
#undef	Pipsecsp
#  if	defined(HASKERNFS_KFS_KT)
#define	kf_kt	kfs_kt
#  endif	/* defined(HASKERNFS_KFS_KT) */
# endif	/* defined(HASKERNFS) */

# if	defined(HASNULLFS)
#define	_KERNEL
#  if	defined(NETBSDV) && NETBSDV>=1005000 && __NetBSD_Version__<106060000
#include "netexport.h"
#  endif	/* defined(NETBSDV) && NETBSDV>=1005000
		   && __NetBSD_Version__<106060000 */
#include <miscfs/nullfs/null.h>
#undef	_KERNEL
# endif	/* defined(HASNULLFS) */

# if	defined(HASPROCFS)
#  if	defined(HASPROCFS_PFSROOT)
#define	_KERNEL
#  endif	/* defined(HASPROCFS_PFSROOT) */
#include <miscfs/procfs/procfs.h>
#  if	defined(HASPROCFS_PFSROOT)
#undef	_KERNEL
#define	Proot		PFSroot
#define	Pproc		PFSproc
#define	Pcurproc	PFScurproc
#define	Pmem		PFSmem
#define	Pregs		PFSregs
#define	Pfile		PFSfile
#define	Pfpregs		PFSfpregs
#define	Pctl		PFSctl
#define	Pstatus		PFSstatus
#define	Pnote		PFSnote
#define	Pnotepg		PFSnotepg
#   if	defined(NetBSDV)
#    if	NETBSDV>=2000000
#define	Pfd		PFSfd
#    endif	/* NETBSDV>=2000000 */
#    if	NETBSDV>=1006000
#define	Pmap		PFSmap
#define	Pmaps		PFSmaps
#    endif	/* NETBSDV>=1006000 */
#   endif	/* defined(NetBSDV) */
#  endif	/* defined(HASPROCFS_PFSROOT) */
#include <machine/reg.h>
# endif	/* defined(HASPROCFS) */

# if	defined(HASPTYFS)
#define	_KERNEL
#include <fs/ptyfs/ptyfs.h>
#include <miscfs/specfs/specdev.h>
#undef	_KERNEL
# endif	/* defined(HASPTYFS) */

#define	KERNEL
#define _KERNEL
#include <sys/file.h>
#include <sys/fcntl.h>

# if	defined(HAS_ADVLOCK_ARGS)
struct vop_advlock_args;
# endif	/* defined(HAS_ADVLOCK_ARGS) */

# if	defined(DTYPE_KQUEUE)
#define	HASKQUEUE				/* has the kqueue file type */
#  if	defined(OPENBSDV)
#include <sys/eventvar.h>
#  endif	/* defined(OPENBSDV) */
# endif	/* defined(DTYPE_KQUEUE) */

#include <sys/lockf.h>
#undef	KERNEL
#undef	_KERNEL

# if	defined(UVM)
#  if	defined(OPENBSDV)
#define	_UVM_UVM_FAULT_I_H_	1		/* avoid OpenBSD's
						/* <uvm/uvm_fault_i.h */
#  endif	/* defined(OPENBSDV) */
#define	FALSE	0
#define	TRUE	1
#include <uvm/uvm.h>
# endif	/* defined(UVM) */

# if	defined(HAS_UVM_INCL)
#include <uvm/uvm.h>
#include <uvm/uvm_map.h>
#include <uvm/uvm_object.h>
#include <uvm/uvm_pager.h>
# else	/* !defined(HAS_UVM_INCL) */
#include <vm/vm.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <vm/vm_pager.h>
# endif	/* defined(HAS_UVM_INCL) */

# if	defined(HAS_SYS_PIPEH)
#  if	OPENBSDV==2030 && defined(__sparc__)
#   if	defined(nbpg)
#undef	nbpg
#   endif	/* defined(nbpg) */
#define	nbpg	4096		/* WARNING!!!  This should be 8192 for sun4,
				 * but there's not much chance this value will
				 * ever be used by any lsof code.  (See the
				 * use of PIPE_NODIRECT in <sys/pipe.h>. */
#  endif	/* OPENBSDV==2030 && defined(__sparc__) */
#include <sys/pipe.h>
#endif	/* defined(HAS_SYS_PIPEH) */

#define	COMP_P		const void
#define DEVINCR		1024	/* device table malloc() increment */
typedef	u_long		KA_T;
#define	KMEM		"/dev/kmem"
#define MALLOC_P	void
#define FREE_P		MALLOC_P
#define MALLOC_S	size_t

# if	!defined(MAXSYSCMDL)
#define	MAXSYSCMDL	MAXCOMLEN	/* max system command name length */
# endif	/* !defined(MAXSYSCMDL) */

# if	defined(N_UNIXV)
#define	N_UNIX_TMP(x)	#x
#define	N_UNIX_STR(x)	N_UNIX_TMP(x)
#define	N_UNIX		N_UNIX_STR(N_UNIXV)
# endif	/* defined(N_UNIXV) */

#define QSORT_P		void
#define	READLEN_T	int
#define STRNCPY_L	size_t
#define SWAP		"/dev/drum"
#define	SZOFFTYPE	unsigned long long
					/* size and offset internal storage
					 * type */
#define	SZOFFPSPEC	"ll"		/* SZOFFTYPE print specification
					 * modifier */


/*
 * Global storage definitions (including their structure definitions)
 */

extern struct file *Cfp;
extern kvm_t *Kd;
extern KA_T Kpa;

struct l_vfs {
	KA_T addr;			/* kernel address */
	fsid_t	fsid;			/* file system ID */
	char type[MFSNAMELEN];		/* type of file system */
	char *dir;			/* mounted directory */
	char *fsname;			/* file system name */
	struct l_vfs *next;		/* forward link */
};
extern struct l_vfs *Lvfs;

struct mounts {
        char *dir;              	/* directory (mounted on) */
	char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;           	/* file system
					 * (symbolic links resolved) */
        dev_t dev;              	/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory st_ino */
	mode_t mode;			/* directory st_mode */
	mode_t fs_mode;			/* file_system st_mode */
        struct mounts *next;    	/* forward link */
};

#define	X_NCACHE	"ncache"
#define	X_NCSIZE	"ncsize"
#define	NL_NAME		n_name

extern int Np;				/* number of kernel processes */

# if	defined(HASKVMGETPROC2) 
struct kinfo_proc2 *P;			/* local process table copy */
# else	/* ! defined(HASKVMGETPROC2) */
struct kinfo_proc *P;			/* local process table copy */
# endif	/* defined(HASKVMGETPROC2) */

extern int pgshift;			/* kernel's page shift */

struct sfile {
	char *aname;			/* argument file name */
	char *name;			/* file name (after readlink()) */
	char *devnm;			/* device name (optional) */
	dev_t dev;			/* device */
	dev_t rdev;			/* raw device */
	u_short mode;			/* S_IFMT mode bits from stat() */
	int type;			/* file type: 0 = file system
				 	 *	      1 = regular file */
	INODETYPE i;			/* inode number */
	int f;				/* file found flag */
	struct sfile *next;		/* forward link */

};


/*
 * Definitions for rdev.c
 */

#define	DIRTYPE	dirent
#define HASDNAMLEN	1	/* struct DIRTYPE has d_namlen element */


/*
 * Definitions for rnam.c and rnmh.c
 */

# if     defined(HASNCACHE)
#  if	(defined(OPENBSDV) && OPENBSDV>=2010) || (defined(NETBSDV) && NETBSDV>=1002000)
#include <stddef.h>
#endif	/* (defined(OPENBSDV) && OPENBSDV>=2010) || (defined(NETBSDV) && NETBSDV>=1002000) */

#include <sys/uio.h>
#include <sys/namei.h>
#define	NCACHE		namecache	/* kernel's structure name */
#define	NCACHE_NM	nc_name		/* name in NCACHE */
#define	NCACHE_NMLEN	nc_nlen		/* name length in NCACHE */
#define	NCACHE_NODEADDR	nc_vp		/* node address in NCACHE */
#define	NCACHE_PARADDR	nc_dvp		/* parent node address in NCACHE */

#  if	(defined(OPENBSDV) && OPENBSDV>=2010) || (defined(NETBSDV) && NETBSDV>=1002000)
#define	NCACHE_NXT	nc_hash.le_next	/* link in NCACHE */
#  else	/* (defined(OPENBSDV) && OPENBSDV>=2010) || (defined(NETBSDV) && NETBSDV>=1002000) */
#   if	defined(NetBSD1_0) && NetBSD<1994101
#define	NCACHE_NXT	nc_nxt		/* link in NCACHE */
#   else	/* !defined(NetBSD1_0) || NetBSD>=1994101 */
#define	NCACHE_NXT	nc_lru.tqe_next	/* link in NCACHE */
#   endif	/* defined(NetBSD1_0) && NetBSD<1994101 */
#  endif	/* (defined(OPENBSDV) && OPENBSDV>=2010) || (defined(NETBSDV) && NETBSDV>=1002000) */

#  if	defined(HASNCVPID)
#define	NCACHE_PARID	nc_dvpid	/* parent node ID in NCACHE */
#define	NCACHE_NODEID	nc_vpid		/* node ID in NCACHE */
#  endif	/* defined(HASNCVPID) */
# endif  /* defined(HASNCACHE) */

#endif	/* NETBSD_LSOF_H */
