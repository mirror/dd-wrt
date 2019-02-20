/*
 * Copyright (C) 2013-2018 Red Hat, Inc. All rights reserved.
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

/*
 * MACROS:
 * - define a configuration section:
 *   cfg_section(id, name, parent, flags, since_version, deprecated_since_version, deprecation_comment, comment)
 *
 * - define a configuration setting of simple type:
 *   cfg(id, name, parent, flags, type, default_value, since_version, unconfigured_default_value, deprecated_since_version, deprecation_comment, comment)
 *
 * - define a configuration array of one or more types:
 *   cfg_array(id, name, parent, flags, types, default_value, since_version, unconfigured_default_value, deprecated_since_version, deprecation_comment, comment)
 *
 * - define a configuration setting where the default value is evaluated in runtime
 *   cfg_runtime(id, name, parent, flags, type, since_version, deprecated_since_version, deprecation_comment, comment)
 *   (for each cfg_runtime, you need to define 'get_default_<name>(struct cmd_context *cmd, struct profile *profile)' function
 *    to get the default value in runtime - usually, these functions are placed in config.[ch] file)
 *
 *
 * If default value can't be assigned statically because it depends on some
 * run-time checks or if it depends on other settings already defined,
 * the configuration setting  or array can be defined with the
 * "{cfg|cfg_array}_runtime" macro. In this case the  default value
 * is evaluated by automatically calling "get_default_<id>" function.
 * See config.h and "function types to evaluate default value at runtime".
 *
 *
 * VARIABLES:
 * 
 * id:                         Unique identifier.
 *
 * name:                       Configuration node name.
 *
 * parent:                     Id of parent configuration node.
 *
 * flags:                      Configuration item flags:
 *                                 CFG_NAME_VARIABLE - configuration node name is variable
 *                                 CFG_ALLOW_EMPTY - node value can be emtpy
 *                                 CFG_ADVANCED - this node belongs to advanced config set
 *                                 CFG_UNSUPPORTED - this node is not officially supported and it's used primarily by developers
 *                                 CFG_PROFILABLE - this node is customizable by a profile
 *                                 CFG_PROFILABLE_METADATA - profilable and attachable to VG/LV metadata
 *                                 CFG_DEFAULT_UNDEFINED - node's default value is undefined (depends on other system/kernel values outside of lvm)
 *                                 CFG_DEFAULT_COMMENTED - node's default value is commented out on output
 *                                 CFG_DISABLED - configuration is disabled (defaults always used)
 *                                 CFG_FORMAT_INT_OCTAL - print integer number in octal form (also prefixed by "0")
 *                                 CFG_SECTION_NO_CHECK - do not check content of the section at all - use with care!!!
 *                                 CFG_DISALLOW_INTERACTIVE - disallow configuration node for use in interactive environment (e.g. cmds run in lvm shell)
 *
 * type:		       Allowed type for the value of simple configuation setting, one of:
 *                                 CFG_TYPE_BOOL
 *                                 CFG_TYPE_INT
 *                                 CFG_TYPE_FLOAT
 *                                 CFG_TYPE_STRING
 *
 * types:                      Allowed types for the values of array configuration setting
 *                             (use logical "OR" to define more than one allowed type,
 *                             e.g. CFG_TYPE_STRING | CFG_TYPE_INT).
 *
 * default_value:              Default value of type 'type' for the configuration node,
 *                             if this is an array with several 'types' defined then
 *                             default value is a string where each string representation
 *                             of each value is prefixed by '#X' where X is one of:
 *                                 'B' for boolean value
 *                                 'I' for integer value
 *                                 'F' for float value
 *                                 'S' for string value
 *                                 '#' for the '#' character itself
 *                             For example, "#Sfd#I16" means default value [ "fd", 16 ].
 *
 * since_version:              The version this configuration node first appeared in (be sure
 *                             that parent nodes are consistent with versioning, no check done
 *                             if parent node is older or the same age as any child node!)
 *                             Use "vsn" macro to translate the "major.minor.release" version
 *                             into a single number that is being stored internally in memory.
 *                             (see also lvmconfig ... --withversions)
 *
 * unconfigured_default_value: Unconfigured default value used as a default value which is
 *                             in "@...@" form and which is then substituted with concrete value
 *                             while running configure.
 *                             (see also 'lvmconfig --type default --unconfigured')
 *
 * deprecated_since_version:   The version since this configuration node is deprecated.
 *
 * deprecation_comment:        Comment about deprecation reason and related info (e.g. which
 *                             configuration is used now instead).
 *
 * comment:                    Comment used in configuration dumps. The very first line is the
 *                             summarizing comment.
 *                             (see also lvmconfig ... --withcomments and --withsummary)
 *
 *
 * Difference between CFG_DEFAULT_COMMENTED and CFG_DEFAULT_UNDEFINED:
 *
 * UNDEFINED is used if default value is NULL or the value
 * depends on other system/kernel values outside of lvm.
 * The most common case is when dm-thin or dm-cache have
 * built-in default settings in the kernel, and lvm will use
 * those built-in default values unless the corresponding lvm
 * config setting is set.
 *
 * COMMENTED is used to comment out the default setting in
 * lvm.conf.  The effect is that if the LVM version is
 * upgraded, and the new version of LVM has new built-in
 * default values, the new defaults are used by LVM unless
 * the previous default value was set (uncommented) in lvm.conf.
 */
#include "lib/config/defaults.h"

cfg_section(root_CFG_SECTION, "(root)", root_CFG_SECTION, 0, vsn(0, 0, 0), 0, NULL, NULL)

#define CFG_PREAMBLE_GENERAL \
	"# This is an example configuration file for the LVM2 system.\n" \
	"# It contains the default settings that would be used if there was no\n" \
	"# @DEFAULT_SYS_DIR@/lvm.conf file.\n" \
	"#\n" \
	"# Refer to 'man lvm.conf' for further information including the file layout.\n" \
	"#\n" \
	"# Refer to 'man lvm.conf' for information about how settings configured in\n" \
	"# this file are combined with built-in values and command line options to\n" \
	"# arrive at the final values used by LVM.\n" \
	"#\n" \
	"# Refer to 'man lvmconfig' for information about displaying the built-in\n" \
	"# and configured values used by LVM.\n" \
	"#\n" \
	"# If a default value is set in this file (not commented out), then a\n" \
	"# new version of LVM using this file will continue using that value,\n" \
	"# even if the new version of LVM changes the built-in default value.\n" \
	"#\n" \
	"# To put this file in a different directory and override @DEFAULT_SYS_DIR@ set\n" \
	"# the environment variable LVM_SYSTEM_DIR before running the tools.\n" \
	"#\n" \
	"# N.B. Take care that each setting only appears once if uncommenting\n" \
	"# example settings in this file.\n\n"

cfg_section(config_CFG_SECTION, "config", root_CFG_SECTION, 0, vsn(2, 2, 99), 0, NULL,
	"How LVM configuration settings are handled.\n")

cfg_section(devices_CFG_SECTION, "devices", root_CFG_SECTION, 0, vsn(1, 0, 0), 0, NULL,
	"How LVM uses block devices.\n")

cfg_section(allocation_CFG_SECTION, "allocation", root_CFG_SECTION, CFG_PROFILABLE, vsn(2, 2, 77), 0, NULL,
	"How LVM selects space and applies properties to LVs.\n")

cfg_section(log_CFG_SECTION, "log", root_CFG_SECTION, CFG_PROFILABLE, vsn(1, 0, 0), 0, NULL,
	"How LVM log information is reported.\n")

cfg_section(backup_CFG_SECTION, "backup", root_CFG_SECTION, 0, vsn(1, 0, 0), 0, NULL,
	"How LVM metadata is backed up and archived.\n"
	"In LVM, a 'backup' is a copy of the metadata for the current system,\n"
	"and an 'archive' contains old metadata configurations. They are\n"
	"stored in a human readable text format.\n")

cfg_section(shell_CFG_SECTION, "shell", root_CFG_SECTION, 0, vsn(1, 0, 0), 0, NULL,
	"Settings for running LVM in shell (readline) mode.\n")

cfg_section(global_CFG_SECTION, "global", root_CFG_SECTION, CFG_PROFILABLE, vsn(1, 0, 0), 0, NULL,
	"Miscellaneous global LVM settings.\n")

cfg_section(activation_CFG_SECTION, "activation", root_CFG_SECTION, CFG_PROFILABLE, vsn(1, 0, 0), 0, NULL, NULL)

cfg_section(metadata_CFG_SECTION, "metadata", root_CFG_SECTION, CFG_DEFAULT_COMMENTED, vsn(1, 0, 0), 0, NULL, NULL)

cfg_section(report_CFG_SECTION, "report", root_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, vsn(1, 0, 0), 0, NULL,
	"LVM report command output formatting.\n")

cfg_section(dmeventd_CFG_SECTION, "dmeventd", root_CFG_SECTION, 0, vsn(1, 2, 3), 0, NULL,
	"Settings for the LVM event daemon.\n")

cfg_section(tags_CFG_SECTION, "tags", root_CFG_SECTION, CFG_DEFAULT_COMMENTED, vsn(1, 0, 18), 0, NULL,
	"Host tag settings.\n")

cfg_section(local_CFG_SECTION, "local", root_CFG_SECTION, 0, vsn(2, 2, 117), 0, NULL,
	"LVM settings that are specific to the local host.\n")

#define CFG_PREAMBLE_LOCAL \
	"# This is a local configuration file template for the LVM2 system\n" \
	"# which should be installed as @DEFAULT_SYS_DIR@/lvmlocal.conf .\n" \
	"#\n" \
	"# Refer to 'man lvm.conf' for information about the file layout.\n" \
	"#\n" \
	"# To put this file in a different directory and override\n" \
	"# @DEFAULT_SYS_DIR@ set the environment variable LVM_SYSTEM_DIR before\n" \
	"# running the tools.\n" \
	"#\n" \
	"# The lvmlocal.conf file is normally expected to contain only the\n" \
	"# \"local\" section which contains settings that should not be shared or\n" \
	"# repeated among different hosts.  (But if other sections are present,\n" \
	"# they *will* get processed.  Settings in this file override equivalent\n" \
	"# ones in lvm.conf and are in turn overridden by ones in any enabled\n" \
	"# lvm_<tag>.conf files.)\n" \
	"#\n" \
	"# Please take care that each setting only appears once if uncommenting\n" \
	"# example settings in this file and never copy this file between hosts.\n\n"

cfg(config_checks_CFG, "checks", config_CFG_SECTION, 0, CFG_TYPE_BOOL, 1, vsn(2, 2, 99), NULL, 0, NULL,
	"If enabled, any LVM configuration mismatch is reported.\n"
	"This implies checking that the configuration key is understood by\n"
	"LVM and that the value of the key is the proper type. If disabled,\n"
	"any configuration mismatch is ignored and the default value is used\n"
	"without any warning (a message about the configuration key not being\n"
	"found is issued in verbose mode only).\n")

cfg(config_abort_on_errors_CFG, "abort_on_errors", config_CFG_SECTION, 0, CFG_TYPE_BOOL, 0, vsn(2,2,99), NULL, 0, NULL,
	"Abort the LVM process if a configuration mismatch is found.\n")

cfg_runtime(config_profile_dir_CFG, "profile_dir", config_CFG_SECTION, CFG_DISALLOW_INTERACTIVE, CFG_TYPE_STRING, vsn(2, 2, 99), 0, NULL,
	"Directory where LVM looks for configuration profiles.\n")

cfg(devices_dir_CFG, "dir", devices_CFG_SECTION, CFG_ADVANCED, CFG_TYPE_STRING, DEFAULT_DEV_DIR, vsn(1, 0, 0), NULL, 0, NULL,
	"Directory in which to create volume group device nodes.\n"
	"Commands also accept this as a prefix on volume group names.\n")

cfg_array(devices_scan_CFG, "scan", devices_CFG_SECTION, CFG_ADVANCED, CFG_TYPE_STRING, "#S/dev", vsn(1, 0, 0), NULL, 0, NULL,
	"Directories containing device nodes to use with LVM.\n")

cfg_array(devices_loopfiles_CFG, "loopfiles", devices_CFG_SECTION, CFG_DEFAULT_UNDEFINED | CFG_UNSUPPORTED, CFG_TYPE_STRING, NULL, vsn(1, 2, 0), NULL, vsn(2, 3, 0), NULL, NULL)

cfg(devices_obtain_device_list_from_udev_CFG, "obtain_device_list_from_udev", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_OBTAIN_DEVICE_LIST_FROM_UDEV, vsn(2, 2, 85), NULL, 0, NULL,
	"Obtain the list of available devices from udev.\n"
	"This avoids opening or using any inapplicable non-block devices or\n"
	"subdirectories found in the udev directory. Any device node or\n"
	"symlink not managed by udev in the udev directory is ignored. This\n"
	"setting applies only to the udev-managed device directory; other\n"
	"directories will be scanned fully. LVM needs to be compiled with\n"
	"udev support for this setting to apply.\n")

cfg(devices_external_device_info_source_CFG, "external_device_info_source", devices_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_EXTERNAL_DEVICE_INFO_SOURCE, vsn(2, 2, 116), NULL, 0, NULL,
	"Select an external device information source.\n"
	"Some information may already be available in the system and LVM can\n"
	"use this information to determine the exact type or use of devices it\n"
	"processes. Using an existing external device information source can\n"
	"speed up device processing as LVM does not need to run its own native\n"
	"routines to acquire this information. For example, this information\n"
	"is used to drive LVM filtering like MD component detection, multipath\n"
	"component detection, partition detection and others.\n"
	"#\n"
	"Accepted values:\n"
	"  none\n"
	"    No external device information source is used.\n"
	"  udev\n"
	"    Reuse existing udev database records. Applicable only if LVM is\n"
	"    compiled with udev support.\n"
	"#\n")

cfg_array(devices_preferred_names_CFG, "preferred_names", devices_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED , CFG_TYPE_STRING, NULL, vsn(1, 2, 19), NULL, 0, NULL,
	"Select which path name to display for a block device.\n"
	"If multiple path names exist for a block device, and LVM needs to\n"
	"display a name for the device, the path names are matched against\n"
	"each item in this list of regular expressions. The first match is\n"
	"used. Try to avoid using undescriptive /dev/dm-N names, if present.\n"
	"If no preferred name matches, or if preferred_names are not defined,\n"
	"the following built-in preferences are applied in order until one\n"
	"produces a preferred name:\n"
	"Prefer names with path prefixes in the order of:\n"
	"/dev/mapper, /dev/disk, /dev/dm-*, /dev/block.\n"
	"Prefer the name with the least number of slashes.\n"
	"Prefer a name that is a symlink.\n"
	"Prefer the path with least value in lexicographical order.\n"
	"#\n"
	"Example\n"
	"preferred_names = [ \"^/dev/mpath/\", \"^/dev/mapper/mpath\", \"^/dev/[hs]d\" ]\n"
	"#\n")

cfg_array(devices_filter_CFG, "filter", devices_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, "#Sa|.*/|", vsn(1, 0, 0), NULL, 0, NULL,
	"Limit the block devices that are used by LVM commands.\n"
	"This is a list of regular expressions used to accept or reject block\n"
	"device path names. Each regex is delimited by a vertical bar '|'\n"
	"(or any character) and is preceded by 'a' to accept the path, or\n"
	"by 'r' to reject the path. The first regex in the list to match the\n"
	"path is used, producing the 'a' or 'r' result for the device.\n"
	"When multiple path names exist for a block device, if any path name\n"
	"matches an 'a' pattern before an 'r' pattern, then the device is\n"
	"accepted. If all the path names match an 'r' pattern first, then the\n"
	"device is rejected. Unmatching path names do not affect the accept\n"
	"or reject decision. If no path names for a device match a pattern,\n"
	"then the device is accepted. Be careful mixing 'a' and 'r' patterns,\n"
	"as the combination might produce unexpected results (test changes.)\n"
	"Run vgscan after changing the filter to regenerate the cache.\n"
	"#\n"
	"Example\n"
	"Accept every block device:\n"
	"filter = [ \"a|.*/|\" ]\n"
	"Reject the cdrom drive:\n"
	"filter = [ \"r|/dev/cdrom|\" ]\n"
	"Work with just loopback devices, e.g. for testing:\n"
	"filter = [ \"a|loop|\", \"r|.*|\" ]\n"
	"Accept all loop devices and ide drives except hdc:\n"
	"filter = [ \"a|loop|\", \"r|/dev/hdc|\", \"a|/dev/ide|\", \"r|.*|\" ]\n"
	"Use anchors to be very specific:\n"
	"filter = [ \"a|^/dev/hda8$|\", \"r|.*/|\" ]\n"
	"#\n")

