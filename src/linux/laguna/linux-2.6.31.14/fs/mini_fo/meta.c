/*
 * Copyright (C) 2004, 2005 Markus Klotzbuecher <mk@creamnet.de>
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

int meta_build_lists(dentry_t *dentry)
{
	struct mini_fo_inode_info *inode_info;

	dentry_t *meta_dentry = 0;
	file_t *meta_file = 0;
	mm_segment_t old_fs;
	void *buf;

	int bytes, len;
	struct vfsmount *meta_mnt;
	char *entry;

	inode_info = itopd(dentry->d_inode);
	if(!(inode_info->deleted_list_size == -1 &&
	     inode_info->renamed_list_size == -1)) {
		printk(KERN_CRIT "mini_fo: meta_build_lists: \
                                  Error, list(s) not virgin.\n");
		return -1;
	}

	/* init our meta lists */
	INIT_LIST_HEAD(&inode_info->deleted_list);
	inode_info->deleted_list_size = 0;

	INIT_LIST_HEAD(&inode_info->renamed_list);
	inode_info->renamed_list_size = 0;

  	/* might there be a META-file? */
	if(dtohd2(dentry) && dtohd2(dentry)->d_inode) {
		mutex_lock(&dtohd2(dentry)->d_inode->i_mutex);
		meta_dentry = lookup_one_len(META_FILENAME,
					     dtohd2(dentry),
					     strlen(META_FILENAME));
		mutex_unlock(&dtohd2(dentry)->d_inode->i_mutex);
		if(!meta_dentry->d_inode) {
			dput(meta_dentry);
			goto out_ok;
		}
		/* $%& err, is this correct? */
		meta_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt2;
		mntget(meta_mnt);


		/* open META-file for reading */
		meta_file = dentry_open(meta_dentry, meta_mnt, 0x0, current_cred());
		if(!meta_file || IS_ERR(meta_file)) {
			printk(KERN_CRIT "mini_fo: meta_build_lists: \
                                          ERROR opening META file.\n");
			goto out_err;
		}

		/* check if fs supports reading */
		if(!meta_file->f_op->read) {
			printk(KERN_CRIT "mini_fo: meta_build_lists: \
                                          ERROR, fs does not support reading.\n");
			goto out_err_close;
		}

		/* allocate a page for transfering the data */
		buf = (void *) __get_free_page(GFP_KERNEL);
		if(!buf) {
			printk(KERN_CRIT "mini_fo: meta_build_lists: \
                                          ERROR, out of mem.\n");
			goto out_err_close;
		}
		meta_file->f_pos = 0;
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		do {
			char *c;
			bytes = meta_file->f_op->read(meta_file, buf, PAGE_SIZE, &meta_file->f_pos);
			if(bytes == PAGE_SIZE) {
				/* trim a cut off filename and adjust f_pos to get it next time */
				for(c = (char*) buf+PAGE_SIZE;
				    *c != '\n';
				    c--, bytes--, meta_file->f_pos--);
			}
			entry = (char *) buf;
			while(entry < (char *) buf+bytes) {

				char *old_path;
				char *dir_name;
				int old_len, new_len;

				/* len without '\n'*/
				len = (int) (strchr(entry, '\n') - entry);
				switch (*entry) {
				case 'D':
					/* format: "D filename" */
					meta_list_add_d_entry(dentry,
							      entry+2,
							      len-2);
					break;
				case 'R':
					/* format: "R path/xy/dir newDir" */
					old_path = entry+2;
					dir_name = strchr(old_path, ' ') + 1;
					old_len =  dir_name - old_path - 1;
					new_len = ((int) entry) + len - ((int ) dir_name);
					meta_list_add_r_entry(dentry,
							      old_path,
							      old_len,
							      dir_name,
							      new_len);
					break;
				default:
					/* unknown entry type detected */
					break;
				}
				entry += len+1;
			}

		} while(meta_file->f_pos < meta_dentry->d_inode->i_size);

		free_page((unsigned long) buf);
		set_fs(old_fs);
		fput(meta_file);
	}
	goto out_ok;

 out_err_close:
	fput(meta_file);
 out_err:
	mntput(meta_mnt);
	dput(meta_dentry);
	return -1;
 out_ok:
	return 1; /* check this!!! inode_info->wol_size; */
}

