//==========================================================================
//
//      sys/kern/sockio.c
//
//     Socket interface to Fileio subsystem
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg
// Date:         2000-06-06
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

/*
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
//==========================================================================

#include <pkgconf/net.h>
#include <pkgconf/io_fileio.h>

#include <sys/types.h>

#include <cyg/io/file.h>

#include <cyg/fileio/fileio.h>
#include <cyg/fileio/sockio.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/route.h>

//==========================================================================
// Forward definitions

static int     bsd_init( cyg_nstab_entry *nste );
static int     bsd_socket( cyg_nstab_entry *nste, int domain, int type,
		       int protocol, cyg_file *file );

static int bsd_bind      ( cyg_file *fp, const sockaddr *sa, socklen_t len );
static int bsd_connect   ( cyg_file *fp, const sockaddr *sa, socklen_t len );
static int bsd_accept    ( cyg_file *fp, cyg_file *new_fp,
                           struct sockaddr *name, socklen_t *anamelen );
static int bsd_listen    ( cyg_file *fp, int len );
static int bsd_getname   ( cyg_file *fp, sockaddr *sa, socklen_t *len, int peer );
static int bsd_shutdown  ( cyg_file *fp, int flags );
static int bsd_getsockopt( cyg_file *fp, int level, int optname,
                           void *optval, socklen_t *optlen);
static int bsd_setsockopt( cyg_file *fp, int level, int optname,
                           const void *optval, socklen_t optlen);
static int bsd_sendmsg   ( cyg_file *fp, const struct msghdr *m,
                               int flags, ssize_t *retsize );
static int bsd_recvmsg   ( cyg_file *fp, struct msghdr *m,
                               socklen_t *namelen, ssize_t *retsize );


// File operations
static int bsd_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int bsd_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int bsd_lseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );
static int bsd_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data);
static int bsd_select    (struct CYG_FILE_TAG *fp, int which, CYG_ADDRWORD info);
static int bsd_fsync     (struct CYG_FILE_TAG *fp, int mode );        
static int bsd_close     (struct CYG_FILE_TAG *fp);
static int bsd_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf );
static int bsd_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );
static int bsd_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );

static int
bsd_recvit(cyg_file *fp, struct msghdr *mp, socklen_t *namelenp, ssize_t *retsize);
static int
bsd_sendit(cyg_file *fp, const struct msghdr *mp, int flags, ssize_t *retsize);


//==========================================================================
// Table entrys

NSTAB_ENTRY( bsd_nste, 0,
             "bsd_tcpip",
             "",
             0,
             bsd_init,
             bsd_socket);

struct cyg_sock_ops bsd_sockops =
{
    bsd_bind,
    bsd_connect,
    bsd_accept,
    bsd_listen,
    bsd_getname,
    bsd_shutdown,
    bsd_getsockopt,
    bsd_setsockopt,
    bsd_sendmsg,
    bsd_recvmsg
};

cyg_fileops bsd_sock_fileops =
{
    bsd_read,
    bsd_write,
    bsd_lseek,
    bsd_ioctl,
    bsd_select,
    bsd_fsync,
    bsd_close,
    bsd_fstat,
    bsd_getinfo,
    bsd_setinfo    
};

//==========================================================================
// NStab functions



// -------------------------------------------------------------------------

static int     bsd_init( cyg_nstab_entry *nste )
{
    // Initialization already handled via constructor
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int     bsd_socket( cyg_nstab_entry *nste, int domain, int type,
		       int protocol, cyg_file *file )
{
    int error = 0;
    struct socket *so;

    error = socreate( domain, &so, type, protocol );

    if( error == ENOERR )
    {

        cyg_selinit(&so->so_rcv.sb_sel);
        cyg_selinit(&so->so_snd.sb_sel);
        
        file->f_flag   |= CYG_FREAD|CYG_FWRITE;
        file->f_type    = CYG_FILE_TYPE_SOCKET;
        file->f_ops     = &bsd_sock_fileops;
        file->f_offset  = 0;
        file->f_data    = (CYG_ADDRWORD)so;
        file->f_xops    = (CYG_ADDRWORD)&bsd_sockops;
    }
    
    return error;
}


//==========================================================================
// Sockops functions

// -------------------------------------------------------------------------

static int bsd_bind      ( cyg_file *fp, const sockaddr *sa, socklen_t len )
{
    struct mbuf *nam;
    int error;

    error = sockargs(&nam, (caddr_t)sa, len, MT_SONAME);

    if (error)
        return (error);

    error = sobind((struct socket *)fp->f_data, nam);

    m_freem(nam);
    
    return error;
}

// -------------------------------------------------------------------------

static int bsd_connect   ( cyg_file *fp, const sockaddr *sa, socklen_t len )
{
    register struct socket *so;
    struct mbuf *nam;
    int error, s;

    so = (struct socket *)fp->f_data;

    if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING))
        return (EALREADY);

    error = sockargs(&nam, (caddr_t)sa, len, MT_SONAME);

    if (error)
        return (error);

    error = soconnect(so, nam);
    if (error)
        goto bad;

    if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
        m_freem(nam);
        return (EINPROGRESS);
    }

    s = splsoftnet();
    while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {
        error = tsleep((caddr_t)&so->so_timeo, PSOCK | PCATCH,
                       netcon, 0);
        if (error)
            break;
    }

    if (error == 0) {
        error = so->so_error;
        so->so_error = 0;
    }

    splx(s);

bad:
    so->so_state &= ~SS_ISCONNECTING;
    m_freem(nam);

    return error;
}

// -------------------------------------------------------------------------

static int bsd_accept    ( cyg_file *fp, cyg_file *new_fp,
                           struct sockaddr *name, socklen_t *anamelen )
{
    struct mbuf *nam;
    socklen_t namelen = 0;
    int error = 0, s;
    register struct socket *so;

    if( anamelen != NULL )
        namelen = *anamelen;

    s = splsoftnet();
    so = (struct socket *)fp->f_data;

    if ((so->so_options & SO_ACCEPTCONN) == 0) {
        splx(s);
        return (EINVAL);
    }

    if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
        splx(s);
        return (EWOULDBLOCK);
    }

    while (so->so_qlen == 0 && so->so_error == 0) {
        if (so->so_state & SS_CANTRCVMORE) {
            so->so_error = ECONNABORTED;
            break;
        }
        error = tsleep((caddr_t)&so->so_timeo, PSOCK | PCATCH,
                       netcon, 0);
        if (error) {
            splx(s);
            return (error);
        }
    }

    if (so->so_error) {
        error = so->so_error;
        so->so_error = 0;
        splx(s);
        return (error);
    }

    {
        struct socket *aso = so->so_q;
        if (soqremque(aso, 1) == 0)
            panic("accept");
        so = aso;
    }

    cyg_selinit(&so->so_rcv.sb_sel);
    cyg_selinit(&so->so_snd.sb_sel);
    
    new_fp->f_type      = DTYPE_SOCKET;
    new_fp->f_flag     |= FREAD|FWRITE;
    new_fp->f_offset    = 0;
    new_fp->f_ops       = &bsd_sock_fileops;
    new_fp->f_data      = (CYG_ADDRWORD)so;
    new_fp->f_xops      = (CYG_ADDRWORD)&bsd_sockops;
    
    nam = m_get(M_WAIT, MT_SONAME);
    (void) soaccept(so, nam);
    if (name) {
        if (namelen > nam->m_len)
            namelen = nam->m_len;
        /* SHOULD COPY OUT A CHAIN HERE */
        if ((error = copyout(mtod(nam, caddr_t),
                             (caddr_t)name, namelen)) == 0)
            *anamelen = namelen;
    }
    m_freem(nam);
    splx(s);
    
    return (error);
}

