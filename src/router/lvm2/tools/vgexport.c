/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#include "tools.h"

static int vgexport_single(struct cmd_context *cmd __attribute__((unused)),
			   const char *vg_name,
			   struct volume_group *vg,
			   struct processing_handle *handle __attribute__((unused)))
{
	struct pv_list *pvl;

	if (lvs_in_vg_activated(vg)) {
		log_error("Volume group \"%s\" has active logical volumes",
			  vg_name);
		goto bad;
	}

	if (vg_is_shared(vg)) {
		struct lv_list *lvl;
		dm_list_iterate_items(lvl, &vg->lvs) {
			if (!lockd_lv_uses_lock(lvl->lv))
				continue;

			if (!lockd_lv(cmd, lvl->lv, "ex", 0)) {
				log_error("LV %s/%s must be inactive on all hosts before vgexport.",
					  vg->name, display_lvname(lvl->lv));
				goto bad;
			}

			if (!lockd_lv(cmd, lvl->lv, "un", 0))
				goto bad;
		}
	}


	if (!archive(vg))
		goto_bad;

	vg->status |= EXPORTED_VG;
	vg->system_id = NULL;

	dm_list_iterate_items(pvl, &vg->pvs)
		pvl->pv->status |= EXPORTED_VG;

	if (!vg_write(vg) || !vg_commit(vg))
		goto_bad;

	backup(vg);

	log_print_unless_silent("Volume group \"%s\" successfully exported", vg->name);

	return ECMD_PROCESSED;

bad:
	return ECMD_FAILED;
}

int vgexport(struct cmd_context *cmd, int argc, char **argv)
{
	if (!argc && !arg_is_set(cmd, all_ARG) && !arg_is_set(cmd, select_ARG)) {
		log_error("Please supply volume groups or use --select for selection or use -a for all.");
		return EINVALID_CMD_LINE;
	}

	if (arg_is_set(cmd, all_ARG) && (argc || arg_is_set(cmd, select_ARG))) {
		log_error("No arguments permitted when using -a for all.");
		return EINVALID_CMD_LINE;
	}

	return process_each_vg(cmd, argc, argv, NULL, NULL, READ_FOR_UPDATE, 0, NULL,
			       &vgexport_single);
}
