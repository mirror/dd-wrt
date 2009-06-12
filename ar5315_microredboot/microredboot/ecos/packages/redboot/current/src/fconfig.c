//==========================================================================
//
//      fconfig.c
//
//      RedBoot - persistent data storage support (FLASH or EEPROM)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Date:         2000-07-28
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <cyg/io/flash.h>
#ifdef CYGOPT_REDBOOT_FIS
#include <fis.h>
#endif

#ifdef CYGSEM_REDBOOT_FLASH_COMBINED_FIS_AND_CONFIG
// Note horrid intertwining of functions, to save precious FLASH
externC void fis_read_directory(void);
externC void fis_update_directory(void);
#endif

#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_EEPROM
externC void write_eeprom(void *buf, int len);
externC void read_eeprom(void *buf, int len);
#endif

#ifdef CYGSEM_REDBOOT_PLF_ESA_VALIDATE
externC bool cyg_plf_redboot_esa_validate(unsigned char *val);
#endif

#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
externC bool do_flash_init(void);
externC int flash_read(void *flash_base, void *ram_base, int len, void **err_address);
#endif

// Round a quantity up
#define _rup(n,s) ((((n)+(s-1))/s)*s)

#include <flash_config.h>

// Configuration data, saved in FLASH, used to set/update RedBoot
// normal "configuration" data items.
struct _config *config, *backup_config;

// Local data used by these routines
#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
extern void *flash_start, *flash_end;
extern int flash_block_size, flash_num_blocks;
extern int __flash_init;
#ifdef CYGOPT_REDBOOT_FIS
extern void *fis_work_block;
extern void *fis_addr;
extern int fisdir_size;  // Size of FIS directory.
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG_READONLY_FALLBACK
static struct _config  *readonly_config;
#endif
void *cfg_base;   // Location in Flash of config data
int   cfg_size;   // Length of config data - rounded to Flash block size
#endif // FLASH MEDIA

// Prototypes for local functions
static unsigned char *flash_lookup_config(char *key);

static bool config_ok;

#define CONFIG_KEY1    0x0BADFACE
#define CONFIG_KEY2    0xDEADDEAD

#define CONFIG_DONE    0
#define CONFIG_ABORT  -1
#define CONFIG_CHANGED 1
#define CONFIG_OK      2
#define CONFIG_BACK    3
#define CONFIG_BAD     4

// Note: the following options are related.  If 'boot_script' is false, then
// the other values are used in the configuration.  Because of the way
// that configuration tables are generated, they should have names which
// are related.  The configuration options will show up lexicographically
// ordered, thus the peculiar naming.
RedBoot_config_option("Run script at boot",
                      boot_script,
                      ALWAYS_ENABLED, true,
                      CONFIG_BOOL,
                      false
    );
RedBoot_config_option("Boot script",
                      boot_script_data,
                      "boot_script", true,
                      CONFIG_SCRIPT,
                      ""
    );
// Some preprocessor magic for building the [constant] prompt string
#define __cat(s1,c2,s3) s1 #c2 s3
#define _cat(s1,c2,s3) __cat(s1,c2,s3)
RedBoot_config_option(_cat("Boot script timeout (",
                           CYGNUM_REDBOOT_BOOT_SCRIPT_TIMEOUT_RESOLUTION,
                           "ms resolution)"),
                      boot_script_timeout,
                      "boot_script", true,
                      CONFIG_INT,
                      0
    );
#undef __cat
#undef _cat

#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
RedBoot_config_option("Console baud rate",
                      console_baud_rate,
                      ALWAYS_ENABLED, true,
                      CONFIG_INT,
                      CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD
    );
#endif

CYG_HAL_TABLE_BEGIN( __CONFIG_options_TAB__, RedBoot_config_options);
CYG_HAL_TABLE_END( __CONFIG_options_TAB_END__, RedBoot_config_options);

extern struct config_option __CONFIG_options_TAB__[], __CONFIG_options_TAB_END__[];

// 
// Layout of config data
// Each data item is variable length, with the name, type and dependencies
// encoded into the object.
//  offset   contents
//       0   data type
//       1   length of name (N)
//       2   enable sense
//       3   length of enable key (M)
//       4   key name
//     N+4   enable key
//   M+N+4   data value
//

#define CONFIG_OBJECT_TYPE(dp)          (dp)[0]
#define CONFIG_OBJECT_KEYLEN(dp)        (dp)[1]
#define CONFIG_OBJECT_ENABLE_SENSE(dp)  (dp)[2]
#define CONFIG_OBJECT_ENABLE_KEYLEN(dp) (dp)[3]
#define CONFIG_OBJECT_KEY(dp)           ((dp)+4)
#define CONFIG_OBJECT_ENABLE_KEY(dp)    ((dp)+4+CONFIG_OBJECT_KEYLEN(dp))
#define CONFIG_OBJECT_VALUE(dp)         ((dp)+4+CONFIG_OBJECT_KEYLEN(dp)+CONFIG_OBJECT_ENABLE_KEYLEN(dp))