// -------------------------------------------------------------------------

static int bsd_listen    ( cyg_file *fp, int backlog )
{
    return (solisten((struct socket *)fp->f_data, backlog));
}

// -------------------------------------------------------------------------

static int bsd_getname   ( cyg_file *fp, sockaddr *asa, socklen_t *alen, int peer )
{
    register struct socket *so;
    struct mbuf *m;
    socklen_t len = 0;
    int error;
    int type = peer ? PRU_PEERADDR : PRU_SOCKADDR;
    
    if( alen != NULL )
        len = *alen;
        
    so = (struct socket *)fp->f_data;

    if ( peer && (so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0)
        return (ENOTCONN);

    m = m_getclr(M_WAIT, MT_SONAME);
    if (m == NULL)
        return (ENOBUFS);
    
    error = (*so->so_proto->pr_usrreq)(so, type, 0, m, 0);
    if (error)
        goto bad;

    if (len > m->m_len)
        len = m->m_len;

    error = copyout(mtod(m, caddr_t), (caddr_t)asa, len);
    
    if (error == 0)
        *alen = len;

bad:
    m_freem(m);

    return (error);
}

// -------------------------------------------------------------------------

static int bsd_shutdown  ( cyg_file *fp, int how )
{
    return (soshutdown((struct socket *)fp->f_data, how));
}

// -------------------------------------------------------------------------

static int bsd_getsockopt( cyg_file *fp, int level, int optname,
                           void *optval, socklen_t *optlen)
{
    struct mbuf *m = NULL;
    socklen_t valsize = 0;
    int error;

    if( optval != NULL && optlen != NULL )
        valsize = *optlen;
    
    error = sogetopt((struct socket *)fp->f_data, level, optname, &m);

    if( error == ENOERR && valsize != 0 && m != NULL)
    {
        if (valsize > m->m_len)
            valsize = m->m_len;

        error = copyout(mtod(m, caddr_t), optval, valsize);

        if( error == ENOERR )
            *optlen = valsize;
        
    }

    if (m != NULL)
        (void) m_free(m);
    return (error);
}

// -------------------------------------------------------------------------

static int bsd_setsockopt( cyg_file *fp, int level, int optname,
                           const void *optval, socklen_t optlen)
{
    int error;
    struct mbuf *m = NULL;
    
    if( optlen > MCLBYTES )
        return EINVAL;

    if (optval != NULL) {
        m = m_get(M_WAIT, MT_SOOPTS);
        if (optlen > MLEN) {
            MCLGET(m, M_DONTWAIT);
            if ((m->m_flags & M_EXT) == 0) {
                m_freem(m);
                return (ENOBUFS);
            }
        }
        if (m == NULL)
            return (ENOBUFS);
        error = copyin(optval, mtod(m, caddr_t), optlen);
        if (error) {
            (void) m_free(m);
            return (error);
        }
        m->m_len = optlen;
    }
    
    return (sosetopt((struct socket *)fp->f_data, level, optname, m));
}

// -------------------------------------------------------------------------

static int bsd_sendmsg   ( cyg_file *fp, const struct msghdr *m, int flags, ssize_t *retsize )
{
    return bsd_sendit( fp, m, flags, retsize);
}

// -------------------------------------------------------------------------

static int bsd_recvmsg   ( cyg_file *fp, struct msghdr *m, socklen_t *namelen, ssize_t *retsize )
{
    return bsd_recvit( fp, m, namelen, retsize);
}

//==========================================================================
// File system call functions

// -------------------------------------------------------------------------

static int bsd_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    return (soreceive((struct socket *)fp->f_data, (struct mbuf **)0,
                      uio, (struct mbuf **)0, (struct mbuf **)0, (int *)0));
}

