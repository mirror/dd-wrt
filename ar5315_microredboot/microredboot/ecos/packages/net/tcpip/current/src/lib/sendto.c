//==========================================================================
//
//      lib/sendto.c
//
//      sendto() system call
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
sendto(int s, const void *buf, size_t buflen, 
       int flags, const struct sockaddr *to, socklen_t tolen)
{
    struct sys_sendto_args args;
    int res, error;
    SYSCALLARG(args,s) = s;
    SYSCALLARG(args,buf) = buf;
    SYSCALLARG(args,len) = buflen;
    SYSCALLARG(args,flags) = flags;
    SYSCALLARG(args,to) = to;
    SYSCALLARG(args,tolen) = tolen;
    error = sys_sendto(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return res;
    }
}