#define LIST_OPT_LIST_ONLY (1)
#define LIST_OPT_NICKNAMES (2)
#define LIST_OPT_FULLNAMES (4)
#define LIST_OPT_DUMBTERM  (8)

static void config_init(void);
static int  config_length(int type);

// Change endianness of config data
void
conf_endian_fixup(void *ptr)
{
#ifdef REDBOOT_FLASH_REVERSE_BYTEORDER
    struct _config *p = (struct _config *)ptr;
    unsigned char *dp = p->config_data;
    void *val_ptr;
    int len;
    cyg_uint16 u16;
    cyg_uint32 u32;

    p->len = CYG_SWAP32(p->len);
    p->key1 = CYG_SWAP32(p->key1);
    p->key2 = CYG_SWAP32(p->key2);
    p->cksum = CYG_SWAP32(p->cksum);

    while (dp < &p->config_data[sizeof(config->config_data)]) {
        len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
            config_length(CONFIG_OBJECT_TYPE(dp));
        val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);

        switch (CONFIG_OBJECT_TYPE(dp)) {
            // Note: the data may be unaligned in the configuration data
        case CONFIG_BOOL:
            if (sizeof(bool) == 2) {
                memcpy(&u16, val_ptr, 2);
                u16 = CYG_SWAP16(u16);
                memcpy(val_ptr, &u16, 2);
            } else if (sizeof(bool) == 4) {
                memcpy(&u32, val_ptr, 4);
                u32 = CYG_SWAP32(u32);
                memcpy(val_ptr, &u32, 4);
            }
            break;
        case CONFIG_INT:
            if (sizeof(unsigned long) == 2) {
                memcpy(&u16, val_ptr, 2);
                u16 = CYG_SWAP16(u16);
                memcpy(val_ptr, &u16, 2);
            } else if (sizeof(unsigned long) == 4) {
                memcpy(&u32, val_ptr, 4);
                u32 = CYG_SWAP32(u32);
                memcpy(val_ptr, &u32, 4);
            }
            break;
        }

        dp += len;
    }
#endif
}

