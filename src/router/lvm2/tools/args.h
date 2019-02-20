/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2016 Red Hat, Inc. All rights reserved.
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
 * Put all long args that don't have a corresponding short option first.
 */
/* *INDENT-OFF* */
arg(ARG_UNUSED, '-', "", 0, 0, 0, NULL)  /* place holder for unused 0 value */

arg(abort_ARG, '\0', "abort", 0, 0, 0,
    "#pvmove\n"
    "Abort any pvmove operations in progress. If a pvmove was started\n"
    "with the --atomic option, then all LVs will remain on the source PV.\n"
    "Otherwise, segments that have been moved will remain on the\n"
    "destination PV, while unmoved segments will remain on the source PV.\n"
    "#lvpoll\n"
    "Stop processing a poll operation in lvmpolld.\n")

arg(activationmode_ARG, '\0', "activationmode", activationmode_VAL, 0, 0,
    "Determines if LV activation is allowed when PVs are missing,\n"
    "e.g. because of a device failure.\n"
    "\\fBcomplete\\fP only allows LVs with no missing PVs to be activated,\n"
    "and is the most restrictive mode.\n"
    "\\fBdegraded\\fP allows RAID LVs with missing PVs to be activated.\n"
    "(This does not include the \"mirror\" type, see \"raid1\" instead.)\n"
    "\\fBpartial\\fP allows any LV with missing PVs to be activated, and\n"
    "should only be used for recovery or repair.\n"
    "For default, see lvm.conf/activation_mode.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n")

arg(addtag_ARG, '\0', "addtag", tag_VAL, ARG_GROUPABLE, 0,
    "Adds a tag to a PV, VG or LV. This option can be repeated to add\n"
    "multiple tags at once. See \\fBlvm\\fP(8) for information about tags.\n")

arg(aligned_ARG, '\0', "aligned", 0, 0, 0,
    "Use with --separator to align the output columns\n")

arg(alloc_ARG, '\0', "alloc", alloc_VAL, 0, 0,
    "Determines the allocation policy when a command needs to allocate\n"
    "Physical Extents (PEs) from the VG. Each VG and LV has an allocation policy\n"
    "which can be changed with vgchange/lvchange, or overriden on the\n"
    "command line.\n"
    "\\fBnormal\\fP applies common sense rules such as not placing parallel stripes\n"
    "on the same PV.\n"
    "\\fBinherit\\fP applies the VG policy to an LV.\n"
    "\\fBcontiguous\\fP requires new PEs be placed adjacent to existing PEs.\n"
    "\\fBcling\\fP places new PEs on the same PV as existing PEs in the same\n"
    "stripe of the LV.\n"
    "If there are sufficient PEs for an allocation, but normal does not\n"
    "use them, \\fBanywhere\\fP will use them even if it reduces performance,\n"
    "e.g. by placing two stripes on the same PV.\n"
    "Optional positional PV args on the command line can also be used to limit\n"
    "which PVs the command will use for allocation.\n"
    "See \\fBlvm\\fP(8) for more information about allocation.\n")

arg(atomic_ARG, '\0', "atomic", 0, 0, 0,
    "Makes a pvmove operation atomic, ensuring that all affected LVs are\n"
    "moved to the destination PV, or none are if the operation is aborted.\n")

arg(atversion_ARG, '\0', "atversion", string_VAL, 0, 0,
    "Specify an LVM version in x.y.z format where x is the major version,\n"
    "the y is the minor version and z is the patchlevel (e.g. 2.2.106).\n"
    "When configuration is displayed, the configuration settings recognized\n"
    "at this LVM version will be considered only. This can be used\n"
    "to display a configuration that a certain LVM version understands and\n"
    "which does not contain any newer settings for which LVM would\n"
    "issue a warning message when checking the configuration.\n")

arg(binary_ARG, '\0', "binary", 0, 0, 0,
    "Use binary values \"0\" or \"1\" instead of descriptive literal values\n"
    "for columns that have exactly two valid values to report (not counting\n"
    "the \"unknown\" value which denotes that the value could not be determined).\n")

arg(bootloaderareasize_ARG, '\0', "bootloaderareasize", sizemb_VAL, 0, 0,
    "Create a separate bootloader area of specified size besides PV's data\n"
    "area. The bootloader area is an area of reserved space on the PV from\n"
    "which LVM will not allocate any extents and it's kept untouched. This is\n"
    "primarily aimed for use with bootloaders to embed their own data or metadata.\n"
    "The start of the bootloader area is always aligned, see also --dataalignment\n"
    "and --dataalignmentoffset. The bootloader area size may eventually\n"
    "end up increased due to the alignment, but it's never less than the\n"
    "size that is requested. To see the bootloader area start and size of\n"
    "an existing PV use pvs -o +pv_ba_start,pv_ba_size.\n")

arg(cache_long_ARG, '\0', "cache", 0, 0, 0,
    "#pvscan\n"
    "Scan one or more devices and record that they are online.\n"
    "#vgscan\n"
    "This option is no longer used.\n"
    "#lvscan\n"
    "This option is no longer used.\n")

arg(cachemetadataformat_ARG, '\0', "cachemetadataformat", cachemetadataformat_VAL, 0, 0,
    "Specifies the cache metadata format used by cache target.\n")

arg(cachemode_ARG, '\0', "cachemode", cachemode_VAL, 0, 0,
    "Specifies when writes to a cache LV should be considered complete.\n"
    "\\fBwriteback\\fP considers a write complete as soon as it is\n"
    "stored in the cache pool.\n"
    "\\fBwritethough\\fP considers a write complete only when it has\n"
    "been stored in both the cache pool and on the origin LV.\n"
    "While writethrough may be slower for writes, it is more\n"
    "resilient if something should happen to a device associated with the\n"
    "cache pool LV. With \\fBpassthrough\\fP, all reads are served\n"
    "from the origin LV (all reads miss the cache) and all writes are\n"
    "forwarded to the origin LV; additionally, write hits cause cache\n"
    "block invalidates. See \\fBlvmcache\\fP(7) for more information.\n")

arg(cachepool_ARG, '\0', "cachepool", lv_VAL, 0, 0,
    "The name of a cache pool LV.\n")

arg(commandprofile_ARG, '\0', "commandprofile", string_VAL, 0, 0,
    "The command profile to use for command configuration.\n"
    "See \\fBlvm.conf\\fP(5) for more information about profiles.\n")

arg(compression_ARG, '\0', "compression", bool_VAL, 0, 0,
    "Controls whether compression is enabled or disable for VDO volume.\n"
    "See \\fBlvmvdo\\fP(7) for more information about VDO usage.\n")

arg(config_ARG, '\0', "config", string_VAL, 0, 0,
    "Config settings for the command. These override lvm.conf settings.\n"
    "The String arg uses the same format as lvm.conf,\n"
    "or may use section/field syntax.\n"
    "See \\fBlvm.conf\\fP(5) for more information about config.\n")

arg(configreport_ARG, '\0', "configreport", configreport_VAL, ARG_GROUPABLE, 1,
    "See \\fBlvmreport\\fP(7).\n")

arg(configtype_ARG, '\0', "typeconfig", configtype_VAL, 0, 0,
    "\\fBcurrent\\fP prints the config settings that would be applied\n"
    "to an lvm command (assuming the command does not override them\n"
    "on the command line.) This includes:\n"
    "settings that have been modified in lvm config files,\n"
    "settings that get their default values from config files,\n"
    "and default settings that have been uncommented in config files.\n"
    "\\fBdefault\\fP prints all settings with their default values.\n"
    "Changes made in lvm config files are not reflected in the output.\n"
    "Some settings get their default values internally,\n"
    "and these settings are printed as comments.\n"
    "Other settings get their default values from config files,\n"
    "and these settings are not printed as comments.\n"
    "\\fBdiff\\fP prints only config settings that have been modified\n"
    "from their default values in config files (the difference between\n"
    "current and default.)\n"
    "\\fBfull\\fP prints every setting uncommented and set to the\n"
    "current value, i.e. how it would be used by an lvm command.\n"
    "This includes settings modified in config files, settings that usually\n"
    "get defaults internally, and settings that get defaults from config files.\n"
    "\\fBlist\\fP prints all config names without values.\n"
    "\\fBmissing\\fP prints settings that are missing from the\n"
    "lvm config files. A missing setting that usually gets its default\n"
    "from config files is printed uncommented and set to the internal default.\n"
    "Settings that get their default internally and are not set in config files\n"
    "are printed commented with the internal default.\n"
    "\\fBnew\\fP prints config settings that have been added since\n"
    "the lvm version specified by --sinceversion. They are printed\n"
    "with their default values.\n"
    "\\fBprofilable\\fP prints settings with their default values that can be set from a profile.\n"
    "\\fBprofilable-command\\fP prints settings with their default values that can be set from a command profile.\n"
    "\\fBprofilable-metadata\\fP prints settings with their default values that can be set from a metadata profile.\n"
    "Also see \\fBlvm.conf\\fP(5).\n")

arg(dataalignment_ARG, '\0', "dataalignment", sizekb_VAL, 0, 0,
    "Align the start of the data to a multiple of this number.\n"
    "Also specify an appropriate Physical Extent size when creating a VG.\n"
    "To see the location of the first Physical Extent of an existing PV,\n"
    "use pvs -o +pe_start. In addition, it may be shifted by an alignment offset.\n"
    "See lvm.conf/data_alignment_offset_detection and --dataalignmentoffset.\n")

arg(dataalignmentoffset_ARG, '\0', "dataalignmentoffset", sizekb_VAL, 0, 0,
    "Shift the start of the data area by this additional offset.\n")

arg(deduplication_ARG, '\0', "deduplication", bool_VAL, 0, 0,
    "Controls whether deduplication is enabled or disable for VDO volume.\n"
    "See \\fBlvmvdo\\fP(7) for more information about VDO usage.\n")

