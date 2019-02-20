/*
 * Copyright (C) 2013 Red Hat, Inc. All rights reserved.
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
 * This file defines the fields (columns) for the devtypes reporting command.
 *
 * The preferred order of the field descriptions in the help text
 * determines the order the entries appear in this file.
 *
 * When adding new entries take care to use the existing style.
 * Displayed fields names normally have a type prefix and use underscores.
 * Field-specific internal functions names normally match the displayed
 * field names but without underscores.
 * Help text ends with a full stop.
 */

/* *INDENT-OFF* */
FIELD(DEVTYPES, devtype, STR, "DevType", name, 7, chars, devtype_name, "Name of Device Type exactly as it appears in /proc/devices.", 0)
FIELD(DEVTYPES, devtype, NUM, "MaxParts", max_partitions, 8, int8, devtype_max_partitions, "Maximum number of partitions. (How many device minor numbers get reserved for each device.)", 0)
FIELD(DEVTYPES, devtype, STR, "Description", desc, 11, string, devtype_description, "Description of Device Type.", 0)
/* *INDENT-ON* */