static int
get_config(unsigned char *dp, char *title, int list_opt, char *newvalue )
{
    char line[256], hold_line[256], *sp, *lp;
    int ret;
    bool hold_bool_val, new_bool_val, enable;
    unsigned long hold_int_val, new_int_val;
#ifdef CYGPKG_REDBOOT_NETWORKING
    in_addr_t hold_ip_val, new_ip_val;
    enet_addr_t hold_esa_val;
    int esa_ptr;
    char *esp;
#endif
    void *val_ptr;
    int type;

    if (CONFIG_OBJECT_ENABLE_KEYLEN(dp)) {
        flash_get_config(CONFIG_OBJECT_ENABLE_KEY(dp), &enable, CONFIG_BOOL);
        if (((bool)CONFIG_OBJECT_ENABLE_SENSE(dp) && !enable) ||
            (!(bool)CONFIG_OBJECT_ENABLE_SENSE(dp) && enable)) {
            return CONFIG_OK;  // Disabled field
        }
    }
    lp = line;  *lp = '\0';
    val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
    if (LIST_OPT_NICKNAMES & list_opt)
        diag_printf("%s: ", CONFIG_OBJECT_KEY(dp));
    if (LIST_OPT_FULLNAMES & list_opt) {
        if (title != (char *)NULL) {
            diag_printf("%s: ", title);
        } else {
            diag_printf("%s: ", CONFIG_OBJECT_KEY(dp));
        }
    }
    switch (type = CONFIG_OBJECT_TYPE(dp)) {
    case CONFIG_BOOL:
        memcpy(&hold_bool_val, val_ptr, sizeof(bool));
        lp += diag_sprintf(lp, "%s", hold_bool_val ? "true" : "false");
        break;
    case CONFIG_INT:
        memcpy(&hold_int_val, val_ptr, sizeof(unsigned long));
        lp += diag_sprintf(lp, "%ld", hold_int_val);
        break;
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        lp += diag_sprintf(lp, "%s", inet_ntoa((in_addr_t *)val_ptr));
        if (0 == strcmp("0.0.0.0", line) && !(LIST_OPT_LIST_ONLY & list_opt)) {
            // then we have a deeply unhelpful starting text - kill it off
            // (unless we are just listing all values)
            lp = line;  *lp = '\0';
        }
        break;
    case CONFIG_ESA:
        for (esa_ptr = 0;  esa_ptr < sizeof(enet_addr_t);  esa_ptr++) {
            lp += diag_sprintf(lp, "0x%02X", ((unsigned char *)val_ptr)[esa_ptr]);
            if (esa_ptr < (sizeof(enet_addr_t)-1)) lp += diag_sprintf(lp, ":");
        }
        break;
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
    case CONFIG_NETPORT:
        lp += diag_sprintf(lp, "%s", (unsigned char *)val_ptr);
        break;
#endif
#endif
    case CONFIG_STRING:
        lp += diag_sprintf(lp, "%s", (unsigned char *)val_ptr);
        break;
    case CONFIG_SCRIPT:
        diag_printf("\n");
        sp = lp = (unsigned char *)val_ptr;
        while (*sp) {
            while (*lp != '\n') lp++;
            *lp = '\0';
            diag_printf(".. %s\n", sp);
            *lp++ = '\n';
            sp = lp;
        }
        break;
    }
    if (LIST_OPT_LIST_ONLY & list_opt) {
        diag_printf("%s\n", line);
        return CONFIG_OK;
    }
    if (type != CONFIG_SCRIPT) {
        if (NULL != newvalue) {
            ret = strlen(newvalue);
            if (ret > sizeof(line))
                return CONFIG_BAD;
            strcpy(hold_line, line); // Hold the old value for comparison
            strcpy(line, newvalue);
            diag_printf("Setting to %s\n", newvalue);
        } else {
            // read from terminal
            strcpy(hold_line, line);
            if (LIST_OPT_DUMBTERM & list_opt) {
                diag_printf( (CONFIG_STRING == type ?
                              "%s > " :
                              "%s ? " ), line);
                *line = '\0';
            }
            ret = _rb_gets_preloaded(line, sizeof(line), 0);
        }
        if (ret < 0) return CONFIG_ABORT;
        // empty input - leave value untouched (else DNS goes away for a
        // minute to try to look it up) but we must accept empty value for strings.
        if (0 == line[0] && CONFIG_STRING != type) return CONFIG_OK; 
        if (strcmp(line, hold_line) == 0) return CONFIG_OK;  // Just a CR - leave value untouched
        lp = &line[strlen(line)-1];
        if (*lp == '.') return CONFIG_DONE;
        if (*lp == '^') return CONFIG_BACK;
    }
    switch (type) {
    case CONFIG_BOOL:
        memcpy(&hold_bool_val, val_ptr, sizeof(bool));
        if (!parse_bool(line, &new_bool_val)) {
            return CONFIG_BAD;
        }
        if (hold_bool_val != new_bool_val) {
            memcpy(val_ptr, &new_bool_val, sizeof(bool));
            return CONFIG_CHANGED;
        } else {
            return CONFIG_OK;
        }
        break;
    case CONFIG_INT:
        memcpy(&hold_int_val, val_ptr, sizeof(unsigned long));
        if (!parse_num(line, &new_int_val, 0, 0)) {
            return CONFIG_BAD;
        }
        if (hold_int_val != new_int_val) {
            memcpy(val_ptr, &new_int_val, sizeof(unsigned long));
            return CONFIG_CHANGED;
        } else {
            return CONFIG_OK;
        }
        break;
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        memcpy(&hold_ip_val.s_addr, &((in_addr_t *)val_ptr)->s_addr, sizeof(in_addr_t));
        if (!_gethostbyname(line, &new_ip_val)) {
            return CONFIG_BAD;
        }
        if (hold_ip_val.s_addr != new_ip_val.s_addr) {
            memcpy(val_ptr, &new_ip_val, sizeof(in_addr_t));
            return CONFIG_CHANGED;
        } else {
            return CONFIG_OK;
        }
        break;
    case CONFIG_ESA:
        memcpy(&hold_esa_val, val_ptr, sizeof(enet_addr_t));
        esp = line;
        for (esa_ptr = 0;  esa_ptr < sizeof(enet_addr_t);  esa_ptr++) {
            unsigned long esa_byte;
            if (!parse_num(esp, &esa_byte, &esp, ":")) {
                memcpy(val_ptr, &hold_esa_val, sizeof(enet_addr_t));
                return CONFIG_BAD;
            }
            ((unsigned char *)val_ptr)[esa_ptr] = esa_byte;
        }
#ifdef CYGSEM_REDBOOT_PLF_ESA_VALIDATE
        if (!cyg_plf_redboot_esa_validate(val_ptr)) {
            memcpy(val_ptr, &hold_esa_val, sizeof(enet_addr_t));
            return CONFIG_BAD;
        }
#endif
        return CONFIG_CHANGED;
        break;
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
    case CONFIG_NETPORT:
	if (strlen(line) >= MAX_STRING_LENGTH || net_devindex(line) < 0) {
	    int index;
	    const char *name;
	    diag_printf("Sorry, Port name must be one of:\n");
	    for (index = 0; (name = net_devname(index)) != NULL; index++)
		diag_printf("    %s\n", name);
            return CONFIG_BAD;
	}
        strcpy((unsigned char *)val_ptr, line);
	break;
#endif
#endif
    case CONFIG_SCRIPT:
        // Assume it always changes
        sp = (unsigned char *)val_ptr;
        diag_printf("Enter script, terminate with empty line\n");
        while (true) {
            *sp = '\0';
            diag_printf(">> ");
            ret = _rb_gets(line, sizeof(line), 0);
            if (ret < 0) return CONFIG_ABORT;
            if (strlen(line) == 0) break;
            lp = line;
            while (*lp) {
                *sp++ = *lp++;
            }
            *sp++ = '\n';
        }
        break;
    case CONFIG_STRING:
        if (strlen(line) >= MAX_STRING_LENGTH) {
            diag_printf("Sorry, value is too long\n");
            return CONFIG_BAD;
        }
        strcpy((unsigned char *)val_ptr, line);
        break;
    }
    return CONFIG_CHANGED;
}

