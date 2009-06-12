//==========================================================================
//
//      lib/socket.c
//
//      socket() system call
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
socket(int domain, int type, int protocol)
{
    struct sys_socket_args args;
    int res, error;
    SYSCALLARG(args,domain) = domain;
    SYSCALLARG(args,type) = type;
    SYSCALLARG(args,protocol) = protocol;
    error = sys_socket(&args, &res);
    if (error) {
        errno = error;
        return -1;
    } else {
        return res;
    }
}
