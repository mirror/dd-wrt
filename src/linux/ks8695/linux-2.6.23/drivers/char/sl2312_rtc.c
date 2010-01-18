
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/mc146818rtc.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sl2312.h>

#define RTC_VERSION		"0.10"
#define RTC_IS_OPEN		0x01	/* means /dev/rtc is in use	*/
#define RTC_TIMER_ON		0x02	/* missed irq timer active	*/

/* define EMAC base address */
#define RTC_PHYSICAL_BASE_ADDR	(SL2312_RTC_BASE) //0x25000000
#define RTC_BASE_ADDR		(IO_ADDRESS(RTC_PHYSICAL_BASE_ADDR))
#define RTC_GLOBAL_BASE_ADDR    (IO_ADDRESS(SL2312_GLOBAL_BASE)) 

#define RTC_READ(offset)        readl(RTC_BASE_ADDR+offset)
#define RTC_WRITE(offset,data)  writel(data,RTC_BASE_ADDR+offset)

/***************************************/
/* the offset address of RTC register */
/***************************************/
enum EMAC_REGISTER {
    RTC_SECOND      = 0x00,
    RTC_MINUTE      = 0x04,
    RTC_HOUR        = 0x08,
    RTC_DAYS        = 0x0c,
    RTC_ALARM_SECOND= 0x10,
    RTC_ALARM_MINUTE= 0x14,
    RTC_ALARM_HOUR  = 0x18,
    RTC_RECORD      = 0x1c,
    RTC_CR          = 0x20
};    


/*
 * rtc_status is never changed by rtc_interrupt, and ioctl/open/close is
 * protected by the big kernel lock. However, ioctl can still disable the timer
 * in rtc_status and then with del_timer after the interrupt has read
 * rtc_status but before mod_timer is called, which would then reenable the
 * timer (but you would need to have an awful timing before you'd trip on it)
 */
static unsigned long rtc_status = 0;	/* bitmapped status byte.	*/
static unsigned long rtc_irq_data = 0;	/* our output to the world	*/

static unsigned long epoch = 1970;	/* year corresponding to 0x00	*/

