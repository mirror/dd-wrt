
/*
 *          npreal2.c  -- MOXA NPort Server family Real TTY driver.
 *
 *      Copyright (C) 1999-2007  Moxa Technologies (support@moxa.com.tw).
 *
 *      This code is loosely based on the Linux serial driver, written by
 *      Linus Torvalds, Theodore T'so and others.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 */

#ifdef 		MODVERSIONS
#ifndef 	MODULE
#define 	MODULE
#endif
#endif

#include <linux/version.h>
#define VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)
#ifdef MODULE
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,18))
#include <linux/config.h>
#endif
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#else
#define	MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif
#include <linux/init.h>
#include <linux/autoconf.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/smp_lock.h>
#include <linux/proc_fs.h>

#define		NPREAL_VERSION			"1.14 Build 07062310"

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))

#define copy_from_user memcpy_fromfs
#define copy_to_user memcpy_tofs
#define put_to_user(arg1, arg2) put_fs_long(arg1, (unsigned long *)arg2)
#define get_from_user(arg1, arg2) arg1 = get_fs_long((unsigned long *)arg2)
#define schedule_timeout(x) {current->timeout = jiffies + (x); schedule();}
#define signal_pending(x) ((x)->signal & ~(x)->blocked)
#else
#include <asm/uaccess.h>
#include <linux/poll.h>
#define put_to_user(arg1, arg2) put_user(arg1, arg2)
#define get_from_user(arg1, arg2) get_user(arg1, arg2)
#endif

#include "npreal2.h"

#define	NPREAL_EVENT_TXLOW	 1
#define	NPREAL_EVENT_HANGUP	 2

#define SERIAL_DO_RESTART

#define SERIAL_TYPE_NORMAL	1
#define SERIAL_TYPE_CALLOUT	2

#define WAKEUP_CHARS		256

#ifndef MAX_SCHEDULE_TIMEOUT
#define	MAX_SCHEDULE_TIMEOUT	((long)(~0UL>>1))
#endif

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#define PORTNO(x)	(MINOR((x)->device) - (x)->driver.minor_start)
#else
#define PORTNO(x)	((x)->index)
#endif

#define RELEVANT_IFLAG(iflag)	(iflag & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define		NPREALMAJOR		 33
#define		NPREALCUMAJOR	 38

static int ttymajor=NPREALMAJOR;
static int calloutmajor=NPREALCUMAJOR;
static int verbose=1;

int	MXDebugLevel = MX_DEBUG_ERROR;

#ifdef MODULE
/* Variables for insmod */

# if (LINUX_VERSION_CODE > VERSION_CODE(2,1,11))
MODULE_AUTHOR("Moxa Tech.,www.moxa.com.tw");
MODULE_DESCRIPTION("MOXA Async/NPort Server Family Real TTY Driver");
# if (LINUX_VERSION_CODE < VERSION_CODE(2,6,6))
MODULE_PARM(ttymajor,        "i");
MODULE_PARM(calloutmajor,    "i");
MODULE_PARM(verbose,        "i");
# else
module_param(ttymajor, int, 0);
module_param(calloutmajor, int, 0);
module_param(verbose, int, 0644);
MODULE_VERSION(NPREAL_VERSION);
# endif
# if (LINUX_VERSION_CODE > VERSION_CODE(2,4,0))
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
# endif
# endif

#endif /* MODULE */

#define	NPREAL_PORTS	 256

#define	DE211	211
#define	DE311	311
#define	DE301	301
#define	DE302	302
#define	DE304	304
#define	DE331	331
#define	DE332	332
#define	DE334	334
#define	DE303	303
#define	DE308	308
#define	DE309	309
#define	CN2100	2100
#define	CN2500	2500

#ifndef B921600
#define	B921600	(B460800 + 1)
#endif

#define	NPREAL_ASPP_COMMAND_SET		1
#define	NPREAL_LOCAL_COMMAND_SET	2

// local command set
#define	LOCAL_CMD_TTY_USED			1
#define	LOCAL_CMD_TTY_UNUSED		2
#define NPREAL_NET_CONNECTED		3
#define NPREAL_NET_DISCONNECTED		4
#define NPREAL_NET_SETTING			5
#define NPREAL_NET_GET_TTY_STATUS	6

#define	NPREAL_CMD_TIMEOUT		10*HZ  // 10 seconds

#define	NPREAL_NET_CMD_RETRIEVE		1
#define	NPREAL_NET_CMD_RESPONSE		2


#define	NPREAL_NET_NODE_OPENED			0x01
#define	NPREAL_NET_NODE_CONNECTED		0x02
#define	NPREAL_NET_NODE_DISCONNECTED	0x04
#define	NPREAL_NET_DO_SESSION_RECOVERY	0x08
#define	NPREAL_NET_DO_INITIALIZE		0x10
#define	NPREAL_NET_TTY_INUSED			0x20

// ASPP command set

#define	ASPP_NOTIFY 			0x26

#define	ASPP_NOTIFY_PARITY 		0x01
#define	ASPP_NOTIFY_FRAMING 	0x02
#define	ASPP_NOTIFY_HW_OVERRUN 	0x04
#define	ASPP_NOTIFY_SW_OVERRUN 	0x08
#define	ASPP_NOTIFY_BREAK 		0x10
#define	ASPP_NOTIFY_MSR_CHG 	0x20

#define	ASPP_CMD_IOCTL			16
#define	ASPP_CMD_FLOWCTRL		17
#define	ASPP_CMD_LSTATUS		19
#define	ASPP_CMD_LINECTRL		18
#define	ASPP_CMD_FLUSH			20
#define	ASPP_CMD_OQUEUE			22
#define	ASPP_CMD_SETBAUD		23
#define	ASPP_CMD_START_BREAK	33
#define	ASPP_CMD_STOP_BREAK		34
#define	ASPP_CMD_START_NOTIFY	36
#define	ASPP_CMD_STOP_NOTIFY	37
#define	ASPP_CMD_HOST			43
#define	ASPP_CMD_PORT_INIT		44
#define	ASPP_CMD_WAIT_OQUEUE 	47

#define	ASPP_CMD_IQUEUE			21
#define	ASPP_CMD_XONXOFF		24
#define	ASPP_CMD_PORT_RESET		32
#define	ASPP_CMD_RESENT_TIME	46
#define	ASPP_CMD_TX_FIFO		48
#define ASPP_CMD_SETXON     	51
#define ASPP_CMD_SETXOFF    	52

#define	ASPP_FLUSH_RX_BUFFER	0
#define	ASPP_FLUSH_TX_BUFFER	1
#define	ASPP_FLUSH_ALL_BUFFER	2

#define	ASPP_IOCTL_B300			0
#define	ASPP_IOCTL_B600			1
#define	ASPP_IOCTL_B1200		2
#define	ASPP_IOCTL_B2400		3
#define	ASPP_IOCTL_B4800		4
#define	ASPP_IOCTL_B7200		5
#define	ASPP_IOCTL_B9600		6
#define	ASPP_IOCTL_B19200		7
#define	ASPP_IOCTL_B38400		8
#define	ASPP_IOCTL_B57600		9
#define	ASPP_IOCTL_B115200		10
#define	ASPP_IOCTL_B230400		11
#define	ASPP_IOCTL_B460800		12
#define	ASPP_IOCTL_B921600		13
#define	ASPP_IOCTL_B150			14
#define	ASPP_IOCTL_B134			15
#define	ASPP_IOCTL_B110			16
#define	ASPP_IOCTL_B75			17
#define	ASPP_IOCTL_B50			18

#define	ASPP_IOCTL_BITS8		3
#define	ASPP_IOCTL_BITS7		2
#define	ASPP_IOCTL_BITS6		1
#define	ASPP_IOCTL_BITS5		0

#define	ASPP_IOCTL_STOP1		0
#define	ASPP_IOCTL_STOP2		4

#define	ASPP_IOCTL_EVEN			8
#define	ASPP_IOCTL_ODD			16
#define	ASPP_IOCTL_MARK			24
#define	ASPP_IOCTL_SPACE		32
#define	ASPP_IOCTL_NONE			0

struct server_setting_struct
{
    int32_t	server_type;
    int32_t	disable_fifo;
};

struct npreal_struct
{
    int			port;
    int			flags;		/* defined in tty.h */
    int			type;		/* UART type */
    struct tty_struct *	tty;
    int			xmit_fifo_size;
    int			custom_divisor;
    int			x_char; 	/* xon/xoff character */
    int			close_delay;
    unsigned short		closing_wait;
    int			modem_control;	/* Modem control register */
    int			modem_status;	/* Line status */
    unsigned long		event;
    int			count;		/* # of fd on device */
    pid_t	    session;	/* Session of opening process */
    pid_t       pgrp;		/* pgrp of opening process */
    unsigned char		*xmit_buf;
    int			xmit_head;
    int			xmit_tail;
    int			xmit_cnt;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    struct tq_struct	tqueue;
    struct tq_struct	process_flip_tqueue;
#else
    struct work_struct 	tqueue;
    struct work_struct	process_flip_tqueue;
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
    struct termios		normal_termios;
    struct termios		callout_termios;
#else
    struct ktermios		normal_termios;
    struct ktermios		callout_termios;
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,4,0))
    struct wait_queue	*open_wait;
    struct wait_queue	*close_wait;
    struct wait_queue	*delta_msr_wait;
#else
    wait_queue_head_t 	open_wait;
    wait_queue_head_t 	close_wait;
    wait_queue_head_t 	delta_msr_wait;
#endif
    struct async_icount	icount; /* kernel counters for the 4 input interrupts */
    struct nd_struct  	*net_node;


#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
    /* We use spin_lock_irqsave instead of semaphonre here.
       Reason: When we use pppd to dialout via Real TTY driver,
       some driver functions, such as npreal_write(), would be
       invoked under interrpute mode which causes warning in
       down/up tx_semaphore.
     */
//  struct semaphore    tx_lock;
    spinlock_t          tx_lock;
    struct semaphore    rx_semaphore;
#else
    struct semaphore    tx_lock;
    struct semaphore	rx_semaphore;
#endif


};

struct nd_struct
{
    int32_t             server_type;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,4,0))
    struct wait_queue	*initialize_wait;
    struct wait_queue	*select_in_wait;
    struct wait_queue	*select_out_wait;
    struct wait_queue	*select_ex_wait;
    struct wait_queue	*cmd_rsp_wait;
#else
    wait_queue_head_t	initialize_wait;
    wait_queue_head_t	select_in_wait;
    wait_queue_head_t	select_out_wait;
    wait_queue_head_t	select_ex_wait;
    wait_queue_head_t	cmd_rsp_wait;
#endif
    int			tx_ready;
    int			rx_ready;
    int			cmd_ready;
    int			wait_oqueue_responsed;
    int			oqueue;
    unsigned char 		cmd_buffer[84];
    unsigned char 		rsp_buffer[84];
    struct semaphore	cmd_semaphore;
    int			rsp_length;
    unsigned long		flag;
    struct proc_dir_entry 	*node_entry;
    struct npreal_struct 	*tty_node;
    struct semaphore	semaphore;
    int			do_session_recovery_len;

};

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
static struct tty_driver	npvar_sdriver;
static struct tty_driver	npvar_cdriver;
#else
static struct tty_driver	*npvar_sdriver;
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
static int			npvar_refcount;
#endif

static struct npreal_struct	npvar_table[NPREAL_PORTS];
static struct nd_struct 	npvar_net_nodes[NPREAL_PORTS];
static struct tty_struct *	npvar_tty[NPREAL_PORTS];
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
static struct termios * 	npvar_termios[NPREAL_PORTS];
static struct termios * 	npvar_termios_locked[NPREAL_PORTS];
#else
static struct ktermios * 	npvar_termios[NPREAL_PORTS];
static struct ktermios * 	npvar_termios_locked[NPREAL_PORTS];
#endif
static int			npvar_diagflag;
static struct proc_dir_entry *  npvar_proc_root;
/*
 * npvar_tmp_buf is used as a temporary buffer by serial_write. We need
 * to lock it in case the memcpy_fromfs blocks while swapping in a page,
 * and some other program tries to do a serial write at the same time.
 * Since the lock will only come under contention when the system is
 * swapping and available memory is low, it makes sense to share one
 * buffer across all the serial ports, since it significantly saves
 * memory if large numbers of serial ports are open.
 */
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
static unsigned char *		npvar_tmp_buf;
static struct semaphore 	npvar_tmp_buf_sem;
#else
static unsigned char *		npvar_tmp_buf = 0;
static struct semaphore 	npvar_tmp_buf_sem = MUTEX;
#endif

#if (LINUX_VERSION_CODE < VERSION_CODE(2,4,0))
static struct inode_operations npreal_net_iops;
#endif
static struct file_operations npreal_net_fops;

#ifdef MODULE
int		init_module(void);
void	cleanup_module(void);
#endif

int		npreal_init(void);
static int 	npreal_init_tty(void);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
static void	npreal_do_softint(void *);
static void npreal_flush_to_ldisc(void *);
#else
static void	npreal_do_softint(struct work_struct *work);
static void npreal_flush_to_ldisc(struct work_struct *work);
#endif
static int	npreal_open(struct tty_struct *,struct file *);
static void	npreal_close(struct tty_struct *,struct file *);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,10))
static int	npreal_write(struct tty_struct *,int,const unsigned char *,int);
#else
static int	npreal_write(struct tty_struct *,const unsigned char *,int);
#endif
static int	npreal_write_room(struct tty_struct *);
static void	npreal_flush_buffer(struct tty_struct *);
static void	npreal_ldisc_flush_buffer(struct tty_struct *);
static int	npreal_chars_in_buffer(struct tty_struct *);
static void	npreal_flush_chars(struct tty_struct *);
static void	npreal_put_char(struct tty_struct *,unsigned char);
static int	npreal_ioctl(struct tty_struct *,struct file *,uint,ulong);
static void	npreal_throttle(struct tty_struct *);
static void	npreal_unthrottle(struct tty_struct *);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
static void	npreal_set_termios(struct tty_struct *,struct termios *);
static int	npreal_port_init(struct npreal_struct *,struct termios *);
#else
static void	npreal_set_termios(struct tty_struct *,struct ktermios *);
static int	npreal_port_init(struct npreal_struct *,struct ktermios *);
#endif
static void	npreal_stop(struct tty_struct *);
static void	npreal_start(struct tty_struct *);
static void	npreal_hangup(struct tty_struct *);
static inline void npreal_check_modem_status(struct npreal_struct *,int);
static int	npreal_block_til_ready(struct tty_struct *,struct file *, struct npreal_struct *);
static int	npreal_startup(struct npreal_struct *,struct file *, struct tty_struct *);
static void	npreal_shutdown(struct npreal_struct *);
static int 	npreal_port_shutdown(struct npreal_struct *);
static int	npreal_get_serial_info(struct npreal_struct *, struct serial_struct *);
static int	npreal_set_serial_info(struct npreal_struct *, struct serial_struct *);
static int	npreal_get_lsr_info(struct npreal_struct *,unsigned int *);
static void	npreal_send_break(struct npreal_struct *,int);

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
static int 	npreal_tiocmget(struct tty_struct *, struct file *);
static int 	npreal_tiocmset(struct tty_struct *, struct file *, unsigned int, unsigned int);
#else
static int	npreal_get_modem_info(struct npreal_struct *,unsigned int *);
static int	npreal_set_modem_info(struct npreal_struct *,unsigned int, unsigned int *);
#endif

static void	npreal_process_notify(struct nd_struct *,char *,int);
static void 	npreal_do_session_recovery(struct npreal_struct *);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
static void 	npreal_wait_until_sent(struct tty_struct *,int);
#endif
static int 	npreal_wait_and_set_command(struct nd_struct *,char,char);
static int 	npreal_wait_command_completed(struct nd_struct *,char,char, long,char *,int *);
static long 	npreal_wait_oqueue(struct npreal_struct *,long);
static int 	npreal_linectrl(struct nd_struct *nd,int modem_control);
static void 	npreal_break(struct tty_struct * ttyinfo, int break_state);
static void 	npreal_start_break(struct nd_struct *nd);
static void 	npreal_stop_break(struct nd_struct *nd);
static int npreal_setxon_xoff(struct npreal_struct * info, int cmd);

/*
 *  File operation declarations
 */
static  int npreal_net_open(struct inode *, struct file * );
static  int npreal_net_ioctl(struct inode *, struct file *,unsigned int, unsigned long );
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
static int	npreal_net_close(struct inode *,struct file * );
static ssize_t	npreal_net_read (struct file *file,char *buf,size_t count, loff_t *ppos);
static ssize_t	npreal_net_write(struct file *file,const char *buf, size_t count,loff_t *ppos);
static  unsigned int  npreal_net_select(struct file *file, struct poll_table_struct *);
#else
static void npreal_net_close(struct inode *,struct file * );
static int npreal_net_read (struct inode *,struct file *,char *,int);
static int npreal_net_write(struct inode *,struct file *,const char *,int);
static int npreal_net_select(struct inode *,struct file *file,int, select_table *);
#endif
/*
 *  "proc" table manipulation functions
 */
