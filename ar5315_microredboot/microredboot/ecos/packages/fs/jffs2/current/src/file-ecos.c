/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001, 2002 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@cambridge.redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/fs/jffs2/current/src/file-ecos.c#3 $
 *
 */

#include <linux/kernel.h>
#include <linux/pagemap.h>
#include <linux/crc32.h>
#include "nodelist.h"

int jffs2_do_readpage_nolock (struct inode *inode, struct page *pg)
{
	struct jffs2_inode_info *f = JFFS2_INODE_INFO(inode);
	struct jffs2_sb_info *c = JFFS2_SB_INFO(inode->i_sb);
	unsigned char *pg_buf;
	int ret;

	D1(printk(KERN_DEBUG "jffs2_do_readpage_nolock(): ino #%lu, page at offset 0x%lx\n", inode->i_ino, pg->index << PAGE_CACHE_SHIFT));

	if (!PageLocked(pg))
                PAGE_BUG(pg);

	pg_buf = (char *)kmap(pg);
	/* FIXME: Can kmap fail? */

	ret = jffs2_read_inode_range(c, f, pg_buf, pg->index << PAGE_CACHE_SHIFT, PAGE_CACHE_SIZE);

	if (ret) {
		ClearPageUptodate(pg);
		SetPageError(pg);
	} else {
		SetPageUptodate(pg);
		ClearPageError(pg);
	}

	flush_dcache_page(pg);
	kunmap(pg);

	D1(printk(KERN_DEBUG "readpage finished\n"));
	return 0;
}

int jffs2_do_readpage_unlock(struct inode *inode, struct page *pg)
{
	int ret = jffs2_do_readpage_nolock(inode, pg);
	UnlockPage(pg);
	return ret;
}


//int jffs2_readpage (struct file *filp, struct page *pg)
int jffs2_readpage (struct inode *d_inode, struct page *pg)
{
	//	struct jffs2_inode_info *f = JFFS2_INODE_INFO(d_inode);
	int ret;
	
	//	down(&f->sem);
	ret = jffs2_do_readpage_unlock(d_inode, pg);
	//	up(&f->sem);
	return ret;
}

//int jffs2_prepare_write (struct file *filp, struct page *pg, unsigned start, unsigned end)
int jffs2_prepare_write (struct inode *d_inode, struct page *pg, unsigned start, unsigned end)
{
	struct inode *inode = d_inode;
	struct jffs2_inode_info *f = JFFS2_INODE_INFO(inode);
	uint32_t pageofs = pg->index << PAGE_CACHE_SHIFT;
	int ret = 0;

	D1(printk(KERN_DEBUG "jffs2_prepare_write()\n"));

	if (pageofs > inode->i_size) {
		/* Make new hole frag from old EOF to new page */
		struct jffs2_sb_info *c = JFFS2_SB_INFO(inode->i_sb);
		struct jffs2_raw_inode ri;
		struct jffs2_full_dnode *fn;
		uint32_t phys_ofs, alloc_len;
		
		D1(printk(KERN_DEBUG "Writing new hole frag 0x%x-0x%x between current EOF and new page\n",
			  (unsigned int)inode->i_size, pageofs));

		ret = jffs2_reserve_space(c, sizeof(ri), &phys_ofs, &alloc_len, ALLOC_NORMAL);
		if (ret)
			return ret;

		down(&f->sem);
		memset(&ri, 0, sizeof(ri));

		ri.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
		ri.nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);
		ri.totlen = cpu_to_je32(sizeof(ri));
		ri.hdr_crc = cpu_to_je32(crc32(0, &ri, sizeof(struct jffs2_unknown_node)-4));

		ri.ino = cpu_to_je32(f->inocache->ino);
		ri.version = cpu_to_je32(++f->highest_version);
		ri.mode = cpu_to_jemode(inode->i_mode);
		ri.uid = cpu_to_je16(inode->i_uid);
		ri.gid = cpu_to_je16(inode->i_gid);

		ri.isize = cpu_to_je32(max((uint32_t)inode->i_size, pageofs));
		ri.atime = ri.ctime = ri.mtime = cpu_to_je32(cyg_timestamp());
		ri.offset = cpu_to_je32(inode->i_size);
		ri.dsize = cpu_to_je32(pageofs - inode->i_size);
		ri.csize = cpu_to_je32(0);
		ri.compr = JFFS2_COMPR_ZERO;
		ri.node_crc = cpu_to_je32(crc32(0, &ri, sizeof(ri)-8));
		ri.data_crc = cpu_to_je32(0);
		
		fn = jffs2_write_dnode(c, f, &ri, NULL, 0, phys_ofs, ALLOC_NORMAL);
		jffs2_complete_reservation(c);
		if (IS_ERR(fn)) {
			ret = PTR_ERR(fn);
			up(&f->sem);
			return ret;
		}
		ret = jffs2_add_full_dnode_to_inode(c, f, fn);
		if (f->metadata) {
			jffs2_mark_node_obsolete(c, f->metadata->raw);
			jffs2_free_full_dnode(f->metadata);
			f->metadata = NULL;
		}
		if (ret) {
			D1(printk(KERN_DEBUG "Eep. add_full_dnode_to_inode() failed in prepare_write, returned %d\n", ret));
			jffs2_mark_node_obsolete(c, fn->raw);
			jffs2_free_full_dnode(fn);
			up(&f->sem);
			return ret;
		}
		inode->i_size = pageofs;
		up(&f->sem);
	}
	

	/* Read in the page if it wasn't already present, unless it's a whole page */
        // eCos has no concept of uptodate and by default always reads pages afresh
	if (!Page_Uptodate(pg) && (start || end < PAGE_CACHE_SIZE)) {
		down(&f->sem);
		ret = jffs2_do_readpage_nolock(inode, pg);
		up(&f->sem);
	}
	D1(printk(KERN_DEBUG "end prepare_write()\n"));
	return ret;
}

