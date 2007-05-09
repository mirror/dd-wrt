/*
 * JFFS2-BBC: File System Extension for Linux Kernel
 *
 * $Id: 301-jffs-compression,v 1.1 2005/03/26 10:33:31 wbx Exp $
 *
 * Copyright (C) 2004, Ferenc Havasi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/jffs2.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#include "nodelist.h"

#include "jffs2_bbc_framework.h"

struct jffs2_bbc_fs_sb_list {
	struct super_block *sb;
	struct jffs2_bbc_fs_sb_list *next;
};

static struct jffs2_bbc_fs_sb_list *sb_list = NULL;

void jffs2_bbc_proc_init(void);
void jffs2_bbc_proc_deinit(void);

void jffs2_bbc_load_model(void *sb_par) {
	struct jffs2_sb_info *c;
	//struct jffs2_inode_info *f;
	struct dentry *config_dentry,*model_dentry;
	struct qstr config_name,model_name;
	struct file *config_file,*model_file;
	char *buff=NULL,*model_buff;
	int config_size,model_size;
	int i,prev_i;
	struct super_block *sb;
	struct jffs2_bbc_fs_sb_list *sb_l;
	
	sb = sb_par;
	sb_l = jffs2_bbc_malloc_small(sizeof(struct jffs2_bbc_fs_sb_list));
	sb_l->sb = sb;
	sb_l->next = sb_list;
	sb_list = sb_l;	
	config_name.name = JFFS2_BBC_CONFIG_FILE;
        config_name.len = strlen(config_name.name);
	config_name.hash = full_name_hash(config_name.name,config_name.len);
        config_dentry = d_alloc(sb->s_root,&config_name);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)	
        sb->s_root->d_inode->i_op->lookup(sb->s_root->d_inode,config_dentry);
#else	
        sb->s_root->d_inode->i_op->lookup(sb->s_root->d_inode,config_dentry,NULL);
#endif
	
	if (config_dentry->d_inode != NULL) {
	        config_size = config_dentry->d_inode->i_size;
		//printk("config_file_size=%d\n",config_size);
		if (config_size > 0) {
		        buff = jffs2_bbc_malloc(config_size+1);
			config_file = dentry_open(config_dentry,NULL,O_RDONLY);
    			kernel_read(config_file,0,buff,config_size);
			buff[config_size] = 0;
			for (prev_i = i = 0 ; i < config_size+1 ; i++) {
				if (buff[i] == '\n') buff[i]=0;
				if (buff[i] == 0) {
					if (prev_i != i) {
						if ((buff[prev_i] == '-') && (buff[prev_i+1] == 0)) break;
						printk("reading model file %s... ",buff+prev_i);
						model_name.name = buff+prev_i;
						model_name.len = strlen(buff+prev_i);
			    			model_name.hash = full_name_hash(model_name.name,model_name.len);
						model_dentry = d_alloc(sb->s_root,&model_name);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
						sb->s_root->d_inode->i_op->lookup(sb->s_root->d_inode,model_dentry);
#else						
						sb->s_root->d_inode->i_op->lookup(sb->s_root->d_inode,model_dentry,NULL);
#endif
						if (model_dentry->d_inode != NULL) {
							c = JFFS2_SB_INFO(model_dentry->d_inode->i_sb);
							//f = JFFS2_INODE_INFO(model_dentry->d_inode);
							model_size = model_dentry->d_inode->i_size;
							model_buff = jffs2_bbc_malloc(model_size);
							model_file = dentry_open(model_dentry,NULL,O_RDONLY);
    							kernel_read(model_file,0,model_buff,model_size);
    							if (jffs2_bbc_model_new(c,model_dentry->d_inode->i_ino,model_buff) != 0) {
        							printk("already loaded.\n");
								jffs2_bbc_free(model_buff);
    							}
							else {
								printk("done (%d bytes readed from inode %d).\n",model_size,(int)(model_dentry->d_inode->i_ino));
							}
						}
						else {
							printk("not found.\n");
						}
						dput(model_dentry);
					}
    					prev_i = i+1;
				}
			}
		}
	}
	dput(config_dentry);
	if (buff != NULL) jffs2_bbc_free(buff);
}

void jffs2_bbc_unload_model(void *sb_par)
{
	struct jffs2_sb_info *c;
	struct super_block *sb = sb_par;
	struct jffs2_bbc_fs_sb_list *sb_l,*sb_l2;
	int done = 0;

	c = JFFS2_SB_INFO(sb);
	jffs2_bbc_model_del(c);
	if (sb_list == NULL) printk("jffs2.bbc: error! NULL sb list!\n");
	else {
		if (sb_list->sb == sb) {
			jffs2_bbc_free_small(sb_list);
			sb_list = NULL;
			done = 1;
		}
		else {
			sb_l = sb_list;
			while (sb_l->next != NULL) {
				if (sb_l->next->sb == sb) {
					sb_l2 = sb_l->next->next;
					jffs2_bbc_free_small(sb_l->next);
					sb_l->next = sb_l2;
					done = 1;
				}
				sb_l = sb_l->next;
			}
			
		}
		if (done == 0) {
			printk("jffs2.bbc: cannot delete sb from sblist!\n");
		}
	}
}

static int jffs2_bbc_get_mounted(void) {
	struct jffs2_bbc_fs_sb_list *sb_l;
	int num = 0;
	
	sb_l = sb_list;
	while (sb_l != NULL) {
		num++;
		sb_l = sb_l->next;
	}
	return num;
	
}

int jffs2_bbc_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int len = 0, mode;

	mode = jffs2_bbc_get_compression_mode();
	len += sprintf(buf + len, "BBC version: %s\n", JFFS2_BBC_VERSION);
	len += sprintf(buf+len,"Mounted jffs2 filesystems: %d\n",jffs2_bbc_get_mounted());
	//len += sprintf(buf+len,"actual model file inode: %d\n",jffs2_bbc_model_get_inum());
	len += sprintf(buf + len, "Compression mode: ");
	if (mode == JFFS2_BBC_ZLIB_MODE)
		len += sprintf(buf + len, "ZLIB mode");
	else if (mode == JFFS2_BBC_SIZE_MODE)
		len += sprintf(buf + len, "SIZE mode");
	else if (mode == JFFS2_BBC_FASTR_MODE)
		len += sprintf(buf + len, "FASTR mode");
	else if (mode == JFFS2_BBC_FASTW_MODE)
		len += sprintf(buf + len, "FASTW mode");
	else if (mode == JFFS2_BBC_FASTS_MODE)
		len += sprintf(buf + len, "FASTS mode");
	else if (mode == JFFS2_BBC_DUMMY_MODE)
		len += sprintf(buf + len, "DUMMY mode");
	else if (mode == JFFS2_BBC_MANUAL_MODE) {
		len += sprintf(buf + len, "MANUAL mode (");
		if (jffs2_bbc_get_manual_compressor() != NULL)
			len += sprintf(buf + len, "%s)", jffs2_bbc_get_manual_compressor()->name);
		else
			len += sprintf(buf + len, "ZLIB)");
	}
	else
		len += sprintf(buf + len, "unknown mode");
	len += sprintf(buf + len, "\n");
	len += sprintf(buf + len, "%s", jffs2_bbc_get_compr_stats());
	len += sprintf(buf + len, "%s", jffs2_bbc_get_model_stats());
	len += sprintf(buf + len, "%s", jffs2_bbc_get_mem_stats());
	*eof = 1;
	return len;
}

int jffs2_bbc_proc_write(struct file *file, const char *buffer_orig, unsigned long count, void *data)
{
	char *buffer;
	int i;
	struct jffs2_bbc_fs_sb_list *sb_l;
	
	if (buffer_orig == NULL) return 0;
	
	buffer = jffs2_bbc_malloc(count+2);
	for (i=0;i<count;i++) buffer[i]=buffer_orig[i];
	buffer[count] = 0;
	if ((*buffer == 'z') || (*buffer == 'Z')) {
		jffs2_bbc_set_compression_mode(JFFS2_BBC_ZLIB_MODE);
		jffs2_bbc_print1("jffs2.bbc: ZLIB compression mode activated.\n");
		jffs2_bbc_free(buffer);
		return count;
	}
	else if ((*buffer == 's') || (*buffer == 'S')) {
		jffs2_bbc_set_compression_mode(JFFS2_BBC_SIZE_MODE);
		jffs2_bbc_print1("jffs2.bbc: SIZE compression mode activated.\n");
		jffs2_bbc_free(buffer);
		return count;
	}
	else if ((*buffer == 'd') || (*buffer == 'D')) {
		jffs2_bbc_set_compression_mode(JFFS2_BBC_DUMMY_MODE);
		jffs2_bbc_print1("jffs2.bbc: DUMMY compression mode activated.\n");
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((*buffer == 'm') || (*buffer == 'M')) && (count >= 3) && (buffer[1] == ':')) {
		jffs2_bbc_print1("jffs2.bbc: activating MANUAL mode.\n");
		jffs2_bbc_set_manual_compressor_by_name(buffer + 2);
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((*buffer == '0')) && (count >= 3) && (buffer[1] == ':')) {
		jffs2_bbc_print1("jffs2.bbc: disabling a compressor... ");
		if (jffs2_bbc_disable_compressor_by_name(buffer + 2) == 0) {
			jffs2_bbc_print1("done.\n");
		}
		else {
			jffs2_bbc_print1("not found.\n");
		}
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((*buffer == '1')) && (count >= 3) && (buffer[1] == ':')) {
		jffs2_bbc_print1("jffs2.bbc: enabling a compressor... ");
		if (jffs2_bbc_enable_compressor_by_name(buffer + 2) == 0) {
			jffs2_bbc_print1("done.\n");
		}
		else {
			jffs2_bbc_print1("not found.\n");
		}
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((*buffer == 'c') || (*buffer == 'C')) && (count >= 3) && (buffer[1] == ':')) {
		jffs2_bbc_compressor_command_by_name(buffer + 2);
		jffs2_bbc_free(buffer);
		return count;
	}
	else if ((*buffer == 'r') || (*buffer == 'R')) {
		jffs2_bbc_print1("jffs2.bbc: reloading model files:\n");
		sb_l = sb_list;
		while (sb_l != NULL) {
			jffs2_bbc_unload_model(sb_l->sb);
			jffs2_bbc_load_model(sb_l->sb);
			sb_l = sb_l->next;
		}
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((buffer[0] == 'f') || (buffer[0] == 'F'))&&((buffer[1] == 'r') || (buffer[1] == 'R'))) {
		jffs2_bbc_set_compression_mode(JFFS2_BBC_FASTR_MODE);
		jffs2_bbc_print1("jffs2.bbc: FASTR compression mode activated.\n");
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((buffer[0] == 'f') || (buffer[0] == 'F'))&&((buffer[1] == 'w') || (buffer[1] == 'W'))) {
		jffs2_bbc_set_compression_mode(JFFS2_BBC_FASTW_MODE);
		jffs2_bbc_print1("jffs2.bbc: FASTW compression mode activated.\n");
		jffs2_bbc_free(buffer);
		return count;
	}
	else if (((buffer[0] == 'f') || (buffer[0] == 'F'))&&((buffer[1] == 's') || (buffer[1] == 'S'))) {
		jffs2_bbc_set_compression_mode(JFFS2_BBC_FASTS_MODE);
		jffs2_bbc_print1("jffs2.bbc: FASTS compression mode activated.\n");
		jffs2_bbc_free(buffer);
		return count;
	}
	
	jffs2_bbc_print1("jffs2.bbc: unkown command. Valid commands are:\n" 
	                 "  z                         = switch to ZLIB compression mode\n" 
			 "  s                         = switch to SIZE compression mode\n" 
			 "  d                         = switch to DUMMY compression mode\n" 
			 "  fr                        = switch to FASTR compression mode\n" 
			 "  fw                        = switch to FASTW compression mode\n" 
			 "  fs                        = switch to FASTS compression mode\n" 
			 "  r                         = reread model files from actual file system\n" 
			 "  m:compressor_name         = switch to MANUAL compression mode\n" 
			 "  0:compressor_name         = disable a compressor\n" 
			 "  1:compressor_name         = enable a compressor\n" 
			 "  c:compressor_name:command = enable a compressor\n");
	jffs2_bbc_free(buffer);
	return count;
}

void jffs2_bbc_proc_init()
{
	struct proc_dir_entry *res = create_proc_entry("jffs2_bbc", 0, NULL);
	jffs2_bbc_compressor_init();
	if (res) {
		res->read_proc = jffs2_bbc_proc_read;
		res->write_proc = jffs2_bbc_proc_write;
	}
}

void jffs2_bbc_proc_deinit()
{
	jffs2_bbc_compressor_deinit();
	remove_proc_entry("jffs2_bbc", NULL);
}
