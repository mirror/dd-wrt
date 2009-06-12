//==========================================================================
//
//      lib/select.c
//
//      'select()' system call
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
// Description:  eCos implementation of 'select()' system call
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <sys/param.h>
#include <cyg/io/file.h>
#include <cyg/kernel/kapi.h>
#include <sys/select.h>
#include <sys/bsdselect.h>

static cyg_flag_t select_flag;
static cyg_bool   select_flag_init = false;
#define SELECT_WAKE  0x01
#define SELECT_ABORT 0x02

//
// Private function which does all the work for 'select()'
//
static int
_cyg_select(int nfd, fd_set *in, fd_set *out, fd_set *ex,
            struct timeval *tv, cyg_bool_t abortable)
{
    int fd, mode, num, ticks;
    struct file *fp;
    fd_set in_res, out_res, ex_res;  // Result sets
    fd_set *selection[3], *result[3];
    cyg_tick_count_t now, then;
    int mode_type[] = {FREAD, FWRITE, 0};
    cyg_flag_value_t flag, wait_flag;

    // Note: since this is called by application programs, it needs no protection
    if (!select_flag_init) {
        select_flag_init = true;
        cyg_flag_init(&select_flag);
    }
    wait_flag = SELECT_WAKE;
    if (abortable) wait_flag |= SELECT_ABORT;
    FD_ZERO(&in_res);
    FD_ZERO(&out_res);
    FD_ZERO(&ex_res);
    // Set up sets
    selection[0] = in;   result[0] = &in_res;
    selection[1] = out;  result[1] = &out_res;
    selection[2] = ex;   result[2] = &ex_res;
    // Compute end time
    if (tv) {
        now = cyg_current_time();
        ticks = (tv->tv_sec * 100) + (tv->tv_usec / 10000);
        then = now + ticks;
    } else {
        then = 0;  // Compiler warnings :-(
        ticks = 0;
    }
    // Scan sets for possible I/O until something found, timeout or error.
    while (true) {
        num = 0;  // Total file descriptors "ready"

        cyg_scheduler_lock(); // Scan the list atomically wrt electing to sleep

        for (mode = 0;  mode < 3;  mode++) {
            if (selection[mode]) {
                for (fd = 0;  fd < nfd;  fd++) {
                    if (FD_ISSET(fd, selection[mode])) {
                        if (getfp(fd, &fp)) {
                            cyg_scheduler_unlock(); // return.
                            errno = EBADF;
                            return -1;
                        }
                        if ((*fp->f_ops->fo_select)(fp, mode_type[mode])) {
                            FD_SET(fd, result[mode]);
                            num++;
                        }
                    }
                }
            }
        }
        if (num) {

            cyg_scheduler_unlock(); // Happy, about to return.

            // Found something, update user's sets
            if (in) {
                memcpy(in, &in_res, sizeof(in_res));
            }
            if (out) {
                memcpy(out, &out_res, sizeof(out_res));
            }
            if (ex) {
                memcpy(ex, &ex_res, sizeof(ex_res));
            }
            return num;
        }
        // Nothing found, see if we want to wait
        if (tv) {
            if (ticks == 0) {
                // Special case of "poll"
                cyg_scheduler_unlock(); // About to return.
                return 0;
            }
            flag = cyg_flag_timed_wait(&select_flag, wait_flag,
                                       CYG_FLAG_WAITMODE_OR,
                                       then);
        } else {
            // Wait forever (until something happens)
            flag = cyg_flag_wait(&select_flag, wait_flag,
                                 CYG_FLAG_WAITMODE_OR);
        }

        cyg_scheduler_unlock(); // waited atomically

        if (flag & SELECT_ABORT) {
            errno = EINTR;
            return -1;
        }
        if (!flag) {
            return 0; // meaning no activity, ergo timeout occurred
        }
    }
    errno = ENOSYS;
    return -1;
}

//
// This function is called by the lower layers to record the
// fact that a particular 'select' event is being requested.
//
void        
selrecord(void *selector, struct selinfo *info)
{
    // Unused by this implementation
}

//
// This function is called to indicate that a 'select' event
// may have occurred.
//
void    
selwakeup(struct selinfo *info)
{
    // Need these ops to be atomic to make sure the clear occurs -
    // otherwise a higher prio thread could hog the CPU when its fds are
    // not ready, but the flag is (already) set, or set for someone else.
    cyg_scheduler_lock();
    cyg_flag_setbits(&select_flag, SELECT_WAKE);
    cyg_flag_maskbits(&select_flag,    0      ); // clear all
    cyg_scheduler_unlock();
}

//
// The public function used by 'normal' programs.  This interface does not allow
// the 'select()' to be externally interrupted.
//
int
select(int nfd, fd_set *in, fd_set *out, fd_set *ex,
       struct timeval *tv)
{
    return _cyg_select(nfd, in, out, ex, tv, false);
}

//
// The public function used by programs which wish to allow interruption,
// using the 'cyg_select_abort()' function below.
//
int
cyg_select_with_abort(int nfd, fd_set *in, fd_set *out, fd_set *ex,
                      struct timeval *tv)
{
    return _cyg_select(nfd, in, out, ex, tv, true);
}

//
// This function can be called by the user to forceably abort any
// current selects.
//
void
cyg_select_abort(void)
{
    // See comments in selwakeup()...
    cyg_scheduler_lock();
    cyg_flag_setbits(&select_flag, SELECT_ABORT); 
    cyg_flag_maskbits(&select_flag,    0       );
    cyg_scheduler_unlock();
}


