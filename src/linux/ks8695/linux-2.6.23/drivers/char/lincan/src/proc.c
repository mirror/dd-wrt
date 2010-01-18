/* proc.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */ 

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/proc.h"
#include "../include/setup.h"

#define __NO_VERSION__
#include <linux/module.h>

int add_channel_to_procdir(struct candevice_t *candev);
int remove_channel_from_procdir(void);
int add_object_to_procdir(int chip_nr);
int remove_object_from_procdir(int chip_nr);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,3,0))
static int can_proc_readlink(struct proc_dir_entry *ent, char *page);
#endif

static int cc=0; /* static counter for each CAN chip */

static struct canproc_t can_proc_base;
static struct canproc_t *base=&can_proc_base;

/* The following functions are needed only for kernel version 2.2. Kernel
 * version 2.4 already defines them for us.
 */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,3,0))
static void can_fill_inode(struct inode *inode, int fill)
{
	if (fill)
		MOD_INC_USE_COUNT;
	else
		MOD_DEC_USE_COUNT;
}

static struct proc_dir_entry * can_create_proc_entry(const char *name, mode_t mode,
					struct proc_dir_entry *parent)
{
	struct proc_dir_entry *new_entry = NULL;
	char *namestore;
	int namelen;
	
	if(!name || !parent)
		return NULL;
	namelen=strlen(name);
	if(!namelen)
		return NULL;

	new_entry = (struct proc_dir_entry *) 
			can_checked_malloc(sizeof(struct proc_dir_entry)+namelen+1);

	if (new_entry == NULL)
		return NULL;

	memset(new_entry, 0, sizeof(struct proc_dir_entry));

	/* Store copy of the proc entry name */
	namestore = ((char *) new_entry) + sizeof(struct proc_dir_entry);
	memcpy(namestore, name, namelen + 1);

	new_entry->low_ino = 0;
	new_entry->namelen = namelen;
	new_entry->name = namestore;
	new_entry->mode = mode;
	new_entry->nlink = 0;
	new_entry->fill_inode = can_fill_inode;
	new_entry->parent = parent;

	proc_register(parent, new_entry);

	return new_entry;
}

static int can_remove_proc_entry(struct proc_dir_entry *del, struct proc_dir_entry *parent)
{
	if (del != NULL) {
		proc_unregister(parent, del->low_ino);
		can_checked_free(del);
		return 0;
	}
	else return -ENODEV;
}


static int can_proc_readlink(struct proc_dir_entry *ent, char *page)
{
	char *link_dest = (char*)ent->data;
	
	strcpy(page, link_dest);
	return strlen(link_dest);
}



/* This compatibility version of proc_symlink does not store local copy of destination */
static inline struct proc_dir_entry *can_proc_symlink(const char *name,
                struct proc_dir_entry *parent, const char *dest)
{
	struct proc_dir_entry *entry;
	
	
	entry = can_create_proc_entry(name, S_IFLNK | S_IRUGO | S_IWUGO | S_IXUGO, parent);
	if (entry == NULL)
		return NULL;
	entry->readlink_proc = can_proc_readlink;
	entry->data = dest;
	return entry;
}

#else /* Functions forwarded for kernel 2.4 and above */

static inline struct proc_dir_entry * can_create_proc_entry(const char *name, mode_t mode,
					struct proc_dir_entry *parent)
{
	return create_proc_entry(name, mode, parent);
}


/* This does not fully follow linux 2.4 and 2.6 prototype to simplify 2.2.x compatibility */
/* The newer kernels use entry name instead of pointer to the entry */
static int can_remove_proc_entry(struct proc_dir_entry *del, struct proc_dir_entry *parent)
{
	if(!del) return -ENODEV;
	remove_proc_entry(del->name,parent);
	return 0;
}

static inline struct proc_dir_entry *can_proc_symlink(const char *name,
                struct proc_dir_entry *parent, const char *dest)
{
	return proc_symlink(name, parent, dest);
}

#endif /* Functions required for kernel 2.2 */

/* can_init_procdir registers the entire CAN directory tree recursively at
 * the proc system.
 */
int can_init_procdir(void)
{
	int board;
	struct candevice_t *candev;
	base->can_proc_entry = can_create_proc_entry("can", S_IFDIR | S_IRUGO | 
					S_IXUGO, 0);
	if (base->can_proc_entry == NULL)
		return -ENODEV;

	for (board=0; board<hardware_p->nr_boards; board++) {
		candev=hardware_p->candevice[board];
		if(candev) add_channel_to_procdir(candev);
	} 

	return 0;
}

/* can_delete_procdir removes the entire CAN tree from the proc system */
int can_delete_procdir(void)
{
	if (remove_channel_from_procdir()) 
		return -ENODEV;
	/* name: "can" */
	if (can_remove_proc_entry(base->can_proc_entry, 0)) 
		return -ENODEV;

	return 0;
}