static struct proc_dir_entry *npreal_create_proc_entry(const char *, mode_t, struct proc_dir_entry *);
static void npreal_remove_proc_entry( struct proc_dir_entry *);


#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
static struct tty_operations mpvar_ops =
{
	.open = npreal_open,
	.close = npreal_close,
	.write = npreal_write,
	.put_char = npreal_put_char,
	.flush_chars = npreal_flush_chars,
	.write_room = npreal_write_room,
	.chars_in_buffer = npreal_chars_in_buffer,
	.flush_buffer = npreal_flush_buffer,
	.ioctl = npreal_ioctl,
	.throttle = npreal_throttle,
	.unthrottle = npreal_unthrottle,
	.set_termios = npreal_set_termios,
	.stop = npreal_stop,
	.start = npreal_start,
	.hangup = npreal_hangup,
	.tiocmget = npreal_tiocmget,
	.tiocmset = npreal_tiocmset,
};
#endif

/*
 * The MOXA NPort server Real TTY driver boot-time initialization code!
 */
#ifdef MODULE
INIT_FUNC_RET INIT_FUNC(void)
{
    int	ret;

    DBGPRINT(MX_DEBUG_INFO, "Loading module npreal major(%d), coutmajor(%d)...\n", ttymajor, calloutmajor);
    ret = npreal_init();
    DBGPRINT(MX_DEBUG_INFO, "Done.\n");
    return (ret);
}

CLEAR_FUNC_RET CLEAR_FUNC(void)
{
    int i,err = 0;
    struct npreal_struct *info;
    struct proc_dir_entry *de;

    info = &npvar_table[0];
    for (i = 0; i < NPREAL_PORTS; i++,info++)
    {
        if (info->net_node)
        {
            if ((de=((struct nd_struct *)(info->net_node))->node_entry))
                npreal_remove_proc_entry(de);
            ((struct nd_struct *)(info->net_node))->node_entry = NULL;
        }
    }
    if (npvar_proc_root)
    {
        npreal_remove_proc_entry( npvar_proc_root);
        npvar_proc_root = NULL;
    }

    DBGPRINT(MX_DEBUG_INFO, "Unloading module npreal ...\n");
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    if ((err |= tty_unregister_driver(&npvar_cdriver)))
    {
        DBGPRINT(MX_DEBUG_ERROR, "Couldn't unregister MOXA Async/NPort server family Real TTY callout driver\n");
    }
#endif
    if ((err |= tty_unregister_driver(DRV_VAR)))
    {
        DBGPRINT(MX_DEBUG_ERROR, "Couldn't unregister MOXA Async/NPort server family Real TTY driver\n");
    }
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
    put_tty_driver(DRV_VAR);
#endif

    DBGPRINT(MX_DEBUG_INFO, "Done.\n");

}
#endif

static int
npreal_init_tty(void)
{
    struct npreal_struct *tty_node;
    int	i;
    struct proc_dir_entry *de;
    struct nd_struct *net_node;
    char	buf[4];

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
    init_MUTEX(&npvar_tmp_buf_sem);
#else
    npvar_tmp_buf_sem = MUTEX;
#endif
    npvar_proc_root = proc_mkdir("npreal2", &proc_root);
    //npvar_proc_root = npreal_create_proc_entry( "npreal2",S_IFDIR, &proc_root);
    if ( !npvar_proc_root  )
        return -ENOMEM;
    tty_node = &npvar_table[0];
    net_node = &npvar_net_nodes[0];

    for ( i=0; i<NPREAL_PORTS; i++, tty_node++,net_node++ )
    {
        sprintf(buf,"%d",i);
        de = npreal_create_proc_entry( buf, S_IRUGO | S_IWUGO | S_IFREG, npvar_proc_root);
        if ( !de )
            return -ENOMEM;

        de->data = (void *) net_node;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
        de->proc_fops = &npreal_net_fops;
#else
        de->ops = &npreal_net_iops;

#endif
        net_node->tty_node = tty_node;
        net_node->node_entry = de;
        net_node->flag = 0;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
        init_MUTEX(&net_node->semaphore);
        init_MUTEX(&net_node->cmd_semaphore);
        init_waitqueue_head(&net_node->initialize_wait);
        init_waitqueue_head(&net_node->select_in_wait);
        init_waitqueue_head(&net_node->select_out_wait);
        init_waitqueue_head(&net_node->select_ex_wait);
        init_waitqueue_head(&net_node->cmd_rsp_wait);
#else
        net_node->semaphore = MUTEX;
        net_node->cmd_semaphore = MUTEX;
#endif

        tty_node->net_node = net_node;
        tty_node->port = i;
        tty_node->type = PORT_16550A;
        tty_node->flags = 0;
        tty_node->xmit_fifo_size = 16;
        tty_node->close_delay = 5*HZ/10;
        tty_node->closing_wait = 30*HZ;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))        
        INIT_WORK(&tty_node->tqueue, npreal_do_softint, tty_node);
        INIT_WORK(&tty_node->process_flip_tqueue, npreal_flush_to_ldisc, tty_node);
#else
		INIT_WORK(&tty_node->tqueue, npreal_do_softint);
        INIT_WORK(&tty_node->process_flip_tqueue, npreal_flush_to_ldisc);
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
        tty_node->callout_termios = npvar_cdriver.init_termios;
#endif
        tty_node->normal_termios = DRV_VAR_P(init_termios);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
        //init_MUTEX(&tty_node->tx_lock);
        tty_node->tx_lock = SPIN_LOCK_UNLOCKED;
        init_MUTEX(&tty_node->rx_semaphore);
        init_waitqueue_head(&tty_node->open_wait);
        init_waitqueue_head(&tty_node->close_wait);
        init_waitqueue_head(&tty_node->delta_msr_wait);
#else
        tty_node->tx_lock = MUTEX;
        tty_node->rx_semaphore = MUTEX;
        tty_node->modem_control = 0;

#endif
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        tty_node->icount.rx = tty_node->icount.tx = 0;
#endif
        tty_node->icount.cts = tty_node->icount.dsr =
                                   tty_node->icount.dsr =  tty_node->icount.dcd = 0;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        tty_node->icount.frame = tty_node->icount.overrun =
                                     tty_node->icount.brk = tty_node->icount.parity = 0;
#endif
    }
    return 0;
}



int
npreal_init(void)
{
    int	ret1, ret2;

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
    npvar_sdriver = alloc_tty_driver(NPREAL_PORTS+1);
    if (!npvar_sdriver)
        return -ENOMEM;
#endif
    printk("MOXA Async/NPort server family Real TTY driver ttymajor %d calloutmajor %d verbose %d (%s)\n", ttymajor, calloutmajor, verbose, NPREAL_VERSION);

    /* Initialize the tty_driver structure */

    memset(DRV_VAR, 0, sizeof(struct tty_driver));
    DRV_VAR_P(magic) = TTY_DRIVER_MAGIC;
    DRV_VAR_P(name) = "ttyr";
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,18))
    DRV_VAR_P(devfs_name) = "tts/r";
#endif
#endif
    DRV_VAR_P(major) = ttymajor;
    DRV_VAR_P(minor_start) = 0;
    DRV_VAR_P(num) = NPREAL_PORTS;
    DRV_VAR_P(type) = TTY_DRIVER_TYPE_SERIAL;
    DRV_VAR_P(subtype) = SERIAL_TYPE_NORMAL;
    DRV_VAR_P(init_termios) = tty_std_termios;
    DRV_VAR_P(init_termios.c_cflag) = B9600|CS8|CREAD|HUPCL|CLOCAL;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,18))
    DRV_VAR_P(flags) = TTY_DRIVER_REAL_RAW | TTY_DRIVER_NO_DEVFS;
#else
    DRV_VAR_P(flags) = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
    tty_set_operations(DRV_VAR, &mpvar_ops);
    DRV_VAR_P(ttys) = npvar_tty;
#else
    DRV_VAR_P(refcount) = &npvar_refcount;
    DRV_VAR_P(table) = npvar_tty;
#endif

    DRV_VAR_P(termios) = npvar_termios;
    DRV_VAR_P(termios_locked) = npvar_termios_locked;

    DRV_VAR_P(open) = npreal_open;
    DRV_VAR_P(close) = npreal_close;
    DRV_VAR_P(write) = npreal_write;
    DRV_VAR_P(put_char) = npreal_put_char;
    DRV_VAR_P(flush_chars) = npreal_flush_chars;
    DRV_VAR_P(write_room) = npreal_write_room;
    DRV_VAR_P(chars_in_buffer) = npreal_chars_in_buffer;
    DRV_VAR_P(flush_buffer) = npreal_ldisc_flush_buffer;
//	DRV_VAR_P(flush_buffer) = npreal_flush_buffer;
    DRV_VAR_P(ioctl) = npreal_ioctl;
    DRV_VAR_P(throttle) = npreal_throttle;
    DRV_VAR_P(unthrottle) = npreal_unthrottle;
    DRV_VAR_P(set_termios) = npreal_set_termios;
    DRV_VAR_P(stop) = npreal_stop;
    DRV_VAR_P(start) = npreal_start;
    DRV_VAR_P(hangup) = npreal_hangup;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
    DRV_VAR_P(wait_until_sent) = npreal_wait_until_sent;
#endif
    DRV_VAR_P(break_ctl) = npreal_break;

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    /*
     * The callout device is just like normal device except for
     * major number and the subtype code.
     */
    npvar_cdriver = npvar_sdriver;
    npvar_cdriver.name = "cur";
    npvar_cdriver.major = calloutmajor;
    npvar_cdriver.subtype = SERIAL_TYPE_CALLOUT;
#endif
    DBGPRINT(MX_DEBUG_INFO, "Tty devices major number = %d, callout devices major number = %d\n",ttymajor,calloutmajor);

    npvar_diagflag = 0;
    memset(npvar_table, 0, NPREAL_PORTS * sizeof(struct npreal_struct));

    ret1 = 0;
    ret2 = 0;
    if ( !(ret1=tty_register_driver(DRV_VAR)) )
    {
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
        if ( (ret2=tty_register_driver(&npvar_cdriver)) )
        {
            tty_unregister_driver(DRV_VAR);
            DBGPRINT(MX_DEBUG_ERROR, "Couldn't install MOXA Async/NPort server family Real TTY callout driver !\n");
        }
#endif
    }

    else
    {
        DBGPRINT(MX_DEBUG_ERROR, "Couldn't install MOXA Async/NPort server family driver !\n");
    }

    if (ret1 || ret2)
    {
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
        put_tty_driver(DRV_VAR);
#endif
        return -1;
    }

    /* Initialize the net node structure */
    memset(&npreal_net_fops,0,sizeof(struct file_operations));
    npreal_net_fops.read = npreal_net_read;
    npreal_net_fops.write = npreal_net_write;
    npreal_net_fops.ioctl = npreal_net_ioctl;
    npreal_net_fops.open = npreal_net_open;
    npreal_net_fops.release = npreal_net_close;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
    npreal_net_fops.poll = npreal_net_select;
#else
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
    npreal_net_fops.poll = npreal_net_select;
    memset(&npreal_net_iops,0,sizeof(struct inode_operations));
    npreal_net_iops.default_file_ops = &npreal_net_fops;
#else
    npreal_net_fops.select = npreal_net_select;
    memset(&npreal_net_iops,0,sizeof(struct inode_operations));
    npreal_net_iops.default_file_ops = &npreal_net_fops;
#endif
#endif
    if (npreal_init_tty() != 0)
    {
        tty_unregister_driver(DRV_VAR);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
        tty_unregister_driver(&npvar_cdriver);
#endif
        DBGPRINT(MX_DEBUG_ERROR, "Couldn't install MOXA Async/NPort server family Real TTY driver !\n");
    }
    return(0);
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
static void
npreal_do_softint(void *private_)
{
    struct npreal_struct *	info = (struct npreal_struct *)private_;
#else
static void
npreal_do_softint(struct work_struct *work)
{
    struct npreal_struct *	info = 
    	container_of(work, struct npreal_struct, tqueue);
#endif

    struct tty_struct *	tty;

    if (!info)
        goto done;

    tty = info->tty;
    if (tty)
    {
#if (LINUX_VERSION_CODE <  VERSION_CODE(2,1,0))
        if ( clear_bit(NPREAL_EVENT_TXLOW, &info->event) )
        {
            if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
                    tty->ldisc.write_wakeup )
                (tty->ldisc.write_wakeup)(tty);
            wake_up_interruptible(&tty->write_wait);
        }
        if ( clear_bit(NPREAL_EVENT_HANGUP, &info->event) )
        {
            // Scott: 2005-09-05
            // Do it when entering npreal_hangup().
            // Scott info->flags |= ASYNC_CLOSING;
            tty_hangup(tty);
        }
#else
        if ( test_and_clear_bit(NPREAL_EVENT_TXLOW, &info->event) )
        {
            if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
                    tty->ldisc.write_wakeup )
                (tty->ldisc.write_wakeup)(tty);
            wake_up_interruptible(&tty->write_wait);
        }
        if ( test_and_clear_bit(NPREAL_EVENT_HANGUP, &info->event) )
        {
            // Scott: 2005-09-05
            // Do it when entering npreal_hangup().
            // Scott info->flags |= ASYNC_CLOSING;
            tty_hangup(tty);
        }
#endif
    }

done:
    ;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    MX_MOD_DEC;
#endif
}

/*
 * This routine is called whenever a serial port is opened.
 */

/* Scott: 2005/07/13
 * Note that on failure, we don't decrement the module use count - the tty driver
 * later will call npreal_close, which will decrement it for us as long as
 * tty->driver_data is set non-NULL.
 */
static int
npreal_open(
    struct tty_struct * tty,
    struct file * filp)
{
    struct npreal_struct 	*info;
    int			line;
    unsigned long		page;
    struct nd_struct  	*nd;

    MX_MOD_INC;
    line = PORTNO(tty);
    if ( (line < 0) || (line >= NPREAL_PORTS) )
    {
        DBGPRINT(MX_DEBUG_ERROR, "invalid line (%d)\n", line);
        MX_MOD_DEC;
        return(-ENODEV);
    }
    info = npvar_table + line;

    nd = info->net_node;
    if ( !nd  || !(nd->flag&NPREAL_NET_NODE_OPENED))
    {
        DBGPRINT(MX_DEBUG_ERROR, "net device not ready\n");
        MX_MOD_DEC;
        return(-ENODEV);
    }


    if ( !npvar_tmp_buf )
    {
        page = GET_FPAGE(GFP_KERNEL);
        if ( !page )
        {
            DBGPRINT(MX_DEBUG_ERROR, "allocate npvar_tmp_buf failed\n");
            MX_MOD_DEC;
            return(-ENOMEM);
        }
        if ( npvar_tmp_buf )
            free_page(page);
        else
            npvar_tmp_buf = (unsigned char *)page;
    }

    /*
     * Start up serial port
     */
// Scott: 2005/07/13
// Set tty->driver_data before entering npreal_startup(), so that the tty driver
// can decrease refcount if npreal_startup() failed, by calling npreal_close().
    tty->driver_data = info;
    info->count++;
// Scott: end

    if (npreal_startup(info,filp,tty))
    {
        DBGPRINT(MX_DEBUG_ERROR, "npreal_startup failed\n");
        return(-EIO);
    }
    if (npreal_block_til_ready(tty, filp, info))
    {
        DBGPRINT(MX_DEBUG_ERROR, "npreal_block_til_ready failed\n");
        return(-EIO);
    }

    if ( (info->count == 1) && (info->flags & ASYNC_SPLIT_TERMIOS) )
    {
        if ( MX_TTY_DRV(subtype) == SERIAL_TYPE_NORMAL )
            *tty->termios = info->normal_termios;
        else
            *tty->termios = info->callout_termios;

        if (npreal_port_init(info, 0))
        {
            DBGPRINT(MX_DEBUG_ERROR, "npreal_port_init failed\n");
            return(-EIO);
        }
    }

    info->session = MX_SESSION();
    info->pgrp = MX_CGRP();

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
    /* It must be always on */
    tty->low_latency = 1;
#endif
    return(0);
}

/*
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.
 */
static void
npreal_close(
    struct tty_struct * tty,
    struct file * filp)
{
    struct npreal_struct * info = (struct npreal_struct *)tty->driver_data;
    long	timeout,et,ret;
    int	cnt;

