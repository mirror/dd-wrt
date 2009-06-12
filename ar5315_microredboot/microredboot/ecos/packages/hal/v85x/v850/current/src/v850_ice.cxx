//========================================================================
//
//      v850_ice.cxx
//
//      ICE debugging support for V850 processors
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2001-03-12
// Purpose:       
// Description:   ICE debugging support for V850 processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_V850_ICE

#include <stddef.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/dbg-threads-api.h>
#include <cyg/hal/nec-stub.h>
#include <cyg/hal/hal_arch.h>

#define ICEDEBUG 0

#if ICEDEBUG
#include <cyg/infra/diag.h>
#endif

/* ----------------------------------------------------------------------- */
/* Common ICE syscall information */

/* Magic area for syscall information, from vectors.S */
__externC char hal_v85x_ice_syscall_info[];

/* Syscall operation codes. This is a "contract" with the host. */

#define V850ICE_SYSCALL_GET_THREADNEXT 1
#define V850ICE_SYSCALL_GET_THREADREGS 2
#define V850ICE_SYSCALL_SET_THREADREGS 3
#define V850ICE_SYSCALL_GET_CURRTHREAD 4
#define V850ICE_SYSCALL_GET_THREADINFO 5
#define V850ICE_SYSCALL_CONSOLE_INPUT  6

/* System call information area layout */
#define ICE_SYSCALL_INFO_VALIDATOR     (*(int *)&hal_v85x_ice_syscall_info[0])
#define ICE_SYSCALL_INFO_STARTPC       (*(int *)&hal_v85x_ice_syscall_info[4])
#define ICE_SYSCALL_INFO_ENDPC         (*(int *)&hal_v85x_ice_syscall_info[8])
#define ICE_SYSCALL_INFO_STACK         (*(int *)&hal_v85x_ice_syscall_info[12])
#define ICE_SYSCALL_INFO_IOBUF         (*(int *)&hal_v85x_ice_syscall_info[16])
#define ICE_SYSCALL_INFO_DIAGOUTPC     (*(int *)&hal_v85x_ice_syscall_info[20])
#define ICE_SYSCALL_INFO_DIAGOUTBUF    (*(int *)&hal_v85x_ice_syscall_info[24])
#define ICE_SYSCALL_INFO_DIAGOUTBUFEND (*(int *)&hal_v85x_ice_syscall_info[28])

/* We can't use normal memcpy when invoked via the ICE. It may be unsafe. */
static void
my_memcpy( void *vd, const void *vs, int n)
{
    char *d = (char *)vd;
    char *s = (char *)vs;

    while (n--)
        *d++=*s++;
}

static void
my_memset( void *vs, char c, int n)
{
    char *s = (char *)vs;

    while (n--)
        *s++ = c;
}

/* ----------------------------------------------------------------------- */
/* Support for diag output via ICE */

#ifdef CYGDBG_HAL_V85X_V850_ICE_DIAG

#include <cyg/hal/hal_if.h>

static volatile cyg_uint8 v850ice_output_buf[128];
static volatile cyg_uint8 *v850ice_output_end = v850ice_output_buf;
#define OUTPUT_BUF_END (&v850ice_output_buf[ sizeof(v850ice_output_buf)])
static volatile cyg_uint8 v850ice_input_buf[128];
#define INPUT_BUF_END (&v850ice_input_buf[ sizeof(v850ice_input_buf)])
static volatile cyg_uint8 *v850ice_input_ptr_put = v850ice_input_buf;
static volatile cyg_uint8 *v850ice_input_ptr_get = v850ice_input_buf;
static volatile cyg_uint8 v850ice_input_buf_bytes_used = 0;

__externC void hal_v850_ice_output_break(void);

static void
hal_v850_ice_indicate_output(void)
{
    HAL_BREAKPOINT(hal_v850_ice_output_break);
}

// Actually write character
void
cyg_hal_plf_diag_ice_putc(void* __ch_data, cyg_uint8 c)
{
    CYGARC_HAL_SAVE_GP();

    // Send character
    *v850ice_output_end++ = c;

    if (c == '\n' ||
        v850ice_output_end == OUTPUT_BUF_END) {
        hal_v850_ice_indicate_output();
        v850ice_output_end = v850ice_output_buf;
    }

    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_diag_ice_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    if ( v850ice_input_buf_bytes_used == 0 )
        return false; // buffer empty

    *ch = *v850ice_input_ptr_get++;
    if ( v850ice_input_ptr_get == INPUT_BUF_END ) {
        v850ice_input_ptr_get = v850ice_input_buf;
    }

    v850ice_input_buf_bytes_used--;

    return true;
}

cyg_uint8
cyg_hal_plf_diag_ice_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_diag_ice_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