static int can_chip_procinfo(char *buf, char **start, off_t offset, 
		 int count, int *eof, void *data)
{
	struct canchip_t *chip=data;
	int len=0;

	/* Generic chip info */
	len += sprintf(buf+len,"type    : %s\n",chip->chip_type);
	len += sprintf(buf+len,"index   : %d\n",chip->chip_idx);
	len += sprintf(buf+len,"irq     : %d\n",chip->chip_irq);
	len += sprintf(buf+len,"addr    : 0x%lx\n",
			can_ioptr2ulong(chip->chip_base_addr));
	len += sprintf(buf+len,"config  : %s\n",
			(chip->flags & CHIP_CONFIGURED) ? "yes":"no");
	len += sprintf(buf+len,"clock   : %ld Hz\n",chip->clock);
	len += sprintf(buf+len,"baud    : %ld\n",chip->baudrate);
	len += sprintf(buf+len,"num obj : %d\n",chip->max_objects);


#if 0
	/* Chip specific info if available */
	if(chip->chipspecops->get_info)
		len += (chip->chipspecops->get_info)(chip,buf+len);
#endif

	*eof = 1;
	return len;
}


int add_channel_to_procdir(struct candevice_t *candev)
{
	int i=0;

	for (i=0; i < candev->nr_all_chips; i++) {

		base->channel[cc] = (struct channelproc_t *)
			can_checked_malloc(sizeof(struct channelproc_t));
		if (base->channel[cc] == NULL)
			return -ENOMEM;

		sprintf(base->channel[cc]->ch_name, "channel%d",cc);
						
		base->channel[cc]->ch_entry = can_create_proc_entry(
						base->channel[cc]->ch_name,
						S_IFDIR | S_IRUGO |S_IXUGO,
						base->can_proc_entry);

		if (base->channel[cc]->ch_entry == NULL)
			return -ENODEV;

		add_object_to_procdir(cc);

		create_proc_read_entry("chip_info",        /* proc entry name */
				       0,                  /* protection mask, 0->default */
				       base->channel[cc]->ch_entry,  /* parent dir, NULL->/proc */
				       can_chip_procinfo,
				       candev->chip[i]);

		cc++;
	} 

	return 0;
}

int remove_channel_from_procdir(void)
{
	
	while (cc != 0) {
		cc--;
		
		if(!base->channel[cc]) continue;

		remove_proc_entry("chip_info", base->channel[cc]->ch_entry);
		
		if (remove_object_from_procdir(cc))
			return -ENODEV; 
			
		/* name: base->channel[cc]->ch_name */
		if (can_remove_proc_entry(base->channel[cc]->ch_entry,
							base->can_proc_entry))
			return -ENODEV;
			
		can_checked_free(base->channel[cc]);
		base->channel[cc] = NULL;
	}

	return 0;
}


int add_object_to_procdir(int chip_nr)
{
	int i, max_objects;

	max_objects=chips_p[chip_nr]->max_objects;

	for (i=0; i<max_objects; i++) {
		base->channel[chip_nr]->object[i] = (struct objectproc_t *)
			can_checked_malloc(sizeof(struct objectproc_t));

		if (base->channel[chip_nr]->object[i] == NULL)
			return -ENOMEM;

		sprintf(base->channel[chip_nr]->object[i]->obj_name,"object%d",i);
		sprintf(base->channel[chip_nr]->object[i]->lnk_name,"dev");
								
		base->channel[chip_nr]->object[i]->obj_entry = can_create_proc_entry(
				base->channel[chip_nr]->object[i]->obj_name,
				S_IFDIR | S_IRUGO | S_IXUGO,
				base->channel[chip_nr]->ch_entry);
		if (base->channel[chip_nr]->object[i]->obj_entry == NULL)
			return -ENODEV;

		sprintf(base->channel[chip_nr]->object[i]->lnk_dev,"/dev/can%d",
			chips_p[chip_nr]->msgobj[i]->minor);

		base->channel[chip_nr]->object[i]->lnk = can_proc_symlink(
				base->channel[chip_nr]->object[i]->lnk_name,
				base->channel[chip_nr]->object[i]->obj_entry,
				base->channel[chip_nr]->object[i]->lnk_dev);
		if (base->channel[chip_nr]->object[i]->lnk == NULL)
			return -ENODEV;

	}
	return 0;
} 

int remove_object_from_procdir(int chip_nr)
{
	int i=0, obj=0;

	obj=chips_p[chip_nr]->max_objects;

	for (i=0; i<obj; i++) {
		if(!base->channel[chip_nr]->object[i]) continue;
		
		/* name: base->channel[chip_nr]->object[i]->lnk_name */
		if (can_remove_proc_entry( base->channel[chip_nr]->object[i]->lnk,
				base->channel[chip_nr]->object[i]->obj_entry))	
			return -ENODEV;
		/* name: base->channel[chip_nr]->object[i]->obj_name */
		if (can_remove_proc_entry(
				base->channel[chip_nr]->object[i]->obj_entry,
				base->channel[chip_nr]->ch_entry))
			return -ENODEV;

		can_checked_free(base->channel[chip_nr]->object[i]);

		base->channel[chip_nr]->object[i]=NULL;
	}
	return 0;
}