static const unsigned char days_in_mo[] = 
{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


static struct fasync_struct *rtc_async_queue;

static DECLARE_WAIT_QUEUE_HEAD(rtc_wait);

static ssize_t rtc_read(struct file *file, char *buf,
			size_t count, loff_t *ppos);

static int rtc_ioctl(struct inode *inode, struct file *file,
		     unsigned int cmd, unsigned long arg);

static void rtc_sw_reset(void);

static void get_rtc_time (struct rtc_time *rtc_tm);
static void get_rtc_alm_time (struct rtc_time *alm_tm);

static int rtc_read_proc(char *page, char **start, off_t off,
                         int count, int *eof, void *data);

void rtc_set_time_second(unsigned int second);
unsigned int rtc_get_time_second(void);

static void rtc_sw_reset(void)
{
    unsigned int reg_val;

    reg_val = readl(RTC_GLOBAL_BASE_ADDR+0x10) | 0x00000400;
    writel(reg_val,RTC_GLOBAL_BASE_ADDR+0x10);
    return;
}

static ssize_t rtc_read(struct file *file, char *buf,
			size_t count, loff_t *ppos)
{
//#if !RTC_IRQ
	return -EIO;
//#endif
}

static int rtc_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg)
{
	struct rtc_time wtime; 
//	unsigned int    i;

	switch (cmd) {
	case RTC_ALM_READ:	/* Read the present alarm time */
	{
		return -EINVAL;
/*
		get_rtc_alm_time(&wtime);
		break; 
*/
	}	
	case RTC_ALM_SET:	/* Store a time into the alarm */
	{
		return -EINVAL;
/*
		unsigned char hrs, min, sec;
		struct rtc_time alm_tm;

		if (copy_from_user(&alm_tm, (struct rtc_time*)arg,
				   sizeof(struct rtc_time)))
			return -EFAULT;

		hrs = alm_tm.tm_hour;
		min = alm_tm.tm_min;
		sec = alm_tm.tm_sec;

		if (hrs >= 24)
			hrs = 0xff;

		if (min >= 60)
			min = 0xff;

		if (sec >= 60)
			sec = 0xff;

		spin_lock_irq(&rtc_lock);
		RTC_WRITE(RTC_ALARM_HOUR,hrs);
		RTC_WRITE(RTC_ALARM_MINUTE,min);
		RTC_WRITE(RTC_ALARM_SECOND,sec);
		spin_unlock_irq(&rtc_lock);

		return 0;
*/
	}
	case RTC_RD_TIME:	/* Read the time/date from RTC	*/
	{
		memset(&wtime, 0, sizeof(wtime));
		get_rtc_time(&wtime);
		break;
	}
	case RTC_SET_TIME:	/* Set the RTC */
	{
		struct rtc_time rtc_tm;
		unsigned char mon, day, hrs, min, sec, leap_year;
		unsigned int years;
//		unsigned int days;
		unsigned int rtc_record;
		unsigned int rtc_sec,rtc_min,rtc_hour,rtc_day,total_sec;

		if (!capable(CAP_SYS_TIME))
			return -EACCES;

		if (copy_from_user(&rtc_tm, (struct rtc_time*)arg,
				   sizeof(struct rtc_time)))
			return -EFAULT;
					
		years = rtc_tm.tm_year + 1900;	// add tm_year offset
		mon = rtc_tm.tm_mon + 1;   /* tm_mon starts at zero */
		day = rtc_tm.tm_mday;
		hrs = rtc_tm.tm_hour;
		min = rtc_tm.tm_min;
		sec = rtc_tm.tm_sec;

	printk("RTC_SET_TIME::%04d-%02d-%02d %02d:%02d:%02d\n",
		years, mon, day, hrs, min, sec);

		if (years < epoch)
			return -EINVAL;
		if (years >= epoch + 178)
			return -EINVAL;		// because RtcDay overflow!!

		leap_year = ((!(years % 4) && (years % 100)) || !(years % 400));

		if ((mon > 12) || (day == 0))
			return -EINVAL;

		if (day > (days_in_mo[mon] + ((mon == 2) && leap_year)))
			return -EINVAL;

		if ((hrs >= 24) || (min >= 60) || (sec >= 60))
			return -EINVAL;

		rtc_record = mktime(years,mon,day,hrs,min,sec);
		rtc_record -= mktime(epoch, 1, 1, 0, 0, 0);

		spin_lock_irq(&rtc_lock);
		rtc_day  = RTC_READ(RTC_DAYS);
		rtc_hour = RTC_READ(RTC_HOUR);
		rtc_min  = RTC_READ(RTC_MINUTE);
		rtc_sec  = RTC_READ(RTC_SECOND);
		total_sec= rtc_day*86400 + rtc_hour*3600 + rtc_min*60 + rtc_sec;

		RTC_WRITE(RTC_RECORD,rtc_record-total_sec);
		RTC_WRITE(RTC_CR,0x01);
		spin_unlock_irq(&rtc_lock);

		return 0;
	}
	case RTC_EPOCH_READ:	/* Read the epoch.	*/
	{
		return put_user (epoch, (unsigned long *)arg);
	}
	case RTC_EPOCH_SET:	/* Set the epoch.	*/
	{

		if (!capable(CAP_SYS_TIME))
			return -EACCES;

		epoch = arg;
		return 0;
	}
	default:
		return -EINVAL;
	}
	return copy_to_user((void *)arg, &wtime, sizeof wtime) ? -EFAULT : 0;
}

/*
 *	We enforce only one user at a time here with the open/close.
 *	Also clear the previous interrupt data on an open, and clean
 *	up things on a close.
 */

/* We use rtc_lock to protect against concurrent opens. So the BKL is not
 * needed here. Or anywhere else in this driver. */
