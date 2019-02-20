/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_GLOBALS_H
#define _LVM_GLOBALS_H

#define VERBOSE_BASE_LEVEL _LOG_WARN
#define SECURITY_LEVEL 0
#define PV_MIN_SIZE_KB 512

enum dev_ext_e;

void init_verbose(int level);
void init_silent(int silent);
void init_test(int level);
void init_use_aio(int useaio);
void init_md_filtering(int level);
void init_internal_filtering(int level);
void init_fwraid_filtering(int level);
void init_pvmove(int level);
void init_external_device_info_source(enum dev_ext_e src);
void init_obtain_device_list_from_udev(int device_list_from_udev);
void init_trust_cache(int trustcache);
void init_debug(int level);
void init_debug_classes_logged(int classes);
void init_cmd_name(int status);
void init_security_level(int level);
void init_mirror_in_sync(int in_sync);
void init_dmeventd_monitor(int reg);
void init_disable_dmeventd_monitoring(int disable);
void init_background_polling(int polling);
void init_ignore_suspended_devices(int ignore);
void init_ignore_lvm_mirrors(int scan);
void init_error_message_produced(int produced);
void init_is_static(unsigned value);
void init_udev_checking(int checking);
void init_pv_min_size(uint64_t sectors);
void init_activation_checks(int checks);
void init_retry_deactivation(int retry);
void init_unknown_device_name(const char *name);

void set_cmd_name(const char *cmd_name);
const char *get_cmd_name(void);
void set_sysfs_dir_path(const char *path);

int test_mode(void);
int use_aio(void);
int md_filtering(void);
int internal_filtering(void);
int fwraid_filtering(void);
int pvmove_mode(void);
int obtain_device_list_from_udev(void);
enum dev_ext_e external_device_info_source(void);
int trust_cache(void);
int verbose_level(void);
int silent_mode(void);
int debug_level(void);
int debug_class_is_logged(int class);
int security_level(void);
int mirror_in_sync(void);
int background_polling(void);
int ignore_suspended_devices(void);
int ignore_lvm_mirrors(void);
const char *log_command_name(void);
unsigned is_static(void);
int udev_checking(void);
const char *sysfs_dir_path(void);
uint64_t pv_min_size(void);
int activation_checks(void);
int retry_deactivation(void);
const char *unknown_device_name(void);

#define DMEVENTD_MONITOR_IGNORE -1
int dmeventd_monitor_mode(void);

#endif
