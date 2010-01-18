/* proc.h
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include <linux/proc_fs.h>
#include "./constants.h"

int can_init_procdir(void);
int can_delete_procdir(void);

struct canproc_t {
	struct proc_dir_entry *can_proc_entry;
	struct channelproc_t *channel[MAX_TOT_CHIPS];
};

struct channelproc_t {
	char ch_name[20];
	struct proc_dir_entry *ch_entry;
	struct objectproc_t *object[MAX_MSGOBJS];
};


struct objectproc_t {
	char obj_name[20];
	struct proc_dir_entry *obj_entry;
	char lnk_name[20];
	char lnk_dev[20];
	struct proc_dir_entry *lnk;
}; 