arg(deltag_ARG, '\0', "deltag", tag_VAL, ARG_GROUPABLE, 0,
    "Deletes a tag from a PV, VG or LV. This option can be repeated to delete\n"
    "multiple tags at once. See \\fBlvm\\fP(8) for information about tags.\n")

arg(detachprofile_ARG, '\0', "detachprofile", 0, 0, 0,
    "Detaches a metadata profile from a VG or LV.\n"
    "See \\fBlvm.conf\\fP(5) for more information about profiles.\n")

arg(discards_ARG, '\0', "discards", discards_VAL, 0, 0,
    "Specifies how the device-mapper thin pool layer in the kernel should\n"
    "handle discards.\n"
    "\\fBignore\\fP causes the thin pool to ignore discards.\n"
    "\\fBnopassdown\\fP causes the thin pool to process discards itself to\n"
    "allow reuse of unneeded extents in the thin pool.\n"
    "\\fBpassdown\\fP causes the thin pool to process discards itself\n"
    "(like nopassdown) and pass the discards to the underlying device.\n"
    "See \\fBlvmthin\\fP(7) for more information.\n")

arg(driverloaded_ARG, '\0', "driverloaded", bool_VAL, 0, 0,
    "If set to no, the command will not attempt to use device-mapper.\n"
    "For testing and debugging.\n")

arg(errorwhenfull_ARG, '\0', "errorwhenfull", bool_VAL, 0, 0,
    "Specifies thin pool behavior when data space is exhausted.\n"
    "When yes, device-mapper will immediately return an error\n"
    "when a thin pool is full and an I/O request requires space.\n"
    "When no, device-mapper will queue these I/O requests for a\n"
    "period of time to allow the thin pool to be extended.\n"
    "Errors are returned if no space is available after the timeout.\n"
    "(Also see dm-thin-pool kernel module option no_space_timeout.)\n"
    "See \\fBlvmthin\\fP(7) for more information.\n")

arg(force_long_ARG, '\0', "force", 0, ARG_COUNTABLE, 0,
    "Force metadata restore even with thin pool LVs.\n"
    "Use with extreme caution. Most changes to thin metadata\n"
    "cannot be reverted.\n"
    "You may lose data if you restore metadata that does not match the\n"
    "thin pool kernel metadata precisely.\n")

arg(foreign_ARG, '\0', "foreign", 0, 0, 0,
    "Report/display foreign VGs that would otherwise be skipped.\n"
    "See \\fBlvmsystemid\\fP(7) for more information about foreign VGs.\n")

arg(handlemissingpvs_ARG, '\0', "handlemissingpvs", 0, 0, 0,
    "Allows a polling operation to continue when PVs are missing,\n"
    "e.g. for repairs due to faulty devices.\n")

arg(ignoreadvanced_ARG, '\0', "ignoreadvanced", 0, 0, 0,
    "Exclude advanced configuration settings from the output.\n")

arg(ignorelocal_ARG, '\0', "ignorelocal", 0, 0, 0,
    "Ignore the local section. The local section should be defined in\n"
    "the lvmlocal.conf file, and should contain config settings\n"
    "specific to the local host which should not be copied to\n"
    "other hosts.\n")

arg(ignorelockingfailure_ARG, '\0', "ignorelockingfailure", 0, 0, 0,
    "Allows a command to continue with read-only metadata\n"
    "operations after locking failures.\n")

arg(ignoremonitoring_ARG, '\0', "ignoremonitoring", 0, 0, 0,
    "Do not interact with dmeventd unless --monitor is specified.\n"
    "Do not use this if dmeventd is already monitoring a device.\n")

arg(ignoreskippedcluster_ARG, '\0', "ignoreskippedcluster", 0, 0, 0,
    "No longer used.\n")

arg(ignoreunsupported_ARG, '\0', "ignoreunsupported", 0, 0, 0,
    "Exclude unsupported configuration settings from the output. These settings are\n"
    "either used for debugging and development purposes only or their support is not\n"
    "yet complete and they are not meant to be used in production. The \\fBcurrent\\fP\n"
    "and \\fBdiff\\fP types include unsupported settings in their output by default,\n"
    "all the other types ignore unsupported settings.\n")

arg(labelsector_ARG, '\0', "labelsector", number_VAL, 0, 0,
    "By default the PV is labelled with an LVM2 identifier in its second\n"
    "sector (sector 1). This lets you use a different sector near the\n"
    "start of the disk (between 0 and 3 inclusive - see LABEL_SCAN_SECTORS\n"
    "in the source). Use with care.\n")

arg(lockopt_ARG, '\0', "lockopt", string_VAL, 0, 0,
    "Used to pass options for special cases to lvmlockd.\n"
    "See \\fBlvmlockd\\fP(8) for more information.\n")

arg(lockstart_ARG, '\0', "lockstart", 0, 0, 0,
    "Start the lockspace of a shared VG in lvmlockd.\n"
    "lvmlockd locks becomes available for the VG, allowing LVM to use the VG.\n"
    "See \\fBlvmlockd\\fP(8) for more information.\n")

arg(lockstop_ARG, '\0', "lockstop", 0, 0, 0,
    "Stop the lockspace of a shared VG in lvmlockd.\n"
    "lvmlockd locks become unavailable for the VG, preventing LVM from using the VG.\n"
    "See \\fBlvmlockd\\fP(8) for more information.\n")

arg(locktype_ARG, '\0', "locktype", locktype_VAL, 0, 0,
    "#vgchange\n"
    "Change the VG lock type to or from a shared lock type used with lvmlockd.\n"
    "See \\fBlvmlockd\\fP(8) for more information.\n"
    "#vgcreate\n"
    "Specify the VG lock type directly in place of using --shared.\n"
    "See \\fBlvmlockd\\fP(8) for more information.\n")

arg(logonly_ARG, '\0', "logonly", 0, 0, 0,
    "Suppress command report and display only log report.\n")

arg(longhelp_ARG, '\0', "longhelp", 0, 0, 0,
    "Display long help text.\n")

arg(maxrecoveryrate_ARG, '\0', "maxrecoveryrate", sizekb_VAL, 0, 0,
    "Sets the maximum recovery rate for a RAID LV.  The rate value\n"
    "is an amount of data per second for each device in the array.\n"
    "Setting the rate to 0 means it will be unbounded.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n")

arg(merge_ARG, '\0', "merge", 0, 0, 0,
    "An alias for --mergethin, --mergemirrors, or --mergesnapshot,\n"
    "depending on the type of LV.\n")

arg(mergemirrors_ARG, '\0', "mergemirrors", 0, 0, 0,
    "Merge LV images that were split from a raid1 LV.\n"
    "See --splitmirrors with --trackchanges.\n")

arg(mergesnapshot_ARG, '\0', "mergesnapshot", 0, 0, 0,
    "Merge COW snapshot LV into its origin.\n"
    "When merging a snapshot, if both the origin and snapshot LVs are not open,\n"
    "the merge will start immediately. Otherwise, the merge will start the\n"
    "first time either the origin or snapshot LV are activated and both are\n"
    "closed. Merging a snapshot into an origin that cannot be closed, for\n"
    "example a root filesystem, is deferred until the next time the origin\n"
    "volume is activated. When merging starts, the resulting LV will have the\n"
    "origin's name, minor number and UUID. While the merge is in progress,\n"
    "reads or writes to the origin appear as being directed to the snapshot\n"
    "being merged. When the merge finishes, the merged snapshot is removed.\n"
    "Multiple snapshots may be specified on the command line or a @tag may be\n"
    "used to specify multiple snapshots be merged to their respective origin.\n")

arg(mergethin_ARG, '\0', "mergethin", 0, 0, 0,
    "Merge thin LV into its origin LV.\n"
    "The origin thin LV takes the content of the thin snapshot,\n"
    "and the thin snapshot LV is removed.\n"
    "See \\fBlvmthin\\fP(7) for more information.\n")

arg(mergedconfig_ARG, '\0', "mergedconfig", 0, 0, 0,
    "When the command is run with --config\n"
    "and/or --commandprofile (or using LVM_COMMAND_PROFILE\n"
    "environment variable), --profile, or --metadataprofile,\n"
    "merge all the contents of the \"config cascade\" before displaying it.\n"
    "Without merging, only the configuration at the front of the\n"
    "cascade is displayed.\n"
    "See \\fBlvm.conf\\fP(5) for more information about config.\n")

arg(metadataignore_ARG, '\0', "metadataignore", bool_VAL, 0, 0,
    "Specifies the metadataignore property of a PV.\n"
    "If yes, metadata areas on the PV are ignored, and lvm will\n"
    "not store metadata in the metadata areas of the PV.\n"
    "If no, lvm will store metadata on the PV.\n")

arg(metadataprofile_ARG, '\0', "metadataprofile", string_VAL, 0, 0,
    "The metadata profile to use for command configuration.\n"
    "See \\fBlvm.conf\\fP(5) for more information about profiles.\n")

arg(metadatasize_ARG, '\0', "metadatasize", sizemb_VAL, 0, 0,
    "The approximate amount of space used for each VG metadata area.\n"
    "The size may be rounded.\n")

arg(minor_ARG, '\0', "minor", number_VAL, ARG_GROUPABLE, 0,
   "#lvcreate\n"
   "#lvchange\n"
   "Sets the minor number of an LV block device.\n"
   "#pvscan\n"
   "The minor number of a device.\n")

arg(minrecoveryrate_ARG, '\0', "minrecoveryrate", sizekb_VAL, 0, 0,
    "Sets the minimum recovery rate for a RAID LV.  The rate value\n"
    "is an amount of data per second for each device in the array.\n"
    "Setting the rate to 0 means it will be unbounded.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n")