    if ( !info )
        return;
    if ( tty_hung_up_p(filp) )
    {
        info->count--;
        MX_MOD_DEC;
        return;
    }
// Scott: 2005/07/13
// Comment out the following two if's.
#if 0
    if (!(info->flags & ASYNC_INITIALIZED))
    {
        info->count--;
        MX_MOD_DEC;
        return;
    }
    if (info->flags & ASYNC_CLOSING)
    {
        info->count--;
        MX_MOD_DEC;
        return;
    }
#endif

#ifndef SP1
    if ( (tty->count == 1) && (info->count != 1) )
    {
#else
#if (LINUX_VERSION_CODE < VERSION_CODE(2,4,21))
    if ( (tty->count == 1) && (info->count != 1) )
    {
#else
        if ( (atomic_read(&tty->count) == 1) && (info->count != 1) )
        {
#endif
#endif
        /*
         * Uh, oh.	tty->count is 1, which means that the tty
         * structure will be freed.  Info->count should always
         * be one in these conditions.  If it's greater than
         * one, we've got real problems, since it means the
         * serial port won't be shutdown.
         */
        DBGPRINT(MX_DEBUG_WARN, "[%d] npreal_close: bad serial port count; tty->count is 1, info->count is %d\n", current->pid, info->count);
        info->count = 1;
    }
    if ( --info->count < 0 )
    {
        DBGPRINT(MX_DEBUG_WARN, "npreal_close: bad serial port count for port %d: %d\n", info->port, info->count);
        info->count = 0;
    }
    if ( info->count )
    {
        MX_MOD_DEC;
        return;
    }

// Scott: 2005-09-05
// Prevent race condition on closing.
    if (info->flags & ASYNC_CLOSING)
        return;

    info->flags |= ASYNC_CLOSING;
    tty->closing = 1;
    /*
     * Save the termios structure, since this port may have
     * separate termios for callout and dialin.
     */
    if ( info->flags & ASYNC_NORMAL_ACTIVE )
        info->normal_termios = *tty->termios;
    if ( info->flags & ASYNC_CALLOUT_ACTIVE )
        info->callout_termios = *tty->termios;
    /*
     * Now we wait for the transmit buffer to clear; and we notify
     * the line discipline to only process XON/XOFF characters.
     */
// Scott: 2005-07-08
// If the open mode is nonblocking mode, don't block on close.
    if ( !(filp->f_flags & O_NONBLOCK) && info->closing_wait != ASYNC_CLOSING_WAIT_NONE )
    {
//	if ( info->closing_wait != ASYNC_CLOSING_WAIT_NONE ) {
        et = jiffies + info->closing_wait;
        tty_wait_until_sent(tty, info->closing_wait);
        cnt = 0;
        while ((timeout = et - jiffies) > 0)
        {
            if ((ret=npreal_wait_oqueue(info,timeout)) == 0)
            {
                if (++cnt >= 3)
                {
                    break;
                }
            }
            else if (ret < 0)
                break;
            else
                cnt = 0;
            current->state = TASK_INTERRUPTIBLE;
            schedule_timeout(HZ/100);
        }
    }
    npreal_flush_buffer(tty);
    if ( tty->ldisc.flush_buffer )
        tty->ldisc.flush_buffer(tty);
    npreal_shutdown(info);
    tty->closing = 0;
    MX_MOD_DEC;
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,10))
static int npreal_write(struct tty_struct * tty, int from_user,
                        const unsigned char * buf, int count)
#else
static int npreal_write(struct tty_struct * tty,
                        const unsigned char * buf, int count)
#endif
{
    int c, total = 0, ret;
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    struct nd_struct  	*nd;
    unsigned long        flags;

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,10))
    int from_user = 0;
#endif

// Scott: 2005-09-12
    if (!info)
        return 0;

    if ( !tty || !info->xmit_buf || !npvar_tmp_buf )
        return(0);

    nd = info->net_node;

    if (!nd)
        return 0;

    if ( from_user )
        down(&npvar_tmp_buf_sem);

    while ( 1 )
    {
        c = MIN(count, MIN(SERIAL_XMIT_SIZE - info->xmit_cnt - 1,
                           SERIAL_XMIT_SIZE - info->xmit_head));
        if ( c <= 0 )
            break;

        if ( from_user )
        {
            ret = copy_from_user(npvar_tmp_buf, buf, c);
            memcpy(info->xmit_buf + info->xmit_head, npvar_tmp_buf, c);
        }
        else
            memcpy(info->xmit_buf + info->xmit_head, buf, c);

        DOWN(info->tx_lock, flags);
        info->xmit_head = (info->xmit_head + c) & (SERIAL_XMIT_SIZE - 1);
        info->xmit_cnt += c;
        UP(info->tx_lock, flags);

        buf += c;
        count -= c;
        total += c;
    }
    if (info->xmit_cnt )
    {
        nd->tx_ready = 1;
        if ( waitqueue_active(&nd->select_in_wait))
            wake_up_interruptible( &nd->select_in_wait );
    }

    if ( from_user )
        up(&npvar_tmp_buf_sem);
    return(total);
}

static void npreal_put_char(struct tty_struct * tty, unsigned char ch)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    struct nd_struct  	*nd;
    unsigned long        flags;

// Scott: 2005-09-12
    if (!info)
        return;

    if ( !tty || !info->xmit_buf )
        return;

    //down(&info->tx_semaphore);
    DOWN(info->tx_lock, flags);

// Scott: 2005-09-12
    if (!info->xmit_buf)
    {
        UP(info->tx_lock, flags);
        return;
    }

    if ( info->xmit_cnt >= SERIAL_XMIT_SIZE - 1 )
    {
        //up(&info->tx_semaphore);
        UP(info->tx_lock, flags);
        return;
    }

    nd = info->net_node;
    if (!nd)
    {
        UP(info->tx_lock, flags);
        return;
    }

    info->xmit_buf[info->xmit_head++] = ch;
    info->xmit_head &= SERIAL_XMIT_SIZE - 1;
    info->xmit_cnt++;

    nd->tx_ready = 1;
    if ( waitqueue_active(&nd->select_in_wait))
        wake_up_interruptible( &nd->select_in_wait );

    //up(&info->tx_semaphore);
    UP(info->tx_lock, flags);

}

static void npreal_flush_chars(struct tty_struct * tty)
{}

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
static void npreal_wait_until_sent(struct tty_struct * tty,int timeout)
{

    struct npreal_struct *info;

    if ((info = (struct npreal_struct *)tty->driver_data))
        npreal_wait_oqueue(info,timeout);

}
#endif

static int npreal_write_room(struct tty_struct * tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    int	ret;

// Scott: 2005-09-12
    if (!info)
        return 0;

    ret = SERIAL_XMIT_SIZE - info->xmit_cnt - 1;
    if ( ret < 0 )
        ret = 0;
    return(ret);
}

static int npreal_chars_in_buffer(struct tty_struct * tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;

    if (!info)
        return (-EIO);
    return(info->xmit_cnt);

}


static void npreal_flush_buffer(struct tty_struct * tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    struct nd_struct	*nd;
    char rsp_buffer[8];
    int  rsp_length = sizeof(rsp_buffer);
    unsigned long   flags;

    if (!info)
        return;

    //down(&info->tx_semaphore);
    DOWN(info->tx_lock, flags);
    info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
    //up(&info->tx_semaphore);
    UP(info->tx_lock, flags);

    wake_up_interruptible(&tty->write_wait);
    if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
            tty->ldisc.write_wakeup )
        (tty->ldisc.write_wakeup)(tty);
    if (!(nd=info->net_node))
        return;
    nd->tx_ready = 0;
    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_FLUSH) < 0)
        return;
    nd->cmd_buffer[2] = 1;
    nd->cmd_buffer[3] = ASPP_FLUSH_ALL_BUFFER;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }
    npreal_wait_command_completed(nd,
                                  NPREAL_ASPP_COMMAND_SET,
                                  ASPP_CMD_FLUSH,NPREAL_CMD_TIMEOUT,
                                  rsp_buffer,
                                  &rsp_length);
}

/* This function may be call from dispatch function,in that case we
 will unable to complete the NPPI function in single CPU platform,
so we just fire the command ASAP and don't wait its completion */

static void npreal_ldisc_flush_buffer(struct tty_struct * tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    struct nd_struct	*nd;
    unsigned long   flags;

    if (!info)
        return;

    //down(&info->tx_semaphore);
    DOWN(info->tx_lock, flags);
    info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
    //up(&info->tx_semaphore);
    UP(info->tx_lock, flags);

    wake_up_interruptible(&tty->write_wait);
    if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
            tty->ldisc.write_wakeup )
        (tty->ldisc.write_wakeup)(tty);
    if (!(nd=info->net_node))
        return;
    nd->tx_ready = 0;
    if (nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)
        return;
    nd->cmd_buffer[0] = NPREAL_ASPP_COMMAND_SET;
    nd->cmd_buffer[1] = ASPP_CMD_FLUSH;
    nd->cmd_buffer[2] = 1;
    nd->cmd_buffer[3] = ASPP_FLUSH_ALL_BUFFER;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
    current->state = TASK_INTERRUPTIBLE;
    schedule_timeout(HZ/100);
#endif
}

static int npreal_ioctl(struct tty_struct * tty, struct file * file,
                        unsigned int cmd, unsigned long arg)
{
    int			error;
    struct npreal_struct *	info = (struct npreal_struct *)tty->driver_data;
    int			retval;
    struct async_icount	cprev, cnow;	    /* kernel counter temps */
    struct serial_icounter_struct *p_cuser;     /* user space */
    unsigned long 		templ;

// Scott: 2005-09-12
    if (!info)
        return -ENODEV;

    if ( (cmd != TIOCGSERIAL) && (cmd != TIOCMIWAIT) &&
            (cmd != TIOCGICOUNT) )
    {
        if ( tty->flags & (1 << TTY_IO_ERROR) )
        {
            return(-EIO);
        }
    }
    switch ( cmd )
    {
    case TCFLSH:
        retval = tty_check_change(tty);
        if (retval)
        {
            return retval;
        }


        switch (arg)
        {
        case TCIFLUSH:
            if (tty->ldisc.flush_buffer)
                tty->ldisc.flush_buffer(tty);
            break;
        case TCIOFLUSH:
            if (tty->ldisc.flush_buffer)
                tty->ldisc.flush_buffer(tty);
            /* fall through */
        case TCOFLUSH:
            npreal_flush_buffer(tty);
            break;
        default:
            return -EINVAL;
        }
        return 0;
    case TCSBRK:	/* SVID version: non-zero arg --> no break */
        retval = tty_check_change(tty);
        if ( retval )
            return(retval);
        tty_wait_until_sent(tty, 0);
        if ( !arg )
            npreal_send_break(info, HZ/4);		/* 1/4 second */
        return(0);
    case TCSBRKP:	/* support for POSIX tcsendbreak() */
        retval = tty_check_change(tty);
        if ( retval )
            return(retval);
        tty_wait_until_sent(tty, 0);
        npreal_send_break(info, arg ? arg*(HZ/10) : HZ/4);
        return(0);
    case TIOCGSOFTCAR:
        error = access_ok(VERIFY_WRITE, (void *)arg, sizeof(long))?0:-EFAULT;
        if ( error )
            return(error);
        put_to_user(C_CLOCAL(tty) ? 1 : 0, (unsigned long *)arg);
        return 0;
    case TIOCSSOFTCAR:
        error = access_ok(VERIFY_READ, (void *)arg, sizeof(long))?0:-EFAULT;
        if ( error )
            return(error);
        get_from_user(templ,(unsigned long *)arg);
        arg = templ;
        tty->termios->c_cflag = ((tty->termios->c_cflag & ~CLOCAL) |
                                 (arg ? CLOCAL : 0));
        return(0);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    case TIOCMGET:
        error = access_ok(VERIFY_WRITE, (void *)arg,
                          sizeof(unsigned int))?0:-EFAULT;
        if ( error )
            return(error);
        return(npreal_get_modem_info(info, (unsigned int *)arg));
    case TIOCMBIS:
    case TIOCMBIC:
    case TIOCMSET:
        return(npreal_set_modem_info(info, cmd, (unsigned int *)arg));
#endif
    case TIOCGSERIAL:
        error = access_ok(VERIFY_WRITE, (void *)arg,
                          sizeof(struct serial_struct))?0:-EFAULT;
        if ( error )
            return(error);
        return(npreal_get_serial_info(info, (struct serial_struct *)arg));
    case TIOCSSERIAL:
        error = access_ok(VERIFY_READ, (void *)arg,
                          sizeof(struct serial_struct))?0:-EFAULT;
        if ( error )
            return(error);
        return(npreal_set_serial_info(info, (struct serial_struct *)arg));
    case TIOCSERGETLSR: /* Get line status register */
        error = access_ok(VERIFY_WRITE, (void *)arg,
                          sizeof(unsigned int))?0:-EFAULT;
        if ( error )
            return(error);
        else
            return(npreal_get_lsr_info(info, (unsigned int *)arg));
        /*
         * Wait for any of the 4 modem inputs (DCD,RI,DSR,CTS) to change
         * - mask passed in arg for lines of interest
         *   (use |'ed TIOCM_RNG/DSR/CD/CTS for masking)
         * Caller should use TIOCGICOUNT to see which one it was
         */
    case TIOCMIWAIT:
    {
        DECLARE_WAITQUEUE(wait, current);
        int ret;
        cprev = info->icount;   /* note the counters on entry */
        add_wait_queue(&info->delta_msr_wait, &wait);
        while ( 1 )
        {
            /* see if a signal did it */
            cnow = info->icount;	/* atomic copy */
            set_current_state(TASK_INTERRUPTIBLE);
            if ( ((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
                    ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
                    ((arg & TIOCM_CD)	&& (cnow.dcd != cprev.dcd)) ||
                    ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts)) )
            {
                ret = 0;
                break;
            }
            if ( signal_pending(current) )
            {
                ret = -ERESTARTSYS;
                break;
            }
            cprev = cnow;
// Scott: 2005-09-04 add begin
            schedule();
// Scott: 2005-09-04 add end
        }
        current->state = TASK_RUNNING;
        remove_wait_queue(&info->delta_msr_wait, &wait);
// Scott: 2005-09-04
        // Scott break;
        return ret;
    }
    /* NOTREACHED */
    /*
     * Get counter of input serial line interrupts (DCD,RI,DSR,CTS)
     * Return: write counters to the user passed counter struct
     * NB: both 1->0 and 0->1 transitions are counted except for
     *     RI where only 0->1 is counted.
     */
    case TIOCGICOUNT:
        error = access_ok(VERIFY_WRITE, (void *)arg,
                          sizeof(struct serial_icounter_struct))?0:-EFAULT;
        if ( error )
            return(error);
        cnow = info->icount;
        p_cuser = (struct serial_icounter_struct *)arg;
        /* modified by casper 1/11/2000 */
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        if (put_user(cnow.frame, &p_cuser->frame))
            return -EFAULT;
        if (put_user(cnow.brk, &p_cuser->brk))
            return -EFAULT;
        if (put_user(cnow.overrun, &p_cuser->overrun))
            return -EFAULT;
        if (put_user(cnow.buf_overrun, &p_cuser->buf_overrun))
            return -EFAULT;
        if (put_user(cnow.parity, &p_cuser->parity))
            return -EFAULT;
        if (put_user(cnow.rx, &p_cuser->rx))
            return -EFAULT;
        if (put_user(cnow.tx, &p_cuser->tx))
            return -EFAULT;
#endif

        put_to_user(cnow.cts, &p_cuser->cts);
        put_to_user(cnow.dsr, &p_cuser->dsr);
        put_to_user(cnow.rng, &p_cuser->rng);
        put_to_user(cnow.dcd, &p_cuser->dcd);

        /* */
        return(0);

    case TCXONC:
        retval = tty_check_change(tty);
        if ( retval )
            return(retval);
        switch (arg)
        {
        case TCOOFF:
            return npreal_setxon_xoff(info, ASPP_CMD_SETXOFF);
        case TCOON:
            return npreal_setxon_xoff(info, ASPP_CMD_SETXON);
            /* fall through */
        default:
            return -EINVAL;
        }
        return 0;
    default:
        return(-ENOIOCTLCMD);
    }
    return(0);
}

/*
 * This routine is called by the upper-layer tty layer to signal that
 * incoming characters should be throttled.
 */
static void npreal_throttle(struct tty_struct * tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    struct nd_struct *nd;

// Scott: 2005-09-12
    if (!info)
        return;

    nd = info->net_node;
    if (!nd)
        return;
    nd->rx_ready = 0;
}

static void npreal_unthrottle(struct tty_struct * tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    struct nd_struct *nd;

    if (!info)
        return;
    nd = info->net_node;
    if (!nd)
        return;
    nd->rx_ready = 1;
    if ( waitqueue_active(&nd->select_out_wait))
        wake_up_interruptible( &nd->select_out_wait );
}
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
static void npreal_set_termios(struct tty_struct * tty,
                               struct termios * old_termios)
#else
static void npreal_set_termios(struct tty_struct * tty,
                               struct ktermios * old_termios)
