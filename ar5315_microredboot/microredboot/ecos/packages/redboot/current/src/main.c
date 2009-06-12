//==========================================================================
//
//      main.c
//
//      RedBoot main routine
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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

#define  DEFINE_VARS
#include <redboot.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>            // Logical driver interfaces
#endif

#include <cyg/hal/hal_tables.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#ifdef CYGBLD_HAL_PLATFORM_STUB_H
#include CYGBLD_HAL_PLATFORM_STUB_H
#else
#include <cyg/hal/plf_stub.h>
#endif
// GDB interfaces
extern void breakpoint(void);
#endif

// Builtin Self Test (BIST)
externC void bist(void);

// Path to code run from a go command or to GDB stubs
static void trampoline(unsigned long entry);

// Return path for code run from a go command or for GDB stubs
static void return_to_redboot(int status);

// Address of area where current context is saved before executing
// trampoline procedure
static void * saved_context;

// Status returned after trampoline execution
static int return_status;
 

// CLI command processing (defined in this file)
RedBoot_cmd("version", 
            "Display RedBoot version information",
            "",
            do_version
    );
RedBoot_cmd("help", 
            "Help about help?", 
            "[<topic>]",
            do_help 
    );

static char go_usage[] = "[-w <timeout>] [-c] "
#ifdef CYGPKG_IO_ETH_DRIVERS
                      "[-n] "
#endif
                      "[entry]";

RedBoot_cmd("go", 
            "Execute code at a location", 
            go_usage,
            do_go 
    );
#ifdef HAL_PLATFORM_RESET
RedBoot_cmd("reset", 
            "Reset the system", 
            "",
            do_reset 
    );
#endif
#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
RedBoot_cmd("baudrate", 
            "Set/Query the system console baud rate", 
            "[-b <rate>]",
            do_baud_rate
    );
#endif

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __RedBoot_INIT_TAB__, RedBoot_inits );
CYG_HAL_TABLE_END( __RedBoot_INIT_TAB_END__, RedBoot_inits );
extern struct init_tab_entry __RedBoot_INIT_TAB__[], __RedBoot_INIT_TAB_END__;

CYG_HAL_TABLE_BEGIN( __RedBoot_CMD_TAB__, RedBoot_commands );
CYG_HAL_TABLE_END( __RedBoot_CMD_TAB_END__, RedBoot_commands );
extern struct cmd __RedBoot_CMD_TAB__[], __RedBoot_CMD_TAB_END__;

CYG_HAL_TABLE_BEGIN( __RedBoot_IDLE_TAB__, RedBoot_idle );
CYG_HAL_TABLE_END( __RedBoot_IDLE_TAB_END__, RedBoot_idle );
extern struct idle_tab_entry __RedBoot_IDLE_TAB__[], __RedBoot_IDLE_TAB_END__;

#ifdef HAL_ARCH_PROGRAM_NEW_STACK
extern void HAL_ARCH_PROGRAM_NEW_STACK(void *fun);
#endif

// 
// [Null] Builtin [Power On] Self Test
//
void bist(void) CYGBLD_ATTRIB_WEAK;

void
bist(void) 
{
}

//
// 'version' command
//
void
do_version(int argc, char *argv[])
{
#if CYGBLD_REDBOOT_MAX_MEM_SEGMENTS > 1
    int seg;
#endif
#ifdef CYGPKG_REDBOOT_FLASH
    externC void _flash_info(void);
#endif
    char *version = CYGACC_CALL_IF_MONITOR_VERSION();

    diag_printf(version);
#ifdef HAL_PLATFORM_CPU
    diag_printf("Platform: %s (%s) %s\n", HAL_PLATFORM_BOARD, HAL_PLATFORM_CPU, HAL_PLATFORM_EXTRA);
#endif
    diag_printf("Copyright (C) 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.\n");
    diag_printf("Copyright (C) 2009 NewMedia-NET GmbH\n\n");
    diag_printf("Board: %s \n", CYGNUM_HAL_BOARD_TYPE);
    diag_printf("RAM: %p-%p, ", (void*)ram_start, (void*)ram_end);
    diag_printf("[%p-%p]", mem_segments[0].start, mem_segments[0].end);
    diag_printf(" available\n");
#if CYGBLD_REDBOOT_MAX_MEM_SEGMENTS > 1
    for (seg = 1;  seg < CYGBLD_REDBOOT_MAX_MEM_SEGMENTS;  seg++) {
        if (mem_segments[seg].start != NO_MEMORY) {
            diag_printf("     %p-%p, ", mem_segments[seg].start, mem_segments[seg].end);
            diag_printf("[%p-%p]", mem_segments[seg].start, mem_segments[seg].end);
            diag_printf(" available\n");
        }
    }
#endif
#ifdef CYGPKG_REDBOOT_FLASH
    _flash_info();
#endif
}