//
// Manage configuration information with the FLASH
//

static int
config_length(int type)
{
    switch (type) {
    case CONFIG_BOOL:
        return sizeof(bool);
    case CONFIG_INT:
        return sizeof(unsigned long);
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        return sizeof(in_addr_t);
    case CONFIG_ESA:
        // Would like this to be sizeof(enet_addr_t), but that causes much
        // pain since it fouls the alignment of data which follows.
        return 8;
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
    case CONFIG_NETPORT:
        return MAX_STRING_LENGTH;
#endif
#endif
    case CONFIG_STRING:
        return MAX_STRING_LENGTH;
    case CONFIG_SCRIPT:
        return MAX_SCRIPT_LENGTH;
    default:
        return 0;
    }
}

static cmd_fun do_flash_config;
RedBoot_cmd("fconfig",
            "Manage configuration kept in FLASH memory",
            "[-i] [-l] [-n] [-f] [-d] | [-d] nickname [value]",
            do_flash_config
    );


static void
do_flash_config(int argc, char *argv[])
{
    bool need_update = false;
    struct config_option *optend = __CONFIG_options_TAB_END__;
    struct config_option *opt = __CONFIG_options_TAB__;
    struct option_info opts[5];
    bool list_only;
    bool nicknames;
    bool fullnames;
    bool dumbterminal;
    int list_opt = 0;
    unsigned char *dp;
    int len, ret;
    char *title;
    char *onlyone = NULL;
    char *onevalue = NULL;
    bool doneone = false;
    bool init = false;

#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
    if (!__flash_init) {
        diag_printf("Sorry, no FLASH memory is available\n");
        return;
    }
#endif


    memcpy(backup_config, config, sizeof(struct _config));
    script = (unsigned char *)0;

    init_opts(&opts[0], 'l', false, OPTION_ARG_TYPE_FLG, 
              (void *)&list_only, (bool *)0, "list configuration only");
    init_opts(&opts[1], 'n', false, OPTION_ARG_TYPE_FLG, 
              (void *)&nicknames, (bool *)0, "show nicknames");
    init_opts(&opts[2], 'f', false, OPTION_ARG_TYPE_FLG, 
              (void *)&fullnames, (bool *)0, "show full names");
    init_opts(&opts[3], 'i', false, OPTION_ARG_TYPE_FLG, 
              (void *)&init, (bool *)0, "initialize configuration database");
    init_opts(&opts[4], 'd', false, OPTION_ARG_TYPE_FLG, 
              (void *)&dumbterminal, (bool *)0, "dumb terminal: no clever edits");

    // First look to see if we are setting or getting a single option
    // by just quoting its nickname
    if ( (2 == argc && '-' != argv[1][0]) ||
         (3 == argc && '-' != argv[1][0] && '-' != argv[2][0])) {
        // then the command was "fconfig foo [value]"
        onlyone = argv[1];
        onevalue = (3 == argc) ? argv[2] : NULL;
        list_opt = LIST_OPT_NICKNAMES;
    }
    // Next see if we are setting or getting a single option with a dumb
    // terminal invoked, ie. no line editing.
    else if (3 == argc &&
             '-' == argv[1][0] && 'd' == argv[1][1] && 0 == argv[1][2] && 
             '-' != argv[2][0]) {
        // then the command was "fconfig -d foo"
        onlyone = argv[2];
        onevalue = NULL;
        list_opt = LIST_OPT_NICKNAMES | LIST_OPT_DUMBTERM;
    }
    else {
        if (!scan_opts(argc, argv, 1, opts, 5, 0, 0, ""))
            return;
        list_opt |= list_only ? LIST_OPT_LIST_ONLY : 0;
        list_opt |= nicknames ? LIST_OPT_NICKNAMES : LIST_OPT_FULLNAMES;
        list_opt |= fullnames ? LIST_OPT_FULLNAMES : 0;
        list_opt |= dumbterminal ? LIST_OPT_DUMBTERM : 0;
    }

    if (init && verify_action("Initialize non-volatile configuration")) {
        config_init();
        need_update = true;
    }

    dp = &config->config_data[0];
    while (dp < &config->config_data[sizeof(config->config_data)]) {
        if (CONFIG_OBJECT_TYPE(dp) == CONFIG_EMPTY) {
            break;
        }
        len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) + 
            config_length(CONFIG_OBJECT_TYPE(dp));
        // Provide a title for well known [i.e. builtin] objects
        title = (char *)NULL;
        opt = __CONFIG_options_TAB__;
        while (opt != optend) {
            if (strcmp(opt->key, CONFIG_OBJECT_KEY(dp)) == 0) {
                title = opt->title;
                break;
            }
            opt++;
        }
        if ( onlyone && 0 != strcmp(CONFIG_OBJECT_KEY(dp), onlyone) )
            ret = CONFIG_OK; // skip this entry
        else {
            doneone = true;
            ret = get_config(dp, title, list_opt, onevalue); // do this opt
        }
        switch (ret) {
        case CONFIG_DONE:
            goto done;
        case CONFIG_ABORT:
            memcpy(config, backup_config, sizeof(struct _config));
            return;
        case CONFIG_CHANGED:
            need_update = true;
        case CONFIG_OK:
            dp += len;
            break;
        case CONFIG_BACK:
            dp = &config->config_data[0];
            continue;
        case CONFIG_BAD:
            // Nothing - make him do it again
            diag_printf ("** invalid entry\n");
            onevalue = NULL; // request a good value be typed in - or abort/whatever
        }
    }

 done:
    if (NULL != onlyone && !doneone) {
#ifdef CYGSEM_REDBOOT_ALLOW_DYNAMIC_FLASH_CONFIG_DATA
        if (verify_action("** entry '%s' not found - add", onlyone)) {
            struct config_option opt;
            diag_printf("Trying to add value\n");
        }
#else
        diag_printf("** entry '%s' not found\n", onlyone);
#endif
    }
    if (!need_update)
        return;
    flash_write_config(true);
}