static cyg_bool
cyg_hal_plf_diag_ice_receive_char(cyg_uint8 ch)
{
     /* buffer full? */
    if ( v850ice_input_buf_bytes_used == sizeof(v850ice_input_buf) )
        return false;

    *v850ice_input_ptr_put++ = ch;
    
    if ( v850ice_input_ptr_put == INPUT_BUF_END ) {
        v850ice_input_ptr_put = v850ice_input_buf;
    }
    return true;
}

static void
cyg_hal_plf_diag_ice_write(void* __ch_data, const cyg_uint8* __buf, 
                           cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

#define MIN(__x, __y) ((__x) < (__y) ? (__x) : (__y))

    while(__len > 0) {
        int copylen = MIN(__len,
                          (cyg_uint32) (OUTPUT_BUF_END - v850ice_output_end));

        my_memcpy( (void *)v850ice_output_buf, __buf, copylen );
        __len -= copylen;
        v850ice_output_end += copylen;
        hal_v850_ice_indicate_output();
        v850ice_output_end = v850ice_output_buf;
    }
            
    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_diag_ice_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_diag_ice_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

static cyg_int32 msec_timeout = 1000;

cyg_bool
cyg_hal_plf_diag_ice_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count = msec_timeout * 10; // delay in .1 ms steps
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    for(;;) {
        res = cyg_hal_plf_diag_ice_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
cyg_hal_plf_diag_ice_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = 0;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = msec_timeout;
        msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
    }        
    default:
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_diag_ice_isr(void *__ch_data, int* __ctrlc, 
                         CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    CYGARC_HAL_SAVE_GP();
    *__ctrlc = 0;
    CYGARC_HAL_RESTORE_GP();
    return 0;
}

__externC void
cyg_hal_plf_ice_diag_init()
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    ICE_SYSCALL_INFO_DIAGOUTPC     = (int)&hal_v850_ice_output_break;
    ICE_SYSCALL_INFO_DIAGOUTBUF    = (int)&v850ice_output_buf[0];
    ICE_SYSCALL_INFO_DIAGOUTBUFEND = (int)&v850ice_output_end;

    // Setup procs in the vector table

    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, 0);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_diag_ice_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_diag_ice_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_diag_ice_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_diag_ice_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_diag_ice_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_diag_ice_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_diag_ice_getc_timeout);
    
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

#endif // ifdef CYGDBG_HAL_V85X_V850_ICE_DIAG


/* ----------------------------------------------------------------------- */
/* Support for debugging via ICE */

#define ICE_STACK_SIZE 1024/sizeof(int)
static int ice_stack[ICE_STACK_SIZE]; // ints so as to ensure alignment
static int ice_iobuf[128];

static void
hal_v85x_ice_syscall_end(void)
{
    for (;;);
}