//
// This function is called when RedBoot is idle (waiting for user
// input).  It will call any registered "idle" routines, e.g. scan
// for incoming network connections, blank an LCD screen, etc.
//
void
do_idle(bool is_idle)
{
    struct idle_tab_entry *idle_entry;

    for (idle_entry = __RedBoot_IDLE_TAB__; 
         idle_entry != &__RedBoot_IDLE_TAB_END__;  idle_entry++) {
        (*idle_entry->fun)(is_idle);
    }
}

// Wrapper used by diag_printf()
static void
_mon_write_char(char c, void **param)
{
    if (c == '\n') {
        mon_write_char('\r');
    }
    mon_write_char(c);
}

//
// Handle illegal memory accesses (and other abort conditions)
//
static hal_jmp_buf error_jmpbuf;
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
externC 
#endif
  void* volatile __mem_fault_handler;

static void error_handler(void)
{
    hal_longjmp(error_jmpbuf, 1);
}


//
// This is the main entry point for RedBoot
//
void
cyg_start(void)
{
    int res = 0;
    bool prompt = true;
    static char line[CYGPKG_REDBOOT_MAX_CMD_LINE];
    char *command;
    struct cmd *cmd;
    int cur;
    struct init_tab_entry *init_entry;
    extern char RedBoot_version[];
#if CYGBLD_REDBOOT_MAX_MEM_SEGMENTS > 1
    int seg;
#endif

    // Export version information
    CYGACC_CALL_IF_MONITOR_VERSION_SET(RedBoot_version);

    CYGACC_CALL_IF_MONITOR_RETURN_SET(return_to_redboot);

    // Make sure the channels are properly initialized.
    diag_init_putc(_mon_write_char);
    hal_if_diag_init();

    // Force console to output raw text - but remember the old setting
    // so it can be restored if interaction with a debugger is
    // required.
    cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL);
#ifdef CYGPKG_REDBOOT_ANY_CONSOLE
    console_selected = false;
#endif
    console_echo = true;
    CYGACC_CALL_IF_DELAY_US((cyg_int32)2*100000);

    ram_start = (unsigned char *)CYGMEM_REGION_ram;
    ram_end = (unsigned char *)(CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE);
#ifdef HAL_MEM_REAL_REGION_TOP
    {
        unsigned char *ram_end_tmp = ram_end;
        ram_end = HAL_MEM_REAL_REGION_TOP( ram_end_tmp );
    }
#endif
#ifdef CYGMEM_SECTION_heap1
    workspace_start = (unsigned char *)CYGMEM_SECTION_heap1;
    workspace_end = (unsigned char *)(CYGMEM_SECTION_heap1+CYGMEM_SECTION_heap1_SIZE);
#else
    workspace_start = (unsigned char *)CYGMEM_REGION_ram;
    workspace_end = (unsigned char *)(CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE);
#endif

    if ( ram_end < workspace_end ) {
        // when *less* SDRAM is installed than the possible maximum,
        // but the heap1 region remains greater...
        workspace_end = ram_end;
    }

    // Nothing has ever been loaded into memory
    entry_address = (unsigned long)NO_MEMORY;

    bist();

#if defined(CYGPRI_REDBOOT_ZLIB_FLASH) && defined(CYGOPT_REDBOOT_FIS_ZLIB_COMMON_BUFFER)
    fis_zlib_common_buffer =
    workspace_end -= CYGNUM_REDBOOT_FIS_ZLIB_COMMON_BUFFER_SIZE;
#endif

#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
    script_timeout = CYGNUM_REDBOOT_BOOT_SCRIPT_DEFAULT_TIMEOUT;
#endif

    for (init_entry = __RedBoot_INIT_TAB__; init_entry != &__RedBoot_INIT_TAB_END__;  init_entry++) {
        (*init_entry->fun)();
    }

    mem_segments[0].start = workspace_start;
    mem_segments[0].end = workspace_end;
#if CYGBLD_REDBOOT_MAX_MEM_SEGMENTS > 1
    for (seg = 1;  seg < CYGBLD_REDBOOT_MAX_MEM_SEGMENTS;  seg++) {
        cyg_plf_memory_segment(seg, &mem_segments[seg].start, &mem_segments[seg].end);
    }
#endif

