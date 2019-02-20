/*
 * Copyright (C) 2016 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This file defines the fields (columns) for the command log reporting.
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
FIELD(CMDLOG, cmd_log_item, NUM, "Seq", seq_num, 3, uint32, log_seq_num, "Log sequence number.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "LogType", type, 7, string, log_type, "Log type.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "Context", context, 7, string, log_context, "Current context.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "ObjType", object_type_name, 7, string, log_object_type, "Current object type.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "ObjName", object_name, 7, string, log_object_name, "Current object name.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "ObjID", object_id, 7, string, log_object_id, "Current object ID.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "ObjGrp", object_group, 7, string, log_object_group, "Current object group.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "ObjGrpID", object_group_id, 8, string, log_object_group_id, "Current object group ID.", 0)
FIELD(CMDLOG, cmd_log_item, STR, "Msg", msg, 7, string, log_message, "Log message.", 0)
FIELD(CMDLOG, cmd_log_item, SNUM, "Errno", current_errno, 5, int32, log_errno, "Errno.", 0)
FIELD(CMDLOG, cmd_log_item, SNUM, "RetCode", ret_code, 7, int32, log_ret_code, "Return code.", 0)
/* *INDENT-ON* */