cfg_array(devices_global_filter_CFG, "global_filter", devices_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, "#Sa|.*/|", vsn(2, 2, 98), NULL, 0, NULL,
	"Limit the block devices that are used by LVM system components.\n"
	"Because devices/filter may be overridden from the command line, it is\n"
	"not suitable for system-wide device filtering, e.g. udev.\n"
	"Use global_filter to hide devices from these LVM system components.\n"
	"The syntax is the same as devices/filter. Devices rejected by\n"
	"global_filter are not opened by LVM.\n")

cfg_runtime(devices_cache_CFG, "cache", devices_CFG_SECTION, 0, CFG_TYPE_STRING, vsn(1, 0, 0), vsn(1, 2, 19), NULL,
	"This setting is no longer used.\n")

cfg_runtime(devices_cache_dir_CFG, "cache_dir", devices_CFG_SECTION, 0, CFG_TYPE_STRING, vsn(1, 2, 19), vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(devices_cache_file_prefix_CFG, "cache_file_prefix", devices_CFG_SECTION, CFG_ALLOW_EMPTY, CFG_TYPE_STRING, DEFAULT_CACHE_FILE_PREFIX, vsn(1, 2, 19), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(devices_write_cache_state_CFG, "write_cache_state", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, 1, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg_array(devices_types_CFG, "types", devices_CFG_SECTION, CFG_DEFAULT_UNDEFINED | CFG_ADVANCED, CFG_TYPE_INT | CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, 0, NULL,
	"List of additional acceptable block device types.\n"
	"These are of device type names from /proc/devices, followed by the\n"
	"maximum number of partitions.\n"
	"#\n"
	"Example\n"
	"types = [ \"fd\", 16 ]\n"
	"#\n")

cfg(devices_sysfs_scan_CFG, "sysfs_scan", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_SYSFS_SCAN, vsn(1, 0, 8), NULL, 0, NULL,
	"Restrict device scanning to block devices appearing in sysfs.\n"
	"This is a quick way of filtering out block devices that are not\n"
	"present on the system. sysfs must be part of the kernel and mounted.)\n")

cfg(devices_scan_lvs_CFG, "scan_lvs", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_SCAN_LVS, vsn(2, 2, 182), NULL, 0, NULL,
	"Scan LVM LVs for layered PVs.\n")

cfg(devices_multipath_component_detection_CFG, "multipath_component_detection", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_MULTIPATH_COMPONENT_DETECTION, vsn(2, 2, 89), NULL, 0, NULL,
	"Ignore devices that are components of DM multipath devices.\n")

cfg(devices_md_component_detection_CFG, "md_component_detection", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_MD_COMPONENT_DETECTION, vsn(1, 0, 18), NULL, 0, NULL,
	"Ignore devices that are components of software RAID (md) devices.\n")

cfg(devices_fw_raid_component_detection_CFG, "fw_raid_component_detection", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_FW_RAID_COMPONENT_DETECTION, vsn(2, 2, 112), NULL, 0, NULL,
	"Ignore devices that are components of firmware RAID devices.\n"
	"LVM must use an external_device_info_source other than none for this\n"
	"detection to execute.\n")

cfg(devices_md_chunk_alignment_CFG, "md_chunk_alignment", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_MD_CHUNK_ALIGNMENT, vsn(2, 2, 48), NULL, 0, NULL,
	"Align PV data blocks with md device's stripe-width.\n"
	"This applies if a PV is placed directly on an md device.\n")

cfg(devices_default_data_alignment_CFG, "default_data_alignment", devices_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_DATA_ALIGNMENT, vsn(2, 2, 75), NULL, 0, NULL,
	"Default alignment of the start of a PV data area in MB.\n"
	"If set to 0, a value of 64KiB will be used.\n"
	"Set to 1 for 1MiB, 2 for 2MiB, etc.\n")

cfg(devices_data_alignment_detection_CFG, "data_alignment_detection", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_DATA_ALIGNMENT_DETECTION, vsn(2, 2, 51), NULL, 0, NULL,
	"Detect PV data alignment based on sysfs device information.\n"
	"The start of a PV data area will be a multiple of minimum_io_size or\n"
	"optimal_io_size exposed in sysfs. minimum_io_size is the smallest\n"
	"request the device can perform without incurring a read-modify-write\n"
	"penalty, e.g. MD chunk size. optimal_io_size is the device's\n"
	"preferred unit of receiving I/O, e.g. MD stripe width.\n"
	"minimum_io_size is used if optimal_io_size is undefined (0).\n"
	"If md_chunk_alignment is enabled, that detects the optimal_io_size.\n"
	"This setting takes precedence over md_chunk_alignment.\n")

cfg(devices_data_alignment_CFG, "data_alignment", devices_CFG_SECTION, 0, CFG_TYPE_INT, 0, vsn(2, 2, 45), NULL, 0, NULL,
	"Alignment of the start of a PV data area in KiB.\n"
	"If a PV is placed directly on an md device and md_chunk_alignment or\n"
	"data_alignment_detection are enabled, then this setting is ignored.\n"
	"Otherwise, md_chunk_alignment and data_alignment_detection are\n"
	"disabled if this is set. Set to 0 to use the default alignment or the\n"
	"page size, if larger.\n")

cfg(devices_data_alignment_offset_detection_CFG, "data_alignment_offset_detection", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_DATA_ALIGNMENT_OFFSET_DETECTION, vsn(2, 2, 50), NULL, 0, NULL,
	"Detect PV data alignment offset based on sysfs device information.\n"
	"The start of a PV aligned data area will be shifted by the\n"
	"alignment_offset exposed in sysfs. This offset is often 0, but may\n"
	"be non-zero. Certain 4KiB sector drives that compensate for windows\n"
	"partitioning will have an alignment_offset of 3584 bytes (sector 7\n"
	"is the lowest aligned logical block, the 4KiB sectors start at\n"
	"LBA -1, and consequently sector 63 is aligned on a 4KiB boundary).\n"
	"pvcreate --dataalignmentoffset will skip this detection.\n")

cfg(devices_ignore_suspended_devices_CFG, "ignore_suspended_devices", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_IGNORE_SUSPENDED_DEVICES, vsn(1, 2, 19), NULL, 0, NULL,
	"Ignore DM devices that have I/O suspended while scanning devices.\n"
	"Otherwise, LVM waits for a suspended device to become accessible.\n"
	"This should only be needed in recovery situations.\n")

cfg(devices_ignore_lvm_mirrors_CFG, "ignore_lvm_mirrors", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_IGNORE_LVM_MIRRORS, vsn(2, 2, 104), NULL, 0, NULL,
	"Do not scan 'mirror' LVs to avoid possible deadlocks.\n"
	"This avoids possible deadlocks when using the 'mirror' segment type.\n"
	"This setting determines whether LVs using the 'mirror' segment type\n"
	"are scanned for LVM labels. This affects the ability of mirrors to\n"
	"be used as physical volumes. If this setting is enabled, it is\n"
	"impossible to create VGs on top of mirror LVs, i.e. to stack VGs on\n"
	"mirror LVs. If this setting is disabled, allowing mirror LVs to be\n"
	"scanned, it may cause LVM processes and I/O to the mirror to become\n"
	"blocked. This is due to the way that the mirror segment type handles\n"
	"failures. In order for the hang to occur, an LVM command must be run\n"
	"just after a failure and before the automatic LVM repair process\n"
	"takes place, or there must be failures in multiple mirrors in the\n"
	"same VG at the same time with write failures occurring moments before\n"
	"a scan of the mirror's labels. The 'mirror' scanning problems do not\n"
	"apply to LVM RAID types like 'raid1' which handle failures in a\n"
	"different way, making them a better choice for VG stacking.\n")

cfg(devices_disable_after_error_count_CFG, "disable_after_error_count", devices_CFG_SECTION, 0, CFG_TYPE_INT, 0, vsn(2, 2, 75), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(devices_require_restorefile_with_uuid_CFG, "require_restorefile_with_uuid", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_REQUIRE_RESTOREFILE_WITH_UUID, vsn(2, 2, 73), NULL, 0, NULL,
	"Allow use of pvcreate --uuid without requiring --restorefile.\n")

cfg(devices_pv_min_size_CFG, "pv_min_size", devices_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_PV_MIN_SIZE_KB, vsn(2, 2, 85), NULL, 0, NULL,
	"Minimum size in KiB of block devices which can be used as PVs.\n"
	"In a clustered environment all nodes must use the same value.\n"
	"Any value smaller than 512KiB is ignored. The previous built-in\n"
	"value was 512.\n")

cfg(devices_issue_discards_CFG, "issue_discards", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_ISSUE_DISCARDS, vsn(2, 2, 85), NULL, 0, NULL,
	"Issue discards to PVs that are no longer used by an LV.\n"
	"Discards are sent to an LV's underlying physical volumes when the LV\n"
	"is no longer using the physical volumes' space, e.g. lvremove,\n"
	"lvreduce. Discards inform the storage that a region is no longer\n"
	"used. Storage that supports discards advertise the protocol-specific\n"
	"way discards should be issued by the kernel (TRIM, UNMAP, or\n"
	"WRITE SAME with UNMAP bit set). Not all storage will support or\n"
	"benefit from discards, but SSDs and thinly provisioned LUNs\n"
	"generally do. If enabled, discards will only be issued if both the\n"
	"storage and kernel provide support.\n")

cfg(devices_allow_changes_with_duplicate_pvs_CFG, "allow_changes_with_duplicate_pvs", devices_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_ALLOW_CHANGES_WITH_DUPLICATE_PVS, vsn(2, 2, 153), NULL, 0, NULL,
	"Allow VG modification while a PV appears on multiple devices.\n"
	"When a PV appears on multiple devices, LVM attempts to choose the\n"
	"best device to use for the PV. If the devices represent the same\n"
	"underlying storage, the choice has minimal consequence. If the\n"
	"devices represent different underlying storage, the wrong choice\n"
	"can result in data loss if the VG is modified. Disabling this\n"
	"setting is the safest option because it prevents modifying a VG\n"
	"or activating LVs in it while a PV appears on multiple devices.\n"
	"Enabling this setting allows the VG to be used as usual even with\n"
	"uncertain devices.\n")

cfg_array(allocation_cling_tag_list_CFG, "cling_tag_list", allocation_CFG_SECTION, CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 77), NULL, 0, NULL,
	"Advise LVM which PVs to use when searching for new space.\n"
	"When searching for free space to extend an LV, the 'cling' allocation\n"
	"policy will choose space on the same PVs as the last segment of the\n"
	"existing LV. If there is insufficient space and a list of tags is\n"
	"defined here, it will check whether any of them are attached to the\n"
	"PVs concerned and then seek to match those PV tags between existing\n"
	"extents and new extents.\n"
	"#\n"
	"Example\n"
	"Use the special tag \"@*\" as a wildcard to match any PV tag:\n"
	"cling_tag_list = [ \"@*\" ]\n"
	"LVs are mirrored between two sites within a single VG, and\n"
	"PVs are tagged with either @site1 or @site2 to indicate where\n"
	"they are situated:\n"
	"cling_tag_list = [ \"@site1\", \"@site2\" ]\n"
	"#\n")

cfg(allocation_maximise_cling_CFG, "maximise_cling", allocation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_MAXIMISE_CLING, vsn(2, 2, 85), NULL, 0, NULL,
	"Use a previous allocation algorithm.\n"
	"Changes made in version 2.02.85 extended the reach of the 'cling'\n"
	"policies to detect more situations where data can be grouped onto\n"
	"the same disks. This setting can be used to disable the changes\n"
	"and revert to the previous algorithm.\n")

cfg(allocation_use_blkid_wiping_CFG, "use_blkid_wiping", allocation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_USE_BLKID_WIPING, vsn(2, 2, 105), "@DEFAULT_USE_BLKID_WIPING@", 0, NULL,
	"Use blkid to detect and erase existing signatures on new PVs and LVs.\n"
	"The blkid library can detect more signatures than the native LVM\n"
	"detection code, but may take longer. LVM needs to be compiled with\n"
	"blkid wiping support for this setting to apply. LVM native detection\n"
	"code is currently able to recognize: MD device signatures,\n"
	"swap signature, and LUKS signatures. To see the list of signatures\n"
	"recognized by blkid, check the output of the 'blkid -k' command.\n")

cfg(allocation_wipe_signatures_when_zeroing_new_lvs_CFG, "wipe_signatures_when_zeroing_new_lvs", allocation_CFG_SECTION, 0, CFG_TYPE_BOOL, 1, vsn(2, 2, 105), NULL, 0, NULL,
	"Look for and erase any signatures while zeroing a new LV.\n"
	"The --wipesignatures option overrides this setting.\n"
	"Zeroing is controlled by the -Z/--zero option, and if not specified,\n"
	"zeroing is used by default if possible. Zeroing simply overwrites the\n"
	"first 4KiB of a new LV with zeroes and does no signature detection or\n"
	"wiping. Signature wiping goes beyond zeroing and detects exact types\n"
	"and positions of signatures within the whole LV. It provides a\n"
	"cleaner LV after creation as all known signatures are wiped. The LV\n"
	"is not claimed incorrectly by other tools because of old signatures\n"
	"from previous use. The number of signatures that LVM can detect\n"
	"depends on the detection code that is selected (see\n"
	"use_blkid_wiping.) Wiping each detected signature must be confirmed.\n"
	"When this setting is disabled, signatures on new LVs are not detected\n"
	"or erased unless the --wipesignatures option is used directly.\n")

cfg(allocation_mirror_logs_require_separate_pvs_CFG, "mirror_logs_require_separate_pvs", allocation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_MIRROR_LOGS_REQUIRE_SEPARATE_PVS, vsn(2, 2, 85), NULL, 0, NULL,
	"Mirror logs and images will always use different PVs.\n"
	"The default setting changed in version 2.02.85.\n")

cfg(allocation_raid_stripe_all_devices_CFG, "raid_stripe_all_devices", allocation_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_ALLOCATION_STRIPE_ALL_DEVICES, vsn(2, 2, 162), NULL, 0, NULL,
	"Stripe across all PVs when RAID stripes are not specified.\n"
	"If enabled, all PVs in the VG or on the command line are used for\n"
	"raid0/4/5/6/10 when the command does not specify the number of\n"
	"stripes to use.\n"
	"This was the default behaviour until release 2.02.162.\n")

cfg(allocation_cache_pool_metadata_require_separate_pvs_CFG, "cache_pool_metadata_require_separate_pvs", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA, CFG_TYPE_BOOL, DEFAULT_CACHE_POOL_METADATA_REQUIRE_SEPARATE_PVS, vsn(2, 2, 106), NULL, 0, NULL,
	"Cache pool metadata and data will always use different PVs.\n")

cfg(allocation_cache_pool_cachemode_CFG, "cache_pool_cachemode", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_CACHE_MODE, vsn(2, 2, 113), NULL, vsn(2, 2, 128),
	"This has been replaced by the allocation/cache_mode setting.\n",
	"Cache mode.\n")

cfg(allocation_cache_metadata_format_CFG, "cache_metadata_format", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_CACHE_METADATA_FORMAT, vsn(2, 2, 169), NULL, 0, NULL,
	"Sets default metadata format for new cache.\n"
	"#\n"
	"Accepted values:\n"
	"  0  Automatically detected best available format\n"
	"  1  Original format\n"
	"  2  Improved 2nd. generation format\n"
	"#\n")

cfg(allocation_cache_mode_CFG, "cache_mode", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_CACHE_MODE, vsn(2, 2, 128), NULL, 0, NULL,
	"The default cache mode used for new cache.\n"
	"#\n"
	"Accepted values:\n"
	"  writethrough\n"
	"    Data blocks are immediately written from the cache to disk.\n"
	"  writeback\n"
	"    Data blocks are written from the cache back to disk after some\n"
	"    delay to improve performance.\n"
	"#\n"
	"This setting replaces allocation/cache_pool_cachemode.\n")

cfg(allocation_cache_policy_CFG, "cache_policy", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, 0, vsn(2, 2, 128), NULL, 0, NULL,
	"The default cache policy used for new cache volume.\n"
	"Since kernel 4.2 the default policy is smq (Stochastic multiqueue),\n"
	"otherwise the older mq (Multiqueue) policy is selected.\n")

cfg_section(allocation_cache_settings_CFG_SECTION, "cache_settings", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, vsn(2, 2, 128), 0, NULL,
	"Settings for the cache policy.\n"
	"See documentation for individual cache policies for more info.\n")

cfg_section(policy_settings_CFG_SUBSECTION, "policy_settings", allocation_cache_settings_CFG_SECTION, CFG_NAME_VARIABLE | CFG_SECTION_NO_CHECK | CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, vsn(2, 2, 128), 0, NULL,
	"Replace this subsection name with a policy name.\n"
	"Multiple subsections for different policies can be created.\n")

cfg_runtime(allocation_cache_pool_chunk_size_CFG, "cache_pool_chunk_size", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_UNDEFINED, CFG_TYPE_INT, vsn(2, 2, 106), 0, NULL,
	"The minimal chunk size in KiB for cache pool volumes.\n"
	"Using a chunk_size that is too large can result in wasteful use of\n"
	"the cache, where small reads and writes can cause large sections of\n"
	"an LV to be mapped into the cache. However, choosing a chunk_size\n"
	"that is too small can result in more overhead trying to manage the\n"
	"numerous chunks that become mapped into the cache. The former is\n"
	"more of a problem than the latter in most cases, so the default is\n"
	"on the smaller end of the spectrum. Supported values range from\n"
	"32KiB to 1GiB in multiples of 32.\n")

cfg(allocation_cache_pool_max_chunks_CFG, "cache_pool_max_chunks", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_UNDEFINED, CFG_TYPE_INT, 0, vsn(2, 2, 165), NULL, 0, NULL,
	"The maximum number of chunks in a cache pool.\n"
	"For cache target v1.9 the recommended maximumm is 1000000 chunks.\n"
	"Using cache pool with more chunks may degrade cache performance.\n")

cfg(allocation_thin_pool_metadata_require_separate_pvs_CFG, "thin_pool_metadata_require_separate_pvs", allocation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_THIN_POOL_METADATA_REQUIRE_SEPARATE_PVS, vsn(2, 2, 89), NULL, 0, NULL,
	"Thin pool metdata and data will always use different PVs.\n")

cfg(allocation_thin_pool_zero_CFG, "thin_pool_zero", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_THIN_POOL_ZERO, vsn(2, 2, 99), NULL, 0, NULL,
	"Thin pool data chunks are zeroed before they are first used.\n"
	"Zeroing with a larger thin pool chunk size reduces performance.\n")

cfg(allocation_thin_pool_discards_CFG, "thin_pool_discards", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_THIN_POOL_DISCARDS, vsn(2, 2, 99), NULL, 0, NULL,
	"The discards behaviour of thin pool volumes.\n"
	"#\n"
	"Accepted values:\n"
	"  ignore\n"
	"  nopassdown\n"
	"  passdown\n"
	"#\n")

cfg(allocation_thin_pool_chunk_size_policy_CFG, "thin_pool_chunk_size_policy", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_THIN_POOL_CHUNK_SIZE_POLICY, vsn(2, 2, 101), NULL, 0, NULL,
	"The chunk size calculation policy for thin pool volumes.\n"
	"#\n"
	"Accepted values:\n"
	"  generic\n"
	"    If thin_pool_chunk_size is defined, use it. Otherwise, calculate\n"
	"    the chunk size based on estimation and device hints exposed in\n"
	"    sysfs - the minimum_io_size. The chunk size is always at least\n"
	"    64KiB.\n"
	"  performance\n"
	"    If thin_pool_chunk_size is defined, use it. Otherwise, calculate\n"
	"    the chunk size for performance based on device hints exposed in\n"
	"    sysfs - the optimal_io_size. The chunk size is always at least\n"
	"    512KiB.\n"
	"#\n")

cfg_runtime(allocation_thin_pool_chunk_size_CFG, "thin_pool_chunk_size", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_UNDEFINED, CFG_TYPE_INT, vsn(2, 2, 99), 0, NULL,
	"The minimal chunk size in KiB for thin pool volumes.\n"
	"Larger chunk sizes may improve performance for plain thin volumes,\n"
	"however using them for snapshot volumes is less efficient, as it\n"
	"consumes more space and takes extra time for copying. When unset,\n"
	"lvm tries to estimate chunk size starting from 64KiB. Supported\n"
	"values are in the range 64KiB to 1GiB.\n")

cfg(allocation_physical_extent_size_CFG, "physical_extent_size", allocation_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_EXTENT_SIZE, vsn(2, 2, 112), NULL, 0, NULL,
	"Default physical extent size in KiB to use for new VGs.\n")

#define VDO_1ST_VSN vsn(2, 3, 0)
cfg(allocation_vdo_use_compression_CFG, "vdo_use_compression", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_USE_COMPRESSION, VDO_1ST_VSN, NULL, 0, NULL,
	"Enables or disables compression when creating a VDO volume.\n"
	"Compression may be disabled if necessary to maximize performance\n"
	"or to speed processing of data that is unlikely to compress.\n")

cfg(allocation_vdo_use_deduplication_CFG, "vdo_use_deduplication", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_USE_DEDUPLICATION, VDO_1ST_VSN, NULL, 0, NULL,
	"Enables or disables deduplication when creating a VDO volume.\n"
	"Deduplication may be disabled in instances where data is not expected\n"
	"to have good deduplication rates but compression is still desired.")

cfg(allocation_vdo_emulate_512_sectors_CFG, "vdo_emulate_512_sectors", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_EMULATE_512_SECTORS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies that the VDO volume is to emulate a 512 byte block device.\n")

cfg(allocation_vdo_block_map_cache_size_mb_CFG, "vdo_block_map_cache_size_mb", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_BLOCK_MAP_CACHE_SIZE_MB, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the amount of memory in MiB allocated for caching block map\n"
	"pages for VDO volume. The value must be a multiple of 4096 and must be\n"
	"at least 128MiB and less than 16TiB. The cache must be at least 16MiB\n"
	"per logical thread. Note that there is a memory overhead of 15%.\n")

cfg(allocation_vdo_block_map_period_CFG, "vdo_block_map_period", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_BLOCK_MAP_PERIOD, VDO_1ST_VSN, NULL, 0, NULL,
	"Tunes the quantity of block map updates that can accumulate\n"
	"before cache pages are flushed to disk. The value must be\n"
	"at least 1 and less then 16380.\n"
	"A lower value means shorter recovery time but lower performance.\n")

cfg(allocation_vdo_check_point_frequency_CFG, "vdo_check_point_frequency", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_CHECK_POINT_FREQUENCY, VDO_1ST_VSN, NULL, 0, NULL,
	"The default check point frequency for VDO volume.\n")

cfg(allocation_vdo_use_sparse_index_CFG, "vdo_use_sparse_index", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_USE_SPARSE_INDEX, VDO_1ST_VSN, NULL, 0, NULL,
	"Enables sparse indexing for VDO volume.\n")

cfg(allocation_vdo_index_memory_size_mb_CFG, "vdo_index_memory_size_mb", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_INDEX_MEMORY_SIZE_MB, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the amount of index memory in MiB for VDO volume.\n"
	"The value must be at least 256MiB and at most 1TiB.\n")

cfg(allocation_vdo_use_read_cache_CFG, "vdo_use_read_cache", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_USE_READ_CACHE, VDO_1ST_VSN, NULL, 0, NULL,
	"Enables or disables the read cache within the VDO volume.\n"
	"The cache should be enabled if write workloads are expected\n"
	"to have high levels of deduplication, or for read intensive\n"
	"workloads of highly compressible data.\n")

cfg(allocation_vdo_read_cache_size_mb_CFG, "vdo_read_cache_size_mb", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_READ_CACHE_SIZE_MB, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the extra VDO volume read cache size in MiB.\n"
	"This space is in addition to a system-defined minimum.\n"
	"The value must be less then 16TiB and 1.12 MiB of memory\n"
	"will be used per MiB of read cache specified, per bio thread.\n")

cfg(allocation_vdo_slab_size_mb_CFG, "vdo_slab_size_mb", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_SLAB_SIZE_MB, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the size in MiB of the increment by which a VDO is grown.\n"
	"Using a smaller size constrains the total maximum physical size\n"
	"that can be accommodated. Must be a power of two between 128MiB and 32GiB.\n")

cfg(allocation_vdo_ack_threads_CFG, "vdo_ack_threads", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_ACK_THREADS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of threads to use for acknowledging\n"
	"completion of requested VDO I/O operations.\n"
	"The value must be at in range [0..100].\n")

cfg(allocation_vdo_bio_threads_CFG, "vdo_bio_threads", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_BIO_THREADS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of threads to use for submitting I/O\n"
	"operations to the storage device of VDO volume.\n"
	"The value must be in range [1..100]\n"
	"Each additional thread after the first will use an additional 18MiB of RAM,\n"
	"plus 1.12 MiB of RAM per megabyte of configured read cache size.\n")

cfg(allocation_vdo_bio_rotation_CFG, "vdo_bio_rotation", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_BIO_ROTATION, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of I/O operations to enqueue for each bio-submission\n"
	"thread before directing work to the next. The value must be in range [1..1024].\n")

cfg(allocation_vdo_cpu_threads_CFG, "vdo_cpu_threads", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_CPU_THREADS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of threads to use for CPU-intensive work such as\n"
	"hashing or compression for VDO volume. The value must be in range [1..100]\n")

cfg(allocation_vdo_hash_zone_threads_CFG, "vdo_hash_zone_threads", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_HASH_ZONE_THREADS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of threads across which to subdivide parts of the VDO\n"
	"processing based on the hash value computed from the block data.\n"
	"The value must be at in range [0..100].\n"
	"vdo_hash_zone_threads, vdo_logical_threads and vdo_physical_threads must be\n"
	"either all zero or all non-zero.")

cfg(allocation_vdo_logical_threads_CFG, "vdo_logical_threads", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_LOGICAL_THREADS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of threads across which to subdivide parts of the VDO\n"
	"processing based on the hash value computed from the block data.\n"
	"A logical thread count of 9 or more will require explicitly specifying\n"
	"a sufficiently large block map cache size, as well.\n"
	"The value must be in range [0..100].\n"
	"vdo_hash_zone_threads, vdo_logical_threads and vdo_physical_threads must be\n"
	"either all zero or all non-zero.\n")

cfg(allocation_vdo_physical_threads_CFG, "vdo_physical_threads", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_PHYSICAL_THREADS, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the number of threads across which to subdivide parts of the VDO\n"
	"processing based on physical block addresses.\n"
	"Each additional thread after the first will use an additional 10MiB of RAM.\n"
	"The value must be in range [0..16].\n"
	"vdo_hash_zone_threads, vdo_logical_threads and vdo_physical_threads must be\n"
	"either all zero or all non-zero.\n")

cfg(allocation_vdo_write_policy_CFG, "vdo_write_policy", allocation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VDO_WRITE_POLICY, VDO_1ST_VSN, NULL, 0, NULL,
	"Specifies the write policy:\n"
	"auto  - VDO will check the storage device and determine whether it supports flushes.\n"
	"        If it does, VDO will run in async mode, otherwise it will run in sync mode.\n"
	"sync  - Writes are acknowledged only after data is stably written.\n"
	"        This policy is not supported if the underlying storage is not also synchronous.\n"
	"async - Writes are acknowledged after data has been cached for writing to stable storage.\n"
	"        Data which has not been flushed is not guaranteed to persist in this mode.\n")

cfg(log_report_command_log_CFG, "report_command_log", log_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED | CFG_DISALLOW_INTERACTIVE, CFG_TYPE_BOOL, DEFAULT_COMMAND_LOG_REPORT, vsn(2, 2, 158), NULL, 0, NULL,
	"Enable or disable LVM log reporting.\n"
	"If enabled, LVM will collect a log of operations, messages,\n"
	"per-object return codes with object identification and associated\n"
	"error numbers (errnos) during LVM command processing. Then the\n"
	"log is either reported solely or in addition to any existing\n"
	"reports, depending on LVM command used. If it is a reporting command\n"
	"(e.g. pvs, vgs, lvs, lvm fullreport), then the log is reported in\n"
	"addition to any existing reports. Otherwise, there's only log report\n"
	"on output. For all applicable LVM commands, you can request that\n"
	"the output has only log report by using --logonly command line\n"
	"option. Use log/command_log_cols and log/command_log_sort settings\n"
	"to define fields to display and sort fields for the log report.\n"
	"You can also use log/command_log_selection to define selection\n"
	"criteria used each time the log is reported.\n")

cfg(log_command_log_sort_CFG, "command_log_sort", log_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED | CFG_DISALLOW_INTERACTIVE, CFG_TYPE_STRING, DEFAULT_COMMAND_LOG_SORT, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to sort by when reporting command log.\n"
	"See <lvm command> --logonly --configreport log -o help\n"
	"for the list of possible fields.\n")

cfg(log_command_log_cols_CFG, "command_log_cols", log_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED | CFG_DISALLOW_INTERACTIVE, CFG_TYPE_STRING, DEFAULT_COMMAND_LOG_COLS, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to report when reporting command log.\n"
	"See <lvm command> --logonly --configreport log -o help\n"
	"for the list of possible fields.\n")

cfg(log_command_log_selection_CFG, "command_log_selection", log_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED | CFG_DISALLOW_INTERACTIVE, CFG_TYPE_STRING, DEFAULT_COMMAND_LOG_SELECTION, vsn(2, 2, 158), NULL, 0, NULL,
	"Selection criteria used when reporting command log.\n"
	"You can define selection criteria that are applied each\n"
	"time log is reported. This way, it is possible to control the\n"
	"amount of log that is displayed on output and you can select\n"
	"only parts of the log that are important for you. To define\n"
	"selection criteria, use fields from log report. See also\n"
	"<lvm command> --logonly --configreport log -S help for the\n"
	"list of possible fields and selection operators. You can also\n"
	"define selection criteria for log report on command line directly\n"
	"using <lvm command> --configreport log -S <selection criteria>\n"
	"which has precedence over log/command_log_selection setting.\n"
	"For more information about selection criteria in general, see\n"
	"lvm(8) man page.\n")

cfg(log_verbose_CFG, "verbose", log_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_VERBOSE, vsn(1, 0, 0), NULL, 0, NULL,
	"Controls the messages sent to stdout or stderr.\n")

cfg(log_silent_CFG, "silent", log_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_SILENT, vsn(2, 2, 98), NULL, 0, NULL,
	"Suppress all non-essential messages from stdout.\n"
	"This has the same effect as -qq. When enabled, the following commands\n"
	"still produce output: dumpconfig, lvdisplay, lvmdiskscan, lvs, pvck,\n"
	"pvdisplay, pvs, version, vgcfgrestore -l, vgdisplay, vgs.\n"
	"Non-essential messages are shifted from log level 4 to log level 5\n"
	"for syslog and lvm2_log_fn purposes.\n"
	"Any 'yes' or 'no' questions not overridden by other arguments are\n"
	"suppressed and default to 'no'.\n")

cfg(log_syslog_CFG, "syslog", log_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_SYSLOG, vsn(1, 0, 0), NULL, 0, NULL,
	"Send log messages through syslog.\n")

cfg(log_file_CFG, "file", log_CFG_SECTION, CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, 0, NULL,
	"Write error and debug log messages to a file specified here.\n")

cfg(log_overwrite_CFG, "overwrite", log_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_OVERWRITE, vsn(1, 0, 0), NULL, 0, NULL,
	"Overwrite the log file each time the program is run.\n")

cfg(log_level_CFG, "level", log_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_LOGLEVEL, vsn(1, 0, 0), NULL, 0, NULL,
	"The level of log messages that are sent to the log file or syslog.\n"
	"There are 6 syslog-like log levels currently in use: 2 to 7 inclusive.\n"
	"7 is the most verbose (LOG_DEBUG).\n")

cfg(log_indent_CFG, "indent", log_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_INDENT, vsn(1, 0, 0), NULL, 0, NULL,
	"Indent messages according to their severity.\n")

cfg(log_command_names_CFG, "command_names", log_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_CMD_NAME, vsn(1, 0, 0), NULL, 0, NULL,
	"Display the command name on each line of output.\n")

cfg(log_prefix_CFG, "prefix", log_CFG_SECTION, CFG_ALLOW_EMPTY, CFG_TYPE_STRING, DEFAULT_MSG_PREFIX, vsn(1, 0, 0), NULL, 0, NULL,
	"A prefix to use before the log message text.\n"
	"(After the command name, if selected).\n"
	"Two spaces allows you to see/grep the severity of each message.\n"
	"To make the messages look similar to the original LVM tools use:\n"
	"indent = 0, command_names = 1, prefix = \" -- \"\n")

cfg(log_activation_CFG, "activation", log_CFG_SECTION, 0, CFG_TYPE_BOOL, 0, vsn(1, 0, 0), NULL, 0, NULL,
	"Log messages during activation.\n"
	"Don't use this in low memory situations (can deadlock).\n")

cfg(log_activate_file_CFG, "activate_file", log_CFG_SECTION, CFG_DEFAULT_UNDEFINED | CFG_UNSUPPORTED, CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, 0, NULL, NULL)

cfg_array(log_debug_classes_CFG, "debug_classes", log_CFG_SECTION, CFG_ALLOW_EMPTY, CFG_TYPE_STRING, "#Smemory#Sdevices#Sio#Sactivation#Sallocation#Smetadata#Scache#Slocking#Slvmpolld#Sdbus", vsn(2, 2, 99), NULL, 0, NULL,
	"Select log messages by class.\n"
	"Some debugging messages are assigned to a class and only appear in\n"
	"debug output if the class is listed here. Classes currently\n"
	"available: memory, devices, io, activation, allocation,\n"
	"metadata, cache, locking, lvmpolld. Use \"all\" to see everything.\n")

cfg(backup_backup_CFG, "backup", backup_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_BACKUP_ENABLED, vsn(1, 0, 0), NULL, 0, NULL,
	"Maintain a backup of the current metadata configuration.\n"
	"Think very hard before turning this off!\n")

cfg_runtime(backup_backup_dir_CFG, "backup_dir", backup_CFG_SECTION, 0, CFG_TYPE_STRING, vsn(1, 0, 0), 0, NULL,
	"Location of the metadata backup files.\n"
	"Remember to back up this directory regularly!\n")

cfg(backup_archive_CFG, "archive", backup_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_ARCHIVE_ENABLED, vsn(1, 0, 0), NULL, 0, NULL,
	"Maintain an archive of old metadata configurations.\n"
	"Think very hard before turning this off.\n")

cfg_runtime(backup_archive_dir_CFG, "archive_dir", backup_CFG_SECTION, 0, CFG_TYPE_STRING, vsn(1, 0, 0), 0, NULL,
	"Location of the metdata archive files.\n"
	"Remember to back up this directory regularly!\n")

cfg(backup_retain_min_CFG, "retain_min", backup_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_ARCHIVE_NUMBER, vsn(1, 0, 0), NULL, 0, NULL,
	"Minimum number of archives to keep.\n")

cfg(backup_retain_days_CFG, "retain_days", backup_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_ARCHIVE_DAYS, vsn(1, 0, 0), NULL, 0, NULL,
	"Minimum number of days to keep archive files.\n")

cfg(shell_history_size_CFG, "history_size", shell_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_MAX_HISTORY, vsn(1, 0, 0), NULL, 0, NULL,
	"Number of lines of history to store in ~/.lvm_history.\n")

cfg(global_umask_CFG, "umask", global_CFG_SECTION, CFG_FORMAT_INT_OCTAL, CFG_TYPE_INT, DEFAULT_UMASK, vsn(1, 0, 0), NULL, 0, NULL,
	"The file creation mask for any files and directories created.\n"
	"Interpreted as octal if the first digit is zero.\n")

cfg(global_test_CFG, "test", global_CFG_SECTION, 0, CFG_TYPE_BOOL, 0, vsn(1, 0, 0), NULL, 0, NULL,
	"No on-disk metadata changes will be made in test mode.\n"
	"Equivalent to having the -t option on every command.\n")

cfg(global_units_CFG, "units", global_CFG_SECTION, CFG_PROFILABLE, CFG_TYPE_STRING, DEFAULT_UNITS, vsn(1, 0, 0), NULL, 0, NULL,
	"Default value for --units argument.\n")

cfg(global_si_unit_consistency_CFG, "si_unit_consistency", global_CFG_SECTION, CFG_PROFILABLE, CFG_TYPE_BOOL, DEFAULT_SI_UNIT_CONSISTENCY,  vsn(2, 2, 54), NULL, 0, NULL,
	"Distinguish between powers of 1024 and 1000 bytes.\n"
	"The LVM commands distinguish between powers of 1024 bytes,\n"
	"e.g. KiB, MiB, GiB, and powers of 1000 bytes, e.g. KB, MB, GB.\n"
	"If scripts depend on the old behaviour, disable this setting\n"
	"temporarily until they are updated.\n")

cfg(global_suffix_CFG, "suffix", global_CFG_SECTION, CFG_PROFILABLE, CFG_TYPE_BOOL, DEFAULT_SUFFIX, vsn(1, 0, 0), NULL, 0, NULL,
	"Display unit suffix for sizes.\n"
	"This setting has no effect if the units are in human-readable form\n"
	"(global/units = \"h\") in which case the suffix is always displayed.\n")

cfg(global_activation_CFG, "activation", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_ACTIVATION, vsn(1, 0, 0), NULL, 0, NULL,
	"Enable/disable communication with the kernel device-mapper.\n"
	"Disable to use the tools to manipulate LVM metadata without\n"
	"activating any logical volumes. If the device-mapper driver\n"
	"is not present in the kernel, disabling this should suppress\n"
	"the error messages.\n")

cfg(global_fallback_to_lvm1_CFG, "fallback_to_lvm1", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, 0, vsn(1, 0, 18), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(global_format_CFG, "format", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_FORMAT, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg_array(global_format_libraries_CFG, "format_libraries", global_CFG_SECTION, CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.")

cfg_array(global_segment_libraries_CFG, "segment_libraries", global_CFG_SECTION, CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 18), NULL, 0, NULL, NULL)

cfg(global_proc_CFG, "proc", global_CFG_SECTION, CFG_ADVANCED, CFG_TYPE_STRING, DEFAULT_PROC_DIR, vsn(1, 0, 0), NULL, 0, NULL,
	"Location of proc filesystem.\n")

cfg(global_etc_CFG, "etc", global_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_ETC_DIR, vsn(2, 2, 117), "@CONFDIR@", 0, NULL,
	"Location of /etc system configuration directory.\n")

cfg(global_locking_type_CFG, "locking_type", global_CFG_SECTION, 0, CFG_TYPE_INT, 1, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.")

cfg(global_wait_for_locks_CFG, "wait_for_locks", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_WAIT_FOR_LOCKS, vsn(2, 2, 50), NULL, 0, NULL,
	"When disabled, fail if a lock request would block.\n")

cfg(global_fallback_to_clustered_locking_CFG, "fallback_to_clustered_locking", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_FALLBACK_TO_CLUSTERED_LOCKING, vsn(2, 2, 42), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(global_fallback_to_local_locking_CFG, "fallback_to_local_locking", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_FALLBACK_TO_LOCAL_LOCKING, vsn(2, 2, 42), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(global_locking_dir_CFG, "locking_dir", global_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_LOCK_DIR, vsn(1, 0, 0), "@DEFAULT_LOCK_DIR@", 0, NULL,
	"Directory to use for LVM command file locks.\n"
	"Local non-LV directory that holds file-based locks while commands are\n"
	"in progress. A directory like /tmp that may get wiped on reboot is OK.\n")

cfg(global_prioritise_write_locks_CFG, "prioritise_write_locks", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_PRIORITISE_WRITE_LOCKS, vsn(2, 2, 52), NULL, 0, NULL,
	"Allow quicker VG write access during high volume read access.\n"
	"When there are competing read-only and read-write access requests for\n"
	"a volume group's metadata, instead of always granting the read-only\n"
	"requests immediately, delay them to allow the read-write requests to\n"
	"be serviced. Without this setting, write access may be stalled by a\n"
	"high volume of read-only requests. This option only affects\n"
	"locking_type 1 viz. local file-based locking.\n")

cfg(global_library_dir_CFG, "library_dir", global_CFG_SECTION, CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, 0, NULL,
	"Search this directory first for shared libraries.\n")

cfg(global_locking_library_CFG, "locking_library", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_LOCKING_LIB, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(global_abort_on_internal_errors_CFG, "abort_on_internal_errors", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_ABORT_ON_INTERNAL_ERRORS, vsn(2, 2, 57), NULL, 0, NULL,
	"Abort a command that encounters an internal error.\n"
	"Treat any internal errors as fatal errors, aborting the process that\n"
	"encountered the internal error. Please only enable for debugging.\n")

cfg(global_detect_internal_vg_cache_corruption_CFG, "detect_internal_vg_cache_corruption", global_CFG_SECTION, 0, CFG_TYPE_BOOL, 0, vsn(2, 2, 96), NULL, vsn(2, 2, 174), NULL,
	"No longer used.\n")

cfg(global_metadata_read_only_CFG, "metadata_read_only", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_METADATA_READ_ONLY, vsn(2, 2, 75), NULL, 0, NULL,
	"No operations that change on-disk metadata are permitted.\n"
	"Additionally, read-only commands that encounter metadata in need of\n"
	"repair will still be allowed to proceed exactly as if the repair had\n"
	"been performed (except for the unchanged vg_seqno). Inappropriate\n"
	"use could mess up your system, so seek advice first!\n")

cfg(global_mirror_segtype_default_CFG, "mirror_segtype_default", global_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_MIRROR_SEGTYPE, vsn(2, 2, 87), "@DEFAULT_MIRROR_SEGTYPE@", 0, NULL,
	"The segment type used by the short mirroring option -m.\n"
	"The --type mirror|raid1 option overrides this setting.\n"
	"#\n"
	"Accepted values:\n"
	"  mirror\n"
	"    The original RAID1 implementation from LVM/DM. It is\n"
	"    characterized by a flexible log solution (core, disk, mirrored),\n"
	"    and by the necessity to block I/O while handling a failure.\n"
	"    There is an inherent race in the dmeventd failure handling logic\n"
	"    with snapshots of devices using this type of RAID1 that in the\n"
	"    worst case could cause a deadlock. (Also see\n"
	"    devices/ignore_lvm_mirrors.)\n"
	"  raid1\n"
	"    This is a newer RAID1 implementation using the MD RAID1\n"
	"    personality through device-mapper. It is characterized by a\n"
	"    lack of log options. (A log is always allocated for every\n"
	"    device and they are placed on the same device as the image,\n"
	"    so no separate devices are required.) This mirror\n"
	"    implementation does not require I/O to be blocked while\n"
	"    handling a failure. This mirror implementation is not\n"
	"    cluster-aware and cannot be used in a shared (active/active)\n"
	"    fashion in a cluster.\n"
	"#\n")

cfg(global_raid10_segtype_default_CFG, "raid10_segtype_default", global_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_RAID10_SEGTYPE, vsn(2, 2, 99), "@DEFAULT_RAID10_SEGTYPE@", 0, NULL,
	"The segment type used by the -i -m combination.\n"
	"The --type raid10|mirror option overrides this setting.\n"
	"The --stripes/-i and --mirrors/-m options can both be specified\n"
	"during the creation of a logical volume to use both striping and\n"
	"mirroring for the LV. There are two different implementations.\n"
	"#\n"
	"Accepted values:\n"
	"  raid10\n"
	"    LVM uses MD's RAID10 personality through DM. This is the\n"
	"    preferred option.\n"
	"  mirror\n"
	"    LVM layers the 'mirror' and 'stripe' segment types. The layering\n"
	"    is done by creating a mirror LV on top of striped sub-LVs,\n"
	"    effectively creating a RAID 0+1 array. The layering is suboptimal\n"
	"    in terms of providing redundancy and performance.\n"
	"#\n")

cfg(global_sparse_segtype_default_CFG, "sparse_segtype_default", global_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_SPARSE_SEGTYPE, vsn(2, 2, 112), "@DEFAULT_SPARSE_SEGTYPE@", 0, NULL,
	"The segment type used by the -V -L combination.\n"
	"The --type snapshot|thin option overrides this setting.\n"
	"The combination of -V and -L options creates a sparse LV. There are\n"
	"two different implementations.\n"
	"#\n"
	"Accepted values:\n"
	"  snapshot\n"
	"    The original snapshot implementation from LVM/DM. It uses an old\n"
	"    snapshot that mixes data and metadata within a single COW\n"
	"    storage volume and performs poorly when the size of stored data\n"
	"    passes hundreds of MB.\n"
	"  thin\n"
	"    A newer implementation that uses thin provisioning. It has a\n"
	"    bigger minimal chunk size (64KiB) and uses a separate volume for\n"
	"    metadata. It has better performance, especially when more data\n"
	"    is used. It also supports full snapshots.\n"
	"#\n")

cfg(global_lvdisplay_shows_full_device_path_CFG, "lvdisplay_shows_full_device_path", global_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_LVDISPLAY_SHOWS_FULL_DEVICE_PATH, vsn(2, 2, 89), NULL, 0, NULL,
	"Enable this to reinstate the previous lvdisplay name format.\n"
	"The default format for displaying LV names in lvdisplay was changed\n"
	"in version 2.02.89 to show the LV name and path separately.\n"
	"Previously this was always shown as /dev/vgname/lvname even when that\n"
	"was never a valid path in the /dev filesystem.\n")

cfg(global_event_activation_CFG, "event_activation", global_CFG_SECTION, 0, CFG_TYPE_BOOL, 1, vsn(2, 3, 1), 0, 0, NULL,
	"Activate LVs based on system-generated device events.\n"
	"When a device appears on the system, a system-generated event runs\n"
	"the pvscan command to activate LVs if the new PV completes the VG.\n"
	"Use auto_activation_volume_list to select which LVs should be\n"
	"activated from these events (the default is all.)\n"
	"When event_activation is disabled, the system will generally run\n"
	"a direct activation command to activate LVs in complete VGs.\n")

cfg(global_use_lvmetad_CFG, "use_lvmetad", global_CFG_SECTION, 0, CFG_TYPE_BOOL, 0, vsn(2, 2, 93), 0, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(global_lvmetad_update_wait_time_CFG, "lvmetad_update_wait_time", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, 0, vsn(2, 2, 151), NULL, vsn(2, 3, 0), NULL,
	"This setting is no longer used.\n")

cfg(global_use_aio_CFG, "use_aio", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_USE_AIO, vsn(2, 2, 183), NULL, 0, NULL,
	"Use async I/O when reading and writing devices.\n")

cfg(global_use_lvmlockd_CFG, "use_lvmlockd", global_CFG_SECTION, 0, CFG_TYPE_BOOL, 0, vsn(2, 2, 124), NULL, 0, NULL,
	"Use lvmlockd for locking among hosts using LVM on shared storage.\n"
	"Applicable only if LVM is compiled with lockd support in which\n"
	"case there is also lvmlockd(8) man page available for more\n"
	"information.\n")

cfg(global_lvmlockd_lock_retries_CFG, "lvmlockd_lock_retries", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_LVMLOCKD_LOCK_RETRIES, vsn(2, 2, 125), NULL, 0, NULL,
	"Retry lvmlockd lock requests this many times.\n"
	"Applicable only if LVM is compiled with lockd support\n")

cfg(global_sanlock_lv_extend_CFG, "sanlock_lv_extend", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_SANLOCK_LV_EXTEND_MB, vsn(2, 2, 124), NULL, 0, NULL,
	"Size in MiB to extend the internal LV holding sanlock locks.\n"
	"The internal LV holds locks for each LV in the VG, and after enough\n"
	"LVs have been created, the internal LV needs to be extended. lvcreate\n"
	"will automatically extend the internal LV when needed by the amount\n"
	"specified here. Setting this to 0 disables the automatic extension\n"
	"and can cause lvcreate to fail. Applicable only if LVM is compiled\n"
	"with lockd support\n")

cfg(global_thin_check_executable_CFG, "thin_check_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, THIN_CHECK_CMD, vsn(2, 2, 94), "@THIN_CHECK_CMD@", 0, NULL,
	"The full path to the thin_check command.\n"
	"LVM uses this command to check that a thin metadata device is in a\n"
	"usable state. When a thin pool is activated and after it is\n"
	"deactivated, this command is run. Activation will only proceed if\n"
	"the command has an exit status of 0. Set to \"\" to skip this check.\n"
	"(Not recommended.) Also see thin_check_options.\n"
	"(See package device-mapper-persistent-data or thin-provisioning-tools)\n")

cfg(global_thin_dump_executable_CFG, "thin_dump_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, THIN_DUMP_CMD, vsn(2, 2, 100), "@THIN_DUMP_CMD@", 0, NULL,
	"The full path to the thin_dump command.\n"
	"LVM uses this command to dump thin pool metadata.\n"
	"(See package device-mapper-persistent-data or thin-provisioning-tools)\n")

cfg(global_thin_repair_executable_CFG, "thin_repair_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, THIN_REPAIR_CMD, vsn(2, 2, 100), "@THIN_REPAIR_CMD@", 0, NULL,
	"The full path to the thin_repair command.\n"
	"LVM uses this command to repair a thin metadata device if it is in\n"
	"an unusable state. Also see thin_repair_options.\n"
	"(See package device-mapper-persistent-data or thin-provisioning-tools)\n")

cfg_array(global_thin_check_options_CFG, "thin_check_options", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_THIN_CHECK_OPTIONS_CONFIG, vsn(2, 2, 96), NULL, 0, NULL,
	"List of options passed to the thin_check command.\n"
	"With thin_check version 2.1 or newer you can add the option\n"
	"--ignore-non-fatal-errors to let it pass through ignorable errors\n"
	"and fix them later. With thin_check version 3.2 or newer you should\n"
	"include the option --clear-needs-check-flag.\n")

cfg_array(global_thin_repair_options_CFG, "thin_repair_options", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_THIN_REPAIR_OPTIONS_CONFIG, vsn(2, 2, 100), NULL, 0, NULL,
	"List of options passed to the thin_repair command.\n")

cfg_array(global_thin_disabled_features_CFG, "thin_disabled_features", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 99), NULL, 0, NULL,
	"Features to not use in the thin driver.\n"
	"This can be helpful for testing, or to avoid using a feature that is\n"
	"causing problems. Features include: block_size, discards,\n"
	"discards_non_power_2, external_origin, metadata_resize,\n"
	"external_origin_extend, error_if_no_space.\n"
	"#\n"
	"Example\n"
	"thin_disabled_features = [ \"discards\", \"block_size\" ]\n"
	"#\n")

cfg_array(global_cache_disabled_features_CFG, "cache_disabled_features", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 128), NULL, 0, NULL,
	"Features to not use in the cache driver.\n"
	"This can be helpful for testing, or to avoid using a feature that is\n"
	"causing problems. Features include: policy_mq, policy_smq, metadata2.\n"
	"#\n"
	"Example\n"
	"cache_disabled_features = [ \"policy_smq\" ]\n"
	"#\n")

cfg(global_cache_check_executable_CFG, "cache_check_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, CACHE_CHECK_CMD, vsn(2, 2, 108), "@CACHE_CHECK_CMD@", 0, NULL,
	"The full path to the cache_check command.\n"
	"LVM uses this command to check that a cache metadata device is in a\n"
	"usable state. When a cached LV is activated and after it is\n"
	"deactivated, this command is run. Activation will only proceed if the\n"
	"command has an exit status of 0. Set to \"\" to skip this check.\n"
	"(Not recommended.) Also see cache_check_options.\n"
	"(See package device-mapper-persistent-data or thin-provisioning-tools)\n")

cfg(global_cache_dump_executable_CFG, "cache_dump_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, CACHE_DUMP_CMD, vsn(2, 2, 108), "@CACHE_DUMP_CMD@", 0, NULL,
	"The full path to the cache_dump command.\n"
	"LVM uses this command to dump cache pool metadata.\n"
	"(See package device-mapper-persistent-data or thin-provisioning-tools)\n")

cfg(global_cache_repair_executable_CFG, "cache_repair_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, CACHE_REPAIR_CMD, vsn(2, 2, 108), "@CACHE_REPAIR_CMD@", 0, NULL,
	"The full path to the cache_repair command.\n"
	"LVM uses this command to repair a cache metadata device if it is in\n"
	"an unusable state. Also see cache_repair_options.\n"
	"(See package device-mapper-persistent-data or thin-provisioning-tools)\n")

cfg_array(global_cache_check_options_CFG, "cache_check_options", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_CACHE_CHECK_OPTIONS_CONFIG, vsn(2, 2, 108), NULL, 0, NULL,
	"List of options passed to the cache_check command.\n"
	"With cache_check version 5.0 or newer you should include the option\n"
	"--clear-needs-check-flag.\n")

cfg_array(global_cache_repair_options_CFG, "cache_repair_options", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_CACHE_REPAIR_OPTIONS_CONFIG, vsn(2, 2, 108), NULL, 0, NULL,
	"List of options passed to the cache_repair command.\n")

cfg(global_vdo_format_executable_CFG, "vdo_format_executable", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, VDO_FORMAT_CMD, VDO_1ST_VSN, "@VDO_FORMAT_CMD@", 0, NULL,
	"The full path to the vdoformat command.\n"
	"LVM uses this command to initial data volume for VDO type logical volume\n")

cfg_array(global_vdo_format_options_CFG, "vdo_format_options", global_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VDO_FORMAT_OPTIONS_CONFIG, VDO_1ST_VSN, NULL, 0, NULL,
	"List of options passed added to standard vdoformat command.\n")

cfg(global_fsadm_executable_CFG, "fsadm_executable", global_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_FSADM_PATH, vsn(2, 2, 170), "@FSADM_PATH@", 0, NULL,
	"The full path to the fsadm command.\n"
	"LVM uses this command to help with lvresize -r operations.\n")

cfg(global_system_id_source_CFG, "system_id_source", global_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_SYSTEM_ID_SOURCE, vsn(2, 2, 117), NULL, 0, NULL,
	"The method LVM uses to set the local system ID.\n"
	"Volume Groups can also be given a system ID (by vgcreate, vgchange,\n"
	"or vgimport.) A VG on shared storage devices is accessible only to\n"
	"the host with a matching system ID. See 'man lvmsystemid' for\n"
	"information on limitations and correct usage.\n"
	"#\n"
	"Accepted values:\n"
	"  none\n"
	"    The host has no system ID.\n"
	"  lvmlocal\n"
	"    Obtain the system ID from the system_id setting in the 'local'\n"
	"    section of an lvm configuration file, e.g. lvmlocal.conf.\n"
	"  uname\n"
	"    Set the system ID from the hostname (uname) of the system.\n"
	"    System IDs beginning localhost are not permitted.\n"
	"  machineid\n"
	"    Use the contents of the machine-id file to set the system ID.\n"
	"    Some systems create this file at installation time.\n"
	"    See 'man machine-id' and global/etc.\n"
	"  file\n"
	"    Use the contents of another file (system_id_file) to set the\n"
	"    system ID.\n"
	"#\n")

cfg(global_system_id_file_CFG, "system_id_file", global_CFG_SECTION, CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 117), NULL, 0, NULL,
	"The full path to the file containing a system ID.\n"
	"This is used when system_id_source is set to 'file'.\n"
	"Comments starting with the character # are ignored.\n")

cfg(activation_checks_CFG, "checks", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_ACTIVATION_CHECKS, vsn(2, 2, 86), NULL, 0, NULL,
	"Perform internal checks of libdevmapper operations.\n"
	"Useful for debugging problems with activation. Some of the checks may\n"
	"be expensive, so it's best to use this only when there seems to be a\n"
	"problem.\n")

cfg(global_use_lvmpolld_CFG, "use_lvmpolld", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_USE_LVMPOLLD, vsn(2, 2, 120), "@DEFAULT_USE_LVMPOLLD@", 0, NULL,
	"Use lvmpolld to supervise long running LVM commands.\n"
	"When enabled, control of long running LVM commands is transferred\n"
	"from the original LVM command to the lvmpolld daemon. This allows\n"
	"the operation to continue independent of the original LVM command.\n"
	"After lvmpolld takes over, the LVM command displays the progress\n"
	"of the ongoing operation. lvmpolld itself runs LVM commands to\n"
	"manage the progress of ongoing operations. lvmpolld can be used as\n"
	"a native systemd service, which allows it to be started on demand,\n"
	"and to use its own control group. When this option is disabled, LVM\n"
	"commands will supervise long running operations by forking themselves.\n"
	"Applicable only if LVM is compiled with lvmpolld support.\n")

cfg(global_notify_dbus_CFG, "notify_dbus", global_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_NOTIFY_DBUS, vsn(2, 2, 145), NULL, 0, NULL,
	"Enable D-Bus notification from LVM commands.\n"
	"When enabled, an LVM command that changes PVs, changes VG metadata,\n"
	"or changes the activation state of an LV will send a notification.\n")

cfg(activation_udev_sync_CFG, "udev_sync", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_UDEV_SYNC, vsn(2, 2, 51), NULL, 0, NULL,
	"Use udev notifications to synchronize udev and LVM.\n"
	"The --nodevsync option overrides this setting.\n"
	"When disabled, LVM commands will not wait for notifications from\n"
	"udev, but continue irrespective of any possible udev processing in\n"
	"the background. Only use this if udev is not running or has rules\n"
	"that ignore the devices LVM creates. If enabled when udev is not\n"
	"running, and LVM processes are waiting for udev, run the command\n"
	"'dmsetup udevcomplete_all' to wake them up.\n")

cfg(activation_udev_rules_CFG, "udev_rules", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_UDEV_RULES, vsn(2, 2, 57), NULL, 0, NULL,
	"Use udev rules to manage LV device nodes and symlinks.\n"
	"When disabled, LVM will manage the device nodes and symlinks for\n"
	"active LVs itself. Manual intervention may be required if this\n"
	"setting is changed while LVs are active.\n")

cfg(activation_verify_udev_operations_CFG, "verify_udev_operations", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_VERIFY_UDEV_OPERATIONS, vsn(2, 2, 86), NULL, 0, NULL,
	"Use extra checks in LVM to verify udev operations.\n"
	"This enables additional checks (and if necessary, repairs) on entries\n"
	"in the device directory after udev has completed processing its\n"
	"events. Useful for diagnosing problems with LVM/udev interactions.\n")

cfg(activation_retry_deactivation_CFG, "retry_deactivation", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_RETRY_DEACTIVATION, vsn(2, 2, 89), NULL, 0, NULL,
	"Retry failed LV deactivation.\n"
	"If LV deactivation fails, LVM will retry for a few seconds before\n"
	"failing. This may happen because a process run from a quick udev rule\n"
	"temporarily opened the device.\n")

cfg(activation_missing_stripe_filler_CFG, "missing_stripe_filler", activation_CFG_SECTION, CFG_ADVANCED, CFG_TYPE_STRING, DEFAULT_STRIPE_FILLER, vsn(1, 0, 0), NULL, 0, NULL,
	"Method to fill missing stripes when activating an incomplete LV.\n"
	"Using 'error' will make inaccessible parts of the device return I/O\n"
	"errors on access. Using 'zero' will return success (and zero) on I/O\n"
	"You can instead use a device path, in which case,\n"
	"that device will be used in place of missing stripes. Using anything\n"
	"other than 'error' with mirrored or snapshotted volumes is likely to\n"
	"result in data corruption.\n")

cfg(activation_use_linear_target_CFG, "use_linear_target", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_USE_LINEAR_TARGET, vsn(2, 2, 89), NULL, 0, NULL,
	"Use the linear target to optimize single stripe LVs.\n"
	"When disabled, the striped target is used. The linear target is an\n"
	"optimised version of the striped target that only handles a single\n"
	"stripe.\n")

cfg(activation_reserved_stack_CFG, "reserved_stack", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_RESERVED_STACK, vsn(1, 0, 0), NULL, 0, NULL,
	"Stack size in KiB to reserve for use while devices are suspended.\n"
	"Insufficent reserve risks I/O deadlock during device suspension.\n")

cfg(activation_reserved_memory_CFG, "reserved_memory", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_RESERVED_MEMORY, vsn(1, 0, 0), NULL, 0, NULL,
	"Memory size in KiB to reserve for use while devices are suspended.\n"
	"Insufficent reserve risks I/O deadlock during device suspension.\n")

cfg(activation_process_priority_CFG, "process_priority", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_PROCESS_PRIORITY, vsn(1, 0, 0), NULL, 0, NULL,
	"Nice value used while devices are suspended.\n"
	"Use a high priority so that LVs are suspended\n"
	"for the shortest possible time.\n")

cfg_array(activation_volume_list_CFG, "volume_list", activation_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 18), NULL, 0, NULL,
	"Only LVs selected by this list are activated.\n"
	"If this list is defined, an LV is only activated if it matches an\n"
	"entry in this list. If this list is undefined, it imposes no limits\n"
	"on LV activation (all are allowed).\n"
	"#\n"
	"Accepted values:\n"
	"  vgname\n"
	"    The VG name is matched exactly and selects all LVs in the VG.\n"
	"  vgname/lvname\n"
	"    The VG name and LV name are matched exactly and selects the LV.\n"
	"  @tag\n"
	"    Selects an LV if the specified tag matches a tag set on the LV\n"
	"    or VG.\n"
	"  @*\n"
	"    Selects an LV if a tag defined on the host is also set on the LV\n"
	"    or VG. See tags/hosttags. If any host tags exist but volume_list\n"
	"    is not defined, a default single-entry list containing '@*'\n"
	"    is assumed.\n"
	"#\n"
	"Example\n"
	"volume_list = [ \"vg1\", \"vg2/lvol1\", \"@tag1\", \"@*\" ]\n"
	"#\n")

cfg_array(activation_auto_activation_volume_list_CFG, "auto_activation_volume_list", activation_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 97), NULL, 0, NULL,
	"Only LVs selected by this list are auto-activated.\n"
	"This list works like volume_list, but it is used only by\n"
	"auto-activation commands. It does not apply to direct activation\n"
	"commands. If this list is defined, an LV is only auto-activated\n"
	"if it matches an entry in this list. If this list is undefined, it\n"
	"imposes no limits on LV auto-activation (all are allowed.) If this\n"
	"list is defined and empty, i.e. \"[]\", then no LVs are selected for\n"
	"auto-activation. An LV that is selected by this list for\n"
	"auto-activation, must also be selected by volume_list (if defined)\n"
	"before it is activated. Auto-activation is an activation command that\n"
	"includes the 'a' argument: --activate ay or -a ay. The 'a' (auto)\n"
	"argument for auto-activation is meant to be used by activation\n"
	"commands that are run automatically by the system, as opposed to LVM\n"
	"commands run directly by a user. A user may also use the 'a' flag\n"
	"directly to perform auto-activation. Also see pvscan(8) for more\n"
	"information about auto-activation.\n"
	"#\n"
	"Accepted values:\n"
	"  vgname\n"
	"    The VG name is matched exactly and selects all LVs in the VG.\n"
	"  vgname/lvname\n"
	"    The VG name and LV name are matched exactly and selects the LV.\n"
	"  @tag\n"
	"    Selects an LV if the specified tag matches a tag set on the LV\n"
	"    or VG.\n"
	"  @*\n"
	"    Selects an LV if a tag defined on the host is also set on the LV\n"
	"    or VG. See tags/hosttags. If any host tags exist but volume_list\n"
	"    is not defined, a default single-entry list containing '@*'\n"
	"    is assumed.\n"
	"#\n"
	"Example\n"
	"auto_activation_volume_list = [ \"vg1\", \"vg2/lvol1\", \"@tag1\", \"@*\" ]\n"
	"#\n")

cfg_array(activation_read_only_volume_list_CFG, "read_only_volume_list", activation_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 89), NULL, 0, NULL,
	"LVs in this list are activated in read-only mode.\n"
	"If this list is defined, each LV that is to be activated is checked\n"
	"against this list, and if it matches, it is activated in read-only\n"
	"mode. This overrides the permission setting stored in the metadata,\n"
	"e.g. from --permission rw.\n"
	"#\n"
	"Accepted values:\n"
	"  vgname\n"
	"    The VG name is matched exactly and selects all LVs in the VG.\n"
	"  vgname/lvname\n"
	"    The VG name and LV name are matched exactly and selects the LV.\n"
	"  @tag\n"
	"    Selects an LV if the specified tag matches a tag set on the LV\n"
	"    or VG.\n"
	"  @*\n"
	"    Selects an LV if a tag defined on the host is also set on the LV\n"
	"    or VG. See tags/hosttags. If any host tags exist but volume_list\n"
	"    is not defined, a default single-entry list containing '@*'\n"
	"    is assumed.\n"
	"#\n"
	"Example\n"
	"read_only_volume_list = [ \"vg1\", \"vg2/lvol1\", \"@tag1\", \"@*\" ]\n"
	"#\n")

 cfg(activation_mirror_region_size_CFG, "mirror_region_size", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_RAID_REGION_SIZE, vsn(1, 0, 0), NULL, vsn(2, 2, 99),
	"This has been replaced by the activation/raid_region_size setting.\n",
	"Size in KiB of each raid or mirror synchronization region.\n")

cfg(activation_raid_region_size_CFG, "raid_region_size", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_RAID_REGION_SIZE, vsn(2, 2, 99), NULL, 0, NULL,
	"Size in KiB of each raid or mirror synchronization region.\n"
	"The clean/dirty state of data is tracked for each region.\n"
	"The value is rounded down to a power of two if necessary, and\n"
	"is ignored if it is not a multiple of the machine memory page size.\n")

cfg(activation_error_when_full_CFG, "error_when_full", activation_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_ERROR_WHEN_FULL, vsn(2, 2, 115), NULL, 0, NULL,
	"Return errors if a thin pool runs out of space.\n"
	"The --errorwhenfull option overrides this setting.\n"
	"When enabled, writes to thin LVs immediately return an error if the\n"
	"thin pool is out of data space. When disabled, writes to thin LVs\n"
	"are queued if the thin pool is out of space, and processed when the\n"
	"thin pool data space is extended. New thin pools are assigned the\n"
	"behavior defined here.\n")

cfg(activation_readahead_CFG, "readahead", activation_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_READ_AHEAD, vsn(1, 0, 23), NULL, 0, NULL,
	"Setting to use when there is no readahead setting in metadata.\n"
	"#\n"
	"Accepted values:\n"
	"  none\n"
	"    Disable readahead.\n"
	"  auto\n"
	"    Use default value chosen by kernel.\n"
	"#\n")

cfg(activation_raid_fault_policy_CFG, "raid_fault_policy", activation_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_RAID_FAULT_POLICY, vsn(2, 2, 89), NULL, 0, NULL,
	"Defines how a device failure in a RAID LV is handled.\n"
	"This includes LVs that have the following segment types:\n"
	"raid1, raid4, raid5*, and raid6*.\n"
	"If a device in the LV fails, the policy determines the steps\n"
	"performed by dmeventd automatically, and the steps perfomed by the\n"
	"manual command lvconvert --repair --use-policies.\n"
	"Automatic handling requires dmeventd to be monitoring the LV.\n"
	"#\n"
	"Accepted values:\n"
	"  warn\n"
	"    Use the system log to warn the user that a device in the RAID LV\n"
	"    has failed. It is left to the user to run lvconvert --repair\n"
	"    manually to remove or replace the failed device. As long as the\n"
	"    number of failed devices does not exceed the redundancy of the LV\n"
	"    (1 device for raid4/5, 2 for raid6), the LV will remain usable.\n"
	"  allocate\n"
	"    Attempt to use any extra physical volumes in the VG as spares and\n"
	"    replace faulty devices.\n"
	"#\n")

cfg_runtime(activation_mirror_image_fault_policy_CFG, "mirror_image_fault_policy", activation_CFG_SECTION, 0, CFG_TYPE_STRING, vsn(2, 2, 57), 0, NULL,
	"Defines how a device failure in a 'mirror' LV is handled.\n"
	"An LV with the 'mirror' segment type is composed of mirror images\n"
	"(copies) and a mirror log. A disk log ensures that a mirror LV does\n"
	"not need to be re-synced (all copies made the same) every time a\n"
	"machine reboots or crashes. If a device in the LV fails, this policy\n"
	"determines the steps perfomed by dmeventd automatically, and the steps\n"
	"performed by the manual command lvconvert --repair --use-policies.\n"
	"Automatic handling requires dmeventd to be monitoring the LV.\n"
	"#\n"
	"Accepted values:\n"
	"  remove\n"
	"    Simply remove the faulty device and run without it. If the log\n"
	"    device fails, the mirror would convert to using an in-memory log.\n"
	"    This means the mirror will not remember its sync status across\n"
	"    crashes/reboots and the entire mirror will be re-synced. If a\n"
	"    mirror image fails, the mirror will convert to a non-mirrored\n"
	"    device if there is only one remaining good copy.\n"
	"  allocate\n"
	"    Remove the faulty device and try to allocate space on a new\n"
	"    device to be a replacement for the failed device. Using this\n"
	"    policy for the log is fast and maintains the ability to remember\n"
	"    sync state through crashes/reboots. Using this policy for a\n"
	"    mirror device is slow, as it requires the mirror to resynchronize\n"
	"    the devices, but it will preserve the mirror characteristic of\n"
	"    the device. This policy acts like 'remove' if no suitable device\n"
	"    and space can be allocated for the replacement.\n"
	"  allocate_anywhere\n"
	"    Not yet implemented. Useful to place the log device temporarily\n"
	"    on the same physical volume as one of the mirror images. This\n"
	"    policy is not recommended for mirror devices since it would break\n"
	"    the redundant nature of the mirror. This policy acts like\n"
	"    'remove' if no suitable device and space can be allocated for the\n"
	"    replacement.\n"
	"#\n")

cfg(activation_mirror_log_fault_policy_CFG, "mirror_log_fault_policy", activation_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_MIRROR_LOG_FAULT_POLICY, vsn(1, 2, 18), NULL, 0, NULL,
	"Defines how a device failure in a 'mirror' log LV is handled.\n"
	"The mirror_image_fault_policy description for mirrored LVs also\n"
	"applies to mirrored log LVs.\n")

cfg(activation_mirror_device_fault_policy_CFG, "mirror_device_fault_policy", activation_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_MIRROR_DEVICE_FAULT_POLICY, vsn(1, 2, 10), NULL, vsn(2, 2, 57),
	"This has been replaced by the activation/mirror_image_fault_policy setting.\n",
	"Define how a device failure affecting a mirror is handled.\n")

cfg(activation_snapshot_autoextend_threshold_CFG, "snapshot_autoextend_threshold", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_SNAPSHOT_AUTOEXTEND_THRESHOLD, vsn(2, 2, 75), NULL, 0, NULL,
	"Auto-extend a snapshot when its usage exceeds this percent.\n"
	"Setting this to 100 disables automatic extension.\n"
	"The minimum value is 50 (a smaller value is treated as 50.)\n"
	"Also see snapshot_autoextend_percent.\n"
	"Automatic extension requires dmeventd to be monitoring the LV.\n"
	"#\n"
	"Example\n"
	"Using 70% autoextend threshold and 20% autoextend size, when a 1G\n"
	"snapshot exceeds 700M, it is extended to 1.2G, and when it exceeds\n"
	"840M, it is extended to 1.44G:\n"
	"snapshot_autoextend_threshold = 70\n"
	"#\n")

cfg(activation_snapshot_autoextend_percent_CFG, "snapshot_autoextend_percent", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_SNAPSHOT_AUTOEXTEND_PERCENT, vsn(2, 2, 75), NULL, 0, NULL,
	"Auto-extending a snapshot adds this percent extra space.\n"
	"The amount of additional space added to a snapshot is this\n"
	"percent of its current size.\n"
	"#\n"
	"Example\n"
	"Using 70% autoextend threshold and 20% autoextend size, when a 1G\n"
	"snapshot exceeds 700M, it is extended to 1.2G, and when it exceeds\n"
	"840M, it is extended to 1.44G:\n"
	"snapshot_autoextend_percent = 20\n"
	"#\n")

cfg(activation_thin_pool_autoextend_threshold_CFG, "thin_pool_autoextend_threshold", activation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA, CFG_TYPE_INT, DEFAULT_THIN_POOL_AUTOEXTEND_THRESHOLD, vsn(2, 2, 89), NULL, 0, NULL,
	"Auto-extend a thin pool when its usage exceeds this percent.\n"
	"Setting this to 100 disables automatic extension.\n"
	"The minimum value is 50 (a smaller value is treated as 50.)\n"
	"Also see thin_pool_autoextend_percent.\n"
	"Automatic extension requires dmeventd to be monitoring the LV.\n"
	"#\n"
	"Example\n"
	"Using 70% autoextend threshold and 20% autoextend size, when a 1G\n"
	"thin pool exceeds 700M, it is extended to 1.2G, and when it exceeds\n"
	"840M, it is extended to 1.44G:\n"
	"thin_pool_autoextend_threshold = 70\n"
	"#\n")

cfg(activation_thin_pool_autoextend_percent_CFG, "thin_pool_autoextend_percent", activation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA, CFG_TYPE_INT, DEFAULT_THIN_POOL_AUTOEXTEND_PERCENT, vsn(2, 2, 89), NULL, 0, NULL,
	"Auto-extending a thin pool adds this percent extra space.\n"
	"The amount of additional space added to a thin pool is this\n"
	"percent of its current size.\n"
	"#\n"
	"Example\n"
	"Using 70% autoextend threshold and 20% autoextend size, when a 1G\n"
	"thin pool exceeds 700M, it is extended to 1.2G, and when it exceeds\n"
	"840M, it is extended to 1.44G:\n"
	"thin_pool_autoextend_percent = 20\n"
	"#\n")

cfg(activation_vdo_pool_autoextend_threshold_CFG, "vdo_pool_autoextend_threshold", activation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA, CFG_TYPE_INT, DEFAULT_VDO_POOL_AUTOEXTEND_THRESHOLD, VDO_1ST_VSN, NULL, 0, NULL,
	"Auto-extend a VDO pool when its usage exceeds this percent.\n"
	"Setting this to 100 disables automatic extension.\n"
	"The minimum value is 50 (a smaller value is treated as 50.)\n"
	"Also see vdo_pool_autoextend_percent.\n"
	"Automatic extension requires dmeventd to be monitoring the LV.\n"
	"#\n"
	"Example\n"
	"Using 70% autoextend threshold and 20% autoextend size, when a 10G\n"
	"VDO pool exceeds 7G, it is extended to 12G, and when it exceeds\n"
	"8.4G, it is extended to 14.4G:\n"
	"vdo_pool_autoextend_threshold = 70\n"
	"#\n")

cfg(activation_vdo_pool_autoextend_percent_CFG, "vdo_pool_autoextend_percent", activation_CFG_SECTION, CFG_PROFILABLE | CFG_PROFILABLE_METADATA | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VDO_POOL_AUTOEXTEND_PERCENT, VDO_1ST_VSN, NULL, 0, NULL,
	"Auto-extending a VDO pool adds this percent extra space.\n"
	"The amount of additional space added to a VDO pool is this\n"
	"percent of its current size.\n"
	"#\n"
	"Example\n"
	"Using 70% autoextend threshold and 20% autoextend size, when a 10G\n"
	"VDO pool exceeds 7G, it is extended to 12G, and when it exceeds\n"
	"8.4G, it is extended to 14.4G:\n")

cfg_array(activation_mlock_filter_CFG, "mlock_filter", activation_CFG_SECTION, CFG_DEFAULT_UNDEFINED | CFG_ADVANCED, CFG_TYPE_STRING, NULL, vsn(2, 2, 62), NULL, 0, NULL,
	"Do not mlock these memory areas.\n"
	"While activating devices, I/O to devices being (re)configured is\n"
	"suspended. As a precaution against deadlocks, LVM pins memory it is\n"
	"using so it is not paged out, and will not require I/O to reread.\n"
	"Groups of pages that are known not to be accessed during activation\n"
	"do not need to be pinned into memory. Each string listed in this\n"
	"setting is compared against each line in /proc/self/maps, and the\n"
	"pages corresponding to lines that match are not pinned. On some\n"
	"systems, locale-archive was found to make up over 80% of the memory\n"
	"used by the process.\n"
	"#\n"
	"Example\n"
	"mlock_filter = [ \"locale/locale-archive\", \"gconv/gconv-modules.cache\" ]\n"
	"#\n")

cfg(activation_use_mlockall_CFG, "use_mlockall", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_USE_MLOCKALL, vsn(2, 2, 62), NULL, 0, NULL,
	"Use the old behavior of mlockall to pin all memory.\n"
	"Prior to version 2.02.62, LVM used mlockall() to pin the whole\n"
	"process's memory while activating devices.\n")

cfg(activation_monitoring_CFG, "monitoring", activation_CFG_SECTION, 0, CFG_TYPE_BOOL, DEFAULT_DMEVENTD_MONITOR, vsn(2, 2, 63), NULL, 0, NULL,
	"Monitor LVs that are activated.\n"
	"The --ignoremonitoring option overrides this setting.\n"
	"When enabled, LVM will ask dmeventd to monitor activated LVs.\n")

cfg(activation_polling_interval_CFG, "polling_interval", activation_CFG_SECTION, 0, CFG_TYPE_INT, DEFAULT_INTERVAL, vsn(2, 2, 63), NULL, 0, NULL,
	"Check pvmove or lvconvert progress at this interval (seconds).\n"
	"When pvmove or lvconvert must wait for the kernel to finish\n"
	"synchronising or merging data, they check and report progress at\n"
	"intervals of this number of seconds. If this is set to 0 and there\n"
	"is only one thing to wait for, there are no progress reports, but\n"
	"the process is awoken immediately once the operation is complete.\n")

cfg(activation_auto_set_activation_skip_CFG, "auto_set_activation_skip", activation_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_AUTO_SET_ACTIVATION_SKIP, vsn(2,2,99), NULL, 0, NULL,
	"Set the activation skip flag on new thin snapshot LVs.\n"
	"The --setactivationskip option overrides this setting.\n"
	"An LV can have a persistent 'activation skip' flag. The flag causes\n"
	"the LV to be skipped during normal activation. The lvchange/vgchange\n"
	"-K option is required to activate LVs that have the activation skip\n"
	"flag set. When this setting is enabled, the activation skip flag is\n"
	"set on new thin snapshot LVs.\n")

cfg(activation_mode_CFG, "activation_mode", activation_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_ACTIVATION_MODE, vsn(2,2,108), NULL, 0, NULL,
	"How LVs with missing devices are activated.\n"
	"The --activationmode option overrides this setting.\n"
	"#\n"
	"Accepted values:\n"
	"  complete\n"
	"    Only allow activation of an LV if all of the Physical Volumes it\n"
	"    uses are present. Other PVs in the Volume Group may be missing.\n"
	"  degraded\n"
	"    Like complete, but additionally RAID LVs of segment type raid1,\n"
	"    raid4, raid5, radid6 and raid10 will be activated if there is no\n"
	"    data loss, i.e. they have sufficient redundancy to present the\n"
	"    entire addressable range of the Logical Volume.\n"
	"  partial\n"
	"    Allows the activation of any LV even if a missing or failed PV\n"
	"    could cause data loss with a portion of the LV inaccessible.\n"
	"    This setting should not normally be used, but may sometimes\n"
	"    assist with data recovery.\n"
	"#\n")

cfg_array(activation_lock_start_list_CFG, "lock_start_list", activation_CFG_SECTION, CFG_ALLOW_EMPTY|CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 124), NULL, 0, NULL,
	"Locking is started only for VGs selected by this list.\n"
	"The rules are the same as those for volume_list.\n")

cfg_array(activation_auto_lock_start_list_CFG, "auto_lock_start_list", activation_CFG_SECTION, CFG_ALLOW_EMPTY|CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 124), NULL, 0, NULL,
	"Locking is auto-started only for VGs selected by this list.\n"
	"The rules are the same as those for auto_activation_volume_list.\n")

cfg(metadata_check_pv_device_sizes_CFG, "check_pv_device_sizes", metadata_CFG_SECTION, CFG_ADVANCED | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, 1, vsn(2, 2, 141), NULL, 0, NULL,
	"Check device sizes are not smaller than corresponding PV sizes.\n"
	"If device size is less than corresponding PV size found in metadata,\n"
	"there is always a risk of data loss. If this option is set, then LVM\n"
	"issues a warning message each time it finds that the device size is\n"
	"less than corresponding PV size. You should not disable this unless\n"
	"you are absolutely sure about what you are doing!\n")

cfg(metadata_record_lvs_history_CFG, "record_lvs_history", metadata_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_RECORD_LVS_HISTORY, vsn(2, 2, 145), NULL, 0, NULL,
	"When enabled, LVM keeps history records about removed LVs in\n"
	"metadata. The information that is recorded in metadata for\n"
	"historical LVs is reduced when compared to original\n"
	"information kept in metadata for live LVs. Currently, this\n"
	"feature is supported for thin and thin snapshot LVs only.\n")

cfg(metadata_lvs_history_retention_time_CFG, "lvs_history_retention_time", metadata_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_LVS_HISTORY_RETENTION_TIME, vsn(2, 2, 145), NULL, 0, NULL,
	"Retention time in seconds after which a record about individual\n"
	"historical logical volume is automatically destroyed.\n"
	"A value of 0 disables this feature.\n")

cfg(metadata_pvmetadatacopies_CFG, "pvmetadatacopies", metadata_CFG_SECTION, CFG_ADVANCED | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_PVMETADATACOPIES, vsn(1, 0, 0), NULL, 0, NULL,
	"Number of copies of metadata to store on each PV.\n"
	"The --pvmetadatacopies option overrides this setting.\n"
	"#\n"
	"Accepted values:\n"
	"  2\n"
	"    Two copies of the VG metadata are stored on the PV, one at the\n"
	"    front of the PV, and one at the end.\n"
	"  1\n"
	"    One copy of VG metadata is stored at the front of the PV.\n"
	"  0\n"
	"    No copies of VG metadata are stored on the PV. This may be\n"
	"    useful for VGs containing large numbers of PVs.\n"
	"#\n")

cfg(metadata_vgmetadatacopies_CFG, "vgmetadatacopies", metadata_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_VGMETADATACOPIES, vsn(2, 2, 69), NULL, 0, NULL,
	"Number of copies of metadata to maintain for each VG.\n"
	"The --vgmetadatacopies option overrides this setting.\n"
	"If set to a non-zero value, LVM automatically chooses which of the\n"
	"available metadata areas to use to achieve the requested number of\n"
	"copies of the VG metadata. If you set a value larger than the the\n"
	"total number of metadata areas available, then metadata is stored in\n"
	"them all. The value 0 (unmanaged) disables this automatic management\n"
	"and allows you to control which metadata areas are used at the\n"
	"individual PV level using pvchange --metadataignore y|n.\n")

cfg(metadata_pvmetadatasize_CFG, "pvmetadatasize", metadata_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_PVMETADATASIZE, vsn(1, 0, 0), NULL, 0, NULL,
	"Approximate number of sectors to use for each metadata copy.\n"
	"VGs with large numbers of PVs or LVs, or VGs containing complex LV\n"
	"structures, may need additional space for VG metadata. The metadata\n"
	"areas are treated as circular buffers, so unused space becomes filled\n"
	"with an archive of the most recent previous versions of the metadata.\n")

cfg(metadata_pvmetadataignore_CFG, "pvmetadataignore", metadata_CFG_SECTION, CFG_ADVANCED | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_PVMETADATAIGNORE, vsn(2, 2, 69), NULL, 0, NULL,
	"Ignore metadata areas on a new PV.\n"
	"The --metadataignore option overrides this setting.\n"
	"If metadata areas on a PV are ignored, LVM will not store metadata\n"
	"in them.\n")

cfg(metadata_stripesize_CFG, "stripesize", metadata_CFG_SECTION, CFG_ADVANCED | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, DEFAULT_STRIPESIZE, vsn(1, 0, 0), NULL, 0, NULL, NULL)

cfg_array(metadata_dirs_CFG, "dirs", metadata_CFG_SECTION, CFG_ADVANCED | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL,
	  "This setting is no longer used.\n")

cfg_section(metadata_disk_areas_CFG_SUBSECTION, "disk_areas", metadata_CFG_SECTION, CFG_UNSUPPORTED | CFG_DEFAULT_COMMENTED, vsn(1, 0, 0), vsn(2, 3, 0), NULL, NULL)
cfg_section(disk_area_CFG_SUBSECTION, "disk_area", metadata_disk_areas_CFG_SUBSECTION, CFG_NAME_VARIABLE | CFG_UNSUPPORTED | CFG_DEFAULT_COMMENTED, vsn(1, 0, 0), vsn(2, 3, 0), NULL, NULL)
cfg(disk_area_start_sector_CFG, "start_sector", disk_area_CFG_SUBSECTION, CFG_UNSUPPORTED | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, 0, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL, NULL)
cfg(disk_area_size_CFG, "size", disk_area_CFG_SUBSECTION, CFG_UNSUPPORTED | CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, 0, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL, NULL)
cfg(disk_area_id_CFG, "id", disk_area_CFG_SUBSECTION, CFG_UNSUPPORTED | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 0), NULL, vsn(2, 3, 0), NULL, NULL)

cfg(report_output_format_CFG, "output_format", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED | CFG_DISALLOW_INTERACTIVE, CFG_TYPE_STRING, DEFAULT_REP_OUTPUT_FORMAT, vsn(2, 2, 158), NULL, 0, NULL,
	"Format of LVM command's report output.\n"
	"If there is more than one report per command, then the format\n"
	"is applied for all reports. You can also change output format\n"
	"directly on command line using --reportformat option which\n"
	"has precedence over log/output_format setting.\n"
	"Accepted values:\n"
	"  basic\n"
	"    Original format with columns and rows. If there is more than\n"
	"    one report per command, each report is prefixed with report's\n"
	"    name for identification.\n"
	"  json\n"
	"    JSON format.\n")

cfg(report_compact_output_CFG, "compact_output", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_COMPACT_OUTPUT, vsn(2, 2, 115), NULL, 0, NULL,
	"Do not print empty values for all report fields.\n"
	"If enabled, all fields that don't have a value set for any of the\n"
	"rows reported are skipped and not printed. Compact output is\n"
	"applicable only if report/buffered is enabled. If you need to\n"
	"compact only specified fields, use compact_output=0 and define\n"
	"report/compact_output_cols configuration setting instead.\n")

cfg(report_compact_output_cols_CFG, "compact_output_cols", report_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_COMPACT_OUTPUT_COLS, vsn(2, 2, 133), NULL, 0, NULL,
	"Do not print empty values for specified report fields.\n"
	"If defined, specified fields that don't have a value set for any\n"
	"of the rows reported are skipped and not printed. Compact output\n"
	"is applicable only if report/buffered is enabled. If you need to\n"
	"compact all fields, use compact_output=1 instead in which case\n"
	"the compact_output_cols setting is then ignored.\n")

cfg(report_aligned_CFG, "aligned", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_ALIGNED, vsn(1, 0, 0), NULL, 0, NULL,
	"Align columns in report output.\n")

cfg(report_buffered_CFG, "buffered", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_BUFFERED, vsn(1, 0, 0), NULL, 0, NULL,
	"Buffer report output.\n"
	"When buffered reporting is used, the report's content is appended\n"
	"incrementally to include each object being reported until the report\n"
	"is flushed to output which normally happens at the end of command\n"
	"execution. Otherwise, if buffering is not used, each object is\n"
	"reported as soon as its processing is finished.\n")

cfg(report_headings_CFG, "headings", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_HEADINGS, vsn(1, 0, 0), NULL, 0, NULL,
	"Show headings for columns on report.\n")

cfg(report_separator_CFG, "separator", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_REP_SEPARATOR, vsn(1, 0, 0), NULL, 0, NULL,
	"A separator to use on report after each field.\n")

cfg(report_list_item_separator_CFG, "list_item_separator", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_REP_LIST_ITEM_SEPARATOR, vsn(2, 2, 108), NULL, 0, NULL,
	"A separator to use for list items when reported.\n")

cfg(report_prefixes_CFG, "prefixes", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_PREFIXES, vsn(2, 2, 36), NULL, 0, NULL,
	"Use a field name prefix for each field reported.\n")

cfg(report_quoted_CFG, "quoted", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_QUOTED, vsn(2, 2, 39), NULL, 0, NULL,
	"Quote field values when using field name prefixes.\n")

cfg(report_columns_as_rows_CFG, "columns_as_rows", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_REP_COLUMNS_AS_ROWS, vsn(1, 0, 0), NULL, 0, NULL,
	"Output each column as a row.\n"
	"If set, this also implies report/prefixes=1.\n")

cfg(report_binary_values_as_numeric_CFG, "binary_values_as_numeric", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, 0, vsn(2, 2, 108), NULL, 0, NULL,
	"Use binary values 0 or 1 instead of descriptive literal values.\n"
	"For columns that have exactly two valid values to report\n"
	"(not counting the 'unknown' value which denotes that the\n"
	"value could not be determined).\n")

cfg(report_time_format_CFG, "time_format", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_TIME_FORMAT, vsn(2, 2, 123), NULL, 0, NULL,
        "Set time format for fields reporting time values.\n"
	"Format specification is a string which may contain special character\n"
	"sequences and ordinary character sequences. Ordinary character\n"
	"sequences are copied verbatim. Each special character sequence is\n"
	"introduced by the '%' character and such sequence is then\n"
	"substituted with a value as described below.\n"
	"#\n"
	"Accepted values:\n"
	"  %a\n"
	"    The abbreviated name of the day of the week according to the\n"
	"    current locale.\n"
	"  %A\n"
	"    The full name of the day of the week according to the current\n"
	"    locale.\n"
	"  %b\n"
	"    The abbreviated month name according to the current locale.\n"
	"  %B\n"
	"    The full month name according to the current locale.\n"
	"  %c\n"
	"    The preferred date and time representation for the current\n"
	"    locale (alt E)\n"
	"  %C\n"
	"    The century number (year/100) as a 2-digit integer. (alt E)\n"
	"  %d\n"
	"    The day of the month as a decimal number (range 01 to 31).\n"
	"    (alt O)\n"
	"  %D\n"
	"    Equivalent to %m/%d/%y. (For Americans only. Americans should\n"
	"    note that in other countries%d/%m/%y is rather common. This\n"
	"    means that in international context this format is ambiguous and\n"
	"    should not be used.\n"
	"  %e\n"
	"    Like %d, the day of the month as a decimal number, but a leading\n"
	"    zero is replaced by a space. (alt O)\n"
	"  %E\n"
	"    Modifier: use alternative local-dependent representation if\n"
	"    available.\n"
	"  %F\n"
	"    Equivalent to %Y-%m-%d (the ISO 8601 date format).\n"
	"  %G\n"
	"    The ISO 8601 week-based year with century as adecimal number.\n"
	"    The 4-digit year corresponding to the ISO week number (see %V).\n"
	"    This has the same format and value as %Y, except that if the\n"
	"    ISO week number belongs to the previous or next year, that year\n"
	"    is used instead.\n"
	"  %g\n"
	"    Like %G, but without century, that is, with a 2-digit year\n"
	"    (00-99).\n"
	"  %h\n"
	"    Equivalent to %b.\n"
	"  %H\n"
	"    The hour as a decimal number using a 24-hour clock\n"
	"    (range 00 to 23). (alt O)\n"
	"  %I\n"
	"    The hour as a decimal number using a 12-hour clock\n"
	"    (range 01 to 12). (alt O)\n"
	"  %j\n"
	"    The day of the year as a decimal number (range 001 to 366).\n"
	"  %k\n"
	"    The hour (24-hour clock) as a decimal number (range 0 to 23);\n"
	"    single digits are preceded by a blank. (See also %H.)\n"
	"  %l\n"
	"    The hour (12-hour clock) as a decimal number (range 1 to 12);\n"
	"    single digits are preceded by a blank. (See also %I.)\n"
	"  %m\n"
	"    The month as a decimal number (range 01 to 12). (alt O)\n"
	"  %M\n"
	"    The minute as a decimal number (range 00 to 59). (alt O)\n"
	"  %O\n"
	"    Modifier: use alternative numeric symbols.\n"
	"  %p\n"
	"    Either \"AM\" or \"PM\" according to the given time value,\n"
	"    or the corresponding strings for the current locale. Noon is\n"
	"    treated as \"PM\" and midnight as \"AM\".\n"
	"  %P\n"
	"    Like %p but in lowercase: \"am\" or \"pm\" or a corresponding\n"
	"    string for the current locale.\n"
	"  %r\n"
	"    The time in a.m. or p.m. notation. In the POSIX locale this is\n"
	"    equivalent to %I:%M:%S %p.\n"
	"  %R\n"
	"    The time in 24-hour notation (%H:%M). For a version including\n"
	"    the seconds, see %T below.\n"
	"  %s\n"
	"    The number of seconds since the Epoch,\n"
	"    1970-01-01 00:00:00 +0000 (UTC)\n"
	"  %S\n"
	"    The second as a decimal number (range 00 to 60). (The range is\n"
	"    up to 60 to allow for occasional leap seconds.) (alt O)\n"
	"  %t\n"
	"    A tab character.\n"
	"  %T\n"
	"    The time in 24-hour notation (%H:%M:%S).\n"
	"  %u\n"
	"    The day of the week as a decimal, range 1 to 7, Monday being 1.\n"
	"    See also %w. (alt O)\n"
	"  %U\n"
	"    The week number of the current year as a decimal number,\n"
	"    range 00 to 53, starting with the first Sunday as the first\n"
	"    day of week 01. See also %V and %W. (alt O)\n"
	"  %V\n"
	"    The ISO 8601 week number of the current year as a decimal number,\n"
	"    range 01 to 53, where week 1 is the first week that has at least\n"
	"    4 days in the new year. See also %U and %W. (alt O)\n"
	"  %w\n"
	"    The day of the week as a decimal, range 0 to 6, Sunday being 0.\n"
	"    See also %u. (alt O)\n"
	"  %W\n"
	"    The week number of the current year as a decimal number,\n"
	"    range 00 to 53, starting with the first Monday as the first day\n"
	"    of week 01. (alt O)\n"
	"  %x\n"
	"    The preferred date representation for the current locale without\n"
	"    the time. (alt E)\n"
	"  %X\n"
	"    The preferred time representation for the current locale without\n"
	"    the date. (alt E)\n"
	"  %y\n"
	"    The year as a decimal number without a century (range 00 to 99).\n"
	"    (alt E, alt O)\n"
	"  %Y\n"
	"    The year as a decimal number including the century. (alt E)\n"
	"  %z\n"
	"    The +hhmm or -hhmm numeric timezone (that is, the hour and minute\n"
	"    offset from UTC).\n"
	"  %Z\n"
	"    The timezone name or abbreviation.\n"
	"  %%\n"
	"    A literal '%' character.\n"
	"#\n")

cfg(report_devtypes_sort_CFG, "devtypes_sort", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DEVTYPES_SORT, vsn(2, 2, 101), NULL, 0, NULL,
	"List of columns to sort by when reporting 'lvm devtypes' command.\n"
	"See 'lvm devtypes -o help' for the list of possible fields.\n")

cfg(report_devtypes_cols_CFG, "devtypes_cols", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DEVTYPES_COLS, vsn(2, 2, 101), NULL, 0, NULL,
	"List of columns to report for 'lvm devtypes' command.\n"
	"See 'lvm devtypes -o help' for the list of possible fields.\n")

cfg(report_devtypes_cols_verbose_CFG, "devtypes_cols_verbose", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DEVTYPES_COLS_VERB, vsn(2, 2, 101), NULL, 0, NULL,
	"List of columns to report for 'lvm devtypes' command in verbose mode.\n"
	"See 'lvm devtypes -o help' for the list of possible fields.\n")

cfg(report_lvs_sort_CFG, "lvs_sort", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_LVS_SORT, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to sort by when reporting 'lvs' command.\n"
	"See 'lvs -o help' for the list of possible fields.\n")

cfg(report_lvs_cols_CFG, "lvs_cols", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_LVS_COLS, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'lvs' command.\n"
	"See 'lvs -o help' for the list of possible fields.\n")

cfg(report_lvs_cols_verbose_CFG, "lvs_cols_verbose", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_LVS_COLS_VERB, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'lvs' command in verbose mode.\n"
	"See 'lvs -o help' for the list of possible fields.\n")

cfg(report_vgs_sort_CFG, "vgs_sort", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VGS_SORT, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to sort by when reporting 'vgs' command.\n"
	"See 'vgs -o help' for the list of possible fields.\n")

cfg(report_vgs_cols_CFG, "vgs_cols", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VGS_COLS, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'vgs' command.\n"
	"See 'vgs -o help' for the list of possible fields.\n")

cfg(report_vgs_cols_verbose_CFG, "vgs_cols_verbose", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VGS_COLS_VERB, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'vgs' command in verbose mode.\n"
	"See 'vgs -o help' for the list of possible fields.\n")

cfg(report_pvs_sort_CFG, "pvs_sort", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVS_SORT, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to sort by when reporting 'pvs' command.\n"
	"See 'pvs -o help' for the list of possible fields.\n")

cfg(report_pvs_cols_CFG, "pvs_cols", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVS_COLS, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'pvs' command.\n"
	"See 'pvs -o help' for the list of possible fields.\n")

cfg(report_pvs_cols_verbose_CFG, "pvs_cols_verbose", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVS_COLS_VERB, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'pvs' command in verbose mode.\n"
	"See 'pvs -o help' for the list of possible fields.\n")

cfg(report_segs_sort_CFG, "segs_sort", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_SEGS_SORT, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to sort by when reporting 'lvs --segments' command.\n"
	"See 'lvs --segments -o help' for the list of possible fields.\n")

cfg(report_segs_cols_CFG, "segs_cols", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_SEGS_COLS, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'lvs --segments' command.\n"
	"See 'lvs --segments -o help' for the list of possible fields.\n")

cfg(report_segs_cols_verbose_CFG, "segs_cols_verbose", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_SEGS_COLS_VERB, vsn(1, 0, 0), NULL, 0, NULL,
	"List of columns to report for 'lvs --segments' command in verbose mode.\n"
	"See 'lvs --segments -o help' for the list of possible fields.\n")

cfg(report_pvsegs_sort_CFG, "pvsegs_sort", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVSEGS_SORT, vsn(1, 1, 3), NULL, 0, NULL,
	"List of columns to sort by when reporting 'pvs --segments' command.\n"
	"See 'pvs --segments -o help' for the list of possible fields.\n")

cfg(report_pvsegs_cols_CFG, "pvsegs_cols", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVSEGS_COLS, vsn(1, 1, 3), NULL, 0, NULL,
	"List of columns to sort by when reporting 'pvs --segments' command.\n"
	"See 'pvs --segments -o help' for the list of possible fields.\n")

cfg(report_pvsegs_cols_verbose_CFG, "pvsegs_cols_verbose", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVSEGS_COLS_VERB, vsn(1, 1, 3), NULL, 0, NULL,
	"List of columns to sort by when reporting 'pvs --segments' command in verbose mode.\n"
	"See 'pvs --segments -o help' for the list of possible fields.\n")

cfg(report_vgs_cols_full_CFG, "vgs_cols_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VGS_COLS_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to report for lvm fullreport's 'vgs' subreport.\n"
	"See 'vgs -o help' for the list of possible fields.\n")

cfg(report_pvs_cols_full_CFG, "pvs_cols_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVS_COLS_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to report for lvm fullreport's 'vgs' subreport.\n"
	"See 'pvs -o help' for the list of possible fields.\n")

cfg(report_lvs_cols_full_CFG, "lvs_cols_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_LVS_COLS_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to report for lvm fullreport's 'lvs' subreport.\n"
	"See 'lvs -o help' for the list of possible fields.\n")

cfg(report_pvsegs_cols_full_CFG, "pvsegs_cols_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVSEGS_COLS_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to report for lvm fullreport's 'pvseg' subreport.\n"
	"See 'pvs --segments -o help' for the list of possible fields.\n")

cfg(report_segs_cols_full_CFG, "segs_cols_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_SEGS_COLS_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to report for lvm fullreport's 'seg' subreport.\n"
	"See 'lvs --segments -o help' for the list of possible fields.\n")

cfg(report_vgs_sort_full_CFG, "vgs_sort_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_VGS_SORT_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to sort by when reporting lvm fullreport's 'vgs' subreport.\n"
	"See 'vgs -o help' for the list of possible fields.\n")

cfg(report_pvs_sort_full_CFG, "pvs_sort_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVS_SORT_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to sort by when reporting lvm fullreport's 'vgs' subreport.\n"
	"See 'pvs -o help' for the list of possible fields.\n")

cfg(report_lvs_sort_full_CFG, "lvs_sort_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_LVS_SORT_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to sort by when reporting lvm fullreport's 'lvs' subreport.\n"
	"See 'lvs -o help' for the list of possible fields.\n")

cfg(report_pvsegs_sort_full_CFG, "pvsegs_sort_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_PVSEGS_SORT_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to sort by when reporting for lvm fullreport's 'pvseg' subreport.\n"
	"See 'pvs --segments -o help' for the list of possible fields.\n")

cfg(report_segs_sort_full_CFG, "segs_sort_full", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_SEGS_SORT_FULL, vsn(2, 2, 158), NULL, 0, NULL,
	"List of columns to sort by when reporting lvm fullreport's 'seg' subreport.\n"
	"See 'lvs --segments -o help' for the list of possible fields.\n")

cfg(report_mark_hidden_devices_CFG, "mark_hidden_devices", report_CFG_SECTION, CFG_PROFILABLE | CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, 1, vsn(2, 2, 140), NULL, 0, NULL,
	"Use brackets [] to mark hidden devices.\n")

cfg(report_two_word_unknown_device_CFG, "two_word_unknown_device", report_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, 0, vsn(2, 2, 146), NULL, 0, NULL,
	"Use the two words 'unknown device' in place of '[unknown]'.\n"
	"This is displayed when the device for a PV is not known.\n")

cfg(dmeventd_mirror_library_CFG, "mirror_library", dmeventd_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_DMEVENTD_MIRROR_LIB, vsn(1, 2, 3), NULL, 0, NULL,
	"The library dmeventd uses when monitoring a mirror device.\n"
	"libdevmapper-event-lvm2mirror.so attempts to recover from\n"
	"failures. It removes failed devices from a volume group and\n"
	"reconfigures a mirror as necessary. If no mirror library is\n"
	"provided, mirrors are not monitored through dmeventd.\n")

cfg(dmeventd_raid_library_CFG, "raid_library", dmeventd_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DMEVENTD_RAID_LIB, vsn(2, 2, 87), NULL, 0, NULL, NULL)

cfg(dmeventd_snapshot_library_CFG, "snapshot_library", dmeventd_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_DMEVENTD_SNAPSHOT_LIB, vsn(1, 2, 26), NULL, 0, NULL,
	"The library dmeventd uses when monitoring a snapshot device.\n"
	"libdevmapper-event-lvm2snapshot.so monitors the filling of snapshots\n"
	"and emits a warning through syslog when the usage exceeds 80%. The\n"
	"warning is repeated when 85%, 90% and 95% of the snapshot is filled.\n")

cfg(dmeventd_thin_library_CFG, "thin_library", dmeventd_CFG_SECTION, 0, CFG_TYPE_STRING, DEFAULT_DMEVENTD_THIN_LIB, vsn(2, 2, 89), NULL, 0, NULL,
	"The library dmeventd uses when monitoring a thin device.\n"
	"libdevmapper-event-lvm2thin.so monitors the filling of a pool\n"
	"and emits a warning through syslog when the usage exceeds 80%. The\n"
	"warning is repeated when 85%, 90% and 95% of the pool is filled.\n")

cfg(dmeventd_thin_command_CFG, "thin_command", dmeventd_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DMEVENTD_THIN_COMMAND, vsn(2, 2, 169), NULL, 0, NULL,
	"The plugin runs command with each 5% increment when thin-pool data volume\n"
	"or metadata volume gets above 50%.\n"
	"Command which starts with 'lvm ' prefix is internal lvm command.\n"
	"You can write your own handler to customise behaviour in more details.\n"
	"User handler is specified with the full path starting with '/'.\n")
	/* TODO: systemd service handler */

cfg(dmeventd_vdo_library_CFG, "vdo_library", dmeventd_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DMEVENTD_VDO_LIB, VDO_1ST_VSN, NULL, 0, NULL,
	"The library dmeventd uses when monitoring a VDO pool device.\n"
	"libdevmapper-event-lvm2vdo.so monitors the filling of a pool\n"
	"and emits a warning through syslog when the usage exceeds 80%. The\n"
	"warning is repeated when 85%, 90% and 95% of the pool is filled.\n")

cfg(dmeventd_vdo_command_CFG, "vdo_command", dmeventd_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DMEVENTD_VDO_COMMAND, VDO_1ST_VSN, NULL, 0, NULL,
	"The plugin runs command with each 5% increment when VDO pool volume\n"
	"gets above 50%.\n"
	"Command which starts with 'lvm ' prefix is internal lvm command.\n"
	"You can write your own handler to customise behaviour in more details.\n"
	"User handler is specified with the full path starting with '/'.\n")
	/* TODO: systemd service handler */

cfg(dmeventd_executable_CFG, "executable", dmeventd_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, DEFAULT_DMEVENTD_PATH, vsn(2, 2, 73), "@DMEVENTD_PATH@", 0, NULL,
	"The full path to the dmeventd binary.\n")

cfg(tags_hosttags_CFG, "hosttags", tags_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_BOOL, DEFAULT_HOSTTAGS, vsn(1, 0, 18), NULL, 0, NULL,
	"Create a host tag using the machine name.\n"
	"The machine name is nodename returned by uname(2).\n")

cfg_section(tag_CFG_SUBSECTION, "tag", tags_CFG_SECTION, CFG_NAME_VARIABLE | CFG_DEFAULT_COMMENTED, vsn(1, 0, 18), 0, NULL,
	"Replace this subsection name with a custom tag name.\n"
	"Multiple subsections like this can be created. The '@' prefix for\n"
	"tags is optional. This subsection can contain host_list, which is a\n"
	"list of machine names. If the name of the local machine is found in\n"
	"host_list, then the name of this subsection is used as a tag and is\n"
	"applied to the local machine as a 'host tag'. If this subsection is\n"
	"empty (has no host_list), then the subsection name is always applied\n"
	"as a 'host tag'.\n"
	"#\n"
	"Example\n"
	"The host tag foo is given to all hosts, and the host tag\n"
	"bar is given to the hosts named machine1 and machine2.\n"
	"tags { foo { } bar { host_list = [ \"machine1\", \"machine2\" ] } }\n"
	"#\n")

cfg_array(tag_host_list_CFG, "host_list", tag_CFG_SUBSECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(1, 0, 18), NULL, 0, NULL,
	"A list of machine names.\n"
	"These machine names are compared to the nodename returned\n"
	"by uname(2). If the local machine name matches an entry in\n"
	"this list, the name of the subsection is applied to the\n"
	"machine as a 'host tag'.\n")

cfg(local_system_id_CFG, "system_id", local_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_COMMENTED, CFG_TYPE_STRING, NULL, vsn(2, 2, 117), NULL, 0, NULL,
	"Defines the local system ID for lvmlocal mode.\n"
	"This is used when global/system_id_source is set to 'lvmlocal' in the\n"
	"main configuration file, e.g. lvm.conf. When used, it must be set to\n"
	"a unique value among all hosts sharing access to the storage,\n"
	"e.g. a host name.\n"
	"#\n"
	"Example\n"
	"Set no system ID:\n"
	"system_id = \"\"\n"
	"Set the system_id to a specific name:\n"
	"system_id = \"host1\"\n"
	"#\n")

cfg_array(local_extra_system_ids_CFG, "extra_system_ids", local_CFG_SECTION, CFG_ALLOW_EMPTY | CFG_DEFAULT_UNDEFINED, CFG_TYPE_STRING, NULL, vsn(2, 2, 117), NULL, 0, NULL,
	"A list of extra VG system IDs the local host can access.\n"
	"VGs with the system IDs listed here (in addition to the host's own\n"
	"system ID) can be fully accessed by the local host. (These are\n"
	"system IDs that the host sees in VGs, not system IDs that identify\n"
	"the local host, which is determined by system_id_source.)\n"
	"Use this only after consulting 'man lvmsystemid' to be certain of\n"
	"correct usage and possible dangers.\n")

cfg(local_host_id_CFG, "host_id", local_CFG_SECTION, CFG_DEFAULT_COMMENTED, CFG_TYPE_INT, 0, vsn(2, 2, 124), NULL, 0, NULL,
	"The lvmlockd sanlock host_id.\n"
	"This must be unique among all hosts, and must be between 1 and 2000.\n"
	"Applicable only if LVM is compiled with lockd support\n")

cfg(CFG_COUNT, NULL, root_CFG_SECTION, 0, CFG_TYPE_INT, 0, vsn(0, 0, 0), NULL, 0, NULL, NULL)