//int jffs2_commit_write (struct file *filp, struct page *pg, unsigned start, unsigned end)
int jffs2_commit_write (struct inode *d_inode, struct page *pg, unsigned start, unsigned end)
{
	/* Actually commit the write from the page cache page we're looking at.
	 * For now, we write the full page out each time. It sucks, but it's simple
	 */
	struct inode *inode = d_inode;
	struct jffs2_inode_info *f = JFFS2_INODE_INFO(inode);
	struct jffs2_sb_info *c = JFFS2_SB_INFO(inode->i_sb);
	struct jffs2_raw_inode *ri;
	int ret = 0;
	uint32_t writtenlen = 0;

	D1(printk(KERN_DEBUG "jffs2_commit_write(): ino #%lu, page at 0x%lx, range %d-%d\n",
		  inode->i_ino, pg->index << PAGE_CACHE_SHIFT, start, end));


	ri = jffs2_alloc_raw_inode();
	if (!ri) {
		D1(printk(KERN_DEBUG "jffs2_commit_write(): Allocation of raw inode failed\n"));
		return -ENOMEM;
	}

	/* Set the fields that the generic jffs2_write_inode_range() code can't find */
	ri->ino = cpu_to_je32(inode->i_ino);
	ri->mode = cpu_to_jemode(inode->i_mode);
	ri->uid = cpu_to_je16(inode->i_uid);
	ri->gid = cpu_to_je16(inode->i_gid);
	ri->isize = cpu_to_je32((uint32_t)inode->i_size);
	ri->atime = ri->ctime = ri->mtime = cpu_to_je32(cyg_timestamp());

	ret = jffs2_write_inode_range(c, f, ri, page_address(pg) + start,
				      (pg->index << PAGE_CACHE_SHIFT) + start,
				      end - start, &writtenlen);

	if (ret) {
		/* There was an error writing. */
		SetPageError(pg);
	}

	if (writtenlen) {
		if (inode->i_size < (pg->index << PAGE_CACHE_SHIFT) + start + writtenlen) {
			inode->i_size = (pg->index << PAGE_CACHE_SHIFT) + start + writtenlen;
			inode->i_ctime = inode->i_mtime = je32_to_cpu(ri->ctime);
		}
	}

	jffs2_free_raw_inode(ri);

	if (start+writtenlen < end) {
		/* generic_file_write has written more to the page cache than we've
		   actually written to the medium. Mark the page !Uptodate so that 
		   it gets reread */
		D1(printk(KERN_DEBUG "jffs2_commit_write(): Not all bytes written. Marking page !uptodate\n"));
		SetPageError(pg);
		ClearPageUptodate(pg);
	}

	D1(printk(KERN_DEBUG "jffs2_commit_write() returning %d\n",writtenlen?writtenlen:ret));
	return writtenlen?writtenlen:ret;
}