/* cleanups up all lists and free's the mem by dentry */
int meta_put_lists(dentry_t *dentry)
{
	if(!dentry || !dentry->d_inode) {
		printk("mini_fo: meta_put_lists: invalid dentry passed.\n");
		return -1;
	}
	return __meta_put_lists(dentry->d_inode);
}

/* cleanups up all lists and free's the mem by inode */
int __meta_put_lists(inode_t *inode)
{
	int err = 0;
	if(!inode || !itopd(inode)) {
		printk("mini_fo: __meta_put_lists: invalid inode passed.\n");
		return -1;
	}
	err = __meta_put_d_list(inode);
	err |= __meta_put_r_list(inode);
	return err;
}

int meta_sync_lists(dentry_t *dentry)
{
	int err = 0;
	if(!dentry || !dentry->d_inode) {
		printk("mini_fo: meta_sync_lists: \
                        invalid dentry passed.\n");
		return -1;
	}
	err = meta_sync_d_list(dentry, 0);
	err |= meta_sync_r_list(dentry, 1);
	return err;
}


/* remove all D entries from the renamed list and free the mem */
int __meta_put_d_list(inode_t *inode)
{
	struct list_head *tmp;
        struct deleted_entry *del_entry;
        struct mini_fo_inode_info *inode_info;

	if(!inode || !itopd(inode)) {
		printk(KERN_CRIT "mini_fo: __meta_put_d_list: \
                                  invalid inode passed.\n");
		return -1;
	}
	inode_info = itopd(inode);

        /* nuke the DELETED-list */
        if(inode_info->deleted_list_size <= 0)
		return 0;

	while(!list_empty(&inode_info->deleted_list)) {
		tmp = inode_info->deleted_list.next;
		list_del(tmp);
		del_entry = list_entry(tmp, struct deleted_entry, list);
		kfree(del_entry->name);
		kfree(del_entry);
	}
	inode_info->deleted_list_size = 0;

	return 0;
}

/* remove all R entries from the renamed list and free the mem */
int __meta_put_r_list(inode_t *inode)
{
	struct list_head *tmp;
	struct renamed_entry *ren_entry;
        struct mini_fo_inode_info *inode_info;

	if(!inode || !itopd(inode)) {
		printk(KERN_CRIT "mini_fo: meta_put_r_list: invalid inode.\n");
		return -1;
	}
	inode_info = itopd(inode);

        /* nuke the RENAMED-list */
        if(inode_info->renamed_list_size <= 0)
		return 0;

	while(!list_empty(&inode_info->renamed_list)) {
		tmp = inode_info->renamed_list.next;
		list_del(tmp);
		ren_entry = list_entry(tmp, struct renamed_entry, list);
		kfree(ren_entry->new_name);
		kfree(ren_entry->old_name);
		kfree(ren_entry);
	}
	inode_info->renamed_list_size = 0;

	return 0;
}

int meta_add_d_entry(dentry_t *dentry, const char *name, int len)
{
	int err = 0;
	err = meta_list_add_d_entry(dentry, name, len);
	err |= meta_write_d_entry(dentry,name,len);
	return err;
}

/* add a D entry to the deleted list */
int meta_list_add_d_entry(dentry_t *dentry, const char *name, int len)
{
        struct deleted_entry *del_entry;
        struct mini_fo_inode_info *inode_info;

	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT "mini_fo: meta_list_add_d_entry: \
                                  invalid dentry passed.\n");
		return -1;
	}
	inode_info = itopd(dentry->d_inode);

        if(inode_info->deleted_list_size < 0)
                return -1;

        del_entry = (struct deleted_entry *)
		kmalloc(sizeof(struct deleted_entry), GFP_KERNEL);
        del_entry->name = (char*) kmalloc(len, GFP_KERNEL);
        if(!del_entry || !del_entry->name) {
                printk(KERN_CRIT "mini_fo: meta_list_add_d_entry: \
                                  out of mem.\n");
		kfree(del_entry->name);
		kfree(del_entry);
                return -ENOMEM;
        }

        strncpy(del_entry->name, name, len);
        del_entry->len = len;

        list_add(&del_entry->list, &inode_info->deleted_list);
        inode_info->deleted_list_size++;
        return 0;
}

int meta_add_r_entry(dentry_t *dentry,
			  const char *old_name, int old_len,
			  const char *new_name, int new_len)
{
	int err = 0;
	err = meta_list_add_r_entry(dentry,
				    old_name, old_len,
				    new_name, new_len);
	err |= meta_write_r_entry(dentry,
				  old_name, old_len,
				  new_name, new_len);
	return err;
}

