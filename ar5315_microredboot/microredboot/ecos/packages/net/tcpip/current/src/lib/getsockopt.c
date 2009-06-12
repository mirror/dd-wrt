//==========================================================================
//
//      lib/getsockopt.c
//
//      getsockopt() system call
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
getsockopt(int s, int level, int name, void *val, socklen_t *avalsize)
{
    struct sys_getsockopt_args args;
    int res, error;
    SYSCALLARG(args,s) = s;
    SYSCALLARG(args,level) = level;
    SYSCALLARG(args,name) = name;
    SYSCALLARG(args,val) = val;
    SYSCALLARG(args,avalsize) = avalsize;
    error = sys_getsockopt(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return 0;
    }
}