arg(mirrorlog_ARG, '\0', "mirrorlog", mirrorlog_VAL, 0, 0,
    "Specifies the type of mirror log for LVs with the \"mirror\" type\n"
    "(does not apply to the \"raid1\" type.)\n"
    "\\fBdisk\\fP is a persistent log and requires a small amount of\n"
    "storage space, usually on a separate device from the data being mirrored.\n"
    "\\fBcore\\fP is not persistent; the log is kept only in memory.\n"
    "In this case, the mirror must be synchronized (by copying LV data from\n"
    "the first device to others) each time the LV is activated, e.g. after reboot.\n"
    "\\fBmirrored\\fP is a persistent log that is itself mirrored, but\n"
    "should be avoided. Instead, use the raid1 type for log redundancy.\n")

arg(mirrorsonly_ARG, '\0', "mirrorsonly", 0, 0, 0,
    "Only remove missing PVs from mirror LVs.\n")

arg(mknodes_ARG, '\0', "mknodes", 0, 0, 0,
    "Also checks the LVM special files in /dev that are needed for active\n"
    "LVs and creates any missing ones and removes unused ones.\n")

arg(monitor_ARG, '\0', "monitor", bool_VAL, 0, 0,
    "Start (yes) or stop (no) monitoring an LV with dmeventd.\n"
    "dmeventd monitors kernel events for an LV, and performs\n"
    "automated maintenance for the LV in reponse to specific events.\n"
    "See \\fBdmeventd\\fP(8) for more information.\n")

arg(nameprefixes_ARG, '\0', "nameprefixes", 0, 0, 0,
    "Add an \"LVM2_\" prefix plus the field name to the output. Useful\n"
    "with --noheadings to produce a list of field=value pairs that can\n"
    "be used to set environment variables (for example, in udev rules).\n")

arg(noheadings_ARG, '\0', "noheadings", 0, 0, 0,
    "Suppress the headings line that is normally the first line of output.\n"
    "Useful if grepping the output.\n")

arg(nohistory_ARG, '\0', "nohistory", 0, 0, 0,
    "Do not record history of LVs being removed.\n"
    "This has no effect unless the configuration setting\n"
    "metadata/record_lvs_history is enabled.\n")

arg(nolocking_ARG, '\0', "nolocking", 0, 0, 0,
    "Disable locking.\n")

arg(norestorefile_ARG, '\0', "norestorefile", 0, 0, 0,
    "In conjunction with --uuid, this allows a uuid to be specified\n"
    "without also requiring that a backup of the metadata be provided.\n")

arg(nosuffix_ARG, '\0', "nosuffix", 0, 0, 0,
    "Suppress the suffix on output sizes. Use with --units\n"
    "(except h and H) if processing the output.\n")

arg(nosync_ARG, '\0', "nosync", 0, 0, 0,
    "Causes the creation of mirror, raid1, raid4, raid5 and raid10 to skip the\n"
    "initial synchronization. In case of mirror, raid1 and raid10, any data\n"
    "written afterwards will be mirrored, but the original contents will not be\n"
    "copied. In case of raid4 and raid5, no parity blocks will be written,\n"
    "though any data written afterwards will cause parity blocks to be stored.\n"
    "This is useful for skipping a potentially long and resource intensive initial\n"
    "sync of an empty mirror/raid1/raid4/raid5 and raid10 LV.\n"
    "This option is not valid for raid6, because raid6 relies on proper parity\n"
    "(P and Q Syndromes) being created during initial synchronization in order\n"
    "to reconstruct proper user date in case of device failures.\n"
    "raid0 and raid0_meta do not provide any data copies or parity support\n"
    "and thus do not support initial synchronization.\n")

arg(notifydbus_ARG, '\0', "notifydbus", 0, 0, 0,
    "Send a notification to D-Bus. The command will exit with an error\n"
    "if LVM is not built with support for D-Bus notification, or if the\n"
    "notify_dbus config setting is disabled.\n")

arg(noudevsync_ARG, '\0', "noudevsync", 0, 0, 0,
    "Disables udev synchronisation. The process will not wait for notification\n"
    "from udev. It will continue irrespective of any possible udev processing\n"
    "in the background. Only use this if udev is not running or has rules that\n"
    "ignore the devices LVM creates.\n")

arg(originname_ARG, '\0', "originname", lv_VAL, 0, 0,
    "Specifies the name to use for the external origin LV when converting an LV\n"
    "to a thin LV. The LV being converted becomes a read-only external origin\n"
    "with this name.\n")

arg(setphysicalvolumesize_ARG, '\0', "setphysicalvolumesize", sizemb_VAL, 0, 0,
    "Overrides the automatically detected size of the PV.\n"
    "Use with care, or prior to reducing the physical size of the device.\n")

arg(poll_ARG, '\0', "poll", bool_VAL, 0, 0,
    "When yes, start the background transformation of an LV.\n"
    "An incomplete transformation, e.g. pvmove or lvconvert interrupted\n"
    "by reboot or crash, can be restarted from the last checkpoint with --poll y.\n"
    "When no, background transformation of an LV will not occur, and the\n"
    "transformation will not complete. It may not be appropriate to immediately\n"
    "poll an LV after activation, in which case --poll n can be used to defer\n"
    "polling until a later --poll y command.\n")

arg(polloperation_ARG, '\0', "polloperation", polloperation_VAL, 0, 0,
    "The command to perform from lvmpolld.\n")

/* Not used. */
arg(pooldatasize_ARG, '\0', "pooldatasize", sizemb_VAL, 0, 0, NULL)

arg(poolmetadata_ARG, '\0', "poolmetadata", lv_VAL, 0, 0,
    "The name of a an LV to use for storing pool metadata.\n")

arg(poolmetadatasize_ARG, '\0', "poolmetadatasize", sizemb_VAL, 0, 0,
    "#lvcreate\n"
    "#lvconvert\n"
    "Specifies the size of the new pool metadata LV.\n"
    "#lvresize\n"
    "#lvextend\n"
    "Specifies the new size of the pool metadata LV.\n"
    "The plus prefix \\fB+\\fP can be used, in which case\n"
    "the value is added to the current size.\n")

arg(poolmetadataspare_ARG, '\0', "poolmetadataspare", bool_VAL, 0, 0,
    "Enable or disable the automatic creation and management of a\n"
    "spare pool metadata LV in the VG. A spare metadata LV is reserved\n"
    "space that can be used when repairing a pool.\n")

arg(profile_ARG, '\0', "profile", string_VAL, 0, 0,
    "An alias for --commandprofile or --metadataprofile, depending\n"
    "on the command.\n")

arg(pvmetadatacopies_ARG, '\0', "pvmetadatacopies", pvmetadatacopies_VAL, 0, 0,
    "The number of metadata areas to set aside on a PV for storing VG metadata.\n"
    "When 2, one copy of the VG metadata is stored at the front of the PV\n"
    "and a second copy is stored at the end.\n"
    "When 1, one copy of the VG metadata is stored at the front of the PV\n"
    "(starting in the 5th sector).\n"
    "When 0, no copies of the VG metadata are stored on the given PV.\n"
    "This may be useful in VGs containing many PVs (this places limitations\n"
    "on the ability to use vgsplit later.)\n")

arg(readonly_ARG, '\0', "readonly", 0, 0, 0,
    "Run the command in a special read-only mode which will read on-disk\n"
    "metadata without needing to take any locks. This can be used to peek\n"
    "inside metadata used by a virtual machine image while the virtual\n"
    "machine is running. No attempt will be made to communicate with the\n"
    "device-mapper kernel driver, so this option is unable to report whether\n"
    "or not LVs are actually in use.\n")

arg(refresh_ARG, '\0', "refresh", 0, 0, 0,
    "If the LV is active, reload its metadata.\n"
    "This is not necessary in normal operation, but may be useful\n"
    "if something has gone wrong, or if some form of manual LV\n"
    "sharing is being used.\n")

arg(removemissing_ARG, '\0', "removemissing", 0, 0, 0,
    "Removes all missing PVs from the VG, if there are no LVs allocated\n"
    "on them. This resumes normal operation of the VG (new LVs may again\n"
    "be created, changed and so on).\n"
    "If this is not possible because LVs are referencing the missing PVs,\n"
    "this option can be combined with --force to have the command remove\n"
    "any partial LVs. In this case, any LVs and dependent snapshots that\n"
    "were partly on the missing disks are removed completely, including\n"
    "those parts on disks that are still present.\n"
    "If LVs spanned several disks, including ones that are lost, salvaging\n"
    "some data first may be possible by activating LVs in partial mode.\n")

arg(rebuild_ARG, '\0', "rebuild", pv_VAL, ARG_GROUPABLE, 0,
    "Selects a PV to rebuild in a raid LV. Multiple PVs can be rebuilt by\n"
    "repeating this option.\n"
    "Use this option in place of --resync or --syncaction repair when the\n"
    "PVs with corrupted data are known, and their data should be reconstructed\n"
    "rather than reconstructing default (rotating) data.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n")

arg(repair_ARG, '\0', "repair", 0, 0, 0,
    "Replace failed PVs in a raid or mirror LV, or run a repair\n"
    "utility on a thin pool. See \\fBlvmraid\\fP(7) and \\fBlvmthin\\fP(7)\n"
    "for more information.\n")

arg(replace_ARG, '\0', "replace", pv_VAL, ARG_GROUPABLE, 0,
    "Replace a specific PV in a raid LV with another PV.\n"
    "The new PV to use can be optionally specified after the LV.\n"
    "Multiple PVs can be replaced by repeating this option.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n")

arg(reportformat_ARG, '\0', "reportformat", reportformat_VAL, 0, 0,
    "Overrides current output format for reports which is defined globally by\n"
    "the report/output_format setting in lvm.conf.\n"
    "\\fBbasic\\fP is the original format with columns and rows.\n"
    "If there is more than one report per command, each report is prefixed\n"
    "with the report name for identification. \\fBjson\\fP produces report\n"
    "output in JSON format. See \\fBlvmreport\\fP(7) for more information.\n")