#ifdef CYGSEM_REDBOOT_PLF_STARTUP
    cyg_plf_redboot_startup();
#endif
    do_version(0,0);

#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
# ifdef CYGDAT_REDBOOT_DEFAULT_BOOT_SCRIPT
    if (!script) {
      script = CYGDAT_REDBOOT_DEFAULT_BOOT_SCRIPT;
    }
# endif
    if (script) {
        // Give the guy a chance to abort any boot script
        unsigned char *hold_script = script;
        int script_timeout_ms = script_timeout * CYGNUM_REDBOOT_BOOT_SCRIPT_TIMEOUT_RESOLUTION;
        diag_printf("== Executing boot script in %d.%03d seconds - enter ^C to abort\n", 
                    script_timeout_ms/1000, script_timeout_ms%1000);
        script = (unsigned char *)0;
        res = _GETS_CTRLC;  // Treat 0 timeout as ^C
        while (script_timeout_ms >= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT) {
            res = _rb_gets(line, sizeof(line), CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT);
            if (res >= _GETS_OK) {
                diag_printf("== Executing boot script in %d.%03d seconds - enter ^C to abort\n", 
                            script_timeout_ms/1000, script_timeout_ms%1000);
                continue;  // Ignore anything but ^C
            }
            if (res != _GETS_TIMEOUT) break;
            script_timeout_ms -= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT;
        }
        if (res == _GETS_CTRLC) {
            script = (unsigned char *)0;  // Disable script
        } else {
            script = hold_script;  // Re-enable script
        }
    }
#endif

    while (true) {
        if (prompt) {
            diag_printf("RedBoot> ");
            prompt = false;
        }
#if CYGNUM_REDBOOT_CMD_LINE_EDITING != 0
        cmd_history = true;  // Enable history collection
#endif
        res = _rb_gets(line, sizeof(line), CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT);
#if CYGNUM_REDBOOT_CMD_LINE_EDITING != 0
        cmd_history = false;  // Enable history collection
#endif
        if (res == _GETS_TIMEOUT) {
            // No input arrived
        } else {
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
            if (res == _GETS_GDB) {
		int dbgchan;
                hal_virtual_comm_table_t *__chan;
                int i;
                // Special case of '$' - need to start GDB protocol
                gdb_active = true;
                // Mask interrupts on all channels
                for (i = 0;  i < CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS;  i++) {
                    CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
                    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
                    CYGACC_COMM_IF_CONTROL( *__chan, __COMMCTL_IRQ_DISABLE );
                }
    
                CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);

                // set up a temporary context that will take us to the trampoline
                HAL_THREAD_INIT_CONTEXT(workspace_end,
                                        breakpoint, trampoline, 0);

                // switch context to trampoline (get GDB stubs started)
                HAL_THREAD_SWITCH_CONTEXT(&saved_context, &workspace_end);

                gdb_active = false;

		dbgchan = CYGACC_CALL_IF_SET_DEBUG_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
		CYGACC_CALL_IF_SET_CONSOLE_COMM(dbgchan);
            } else 
#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
            {
#ifdef CYGSEM_REDBOOT_FLASH_ALIASES              
                expand_aliases(line, sizeof(line));
#endif
		command = (char *)&line;
                if ((*command == '#') || (*command == '=')) {
                    // Special cases
                    if (*command == '=') {
                        // Print line on console
                        diag_printf("%s\n", &line[2]);
                    }
                } else {
                    while (strlen(command) > 0) {                    
                        if ((cmd = parse(&command, &argc, &argv[0])) != (struct cmd *)0) {
                            // Try to handle aborts - messy because of the stack unwinding...
                            __mem_fault_handler = error_handler;
                            if (hal_setjmp(error_jmpbuf)) {
                                diag_printf("** command abort - illegal memory access?\n");
                            } else {
                                (cmd->fun)(argc, argv);
                            }
                            __mem_fault_handler = 0;
                        } else {
                            diag_printf("** Error: Illegal command: \"%s\"\n", argv[0]);
                        }
                    }
                }
                prompt = true;
            }
        }
    }
}

void
show_help(struct cmd *cmd, struct cmd *cmd_end, char *which, char *pre)
{
    bool show;
    int len = 0;

    if (which) {
        len = strlen(which);
    }
    while (cmd != cmd_end) {
        show = true;
        if (which && (strncasecmp(which, cmd->str, len) != 0)) {
            show = false;
        }
        if (show) {
            diag_printf("%s\n  %s %s %s\n", cmd->help, pre, cmd->str, cmd->usage);
            if ((cmd->sub_cmds != (struct cmd *)0) && (which != (char *)0)) {
                show_help(cmd->sub_cmds, cmd->sub_cmds_end, 0, cmd->str);
            }
        }
        cmd++;
    }
}