static int rtc_open(struct inode *inode, struct file *file)
{
	spin_lock_irq (&rtc_lock);

	if(rtc_status & RTC_IS_OPEN)
		goto out_busy;

	rtc_status |= RTC_IS_OPEN;

	rtc_irq_data = 0;
	spin_unlock_irq (&rtc_lock);
	return 0;

out_busy:
	spin_unlock_irq (&rtc_lock);
	return -EBUSY;
}

static int rtc_fasync (int fd, struct file *filp, int on)

{
	return fasync_helper (fd, filp, on, &rtc_async_queue);
}

static int rtc_release(struct inode *inode, struct file *file)
{

	spin_lock_irq (&rtc_lock);
	rtc_irq_data = 0;
	spin_unlock_irq (&rtc_lock);

	/* No need for locking -- nobody else can do anything until this rmw is
	 * committed, and no timer is running. */
	rtc_status &= ~RTC_IS_OPEN;
	return 0;
}


/*
 *	The various file operations we support.
 */

static struct file_operations rtc_fops = {
	owner:		THIS_MODULE,
	llseek:		no_llseek,
	read:		rtc_read,
	ioctl:		rtc_ioctl,
	open:		rtc_open,
	release:	rtc_release,
	fasync:		rtc_fasync,
};

static struct miscdevice rtc_dev=
{
	RTC_MINOR,
	"rtc",
	&rtc_fops
};

extern int (*set_rtc)(void);
 
static int sl2312_set_rtc(void)
{
    rtc_set_time_second(xtime.tv_sec);
    return 1;
}

static int __init rtc_init(void)
{

    set_rtc = sl2312_set_rtc;

	misc_register(&rtc_dev);
	create_proc_read_entry ("driver/rtc", 0, 0, rtc_read_proc, NULL);

	printk(KERN_INFO "Real Time Clock Driver v" RTC_VERSION "\n");
    
    RTC_WRITE(RTC_CR, 0x01);

	return 0;
}

static void __exit rtc_exit (void)
{
	remove_proc_entry ("driver/rtc", NULL);
	misc_deregister(&rtc_dev);
}

module_init(rtc_init);
module_exit(rtc_exit);
//EXPORT_NO_SYMBOLS;


/*
 *	Info exported via "/proc/driver/rtc".
 */

static int rtc_proc_output (char *buf)
{
#define YN(bit) ((ctrl & bit) ? "yes" : "no")
#define NY(bit) ((ctrl & bit) ? "no" : "yes")
	char *p;
	struct rtc_time tm;

	p = buf;

	get_rtc_time(&tm);

	/*
	 * There is no way to tell if the luser has the RTC set for local
	 * time or for Universal Standard Time (GMT). Probably local though.
	 */
	p += sprintf(p,
		     "rtc_time\t: %02d:%02d:%02d\n"
		     "rtc_date\t: %04d-%02d-%02d\n"
	 	     "rtc_epoch\t: %04lu\n",
		     tm.tm_hour, tm.tm_min, tm.tm_sec,
		     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, epoch);

	get_rtc_alm_time(&tm);

	/*
	 * We implicitly assume 24hr mode here. Alarm values >= 0xc0 will
	 * match any value for that particular field. Values that are
	 * greater than a valid time, but less than 0xc0 shouldn't appear.
	 */
	p += sprintf(p, "alarm\t\t: ");
	if (tm.tm_hour <= 24)
		p += sprintf(p, "%02d:", tm.tm_hour);
	else
		p += sprintf(p, "**:");

	if (tm.tm_min <= 59)
		p += sprintf(p, "%02d:", tm.tm_min);
	else
		p += sprintf(p, "**:");

	if (tm.tm_sec <= 59)
		p += sprintf(p, "%02d\n", tm.tm_sec);
	else
		p += sprintf(p, "**\n");


	return  p - buf;
#undef YN
#undef NY
}