/* add a R entry to the renamed list */
int meta_list_add_r_entry(dentry_t *dentry,
			  const char *old_name, int old_len,
			  const char *new_name, int new_len)
{
        struct renamed_entry *ren_entry;
        struct mini_fo_inode_info *inode_info;

	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT "mini_fo: meta_list_add_r_entry: \
                                  invalid dentry passed.\n");
		return -1;
	}
	inode_info = itopd(dentry->d_inode);

        if(inode_info->renamed_list_size < 0)
                return -1;

        ren_entry = (struct renamed_entry *)
		kmalloc(sizeof(struct renamed_entry), GFP_KERNEL);
        ren_entry->old_name = (char*) kmalloc(old_len, GFP_KERNEL);
        ren_entry->new_name = (char*) kmalloc(new_len, GFP_KERNEL);

        if(!ren_entry || !ren_entry->old_name || !ren_entry->new_name) {
                printk(KERN_CRIT "mini_fo: meta_list_add_r_entry: \
                                  out of mem.\n");
		kfree(ren_entry->new_name);
		kfree(ren_entry->old_name);
		kfree(ren_entry);
                return -ENOMEM;
        }

        strncpy(ren_entry->old_name, old_name, old_len);
        ren_entry->old_len = old_len;
        strncpy(ren_entry->new_name, new_name, new_len);
        ren_entry->new_len = new_len;

        list_add(&ren_entry->list, &inode_info->renamed_list);
        inode_info->renamed_list_size++;
        return 0;
}


int meta_remove_r_entry(dentry_t *dentry, const char *name, int len)
{
	int err = 0;
	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT
		       "mini_fo: meta_remove_r_entry: \
                        invalid dentry passed.\n");
		return -1;
	}

	err = meta_list_remove_r_entry(dentry, name, len);
	err |= meta_sync_lists(dentry);
	return err;
}

int meta_list_remove_r_entry(dentry_t *dentry, const char *name, int len)
{
	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT
		       "mini_fo: meta_list_remove_r_entry: \
                        invalid dentry passed.\n");
		return -1;
	}
	return __meta_list_remove_r_entry(dentry->d_inode, name, len);
}

int __meta_list_remove_r_entry(inode_t *inode, const char *name, int len)
{
	struct list_head *tmp;
        struct renamed_entry *ren_entry;
        struct mini_fo_inode_info *inode_info;

	if(!inode || !itopd(inode))
		printk(KERN_CRIT
		       "mini_fo: __meta_list_remove_r_entry: \
                        invalid inode passed.\n");
	inode_info = itopd(inode);

        if(inode_info->renamed_list_size < 0)
                return -1;
        if(inode_info->renamed_list_size == 0)
                return 1;

	list_for_each(tmp, &inode_info->renamed_list) {
		ren_entry = list_entry(tmp, struct renamed_entry, list);
		if(ren_entry->new_len != len)
			continue;

		if(!strncmp(ren_entry->new_name, name, len)) {
			list_del(tmp);
			kfree(ren_entry->new_name);
			kfree(ren_entry->old_name);
			kfree(ren_entry);
			inode_info->renamed_list_size--;
			return 0;
		}
	}
	return 1;
}


/* append a single D entry to the meta file */
int meta_write_d_entry(dentry_t *dentry, const char *name, int len)
{
	dentry_t *meta_dentry = 0;
        file_t *meta_file = 0;
        mm_segment_t old_fs;

        int bytes, err;
        struct vfsmount *meta_mnt = 0;
        char *buf;

	err = 0;

	if(itopd(dentry->d_inode)->deleted_list_size < 0) {
		err = -1;
		goto out;
	}

	if(dtopd(dentry)->state == UNMODIFIED) {
                err = build_sto_structure(dentry->d_parent, dentry);
                if(err) {
                        printk(KERN_CRIT "mini_fo: meta_write_d_entry: \
                                          build_sto_structure failed.\n");
			goto out;
                }
        }

	mutex_lock(&dtohd2(dentry)->d_inode->i_mutex);
	meta_dentry = lookup_one_len(META_FILENAME,
				     dtohd2(dentry), strlen (META_FILENAME));
	mutex_unlock(&dtohd2(dentry)->d_inode->i_mutex);

	/* We need to create a META-file */
        if(!meta_dentry->d_inode) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
		vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry,
			   S_IRUSR | S_IWUSR,
			   NULL);