arg(restorefile_ARG, '\0', "restorefile", string_VAL, 0, 0,
    "In conjunction with --uuid, this reads the file (produced by\n"
    "vgcfgbackup), extracts the location and size of the data on the PV,\n"
    "and ensures that the metadata produced by the program is consistent\n"
    "with the contents of the file, i.e. the physical extents will be in\n"
    "the same place and not be overwritten by new metadata. This provides\n"
    "a mechanism to upgrade the metadata format or to add/remove metadata\n"
    "areas. Use with care.\n")

arg(restoremissing_ARG, '\0', "restoremissing", 0, 0, 0,
    "Add a PV back into a VG after the PV was missing and then returned,\n"
    "e.g. due to a transient failure. The PV is not reinitialized.\n")

arg(resync_ARG, '\0', "resync", 0, 0, 0,
    "Initiates mirror synchronization. Synchronization generally happens\n"
    "automatically, but this option forces it to run.\n"
    "Also see --rebuild to synchronize a specific PV.\n"
    "During synchronization, data is read from the primary mirror device\n"
    "and copied to the others. This can take considerable time, during\n"
    "which the LV is without a complete redundant copy of the data.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n")

arg(rows_ARG, '\0', "rows", 0, 0, 0,
    "Output columns as rows.\n")

arg(segments_ARG, '\0', "segments", 0, 0, 0,
    "#pvs\n"
    "Produces one line of output for each contiguous allocation of space on each\n"
    "PV, showing the start (pvseg_start) and length (pvseg_size) in units of\n"
    "physical extents.\n"
    "#lvs\n"
    "Use default columns that emphasize segment information.\n")

arg(separator_ARG, '\0', "separator", string_VAL, 0, 0,
    "String to use to separate each column. Useful if grepping the output.\n")

arg(shared_ARG, '\0', "shared", 0, 0, 0,
    "#vgcreate\n"
    "Create a shared VG using lvmlockd if LVM is compiled with lockd support.\n"
    "lvmlockd will select lock type sanlock or dlm depending on which lock\n"
    "manager is running. This allows multiple hosts to share a VG on shared\n"
    "devices. lvmlockd and a lock manager must be configured and running.\n"
    "See \\fBlvmlockd\\fP(8) for more information about shared VGs.\n"
    "#vgs\n"
    "#lvs\n"
    "#pvs\n"
    "#fullreport\n"
    "#vgdisplay\n"
    "#lvdisplay\n"
    "#pvdisplay\n"
    "Report/display shared VGs that would otherwise be skipped when\n"
    "lvmlockd is not being used on the host.\n"
    "See \\fBlvmlockd\\fP(8) for more information about shared VGs.\n")

arg(sinceversion_ARG, '\0', "sinceversion", string_VAL, 0, 0,
    "Specify an LVM version in x.y.z format where x is the major version,\n"
    "the y is the minor version and z is the patchlevel (e.g. 2.2.106).\n"
    "This option is currently applicable only with --typeconfig new\n"
    "to display all configuration settings introduced since given version.\n")

arg(splitcache_ARG, '\0', "splitcache", 0, 0, 0,
    "Separates a cache pool from a cache LV, and keeps the unused cache pool LV.\n"
    "Before the separation, the cache is flushed. Also see --uncache.\n")

arg(splitmirrors_ARG, '\0', "splitmirrors", number_VAL, 0, 0,
    "Splits the specified number of images from a raid1 or mirror LV\n"
    "and uses them to create a new LV. If --trackchanges is also specified,\n"
    "changes to the raid1 LV are tracked while the split LV remains detached.\n"
    "If --name is specified, then the images are permanently split from the\n"
    "original LV and changes are not tracked.\n")

arg(splitsnapshot_ARG, '\0', "splitsnapshot", 0, 0, 0,
    "Separates a COW snapshot from its origin LV. The LV that is split off\n"
    "contains the chunks that differ from the origin LV along with metadata\n"
    "describing them. This LV can be wiped and then destroyed with lvremove.\n")

arg(showdeprecated_ARG, '\0', "showdeprecated", 0, 0, 0,
    "Include deprecated configuration settings in the output. These settings\n"
    "are deprecated after a certain version. If a concrete version is specified\n"
    "with --atversion, deprecated settings are automatically included\n"
    "if the specified version is lower than the version in which the settings were\n"
    "deprecated. The current and diff types include deprecated settings\n"
    "in their output by default, all the other types ignore deprecated settings.\n")

arg(showunsupported_ARG, '\0', "showunsupported", 0, 0, 0,
    "Include unsupported configuration settings in the output. These settings\n"
    "are either used for debugging or development purposes only, or their support\n"
    "is not yet complete and they are not meant to be used in production. The\n"
    "current and diff types include unsupported settings in their\n"
    "output by default, all the other types ignore unsupported settings.\n")

arg(startpoll_ARG, '\0', "startpoll", 0, 0, 0,
    "Start polling an LV to continue processing a conversion.\n")

arg(stripes_long_ARG, '\0', "stripes", number_VAL, 0, 0,
    "Specifies the number of stripes in a striped LV. This is the number of\n"
    "PVs (devices) that a striped LV is spread across. Data that\n"
    "appears sequential in the LV is spread across multiple devices in units of\n"
    "the stripe size (see --stripesize). This does not apply to\n"
    "existing allocated space, only newly allocated space can be striped.\n")

arg(swapmetadata_ARG, '\0', "swapmetadata", 0, 0, 0,
    "Extracts the metadata LV from a pool and replaces it with another specified LV.\n"
    "The extracted LV is preserved and given the name of the LV that replaced it.\n"
    "Use for repair only. When the metadata LV is swapped out of the pool, it can\n"
    "be activated directly and used with thin provisioning tools:\n"
    "\\fBcache_dump\\fP(8), \\fBcache_repair\\fP(8), \\fBcache_restore\\fP(8),\n"
    "\\fBthin_dump\\fP(8), \\fBthin_repair\\fP(8), \\fBthin_restore\\fP(8).\n")

arg(syncaction_ARG, '\0', "syncaction", syncaction_VAL, 0, 0,
    "Initiate different types of RAID synchronization.\n"
    "This causes the RAID LV to read all data and parity\n"
    "blocks in the array and check for discrepancies\n"
    "(mismatches between mirrors or incorrect parity values).\n"
    "\\fBcheck\\fP will count but not correct discrepancies.\n"
    "\\fBrepair\\fP will correct discrepancies.\n"
    "See lvs for reporting discrepancies found or repaired.\n")
    
arg(sysinit_ARG, '\0', "sysinit", 0, 0, 0,
    "Indicates that vgchange/lvchange is being invoked from early system initialisation\n"
    "scripts (e.g. rc.sysinit or an initrd), before writable filesystems are\n"
    "available. As such, some functionality needs to be disabled and this option\n"
    "acts as a shortcut which selects an appropriate set of options. Currently,\n"
    "this is equivalent to using --ignorelockingfailure, --ignoremonitoring,\n"
    "--poll n, and setting env var LVM_SUPPRESS_LOCKING_FAILURE_MESSAGES.\n"
    "vgchange/lvchange skip autoactivation, and defer to pvscan autoactivation.\n")

arg(systemid_ARG, '\0', "systemid", string_VAL, 0, 0,
    "#vgcreate\n"
    "Specifies the system ID that will be given to the new VG, overriding the\n"
    "system ID of the host running the command. A VG is normally created\n"
    "without this option, in which case the new VG is given the system ID of\n"
    "the host creating it. Using this option requires caution because the\n"
    "system ID of the new VG may not match the system ID of the host running\n"
    "the command, leaving the VG inaccessible to the host.\n"
    "See \\fBlvmsystemid\\fP(7) for more information.\n"
    "#vgchange\n"
    "Changes the system ID of the VG.  Using this option requires caution\n"
    "because the VG may become foreign to the host running the command,\n"
    "leaving the host unable to access it.\n"
    "See \\fBlvmsystemid\\fP(7) for more information.\n")

arg(thinpool_ARG, '\0', "thinpool", lv_VAL, 0, 0,
    "The name of a thin pool LV.\n")

arg(trackchanges_ARG, '\0', "trackchanges", 0, 0, 0,
    "Can be used with --splitmirrors on a raid1 LV. This causes\n"
    "changes to the original raid1 LV to be tracked while the split images\n"
    "remain detached. This is a temporary state that allows the read-only\n"
    "detached image to be merged efficiently back into the raid1 LV later.\n"
    "Only the regions with changed data are resynchronized during merge.\n"
    "While a raid1 LV is tracking changes, operations on it are limited to\n"
    "merging the split image (see --mergemirrors) or permanently splitting\n"
    "the image (see --splitmirrors with --name.\n")

/* TODO: hide this? */
arg(trustcache_ARG, '\0', "trustcache", 0, 0, 0,
    "Avoids certain device scanning during command processing. Do not use.\n")

arg(type_ARG, '\0', "type", segtype_VAL, 0, 0,
    "The LV type, also known as \"segment type\" or \"segtype\".\n"
    "See usage descriptions for the specific ways to use these types.\n"
    "For more information about redundancy and performance (\\fBraid\\fP<N>, \\fBmirror\\fP, \\fBstriped\\fP, \\fBlinear\\fP) see \\fBlvmraid\\fP(7).\n"
    "For thin provisioning (\\fBthin\\fP, \\fBthin-pool\\fP) see \\fBlvmthin\\fP(7).\n"
    "For performance caching (\\fBcache\\fP, \\fBcache-pool\\fP) see \\fBlvmcache\\fP(7).\n"
    "For copy-on-write snapshots (\\fBsnapshot\\fP) see usage definitions.\n"
    "Several commands omit an explicit type option because the type\n"
    "is inferred from other options or shortcuts\n"
    "(e.g. --stripes, --mirrors, --snapshot, --virtualsize, --thin, --cache).\n"
    "Use inferred types with care because it can lead to unexpected results.\n")

arg(unbuffered_ARG, '\0', "unbuffered", 0, 0, 0,
    "Produce output immediately without sorting or aligning the columns properly.\n")

arg(uncache_ARG, '\0', "uncache", 0, 0, 0,
    "Separates a cache pool from a cache LV, and deletes the unused cache pool LV.\n"
    "Before the separation, the cache is flushed. Also see --splitcache.\n")

