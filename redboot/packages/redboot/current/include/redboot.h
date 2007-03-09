//==========================================================================
//
//      redboot.h
//
//      Standard interfaces for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, tkoeller
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _REDBOOT_H_
#define _REDBOOT_H_

#include <pkgconf/redboot.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_tables.h>
#include <cyg/hal/hal_endian.h>
#include <cyg/infra/diag.h>
#include <cyg/crc/crc.h>
#include <string.h>

#ifdef CYGPKG_REDBOOT_NETWORKING
#include <net/net.h>
#include <net/bootp.h>
// Determine an IP address for this node, using BOOTP
extern int __bootp_find_local_ip(bootp_header_t *info);
#endif

#ifdef DEFINE_VARS
#define EXTERN
#else
#define EXTERN extern
#endif

// Global variables
EXTERN int argc;
#define MAX_ARGV 16
EXTERN char *argv[MAX_ARGV];
EXTERN unsigned char *ram_start, *ram_end;
EXTERN struct _mem_segment {
    unsigned char *start, *end;
} mem_segments[CYGBLD_REDBOOT_MAX_MEM_SEGMENTS];
#define NO_MEMORY (unsigned char *)0xFFFFFFFF
EXTERN bool valid_address(unsigned char *addr);
EXTERN void cyg_plf_memory_segment(int seg, unsigned char **start, unsigned char **end);
EXTERN unsigned char *workspace_start, *workspace_end;

// Data squirreled away after a load operation
EXTERN unsigned long entry_address;
EXTERN unsigned long load_address;
EXTERN unsigned long load_address_end;


#ifdef CYGPKG_REDBOOT_ANY_CONSOLE
EXTERN bool console_selected;
#endif
EXTERN bool console_echo;
EXTERN bool gdb_active;
#if CYGNUM_REDBOOT_CMD_LINE_EDITING != 0
EXTERN bool cmd_history;
#endif

#ifdef CYGPKG_REDBOOT_NETWORKING
EXTERN bool have_net, use_bootp;
EXTERN bootp_header_t my_bootp_info;
EXTERN int gdb_port;
EXTERN bool net_debug;
#endif

#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
EXTERN unsigned char *script;
EXTERN int script_timeout;
#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
EXTERN int console_baud_rate;
#endif
#endif

#ifdef CYGOPT_REDBOOT_FIS_ZLIB_COMMON_BUFFER
EXTERN unsigned char *fis_zlib_common_buffer;
#endif

#ifdef CYGSEM_REDBOOT_PLF_STARTUP
EXTERN void cyg_plf_redboot_startup(void);
#endif

// Prototypes
typedef int _printf_fun(const char *fmt, ...);
externC int  strcasecmp(const char *s1, const char *s2);
externC int  strncasecmp(const char *s1, const char *s2, size_t len);

externC void mon_write_char(char c);
externC bool verify_action(char *fmt, ...);
externC bool verify_action_with_timeout(int timeout, char *fmt, ...);

// Read a single line of input from the console, possibly with timeout
externC int  _rb_gets(char *line, int len, int timeout);
// Just like _rb_gets(), except that the line buffer is assumed to contain
// valid input data.  This provides an easy mechanism for edit-in-place.
externC int  _rb_gets_preloaded(char *line, int len, int timeout);
// Result codes from 'gets()'
#define _GETS_TIMEOUT -1
#define _GETS_CTRLC   -2
#define _GETS_GDB      0
#define _GETS_OK       1
// Test for ^C on the console.  This function should only be used if any
// other console input can be discarded, e.g. while performing some long
// computation, waiting for the network, etc.  Returns 'true' if ^C typed.
externC bool _rb_break(int timeout);

// "console" selection
externC int  start_console(void);
externC void end_console(int old_console);

// Alias functions
#ifdef CYGSEM_REDBOOT_FLASH_ALIASES
externC char *flash_lookup_alias(char *alias, char *alias_buf);
#endif
externC void expand_aliases(char *line, int len);

//
// Stream I/O support
//