#ifdef CYGSEM_REDBOOT_FLASH_ALIASES
static cmd_fun do_alias;
RedBoot_cmd("alias",
            "Manage aliases kept in FLASH memory",
            "name [value]",
            do_alias
    );

static void
make_alias(char *alias, char *name)
{
    diag_sprintf(alias, "alias/%s", name);
}

static void
do_alias(int argc, char *argv[])
{
    char name[80];
    char *val;
    struct config_option opt;

    switch (argc) {
    case 2:
        make_alias(name, argv[1]);
        if (flash_get_config(name, &val, CONFIG_STRING)) {
            diag_printf("'%s' = '%s'\n", argv[1], val);
        } else {
            diag_printf("'%s' not found\n", argv[1]);
        }
        break;
    case 3:
        if (strlen(argv[2]) >= MAX_STRING_LENGTH) {
            diag_printf("Sorry, value is too long\n");
            break;
        }
        make_alias(name, argv[1]);
        opt.type = CONFIG_STRING;
        opt.enable = (char *)0;
        opt.enable_sense = 1;
        opt.key = name;
        opt.dflt = (CYG_ADDRESS)argv[2];
        flash_add_config(&opt, true);
        break;
    default:
        diag_printf("usage: alias name [value]\n");
    }
}

// Lookup an alias. First try plain string aliases. If that fails try
// other types so allowing access to all configured values. This allows
// for alias (macro) expansion of normal 'fconfig' data, such as the
// board IP address.
char *
flash_lookup_alias(char *alias, char *alias_buf)
{
    char name[80];
    char *val;
    unsigned char * dp;
    void *val_ptr;
    int type;
    bool hold_bool_val;
    long hold_long_val;
#ifdef CYGPKG_REDBOOT_NETWORKING
    int esa_ptr;
#endif

    make_alias(name, alias);
    if (flash_get_config(name, &val, CONFIG_STRING)) {
        return val;
    } else {
        dp = flash_lookup_config(alias);
        if (dp) {
            val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
            switch (type = CONFIG_OBJECT_TYPE(dp)) {
            case CONFIG_BOOL:
                memcpy(&hold_bool_val, val_ptr, sizeof(bool));
                diag_sprintf(alias_buf, "%s", hold_bool_val ? "true" : "false");
                break;
            case CONFIG_INT:
                memcpy(&hold_long_val, val_ptr, sizeof(unsigned long));
                diag_sprintf(alias_buf,"%ld", hold_long_val);
                break;
#ifdef CYGPKG_REDBOOT_NETWORKING
            case CONFIG_IP:
                diag_sprintf(alias_buf,"%s", inet_ntoa((in_addr_t *)val_ptr));
                break;
            case CONFIG_ESA:
                for (esa_ptr = 0;  esa_ptr < sizeof(enet_addr_t);  esa_ptr++) {
                    diag_sprintf(alias_buf+(3*esa_ptr), "0x%02X", ((unsigned char *)val_ptr)[esa_ptr]);
                    if (esa_ptr < (sizeof(enet_addr_t)-1)) diag_printf(":");
                }
                break;
#endif
            case CONFIG_SCRIPT:
                return (char *) val_ptr;
                break;
            default:
                return (char *)NULL;
            }
            return alias_buf;
        } 
        return (char *)NULL;
    }
}