// -------------------------------------------------------------------------

static int bsd_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
	return (sosend((struct socket *)fp->f_data, (struct mbuf *)0,
		uio, (struct mbuf *)0, (struct mbuf *)0, 0));
}

// -------------------------------------------------------------------------

static int bsd_lseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence )
{
    return ESPIPE;
}

// -------------------------------------------------------------------------

static int bsd_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD cmd,
                                CYG_ADDRWORD data)
{
	register struct socket *so = (struct socket *)fp->f_data;
        void *p = 0;

	switch (cmd) {

	case FIONBIO:
		if (*(int *)data)
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		return (0);

	case FIOASYNC:
		if (*(int *)data) {
			so->so_state |= SS_ASYNC;
			so->so_rcv.sb_flags |= SB_ASYNC;
			so->so_snd.sb_flags |= SB_ASYNC;
		} else {
			so->so_state &= ~SS_ASYNC;
			so->so_rcv.sb_flags &= ~SB_ASYNC;
			so->so_snd.sb_flags &= ~SB_ASYNC;
		}
		return (0);

	case FIONREAD:
		*(int *)data = so->so_rcv.sb_cc;
		return (0);

	case SIOCATMARK:
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
		return (0);
	}
	/*
	 * Interface/routing/protocol specific ioctls:
	 * interface and routing ioctls should have a
	 * different entry since a socket's unnecessary
	 */
	if (IOCGROUP(cmd) == 'i')
		return (ifioctl(so, (u_long)cmd, (caddr_t)data, p));
	if (IOCGROUP(cmd) == 'r')
		return (rtioctl((u_long)cmd, (caddr_t)data, p));
	return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
	    (struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0));

}

// -------------------------------------------------------------------------

