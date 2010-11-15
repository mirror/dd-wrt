/*
 * Copyright (C) 2005 Markus Klotzbuecher <mk@creamnet.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include "fist.h"
#include "mini_fo.h"


/* create the storage file, setup new states */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
int create_sto_reg_file(dentry_t *dentry, int mode, struct nameidata *nd)
#else
int create_sto_reg_file(dentry_t *dentry, int mode)
#endif
{
	int err = 0;
	inode_t *dir;
	dentry_t *hidden_sto_dentry;
	dentry_t *hidden_sto_dir_dentry;

	if(exists_in_storage(dentry)) {
		printk(KERN_CRIT "mini_fo: create_sto_file: wrong type or state.\n");
		err = -EINVAL;
		goto out;
	}
	err = get_neg_sto_dentry(dentry);

	if (err) {
		printk(KERN_CRIT "mini_fo: create_sto_file: ERROR getting neg. sto dentry.\n");
		goto out;
	}

	dir = dentry->d_parent->d_inode;
	hidden_sto_dentry = dtohd2(dentry);

	/* lock parent */
	hidden_sto_dir_dentry = dget(hidden_sto_dentry->d_parent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
        down(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

	err = PTR_ERR(hidden_sto_dir_dentry);
        if (IS_ERR(hidden_sto_dir_dentry))
                goto out;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	err = vfs_create(hidden_sto_dir_dentry->d_inode,
			 hidden_sto_dentry,
			 mode, nd);
#else
	err = vfs_create(hidden_sto_dir_dentry->d_inode,
			 hidden_sto_dentry,
			 mode);
#endif
        if(err) {
		printk(KERN_CRIT "mini_fo: create_sto_file: ERROR creating sto file.\n");
                goto out_lock;
	}

	if(!dtohd2(dentry)->d_inode) {
		printk(KERN_CRIT "mini_fo: create_sto_file: ERROR creating sto file [2].\n");
                err = -EINVAL;
                goto out_lock;
        }

        /* interpose the new inode */
        if(dtost(dentry) == DELETED) {
                dtost(dentry) = DEL_REWRITTEN;
                err = mini_fo_tri_interpose(NULL, hidden_sto_dentry, dentry, dir->i_sb, 0);
                if(err)
                        goto out_lock;
        }
        else if(dtost(dentry) == NON_EXISTANT) {
                dtost(dentry) = CREATED;
                err = mini_fo_tri_interpose(dtohd(dentry), hidden_sto_dentry, dentry, dir->i_sb, 0);
                if(err)
                        goto out_lock;
        }
        else if(dtost(dentry) == UNMODIFIED) {
                dtost(dentry) = MODIFIED;
                /* interpose on new inode */
                if(itohi2(dentry->d_inode) != NULL) {
                        printk(KERN_CRIT "mini_fo: create_sto_file: invalid inode detected.\n");
                        err = -EINVAL;
                        goto out_lock;
                }
                itohi2(dentry->d_inode) = igrab(dtohd2(dentry)->d_inode);
	}
	fist_copy_attr_timesizes(dentry->d_parent->d_inode,
				 hidden_sto_dir_dentry->d_inode);

 out_lock:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif
        dput(hidden_sto_dir_dentry);
 out:
	return err;
}

/* create the sto dir, setup states */
int create_sto_dir(dentry_t *dentry, int mode)
{
	int err = 0;
	inode_t *dir;
	dentry_t *hidden_sto_dentry;
        dentry_t *hidden_sto_dir_dentry;

	/* had to take the "!S_ISDIR(mode))" check out, because it failed */
	if(exists_in_storage(dentry)) {
                printk(KERN_CRIT "mini_fo: create_sto_dir: wrong type or state.\\
n");
                err = -EINVAL;
                goto out;
        }

	err = get_neg_sto_dentry(dentry);
	if(err) {
		err = -EINVAL;
		goto out;
	}

	dir = dentry->d_parent->d_inode;
	hidden_sto_dentry = dtohd2(dentry);

	/* was: hidden_sto_dir_dentry = lock_parent(hidden_sto_dentry); */
	hidden_sto_dir_dentry = dget(hidden_sto_dentry->d_parent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	down(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

	err = PTR_ERR(hidden_sto_dir_dentry);
	if (IS_ERR(hidden_sto_dir_dentry))
		goto out;

	err = vfs_mkdir(hidden_sto_dir_dentry->d_inode,
			hidden_sto_dentry,
			mode);
	if(err) {
		printk(KERN_CRIT "mini_fo: create_sto_dir: ERROR creating sto dir.\n");
		goto out_lock;
	}

	if(!dtohd2(dentry)->d_inode) {
		printk(KERN_CRIT "mini_fo: create_sto_dir: ERROR creating sto dir [2].\n");
		err = -EINVAL;
		goto out_lock;
	}

	/* interpose the new inode */
	if(dtost(dentry) == DELETED) {
		dtost(dentry) = DEL_REWRITTEN;
		err = mini_fo_tri_interpose(NULL, hidden_sto_dentry, dentry, dir->i_sb, 0);
		if(err)
			goto out_lock;
	}
	else if(dtopd(dentry)->state == NON_EXISTANT) {
		dtopd(dentry)->state = CREATED;
		err = mini_fo_tri_interpose(dtohd(dentry), hidden_sto_dentry, dentry, dir->i_sb, 0);
		if(err)
			goto out_lock;
	}
	else if(dtopd(dentry)->state == UNMODIFIED) {
		dtopd(dentry)->state = MODIFIED;
		/* interpose on new inode */
		if(itohi2(dentry->d_inode) != NULL) {
			printk(KERN_CRIT "mini_fo:  create_sto_dir: ERROR, invalid inode detected.\n");
			err = -EINVAL;
			goto out_lock;
		}
		itohi2(dentry->d_inode) = igrab(dtohd2(dentry)->d_inode);
	}

	fist_copy_attr_timesizes(dir, hidden_sto_dir_dentry->d_inode);

	/* initalize the wol list */
	itopd(dentry->d_inode)->deleted_list_size = -1;
	itopd(dentry->d_inode)->renamed_list_size = -1;
	meta_build_lists(dentry);


 out_lock:
	/* was: unlock_dir(hidden_sto_dir_dentry); */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif
	dput(hidden_sto_dir_dentry);
 out:
	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
int create_sto_nod(dentry_t *dentry, int mode, dev_t dev)
#else
int create_sto_nod(dentry_t *dentry, int mode, int dev)
#endif
{
	int err = 0;
	inode_t *dir;
	dentry_t *hidden_sto_dentry;
	dentry_t *hidden_sto_dir_dentry;

	if(exists_in_storage(dentry)) {
		err = -EEXIST;
		goto out;
	}
	err = get_neg_sto_dentry(dentry);

	if (err) {
                printk(KERN_CRIT "mini_fo: create_sto_nod: ERROR getting neg. sto dentry.\n");
                goto out;
        }

	dir = dentry->d_parent->d_inode;
	hidden_sto_dentry = dtohd2(dentry);

	/* lock parent */
	hidden_sto_dir_dentry = dget(hidden_sto_dentry->d_parent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	down(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

	err = PTR_ERR(hidden_sto_dir_dentry);
	if (IS_ERR(hidden_sto_dir_dentry))
		goto out;

	err = vfs_mknod(hidden_sto_dir_dentry->d_inode, hidden_sto_dentry, mode, dev);
	if(err)
		goto out_lock;

	if(!dtohd2(dentry)->d_inode) {
		printk(KERN_CRIT "mini_fo: create_sto_nod: creating storage inode failed [1].\n");
		err = -EINVAL; /* return something indicating failure */
		goto out_lock;
	}

	/* interpose the new inode */
	if(dtost(dentry) == DELETED) {
		dtost(dentry) = DEL_REWRITTEN;
		err = mini_fo_tri_interpose(NULL, hidden_sto_dentry, dentry, dir->i_sb, 0);
		if(err)
			goto out_lock;
	}
	else if(dtost(dentry) == NON_EXISTANT) {
		dtost(dentry) = CREATED;
		err = mini_fo_tri_interpose(dtohd(dentry), hidden_sto_dentry, dentry, dir->i_sb, 0);
		if(err)
			goto out_lock;
	}
	else if(dtost(dentry) == UNMODIFIED) {
		dtost(dentry) = MODIFIED;
		/* interpose on new inode */
		if(itohi2(dentry->d_inode) != NULL) {
			printk(KERN_CRIT "mini_fo: create_sto_nod: error, invalid inode detected.\n");
			err = -EINVAL;
			goto out_lock;
		}
		itohi2(dentry->d_inode) = igrab(dtohd2(dentry)->d_inode);
	}

	fist_copy_attr_timesizes(dir, hidden_sto_dir_dentry->d_inode);

 out_lock:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif
	dput(hidden_sto_dir_dentry);
 out:
	return err;
}


/* unimplemented (and possibly not usefull):

   nondir-del_to_del_rew
   nondir-non_exist_to_creat

   dir-unmod_to_del
   dir-mod_to_del
   dir-creat_to_del
   dir-del_rew_to_del
   dir-del_to_del_rew
   dir-non_exist_to_creat
*/


/* bring a file of any type from state UNMODIFIED to MODIFIED */
int nondir_unmod_to_mod(dentry_t *dentry, int cp_flag)
{
	int err = 0;
	struct vfsmount *tgt_mnt;
	struct vfsmount *src_mnt;
	dentry_t *tgt_dentry;
	dentry_t *src_dentry;
	dentry_t *hidden_sto_dentry;
	dentry_t *hidden_sto_dir_dentry;

	check_mini_fo_dentry(dentry);

	if((dtost(dentry) != UNMODIFIED) ||
	   S_ISDIR(dentry->d_inode->i_mode)) {
		printk(KERN_CRIT "mini_fo: nondir_unmod_to_mod: \
                                  wrong type or state.\n");
		err = -1;
		goto out;
	}
	err = get_neg_sto_dentry(dentry);

	if (err) {
		printk(KERN_CRIT "mini_fo: nondir_unmod_to_mod: \
                                  ERROR getting neg. sto dentry.\n");
		goto out;
	}

	/* create sto file */
	hidden_sto_dentry = dtohd2(dentry);

	/* lock parent */
	hidden_sto_dir_dentry = dget(hidden_sto_dentry->d_parent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
        down(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

	err = PTR_ERR(hidden_sto_dir_dentry);
        if (IS_ERR(hidden_sto_dir_dentry))
                goto out;

	/* handle different types of nondirs */
	if(S_ISCHR(dentry->d_inode->i_mode) ||
	   S_ISBLK(dentry->d_inode->i_mode)) {
		err = vfs_mknod(hidden_sto_dir_dentry->d_inode,
				hidden_sto_dentry,
				dtohd(dentry)->d_inode->i_mode,
				dtohd(dentry)->d_inode->i_rdev);
	}

	else if(S_ISREG(dentry->d_inode->i_mode)) {

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
		err = vfs_create(hidden_sto_dir_dentry->d_inode,
				 hidden_sto_dentry,
				 dtohd(dentry)->d_inode->i_mode, NULL);
#else
		err = vfs_create(hidden_sto_dir_dentry->d_inode,
				 hidden_sto_dentry,
				 dtohd(dentry)->d_inode->i_mode);
#endif
	}
        if(err) {
		printk(KERN_CRIT "mini_fo: nondir_unmod_to_mod: \
                                  ERROR creating sto file.\n");
                goto out_lock;
	}

	/* interpose on new inode */
	if(itohi2(dentry->d_inode) != NULL) {
		printk(KERN_CRIT "mini_fo: nondir_unmod_to_mod: \
                                  ERROR, invalid inode detected.\n");
		err = -EINVAL;
		goto out_lock;
	}

	itohi2(dentry->d_inode) = igrab(dtohd2(dentry)->d_inode);

        fist_copy_attr_timesizes(dentry->d_parent->d_inode,
				 hidden_sto_dir_dentry->d_inode);
	dtost(dentry) = MODIFIED;

	/* copy contents if regular file and cp_flag = 1 */
	if((cp_flag == 1) && S_ISREG(dentry->d_inode->i_mode)) {

		/* unlock first */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
		mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
		up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

		dput(hidden_sto_dir_dentry);

		tgt_dentry = dtohd2(dentry);
		tgt_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt2;
		src_dentry = dtohd(dentry);
		src_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt;

		err = mini_fo_cp_cont(tgt_dentry, tgt_mnt,
				      src_dentry, src_mnt);
		if(err) {
			printk(KERN_CRIT "mini_fo: nondir_unmod_to_mod: \
                                          ERROR copying contents.\n");
		}
		goto out;
	}

 out_lock:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif
        dput(hidden_sto_dir_dentry);
 out:
	return err;
}

/* this function is currently identical to nondir_creat_to_del */
int nondir_del_rew_to_del(dentry_t *dentry)
{
	return nondir_creat_to_del(dentry);
}

int nondir_creat_to_del(dentry_t *dentry)
{
	int err = 0;

	inode_t *hidden_sto_dir_inode;
	dentry_t *hidden_sto_dir_dentry;
	dentry_t *hidden_sto_dentry;

	check_mini_fo_dentry(dentry);

	/* for now this function serves for both state DEL_REWRITTEN and
	 * CREATED */
	if(!(dtost(dentry) == CREATED || (dtost(dentry) == DEL_REWRITTEN)) ||
	   S_ISDIR(dentry->d_inode->i_mode)) {
		printk(KERN_CRIT "mini_fo: nondir_mod_to_del/del_rew_to_del: \
                                  wrong type or state.\n");
		err = -1;
		goto out;
	}

	hidden_sto_dir_inode = itohi2(dentry->d_parent->d_inode);
	hidden_sto_dentry = dtohd2(dentry);

	/* was: hidden_sto_dir_dentry = lock_parent(hidden_sto_dentry);*/
	hidden_sto_dir_dentry = dget(hidden_sto_dentry->d_parent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	down(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

	/* avoid destroying the hidden inode if the file is in use */
	dget(hidden_sto_dentry);
	err = vfs_unlink(hidden_sto_dir_inode, hidden_sto_dentry);
	dput(hidden_sto_dentry);
	if(!err)
		d_delete(hidden_sto_dentry);

	/* propagate number of hard-links */
	dentry->d_inode->i_nlink = itohi2(dentry->d_inode)->i_nlink;

	dtost(dentry) = NON_EXISTANT;

	/* was: unlock_dir(hidden_sto_dir_dentry); */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif
	dput(hidden_sto_dir_dentry);

 out:
	return err;
}

int nondir_mod_to_del(dentry_t *dentry)
{
	int err;
	dentry_t *hidden_sto_dentry;
	inode_t *hidden_sto_dir_inode;
	dentry_t *hidden_sto_dir_dentry;

	check_mini_fo_dentry(dentry);

	if(dtost(dentry) != MODIFIED ||
	   S_ISDIR(dentry->d_inode->i_mode)) {
		printk(KERN_CRIT "mini_fo: nondir_mod_to_del: \
                                  wrong type or state.\n");
		err = -1;
		goto out;
	}

	hidden_sto_dir_inode = itohi2(dentry->d_parent->d_inode);
	hidden_sto_dentry = dtohd2(dentry);

	/* was hidden_sto_dir_dentry = lock_parent(hidden_sto_dentry); */
	hidden_sto_dir_dentry = dget(hidden_sto_dentry->d_parent);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	down(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif

	/* avoid destroying the hidden inode if the file is in use */
	dget(hidden_sto_dentry);
	err = vfs_unlink(hidden_sto_dir_inode, hidden_sto_dentry);
	dput(hidden_sto_dentry);
	if(!err)
		d_delete(hidden_sto_dentry);

	/* propagate number of hard-links */
	dentry->d_inode->i_nlink = itohi2(dentry->d_inode)->i_nlink;

	/* dput base dentry, this will relase the inode and free the
	 * dentry, as we will never need it again. */
	dput(dtohd(dentry));
	dtohd(dentry) = NULL;
	dtost(dentry) = DELETED;

	/* was: unlock_dir(hidden_sto_dir_dentry); */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&hidden_sto_dir_dentry->d_inode->i_mutex);
#else
	up(&hidden_sto_dir_dentry->d_inode->i_sem);
#endif
	/* add deleted file to META-file */
	meta_add_d_entry(dentry->d_parent,
			 dentry->d_name.name,
			 dentry->d_name.len);

	dput(hidden_sto_dir_dentry);

 out:
	return err;
}

int nondir_unmod_to_del(dentry_t *dentry)
{
	int err = 0;

	check_mini_fo_dentry(dentry);

	if(dtost(dentry) != UNMODIFIED ||
	   S_ISDIR(dentry->d_inode->i_mode)) {
		printk(KERN_CRIT "mini_fo: nondir_unmod_to_del: \
                                  wrong type or state.\n");
		err = -1;
		goto out;
	}

	 /* next we have to get a negative dentry for the storage file */
	err = get_neg_sto_dentry(dentry);

	if(err)
		goto out;

	/* add deleted file to META lists */
	err = meta_add_d_entry(dentry->d_parent,
			       dentry->d_name.name,
			       dentry->d_name.len);

	if(err)
		goto out;

	/* dput base dentry, this will relase the inode and free the
	 * dentry, as we will never need it again. */
	dput(dtohd(dentry));
	dtohd(dentry) = NULL;
	dtost(dentry) = DELETED;

 out:
	return err;
}

/* bring a dir from state UNMODIFIED to MODIFIED */
int dir_unmod_to_mod(dentry_t *dentry)
{
	int err;

	check_mini_fo_dentry(dentry);

	if(dtost(dentry) != UNMODIFIED ||
	   !S_ISDIR(dentry->d_inode->i_mode)) {
		printk(KERN_CRIT "mini_fo: dir_unmod_to_mod: \
                                  wrong type or state.\n");
		err = -1;
		goto out;
	}

	/* this creates our dir incl. sto. structure */
	err = build_sto_structure(dentry->d_parent, dentry);
	if(err) {
		printk(KERN_CRIT "mini_fo: dir_unmod_to_mod: \
                                  build_sto_structure failed.\n");
		goto out;
	}
 out:
	return err;
}

