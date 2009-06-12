/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/fs/jffs2/current/src/gcthread.c#3 $
 *
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/jffs2.h>
#include "nodelist.h"


static void jffs2_garbage_collect_thread(struct jffs2_sb_info *c);

void jffs2_garbage_collect_trigger(struct jffs2_sb_info *c)
{
	/* Wake up the thread */
	(void)&jffs2_garbage_collect_thread;
}

void jffs2_start_garbage_collect_thread(struct jffs2_sb_info *c)
{
	/* Start the thread. Doesn't matter if it fails -- it's only an optimisation anyway */
}

void jffs2_stop_garbage_collect_thread(struct jffs2_sb_info *c)
{
	/* Stop the thread and wait for it if necessary */
}


static void jffs2_garbage_collect_thread(struct jffs2_sb_info *c)
{
#define this_thread_should_die() 0
	while(!this_thread_should_die()) {
		while(!jffs2_thread_should_wake(c)) {
			/* Sleep.... */
			continue;
		}
		if (jffs2_garbage_collect_pass(c) == -ENOSPC) {
			printf("No space for garbage collection. Aborting JFFS2 GC thread\n");
			break;
		}
	}
}