arg(cachepolicy_ARG, '\0', "cachepolicy", string_VAL, 0, 0,
    "Specifies the cache policy for a cache LV.\n"
    "See \\fBlvmcache\\fP(7) for more information.\n")

arg(cachesettings_ARG, '\0', "cachesettings", string_VAL, ARG_GROUPABLE, 0,
    "Specifies tunable values for a cache LV in \"Key = Value\" form.\n"
    "Repeat this option to specify multiple values.\n"
    "(The default values should usually be adequate.)\n"
    "The special string value \\fBdefault\\fP switches\n"
    "settings back to their default kernel values and removes\n"
    "them from the list of settings stored in LVM metadata.\n"
    "See \\fBlvmcache\\fP(7) for more information.\n")

arg(unconfigured_ARG, '\0', "unconfigured", 0, 0, 0,
    "Internal option used for generating config file during build.\n")

arg(units_ARG, '\0', "units", units_VAL, 0, 0,
    "All sizes are output in these units:\n"
    "human-(r)eadable with '<' rounding indicator,\n"
    "(h)uman-readable, (b)ytes, (s)ectors, (k)ilobytes, (m)egabytes,\n"
    "(g)igabytes, (t)erabytes, (p)etabytes, (e)xabytes.\n"
    "Capitalise to use multiples of 1000 (S.I.) instead of 1024.\n"
    "Custom units can be specified, e.g. --units 3M.\n")

arg(unquoted_ARG, '\0', "unquoted", 0, 0, 0,
    "When used with --nameprefixes, output values in the field=value\n"
    "pairs are not quoted.\n")

arg(usepolicies_ARG, '\0', "usepolicies", 0, 0, 0,
    "Perform an operation according to the policy configured in lvm.conf\n"
    "or a profile.\n")

arg(validate_ARG, '\0', "validate", 0, 0, 0,
    "Validate current configuration used and exit with appropriate\n"
    "return code. The validation is done only for the configuration\n"
    "at the front of the \"config cascade\". To validate the whole\n"
    "merged configuration tree, also use --mergedconfig.\n"
    "The validation is done even if lvm.conf config/checks is disabled.\n")

arg(vdo_ARG, '\0', "vdo", 0, 0, 0,
    "Specifies the command is handling VDO LV.\n"
    "See --type vdo.\n"
    "See \\fBlvmvdo\\fP(7) for more information about VDO usage.\n")

arg(vdopool_ARG, '\0', "vdopool", lv_VAL, 0, 0,
    "The name of a VDO pool LV.\n"
    "See \\fBlvmvdo\\fP(7) for more information about VDO usage.\n")

arg(version_ARG, '\0', "version", 0, 0, 0,
    "Display version information.\n")

arg(vgmetadatacopies_ARG, '\0', "vgmetadatacopies", vgmetadatacopies_VAL, 0, 0,
    "Number of copies of the VG metadata that are kept.\n"
    "VG metadata is kept in VG metadata areas on PVs in the VG,\n"
    "i.e. reserved space at the start and/or end of the PVs.\n"
    "Keeping a copy of the VG metadata on every PV can reduce performance\n"
    "in VGs containing a large number of PVs.\n"
    "When this number is set to a non-zero value, LVM will automatically\n"
    "choose PVs on which to store metadata, using the metadataignore flags\n"
    "on PVs to achieve the specified number.\n"
    "The number can also be replaced with special string values:\n"
    "\\fBunmanaged\\fP causes LVM to not automatically manage the PV\n"
    "metadataignore flags.\n"
    "\\fBall\\fP causes LVM to first clear the metadataignore flags on\n"
    "all PVs, and then to become unmanaged.\n")

arg(withsummary_ARG, '\0', "withsummary", 0, 0, 0,
    "Display a one line comment for each configuration node.\n")

arg(withcomments_ARG, '\0', "withcomments", 0, 0, 0,
    "Display a full comment for each configuration node. For deprecated\n"
    "settings, also display comments about deprecation.\n")

arg(withgeneralpreamble_ARG, '\0', "withgeneralpreamble", 0, 0, 0,
    "Include general config file preamble.\n")

arg(withlocalpreamble_ARG, '\0', "withlocalpreamble", 0, 0, 0,
    "Include local config file preamble.\n")

arg(withspaces_ARG, '\0', "withspaces", 0, 0, 0,
    "Where appropriate, add more spaces in output for better readability.\n")

arg(withversions_ARG, '\0', "withversions", 0, 0, 0,
    "Also display a comment containing the version of introduction for\n"
    "each configuration node. If the setting is deprecated, also display\n"
    "the version since which it is deprecated.\n")

arg(writebehind_ARG, '\0', "writebehind", number_VAL, 0, 0,
    "The maximum number of outstanding writes that are allowed to\n"
    "devices in a RAID1 LV that is marked write-mostly.\n"
    "Once this value is exceeded, writes become synchronous (i.e. all writes\n"
    "to the constituent devices must complete before the array signals the\n"
    "write has completed). Setting the value to zero clears the preference\n"
    "and allows the system to choose the value arbitrarily.\n")

arg(writemostly_ARG, '\0', "writemostly", writemostly_VAL, ARG_GROUPABLE, 0,
    "Mark a device in a RAID1 LV as write-mostly.  All reads\n"
    "to these drives will be avoided unless absolutely necessary. This keeps\n"
    "the number of I/Os to the drive to a minimum. The default behavior is to\n"
    "set the write-mostly attribute for the specified PV.\n"
    "It is also possible to remove the write-mostly flag by adding the\n"
    "suffix \\fB:n\\fP at the end of the PV name, or to toggle the value with\n"
    "the suffix \\fB:t\\fP. Repeat this option to change the attribute on\n"
    "multiple PVs.\n")

/*
 * Synonyms of other options.
 *
 * Only the standard option names are used in command definitions.
 *
 * If used on the command line, lvm automatically translates them
 * to the standard option name.
 *
 * The generated help and man output does not include
 * these variants.  The description of the standard option names
 * can mention a synonym, or in some cases the man page generation
 * recognizes some of these and prints the option name to include
 * the variant, e.g. man page generation prints --[raid]writebehind.
 */
arg(corelog_ARG, '\0', "corelog", 0, 0, 0, NULL)
arg(resizable_ARG, '\0', "resizable", bool_VAL, 0, 0, NULL)
arg(allocation_ARG, '\0', "allocation", bool_VAL, 0, 0, NULL)
arg(available_ARG, '\0', "available", activation_VAL, 0, 0, NULL)
arg(raidrebuild_ARG, '\0', "raidrebuild", pv_VAL, ARG_GROUPABLE, 0, NULL)
arg(raidsyncaction_ARG, '\0', "raidsyncaction", syncaction_VAL, 0, 0, NULL)
arg(raidwritemostly_ARG, '\0', "raidwritemostly", writemostly_VAL, ARG_GROUPABLE, 0, NULL)
arg(raidminrecoveryrate_ARG, '\0', "raidminrecoveryrate", sizekb_VAL, 0, 0, NULL)
arg(raidmaxrecoveryrate_ARG, '\0', "raidmaxrecoveryrate", sizekb_VAL, 0, 0, NULL)
arg(raidwritebehind_ARG, '\0', "raidwritebehind", number_VAL, 0, 0, NULL)
arg(virtualoriginsize_ARG, '\0', "virtualoriginsize", sizemb_VAL, 0, 0, NULL)
arg(split_ARG, '\0', "split", 0, 0, 0, NULL)
arg(metadatacopies_ARG, '\0', "metadatacopies", metadatacopies_VAL, 0, 0, NULL)


/*
 * ... and now the short args.
 */
arg(activate_ARG, 'a', "activate", activation_VAL, 0, 0,
    "#pvscan\n"
    "Auto-activate LVs in a VG when the PVs scanned have completed the VG.\n"
    "(Only \\fBay\\fP is applicable.)\n"
    "#lvchange\n"
    "#vgchange\n"
    "Change the active state of LVs.\n"
    "An active LV can be used through a block device,\n"
    "allowing data on the LV to be accessed.\n"
    "\\fBy\\fP makes LVs active, or available.\n"
    "\\fBn\\fP makes LVs inactive, or unavailable.\n"
    "The block device for the LV is added or removed from the system\n"
    "using device-mapper in the kernel.\n"
    "A symbolic link /dev/VGName/LVName pointing to the device node is also added/removed.\n"
    "All software and scripts should access the device through the symbolic\n"
    "link and present this as the name of the device.\n"
    "The location and name of the underlying device node may depend on\n"
    "the distribution, configuration (e.g. udev), or release version.\n"
    "\\fBay\\fP specifies autoactivation, in which case an LV is activated\n"
    "only if it matches an item in lvm.conf activation/auto_activation_volume_list.\n"
    "If the list is not set, all LVs are considered to match, and if\n"
    "if the list is set but empty, no LVs match.\n"
    "Autoactivation should be used during system boot to make it possible\n"
    "to select which LVs should be automatically activated by the system.\n"
    "See \\fBlvmlockd\\fP(8) for more information about activation options \\fBey\\fP and \\fBsy\\fP for shared VGs.\n"
    "#lvcreate\n"
    "Controls the active state of the new LV.\n"
    "\\fBy\\fP makes the LV active, or available.\n"
    "New LVs are made active by default.\n"
    "\\fBn\\fP makes the LV inactive, or unavailable, only when possible.\n"
    "In some cases, creating an LV requires it to be active.\n"
    "For example, COW snapshots of an active origin LV can only\n"
    "be created in the active state (this does not apply to thin snapshots).\n"
    "The --zero option normally requires the LV to be active.\n"
    "If autoactivation \\fBay\\fP is used, the LV is only activated\n"
    "if it matches an item in lvm.conf activation/auto_activation_volume_list.\n"
    "\\fBay\\fP implies --zero n and --wipesignatures n.\n"
    "See \\fBlvmlockd\\fP(8) for more information about activation options for shared VGs.\n")