#endif
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
    npreal_port_init(info, old_termios);

}

/*
 * npreal_stop() and npreal_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 */
static void npreal_stop(struct tty_struct * tty)
{}

static void npreal_start(struct tty_struct * tty)
{}

static void npreal_hangup(struct tty_struct *tty)
{
    struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;

    if (!info)
        return;

// Scott: 2005-09-05
// Prevent race condition on closing.
    if (info->flags & ASYNC_CLOSING)
        return;

    info->flags |=  ASYNC_CLOSING;
//
// do_tty_hangup() already do this
//	npreal_flush_buffer(tty);
    npreal_shutdown(info);
}

static inline void npreal_check_modem_status(struct npreal_struct *info,
        int status)
{
    int	is_dcd_changed = 0;

    /* update input line counters */
    if ( (info->modem_status & UART_MSR_DSR) != (status & UART_MSR_DSR ))
        info->icount.dsr++;
    if ( (info->modem_status & UART_MSR_DCD) != (status & UART_MSR_DCD ))
    {
        info->icount.dcd++;
        is_dcd_changed = 1;
    }
    if ( (info->modem_status & UART_MSR_CTS) != (status & UART_MSR_CTS ))
        info->icount.cts++;
    info->modem_status = status;
    wake_up_interruptible(&info->delta_msr_wait);

    if ( (info->flags & ASYNC_CHECK_CD) && (is_dcd_changed))
    {

        if ( status & UART_MSR_DCD )
        {
            wake_up_interruptible(&info->open_wait);
        }
        else
        {
            set_bit(NPREAL_EVENT_HANGUP,&info->event);
            MXQ_TASK(&info->tqueue);
        }
// Scott: 2005-09-06
#if 0
        else if ( !((info->flags & ASYNC_CALLOUT_ACTIVE) &&
                    (info->flags & ASYNC_CALLOUT_NOHUP)) &&
                  !(info->flags &ASYNC_CLOSING) )
        {
            set_bit(NPREAL_EVENT_HANGUP,&info->event);
            MXQ_TASK(&info->tqueue);
        }
#endif
    }

}

static int npreal_block_til_ready(struct tty_struct *tty, struct file * filp,
                                  struct npreal_struct *info)
{
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
    DECLARE_WAITQUEUE(wait, current);
#else
    struct wait_queue	wait =
        {
            current, NULL
        };
#endif
    int			retval;
    int			do_clocal = 0;
    struct nd_struct	*nd;


    if (!(nd=info->net_node))
        return (-EIO);
#if 0
    /*
     * If the device is in the middle of being closed, then block
     * until it's done, and then try again.
     */
    if ( tty_hung_up_p(filp) || (info->flags & ASYNC_CLOSING) )
    {
        if ( !tty_hung_up_p(filp) )
            interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
        if ( info->flags & ASYNC_HUP_NOTIFY )
        {
            return(-EAGAIN);
        }
        else
            return(-ERESTARTSYS);
#else
        return(-EAGAIN);
#endif
    }
#endif

    /*
     * If this is a callout device, then just make sure the normal
     * device isn't being used.
     */
    if ( MX_TTY_DRV(subtype) == SERIAL_TYPE_CALLOUT )
    {
        if ( info->flags & ASYNC_NORMAL_ACTIVE )
            return(-EBUSY);
        if ( (info->flags & ASYNC_CALLOUT_ACTIVE) &&
                (info->flags & ASYNC_SESSION_LOCKOUT) &&
                (info->session != MX_SESSION()) )
            return(-EBUSY);
        if ( (info->flags & ASYNC_CALLOUT_ACTIVE) &&
                (info->flags & ASYNC_PGRP_LOCKOUT) &&
                (info->pgrp != MX_CGRP()) )
            return(-EBUSY);
        info->flags |= ASYNC_CALLOUT_ACTIVE;
        return(0);
    }

    /*
     * If non-blocking mode is set, or the port is not enabled,
     * then make the check up front and then exit.
     */
    if ( (filp->f_flags & O_NONBLOCK) ||
            (tty->flags & (1 << TTY_IO_ERROR)) )
    {
        if ( info->flags & ASYNC_CALLOUT_ACTIVE )
        {
            return(-EBUSY);
        }
        info->flags |= ASYNC_NORMAL_ACTIVE;
        return(0);
    }

    if ( info->flags & ASYNC_CALLOUT_ACTIVE )
    {
        if ( info->normal_termios.c_cflag & CLOCAL )
            do_clocal = 1;
    }
    else
    {
        if ( tty->termios->c_cflag & CLOCAL )
            do_clocal = 1;
    }

    /*
     * Block waiting for the carrier detect and the line to become
     * free (i.e., not in use by the callout).  While we are in
     * this loop, info->count is dropped by one, so that
     * npreal_close() knows when to free things.  We restore it upon
     * exit, either normal or abnormal.
     */
    retval = 0;
    add_wait_queue(&info->open_wait, &wait);
    while ( 1 )
    {
        set_current_state(TASK_INTERRUPTIBLE);
        if ( tty_hung_up_p(filp) || (info->flags & ASYNC_CLOSING) )
        {
            if ( !tty_hung_up_p(filp) )
            {
#ifdef SERIAL_DO_RESTART
                if ( info->flags & ASYNC_HUP_NOTIFY )
                {
                    retval = -EAGAIN;
                }
                else
                    retval = -ERESTARTSYS;
#else
                retval = -EAGAIN;
#endif
            }
            break;
        }
        if ( !(info->flags & ASYNC_CALLOUT_ACTIVE) &&
                !(info->flags & ASYNC_CLOSING) &&
                (do_clocal || (info->modem_status & UART_MSR_DCD)) )
            break;
        if ( signal_pending(current) )
        {
            retval = -EIO;
            break;
        }
        schedule();
    }
    current->state = TASK_RUNNING;
    remove_wait_queue(&info->open_wait, &wait);
    if ( retval )
        return(retval);
    info->flags |= ASYNC_NORMAL_ACTIVE;
    return(0);
}

static int npreal_startup(struct npreal_struct * info,struct file *filp,struct tty_struct *tty)
{
    unsigned long	page;
    struct	nd_struct *nd;
    char rsp_buffer[8];
    int  rsp_length = sizeof(rsp_buffer);
    int	cnt = 0;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
    DECLARE_WAITQUEUE(wait, current);
#else
    struct wait_queue	wait =
        {
            current, NULL
        };
#endif


    if (!(nd=info->net_node))
    {
        // Scott info->count++;
        DBGPRINT(MX_DEBUG_ERROR, "info->net_node is null\n");
        return -EIO;
    }

    add_wait_queue(&nd->initialize_wait, &wait);
#if (LINUX_VERSION_CODE <  VERSION_CODE(2,1,0))
    while (set_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag))
    {
#else
    while (test_and_set_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag))
    {
#endif
        if ( signal_pending(current) )
        {
            DBGPRINT(MX_DEBUG_ERROR, "signal_pending break\n");
            break;
        }
        schedule();
    }
    current->state = TASK_RUNNING;
    remove_wait_queue(&nd->initialize_wait, &wait);

// Scott: 2005/07/13
// Set tty->driver_data before entering npreal_startup(), so that the tty driver
// can decrease refcount if npreal_startup() failed, by calling npreal_close().
// Scott	info->count++;
// Scott	tty->driver_data = info;
    info->tty = tty;
    if ( signal_pending(current) )
    {
        DBGPRINT(MX_DEBUG_ERROR, "signal_pending occurred\n");
        if ( waitqueue_active(&nd->initialize_wait))
            wake_up_interruptible( &nd->initialize_wait );
        return -EIO;
    }
#if 0
    if ( tty_hung_up_p(filp) || (info->flags & ASYNC_CLOSING) )
    {
        clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
        if ( waitqueue_active(&nd->initialize_wait))
            wake_up_interruptible( &nd->initialize_wait );
        if ( !tty_hung_up_p(filp) )
            interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
        if ( info->flags & ASYNC_HUP_NOTIFY )
            return(-EAGAIN);
        else
            return(-ERESTARTSYS);
#else
        return(-EAGAIN);
#endif
    }
#endif
    if ( info->flags & ASYNC_INITIALIZED )
    {
        clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
        if ( waitqueue_active(&nd->initialize_wait))
            wake_up_interruptible( &nd->initialize_wait );
        return(0);
    }

    page = GET_FPAGE(GFP_KERNEL);
    if ( !page )
    {
        clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
        if ( waitqueue_active(&nd->initialize_wait))
            wake_up_interruptible( &nd->initialize_wait );
        DBGPRINT(MX_DEBUG_ERROR, "allocate page failed\n");
        return(-ENOMEM);
    }

    if (!(nd->flag & NPREAL_NET_TTY_INUSED))
    {
        nd->cmd_buffer[0] = 0;
        npreal_wait_and_set_command(nd,NPREAL_LOCAL_COMMAND_SET,LOCAL_CMD_TTY_USED);
        nd->cmd_buffer[2] = 0;
        nd->cmd_ready = 1;
        if ( waitqueue_active(&nd->select_ex_wait))
        {
            wake_up_interruptible( &nd->select_ex_wait );
        }
        if (npreal_wait_command_completed(nd,
                                          NPREAL_LOCAL_COMMAND_SET,
                                          LOCAL_CMD_TTY_USED,
                                          NPREAL_CMD_TIMEOUT, // Scott MAX_SCHEDULE_TIMEOUT,
                                          rsp_buffer,
                                          &rsp_length) != 0)
        {

            DBGPRINT(MX_DEBUG_ERROR, "wait for LOCAL_CMD_TTY_USED response failed\n");
            npreal_wait_and_set_command(nd,NPREAL_LOCAL_COMMAND_SET,LOCAL_CMD_TTY_UNUSED);
            nd->cmd_buffer[2] = 0;
            nd->cmd_ready = 1;
            if ( waitqueue_active(&nd->select_ex_wait))
            {
                wake_up_interruptible( &nd->select_ex_wait );
            }
            npreal_wait_command_completed(nd,
                                          NPREAL_LOCAL_COMMAND_SET,
                                          LOCAL_CMD_TTY_UNUSED, // LOCAL_CMD_TTY_USED,
                                          NPREAL_CMD_TIMEOUT, // Scott MAX_SCHEDULE_TIMEOUT,
                                          rsp_buffer,
                                          &rsp_length);
            goto startup_err;
        }
        nd->flag |= NPREAL_NET_TTY_INUSED;
    }
    else
    {
        while ((nd->cmd_ready == 1)&&(cnt++ < 10))
        {
            current->state = TASK_INTERRUPTIBLE;
            schedule_timeout(HZ/100);
        }
    }
    /*
     * and set the speed of the serial port
     */
    nd->flag &= ~NPREAL_NET_DO_SESSION_RECOVERY;
    info->modem_status = 0;
    info->modem_control = 0;
    if (info->tty->termios->c_cflag & CBAUD)
        info->modem_control = UART_MCR_DTR | UART_MCR_RTS;
    if (npreal_port_init(info, 0) != 0)
    {
        DBGPRINT(MX_DEBUG_ERROR, "npreal_port_init() failed\n");
        goto startup_err;
    }
    if (info->type == PORT_16550A)
    {
        if (nd->server_type == CN2500)
            info->xmit_fifo_size = 64;
        else
            info->xmit_fifo_size = 16;
    }
    else
        info->xmit_fifo_size = 1;
    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_TX_FIFO) < 0)
    {
        DBGPRINT(MX_DEBUG_ERROR, "Set ASPP_CMD_TX_FIFO failed\n");
        goto startup_err;
    }
    nd->cmd_buffer[2] = 1;
    nd->cmd_buffer[3] = info->xmit_fifo_size;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }
    rsp_length = sizeof(rsp_buffer);
    if (npreal_wait_command_completed(nd,
                                      NPREAL_ASPP_COMMAND_SET,
                                      ASPP_CMD_TX_FIFO,NPREAL_CMD_TIMEOUT,
                                      rsp_buffer,
                                      &rsp_length) != 0)
    {
        DBGPRINT(MX_DEBUG_ERROR, "Wait for ASPP_CMD_TX_FIFO response failed\n");
        goto startup_err;
    }


    if ( info->xmit_buf )
        free_page(page);
    else
        info->xmit_buf = (unsigned char *)page;

#if (LINUX_VERSION_CODE <  VERSION_CODE(2,1,0))
    if ( info->tty )
        clear_bit(TTY_IO_ERROR, &info->tty->flags);
#else
    if ( info->tty )
        test_and_clear_bit(TTY_IO_ERROR, &info->tty->flags);
#endif
    info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;

    info->flags |= ASYNC_INITIALIZED;
    clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
    if ( waitqueue_active(&nd->initialize_wait))
        wake_up_interruptible( &nd->initialize_wait );
    return(0);
startup_err:
    ;
    free_page(page);

    clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
    if ( waitqueue_active(&nd->initialize_wait))
        wake_up_interruptible( &nd->initialize_wait );
    return -EIO;
}

/*
 * This routine will shutdown a serial port; interrupts maybe disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */
static void npreal_shutdown(struct npreal_struct * info)
{
    struct nd_struct *nd = info->net_node;
    unsigned long   flags;

// Scott: 2005-09-18
// nd can't be null.
    if (!nd)
        DBGPRINT(MX_DEBUG_ERROR, "nd is null\n");

#if (LINUX_VERSION_CODE <  VERSION_CODE(2,1,0))
    while (set_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag))
    {
#else
    while (test_and_set_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag))
    {
#endif
        if ( signal_pending(current) )
            break;
        current->state = TASK_INTERRUPTIBLE;
        schedule_timeout(HZ/100);
    }


    if ( !(info->flags & ASYNC_INITIALIZED) )
    {
        goto shutdown_ok;
    }

    //down (&info->tx_semaphore);
    DOWN(info->tx_lock, flags);
    if ( info->xmit_buf )
    {
        free_page((unsigned long)info->xmit_buf);
        info->xmit_buf = 0;
    }
    //up (&info->tx_semaphore);
    UP(info->tx_lock, flags);

    if ( info->tty )
    {
        set_bit(TTY_IO_ERROR, &info->tty->flags);
        npreal_unthrottle(info->tty);
    }

    if (!info->tty || (info->tty->termios->c_cflag & HUPCL))
    {
        info->modem_control &= ~(UART_MCR_DTR | UART_MCR_RTS);
    }
    npreal_port_shutdown(info);
shutdown_ok:
    ;

    info->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CALLOUT_ACTIVE|ASYNC_INITIALIZED|ASYNC_CLOSING);
    down (&info->rx_semaphore);
    info->tty = 0;
    up (&info->rx_semaphore);
    clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
    wake_up_interruptible(&info->open_wait);
    wake_up_interruptible(&info->close_wait);
    if ( waitqueue_active(&nd->initialize_wait))
        wake_up_interruptible( &nd->initialize_wait );
}
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
static int npreal_port_init(struct npreal_struct *info,
                            struct termios *old_termios)
#else
static int npreal_port_init(struct npreal_struct *info,
                            struct ktermios *old_termios)
#endif
{
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
    struct 	termios	*termio;
#else
	struct 	ktermios	*termio;
#endif
    int32_t     baud,mode;
    int		baudIndex,modem_status;
    struct 	nd_struct	*nd;
    char rsp_buffer[8];
    int  rsp_length = sizeof(rsp_buffer);

    nd = info->net_node;
    if ( !info->tty || !nd)
    {
        DBGPRINT(MX_DEBUG_ERROR, "info->tty or nd is null\n");
        return -EIO;
    }
    if (!(termio = info->tty->termios))
    {
        DBGPRINT(MX_DEBUG_ERROR, "info->tty->termios is null\n");
        return -EIO;
    }

    mode = termio->c_cflag & CSIZE;
    if (mode == CS5)
        mode = ASPP_IOCTL_BITS5;
    else if (mode == CS6)
        mode = ASPP_IOCTL_BITS6;
    else if (mode == CS7)
        mode = ASPP_IOCTL_BITS7;
    else if (mode == CS8)
        mode = ASPP_IOCTL_BITS8;

    if (termio->c_cflag & CSTOPB)
        mode |= ASPP_IOCTL_STOP2;
    else
        mode |= ASPP_IOCTL_STOP1;

    if (termio->c_cflag & PARENB)
    {
#ifdef CMSPAR
        if (termio->c_cflag & CMSPAR)
            if (termio->c_cflag & PARODD)
                mode |= ASPP_IOCTL_MARK;
            else
                mode |= ASPP_IOCTL_SPACE;
        else
#endif
            if (termio->c_cflag & PARODD)
                mode |= ASPP_IOCTL_ODD;
            else
                mode |= ASPP_IOCTL_EVEN;
    }
    else
        mode |= ASPP_IOCTL_NONE;


    switch ( termio->c_cflag & (CBAUD|CBAUDEX))
    {
    case B921600:
        baud = 921600L;
        baudIndex = ASPP_IOCTL_B921600;
        break;
    case B460800:
        baud = 460800;
        baudIndex = ASPP_IOCTL_B460800;
        break;
    case B230400:
        baud = 230400L;
        baudIndex = ASPP_IOCTL_B230400;
        break;
    case B115200:
        baud = 115200L;
        baudIndex = ASPP_IOCTL_B115200;
        break;
    case B57600:
        baud = 57600L;
        baudIndex = ASPP_IOCTL_B57600;
        break;
    case B38400:
        baud = 38400L;
        baudIndex = ASPP_IOCTL_B38400;
        if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI )
        {
            baud = 57600L;
            baudIndex = ASPP_IOCTL_B57600;
        }
        if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI )
        {
            baud = 115200L;
            baudIndex = ASPP_IOCTL_B115200;
        }