#else
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry,
			   S_IRUSR | S_IWUSR);
#endif
	}

	/* $%& err, is this correct? */
	meta_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt2;
	mntget(meta_mnt);

        /* open META-file for writing */
        meta_file = dentry_open(meta_dentry, meta_mnt, 0x1, current_cred());
        if(!meta_file || IS_ERR(meta_file)) {
                printk(KERN_CRIT "mini_fo: meta_write_d_entry: \
                                  ERROR opening meta file.\n");
                mntput(meta_mnt); /* $%& is this necessary? */
                dput(meta_dentry);
		err = -1;
                goto out;
        }

        /* check if fs supports writing */
        if(!meta_file->f_op->write) {
                printk(KERN_CRIT "mini_fo: meta_write_d_entry: \
                                  ERROR, fs does not support writing.\n");
                goto out_err_close;
        }

	meta_file->f_pos = meta_dentry->d_inode->i_size; /* append */
        old_fs = get_fs();
        set_fs(KERNEL_DS);

	/* size: len for name, 1 for \n and 2 for "D " */
	buf = (char *) kmalloc(len+3, GFP_KERNEL);
	if (!buf) {
		printk(KERN_CRIT "mini_fo: meta_write_d_entry: \
                                  out of mem.\n");
		return -ENOMEM;
	}

	buf[0] = 'D';
	buf[1] = ' ';
	strncpy(buf+2, name, len);
	buf[len+2] = '\n';
	bytes = meta_file->f_op->write(meta_file, buf, len+3,
				       &meta_file->f_pos);
	if(bytes != len+3) {
		printk(KERN_CRIT "mini_fo: meta_write_d_entry: \
                                  ERROR writing.\n");
		err = -1;
	}
	kfree(buf);
	set_fs(old_fs);

 out_err_close:
	fput(meta_file);
 out:
	return err;
}

/* append a single R entry to the meta file */
int meta_write_r_entry(dentry_t *dentry,
		       const char *old_name, int old_len,
		       const char *new_name, int new_len)
{
	dentry_t *meta_dentry = 0;
        file_t *meta_file = 0;
        mm_segment_t old_fs;

        int bytes, err, buf_len;
	struct vfsmount *meta_mnt = 0;
        char *buf;


	err = 0;

	if(itopd(dentry->d_inode)->renamed_list_size < 0) {
		err = -1;
		goto out;
	}

	/* build the storage structure? */
	if(dtopd(dentry)->state == UNMODIFIED) {
                err = build_sto_structure(dentry->d_parent, dentry);
                if(err) {
                        printk(KERN_CRIT "mini_fo: meta_write_r_entry: \
                                          build_sto_structure failed.\n");
			goto out;
                }
        }

	mutex_lock(&dtohd2(dentry)->d_inode->i_mutex);
	meta_dentry = lookup_one_len(META_FILENAME,
				     dtohd2(dentry),
				     strlen (META_FILENAME));
	mutex_unlock(&dtohd2(dentry)->d_inode->i_mutex);

        if(!meta_dentry->d_inode) {
                /* We need to create a META-file */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry, S_IRUSR | S_IWUSR, NULL);
#else
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry, S_IRUSR | S_IWUSR);
#endif
	}

	/* $%& err, is this correct? */
	meta_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt2;
	mntget(meta_mnt);

        /* open META-file for writing */
        meta_file = dentry_open(meta_dentry, meta_mnt, 0x1, current_cred());
        if(!meta_file || IS_ERR(meta_file)) {
                printk(KERN_CRIT "mini_fo: meta_write_r_entry: \
                                  ERROR opening meta file.\n");
                mntput(meta_mnt);
                dput(meta_dentry);
		err = -1;
                goto out;
        }

        /* check if fs supports writing */
        if(!meta_file->f_op->write) {
                printk(KERN_CRIT "mini_fo: meta_write_r_entry: \
                                  ERROR, fs does not support writing.\n");
                goto out_err_close;
        }

	meta_file->f_pos = meta_dentry->d_inode->i_size; /* append */
        old_fs = get_fs();
        set_fs(KERNEL_DS);

	/* size: 2 for "R ", old_len+new_len for names, 1 blank+1 \n */
	buf_len = old_len + new_len + 4;
	buf = (char *) kmalloc(buf_len, GFP_KERNEL);
	if (!buf) {
		printk(KERN_CRIT "mini_fo: meta_write_r_entry: out of mem.\n");
		return -ENOMEM;
	}

	buf[0] = 'R';
	buf[1] = ' ';
	strncpy(buf + 2, old_name, old_len);
	buf[old_len + 2] = ' ';
	strncpy(buf + old_len + 3, new_name, new_len);
	buf[buf_len -1] = '\n';
	bytes = meta_file->f_op->write(meta_file, buf, buf_len, &meta_file->f_pos);
	if(bytes != buf_len) {
		printk(KERN_CRIT "mini_fo: meta_write_r_entry: ERROR writing.\n");
		err = -1;
	}

	kfree(buf);
	set_fs(old_fs);

 out_err_close:
	fput(meta_file);
 out:
	return err;
}

