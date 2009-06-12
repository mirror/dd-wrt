//==========================================================================
//
//      lib/shutdown.c
//
//      shutdown() system call
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
shutdown(int s, int how)
{
    struct sys_shutdown_args args;
    int res, error;
    SYSCALLARG(args,s) = s;
    SYSCALLARG(args,how) = how;
    error = sys_shutdown(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return 0;
    }
}