void
do_help(int argc, char *argv[])
{
    struct cmd *cmd;
    char *which = (char *)0;

    if (!scan_opts(argc, argv, 1, 0, 0, (void *)&which, OPTION_ARG_TYPE_STR, "<topic>")) {
        diag_printf("Invalid argument\n");
        return;
    }
    cmd = __RedBoot_CMD_TAB__;
    show_help(cmd, &__RedBoot_CMD_TAB_END__, which, "");
    return;
}

static void
trampoline(unsigned long entry)
{
    typedef void code_fun(void);
    code_fun *fun = (code_fun *)entry;
    unsigned long oldints;

    HAL_DISABLE_INTERRUPTS(oldints);

#ifdef HAL_ARCH_PROGRAM_NEW_STACK
    HAL_ARCH_PROGRAM_NEW_STACK(fun);
#else
    (*fun)();
#endif

    HAL_THREAD_LOAD_CONTEXT(&saved_context);
}

static void
return_to_redboot(int status)
{
    CYGARC_HAL_SAVE_GP();

    return_status = status;
    HAL_THREAD_LOAD_CONTEXT(&saved_context);
    // never returns

    // need this to balance above CYGARC_HAL_SAVE_GP on
    // some platforms. It will never run, though.
    CYGARC_HAL_RESTORE_GP();
}

void
do_go(int argc, char *argv[])
{
    int i, cur, num_options;
    unsigned long entry;
    unsigned long oldints;
    bool wait_time_set;
    int  wait_time, res;
    bool cache_enabled = false;
#ifdef CYGPKG_IO_ETH_DRIVERS
    bool stop_net = false;
#endif
    struct option_info opts[3];
    char line[8];
    hal_virtual_comm_table_t *__chan;

    __mem_fault_handler = 0; // Let GDB handle any faults directly
    entry = entry_address;  // Default from last 'load' operation
    init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, 
              (void *)&wait_time, (bool *)&wait_time_set, "wait timeout");
    init_opts(&opts[1], 'c', false, OPTION_ARG_TYPE_FLG, 
              (void *)&cache_enabled, (bool *)0, "go with caches enabled");
    num_options = 2;
#ifdef CYGPKG_IO_ETH_DRIVERS
    init_opts(&opts[2], 'n', false, OPTION_ARG_TYPE_FLG, 
              (void *)&stop_net, (bool *)0, "go with network driver stopped");
    num_options++;
#endif

    CYG_ASSERT(num_options <= NUM_ELEMS(opts), "Too many options");

    if (!scan_opts(argc, argv, 1, opts, num_options, (void *)&entry, OPTION_ARG_TYPE_NUM, "starting address"))
    {
        return;
    }
    if (entry == (unsigned long)NO_MEMORY) {
        diag_printf("No entry point known - aborted\n");
        return;
    }
    if (wait_time_set) {
        int script_timeout_ms = wait_time * 1000;
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
        unsigned char *hold_script = script;
        script = (unsigned char *)0;
#endif
        diag_printf("About to start execution at %p - abort with ^C within %d seconds\n",
                    (void *)entry, wait_time);
        while (script_timeout_ms >= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT) {
            res = _rb_gets(line, sizeof(line), CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT);
            if (res == _GETS_CTRLC) {
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
                script = hold_script;  // Re-enable script
#endif
                return;
            }
            script_timeout_ms -= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT;
        }
    }

    // Mask interrupts on all channels
    cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    for (i = 0;  i < CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS;  i++) {
	CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
	__chan = CYGACC_CALL_IF_CONSOLE_PROCS();
	CYGACC_COMM_IF_CONTROL( *__chan, __COMMCTL_IRQ_DISABLE );
    }
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);

    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_ENABLE_LINE_FLUSH);

#ifdef CYGPKG_IO_ETH_DRIVERS
    if (stop_net)
	eth_drv_stop();
#endif
	
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    if (!cache_enabled) {
	HAL_ICACHE_DISABLE();
	HAL_DCACHE_DISABLE();
	HAL_DCACHE_SYNC();
    }
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_DCACHE_INVALIDATE_ALL();
    // set up a temporary context that will take us to the trampoline
    HAL_THREAD_INIT_CONTEXT(workspace_end, entry, trampoline, 0);

    // switch context to trampoline
    HAL_THREAD_SWITCH_CONTEXT(&saved_context, &workspace_end);

    // we get back here by way of return_to_redboot()

    // undo the changes we made before switching context
    if (!cache_enabled) {
	HAL_ICACHE_ENABLE();
	HAL_DCACHE_ENABLE();
    }

    CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_DISABLE_LINE_FLUSH);

    HAL_RESTORE_INTERRUPTS(oldints);

    diag_printf("\nProgram completed with status %d\n", return_status);
}