#endif //  CYGSEM_REDBOOT_FLASH_ALIASES


//
// Write the in-memory copy of the configuration data to the flash device.
//
void
flash_write_config(bool prompt)
{
#if defined(CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH)
    void *err_addr;
#if !defined(CYGSEM_REDBOOT_FLASH_COMBINED_FIS_AND_CONFIG)
    int stat;
#endif
#endif

    config->len = sizeof(struct _config);
    config->key1 = CONFIG_KEY1;  
    config->key2 = CONFIG_KEY2;
    config->cksum = 0;//flash_crc(config);
    if (!prompt || verify_action("Update RedBoot non-volatile configuration")) {
#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
#ifdef CYGSEM_REDBOOT_FLASH_COMBINED_FIS_AND_CONFIG
        fis_read_directory();
        fis_update_directory();
#else 
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the config page is unlocked before trying to update
        flash_unlock((void *)cfg_base, cfg_size, (void **)&err_addr);
#endif
        if ((stat = flash_erase(cfg_base, cfg_size, (void **)&err_addr)) != 0) {
            diag_printf("   initialization failed at %p: %s\n", err_addr, flash_errmsg(stat));
        } else {
            conf_endian_fixup(config);
            if ((stat = FLASH_PROGRAM(cfg_base, config, sizeof(struct _config), (void **)&err_addr)) != 0) {
                diag_printf("Error writing config data at %p: %s\n", 
                            err_addr, flash_errmsg(stat));
            }
            conf_endian_fixup(config);
        }
#ifdef CYGSEM_REDBOOT_FLASH_LOCK_SPECIAL
        // Insure [quietly] that the config data is locked after the update
        flash_lock((void *)cfg_base, cfg_size, (void **)&err_addr);
#endif
#endif // CYGSEM_REDBOOT_FLASH_COMBINED_FIS_AND_CONFIG
#else  // CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
        write_eeprom(config, sizeof(struct _config));  // into 'config'
#endif
    }
}

//
// Find the configuration entry for a particular key
//
static unsigned char *
flash_lookup_config(char *key)
{
    unsigned char *dp;
    int len;

    if (!config_ok) return (unsigned char *)NULL;

    dp = &config->config_data[0];
    while (dp < &config->config_data[sizeof(config->config_data)]) {
        len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
            config_length(CONFIG_OBJECT_TYPE(dp));
        if (strcmp(key, CONFIG_OBJECT_KEY(dp)) == 0) {
            return dp;
        }
        dp += len;
    }
//    diag_printf("Can't find config data for '%s'\n", key);
    return false;
}

//
// Enumerate the keys from the configuration
//
bool
flash_next_key(char *key, int keylen, int *type, int *offset)
{
    unsigned char *dp;
    int len;

    if (!config_ok) return false;
    if ((*offset < 0) || (*offset >= MAX_CONFIG_DATA)) return false;

    dp = &config->config_data[*offset];
    if ((*type = CONFIG_OBJECT_TYPE(dp)) == CONFIG_EMPTY) return false;
    if ((len = CONFIG_OBJECT_KEYLEN(dp)) > keylen) return false;        
    memcpy(key, CONFIG_OBJECT_KEY(dp), len);
    *offset += 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
        config_length(CONFIG_OBJECT_TYPE(dp));
    return true;
}

//
// Retrieve a data object from the data base (in memory copy)
//
bool
flash_get_config(char *key, void *val, int type)
{
    unsigned char *dp;
    void *val_ptr;
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG_READONLY_FALLBACK
    struct _config *save_config = 0;
    bool res;
#endif

    if (!config_ok) return false;

    if ((dp = flash_lookup_config(key)) != (unsigned char *)NULL) {
        if (CONFIG_OBJECT_TYPE(dp) == type) {
            val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
            switch (type) {
                // Note: the data may be unaligned in the configuration data
            case CONFIG_BOOL:
                memcpy(val, val_ptr, sizeof(bool));
                break;
            case CONFIG_INT:
                memcpy(val, val_ptr, sizeof(unsigned long));
                break;
#ifdef CYGPKG_REDBOOT_NETWORKING
            case CONFIG_IP:
                memcpy(val, val_ptr, sizeof(in_addr_t));
                break;
            case CONFIG_ESA:
                memcpy(val, val_ptr, sizeof(enet_addr_t));
                break;
#endif
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
	    case CONFIG_NETPORT:
#endif
            case CONFIG_STRING:
            case CONFIG_SCRIPT:
                // Just return a pointer to the script/line
                *(unsigned char **)val = (unsigned char *)val_ptr;
                break;
            }
        } else {
            diag_printf("Request for config value '%s' - wrong type\n", key);
        }
        return true;
    }
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG_READONLY_FALLBACK
    // Did not find key. Is configuration data valid?
    // Check to see if the config data is valid, if not, revert to 
    // readonly mode, by setting config to readonly_config.  We
    // will set it back before we leave this function.
    if ( (config != readonly_config) && ((config->key1 != CONFIG_KEY1)|| (config->key2 != CONFIG_KEY2))) {
        save_config = config;
        config = readonly_config;
        if ((config->key1 != CONFIG_KEY1)|| (config->key2 != CONFIG_KEY2)) {
            diag_printf("FLASH configuration checksum error or invalid key\n");
            config = save_config;
            return false;
        }
        else{
            diag_printf("Getting config information in READONLY mode\n");
            res = flash_get_config(key, val, type);
            config = save_config;
            return res;
        }        
    }
#endif
    return false;
}

