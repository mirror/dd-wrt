/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
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

int lvmsadc(struct cmd_context *cmd __attribute__((unused)),
	    int argc __attribute__((unused)),
	    char **argv __attribute__((unused)))
{
	log_error("There's no 'lvmsadc' command in LVM2.");
	log_error("Please use the superior 'dmstats' facilities instead.");
	return ECMD_FAILED;
}

int lvmsar(struct cmd_context *cmd __attribute__((unused)),
	   int argc __attribute__((unused)),
	   char **argv __attribute__((unused)))
{
	log_error("There's no 'lvmsar' command in LVM2.");
	log_error("Please use the superior 'dmstats' facilities instead.");
	return ECMD_FAILED;
}

int pvdata(struct cmd_context *cmd __attribute__((unused)),
	   int argc __attribute__((unused)),
	   char **argv __attribute__((unused)))
{
	log_error("There's no 'pvdata' command in LVM2.");
	log_error("Use lvs, pvs, vgs instead; or use vgcfgbackup and read the text file backup.");
	return ECMD_FAILED;
}

int lvmchange(struct cmd_context *cmd __attribute__((unused)),
	      int argc __attribute__((unused)),
	      char **argv __attribute__((unused)))
{
	log_error("There's no 'lvmchange' command in LVM2.");
	log_error("Use 'dmsetup' commands to reset the kernel device-mapper driver.");
	return ECMD_FAILED;
}

int vgconvert(struct cmd_context *cmd __attribute__((unused)),
	      int argc __attribute__((unused)),
	      char **argv __attribute__((unused)))
{
	log_error("The vgconvert command has been removed along with the lvm1 format.");
	log_error("Use a previous version of lvm to convert the lvm1 format to lvm2.");
	return ECMD_FAILED;
}