static int bsd_select    (struct CYG_FILE_TAG *fp, int which, CYG_ADDRWORD info)
{
    register struct socket *so = (struct socket *)fp->f_data;
    register int s = splsoftnet();

    switch (which) {

    case FREAD:
        if (soreadable(so)) {
            splx(s);
            return (1);
        }
        cyg_selrecord(info, &so->so_rcv.sb_sel);
        so->so_rcv.sb_flags |= SB_SEL;
        break;

    case FWRITE:
        if (sowriteable(so)) {
            splx(s);
            return (1);
        }
        cyg_selrecord(info, &so->so_snd.sb_sel);
        so->so_snd.sb_flags |= SB_SEL;
        break;

    case 0:
        if (so->so_oobmark || (so->so_state & SS_RCVATMARK)) {
            splx(s);
            return (1);
        }
        cyg_selrecord(info, &so->so_rcv.sb_sel);
        so->so_rcv.sb_flags |= SB_SEL;
        break;
    }
    splx(s);
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int bsd_fsync     (struct CYG_FILE_TAG *fp, int mode )
{
    // FIXME: call some sort of flush IOCTL?
    return 0;
}

// -------------------------------------------------------------------------

static int bsd_close     (struct CYG_FILE_TAG *fp)
{
    int error = 0;

    if (fp->f_data)
        error = soclose((struct socket *)fp->f_data);
    fp->f_data = 0;
    return (error);
}

// -------------------------------------------------------------------------

static int bsd_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf )
{
    register struct socket *so = (struct socket *)fp->f_data;

    bzero((caddr_t)buf, sizeof (*buf));

    // Mark socket as a fifo for now. We need to add socket types to
    // sys/stat.h.
    buf->st_mode = __stat_mode_FIFO;
    
    return ((*so->so_proto->pr_usrreq)(so, PRU_SENSE,
                                       (struct mbuf *)buf,
                                       (struct mbuf *)0, 
                                       (struct mbuf *)0));
}

// -------------------------------------------------------------------------

static int bsd_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    return ENOSYS;
}

// -------------------------------------------------------------------------

static int bsd_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    return ENOSYS;
}



//==========================================================================
// Select support

// -------------------------------------------------------------------------
// This function is called by the lower layers to record the
// fact that a particular 'select' event is being requested.
//

void        
selrecord(void *selector, struct selinfo *info)
{
    // Unused by this implementation
}

// -------------------------------------------------------------------------
// This function is called to indicate that a 'select' event
// may have occurred.
//

void    
selwakeup(struct selinfo *info)
{
    cyg_selwakeup( info );
}

//==========================================================================
// Misc support functions

int
sockargs(mp, buf, buflen, type)
	struct mbuf **mp;
	caddr_t buf;
	socklen_t buflen;
	int type;
{
	register struct sockaddr *sa;
	register struct mbuf *m;
	int error;

	if (buflen > MLEN) {
#ifdef COMPAT_OLDSOCK
		if (type == MT_SONAME && buflen <= 112)
			buflen = MLEN;		/* unix domain compat. hack */
		else
#endif
		return (EINVAL);
	}
	m = m_get(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = buflen;
	error = copyin(buf, mtod(m, caddr_t), buflen);
	if (error) {
		(void) m_free(m);
		return (error);
	}
	*mp = m;
	if (type == MT_SONAME) {
		sa = mtod(m, struct sockaddr *);

#if defined(COMPAT_OLDSOCK) && BYTE_ORDER != BIG_ENDIAN
		if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
			sa->sa_family = sa->sa_len;
#endif
		sa->sa_len = buflen;
	}
	return (0);
}


// -------------------------------------------------------------------------
// bsd_recvit()
// Support for message reception. This is a lightly edited version of the
// recvit() function is uipc_syscalls.c.

static int
bsd_recvit(cyg_file *fp, struct msghdr *mp, socklen_t *namelenp, ssize_t *retsize)
{
        struct uio auio;
	register struct iovec *iov;
	register int i;
	size_t len;
	int error;
	struct mbuf *from = 0, *control = 0;

	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		/* Don't allow sum > SSIZE_MAX */
		if (iov->iov_len > SSIZE_MAX ||
		    (auio.uio_resid += iov->iov_len) > SSIZE_MAX)
			return (EINVAL);
	}

	len = auio.uio_resid;
	error = soreceive((struct socket *)fp->f_data, &from, &auio,
			  NULL, mp->msg_control ? &control : NULL,
			  &mp->msg_flags);
	if (error) {
		if (auio.uio_resid != len && 
                    (error == EINTR || error == EWOULDBLOCK))
			error = 0;
	}
	if (error)
		goto out;
	*retsize = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
		        /* save sa_len before it is destroyed by MSG_COMPAT */
			if (len > from->m_len)
				len = from->m_len;
			/* else if len < from->m_len ??? */
#ifdef COMPAT_OLDSOCK
			if (mp->msg_flags & MSG_COMPAT)
				mtod(from, struct osockaddr *)->sa_family =
				    mtod(from, struct sockaddr *)->sa_family;
#endif
			error = copyout(mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len);
			if (error)
				goto out;
		}
		mp->msg_namelen = len;
		if (namelenp ) {
                        *namelenp = len;
#ifdef COMPAT_OLDSOCK
			if (mp->msg_flags & MSG_COMPAT)
				error = 0;	/* old recvfrom didn't check */
			else
#endif
			goto out;
		}
	}
	if (mp->msg_control) {
#ifdef COMPAT_OLDSOCK
		/*
		 * We assume that old recvmsg calls won't receive access
		 * rights and other control info, esp. as control info
		 * is always optional and those options didn't exist in 4.3.
		 * If we receive rights, trim the cmsghdr; anything else
		 * is tossed.
		 */
		if (control && mp->msg_flags & MSG_COMPAT) {
			if (mtod(control, struct cmsghdr *)->cmsg_level !=
			    SOL_SOCKET ||
			    mtod(control, struct cmsghdr *)->cmsg_type !=
			    SCM_RIGHTS) {
				mp->msg_controllen = 0;
				goto out;
			}
			control->m_len -= sizeof (struct cmsghdr);
			control->m_data += sizeof (struct cmsghdr);
		}
#endif
		len = mp->msg_controllen;
		if (len <= 0 || control == 0)
			len = 0;
		else {
			struct mbuf *m = control;
			caddr_t p = (caddr_t)mp->msg_control;

			do {
				i = m->m_len;
				if (len < i) {
					mp->msg_flags |= MSG_CTRUNC;
					i = len;
				}
				error = copyout(mtod(m, caddr_t), p,
				    (unsigned)i);
				if (m->m_next)
					i = ALIGN(i);
				p += i;
				len -= i;
				if (error != 0 || len <= 0)
					break;
			} while ((m = m->m_next) != NULL);
			len = p - (caddr_t)mp->msg_control;
		}
		mp->msg_controllen = len;
	}
