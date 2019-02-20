
/*
 * Define value types which describe values accepted
 * by the --option's in args.h, and can also describe
 * the values accepted as positional args.
 *
 * Previously, accepted values were only "described"
 * by identifying the parsing function to use.
 *
 * Some standard val types are used by many options,
 * e.g. many options (aa_ARG, bb_ARG, cc_ARG) all
 * accept a number_VAL.
 *
 * Other special val types are used by only one option,
 * e.g. only mirrorlog_ARG accepts a mirrorlog_VAL.
 * This typically means that there are some specific
 * words that are recognized after the option.
 *
 * Some options currently take a standard val type,
 * (esp string_VAL), but they could be given their
 * own custom val type.  The advantage of using a
 * custom val type is the possibility of validating
 * the value when parsing it with a custom parsing
 * function, and the possibility of displaying the
 * actual accepted values in the command usage.
 * Without a custom val type, the code must do ad hoc
 * validation of the string values, and the usage
 * output for the option will only say "String"
 * rather than giving the accepted string values.
 * Even without a custom parsing function, there is
 * reason to define a custom x_VAL enum so that a
 * more descriptive usage string can be specified
 * as opposed to just "String".
 *
 * Most of the val types defined here are used after
 * --option's, and are referenced in foo_ARG entries
 * in args.h.  But, some val types are only used to
 * represent positional values in command definitions,
 * e.g. vg_VAL.
 *
 * val(a, b, c, d)
 *
 * a:  foo_VAL enums
 * b:  the function to parse and set the value
 * c:  the name used to reference this value in command defs
 * d:  what to display in usage output for this value
 *
 * command defintions will use --option NAME, where NAME
 * is shown in val() field c.  NAME will be translated to
 * foo_VAL enum in field a, which is used in commands[]
 * structs.
 *
 * option definitions (arg.h) will reference foo_VAL enum
 * in field a.
 *
 * FIXME: for specialized val types, the set of recognized
 * words is not defined or stored in a consistent way,
 * but is just whatever the parsing function happens to look
 * for, so adding a new accepted value for the val type is
 * generally making the parsing function recognize a new
 * word, and making the implementation code also recognize
 * that word to do something different.  This new word should
 * then also be added to the usage string for the val type here.
 * It would be nice if the accepted values could be defined in a
 * more consistent way, and perhaps in a single place, perhaps in
 * struct val_names.
 *
 * The usage text for an option is not always the full
 * set of words accepted for an option, but may be a
 * subset.  i.e. an outdated word that no longer does
 * anything may not be shown, but may still be recognized
 * and ignored, or an option that shouldn't be used in
 * general isn't shown to avoid suggesting it.
 * e.g. for --activate we show the most common "y|n|ay"
 * without showing the lvmlockd variations "ey|sy" which
 * are not applicable in general.
 *
 * FIXME: are there some specialized or irrelevant
 * options included in the usage text below that should
 * be removed?
 *
 * Size is a Number that takes an optional unit.
 * A full usage could be "Size[b|B|s|S|k|K|m|M|g|G|t|T|p|P|e|E]"
 * but repeating this full specification produces long and
 * cluttered output, and doesn't indicate which unit is the default.
 * "Size[Units]" would be cleaner, as would a subset of
 * common units, e.g. "Size[kmg...]", but neither helps
 * with default.  "Size[k|UNIT]" and "Size[m|UNIT]" show
 * the default, and "UNIT" indicates that other units
 * are possible without listing them all.  This also
 * suggests using the preferred lower case letters, because
 * --size and other option args treat upper/lower letters
 * the same, all as 1024 SI base.  For this reason, we
 * should avoid suggesting the upper case letters.
 */