//
// Update a data object in the data base (in memory copy & backing store)
//
bool
flash_set_config(char *key, void *val, int type)
{
    unsigned char *dp;
    void *val_ptr;

    if (!config_ok) return false;

    if ((dp = flash_lookup_config(key)) != (unsigned char *)NULL) {
        if (CONFIG_OBJECT_TYPE(dp) == type) {
            val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);
            switch (type) {
                // Note: the data may be unaligned in the configuration data
            case CONFIG_BOOL:
                memcpy(val_ptr, val, sizeof(bool));
                break;
            case CONFIG_INT:
                memcpy(val_ptr, val, sizeof(unsigned long));
                break;
#ifdef CYGPKG_REDBOOT_NETWORKING
            case CONFIG_IP:
                memcpy(val_ptr, val, sizeof(in_addr_t));
                break;
            case CONFIG_ESA:
                memcpy(val_ptr, val, sizeof(enet_addr_t));
                break;
#endif
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
	    case CONFIG_NETPORT:
#endif
            case CONFIG_STRING:
            case CONFIG_SCRIPT:
                memcpy(val_ptr, val, config_length(CONFIG_STRING));
                break;
            }
        } else {
            diag_printf("Can't set config value '%s' - wrong type\n", key);
            return false;
        }
        flash_write_config(false);
        return true;
    }
    return false;
}

//
// Copy data into the config area
//
static void
flash_config_insert_value(unsigned char *dp, struct config_option *opt)
{
    switch (opt->type) {
        // Note: the data may be unaligned in the configuration data
    case CONFIG_BOOL:
        memcpy(dp, (void *)&opt->dflt, sizeof(bool));
        break;
    case CONFIG_INT:
        memcpy(dp, (void *)&opt->dflt, sizeof(unsigned long));
        break;
#ifdef CYGPKG_REDBOOT_NETWORKING
    case CONFIG_IP:
        memcpy(dp, (void *)&opt->dflt, sizeof(in_addr_t));
        break;
    case CONFIG_ESA:
        memcpy(dp, (void *)opt->dflt, sizeof(enet_addr_t));
        break;
#if defined(CYGHWR_NET_DRIVERS) && (CYGHWR_NET_DRIVERS > 1)
    case CONFIG_NETPORT:
	// validate dflt and if not acceptable use first port
        {
	    int index;
	    const char *name;
	    for (index = 0; (name = net_devname(index)) != NULL; index++)
		if (!strcmp((char *)opt->dflt, name))
		    break;
	    if (name == NULL)
		name = net_devname(0);
	    memcpy(dp, name, strlen(name) + 1);
        }
        break;
#endif
#endif
    case CONFIG_STRING:
        memcpy(dp, (void *)opt->dflt, config_length(CONFIG_STRING));
        break;
    case CONFIG_SCRIPT:
        break;
    }
}

//
// Add a new option to the database
//
bool
flash_add_config(struct config_option *opt, bool update)
{
    unsigned char *dp, *kp;
    int len, elen, size;

    // If data item is already present, just update it
    // Note: only the data value can be thusly changed
    if ((dp = flash_lookup_config(opt->key)) != (unsigned char *)NULL) {
        flash_config_insert_value(CONFIG_OBJECT_VALUE(dp), opt);
        if (update) {
            flash_write_config(true);
        }
        return true;
    }
    // Add the data item
    dp = &config->config_data[0];
    size = 0;
    while (size < sizeof(config->config_data)) {
        if (CONFIG_OBJECT_TYPE(dp) == CONFIG_EMPTY) {
            kp = opt->key;
            len = strlen(kp) + 1;
            size += len + 2 + 2 + config_length(opt->type);
            if (opt->enable) {
                elen = strlen(opt->enable) + 1;
                size += elen;
            } else {
                elen = 0;
            }
            if (size > sizeof(config->config_data)) {
                break;
            }
            CONFIG_OBJECT_TYPE(dp) = opt->type; 
            CONFIG_OBJECT_KEYLEN(dp) = len;
            CONFIG_OBJECT_ENABLE_SENSE(dp) = opt->enable_sense;
            CONFIG_OBJECT_ENABLE_KEYLEN(dp) = elen;
            dp = CONFIG_OBJECT_KEY(dp);
            while (*kp) *dp++ += *kp++;
            *dp++ = '\0';    
            if (elen) {
                kp = opt->enable;
                while (*kp) *dp++ += *kp++;
                *dp++ = '\0';    
            }
            flash_config_insert_value(dp, opt);
            if (update) {
                flash_write_config(true);
            }
            return true;
        } else {
            len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) +
                config_length(CONFIG_OBJECT_TYPE(dp));
            dp += len;
            size += len;
        }
    }
    diag_printf("No space to add '%s'\n", opt->key);
    return false;
}