#ifdef HAL_PLATFORM_RESET
void
do_reset(int argc, char *argv[])
{
    diag_printf("... Resetting.");
    CYGACC_CALL_IF_DELAY_US(2*100000);
    diag_printf("\n");
    CYGACC_CALL_IF_RESET();
    diag_printf("!! oops, RESET not working on this platform\n");
}
#endif

#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <flash_config.h>
#endif

static int
set_comm_baud_rate(hal_virtual_comm_table_t *chan, int rate)
{
    int current_rate;

    current_rate = CYGACC_COMM_IF_CONTROL(*chan, __COMMCTL_GETBAUD);
    if (rate != current_rate)
        return CYGACC_COMM_IF_CONTROL(*chan, __COMMCTL_SETBAUD, rate);

    return 0;
}

int
set_console_baud_rate(int rate)
{
    int ret = -1;
#ifdef CYGPKG_REDBOOT_ANY_CONSOLE
    if (!console_selected) {
        int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
        int i;
        // Set baud for all channels
        for (i = 0;  i < CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS;  i++) {
            CYGACC_CALL_IF_SET_CONSOLE_COMM(i);
	    ret = set_comm_baud_rate(CYGACC_CALL_IF_CONSOLE_PROCS(), rate);
	    if (ret < 0)
		break;
        }
        CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
    } else
#endif
    ret = set_comm_baud_rate(CYGACC_CALL_IF_CONSOLE_PROCS(), rate);

    if (ret < 0)
	diag_printf("Setting console baud rate to %d failed\n", rate);

    return ret;
}

static void
_sleep(int ms)
{
    int i;
    for (i = 0;  i < ms;  i++) {
        CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
    }
}

void
do_baud_rate(int argc, char *argv[])
{
    int new_rate, ret, old_rate;
    bool new_rate_set;
    hal_virtual_comm_table_t *__chan;
    struct option_info opts[1];
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    struct config_option opt;
#endif

    init_opts(&opts[0], 'b', true, OPTION_ARG_TYPE_NUM, 
              (void *)&new_rate, (bool *)&new_rate_set, "new baud rate");
    if (!scan_opts(argc, argv, 1, opts, 1, 0, 0, "")) {
        return;
    }
    __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    if (new_rate_set) {
        diag_printf("Baud rate will be changed to %d - update your settings\n", new_rate);
        _sleep(500);  // Give serial time to flush
        old_rate = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_GETBAUD);
        ret = set_console_baud_rate(new_rate);
        if (ret < 0) {
            if (old_rate > 0) {
                // Try to restore
                set_console_baud_rate(old_rate);
                _sleep(500);  // Give serial time to flush
                diag_printf("\nret = %d\n", ret);
            }
            return;  // Couldn't set the desired rate
        }
        // Make sure this new rate works or back off to previous value
        // Sleep for a few seconds, then prompt to see if it works
        _sleep(3000);  // Give serial time to flush
        if (!verify_action_with_timeout(5000, "Baud rate changed to %d", new_rate)) {
            _sleep(500);  // Give serial time to flush
            set_console_baud_rate(old_rate);
            _sleep(500);  // Give serial time to flush
            return;
        }
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
        opt.type = CONFIG_INT;
        opt.enable = (char *)0;
        opt.enable_sense = 1;
        opt.key = "console_baud_rate";
        opt.dflt = new_rate;
        flash_add_config(&opt, true);
#endif
    } else {
        ret = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_GETBAUD);
        diag_printf("Baud rate = ");
        if (ret <= 0) {
            diag_printf("unknown\n");
        } else {
            diag_printf("%d\n", ret);
        }
    }
}
#endif

//
// Validate an address to see if it is within any known RAM area
//
bool
valid_address(unsigned char *addr)
{
    int seg;

    for (seg = 0;  seg < CYGBLD_REDBOOT_MAX_MEM_SEGMENTS;  seg++) {
        if (mem_segments[seg].start != NO_MEMORY) {
            if ((addr >= mem_segments[seg].start) && (addr < mem_segments[seg].end)) {
                return true;
            }
        }
    }
    return false;
}