out:
	if (from)
		m_freem(from);
	if (control)
		m_freem(control);
	return (error);
}

// -------------------------------------------------------------------------
// sendit()
// Support for message transmission. This is a lightly edited version of the
// synonymous function is uipc_syscalls.c.

static int
bsd_sendit(cyg_file *fp, const struct msghdr *mp, int flags, ssize_t *retsize)
{
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *control;
	int len, error;
	
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		/* Don't allow sum > SSIZE_MAX */
		if (iov->iov_len > SSIZE_MAX ||
		    (auio.uio_resid += iov->iov_len) > SSIZE_MAX)
			return (EINVAL);
	}
	if (mp->msg_name) {
		error = sockargs(&to, mp->msg_name, mp->msg_namelen,
				 MT_SONAME);
		if (error)
			return (error);
	} else
		to = 0;
	if (mp->msg_control) {
		if (mp->msg_controllen < sizeof(struct cmsghdr)
#ifdef COMPAT_OLDSOCK
		    && mp->msg_flags != MSG_COMPAT
#endif
		) {
			error = EINVAL;
			goto bad;
		}
		error = sockargs(&control, mp->msg_control,
				 mp->msg_controllen, MT_CONTROL);
		if (error)
			goto bad;
#ifdef COMPAT_OLDSOCK
		if (mp->msg_flags == MSG_COMPAT) {
			register struct cmsghdr *cm;

			M_PREPEND(control, sizeof(*cm), M_WAIT);
			if (control == 0) {
				error = ENOBUFS;
				goto bad;
			} else {
				cm = mtod(control, struct cmsghdr *);
				cm->cmsg_len = control->m_len;
				cm->cmsg_level = SOL_SOCKET;
				cm->cmsg_type = SCM_RIGHTS;
			}
		}
#endif
	} else
		control = 0;

	len = auio.uio_resid;
	error = sosend((struct socket *)fp->f_data, to, &auio,
		       NULL, control, flags);
	if (error) {
		if (auio.uio_resid != len && 
                    (error == EINTR || error == EWOULDBLOCK))
			error = 0;
#ifndef __ECOS
		if (error == EPIPE)
			psignal(p, SIGPIPE);
#endif
	}
	if (error == 0)
		*retsize = len - auio.uio_resid;
bad:
	if (to)
		m_freem(to);
	return (error);
}


//==========================================================================
// End of sockio.c
