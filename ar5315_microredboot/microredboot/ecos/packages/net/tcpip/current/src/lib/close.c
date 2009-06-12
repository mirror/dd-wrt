//==========================================================================
//
//      lib/close.c
//
//      close() system call
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
close(int fd)
{
    int error;
    struct file *fp;
    if (getfp(fd, &fp))
        return (EBADF);
    error = (*fp->f_ops->fo_close)(fp);
    if (error) {
        errno = error;
        return -1;
    } else {
        ffree(fp);
        return 0;
    }
}