#ifdef ASYNC_SPD_SHI
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
        {
            baud = 230400L;
            baudIndex = ASPP_IOCTL_B230400;
        }
#endif

#ifdef ASYNC_SPD_WARP
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
        {
            baud = 460800L;
            baudIndex = ASPP_IOCTL_B460800;
        }
#endif
        break;
    case B19200:
        baud = 19200L;
        baudIndex = ASPP_IOCTL_B19200;
        break;
    case B9600:
        baud = 9600L;
        baudIndex = ASPP_IOCTL_B9600;
        break;
    case B4800:
        baud = 4800L;
        baudIndex = ASPP_IOCTL_B4800;
        break;
    case B2400:
        baud = 2400L;
        baudIndex = ASPP_IOCTL_B2400;
        break;
    case B1800:
        baud = 1800L;
        baudIndex = 0xff;
        break;
    case B1200:
        baud = 1200L;
        baudIndex = ASPP_IOCTL_B1200;
        break;
    case B600:
        baud = 600L;
        baudIndex = ASPP_IOCTL_B600;
        break;
    case B300:
        baud = 300L;
        baudIndex = ASPP_IOCTL_B300;
        break;
    case B200:
        baud = 200L;
        baudIndex = 0xff;
        break;
    case B150:
        baud = 150L;
        baudIndex = ASPP_IOCTL_B150;
        break;
    case B134:
        baud = 134L;
        baudIndex = ASPP_IOCTL_B134;
        break;
    case B110:
        baud = 110L;
        baudIndex = ASPP_IOCTL_B110;
        break;
    case B75:
        baud = 75L;
        baudIndex = ASPP_IOCTL_B75;
        break;
    case B50:
        baud = 50L;
        baudIndex = ASPP_IOCTL_B50;
        break;
    default:
        baud = 0;
        baudIndex = 0xff;
    }
    if (baud > 921600L)
    {
        termio->c_cflag &= ~(CBAUD|CBAUDEX);
        termio->c_cflag |=
            old_termios->c_cflag &(CBAUD|CBAUDEX);
    }
    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_PORT_INIT) < 0)
    {
        DBGPRINT(MX_DEBUG_ERROR, "set ASPP_CMD_PORT_INIT failed\n");
        return (-EIO);
    }
    nd->cmd_buffer[2] = 8;
//
// baud rate
//
    nd->cmd_buffer[3] = baudIndex;
//
// mode
//
    nd->cmd_buffer[4] = mode;
//
// line control
//
    if (info->modem_control & UART_MCR_DTR)
        nd->cmd_buffer[5] = 1;
    else
        nd->cmd_buffer[5] = 0;
    if (info->modem_control & UART_MCR_RTS)
        nd->cmd_buffer[6] = 1;
    else
        nd->cmd_buffer[6] = 0;
//
// flow control
//
    if (termio->c_cflag & CRTSCTS)
    {
        nd->cmd_buffer[7] = 1;
        nd->cmd_buffer[8] = 1;
    }
    else
    {
        nd->cmd_buffer[7] = 0;
        nd->cmd_buffer[8] = 0;
    }
    if (termio->c_iflag & IXON)
    {
        nd->cmd_buffer[9] = 1;
    }
    else
    {
        nd->cmd_buffer[9] = 0;
    }
    if (termio->c_iflag & IXOFF)
    {
        nd->cmd_buffer[10] = 1;
    }
    else
    {
        nd->cmd_buffer[10] = 0;
    }
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
        wake_up_interruptible( &nd->select_ex_wait );
    if (npreal_wait_command_completed(nd,
                                      NPREAL_ASPP_COMMAND_SET,
                                      ASPP_CMD_PORT_INIT,NPREAL_CMD_TIMEOUT,
                                      rsp_buffer,
                                      &rsp_length))
    {
        DBGPRINT(MX_DEBUG_ERROR, "wait ASPP_CMD_PORT_INIT response failed\n");
        return(-EIO);
    }
    if (rsp_length != 6)
    {
        DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_PORT_INIT response1\n");
        return(-EIO);
    }
    if (rsp_buffer[2] != 3)
    {
        DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_PORT_INIT response2\n");
        return(-EIO);
    }
    modem_status = 0;

    if (((unsigned char)rsp_buffer[3]==0xff) &&
            ((unsigned char)rsp_buffer[4]==0xff) &&
            ((unsigned char)rsp_buffer[5]==0xff))
    {
        termio->c_cflag &= ~(CBAUD|CBAUDEX);
        termio->c_cflag |=
            old_termios->c_cflag &(CBAUD|CBAUDEX);
    }
    else
    {
        if (rsp_buffer[3])
            modem_status |= UART_MSR_DSR;
        if (rsp_buffer[4])
            modem_status |= UART_MSR_CTS;
        if (rsp_buffer[5])
        {
            modem_status |= UART_MSR_DCD;
        }
    }

    npreal_check_modem_status(info,modem_status);

    if ((baudIndex == 0xff)&&(baud != 0))
    {
        if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_SETBAUD) < 0)
        {
            DBGPRINT(MX_DEBUG_ERROR, "set ASPP_CMD_SETBAUD failed\n");
            return(-EIO);
        }
        nd->cmd_buffer[2] = 4;
        memcpy(&nd->cmd_buffer[3],&baud,4);
        nd->cmd_ready = 1;
        if ( waitqueue_active(&nd->select_ex_wait))
            wake_up_interruptible( &nd->select_ex_wait );
        rsp_length = sizeof (rsp_buffer);
        if (npreal_wait_command_completed(nd,
                                          NPREAL_ASPP_COMMAND_SET,
                                          ASPP_CMD_SETBAUD,NPREAL_CMD_TIMEOUT,
                                          rsp_buffer,
                                          &rsp_length))
        {
            DBGPRINT(MX_DEBUG_ERROR, "wait ASPP_CMD_SETBAUD response failed\n");
            return(-EIO);
        }
        if (rsp_length != 4)
        {
            DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_SETBAUD response1\n");
            return(-EIO);
        }
        if ((rsp_buffer[2] != 'O') ||
                (rsp_buffer[3] != 'K') )
        {
            DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_SETBAUD response2\n");
            return(-EIO);
        }

    }

    if (termio->c_iflag & (IXON | IXOFF))
    {
        if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_XONXOFF))
        {
            DBGPRINT(MX_DEBUG_ERROR, "set ASPP_CMD_XONXOFF failed\n");
            return -EIO;
        }
        nd->cmd_buffer[2] = 2;
        nd->cmd_buffer[3] = termio->c_cc[VSTART];
        nd->cmd_buffer[4] = termio->c_cc[VSTOP];
        nd->cmd_ready = 1;
        if ( waitqueue_active(&nd->select_ex_wait))
            wake_up_interruptible( &nd->select_ex_wait );
        rsp_length = sizeof (rsp_buffer);
        if (npreal_wait_command_completed(nd,
                                          NPREAL_ASPP_COMMAND_SET,
                                          ASPP_CMD_XONXOFF,NPREAL_CMD_TIMEOUT,
                                          rsp_buffer,
                                          &rsp_length))
        {
            DBGPRINT(MX_DEBUG_ERROR, "wait ASPP_CMD_XONXOFF response failed\n");
            return(-EIO);
        }
        if (rsp_length != 4)
        {
            DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_XONXOFF response1\n");
            return(-EIO);
        }
        if ((rsp_buffer[2] != 'O') ||
                (rsp_buffer[3] != 'K') )
        {
            DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_XONXOFF response2\n");
            return(-EIO);
        }

    }

    if ( termio->c_cflag & CLOCAL )
    {
        info->flags &= ~ASYNC_CHECK_CD;
    }
    else
    {
        info->flags |= ASYNC_CHECK_CD;
    }
    if ( !info->tty)
    {
        DBGPRINT(MX_DEBUG_ERROR, "info->tty is null\n");
        return -EIO;
    }
    return(0);
}

#if 1 // Scott
static int npreal_port_shutdown(struct npreal_struct *info)
{
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
    struct 	termios	*termio;
#else
	struct 	ktermios	*termio;
#endif
    struct 	nd_struct	*nd;
    char rsp_buffer[8];
    int  rsp_length = sizeof(rsp_buffer);

    nd = info->net_node;
    if ( !info->tty || !nd)
    {
        return -EIO;
    }
    if (!(termio = info->tty->termios))
    {
        return -EIO;
    }

    nd->cmd_buffer[0] = NPREAL_LOCAL_COMMAND_SET;
    nd->cmd_buffer[1] = LOCAL_CMD_TTY_UNUSED;
    nd->cmd_buffer[2] = 0;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
        wake_up_interruptible( &nd->select_ex_wait );

    npreal_wait_command_completed(nd,
                                  NPREAL_LOCAL_COMMAND_SET,
                                  LOCAL_CMD_TTY_UNUSED,NPREAL_CMD_TIMEOUT,
                                  rsp_buffer,
                                  &rsp_length);

    nd->flag &= ~NPREAL_NET_TTY_INUSED;
    return(0);
}
#else
static int npreal_port_shutdown(struct npreal_struct *info)
{
    struct 	termios	*termio;
    int32_t	baud,mode;
    int		baudIndex;
    struct 	nd_struct	*nd;

    nd = info->net_node;
    if ( !info->tty || !nd)
    {
        return -EIO;
    }
    if (!(termio = info->tty->termios))
    {
        return -EIO;
    }

    mode = termio->c_cflag & CSIZE;
    if (mode == CS5)
        mode = ASPP_IOCTL_BITS5;
    else if (mode == CS6)
        mode = ASPP_IOCTL_BITS6;
    else if (mode == CS7)
        mode = ASPP_IOCTL_BITS7;
    else if (mode == CS8)
        mode = ASPP_IOCTL_BITS8;

    if (termio->c_cflag & CSTOPB)
        mode |= ASPP_IOCTL_STOP2;
    else
        mode |= ASPP_IOCTL_STOP1;

    if (termio->c_cflag & PARENB)
    {
        if (termio->c_cflag & PARODD)
            mode |= ASPP_IOCTL_ODD;
        else
            mode |= ASPP_IOCTL_EVEN;
    }
    else
        mode |= ASPP_IOCTL_NONE;


    switch ( termio->c_cflag & (CBAUD|CBAUDEX))
    {
    case B921600:
        baud = 921600L;
        baudIndex = ASPP_IOCTL_B921600;
        break;
    case B460800:
        baud = 460800;
        baudIndex = ASPP_IOCTL_B460800;
        break;
    case B230400:
        baud = 230400L;
        baudIndex = ASPP_IOCTL_B230400;
        break;
    case B115200:
        baud = 115200L;
        baudIndex = ASPP_IOCTL_B115200;
        break;
    case B57600:
        baud = 57600L;
        baudIndex = ASPP_IOCTL_B57600;
        break;
    case B38400:
        baud = 38400L;
        baudIndex = ASPP_IOCTL_B38400;
        if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI )
        {
            baud = 57600L;
            baudIndex = ASPP_IOCTL_B57600;
        }
        if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI )
        {
            baud = 115200L;
            baudIndex = ASPP_IOCTL_B115200;
        }

#ifdef ASYNC_SPD_SHI
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
        {
            baud = 230400L;
            baudIndex = ASPP_IOCTL_B230400;
        }
#endif

#ifdef ASYNC_SPD_WARP
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
        {
            baud = 460800L;
            baudIndex = ASPP_IOCTL_B460800;
        }
#endif
        break;
    case B19200:
        baud = 19200L;
        baudIndex = ASPP_IOCTL_B19200;
        break;
    case B9600:
        baud = 9600L;
        baudIndex = ASPP_IOCTL_B9600;
        break;
    case B4800:
        baud = 4800L;
        baudIndex = ASPP_IOCTL_B4800;
        break;
    case B2400:
        baud = 2400L;
        baudIndex = ASPP_IOCTL_B2400;
        break;
    case B1800:
        baud = 1800L;
        baudIndex = 0xff;
        break;
    case B1200:
        baud = 1200L;
        baudIndex = ASPP_IOCTL_B1200;
        break;
    case B600:
        baud = 600L;
        baudIndex = ASPP_IOCTL_B600;
        break;
    case B300:
        baud = 300L;
        baudIndex = ASPP_IOCTL_B300;
        break;
    case B200:
        baud = 200L;
        baudIndex = 0xff;
        break;
    case B150:
        baud = 150L;
        baudIndex = ASPP_IOCTL_B150;
        break;
    case B134:
        baud = 134L;
        baudIndex = ASPP_IOCTL_B134;
        break;
    case B110:
        baud = 110L;
        baudIndex = ASPP_IOCTL_B110;
        break;
    case B75:
        baud = 75L;
        baudIndex = ASPP_IOCTL_B75;
        break;
    case B50:
        baud = 50L;
        baudIndex = 0xff;
        break;
        break;
    default:
        baud = 0;
        baudIndex = 0xff;
    }
#if 0
    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_PORT_INIT) < 0)
    {
        return (-EIO);
    }
#endif
    nd->cmd_buffer[0] = NPREAL_ASPP_COMMAND_SET;
    nd->cmd_buffer[1] = ASPP_CMD_PORT_INIT;
    nd->cmd_buffer[2] = 8;
    //
    // baud rate
    //
    nd->cmd_buffer[3] = baudIndex;
    //
    // mode
    //
    nd->cmd_buffer[4] = mode;
    //
    // line control
    //
    if (info->modem_control & UART_MCR_DTR)
        nd->cmd_buffer[5] = 1;
    else
        nd->cmd_buffer[5] = 0;
    if (info->modem_control & UART_MCR_RTS)
        nd->cmd_buffer[6] = 1;
    else
        nd->cmd_buffer[6] = 0;
    // H/W flow control
    nd->cmd_buffer[7] = 0;
    nd->cmd_buffer[8] = 0;
    // Software flow control
    if (termio->c_iflag & IXON)
        nd->cmd_buffer[9] = 1;
    else
        nd->cmd_buffer[9] = 0;
    if (termio->c_iflag & IXOFF)
        nd->cmd_buffer[10] = 1;
    else
        nd->cmd_buffer[10] = 0;

    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
        wake_up_interruptible( &nd->select_ex_wait );
    //
    // We don't check result at this moment,because it will take time and then
    // next open may fail
    //
    /*
    if (npreal_wait_command_completed(nd,
    NPREAL_ASPP_COMMAND_SET,
    ASPP_CMD_PORT_INIT,NPREAL_CMD_TIMEOUT,
    rsp_buffer,
    &rsp_length)) {
    return(-EIO);
    }
    if (rsp_length != 6) {
    return(-EIO);
    }
    if (rsp_buffer[2] != 3) {
    return(-EIO);
    }
    */
    return(0);
}
#endif

/*
 * ------------------------------------------------------------
 * friends of npreal_ioctl()
 * ------------------------------------------------------------
 */
static int npreal_get_serial_info(struct npreal_struct * info,
                                  struct serial_struct * retinfo)
{
    struct serial_struct	tmp;

    if ( !retinfo )
        return(-EFAULT);
    memset(&tmp, 0, sizeof(tmp));
    tmp.type = info->type;
    tmp.line = info->port;
    tmp.flags = info->flags;
    tmp.close_delay = info->close_delay;
    tmp.closing_wait = info->closing_wait;
    tmp.custom_divisor = info->custom_divisor;
    tmp.hub6 = 0;
    if (copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
        return(-EFAULT);
    return(0);
}

static int npreal_set_serial_info(struct npreal_struct * info,
                                  struct serial_struct * new_info)
{
    struct serial_struct	new_serial;
    unsigned int		flags;
    int			retval = 0;
    char	rsp_buffer[8];
    int	rsp_length;


    if ( !new_info)
        return(-EFAULT);
    if (copy_from_user(&new_serial, new_info, sizeof(new_serial)))
        return(-EFAULT);