arg(all_ARG, 'a', "all", 0, 0, 0,
    "#vgreduce\n"
    "Removes all empty PVs if none are named on the command line.\n"
    "#pvchange\n"
    "Change all visible PVs.\n"
    "#vgimport\n"
    "Import all visible VGs.\n"
    "#lvscan\n"
    "#lvdisplay\n"
    "#lvs\n"
    "Show information about internal LVs.\n"
    "These are components of normal LVs, such as mirrors,\n"
    "which are not independently accessible, e.g. not mountable.\n"
    "#vgs\n"
    "List all VGs. Equivalent to not specifying any VGs.\n"
    "#pvs\n"
    "#pvdisplay\n"
    "Show information about devices that have not been initialized\n"
    "by LVM, i.e. they are not PVs.\n")

arg(autobackup_ARG, 'A', "autobackup", bool_VAL, 0, 0,
    "Specifies if metadata should be backed up automatically after a change.\n"
    "Enabling this is strongly advised! See \\fBvgcfgbackup\\fP(8) for more information.\n")

arg(activevolumegroups_ARG, 'A', "activevolumegroups", 0, 0, 0,
    "Only select active VGs. The VG is considered active\n"
    "if at least one of its LVs is active.\n")

arg(background_ARG, 'b', "background", 0, 0, 0,
    "If the operation requires polling, this option causes the command to\n"
    "return before the operation is complete, and polling is done in the\n"
    "background.\n")

arg(basevgname_ARG, 'n', "basevgname", string_VAL, 0, 0,
    "By default the snapshot VG will be renamed to the original name plus a\n"
    "numeric suffix to avoid duplicate naming (e.g. 'test_vg' would be renamed\n"
    "to 'test_vg1'). This option will override the base VG name that is\n"
    "used for all VG renames. If a VG already exists with the specified name\n"
    "a numeric suffix will be added (like the previous example) to make it unique.\n")

arg(blockdevice_ARG, 'b', "blockdevice", 0, 0, 0,
    "No longer used.\n")

arg(chunksize_ARG, 'c', "chunksize", sizekb_VAL, 0, 0,
    "The size of chunks in a snapshot, cache pool or thin pool.\n"
    "For snapshots, the value must be a power of 2 between 4KiB and 512KiB\n"
    "and the default value is 4.\n"
    "For a cache pool the value must be between 32KiB and 1GiB\n"
    "and the default value is 64.\n"
    "For a thin pool the value must be between 64KiB and 1GiB\n"
    "and the default value starts with 64 and scales up to fit the\n"
    "pool metadata size within 128MiB, if the pool metadata size is not specified.\n"
    "The value must be a multiple of 64KiB.\n"
    "See \\fBlvmthin\\fP(7) and \\fBlvmcache\\fP(7) for more information.\n")

arg(clustered_ARG, 'c', "clustered", bool_VAL, 0, 0,
    "This option was specific to clvm and is now replaced by\n"
    "the --shared option with \\fBlvmlockd\\fP(8).\n")

arg(colon_ARG, 'c', "colon", 0, 0, 0,
    "Generate colon separated output for easier parsing in scripts or programs.\n"
    "Also see \\fBvgs\\fP(8) which provides considerably more control over the output.\n")

arg(columns_ARG, 'C', "columns", 0, 0, 0,
    "Display output in columns, the equivalent of \\fBvgs\\fP(8).\n"
    "Options listed are the same as options given in \\fBvgs\\fP(8).\n")

arg(contiguous_ARG, 'C', "contiguous", bool_VAL, 0, 0,
    "Sets or resets the contiguous allocation policy for LVs.\n"
    "Default is no contiguous allocation based on a next free principle.\n"
    "It is only possible to change a non-contiguous allocation policy\n"
    "to contiguous if all of the allocated physical extents in the LV\n"
    "are already contiguous.\n")

arg(debug_ARG, 'd', "debug", 0, ARG_COUNTABLE, 0,
    "Set debug level. Repeat from 1 to 6 times to increase the detail of\n"
    "messages sent to the log file and/or syslog (if configured).\n")

arg(exported_ARG, 'e', "exported", 0, 0, 0,
    "Only show PVs belonging to exported VGs.\n")

/* Not used. */
arg(physicalextent_ARG, 'E', "physicalextent", 0, 0, 0, NULL)

arg(file_ARG, 'f', "file", string_VAL, 0, 0,
    "#lvmconfig\n"
    "#dumpconfig\n"
    "#config\n"
    "Write output to the named file.\n"
    "#vgcfgbackup\n"
    "Write the backup to the named file.\n"
    "When backing up more than one VG, the file name is\n"
    "treated as a template, and %s is replaced by the VG name.\n"
    "#vgcfgrestore\n"
    "Read metadata backup from the named file.\n" 
    "Usually this file was created by vgcfgbackup.\n")

arg(force_ARG, 'f', "force", 0, ARG_COUNTABLE, 0,
    "Override various checks, confirmations and protections.\n"
    "Use with extreme caution.\n")

/* Not used. */
arg(full_ARG, 'f', "full", 0, 0, 0, NULL)

arg(help_ARG, 'h', "help", 0, 0, 0,
    "Display help text.\n")

arg(cache_ARG, 'H', "cache", 0, 0, 0,
    "Specifies the command is handling a cache LV or cache pool.\n"
    "See --type cache and --type cache-pool.\n"
    "See \\fBlvmcache\\fP(7) for more information about LVM caching.\n")

arg(history_ARG, 'H', "history", 0, 0, 0,
    "Include historical LVs in the output.\n"
    "(This has no effect unless LVs were removed while\n"
    "lvm.conf metadata/record_lvs_history was enabled.\n")

/* Not used */
arg(help2_ARG, '?', "", 0, 0, 0, NULL)

arg(import_ARG, 'i', "import", 0, 0, 0,
    "Import exported VGs. Otherwise VGs that have been exported\n"
    "will not be changed (nor will their associated PVs).\n")

arg(interval_ARG, 'i', "interval", number_VAL, 0, 0,
    "Report progress at regular intervals.\n")

/* Not used */
arg(iop_version_ARG, 'i', "iop_version", 0, 0, 0, NULL)

arg(stripes_ARG, 'i', "stripes", number_VAL, 0, 0,
    "Specifies the number of stripes in a striped LV. This is the number of\n"
    "PVs (devices) that a striped LV is spread across. Data that\n"
    "appears sequential in the LV is spread across multiple devices in units of\n"
    "the stripe size (see --stripesize). This does not change existing\n"
    "allocated space, but only applies to space being allocated by the command.\n"
    "When creating a RAID 4/5/6 LV, this number does not include the extra\n"
    "devices that are required for parity. The largest number depends on\n"
    "the RAID type (raid0: 64, raid10: 32, raid4/5: 63, raid6: 62), and\n"
    "when unspecified, the default depends on the RAID type\n"
    "(raid0: 2, raid10: 2, raid4/5: 3, raid6: 5.)\n"
    "To stripe a new raid LV across all PVs by default,\n"
    "see lvm.conf allocation/raid_stripe_all_devices.\n")

arg(stripesize_ARG, 'I', "stripesize", sizekb_VAL, 0, 0,
    "The amount of data that is written to one device before\n"
    "moving to the next in a striped LV.\n")

arg(logicalvolume_ARG, 'l', "logicalvolume", uint32_VAL, 0, 0,
    "Sets the maximum number of LVs allowed in a VG.\n")

arg(maxlogicalvolumes_ARG, 'l', "maxlogicalvolumes", uint32_VAL, 0, 0,
    "Sets the maximum number of LVs allowed in a VG.\n")

/*
 * The extents_VAL is overriden in configure_command_option_values()
 * according to the command being run.  Different commands accept
 * different signs with the value.
 */
arg(extents_ARG, 'l', "extents", extents_VAL, 0, 0,
    "#lvcreate\n"
    "Specifies the size of the new LV in logical extents.\n"
    "The --size and --extents options are alternate methods of specifying size.\n"
    "The total number of physical extents used will be\n"
    "greater when redundant data is needed for RAID levels.\n"
    "An alternate syntax allows the size to be determined indirectly\n"
    "as a percentage of the size of a related VG, LV, or set of PVs. The\n"
    "suffix \\fB%VG\\fP denotes the total size of the VG, the suffix \\fB%FREE\\fP\n"
    "the remaining free space in the VG, and the suffix \\fB%PVS\\fP the free\n"
    "space in the specified PVs.  For a snapshot, the size\n"
    "can be expressed as a percentage of the total size of the origin LV\n"
    "with the suffix \\fB%ORIGIN\\fP (\\fB100%ORIGIN\\fP provides space for\n"
    "the whole origin).\n"
    "When expressed as a percentage, the size defines an upper limit for the\n"
    "number of logical extents in the new LV. The precise number of logical\n"
    "extents in the new LV is not determined until the command has completed.\n"
    "#lvreduce\n"
    "#lvextend\n"
    "#lvresize\n"
    "Specifies the new size of the LV in logical extents.\n"
    "The --size and --extents options are alternate methods of specifying size.\n"
    "The total number of physical extents used will be\n"
    "greater when redundant data is needed for RAID levels.\n"
    "An alternate syntax allows the size to be determined indirectly\n"
    "as a percentage of the size of a related VG, LV, or set of PVs. The\n"
    "suffix \\fB%VG\\fP denotes the total size of the VG, the suffix \\fB%FREE\\fP\n"
    "the remaining free space in the VG, and the suffix \\fB%PVS\\fP the free\n"
    "space in the specified PVs.  For a snapshot, the size\n"
    "can be expressed as a percentage of the total size of the origin LV\n"
    "with the suffix \\fB%ORIGIN\\fP (\\fB100%ORIGIN\\fP provides space for\n"
    "the whole origin).\n"
    "When expressed as a percentage, the size defines an upper limit for the\n"
    "number of logical extents in the new LV. The precise number of logical\n"
    "extents in the new LV is not determined until the command has completed.\n"
    "When the plus \\fB+\\fP or minus \\fB-\\fP prefix is used,\n"
    "the value is not an absolute size, but is relative and added or subtracted\n"
    "from the current size.\n")