static int rtc_read_proc(char *page, char **start, off_t off,
                         int count, int *eof, void *data)
{
        int len = rtc_proc_output (page);
        if (len <= off+count) *eof = 1;
        *start = page + off;
        len -= off;
        if (len>count) len = count;
        if (len<0) len = 0;
        return len;
}


static void get_rtc_time(struct rtc_time *rtc_tm)
{
// #if 0    
	unsigned int  days;
	unsigned int  months=0;
	unsigned int  years;
	unsigned int  hrs;
	unsigned int  min;
	unsigned int  sec;
	unsigned int  rtc_record; 
	unsigned int  total_sec;
	unsigned int  leap_year;
	unsigned int  i;

	spin_lock_irq(&rtc_lock);
	sec = RTC_READ(RTC_SECOND);
	min = RTC_READ(RTC_MINUTE);
	hrs = RTC_READ(RTC_HOUR);
	days = RTC_READ(RTC_DAYS);
	rtc_record = RTC_READ(RTC_RECORD);
	spin_unlock_irq(&rtc_lock);

	total_sec = rtc_record + days*86400 + hrs*3600 + min*60 + sec;

	rtc_tm->tm_sec = total_sec % 60;
	rtc_tm->tm_min = (total_sec/60) % 60;
	rtc_tm->tm_hour = (total_sec/3600) % 24;

	years = epoch;
	days  = total_sec/86400;
	while ( days > 365 )	{
		leap_year = (!(years % 4) && (years % 100)) || !(years % 400);
		days = days - 365 - leap_year;
		years++;
	}
	leap_year = (!(years % 4) && (years+epoch % 100)) || !(years % 400);

	for (i=1;i<=12;i++)
	{
	    if (days > (days_in_mo[i] + ((i == 2) && leap_year)))
	    {
		days = days - (days_in_mo[i] + ((i == 2) && leap_year));
	    } else {
		months = i;
		break;
	    }
	}

	rtc_tm->tm_mday = days+1;
	rtc_tm->tm_mon  = months-1;
	rtc_tm->tm_year = years-1900;
// #endif
}

static void get_rtc_alm_time(struct rtc_time *alm_tm)
{

	spin_lock_irq(&rtc_lock);
	alm_tm->tm_sec = RTC_READ(RTC_ALARM_SECOND);
	alm_tm->tm_min = RTC_READ(RTC_ALARM_MINUTE);
	alm_tm->tm_hour = RTC_READ(RTC_ALARM_HOUR);
	spin_unlock_irq(&rtc_lock);
}

unsigned int rtc_get_time_second(void)
{
    unsigned int    sec,min,hr,day,rtc_record;
    unsigned int    total_sec;
    
	spin_lock_irq(&rtc_lock);
	sec = RTC_READ(RTC_SECOND);
	min = RTC_READ(RTC_MINUTE);
	hr = RTC_READ(RTC_HOUR);
	day = RTC_READ(RTC_DAYS);
	rtc_record = RTC_READ(RTC_RECORD);
	spin_unlock_irq(&rtc_lock);
	total_sec = rtc_record + day*86400 + hr*3600 + min*60 + sec;
	return (total_sec);
}

void rtc_set_time_second(unsigned int second)
{
    unsigned int    sec,min,hr,day,rtc_record;
    unsigned int    total_sec;

	spin_lock_irq(&rtc_lock);
	sec = RTC_READ(RTC_SECOND);
	min = RTC_READ(RTC_MINUTE);
	hr = RTC_READ(RTC_HOUR);
	day = RTC_READ(RTC_DAYS);
	spin_unlock_irq(&rtc_lock);
	total_sec = day*86400 + hr*3600 + min*60 + sec;
	rtc_record = second - total_sec;
	RTC_WRITE(RTC_RECORD,rtc_record);
	RTC_WRITE(RTC_CR,0x01);
	
	return ;
}
    
MODULE_AUTHOR("Storlink");
MODULE_LICENSE("GPL");