    flags = info->flags & ASYNC_SPD_MASK;

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
    if ( !suser() )
    {
#else
    if ( !capable(CAP_SYS_ADMIN))
    {
#endif
        if ((new_serial.close_delay != info->close_delay) ||
                ((new_serial.flags & ~ASYNC_USR_MASK) !=
                 (info->flags & ~ASYNC_USR_MASK)) )
            return(-EPERM);
        info->flags = ((info->flags & ~ASYNC_USR_MASK) |
                       (new_serial.flags & ASYNC_USR_MASK));
    }
    else
    {
        /*
         * OK, past this point, all the error checking has been done.
         * At this point, we start making changes.....
         */
        info->flags = ((info->flags & ~ASYNC_FLAGS) |
                       (new_serial.flags & ASYNC_FLAGS));
        info->close_delay = new_serial.close_delay * HZ/100;
// Scott: 2005-07-08
// If user wants to set closing_wait to ASYNC_CLOSING_WAIT_NONE, don't modify the value,
// since it will be used as a flag indicating closing wait none.
        if (new_serial.closing_wait == ASYNC_CLOSING_WAIT_NONE)
            info->closing_wait = ASYNC_CLOSING_WAIT_NONE;
        else
            info->closing_wait = new_serial.closing_wait * HZ/100;
    }

    info->type = new_serial.type;
    if (info->type == PORT_16550A)
    {
        if (info->net_node)
        {
            if (info->net_node->server_type == CN2500)
                info->xmit_fifo_size = 64;
            else
                info->xmit_fifo_size = 16;
        }
        else
            info->xmit_fifo_size = 16;
    }
    else
        info->xmit_fifo_size = 1;
    if ( info->flags & ASYNC_INITIALIZED )
    {
        if ( flags != (info->flags & ASYNC_SPD_MASK) )
        {
            retval=npreal_port_init(info,0);
        }
        if (info->net_node)
        {
            npreal_wait_and_set_command(info->net_node,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_TX_FIFO);
            info->net_node->cmd_buffer[2] = 1;
            info->net_node->cmd_buffer[3] = info->xmit_fifo_size;
            info->net_node->cmd_ready = 1;
            if ( waitqueue_active(&info->net_node->select_ex_wait))
                wake_up_interruptible(&info->net_node->select_ex_wait);
            npreal_wait_command_completed(info->net_node,
                                          NPREAL_ASPP_COMMAND_SET,
                                          ASPP_CMD_TX_FIFO,
                                          NPREAL_CMD_TIMEOUT,
                                          rsp_buffer,
                                          &rsp_length);
        }
    }
    return(retval);
}

/*
 * npreal_get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 *	    is emptied.  On bus types like RS485, the transmitter must
 *	    release the bus after transmitting. This must be done when
 *	    the transmit shift register is empty, not be done when the
 *	    transmit holding register is empty.  This functionality
 *	    allows an RS485 driver to be written in user space.
 */
static int npreal_get_lsr_info(struct npreal_struct * info, unsigned int *value)
{
    unsigned int	result = 0;

    if (npreal_wait_oqueue(info,0) == 0)
        result  = TIOCSER_TEMT;
    put_to_user(result, value);
    return(0);
}


static void npreal_start_break(struct nd_struct *nd)
{
    char	rsp_buffer[8];
    int	rsp_length = sizeof (rsp_buffer);

    npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_START_BREAK);
    nd->cmd_buffer[2] = 0;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }

    if (npreal_wait_command_completed(nd,
                                      NPREAL_ASPP_COMMAND_SET,
                                      ASPP_CMD_START_BREAK,NPREAL_CMD_TIMEOUT,
                                      rsp_buffer,
                                      &rsp_length))
        return;
    if (rsp_length != 4)
        return;
    if ((rsp_buffer[2] != 'O') ||
            (rsp_buffer[3] != 'K') )
        return;
}

static void npreal_stop_break(struct nd_struct *nd)
{
    char	rsp_buffer[8];
    int	rsp_length = sizeof (rsp_buffer);

    npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_STOP_BREAK);
    nd->cmd_buffer[2] = 0;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
        wake_up_interruptible( &nd->select_ex_wait );
    rsp_length = sizeof(rsp_buffer);
    if (npreal_wait_command_completed(nd,
                                      NPREAL_ASPP_COMMAND_SET,
                                      ASPP_CMD_STOP_BREAK,NPREAL_CMD_TIMEOUT,
                                      rsp_buffer,
                                      &rsp_length))
        return;
    if (rsp_length != 4)
        return;
    if ((rsp_buffer[2] != 'O') ||
            (rsp_buffer[3] != 'K') )
        return;
}

static void npreal_break(struct tty_struct *ttyinfo, int break_state)
{
    struct npreal_struct *info;
    struct nd_struct  	*nd;

    if ( !ttyinfo  )
        return;

    info = (struct npreal_struct *)ttyinfo->driver_data;

// Scott: 2005-09-12
    if (!info)
        return;

    if (!(nd = info->net_node))
        return;

    if (break_state == -1)
    {
        npreal_start_break(nd);
    }
    else
    {
        npreal_stop_break(nd);
    }

}

/*
 * This routine sends a break character out the serial port.
 */
static void npreal_send_break(struct npreal_struct * info, int duration)
{
    struct	nd_struct	*nd;

    if (!(nd = info->net_node))
        return;

    npreal_start_break(nd);

    current->state = TASK_INTERRUPTIBLE;
    schedule_timeout(duration);

    npreal_stop_break(nd);

}


#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
static int npreal_tiocmget(struct tty_struct *tty, struct file *file)
{
    struct npreal_struct *info = (struct npreal_struct *) tty->driver_data;

    if (!info)
    {
        DBGPRINT(MX_DEBUG_ERROR, "info is null\n");
        return (-EINVAL);
    }

    if (PORTNO(tty) == NPREAL_PORTS)
        return (-ENOIOCTLCMD);
    if (tty->flags & (1 << TTY_IO_ERROR))
        return (-EIO);

    return ((info->modem_control & UART_MCR_RTS) ? TIOCM_RTS : 0) |
           ((info->modem_control & UART_MCR_DTR) ? TIOCM_DTR : 0) |
           ((info->modem_status  & UART_MSR_DCD) ? TIOCM_CAR : 0) |
           ((info->modem_status  & UART_MSR_RI)  ? TIOCM_RNG : 0) |
           ((info->modem_status  & UART_MSR_DSR) ? TIOCM_DSR : 0) |
           ((info->modem_status  & UART_MSR_CTS) ? TIOCM_CTS : 0);
}

static int npreal_tiocmset(struct tty_struct *tty, struct file *file,
                           unsigned int set, unsigned int clear)
{
    struct npreal_struct *info = (struct npreal_struct *) tty->driver_data;
    struct	nd_struct	*nd;

    if (!info)
    {
        DBGPRINT(MX_DEBUG_ERROR, "info is null\n");
        return (-EINVAL);
    }

    if (!(nd = info->net_node))
        return(-EINVAL);

    if (PORTNO(tty) == NPREAL_PORTS)
        return (-ENOIOCTLCMD);
    if (tty->flags & (1 << TTY_IO_ERROR))
        return (-EIO);

    if (set & TIOCM_RTS)
        info->modem_control |= UART_MCR_RTS;
    if (set & TIOCM_DTR)
        info->modem_control |= UART_MCR_DTR;

    if (clear & TIOCM_RTS)
        info->modem_control &= ~UART_MCR_RTS;
    if (clear & TIOCM_DTR)
        info->modem_control &= ~UART_MCR_DTR;

    return npreal_linectrl(nd,info->modem_control);
}

#else

static int npreal_get_modem_info(struct npreal_struct * info,
                                 unsigned int *value)
{
    unsigned int	result;

    result = ((info->modem_control & UART_MCR_RTS) ? TIOCM_RTS : 0) |
             ((info->modem_control & UART_MCR_DTR) ? TIOCM_DTR : 0) |
             ((info->modem_status  & UART_MSR_DCD) ? TIOCM_CAR : 0) |
             ((info->modem_status  & UART_MSR_RI)  ? TIOCM_RNG : 0) |
             ((info->modem_status  & UART_MSR_DSR) ? TIOCM_DSR : 0) |
             ((info->modem_status  & UART_MSR_CTS) ? TIOCM_CTS : 0);
    put_to_user(result, value);
    return(0);
}

static int npreal_set_modem_info(struct npreal_struct * info, unsigned int cmd,
                                 unsigned int *value)
{
    int		error;
    unsigned int	arg;
    struct	nd_struct	*nd;

    if (!(nd = info->net_node))
        return(-EINVAL);
    error = access_ok(VERIFY_READ, value, sizeof(int))?0:-EFAULT;
    if ( error )
        return(error);
    get_from_user(arg,value);
    switch ( cmd )
    {
    case TIOCMBIS:
        if ( arg & TIOCM_RTS )
            info->modem_control |= UART_MCR_RTS;
        if ( arg & TIOCM_DTR )
            info->modem_control |= UART_MCR_DTR;
        break;
    case TIOCMBIC:
        if ( arg & TIOCM_RTS )
            info->modem_control &= ~UART_MCR_RTS;
        if ( arg & TIOCM_DTR )
            info->modem_control &= ~UART_MCR_DTR;
        break;
    case TIOCMSET:
        info->modem_control =
            ((info->modem_control & ~(UART_MCR_RTS | UART_MCR_DTR)) |
             ((arg & TIOCM_RTS) ? UART_MCR_RTS : 0) |
             ((arg & TIOCM_DTR) ? UART_MCR_DTR : 0));
        break;
    default:
        return(-EINVAL);
    }
    return (npreal_linectrl(nd,info->modem_control));
}
#endif

#if ((LINUX_VERSION_CODE > VERSION_CODE(2,6,15)) || ((LINUX_VERSION_CODE == VERSION_CODE(2,6,15)) && defined(FEDORA)))
static void tty_buffer_free(struct tty_struct *tty, struct tty_buffer *b)
{
    /* Dumb strategy for now - should keep some stats */
    /* 	printk("Flip dispose %p\n", b); */
    if (b->size >= 512)
        kfree(b);
    else
    {
        b->next = tty->buf.free;
        tty->buf.free = b;
    }
}
#endif