/* sync D list to disk, append data if app_flag is 1 */
/* check the meta_mnt, which seems not to be used (properly)  */

int meta_sync_d_list(dentry_t *dentry, int app_flag)
{
	dentry_t *meta_dentry;
        file_t *meta_file;
        mm_segment_t old_fs;

        int bytes, err;
        struct vfsmount *meta_mnt;
        char *buf;

	struct list_head *tmp;
        struct deleted_entry *del_entry;
        struct mini_fo_inode_info *inode_info;

	err = 0;
	meta_file=0;
	meta_mnt=0;

	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                  invalid inode passed.\n");
		err = -1;
		goto out;
	}
	inode_info = itopd(dentry->d_inode);

        if(inode_info->deleted_list_size < 0) {
		err = -1;
		goto out;
	}

	/* ok, there is something to sync */

	/* build the storage structure? */
        if(!dtohd2(dentry) && !itohi2(dentry->d_inode)) {
                err = build_sto_structure(dentry->d_parent, dentry);
                if(err) {
                        printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                          build_sto_structure failed.\n");
			goto out;
                }
        }

	mutex_lock(&dtohd2(dentry)->d_inode->i_mutex);
	meta_dentry = lookup_one_len(META_FILENAME,
				     dtohd2(dentry),
				     strlen(META_FILENAME));
	mutex_unlock(&dtohd2(dentry)->d_inode->i_mutex);

        if(!meta_dentry->d_inode) {
                /* We need to create a META-file */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry, S_IRUSR | S_IWUSR, NULL);
#else
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry, S_IRUSR | S_IWUSR);
#endif
		app_flag = 0;
	}
	/* need we truncate the meta file? */
	if(!app_flag) {
		struct iattr newattrs;
                newattrs.ia_size = 0;
                newattrs.ia_valid = ATTR_SIZE | ATTR_CTIME;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
		mutex_lock(&meta_dentry->d_inode->i_mutex);
#else
                down(&meta_dentry->d_inode->i_sem);
#endif
                err = notify_change(meta_dentry, &newattrs);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
		mutex_unlock(&meta_dentry->d_inode->i_mutex);
#else
                up(&meta_dentry->d_inode->i_sem);
#endif

                if(err || meta_dentry->d_inode->i_size != 0) {
                        printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                          ERROR truncating meta file.\n");
                        goto out_err_close;
		}
	}

	/* $%& err, is this correct? */
	meta_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt2;
	mntget(meta_mnt);

        /* open META-file for writing */
        meta_file = dentry_open(meta_dentry, meta_mnt, 0x1, current_cred());
        if(!meta_file || IS_ERR(meta_file)) {
                printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                  ERROR opening meta file.\n");
		mntput(meta_mnt);
		dput(meta_dentry);
		err = -1;
                goto out;
        }

        /* check if fs supports writing */
        if(!meta_file->f_op->write) {
                printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                  ERROR, fs does not support writing.\n");
                goto out_err_close;
        }

	meta_file->f_pos = meta_dentry->d_inode->i_size; /* append */
        old_fs = get_fs();
        set_fs(KERNEL_DS);

	/* here we go... */
        list_for_each(tmp, &inode_info->deleted_list) {
		del_entry = list_entry(tmp, struct deleted_entry, list);

		/* size: len for name, 1 for \n and 2 for "D " */
		buf = (char *) kmalloc(del_entry->len+3, GFP_KERNEL);
		if (!buf) {
			printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                          out of mem.\n");
			return -ENOMEM;
		}

		buf[0] = 'D';
		buf[1] = ' ';
		strncpy(buf+2, del_entry->name, del_entry->len);
		buf[del_entry->len+2] = '\n';
		bytes = meta_file->f_op->write(meta_file, buf,
					       del_entry->len+3,
					       &meta_file->f_pos);
		if(bytes != del_entry->len+3) {
			printk(KERN_CRIT "mini_fo: meta_sync_d_list: \
                                          ERROR writing.\n");
			err |= -1;
		}
		kfree(buf);
	}
	set_fs(old_fs);

 out_err_close:
	fput(meta_file);
 out:
	return err;

}