arg(list_ARG, 'l', "list", 0, 0, 0,
    "#lvmconfig\n"
    "#dumpconfig\n"
    "#config\n"
    "List config settings with summarizing comment. This is the same as using\n"
    "options --typeconfig list --withsummary.\n"
    "#vgcfgrestore\n"
    "List metadata backup and archive files pertaining to the VG.\n"
    "May be used with --file. Does not restore the VG.\n"
    "#vgmerge\n"
    "Display merged destination VG like vgdisplay -v.\n")

arg(lvmpartition_ARG, 'l', "lvmpartition", 0, 0, 0,
    "Only report PVs.\n")

/*
 * The sizemb_VAL is overriden in configure_command_option_values()
 * according to the command being run.  Different commands accept
 * different signs with the value.
 */
arg(size_ARG, 'L', "size", sizemb_VAL, 0, 0,
    "#lvcreate\n"
    "Specifies the size of the new LV.\n"
    "The --size and --extents options are alternate methods of specifying size.\n"
    "The total number of physical extents used will be\n"
    "greater when redundant data is needed for RAID levels.\n"
    "#lvreduce\n"
    "#lvextend\n"
    "#lvresize\n"
    "Specifies the new size of the LV.\n"
    "The --size and --extents options are alternate methods of specifying size.\n"
    "The total number of physical extents used will be\n"
    "greater when redundant data is needed for RAID levels.\n"
    "When the plus \\fB+\\fP or minus \\fB-\\fP prefix is used,\n"
    "the value is not an absolute size, but is relative and added or subtracted\n"
    "from the current size.\n")

arg(persistent_ARG, 'M', "persistent", bool_VAL, 0, 0,
    "When yes, makes the specified minor number persistent.\n")

arg(major_ARG, 'j', "major", number_VAL, ARG_GROUPABLE, 0,
   "#lvcreate\n"
   "#lvchange\n"
   "Sets the major number of an LV block device.\n"
   "#pvscan\n"
   "The major number of a device.\n")

arg(setactivationskip_ARG, 'k', "setactivationskip", bool_VAL, 0, 0,
    "Persistently sets (yes) or clears (no) the \"activation skip\" flag on an LV.\n"
    "An LV with this flag set is not activated unless the\n"
    "--ignoreactivationskip option is used by the activation command.\n"
    "This flag is set by default on new thin snapshot LVs.\n"
    "The flag is not applied to deactivation.\n"
    "The current value of the flag is indicated in the lvs lv_attr bits.\n")

arg(ignoreactivationskip_ARG, 'K', "ignoreactivationskip", 0, 0, 0,
    "Ignore the \"activation skip\" LV flag during activation\n"
    "to allow LVs with the flag set to be activated.\n")

arg(maps_ARG, 'm', "maps", 0, 0, 0,
    "#lvdisplay\n"
    "Display the mapping of logical extents to PVs and physical extents.\n"
    "To map physical extents to logical extents use:\n"
    "pvs --segments -o+lv_name,seg_start_pe,segtype\n"
    "#pvdisplay\n"
    "Display the mapping of physical extents to LVs and logical extents.\n")

/* FIXME: should the unused mirrors option be removed from lvextend? */

arg(mirrors_ARG, 'm', "mirrors", number_VAL, 0, 0,
    "#lvcreate\n"
    "Specifies the number of mirror images in addition to the original LV\n"
    "image, e.g. --mirrors 1 means there are two images of the data, the\n"
    "original and one mirror image.\n"
    "Optional positional PV args on the command line can specify the devices\n"
    "the images should be placed on.\n"
    "There are two mirroring implementations: \"raid1\" and \"mirror\".\n"
    "These are the names of the corresponding LV types, or \"segment types\".\n"
    "Use the --type option to specify which to use (raid1 is default,\n"
    "and mirror is legacy)\n"
    "Use lvm.conf global/mirror_segtype_default and\n"
    "global/raid10_segtype_default to configure the default types.\n"
    "See the --nosync option for avoiding initial image synchronization.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n"
    "#lvconvert\n"
    "Specifies the number of mirror images in addition to the original LV\n"
    "image, e.g. --mirrors 1 means there are two images of the data, the\n"
    "original and one mirror image.\n"
    "Optional positional PV args on the command line can specify the devices\n"
    "the images should be placed on.\n"
    "There are two mirroring implementations: \"raid1\" and \"mirror\".\n"
    "These are the names of the corresponding LV types, or \"segment types\".\n"
    "Use the --type option to specify which to use (raid1 is default,\n"
    "and mirror is legacy)\n"
    "Use lvm.conf global/mirror_segtype_default and\n"
    "global/raid10_segtype_default to configure the default types.\n"
    "The plus prefix \\fB+\\fP can be used, in which case\n"
    "the number is added to the current number of images,\n"
    "or the minus prefix \\fB-\\fP can be used, in which case\n"
    "the number is subtracted from the current number of images.\n"
    "See \\fBlvmraid\\fP(7) for more information.\n"
    "#lvextend\n"
    "Not used.\n")

arg(metadatatype_ARG, 'M', "metadatatype", metadatatype_VAL, 0, 0,
    "Specifies the type of on-disk metadata to use.\n"
    "\\fBlvm2\\fP (or just \\fB2\\fP) is the current, standard format.\n"
    "\\fBlvm1\\fP (or just \\fB1\\fP) is no longer used.\n")

arg(name_ARG, 'n', "name", string_VAL, 0, 0,
    "#lvcreate\n"
    "#lvconvert\n"
    "Specifies the name of a new LV.\n"
    "When unspecified, a default name of \"lvol#\" is\n"
    "generated, where # is a number generated by LVM.\n"
    "#pvmove\n"
    "Move only the extents belonging to the named LV.\n"
    "#vgsplit\n"
    "Move only PVs used by the named LV.\n")

arg(nofsck_ARG, 'n', "nofsck", 0, 0, 0,
    "Do not perform fsck before resizing filesystem when filesystem\n"
    "requires it. You may need to use --force to proceed with\n"
    "this option.\n")

arg(novolumegroup_ARG, 'n', "novolumegroup", 0, 0, 0,
    "Only show PVs not belonging to any VG.\n")

/* Not used */
arg(oldpath_ARG, 'n', "oldpath", 0, 0, 0, NULL)

/*
 * FIXME: a free-form discussion section and document the
 * VG/LV/PV attr bits which were previously listed
 * in the description for -o.
 */

arg(options_ARG, 'o', "options", string_VAL, ARG_GROUPABLE, 0,
    "Comma-separated, ordered list of fields to display in columns.\n"
    "String arg syntax is: [+|-|#]Field1[,Field2 ...]\n"
    "The prefix \\fB+\\fP will append the specified fields to the default fields,\n"
    "\\fB-\\fP will remove the specified fields from the default fields, and\n"
    "\\fB#\\fP will compact specified fields (removing them when empty for all rows.)\n"
    "Use \\fB-o help\\fP to view the list of all available fields.\n"
    "Use separate lists of fields to add, remove or compact by repeating the -o option:\n"
    "-o+field1,field2 -o-field3,field4 -o#field5.\n"
    "These lists are evaluated from left to right.\n"
    "Use field name \\fBlv_all\\fP to view all LV fields,\n"
    "\\fBvg_all\\fP all VG fields,\n"
    "\\fBpv_all\\fP all PV fields,\n"
    "\\fBpvseg_all\\fP all PV segment fields,\n"
    "\\fBseg_all\\fP all LV segment fields, and\n"
    "\\fBpvseg_all\\fP all PV segment columns.\n"
    "See the lvm.conf report section for more config options.\n"
    "See \\fBlvmreport\\fP(7) for more information about reporting.\n")

arg(sort_ARG, 'O', "sort", string_VAL, ARG_GROUPABLE, 0,
    "Comma-separated ordered list of columns to sort by. Replaces the default\n"
    "selection. Precede any column with \\fB-\\fP for a reverse sort on that column.\n")

arg(maxphysicalvolumes_ARG, 'p', "maxphysicalvolumes", uint32_VAL, 0, 0,
    "Sets the maximum number of PVs that can belong to the VG.\n"
    "The value 0 removes any limitation.\n"
    "For large numbers of PVs, also see options --pvmetadatacopies,\n"
    "and --vgmetadatacopies for improving performance.\n")

arg(permission_ARG, 'p', "permission", permission_VAL, 0, 0,
    "Set access permission to read only \\fBr\\fP or read and write \\fBrw\\fP.\n")

arg(partial_ARG, 'P', "partial", 0, 0, 0,
    "Commands will do their best to activate LVs with missing PV extents.\n"
    "Missing extents may be replaced with error or zero segments\n"
    "according to the lvm.conf missing_stripe_filler setting.\n"
    "Metadata may not be changed with this option.\n")

/* Not used */
arg(physicalvolume_ARG, 'P', "physicalvolume", 0, 0, 0, NULL)

arg(quiet_ARG, 'q', "quiet", 0, ARG_COUNTABLE, 0,
    "Suppress output and log messages. Overrides --debug and --verbose.\n"
    "Repeat once to also suppress any prompts with answer 'no'.\n")

arg(readahead_ARG, 'r', "readahead", readahead_VAL, 0, 0,
    "Sets read ahead sector count of an LV.\n"
    "\\fBauto\\fP is the default which allows the kernel to choose\n"
    "a suitable value automatically.\n"
    "\\fBnone\\fP is equivalent to zero.\n")

arg(resizefs_ARG, 'r', "resizefs", 0, 0, 0,
    "Resize underlying filesystem together with the LV using fsadm(8).\n")

/* Not used */
arg(reset_ARG, 'R', "reset", 0, 0, 0, NULL)