/*
 * This routine is called out of the software interrupt to flush data
 * from the flip buffer to the line discipline.
 */
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))  
static void npreal_flush_to_ldisc(void *private_)
{
    struct npreal_struct *	info = (struct npreal_struct *)private_;
#else
static void npreal_flush_to_ldisc(struct work_struct *work)
{
    struct npreal_struct *	info = 
    	container_of(work, struct npreal_struct, process_flip_tqueue);
#endif
    struct tty_struct *	tty;
    int		count;

#if ((LINUX_VERSION_CODE > VERSION_CODE(2,6,15)) || ((LINUX_VERSION_CODE == VERSION_CODE(2,6,15)) && defined(FEDORA)))
    struct tty_ldisc *disc;
    struct tty_buffer *tbuf, *head;
    unsigned long 	flags;
    unsigned char	*fp;
    char		*cp;
#else
    unsigned char	*cp;
    char		*fp;
#endif
    if (!info)
        goto done;
#if ((LINUX_VERSION_CODE <= VERSION_CODE(2,6,15)) && !defined(FEDORA))
    down(&info->rx_semaphore);
#endif
    tty = info->tty;
#if ((LINUX_VERSION_CODE > VERSION_CODE(2,6,15)) || ((LINUX_VERSION_CODE == VERSION_CODE(2,6,15)) && defined(FEDORA)))
    disc = tty_ldisc_ref(tty);
    if (disc == NULL)	/*  !TTY_LDISC */
        return;
#endif
    if ( tty && (info->flags & ASYNC_INITIALIZED))
    {
#if ((LINUX_VERSION_CODE <= VERSION_CODE(2,6,15)) && !defined(FEDORA))
        if (tty->flip.buf_num)
        {
            cp = tty->flip.char_buf + TTY_FLIPBUF_SIZE;
            fp = tty->flip.flag_buf + TTY_FLIPBUF_SIZE;
            tty->flip.buf_num = 0;
            tty->flip.char_buf_ptr = tty->flip.char_buf;
            tty->flip.flag_buf_ptr = tty->flip.flag_buf;
        }
        else
        {
            cp = tty->flip.char_buf;
            fp = tty->flip.flag_buf;
            tty->flip.buf_num = 1;

            tty->flip.char_buf_ptr = tty->flip.char_buf + TTY_FLIPBUF_SIZE;
            tty->flip.flag_buf_ptr = tty->flip.flag_buf + TTY_FLIPBUF_SIZE;
        }
        count = tty->flip.count;
        tty->flip.count = 0;
        tty->ldisc.receive_buf(tty, cp, fp, count);
        //DBGPRINT(MX_DEBUG_TRACE, "flush %d bytes\n", count);
#else
        spin_lock_irqsave(&tty->buf.lock, flags);
        head = tty->buf.head;
        if (head != NULL)
        {
            tty->buf.head = NULL;
            for (;;)
            {
                count = head->commit - head->read;
                if (!count)
                {
                    if (head->next == NULL)
                        break;
                    tbuf = head;
                    head = head->next;
                    tty_buffer_free(tty, tbuf);
                    continue;
                }
                if (!tty->receive_room)
                {
                    schedule_delayed_work(&tty->buf.work, 1);
                    break;
                }
                if (count > tty->receive_room)
                    count = tty->receive_room;
                cp = head->char_buf_ptr + head->read;
                fp = head->flag_buf_ptr + head->read;
                head->read += count;
                spin_unlock_irqrestore(&tty->buf.lock, flags);
                disc->receive_buf(tty, cp, fp, count);
                spin_lock_irqsave(&tty->buf.lock, flags);
            }
            tty->buf.head = head;
        }
        spin_unlock_irqrestore(&tty->buf.lock, flags);
        tty_ldisc_deref(disc);
#endif
    }
#if ((LINUX_VERSION_CODE <= VERSION_CODE(2,6,15)) && !defined(FEDORA))
    up(&info->rx_semaphore);
#endif

done:
    ;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    MX_MOD_DEC;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
/*
 *  This function doesn't exist prior to the 2.1 kernels.
 */
static struct
            proc_dir_entry *npreal_create_proc_entry(
                const char *name,
                mode_t mode,
                struct proc_dir_entry *parent)
{
    struct proc_dir_entry *ent = NULL;
    const char *fn = name;
    int len;

    if (!parent)
        goto out;
    len = strlen(fn);

    ent = kmalloc(sizeof(struct proc_dir_entry) + len + 1, GFP_KERNEL);
    if (!ent)
        goto out;
    memset(ent, 0, sizeof(struct proc_dir_entry));
    memcpy(((char *) ent) + sizeof(*ent), fn, len + 1);
    ent->name = ((char *) ent) + sizeof(*ent);
    ent->namelen = len;

    if (mode & S_IFDIR)
    {
        mode |= S_IRUGO | S_IXUGO;
        ent->ops = &proc_dir_inode_operations;
        ent->nlink = 2;
    }
    else
    {
        if (mode == 0)
            mode |= S_IFREG | S_IRUGO;
        ent->nlink = 1;
    }
    ent->mode = mode;

    proc_register_dynamic(parent, ent);

out:
    return ent;
}

static void
npreal_remove_proc_entry ( struct proc_dir_entry *pde )
{
    if (!pde) return;

    proc_unregister(pde->parent, pde->low_ino);

    kfree(pde);
}

#else

static struct proc_dir_entry *
            npreal_create_proc_entry(
                const char *name,
                const mode_t mode,
                struct proc_dir_entry *parent)
{
    return( create_proc_entry( name, mode, parent ) );
}

static void
npreal_remove_proc_entry( struct proc_dir_entry *pde )
{
    if (!pde) return;

    remove_proc_entry(pde->name, pde->parent);
}

#endif

static int
npreal_net_open (
    struct inode *inode,
    struct file *file )
{
    struct nd_struct *nd;
    int     rtn = 0;

    struct proc_dir_entry *de;

    MX_MOD_INC;

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
    if ( !suser() )
    {
#else
    if ( !capable(CAP_SYS_ADMIN) )
    {
#endif
        rtn = -EPERM;
        goto done;
    }


    /*
     *  Make sure that the "private_data" field hasn't already been used.
     */
    if ( file->private_data )
    {
        rtn = -EINVAL;
        goto done;
    }



    /*
     *  Get the node pointer, and fail if it doesn't exist.
     */
    //de = (struct proc_dir_entry *)inode->u.generic_ip;
    /* Casper, 9-11-04
    	Don't get pointer link above. It fail on 2.6 kernel.
    	PDE macro is ok on 2.6.
    */

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    de = (struct proc_dir_entry *)inode->u.generic_ip;
#else
    de = PDE(inode);
#endif
    if ( !de )
    {
        rtn = -ENXIO;
        goto done;
    }


    nd = (struct nd_struct *)de->data;
    if ( !nd )
    {
        rtn = -ENXIO;
        goto done;
    }

    file->private_data = (void *)nd;

    /*
     *  This is an exclusive access device.  If it is already
     *  open, return an error.
     */

    /*
     *  Grab the NET lock.
     */
    down(&nd->semaphore);


    if ( nd->flag & NPREAL_NET_NODE_OPENED)
    {
        rtn = -EBUSY;
        goto unlock;
    }

    nd->flag |= NPREAL_NET_NODE_OPENED;
    nd->tx_ready = 0;
    nd->rx_ready = 1;
    nd->cmd_ready = 0;

unlock:

    /*
     *  Release the NET lock.
     */
    up( &nd->semaphore );
//	(struct nd_struct *)(file->private_data) = nd;
    file->private_data = (void*)nd;

done:

    if ( rtn )
    {
        MX_MOD_DEC;
    }
    return rtn;
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
static void
npreal_net_close (
    struct inode *inode,
    struct file *file )
#else
static int
npreal_net_close (
    struct inode *inode,
    struct file *file )
#endif
{
    struct nd_struct *nd;
    /*
     *  Get the node pointer, and quit if it doesn't exist.
     */
    nd = (struct nd_struct *)(file->private_data);
    if ( !nd )
    {
        goto done;
    }
    nd->flag  &= ~NPREAL_NET_NODE_OPENED;


done:
    file->private_data = NULL;
    MX_MOD_DEC;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
    return;
#else
    return(0);
#endif
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))

static int
npreal_net_select (
    struct inode *inode,
    struct file *file,
    int mode,
    select_table *table)
{
    unsigned int retval = 0;
    struct nd_struct *nd  = file->private_data;

    if (!nd)
    {
        DBGPRINT(MX_DEBUG_ERROR, "nd is null\n");
        return retval;
    }

    switch ( mode )
    {

    case SEL_IN:
        if ( nd->tx_ready )
        {
            retval = 1;            /* Conditionally readable */
            break;
        }

        select_wait( &nd->select_in_wait, table );

        break;

    case SEL_OUT:
        if ( nd->rx_ready )
        {
            retval = 1;            /* Conditionally readable */
            break;
        }
        select_wait( &nd->select_out_wait, table );

        break;

    case SEL_EX:
        if ( nd->cmd_ready )
        {
            retval = 1;            /* Conditionally readable */
            break;
        }
        select_wait( &nd->select_ex_wait, table );

        break;
    }

    return retval;
}

#else

static unsigned int
npreal_net_select (
    struct file *file,
    struct poll_table_struct *table)
{
    unsigned int retval = 0;
    struct nd_struct *nd  = file->private_data;

    if (!nd)
    {
        DBGPRINT(MX_DEBUG_ERROR, "nd is null\n");
        return retval;
    }

    poll_wait( file, &nd->select_in_wait, table );
    poll_wait( file, &nd->select_out_wait, table );
    poll_wait( file, &nd->select_ex_wait, table );

    if ( nd->tx_ready )
    {
        retval |= POLLIN | POLLRDNORM;
    }

    if ( nd->rx_ready )
        retval |= POLLOUT | POLLWRNORM;

    if ( nd->cmd_ready )
    {
        retval |= POLLPRI;
    }


    return retval;
}
#endif

static int
npreal_net_ioctl (
    struct inode	*inode,
    struct file	*file,
    unsigned int	cmd,
    unsigned long 	arg )
{
    struct nd_struct *nd  = file->private_data;
    int    rtn  = 0;
    int    size,len;

    if ( !nd )
    {
        rtn = -ENXIO;
        goto done;
    }

    size = _IOC_SIZE( cmd );
    switch (_IOC_NR(cmd))
    {
    case NPREAL_NET_CMD_RETRIEVE :
        if (!nd->cmd_ready)
        {
            rtn = -ENXIO;
            goto done;
        }
        if (nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)
            len = nd->do_session_recovery_len;
        else
            len = (int)nd->cmd_buffer[2] + 3;
        rtn = access_ok( VERIFY_WRITE, (void *)arg, len)?0:-EFAULT;
        if ( rtn )
        {
            goto done;
        }
        if (copy_to_user( (void *)arg, (void *)nd->cmd_buffer,len ))
        {
            rtn = -EFAULT;
            goto done;
        }
        nd->cmd_buffer[0] = 0;
        rtn = len;
        nd->cmd_ready = 0;
        break;

    case NPREAL_NET_CMD_RESPONSE :
    {
        unsigned char rsp_buffer[84];

        if (size < 2)
            goto done;
        if (size > 84)
            size = 84;

        rtn = access_ok( VERIFY_READ,  (void *)arg, size )?0:-EFAULT;
        if ( rtn )
        {
            goto done;
        }
        if (copy_from_user( (void *)rsp_buffer, (void *)arg,size))
        {
            rtn = -EFAULT;
            goto done;
        }

        if (rsp_buffer[0] == NPREAL_LOCAL_COMMAND_SET)
        {
            down(&nd->cmd_semaphore);
            memcpy(nd->rsp_buffer,rsp_buffer,size);
            nd->rsp_length = size;
            up(&nd->cmd_semaphore);
            if ( waitqueue_active(&nd->cmd_rsp_wait))
                wake_up_interruptible(&nd->cmd_rsp_wait);
            break;
        }

        down(&nd->cmd_semaphore);
        if (nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)
        {
            if (rsp_buffer[1] == ASPP_CMD_LINECTRL)
            {
                nd->flag &= ~ NPREAL_NET_DO_SESSION_RECOVERY;
                up(&nd->cmd_semaphore);
                break;
            }
            else if (rsp_buffer[1] == ASPP_CMD_PORT_INIT)
            {
                int state = 0;
                struct npreal_struct *info;

                up(&nd->cmd_semaphore);
                if (!(info=nd->tty_node))
                    break;
                if (size != 6)
                    break;
                if (rsp_buffer[2] != 3)
                    break;
                if (rsp_buffer[3])
                    state |= UART_MSR_DSR;
                if (rsp_buffer[4])
                    state |= UART_MSR_CTS;
                if (rsp_buffer[5])
                    state |= UART_MSR_DCD;
                npreal_check_modem_status(info,state);
            }
            else
            {
                up(&nd->cmd_semaphore);
                break;
            }
        }
        else
            up(&nd->cmd_semaphore);
        if (rsp_buffer[1] == ASPP_NOTIFY)
        {
            npreal_process_notify(nd,rsp_buffer,size);
        }
        else if (rsp_buffer[1] == ASPP_CMD_WAIT_OQUEUE)
        {
            if (size == 5)
            {
                memcpy(nd->rsp_buffer,rsp_buffer,size);
                nd->oqueue = rsp_buffer[4]*16 + rsp_buffer[3];
                nd->rsp_length = size;
                nd->wait_oqueue_responsed = 1;
                if ( waitqueue_active(&nd->cmd_rsp_wait))
                    wake_up_interruptible(&nd->cmd_rsp_wait);
            }
        }
        else
        {
            down(&nd->cmd_semaphore);
            memcpy(nd->rsp_buffer,rsp_buffer,size);
            nd->rsp_length = size;
            up(&nd->cmd_semaphore);
            if ( waitqueue_active(&nd->cmd_rsp_wait))
                wake_up_interruptible(&nd->cmd_rsp_wait);
        }

        break;
    }
    case NPREAL_NET_CONNECTED :
    {
        struct npreal_struct *info;

        if (!(info=nd->tty_node))
            break;
        if (nd->flag & NPREAL_NET_NODE_DISCONNECTED)
        {
            nd->flag &= ~NPREAL_NET_NODE_DISCONNECTED;
            nd->flag |= NPREAL_NET_NODE_CONNECTED;
            npreal_do_session_recovery(info);
        }
        break;
    }
    case NPREAL_NET_DISCONNECTED :
        nd->flag &= ~NPREAL_NET_NODE_CONNECTED;
        nd->flag |= NPREAL_NET_NODE_DISCONNECTED;
        if (waitqueue_active(&nd->cmd_rsp_wait)){
            nd->wait_oqueue_responsed = 1;
            wake_up_interruptible(&nd->cmd_rsp_wait);
        }
        break;

    case NPREAL_NET_GET_TTY_STATUS:
    {
        int	status;

        if (size != sizeof (status))
            goto done;

        rtn = access_ok( VERIFY_READ,  (void *)arg, size )?0:-EFAULT;
        if ( rtn )
        {
            goto done;
        }
        status = (nd->flag & NPREAL_NET_TTY_INUSED) ? 1 : 0;
        if (copy_to_user( (void *)arg, (void *)&status,size ))
        {
            rtn = -EFAULT;
            goto done;
        }
        break;

    }
    case NPREAL_NET_SETTING :
    {
        struct server_setting_struct settings;
        struct npreal_struct *info;

        if (!(info=nd->tty_node))
            break;

        if (size != sizeof (struct server_setting_struct))
            goto done;

        rtn = access_ok( VERIFY_READ,  (void *)arg, size )?0:-EFAULT;
        if ( rtn )
        {
            goto done;
        }
        if (copy_from_user( (void *)&settings, (void *)arg,size))
        {
            rtn = -EFAULT;
            goto done;
        }
        if ((settings.server_type == DE311) ||
                (settings.server_type == DE301) ||
                (settings.server_type == DE302) ||
                (settings.server_type == DE304) ||
                (settings.server_type == DE331) ||
                (settings.server_type == DE332) ||
                (settings.server_type == DE334) ||
                (settings.server_type == DE303) ||
                (settings.server_type == DE308) ||
                (settings.server_type == DE309) ||
                (settings.server_type == CN2100) ||
                (settings.server_type == CN2500))
            nd->server_type =  settings.server_type;
        if (settings.disable_fifo)
            info->type = PORT_16450;
        else
            info->type = PORT_16550A;
        if (info->type == PORT_16550A)
        {
            if (nd->server_type == CN2500)
                info->xmit_fifo_size = 64;
            else
                info->xmit_fifo_size = 16;
        }
        else
            info->xmit_fifo_size = 1;

        break;
    }
    default :
        break;
    }
done:

    return rtn;
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
static int
npreal_net_read (
    struct inode *inode,
    struct file *file,
    char *buf,
    int count)
#else
static ssize_t
npreal_net_read (
    struct file *file,
    char *buf,
    size_t count,
    loff_t *ppos )
#endif
{
    struct nd_struct *nd  = file->private_data;
    ssize_t  rtn = 0;
    int	 cnt;
    struct npreal_struct *info;
    unsigned long   flags;
    struct tty_struct *	tty;

    /*
     *  Get the node pointer, and quit if it doesn't exist.
     */

    if ( !nd )
    {
        rtn = -ENXIO;
        goto done;
    }

    if (!(info = (struct npreal_struct *)nd->tty_node))
    {
        rtn = -ENXIO;
        goto done;
    }

    tty = info->tty;
    if ( !tty )
    {
        rtn = -ENXIO;
        goto done;
    }

    if ( info->x_char )
    {
        rtn = 1;
        if (copy_to_user( buf, &info->x_char,rtn ))
        {
            rtn = -EFAULT;
            goto done;
        }
        info->x_char = 0;
        DOWN(info->tx_lock, flags);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        info->icount.tx++;
#endif
        UP(info->tx_lock, flags);
        goto done;
    }

    DOWN(info->tx_lock, flags);
    if (!info->xmit_buf || info->xmit_cnt <= 0)
    {
        rtn = 0;
        UP(info->tx_lock, flags);
        goto done;
    }
    UP(info->tx_lock, flags);

    while ( count )
    {
        cnt = MIN(count, MIN( info->xmit_cnt,
                              SERIAL_XMIT_SIZE - info->xmit_tail));
        if ( cnt <= 0 )
            break;
        if (copy_to_user( buf+rtn,info->xmit_buf + info->xmit_tail,cnt ))
        {
            rtn = -EFAULT;
            goto done;
        }
        rtn += cnt;
        count -= cnt;

        DOWN(info->tx_lock, flags);
        info->xmit_cnt -= cnt;
        info->xmit_tail += cnt;
        info->xmit_tail = info->xmit_tail & (SERIAL_XMIT_SIZE - 1);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        info->icount.tx += cnt;
#endif
        UP(info->tx_lock, flags);
    }
    if (info->xmit_cnt <= 0)
    {
        nd->tx_ready = 0;
    }
    else
    {
        nd->tx_ready = 1;
        if ( waitqueue_active(&nd->select_in_wait))
            wake_up_interruptible( &nd->select_in_wait );
    }

// Scott: 2005-09-14
// Comment out the following code to prevent softirq from happening.
#if 0
    if ( info->xmit_cnt < WAKEUP_CHARS )
    {
        set_bit(NPREAL_EVENT_TXLOW,&info->event);
        MXQ_TASK(&info->tqueue);
    }
#else
    if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
            tty->ldisc.write_wakeup )
        (tty->ldisc.write_wakeup)(tty);
    wake_up_interruptible(&tty->write_wait);
#endif
done:
    return rtn;
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
static int
npreal_net_write (
    struct inode *inode,
    struct file *file,
    const char *buf,
    int count)
#else
static ssize_t
npreal_net_write (
    struct file *file,
    const char *buf,
    size_t count,
    loff_t *ppos)
#endif
{
    struct nd_struct *nd  = file->private_data;
    ssize_t  rtn = 0;
    int cnt;
    struct npreal_struct *info;
    struct tty_struct *	tty;

    /*
     *  Get the node pointer, and quit if it doesn't exist.
     */

    if ( !buf )
    {
        rtn = count; /* throw it away*/
        goto done;
    }

    if ( !nd )
    {
        rtn = count; /* throw it away*/
        goto done;
    }

    if (!(info = (struct npreal_struct *)nd->tty_node))
    {
        rtn = count; /* throw it away*/
        goto done;
    }

    if (info->flags & ASYNC_CLOSING)
    {
        rtn = count; /* throw it away*/
        goto done;
    }

    down(&info->rx_semaphore);

    if (!(tty = info->tty))
    {
        rtn = count; /* throw it away*/
        up(&info->rx_semaphore);
        goto done;
    }
    if (test_bit(TTY_IO_ERROR, &tty->flags))
    {
        rtn = count; /* throw it away*/
        up(&info->rx_semaphore);
        goto done;
    }

    if ( !nd->rx_ready )
    {
        up(&info->rx_semaphore);
        DBGPRINT(MX_DEBUG_TRACE, "Port %d RX is not ready\n", info->port);
        goto done;
    }

    /*  The receive buffer will overrun,as the TTY_THRESHOLD_THROTTLE is 128*/
#if ((LINUX_VERSION_CODE <= VERSION_CODE(2,6,15)) && !defined(FEDORA))
    if ((cnt = MIN(count,TTY_FLIPBUF_SIZE-tty->flip.count)) <= 0)
    {
#else
    if ((cnt = tty_buffer_request_room(tty, count)) <= 0)
    {
#endif
        /*
        * Doing throttle here,because that it will spent times
        * for upper layer driver to throttle and the application 
        * may call write so many times but just can not write.
        	* If we are doing input canonicalization, and there are no
        	* pending newlines, let characters through without limit, so
        	* that erase characters will be handled.  Other excess
        	* characters will be beeped.
        	*/

        if (!tty->icanon || tty->canon_data)
        {
#if (LINUX_VERSION_CODE <  VERSION_CODE(2,1,0))
            if (!set_bit(TTY_THROTTLED,&tty->flags))
            {
#else
            if (!test_and_set_bit(TTY_THROTTLED,&tty->flags))
            {
#endif
                npreal_throttle(tty);
            }
        }
        up(&info->rx_semaphore);
        goto done;
    }
    if (!tty->icanon || tty->canon_data)
    {
        if ((cnt = MIN(cnt,(N_TTY_BUF_SIZE-1) - tty->read_cnt )) <= 0)
        {

            /*
            * Doing throttle here,because that it will spent times
            * for upper layer driver to throttle and the application 
            * may call write so many times but just can not write.
            	* If we are doing input canonicalization, and there are no
            	* pending newlines, let characters through without limit, so
            	* that erase characters will be handled.  Other excess
            	* characters will be beeped.
            	*/
#if (LINUX_VERSION_CODE <  VERSION_CODE(2,1,0))
            if (!set_bit(TTY_THROTTLED,&tty->flags))
            {
#else
            if (!test_and_set_bit(TTY_THROTTLED,&tty->flags))
            {
#endif
                npreal_throttle(tty);
            }
            up(&info->rx_semaphore);

            goto done;
        }
    }

#if ((LINUX_VERSION_CODE <= VERSION_CODE(2,6,15)) && !defined(FEDORA))
    if (copy_from_user( tty->flip.char_buf_ptr, buf,cnt ))
    {
#else
    if ((count = tty_insert_flip_string(tty, (unsigned char *)buf, cnt)))
    {
        tty_flip_buffer_push(tty);
#endif
        rtn = count; /* throw it away*/
        up(&info->rx_semaphore);
        goto done;
    }
#if ((LINUX_VERSION_CODE <= VERSION_CODE(2,6,15)) && !defined(FEDORA))
//	DBGPRINT(MX_DEBUG_TRACE, "write %d bytes (1st tty->flip.char_buf_ptr=0x%02X)\n", cnt, (unsigned char)(((char*)(tty->flip.char_buf_ptr))[0]));
    tty->flip.count += cnt;
    rtn = cnt;
    tty->flip.char_buf_ptr += cnt;
//	DBGPRINT(MX_DEBUG_TRACE, "flip.count=%d\n", tty->flip.count);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
    info->icount.rx += cnt;
#endif
    memset(tty->flip.flag_buf_ptr,TTY_NORMAL,cnt);
    tty->flip.flag_buf_ptr += cnt;
#endif
    up(&info->rx_semaphore);
    MXQ_TASK(&info->process_flip_tqueue);

done:

    return rtn;
}

static int
npreal_wait_and_set_command(
    struct nd_struct *nd,
    char command_set,
    char command)
{

    unsigned long	et;

    if ((command_set != NPREAL_LOCAL_COMMAND_SET)&&((nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)||(nd->flag&NPREAL_NET_NODE_DISCONNECTED)))
    {
        return (-1);
    }

    et = jiffies + NPREAL_CMD_TIMEOUT;
    while (1)
    {
        down (&nd->cmd_semaphore);
        if (nd->cmd_buffer[0] == 0)
        {
            nd->cmd_buffer[0] = command_set;
            nd->cmd_buffer[1] = command;
            up (&nd->cmd_semaphore);
            return (0);
        }
        else if ((jiffies >= et)||signal_pending(current))
        { // timeout
            nd->cmd_buffer[0] = command_set;
            nd->cmd_buffer[1] = command;
            up (&nd->cmd_semaphore);
            return (0);
        }
        else
        {
            up (&nd->cmd_semaphore);
            current->state = TASK_INTERRUPTIBLE;
            schedule_timeout(1);
        }
    }
}

static int
npreal_wait_command_completed(
    struct nd_struct *nd,
    char command_set,
    char command,
    long timeout,
    char *rsp_buf,
    int  *rsp_len)
{
    long	et = 0;

    if ((command_set != NPREAL_LOCAL_COMMAND_SET)&&((nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)||(nd->flag&NPREAL_NET_NODE_DISCONNECTED)))
    {
        return (-1);
    }

    if (*rsp_len <= 0)
        return (-1);

    while (1)
    {
        down(&nd->cmd_semaphore);
        if ((nd->rsp_length)&&(nd->rsp_buffer[0] == command_set)&&(nd->rsp_buffer[1] == command))
        {
            if (nd->rsp_length > *rsp_len)
                return (-1);
            *rsp_len = nd->rsp_length;
            memcpy(rsp_buf,nd->rsp_buffer,*rsp_len);
            nd->rsp_length = 0;
            up(&nd->cmd_semaphore);
            return (0);
        }
        else if ( timeout > 0)
        {
            up(&nd->cmd_semaphore);
            if ( signal_pending(current) )
            {
                return(-1);
            }
            if (timeout != MAX_SCHEDULE_TIMEOUT)
                et = jiffies + timeout;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
            interruptible_sleep_on_timeout(&nd->cmd_rsp_wait,timeout);
#else
            current->timeout = timeout;
            interruptible_sleep_on(&nd->cmd_rsp_wait);

#endif
            if (timeout != MAX_SCHEDULE_TIMEOUT)
                timeout = et - jiffies;
        }
        else
        { // timeout
            up(&nd->cmd_semaphore);
            return (-1);
        }
    }
}

static void
npreal_process_notify(
    struct nd_struct *nd,
    char *rsp_buffer,
    int rsp_length)
{
    int	state;
    struct npreal_struct	*info = nd->tty_node;

    if (!info)
        return;
    if (rsp_length != 5)
        return;
    if (rsp_buffer[2] & ASPP_NOTIFY_MSR_CHG)
    {
        state = 0;
        if (rsp_buffer[3] & 0x10)
            state |= UART_MSR_CTS;
        if (rsp_buffer[3] & 0x20)
            state |= UART_MSR_DSR;
        if (rsp_buffer[3] & 0x80)
            state |= UART_MSR_DCD;
        npreal_check_modem_status(info,state);

    }
    if (rsp_buffer[2] & ASPP_NOTIFY_BREAK)
    {
        struct tty_struct	*tty;

        down (&info->rx_semaphore);
        if (!(tty= info->tty))
        {
            up (&info->rx_semaphore);
            return;
        }
        tty_insert_flip_char(tty, 0, TTY_BREAK);
        up (&info->rx_semaphore);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        info->icount.rx ++;
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
        info->icount.brk++;
#endif
        MXQ_TASK(&info->process_flip_tqueue);

        if ( info->flags & ASYNC_SAK )
        {
            do_SAK(info->tty);
        }
    }
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
    if (rsp_buffer[2] & ASPP_NOTIFY_PARITY)
        info->icount.parity++;
    if (rsp_buffer[2] & ASPP_NOTIFY_FRAMING)
        info->icount.frame++;
    if ((rsp_buffer[2] & ASPP_NOTIFY_SW_OVERRUN) ||
            (rsp_buffer[2] & ASPP_NOTIFY_HW_OVERRUN))
        info->icount.overrun++;
#endif

}

static void
npreal_do_session_recovery(struct npreal_struct *info)
{
    struct tty_struct *	tty;
    struct nd_struct *	nd;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))
    struct termios *	termio;
#else
	struct ktermios *	termio;
#endif
    int32_t    		    baud,mode;
    int		baudIndex,index;

    tty = info->tty;
    nd = info->net_node;
    if ( !tty || !nd)
        return;
    if (!(nd->flag & NPREAL_NET_TTY_INUSED))
        return;
    if (nd->flag&ASYNC_INITIALIZED)
    {
        if (!(termio = info->tty->termios))
            return;
    }
    else
    {
        if (!(termio = &info->normal_termios))
            return;
    }
    down (&nd->cmd_semaphore);
    mode = termio->c_cflag & CSIZE;
    if (mode == CS5)
        mode = ASPP_IOCTL_BITS5;
    else if (mode == CS6)
        mode = ASPP_IOCTL_BITS6;
    else if (mode == CS7)
        mode = ASPP_IOCTL_BITS7;
    else if (mode == CS8)
        mode = ASPP_IOCTL_BITS8;

    if (termio->c_cflag & CSTOPB)
        mode |= ASPP_IOCTL_STOP2;
    else
        mode |= ASPP_IOCTL_STOP1;

    if (termio->c_cflag & PARENB)
    {
        if (termio->c_cflag & PARODD)
            mode |= ASPP_IOCTL_ODD;
        else
            mode |= ASPP_IOCTL_EVEN;
    }
    else
        mode |= ASPP_IOCTL_NONE;

    switch ( termio->c_cflag & (CBAUD|CBAUDEX))
    {
    case B921600:
        baud = 921600L;
        baudIndex = ASPP_IOCTL_B921600;
        break;
    case B460800:
        baud = 460800;
        baudIndex = ASPP_IOCTL_B460800;
        break;
    case B230400:
        baud = 230400L;
        baudIndex = ASPP_IOCTL_B230400;
        break;
    case B115200:
        baud = 115200L;
        baudIndex = ASPP_IOCTL_B115200;
        break;
    case B57600:
        baud = 57600L;
        baudIndex = ASPP_IOCTL_B57600;
        break;
    case B38400:
        baud = 38400L;
        baudIndex = ASPP_IOCTL_B38400;
        if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI )
        {
            baud = 57600L;
            baudIndex = ASPP_IOCTL_B57600;
        }
        if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI )
        {
            baud = 115200L;
            baudIndex = ASPP_IOCTL_B115200;
        }

#ifdef ASYNC_SPD_SHI
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
        {
            baud = 230400L;
            baudIndex = ASPP_IOCTL_B230400;
        }
#endif

#ifdef ASYNC_SPD_WARP
        if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
        {
            baud = 460800L;
            baudIndex = ASPP_IOCTL_B460800;
        }
#endif
        break;
    case B19200:
        baud = 19200L;
        baudIndex = ASPP_IOCTL_B19200;
        break;
    case B9600:
        baud = 9600L;
        baudIndex = ASPP_IOCTL_B9600;
        break;
    case B4800:
        baud = 4800L;
        baudIndex = ASPP_IOCTL_B4800;
        break;
    case B2400:
        baud = 2400L;
        baudIndex = ASPP_IOCTL_B2400;
        break;
    case B1800:
        baud = 1800L;
        baudIndex = 0xff;
        break;
    case B1200:
        baud = 1200L;
        baudIndex = ASPP_IOCTL_B1200;
        break;
    case B600:
        baud = 600L;
        baudIndex = ASPP_IOCTL_B600;
        break;
    case B300:
        baud = 300L;
        baudIndex = ASPP_IOCTL_B300;
        break;
    case B200:
        baud = 200L;
        baudIndex = 0xff;
        break;
    case B150:
        baud = 150L;
        baudIndex = ASPP_IOCTL_B150;
        break;
    case B134:
        baud = 134L;
        baudIndex = ASPP_IOCTL_B134;
        break;
    case B110:
        baud = 110L;
        baudIndex = ASPP_IOCTL_B110;
        break;
    case B75:
        baud = 75L;
        baudIndex = ASPP_IOCTL_B75;
        break;
    case B50:
        baud = 50L;
        baudIndex = 0xff;
        break;
        break;
    default:
        baud = 0;
        baudIndex = 0xff;
    }
    nd->cmd_buffer[2] = 8;
//
// baud rate
//
    nd->cmd_buffer[3] = baudIndex;
//
// mode
//
    nd->cmd_buffer[4] = mode;
//
// line control
//
    if (info->modem_control & UART_MCR_DTR)
        nd->cmd_buffer[5] = 1;
    else
        nd->cmd_buffer[5] = 0;
    if (info->modem_control & UART_MCR_RTS)
        nd->cmd_buffer[6] = 1;
    else
        nd->cmd_buffer[6] = 0;
//
// flow control
//
    if (info->flags & ASYNC_INITIALIZED)
    {
        if (termio->c_cflag & CRTSCTS)
        {
            nd->cmd_buffer[7] = 1;
            nd->cmd_buffer[8] = 1;
        }
        else
        {
            nd->cmd_buffer[7] = 0;
            nd->cmd_buffer[8] = 0;
        }
    }
    else
    {
        nd->cmd_buffer[7] = 0;
        nd->cmd_buffer[8] = 0;

    }

    if (termio->c_iflag & IXON)
        nd->cmd_buffer[9] = 1;
    else
        nd->cmd_buffer[9] = 0;
    if (termio->c_iflag & IXOFF)
        nd->cmd_buffer[10] = 1;
    else
        nd->cmd_buffer[10] = 0;
    nd->cmd_buffer[0] = NPREAL_ASPP_COMMAND_SET;
    nd->cmd_buffer[1] = ASPP_CMD_PORT_INIT;
    index = 11;

    if ((baudIndex == 0xff)&&(baud != 0))
    {
        nd->cmd_buffer[index+0] = ASPP_CMD_SETBAUD;
        nd->cmd_buffer[index+1] = 4;
        memcpy(&nd->cmd_buffer[index+2],&baud,4);
        index += 6;
    }

    if (termio->c_iflag & (IXON | IXOFF))
    {
        nd->cmd_buffer[index+0] = ASPP_CMD_XONXOFF;
        nd->cmd_buffer[index+1] = 2;
        nd->cmd_buffer[index+2] = termio->c_cc[VSTART];
        nd->cmd_buffer[index+3] = termio->c_cc[VSTOP];
        index += 4;
    }
    nd->cmd_buffer[index+0] = ASPP_CMD_TX_FIFO;
    nd->cmd_buffer[index+1] = 1;
    nd->cmd_buffer[index+2] = info->xmit_fifo_size;
    index += 3;

    nd->cmd_buffer[index+0] = ASPP_CMD_LINECTRL;
    nd->cmd_buffer[index+1] = 2;
    if (info->modem_control & UART_MCR_DTR)
        nd->cmd_buffer[index+2] = 1;
    else
        nd->cmd_buffer[index+2] = 0;
    if (info->modem_control & UART_MCR_RTS)
        nd->cmd_buffer[index+3] = 1;
    else
        nd->cmd_buffer[index+3] = 0;
    index += 4;
    nd->do_session_recovery_len = index;
    nd->flag |= NPREAL_NET_DO_SESSION_RECOVERY;
    nd->cmd_ready = 1;
    up (&nd->cmd_semaphore);
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }
    return;

}

static long
npreal_wait_oqueue(
    struct npreal_struct * info,
    long timeout)
{
    struct	nd_struct	*nd;
    long	et = 0;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,1,0))
    long	st;
#endif
    uint32_t    tout;

    if (!(nd = info->net_node))
        return (-EIO);
    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_WAIT_OQUEUE) < 0)
        return (-EIO);
    if (timeout != MAX_SCHEDULE_TIMEOUT)
    {
        if (timeout < HZ/10)  // at least wait for 100 ms
            timeout = HZ/10;
        et = jiffies + timeout;
    }

    if (timeout != MAX_SCHEDULE_TIMEOUT)
        tout = (uint32_t)timeout;
    else
        tout = 0x7FFFFFFF;

    nd->cmd_buffer[2] = 4;
    memcpy(&nd->cmd_buffer[3],(void *)&tout,4);
    nd->wait_oqueue_responsed = 0;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
        wake_up_interruptible( &nd->select_ex_wait );
    while (nd->cmd_ready == 1)
    {
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
        interruptible_sleep_on_timeout(&nd->cmd_rsp_wait,1);
#else
        current->timeout = 1;
        interruptible_sleep_on(&nd->cmd_rsp_wait);

#endif
        if (timeout != MAX_SCHEDULE_TIMEOUT)
        {
            if (jiffies > et)
                return (-EIO);
        }

    }
    if (timeout != MAX_SCHEDULE_TIMEOUT)
        timeout += 10;
    nd->cmd_buffer[0] = 0;
    do
    {
        if (nd->wait_oqueue_responsed == 0)
        {
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
            timeout =
                interruptible_sleep_on_timeout(&nd->cmd_rsp_wait,timeout);
#else
            st = jiffies;
            current->timeout = timeout;
            interruptible_sleep_on(&nd->cmd_rsp_wait);
            timeout = jiffies - st;

#endif
            if (nd->wait_oqueue_responsed)
            {
                return (nd->oqueue);
            }
        }
        else
        {
            return (nd->oqueue);
        }
    }
    while (timeout > 0);
    return (-EIO);
}

