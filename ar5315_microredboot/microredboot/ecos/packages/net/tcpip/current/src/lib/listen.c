//==========================================================================
//
//      lib/listen.c
//
//      listen() system call
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
listen(int s, int backlog)
{
    struct sys_listen_args args;
    int res, error;
    SYSCALLARG(args,s) = s;
    SYSCALLARG(args,backlog) = backlog;
    error = sys_listen(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return 0;
    }
}