arg(regionsize_ARG, 'R', "regionsize", regionsizemb_VAL, 0, 0,
    "Size of each raid or mirror synchronization region.\n"
    "lvm.conf activation/raid_region_size can be used to\n"
    "configure a default.\n")

arg(physicalextentsize_ARG, 's', "physicalextentsize", sizemb_VAL, 0, 0,
    "#vgcreate\n"
    "Sets the physical extent size of PVs in the VG.\n"
    "The value must be either a power of 2 of at least 1 sector\n"
    "(where the sector size is the largest sector size of the PVs\n"
    "currently used in the VG), or at least 128KiB.\n"
    "Once this value has been set, it is difficult to change\n"
    "without recreating the VG, unless no extents need moving.\n"
    "#vgchange\n"
    "Sets the physical extent size of PVs in the VG.\n"
    "The value must be either a power of 2 of at least 1 sector\n"
    "(where the sector size is the largest sector size of the PVs\n"
    "currently used in the VG), or at least 128KiB.\n"
    "Once this value has been set, it is difficult to change\n"
    "without recreating the VG, unless no extents need moving.\n"
    "Before increasing the physical extent size, you might need to use lvresize,\n"
    "pvresize and/or pvmove so that everything fits. For example, every\n"
    "contiguous range of extents used in a LV must start and end on an extent boundary.\n")

arg(snapshot_ARG, 's', "snapshot", 0, 0, 0,
    "#lvcreate\n"
    "Create a snapshot. Snapshots provide a \"frozen image\" of an origin LV.\n"
    "The snapshot LV can be used, e.g. for backups, while the origin LV\n"
    "continues to be used.\n"
    "This option can create a COW (copy on write) snapshot,\n"
    "or a thin snapshot (in a thin pool.)\n"
    "Thin snapshots are created when the origin is a thin LV and\n"
    "the size option is NOT specified. Thin snapshots share the same blocks\n"
    "in the thin pool, and do not allocate new space from the VG.\n"
    "Thin snapshots are created with the \"activation skip\" flag,\n"
    "see --setactivationskip.\n"
    "A thin snapshot of a non-thin \"external origin\" LV is created\n"
    "when a thin pool is specified. Unprovisioned blocks in the thin snapshot\n"
    "LV are read from the external origin LV. The external origin LV must\n"
    "be read-only.\n"
    "See \\fBlvmthin\\fP(7) for more information about LVM thin provisioning.\n"
    "COW snapshots are created when a size is specified. The size is allocated\n"
    "from space in the VG, and is the amount of space that can be used\n"
    "for saving COW blocks as writes occur to the origin or snapshot.\n"
    "The size chosen should depend upon the amount of writes that are expected;\n"
    "often 20% of the origin LV is enough. If COW space runs low, it can\n"
    "be extended with lvextend (shrinking is also allowed with lvreduce.)\n"
    "A small amount of the COW snapshot LV size is used to track COW block\n"
    "locations, so the full size is not available for COW data blocks.\n"
    "Use lvs to check how much space is used, and see --monitor to\n"
    "to automatically extend the size to avoid running out of space.\n"
    "#lvconvert\n"
    "Combine a former COW snapshot LV with a former origin LV to reverse\n"
    "a previous --splitsnapshot command.\n")

arg(short_ARG, 's', "short", 0, 0, 0,
    "#pvdisplay\n"
    "Only display the size of the given PVs.\n"
    "#vgdisplay\n"
    "Give a short listing showing the existence of VGs.\n"
    "#pvscan\n"
    "Short listing format.\n")

/* Not used */
arg(stdin_ARG, 's', "stdin", 0, 0, 0, NULL)

arg(select_ARG, 'S', "select", string_VAL, ARG_GROUPABLE, 0,
    "Select objects for processing and reporting based on specified criteria.\n"
    "The criteria syntax is described by \\fB--select help\\fP and \\fBlvmreport\\fP(7).\n"
    "For reporting commands, one row is displayed for each object matching the criteria.\n"
    "See \\fB--options help\\fP for selectable object fields.\n"
    "Rows can be displayed with an additional \"selected\" field (-o selected)\n"
    "showing 1 if the row matches the selection and 0 otherwise.\n"
    "For non-reporting commands which process LVM entities, the selection is\n"
    "used to choose items to process.\n")

arg(test_ARG, 't', "test", 0, 0, 0,
    "Run in test mode. Commands will not update metadata.\n"
    "This is implemented by disabling all metadata writing but nevertheless\n"
    "returning success to the calling function. This may lead to unusual\n"
    "error messages in multi-stage operations if a tool relies on reading\n"
    "back metadata it believes has changed but hasn't.\n")

arg(thin_ARG, 'T', "thin", 0, 0, 0,
    "Specifies the command is handling a thin LV or thin pool.\n"
    "See --type thin, --type thin-pool, and --virtualsize.\n"
    "See \\fBlvmthin\\fP(7) for more information about LVM thin provisioning.\n")

arg(uuid_ARG, 'u', "uuid", 0, 0, 0,
    "#pvchange\n"
    "Generate new random UUID for specified PVs.\n"
    "#pvscan\n"
    "Show UUIDs in addition to device names.\n"
    "#vgchange\n"
    "Generate new random UUID for specified VGs.\n")

arg(uuidstr_ARG, 'u', "uuid", string_VAL, 0, 0,
    "Specify a UUID for the device.\n"
    "Without this option, a random UUID is generated.\n"
    "This option is needed before restoring a backup of LVM metadata\n"
    "onto a replacement device; see \\fBvgcfgrestore\\fP(8). As such, use of\n"
    "--restorefile is compulsory unless the --norestorefile is used.\n"
    "All PVs must have unique UUIDs, and LVM will prevent certain operations\n"
    "if multiple devices are seen with the same UUID.\n"
    "See \\fBvgimportclone\\fP(8) for more information.\n")

/* Not used */
arg(uuidlist_ARG, 'U', "uuidlist", 0, 0, 0, NULL)

arg(verbose_ARG, 'v', "verbose", 0, ARG_COUNTABLE, 0,
    "Set verbose level. Repeat from 1 to 4 times to increase the detail\n"
    "of messages sent to stdout and stderr.\n")

/* Not used */
arg(volumegroup_ARG, 'V', "volumegroup", 0, 0, 0, NULL)

arg(virtualsize_ARG, 'V', "virtualsize", sizemb_VAL, 0, 0,
    "The virtual size of a new thin LV.\n"
    "See \\fBlvmthin\\fP(7) for more information about LVM thin provisioning.\n"
    "Using virtual size (-V) and actual size (-L) together creates\n"
    "a sparse LV.\n"
    "lvm.conf global/sparse_segtype_default determines the\n"
    "default segment type used to create a sparse LV.\n"
    "Anything written to a sparse LV will be returned when reading from it.\n"
    "Reading from other areas of the LV will return blocks of zeros.\n"
    "When using a snapshot to create a sparse LV, a hidden virtual device\n"
    "is created using the zero target, and the LV has the suffix _vorigin.\n"
    "Snapshots are less efficient than thin provisioning when creating\n"
    "large sparse LVs (GiB).\n")

arg(wipesignatures_ARG, 'W', "wipesignatures", bool_VAL, 0, 0,
    "Controls detection and subsequent wiping of signatures on new LVs.\n"
    "There is a prompt for each signature detected to confirm its wiping\n"
    "(unless --yes is used to override confirmations.)\n"
    "When not specified, signatures are wiped whenever zeroing is done\n"
    "(see --zero). This behaviour can be configured with\n"
    "lvm.conf allocation/wipe_signatures_when_zeroing_new_lvs.\n"
    "If blkid wiping is used (lvm.conf allocation/use_blkid_wiping)\n"
    "and LVM is compiled with blkid wiping support, then the blkid(8)\n"
    "library is used to detect the signatures (use blkid -k to list the\n"
    "signatures that are recognized).\n"
    "Otherwise, native LVM code is used to detect signatures\n"
    "(only MD RAID, swap and LUKS signatures are detected in this case.)\n"
    "The LV is not wiped if the read only flag is set.\n")

arg(allocatable_ARG, 'x', "allocatable", bool_VAL, 0, 0,
    "Enable or disable allocation of physical extents on this PV.\n")

arg(resizeable_ARG, 'x', "resizeable", bool_VAL, 0, 0,
    "Enables or disables the addition or removal of PVs to/from a VG\n"
    "(by vgextend/vgreduce).\n")

arg(yes_ARG, 'y', "yes", 0, 0, 0,
    "Do not prompt for confirmation interactively but always assume the\n"
    "answer yes. Use with extreme caution.\n"
    "(For automatic no, see -qq.)\n")

arg(zero_ARG, 'Z', "zero", bool_VAL, 0, 0,
    "#lvchange\n"
    "Set zeroing mode for thin pool. Note: already provisioned blocks from pool\n"
    "in non-zero mode are not cleared in unwritten parts when setting --zero y.\n"
    "#lvconvert\n"
    "For snapshots, this controls zeroing of the first 4KiB of data in the\n"
    "snapshot. If the LV is read-only, the snapshot will not be zeroed.\n"
    "For thin pools, this controls zeroing of provisioned blocks.\n"
    "Provisioning of large zeroed chunks negatively impacts performance.\n"
    "#lvcreate\n"
    "Controls zeroing of the first 4KiB of data in the new LV.\n"
    "Default is \\fBy\\fP.\n"
    "Snapshot COW volumes are always zeroed.\n"
    "LV is not zeroed if the read only flag is set.\n"
    "Warning: trying to mount an unzeroed LV can cause the system to hang.\n"
    "#pvcreate\n"
    "#vgcreate\n"
    "#vgextend\n"
    "Controls if the first 4 sectors (2048 bytes) of the device are wiped.\n"
    "The default is to wipe these sectors unless either or both of\n"
    "--restorefile or --uuid are specified.\n")

/* this should always be last */
arg(ARG_COUNT, '-', "", 0, 0, 0, NULL)
/* *INDENT-ON* */
