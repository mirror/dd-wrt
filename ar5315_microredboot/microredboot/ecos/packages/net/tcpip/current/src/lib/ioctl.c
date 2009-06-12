//==========================================================================
//
//      lib/ioctl.c
//
//      ioctl() system call
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

int 
ioctl(int fd, u_long cmd, void *data)
{
    struct sys_ioctl_args args;
    int res, error;
    SYSCALLARG(args,fd) = fd;
    SYSCALLARG(args,com) = cmd;
    SYSCALLARG(args,data) = data;
    error = sys_ioctl(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return 0;
    }
}