val(none_VAL, NULL, "None", "ERR")             /* unused, for enum value 0 */
val(conststr_VAL, NULL, "ConstString", "ERR")  /* used only for command defs */
val(constnum_VAL, NULL, "ConstNumber", "ERR")  /* used only for command defs */
val(bool_VAL, yes_no_arg, "Bool", "y|n")
val(number_VAL, int_arg, "Number", NULL)
val(snumber_VAL, int_arg_with_sign, "SNumber", "[+|-]Number")
val(pnumber_VAL, int_arg_with_plus, "PNumber", "[+]Number")
val(uint32_VAL, uint32_arg, "Uint32", "Number")
val(string_VAL, string_arg, "String", NULL)
val(vg_VAL, string_arg, "VG", NULL)
val(lv_VAL, string_arg, "LV", NULL)
val(pv_VAL, string_arg, "PV", NULL)
val(tag_VAL, tag_arg, "Tag", NULL)
val(select_VAL, NULL, "Select", NULL)       /* used only for command defs */
val(activationmode_VAL, string_arg, "ActivationMode", "partial|degraded|complete")
val(activation_VAL, activation_arg, "Active", "y|n|ay")
val(cachemetadataformat_VAL, cachemetadataformat_arg, "CacheMetadataFormat", "auto|1|2")
val(cachemode_VAL, cachemode_arg, "CacheMode", "writethrough|writeback|passthrough")
val(discards_VAL, discards_arg, "Discards", "passdown|nopassdown|ignore")
val(mirrorlog_VAL, mirrorlog_arg, "MirrorLog", "core|disk")
val(sizekb_VAL, size_kb_arg, "SizeKB", "Size[k|UNIT]")
val(ssizekb_VAL, ssize_kb_arg, "SSizeKB", "[+|-]Size[k|UNIT]")
val(sizemb_VAL, size_mb_arg, "SizeMB", "Size[m|UNIT]")
val(ssizemb_VAL, ssize_mb_arg, "SSizeMB", "[+|-]Size[m|UNIT]")
val(psizemb_VAL, psize_mb_arg, "PSizeMB", "[+]Size[m|UNIT]")
val(nsizemb_VAL, nsize_mb_arg, "NSizeMB", "[-]Size[m|UNIT]")
val(regionsizemb_VAL, regionsize_mb_arg, "RegionSize", "Size[m|UNIT]")
val(extents_VAL, extents_arg, "Extents", "Number[PERCENT]")
val(sextents_VAL, sextents_arg, "SExtents", "[+|-]Number[PERCENT]")
val(pextents_VAL, pextents_arg, "PExtents", "[+]Number[PERCENT]")
val(nextents_VAL, nextents_arg, "NExtents", "[-]Number[PERCENT]")
val(permission_VAL, permission_arg, "Permission", "rw|r")
val(metadatatype_VAL, metadatatype_arg, "MetadataType", "lvm2")
val(units_VAL, string_arg, "Units", "r|R|h|H|b|B|s|S|k|K|m|M|g|G|t|T|p|P|e|E")
val(segtype_VAL, segtype_arg, "SegType", "linear|striped|snapshot|mirror|raid|thin|cache|thin-pool|cache-pool")
val(alloc_VAL, alloc_arg, "Alloc", "contiguous|cling|cling_by_tags|normal|anywhere|inherit")
val(locktype_VAL, locktype_arg, "LockType", "sanlock|dlm|none")
val(readahead_VAL, readahead_arg, "Readahead", "auto|none|Number")
val(vgmetadatacopies_VAL, vgmetadatacopies_arg, "MetadataCopiesVG", "all|unmanaged|Number")
val(pvmetadatacopies_VAL, pvmetadatacopies_arg, "MetadataCopiesPV", "0|1|2")
val(metadatacopies_VAL, metadatacopies_arg, "unused", "unused")
val(polloperation_VAL, polloperation_arg, "PollOp", "pvmove|convert|merge|merge_thin")
val(writemostly_VAL, writemostly_arg, "WriteMostlyPV", "PV[:t|n|y]")
val(syncaction_VAL, syncaction_arg, "SyncAction", "check|repair")
val(reportformat_VAL, reportformat_arg, "ReportFmt", "basic|json")
val(configreport_VAL, configreport_arg, "ConfigReport", "log|vg|lv|pv|pvseg|seg")
val(configtype_VAL, configtype_arg, "ConfigType", "current|default|diff|full|list|missing|new|profilable|profilable-command|profilable-metadata")

/* this should always be last */
val(VAL_COUNT, NULL, NULL, NULL)