static int
npreal_linectrl(
    struct nd_struct *nd,
    int modem_control)
{
    char rsp_buffer[8];
    int  rsp_length = sizeof(rsp_buffer);

    if (!nd)
        return (-EIO);
    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_LINECTRL) < 0)
        return (-EIO);
    nd->cmd_buffer[2] = 2;
    if (modem_control & UART_MCR_DTR)
        nd->cmd_buffer[3] = 1;
    else
        nd->cmd_buffer[3] = 0;
    if (modem_control & UART_MCR_RTS)
        nd->cmd_buffer[4] = 1;
    else
        nd->cmd_buffer[4] = 0;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }
    if (npreal_wait_command_completed(nd,
                                      NPREAL_ASPP_COMMAND_SET,
                                      ASPP_CMD_LINECTRL,NPREAL_CMD_TIMEOUT,
                                      rsp_buffer,
                                      &rsp_length))
        return(-EIO);
    if (rsp_length != 4)
        return(-EIO);
    if ((rsp_buffer[2] != 'O') ||
            (rsp_buffer[3] != 'K') )
        return(-EIO);
    return (0);
}

/*
Scott: 2005-08-11
ASPP command. This command pretend the serial port receives
XON (or XOFF) character.
*/
static int
npreal_setxon_xoff(struct npreal_struct * info, int cmd)
{
    char rsp_buffer[8];
    int  rsp_length = sizeof(rsp_buffer);
    struct	nd_struct	*nd;

    if (!(nd = info->net_node))
        return (-EIO);

    if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,cmd) < 0)
        return (-EIO);
    nd->cmd_buffer[2] = 0;
    nd->cmd_ready = 1;
    if ( waitqueue_active(&nd->select_ex_wait))
    {
        wake_up_interruptible( &nd->select_ex_wait );
    }
    if (npreal_wait_command_completed(nd,
                                      NPREAL_ASPP_COMMAND_SET,
                                      cmd,NPREAL_CMD_TIMEOUT,
                                      rsp_buffer,
                                      &rsp_length))
        return(-EIO);
    if (rsp_length != 4)
        return(-EIO);
    if ((rsp_buffer[2] != 'O') ||
            (rsp_buffer[3] != 'K') )
        return(-EIO);
    return (0);
}

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
module_init(npreal2_module_init);
module_exit(npreal2_module_exit);
#endif