//
// Reset/initialize configuration data - used only when starting from scratch
//
static void
config_init(void)
{
    // Well known option strings
    struct config_option *optend = __CONFIG_options_TAB_END__;
    struct config_option *opt = __CONFIG_options_TAB__;

    memset(config, 0, sizeof(struct _config));
    while (opt != optend) {
        if (!flash_add_config(opt, false)) {
            return;
        }
        opt++;
    }
    config_ok = true;
}

//
// Attempt to get configuration information from the FLASH.
// If available (i.e. good checksum, etc), initialize "known"
// values for later use.
//
static void
load_flash_config(void)
{
    bool use_boot_script;
    unsigned char *cfg_temp = (unsigned char *)workspace_end;
#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
    void *err_addr;
#endif

    config_ok = false;
    script = (unsigned char *)0;
    cfg_temp -= sizeof(struct _config);  // Space for primary config data
    config = (struct _config *)cfg_temp;
    cfg_temp -= sizeof(struct _config);  // Space for backup config data
    backup_config = (struct _config *)cfg_temp;
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG_READONLY_FALLBACK
    cfg_temp -= sizeof(struct _config);  // Space for readonly copy of config data
    readonly_config = (struct _config *)cfg_temp;
#endif
    workspace_end = cfg_temp;
#ifdef CYGHWR_REDBOOT_FLASH_CONFIG_MEDIA_FLASH
    if (!do_flash_init()) return;
#ifdef CYGSEM_REDBOOT_FLASH_COMBINED_FIS_AND_CONFIG
    cfg_size = _rup(sizeof(struct _config), sizeof(struct fis_image_desc));
    if ((fisdir_size-cfg_size) < (CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_COUNT *
                                  CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_SIZE)) {
        // Too bad this can't be checked at compile/build time
        diag_printf("Sorry, FLASH config exceeds available space in FIS directory\n");
        return;
    }
    cfg_base = (void *)(((CYG_ADDRESS)fis_addr + fisdir_size) - cfg_size);
    fisdir_size -= cfg_size;
#else
    cfg_size = (flash_block_size > sizeof(struct _config)) ? 
        sizeof(struct _config) : 
        _rup(sizeof(struct _config), flash_block_size);
    if (CYGNUM_REDBOOT_FLASH_CONFIG_BLOCK < 0) {
        cfg_base = (void *)((CYG_ADDRESS)flash_end + 1 -
           _rup(_rup((-CYGNUM_REDBOOT_FLASH_CONFIG_BLOCK*flash_block_size), cfg_size), flash_block_size));
    } else {
        cfg_base = (void *)((CYG_ADDRESS)flash_start + 
           _rup(_rup((CYGNUM_REDBOOT_FLASH_CONFIG_BLOCK*flash_block_size), cfg_size), flash_block_size));
    }
#endif
    FLASH_READ(cfg_base, config, sizeof(struct _config), &err_addr);
    conf_endian_fixup(config);
#else
    read_eeprom(config, sizeof(struct _config));  // into 'config'
#endif
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG_READONLY_FALLBACK
    memcpy(readonly_config, config, sizeof(struct _config));
#endif
    if ((config->key1 != CONFIG_KEY1)|| (config->key2 != CONFIG_KEY2)) {
        diag_printf("**Warning** FLASH configuration checksum error or invalid key\n");
        diag_printf("Use 'fconfig -i' to [re]initialize database\n");
        config_init();
        return;
    }
    config_ok = true;
    flash_get_config("boot_script", &use_boot_script, CONFIG_BOOL);
    if (use_boot_script) {
        flash_get_config("boot_script_data", &script, CONFIG_SCRIPT);
        flash_get_config("boot_script_timeout", &script_timeout, CONFIG_INT);
    }
#ifdef CYGSEM_REDBOOT_VARIABLE_BAUD_RATE
    if (flash_get_config("console_baud_rate", &console_baud_rate, CONFIG_INT)) {
        extern int set_console_baud_rate(int);
        set_console_baud_rate(console_baud_rate);
    }
#endif
}

RedBoot_init(load_flash_config, RedBoot_INIT_SECOND);

// EOF fconfig.c
