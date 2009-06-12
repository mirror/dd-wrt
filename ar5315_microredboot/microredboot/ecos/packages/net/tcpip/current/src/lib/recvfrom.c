//==========================================================================
//
//      lib/recvfrom.c
//
//      recvfrom() system call
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <sys/param.h>
#include <cyg/io/file.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <sys/syscallargs.h>

ssize_t	
recvfrom(int s, const void *buf, size_t buflen, 
       int flags, const struct sockaddr *from, socklen_t *fromlen)
{
    struct sys_recvfrom_args args;
    int res, error;
    SYSCALLARG(args,s) = s;
    SYSCALLARG(args,buf) = (void *)buf;
    SYSCALLARG(args,len) = buflen;
    SYSCALLARG(args,flags) = flags; 
    SYSCALLARG(args,from) = (struct sockaddr *)from;
    SYSCALLARG(args,fromlenaddr) = fromlen;
    error = sys_recvfrom(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return res;
    }
}