int meta_sync_r_list(dentry_t *dentry, int app_flag)
{
	dentry_t *meta_dentry;
        file_t *meta_file;
        mm_segment_t old_fs;

        int bytes, err, buf_len;
        struct vfsmount *meta_mnt;
        char *buf;

	struct list_head *tmp;
        struct renamed_entry *ren_entry;
        struct mini_fo_inode_info *inode_info;

	err = 0;
	meta_file=0;
	meta_mnt=0;

	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                  invalid dentry passed.\n");
		err = -1;
		goto out;
	}
	inode_info = itopd(dentry->d_inode);

        if(inode_info->deleted_list_size < 0) {
		err = -1;
		goto out;
	}

	/* ok, there is something to sync */

	/* build the storage structure? */
        if(!dtohd2(dentry) && !itohi2(dentry->d_inode)) {
                err = build_sto_structure(dentry->d_parent, dentry);
                if(err) {
                        printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                          build_sto_structure failed.\n");
			goto out;
                }
        }

	mutex_lock(&dtohd2(dentry)->d_inode->i_mutex);
	meta_dentry = lookup_one_len(META_FILENAME,
				     dtohd2(dentry),
				     strlen(META_FILENAME));
	mutex_unlock(&dtohd2(dentry)->d_inode->i_mutex);

        if(!meta_dentry->d_inode) {
                /* We need to create a META-file */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry, S_IRUSR | S_IWUSR, NULL);
#else
                vfs_create(dtohd2(dentry)->d_inode,
			   meta_dentry, S_IRUSR | S_IWUSR);
#endif
		app_flag = 0;
	}
	/* need we truncate the meta file? */
	if(!app_flag) {
		struct iattr newattrs;
                newattrs.ia_size = 0;
                newattrs.ia_valid = ATTR_SIZE | ATTR_CTIME;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
		mutex_lock(&meta_dentry->d_inode->i_mutex);
#else
                down(&meta_dentry->d_inode->i_sem);
#endif
                err = notify_change(meta_dentry, &newattrs);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
		mutex_unlock(&meta_dentry->d_inode->i_mutex);
#else
                up(&meta_dentry->d_inode->i_sem);
#endif
                if(err || meta_dentry->d_inode->i_size != 0) {
                        printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                          ERROR truncating meta file.\n");
                        goto out_err_close;
		}
	}

	/* $%& err, is this correct? */
	meta_mnt = stopd(dentry->d_inode->i_sb)->hidden_mnt2;
	mntget(meta_mnt);

        /* open META-file for writing */
        meta_file = dentry_open(meta_dentry, meta_mnt, 0x1, current_cred());
        if(!meta_file || IS_ERR(meta_file)) {
                printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                  ERROR opening meta file.\n");
		mntput(meta_mnt);
		dput(meta_dentry);
		err = -1;
                goto out;
        }

        /* check if fs supports writing */
        if(!meta_file->f_op->write) {
                printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                  ERROR, fs does not support writing.\n");
                goto out_err_close;
        }

	meta_file->f_pos = meta_dentry->d_inode->i_size; /* append */
        old_fs = get_fs();
        set_fs(KERNEL_DS);

	/* here we go... */
        list_for_each(tmp, &inode_info->renamed_list) {
		ren_entry = list_entry(tmp, struct renamed_entry, list);
		/* size:
		 * 2 for "R ", old_len+new_len for names, 1 blank+1 \n */
		buf_len = ren_entry->old_len + ren_entry->new_len + 4;
		buf = (char *) kmalloc(buf_len, GFP_KERNEL);
		if (!buf) {
			printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                          out of mem.\n");
			return -ENOMEM;
		}
		buf[0] = 'R';
		buf[1] = ' ';
		strncpy(buf + 2, ren_entry->old_name, ren_entry->old_len);
		buf[ren_entry->old_len + 2] = ' ';
		strncpy(buf + ren_entry->old_len + 3,
			ren_entry->new_name, ren_entry->new_len);
		buf[buf_len - 1] = '\n';
		bytes = meta_file->f_op->write(meta_file, buf,
					       buf_len, &meta_file->f_pos);
		if(bytes != buf_len) {
			printk(KERN_CRIT "mini_fo: meta_sync_r_list: \
                                          ERROR writing.\n");
			err |= -1;
		}
		kfree(buf);
	}
	set_fs(old_fs);

 out_err_close:
	fput(meta_file);
 out:
	return err;
}

