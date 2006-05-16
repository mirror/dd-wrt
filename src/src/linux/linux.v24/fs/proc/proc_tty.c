/*
 * proc_tty.c -- handles /proc/tty
 *
 * Copyright 1997, Theodore Ts'o
 */

#include <asm/uaccess.h>

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/tty.h>
#include <asm/bitops.h>

extern struct tty_driver *tty_drivers;	/* linked list of tty drivers */

static int tty_drivers_read_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data);
static int tty_ldiscs_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data);

/*
 * The /proc/tty directory inodes...
 */
static struct proc_dir_entry *proc_tty_ldisc, *proc_tty_driver;

/*
 * This is the handler for /proc/tty/drivers
 */
static int tty_drivers_read_proc(char *page, char **start, off_t off,
				 int count, int *eof, void *data)
{
	int	len = 0;
	off_t	begin = 0;
	struct tty_driver *p;
	char	range[20], deftype[20];
	char	*type;

	for (p = tty_drivers; p; p = p->next) {
		if (p->num > 1)
			sprintf(range, "%d-%d", p->minor_start,
				p->minor_start + p->num - 1);
		else
			sprintf(range, "%d", p->minor_start);
		switch (p->type) {
		case TTY_DRIVER_TYPE_SYSTEM:
			if (p->subtype == SYSTEM_TYPE_TTY)
				type = "system:/dev/tty";
			else if (p->subtype == SYSTEM_TYPE_SYSCONS)
				type = "system:console";
			else if (p->subtype == SYSTEM_TYPE_CONSOLE)
				type = "system:vtmaster";
			else
				type = "system";
			break;
		case TTY_DRIVER_TYPE_CONSOLE:
			type = "console";
			break;
		case TTY_DRIVER_TYPE_SERIAL:
			if (p->subtype == 2)
				type = "serial:callout";
			else
				type = "serial";
			break;
		case TTY_DRIVER_TYPE_PTY:
			if (p->subtype == PTY_TYPE_MASTER)
				type = "pty:master";
			else if (p->subtype == PTY_TYPE_SLAVE)
				type = "pty:slave";
			else
				type = "pty";
			break;
		default:
			sprintf(deftype, "type:%d.%d", p->type, p->subtype);
			type = deftype;
			break;
		}
		len += sprintf(page+len, "%-20s /dev/%-8s %3d %7s %s\n",
			       p->driver_name ? p->driver_name : "unknown",
			       p->name, p->major, range, type);
		if (len+begin > off+count)
			break;
		if (len+begin < off) {
			begin += len;
			len = 0;
		}
	}
	if (!p)
		*eof = 1;
	if (off >= len+begin)
		return 0;
	*start = page + (off-begin);
	return ((count < begin+len-off) ? count : begin+len-off);
}

/*
 * This is the handler for /proc/tty/ldiscs
 */
static int tty_ldiscs_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int	i;
	int	len = 0;
	off_t	begin = 0;
	struct tty_ldisc *ld;
	
	for (i=0; i < NR_LDISCS; i++) {
		ld = tty_ldisc_get(i);
		if (ld == NULL)
			continue;
		len += sprintf(page+len, "%-10s %2d\n",
			       ld->name ? ld->name : "???", i);
		tty_ldisc_put(i);
		if (len+begin > off+count)
			break;
		if (len+begin < off) {
			begin += len;
			len = 0;
		}
	}
	if (i >= NR_LDISCS)
		*eof = 1;
	if (off >= len+begin)
		return 0;
	*start = page + (off-begin);
	return ((count < begin+len-off) ? count : begin+len-off);
}

/*
 * This function is called by tty_register_driver() to handle
 * registering the driver's /proc handler into /proc/tty/driver/<foo>
 */
void proc_tty_register_driver(struct tty_driver *driver)
{
	struct proc_dir_entry *ent;
		
	if ((!driver->read_proc && !driver->write_proc) ||
	    !driver->driver_name ||
	    driver->proc_entry)
		return;

	ent = create_proc_entry(driver->driver_name, 0, proc_tty_driver);
	if (!ent)
		return;
	ent->read_proc = driver->read_proc;
	ent->write_proc = driver->write_proc;
	ent->data = driver;

	driver->proc_entry = ent;
}

/*
 * This function is called by tty_unregister_driver()
 */
void proc_tty_unregister_driver(struct tty_driver *driver)
{
	struct proc_dir_entry *ent;

	ent = driver->proc_entry;
	if (!ent)
		return;
		
	remove_proc_entry(driver->driver_name, proc_tty_driver);
	
	driver->proc_entry = 0;
}

/*
 * Called by proc_root_init() to initialize the /proc/tty subtree
 */
void __init proc_tty_init(void)
{
	if (!proc_mkdir("tty", 0))
		return;
	proc_tty_ldisc = proc_mkdir("tty/ldisc", 0);
	/*
	 * /proc/tty/driver/serial reveals the exact character counts for
	 * serial links which is just too easy to abuse for inferring
	 * password lengths and inter-keystroke timings during password
	 * entry.
	 */
	proc_tty_driver = proc_mkdir_mode("tty/driver", S_IRUSR | S_IXUSR, 0);

	create_proc_read_entry("tty/ldiscs", 0, 0, tty_ldiscs_read_proc,NULL);
	create_proc_read_entry("tty/drivers", 0, 0, tty_drivers_read_proc,NULL);
}