static void
hal_v85x_ice_syscall(void)
{
    int code, len;
    code = ice_iobuf[0];
    len = ice_iobuf[1];
    switch (code) {
    case V850ICE_SYSCALL_GET_THREADNEXT:
        {
            int ret;
            threadref currthread, nextthread;
            int thread_id;

            /* Unmarshall thread ref */
            my_memcpy( &currthread, &ice_iobuf[2], 8 );
#if ICEDEBUG
            diag_printf("*NEXTTHREAD* currthread=%08x,%08x\n",
                        *(int *)&currthread[0],
                        *(int *)(((char *)&currthread[0])+4));
#endif
            // null threadref?
            if ((ice_iobuf[2] == 0) &&
                (ice_iobuf[3] == 0)) {
#if ICEDEBUG
                diag_printf("null threadref\n");
#endif
                ret = dbg_threadlist( 1, NULL, &nextthread );
            } else {
#if ICEDEBUG
                diag_printf("non-null threadref\n");
#endif
                ret = dbg_threadlist( 0, &currthread, &nextthread );
            }
#if ICEDEBUG
            diag_printf("*NEXTTHREAD* nextthread=%08x,%08x\n",
                        *(int *)&nextthread[0],
                        *(int *)(((char *)&nextthread[0])+4));
#endif
            if (ret) { // if valid thread found
                thread_id = dbg_thread_id( &nextthread );
                /* Returns 0 on error */
                if (thread_id != 0) {
                    ice_iobuf[1] = thread_id;
                    my_memcpy( &ice_iobuf[2], nextthread, 8 );
                    
                    // set return data size to 12
                    ice_iobuf[0] = 12;
#if ICEDEBUG
                    {
                        int i;
                        for (i=0; i<3; i++)
                            diag_printf("%x, ", ice_iobuf[i]);
                        diag_printf("\n");
                    }
#endif
                } else {
                    ret = 0;
                }
            } 
            if (!ret) {
                // set to null
                my_memset( &ice_iobuf[1], 0, 12 );
            }
        }
        break;
    case V850ICE_SYSCALL_GET_THREADREGS:
        {
            int ret;
            threadref thread;

            /* Unmarshall thread ref */
            my_memcpy( &thread, &ice_iobuf[2], 8 );
#if ICEDEBUG
            diag_printf("*GTHREADREGS* thread=%08x,%08x\n", *(int *)&thread[0],
                        *(int *)(((char *)&thread[0])+4));
#endif
            ret = dbg_getthreadreg( &thread, NUMREGS, &ice_iobuf[1]);
            if (ret)
                ice_iobuf[0] = NUMREGS * 4;
            else
                ice_iobuf[0] = 0;
                                    
        }
        break;
    case V850ICE_SYSCALL_SET_THREADREGS:
        {
            int ret;
            threadref thread;

            /* Unmarshall thread ref */
            my_memcpy( &thread, &ice_iobuf[2], 8 );
#if ICEDEBUG
            diag_printf("*STHREADREGS* thread=%08x,%08x\n", *(int *)&thread[0],
                        *(int *)(((char *)&thread[0])+4));
#endif
            ret = dbg_setthreadreg( &thread, NUMREGS, &ice_iobuf[4]);
            if (ret)
                ice_iobuf[0] = 1;
            else
                ice_iobuf[0] = 0;
        }
        break;
    case V850ICE_SYSCALL_GET_CURRTHREAD:
        {
            int ret, thread_id;
            threadref thread;

            ret = dbg_currthread( &thread );
#if ICEDEBUG
            diag_printf("*CURRTHREAD* thread=%08x,%08x\n", *(int *)&thread[0],
                        *(int *)(((char *)&thread[0])+4));
#endif
            
            if (ret) {
                thread_id = dbg_thread_id( &thread );
                /* Returns 0 on error */
                if (thread_id != 0) {
                    ice_iobuf[1] = thread_id;
                    my_memcpy( &ice_iobuf[2], thread, 8 );
                }
                else {
                    ret = 0;
                }
            }
            if (ret)
                ice_iobuf[0] = 12;
            else
                ice_iobuf[0] = 0;
        } 
        break;
    case V850ICE_SYSCALL_GET_THREADINFO:
        {
            int ret;
            threadref thread;
            struct cygmon_thread_debug_info info;
            char *s=(char *)&ice_iobuf[1], *p;

            /* Unmarshall thread ref */
            my_memcpy( &thread, &ice_iobuf[2], 8 );
#if ICEDEBUG
            diag_printf("*INFO* thread=%08x,%08x\n", *(int *)&thread[0],
                        *(int *)(((char *)&thread[0])+4));
#endif

            ret = dbg_threadinfo( &thread, &info );
            if (ret) {
                if (info.thread_display) {
                    my_memcpy (s, "State: ", 7);
                    s += 7;
                    p = info.thread_display;
                    while (*p) {
                        *s++ = *p++;
                    }
                }

                if (info.unique_thread_name && info.unique_thread_name[0]) {
                    my_memcpy (s, ", Name: ", 8);
                    s += 8;
                    p = info.unique_thread_name;
                    while (*p) {
                        *s++ = *p++;
                    }
                }

                if (info.more_display) {
                    my_memcpy (s, ", ", 2);
                    s += 2;
                    p = info.more_display;
                    while (*p) {
                        *s++ = *p++;
                    }
                }

            }
            *s++ = '\0';
            if (ret)
                ice_iobuf[0] = s - (char *)&ice_iobuf[1];
            else
                ice_iobuf[0] = 0;
        }
        break;
    case V850ICE_SYSCALL_CONSOLE_INPUT:
        {
#ifdef CYGDBG_HAL_V85X_V850_ICE_DIAG    
            int len = ice_iobuf[0];
            int i;

            for (i=1; i <= len; i++) {
                if (false == cyg_hal_plf_diag_ice_receive_char(ice_iobuf[i]))
                    break;
            }
            ice_iobuf[0] = i-1;
#else
            ice_iobuf[0] = 0;
#endif
        }
        break;
    default:
        // set return data size to 0
        ice_iobuf[0] = 0;
        break;
    }
    hal_v85x_ice_syscall_end();
}

class Cyg_dummy_ice_syscall_init_class {
public:
    Cyg_dummy_ice_syscall_init_class() {

        const int valid_string = 0x45434956; // "VICE"

        ICE_SYSCALL_INFO_STARTPC = (int)&hal_v85x_ice_syscall;
        ICE_SYSCALL_INFO_ENDPC = (int)&hal_v85x_ice_syscall_end;
        // top of stack, less 4 ints for parameter flushback area as per ABI
        ICE_SYSCALL_INFO_STACK = (int)&ice_stack[ICE_STACK_SIZE-4];
        ICE_SYSCALL_INFO_IOBUF = (int)&ice_iobuf[0];

        HAL_REORDER_BARRIER();

        // Leave validation string to last
        ICE_SYSCALL_INFO_VALIDATOR = valid_string;
    }
};

static Cyg_dummy_ice_syscall_init_class dummy_syscall_class;

#endif // ifdef CYGDBG_HAL_V850_ICE

// EOF v850_ice.cxx