int meta_check_d_entry(dentry_t *dentry, const char *name, int len)
{
	if(!dentry || !dentry->d_inode)
		printk(KERN_CRIT "mini_fo: meta_check_d_dentry: \
                                  invalid dentry passed.\n");
	return __meta_check_d_entry(dentry->d_inode, name, len);
}

int __meta_check_d_entry(inode_t *inode, const char *name, int len)
{
	struct list_head *tmp;
        struct deleted_entry *del_entry;
        struct mini_fo_inode_info *inode_info;

	if(!inode || !itopd(inode))
		printk(KERN_CRIT "mini_fo: __meta_check_d_dentry: \
                                  invalid inode passed.\n");

        inode_info = itopd(inode);

        if(inode_info->deleted_list_size <= 0)
                return 0;

        list_for_each(tmp, &inode_info->deleted_list) {
		del_entry = list_entry(tmp, struct deleted_entry, list);
		if(del_entry->len != len)
			continue;

		if(!strncmp(del_entry->name, name, len))
			return 1;
	}
	return 0;
}

/*
 * check if file has been renamed and return path to orig. base dir.
 * Implements no error return values so far, what of course sucks.
 * String is null terminated.'
 */
char* meta_check_r_entry(dentry_t *dentry, const char *name, int len)
{
	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT "mini_fo: meta_check_r_dentry: \
                                  invalid dentry passed.\n");
		return NULL;
	}
	return __meta_check_r_entry(dentry->d_inode, name, len);
}

char* __meta_check_r_entry(inode_t *inode, const char *name, int len)
{
	struct list_head *tmp;
        struct renamed_entry *ren_entry;
        struct mini_fo_inode_info *inode_info;
	char *old_path;

	if(!inode || !itopd(inode)) {
		printk(KERN_CRIT "mini_fo: meta_check_r_dentry: \
                                  invalid inode passed.\n");
		return NULL;
	}
	inode_info = itopd(inode);

        if(inode_info->renamed_list_size <= 0)
                return NULL;

        list_for_each(tmp, &inode_info->renamed_list) {
		ren_entry = list_entry(tmp, struct renamed_entry, list);
		if(ren_entry->new_len != len)
			continue;

		if(!strncmp(ren_entry->new_name, name, len)) {
			old_path = (char *)
				kmalloc(ren_entry->old_len+1, GFP_KERNEL);
			strncpy(old_path,
				ren_entry->old_name,
				ren_entry->old_len);
			old_path[ren_entry->old_len]='\0';
			return old_path;
		}
	}
	return NULL;
}

/*
 * This version only checks if entry exists and return:
 *     1 if exists,
 *     0 if not,
 *    -1 if error.
 */
int meta_is_r_entry(dentry_t *dentry, const char *name, int len)
{
	if(!dentry || !dentry->d_inode) {
		printk(KERN_CRIT "mini_fo: meta_check_r_dentry [2]: \
                                  invalid dentry passed.\n");
		return -1;
	}
	return __meta_is_r_entry(dentry->d_inode, name, len);
}

int __meta_is_r_entry(inode_t *inode, const char *name, int len)
{
	struct list_head *tmp;
        struct renamed_entry *ren_entry;
        struct mini_fo_inode_info *inode_info;

	if(!inode || !itopd(inode)) {
		printk(KERN_CRIT "mini_fo: meta_check_r_dentry [2]: \
                                  invalid inode passed.\n");
		return -1;
	}
	inode_info = itopd(inode);

        if(inode_info->renamed_list_size <= 0)
                return -1;

        list_for_each(tmp, &inode_info->renamed_list) {
		ren_entry = list_entry(tmp, struct renamed_entry, list);
		if(ren_entry->new_len != len)
			continue;

		if(!strncmp(ren_entry->new_name, name, len))
			return 1;
	}
	return 0;
}