typedef struct {
    char *filename;
    int   mode;
    int   chan;
#ifdef CYGPKG_REDBOOT_NETWORKING
    struct sockaddr_in *server;
#endif
} connection_info_t;

typedef struct {
    int   (*open)(connection_info_t *info, int *err);    
    void  (*close)(int *err);    
    void  (*terminate)(bool abort, int (*getc)(void));    
    int   (*read)(char *buf, int size, int *err);    
    char *(*error)(int err);
} getc_io_funcs_t;

#define GETC_IO_FUNCS(_label_, _open_, _close_, _terminate_, _read_, _error_)   \
getc_io_funcs_t _label_ = {                                                     \
    _open_, _close_, _terminate_, _read_, _error_                               \
};

struct load_io_entry {
    char            *name;
    getc_io_funcs_t *funcs;    
    bool             can_verbose;
    bool             need_filename;
    int              mode;
} CYG_HAL_TABLE_TYPE;
#define _RedBoot_load(_name_,_funcs_,_verbose_,_filename_,_mode_)       \
struct load_io_entry _load_tab_##_funcs_##_name_                        \
   CYG_HAL_TABLE_QUALIFIED_ENTRY(RedBoot_load,_funcs_##_name) =         \
     { #_name_, &_funcs_, _verbose_, _filename_, _mode_ }; 
#define RedBoot_load(_name_,_funcs_,_verbose_,_filename_, _mode_)       \
   _RedBoot_load(_name_,_funcs_,_verbose_,_filename_,_mode_)
 
#ifdef CYGPKG_COMPRESS_ZLIB
// Decompression support
typedef struct _pipe {
    unsigned char* in_buf;              // only changed by producer
    int in_avail;                       // only changed by producer
    unsigned char* out_buf;             // only changed by consumer (init by producer)
    int out_size;                       // only changed by consumer (init by producer)
    int out_max;                        // set by producer
    const char* msg;                    // message from consumer
    void* priv;                         // handler's data
} _pipe_t;

typedef int _decompress_fun_init(_pipe_t*);
typedef int _decompress_fun_inflate(_pipe_t*);
typedef int _decompress_fun_close(_pipe_t*, int);

externC _decompress_fun_init* _dc_init;
externC _decompress_fun_inflate* _dc_inflate;
externC _decompress_fun_close* _dc_close;
#endif // CYGPKG_COMPRESS_ZLIB

// CLI support functions
externC bool parse_num(char *s, unsigned long *val, char **es, char *delim);
externC bool parse_bool(char *s, bool *val);

typedef void cmd_fun(int argc, char *argv[]);
struct cmd {
    char    *str;
    char    *help;
    char    *usage;
    cmd_fun *fun;
    struct cmd *sub_cmds, *sub_cmds_end;
} CYG_HAL_TABLE_TYPE;
externC struct cmd *cmd_search(struct cmd *tab, struct cmd *tabend, char *arg);
externC void        cmd_usage(struct cmd *tab, struct cmd *tabend, char *prefix);
#define RedBoot_cmd(_s_,_h_,_u_,_f_) cmd_entry(_s_,_h_,_u_,_f_,0,0,RedBoot_commands)
#define RedBoot_nested_cmd(_s_,_h_,_u_,_f_,_subs_,_sube_) cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,RedBoot_commands)
#define _cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,_n_)                                   \
cmd_fun _f_;                                                      \
struct cmd _cmd_tab_##_f_ CYG_HAL_TABLE_QUALIFIED_ENTRY(_n_,_f_) = {_s_, _h_, _u_, _f_, _subs_, _sube_};
#define cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,_n_)                                   \
extern _cmd_entry(_s_,_h_,_u_,_f_,_subs_,_sube_,_n_)
#define local_cmd_entry(_s_,_h_,_u_,_f_,_n_)                             \
static _cmd_entry(_s_,_h_,_u_,_f_,0,0,_n_)

// Initialization functions
#define RedBoot_INIT_FIRST  0000
#define RedBoot_INIT_SECOND 0100
// Specify a 3 digit numeric value for proper prioritizing
#define RedBoot_INIT_PRIO(_n_) 1##_n_
#define RedBoot_INIT_LAST  9999
typedef void void_fun(void);
typedef void_fun *void_fun_ptr;
struct init_tab_entry {
    void_fun_ptr fun;
} CYG_HAL_TABLE_TYPE;
#define _RedBoot_init(_f_,_p_)                                          \
struct init_tab_entry _init_tab_##_p_##_f_                              \
  CYG_HAL_TABLE_QUALIFIED_ENTRY(RedBoot_inits,_p_##_f_) = { _f_ }; 
#define RedBoot_init(_f_,_p_) _RedBoot_init(_f_,_p_)

// Main loop [idle] call-back functions
#define RedBoot_IDLE_FIRST          0000
#define RedBoot_IDLE_BEFORE_NETIO   3000
#define RedBoot_IDLE_NETIO          5000
#define RedBoot_IDLE_AFTER_NETIO    7000
#define RedBoot_IDLE_LAST           9999
typedef void idle_fun(bool);
typedef idle_fun *idle_fun_ptr;
struct idle_tab_entry {
    idle_fun_ptr fun;
} CYG_HAL_TABLE_TYPE;
#define _RedBoot_idle(_f_,_p_)                                          \
struct idle_tab_entry _idle_tab_##_p_##_f_                              \
   CYG_HAL_TABLE_QUALIFIED_ENTRY(RedBoot_idle,_p_##_f_) = { _f_ }; 
#define RedBoot_idle(_f_,_p_) _RedBoot_idle(_f_,_p_)

// This function called when changing idle/not - mostly used by I/O
// to support idle when timeout, etc.
void do_idle(bool state);

// Option processing support

struct option_info {
    char flag;
    bool takes_arg;
    int  arg_type;
    void *arg;
    bool *arg_set;
    char *name;
};

#define NUM_ELEMS(s) (sizeof(s)/sizeof(s[0]))

#define OPTION_ARG_TYPE_NUM 0    // Numeric data
#define OPTION_ARG_TYPE_STR 1    // Generic string
#define OPTION_ARG_TYPE_FLG 2    // Flag only

// Command line parsing
externC struct cmd *parse(char **line, int *argc, char **argv);

externC void init_opts(struct option_info *opts, char flag, bool takes_arg, 
                       int arg_type, void *arg, bool *arg_set, char *name);
externC bool scan_opts(int argc, char *argv[], int first, 
                       struct option_info *opts, int num_opts, 
                       void *def_arg, int def_arg_type, char *def_descr);

#ifdef CYGNUM_HAL_VIRTUAL_VECTOR_AUX_CHANNELS
#define CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS \
  (CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS+CYGNUM_HAL_VIRTUAL_VECTOR_AUX_CHANNELS)
#else
#define CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS \
  CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS
#endif

#ifdef CYGPKG_REDBOOT_NETWORKING
//-----------------------------------------------------------------------------
// DNS wrapper
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS

// I would really like if we could just pull in cyg/ns/dns/dns.h, but
// that would require adding dummy <network.h> and <netinet/in.h> files.

// Host name / IP mapping
struct hostent {
    char    *h_name;        /* official name of host */
    char    **h_aliases;    /* alias list */
    int     h_addrtype;     /* host address type */
    int     h_length;       /* length of address */
    char    **h_addr_list;  /* list of addresses */
};
#define h_addr  h_addr_list[0]  /* for backward compatibility */

externC int redboot_dns_res_init(void);
externC void set_dns(char* new_ip);
externC void show_dns(void);
externC struct hostent *gethostbyname(const char *host);

// Error reporting
externC int h_errno;

#define DNS_SUCCESS  0
#define HOST_NOT_FOUND 1
#define TRY_AGAIN      2
#define NO_RECOVERY    3
#define NO_DATA        4

static inline bool
_gethostbyname(const char* name, in_addr_t* host)
{ 
    struct hostent* hent = gethostbyname(name);
    if (hent) {
        memcpy(host, hent->h_addr_list[0], sizeof(in_addr_t));
        return true;
    }
    // Fall back to inet_aton - gethostbyname may already have tried
    // it, but we can't know for sure (the DNS IP may not have been
    // valid, preventing the inet_aton).
    return inet_aton(name, host);
}
#else
static inline bool
_gethostbyname(const char* name, in_addr_t* host)
{ 
    return inet_aton(name, host);
}
#endif // CYGPKG_REDBOOT_NETWORKING_DNS
#endif // CYGPKG_REDBOOT_NETWORKING

//-----------------------------------------------------------------------------
// String functions. Some of these are duplicates of the same functions in
// the I18N package.

// Validate a hex character
__inline__ static bool
_is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||            
            ((c >= 'a') && (c <= 'f')));
}

// Convert a single hex nibble
__inline__ static int
_from_hex(char c) 
{
    int ret = 0;

    if ((c >= '0') && (c <= '9')) {
        ret = (c - '0');
    } else if ((c >= 'a') && (c <= 'f')) {
        ret = (c - 'a' + 0x0a);
    } else if ((c >= 'A') && (c <= 'F')) {
        ret = (c - 'A' + 0x0A);
    }
    return ret;
}

// Convert a character to lower case
__inline__ static char
_tolower(char c)
{
    if ((c >= 'A') && (c <= 'Z')) {
        c = (c - 'A') + 'a';
    }
    return c;
}

// Validate alpha
__inline__ static bool
isalpha(int c)
{
    return (((c >= 'a') && (c <= 'z')) || 
            ((c >= 'A') && (c <= 'Z')));
}

// Validate digit
__inline__ static bool
isdigit(int c)
{
    return ((c >= '0') && (c <= '9'));
}

// Validate alphanum
__inline__ static bool
isalnum(int c)
{
    return (isalpha(c) || isdigit(c));
}

//----------------------------------------------------------------------------
// syscall values
#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)

// These are required by the ANSI C part of newlib (excluding system() of
// course).
#define SYS_exit         1
#define SYS_open         2
#define SYS_close        3
#define SYS_read         4
#define SYS_write        5
#define SYS_lseek        6
#define SYS_unlink       7
#define SYS_getpid       8
#define SYS_kill         9
#define SYS_fstat        10
//#define SYS_sbrk       11 - not currently a system call, but reserved.

// ARGV support.
#define SYS_argvlen      12
#define SYS_argv         13

// These are extras added for one reason or another.
#define SYS_chdir        14
#define SYS_stat         15
#define SYS_chmod        16
#define SYS_utime        17
#define SYS_time         18
#define SYS_gettimeofday 19
#define SYS_times        20

#define SYS_interrupt   1000
#define SYS_meminfo     1001

#define __GET_SHARED  0xbaad // 47789 decimal

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

#define SYS_timer_call_back 2001
#define SYS_timer_frequency 2002
#define SYS_timer_reset     2003

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
#define SYS_rename          3001
#define SYS_isatty          3002
#define SYS_system          3003

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS


//----------------------------------------------------------------------------
// Allow HAL to override RedBoot flash read/program operations.
#ifdef HAL_FLASH_READ
#define FLASH_READ(f, r, l, e) HAL_FLASH_READ((f),(r),(l),(e))
#else
#define FLASH_READ(f, r, l, e) flash_read((f), (r), (l), (e))
#endif

#ifdef HAL_FLASH_PROGRAM
#define FLASH_PROGRAM(f, r, l, e) HAL_FLASH_PROGRAM((f),(r),(l),(e))
#else
#define FLASH_PROGRAM(f, r, l, e) flash_program((f), (r), (l), (e))
#endif


// Define REDBOOT_FLASH_REVERSE_BYTEORDER if config and fis info is stored in flash
// with byte ordering opposite from CYG_BYTEORDER. 
#if (defined(CYGOPT_REDBOOT_FLASH_BYTEORDER_MSBFIRST) && (CYG_BYTEORDER != CYG_MSBFIRST)) || \
    (defined(CYGOPT_REDBOOT_FLASH_BYTEORDER_LSBFIRST) && (CYG_BYTEORDER != CYG_LSBFIRST))
#define REDBOOT_FLASH_REVERSE_BYTEORDER
#endif

#endif // _REDBOOT_H_
