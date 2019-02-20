#ifndef _NFS_NFS_H
#define _NFS_NFS_H

#include <config.h>

#include <linux/posix_types.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <rpcsvc/nfs_prot.h>
#include <nfs/export.h>
#include <limits.h>

#define	NFS3_FHSIZE	64
#define	NFS_FHSIZE	32

#define NFSD_MINVERS 2
#define NFSD_MAXVERS 4

#define NFS4_MINMINOR 0
#define NFS4_MAXMINOR (WORD_BIT-1)

struct nfs_fh_len {
	int		fh_size;
	u_int8_t	fh_handle[NFS3_FHSIZE];
};


#define NFSCTL_UDPBIT		      (1 << (17 - 1))
#define NFSCTL_TCPBIT		      (1 << (18 - 1))
#define NFSCTL_PROTODEFAULT	      (NFSCTL_TCPBIT)

#define NFSCTL_VERUNSET(_cltbits, _v) ((_cltbits) &= ~(1 << ((_v) - 1))) 
#define NFSCTL_MINORUNSET(_cltbits, _v) ((_cltbits) &= ~(1 << (_v)))
#define NFSCTL_UDPUNSET(_cltbits)     ((_cltbits) &= ~NFSCTL_UDPBIT) 
#define NFSCTL_TCPUNSET(_cltbits)     ((_cltbits) &= ~NFSCTL_TCPBIT) 

#define NFSCTL_VERISSET(_cltbits, _v) ((_cltbits) & (1 << ((_v) - 1))) 
#define NFSCTL_MINORISSET(_cltbits, _v) ((_cltbits) & (1 << (_v)))
#define NFSCTL_UDPISSET(_cltbits)     ((_cltbits) & NFSCTL_UDPBIT) 
#define NFSCTL_TCPISSET(_cltbits)     ((_cltbits) & NFSCTL_TCPBIT) 

#define NFSCTL_VERDEFAULT (0xc)       /* versions 3 and 4 */
#define NFSCTL_MINDEFAULT (0x7)       /* minor versions 4.1 and 4.2 */
#define NFSCTL_VERSET(_cltbits, _v)   ((_cltbits) |= (1 << ((_v) - 1))) 
#define NFSCTL_MINORSET(_cltbits, _v)   ((_cltbits) |= (1 << (_v)))
#define NFSCTL_UDPSET(_cltbits)       ((_cltbits) |= NFSCTL_UDPBIT)
#define NFSCTL_TCPSET(_cltbits)       ((_cltbits) |= NFSCTL_TCPBIT)

#define NFSCTL_ANYPROTO(_cltbits)     ((_cltbits) & (NFSCTL_UDPBIT | NFSCTL_TCPBIT))

#endif /* _NFS_NFS_H */
