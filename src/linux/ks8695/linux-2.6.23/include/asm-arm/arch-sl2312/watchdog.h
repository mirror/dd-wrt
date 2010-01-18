#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#define WATCHDOG_BASE		          (IO_ADDRESS (SL2312_WAQTCHDOG_BASE))
#define WATCHDOG_COUNTER                  (WATCHDOG_BASE + 0x00)
#define WATCHDOG_LOAD                     (WATCHDOG_BASE + 0x04)
#define WATCHDOG_RESTART                  (WATCHDOG_BASE + 0x08)
#define WATCHDOG_CR                       (WATCHDOG_BASE + 0x0C)
#define WATCHDOG_STATUS                   (WATCHDOG_BASE + 0x10)
#define WATCHDOG_CLEAR                    (WATCHDOG_BASE + 0x14)
#define WATCHDOG_INTRLEN                  (WATCHDOG_BASE + 0x18)

#define WATCHDOG_WDENABLE_MSK 		  (0x00000001)
#define WATCHDOG_WDENABLE_OFST 		  (0)
#define WATCHDOG_WDRST_MSK  		  (0x00000002)
#define WATCHDOG_WDRST_OFST 		  (1)
#define WATCHDOG_WDINTR_MSK 		  (0x00000004)
#define WATCHDOG_WDINTR_OFST 		  (2)
#define WATCHDOG_WDEXT_MSK 		  (0x00000008)
#define WATCHDOG_WDEXT_OFST 		  (3)
#define WATCHDOG_WDCLOCK_MSK 		  (0x00000010)
#define WATCHDOG_WDCLOCK_OFST 		  (4)
#define WATCHDOG_CR_MASK                  (0x0000001F)

#define WATCHDOG_CLEAR_STATUS             0x1
#define WATCHDOG_ENABLE                   1
#define WATCHDOG_DISABLE                  0
#define WATCHDOG_RESTART_VALUE            0x5AB9

#define WATCHDOG_MINOR	                  130

#define WATCHDOG_IOCTRL_DISABLE	          0x01
#define WATCHDOG_IOCTRL_SETTIME	          0x02
#define WATCHDOG_IOCTRL_ENABLE	          0x03
#define WATCHDOG_IOCTRL_RESTART	          0x04

#define WATCHDOG_TIMEOUT_SCALE            APB_CLK
#define WATCHDOG_TIMEOUT_MARGIN           30
#define WATCHDOG_DRIVER_OPEN              1
#define WATCHDOG_DRIVER_CLOSE             0


static void     watchdog_disable(void);
static void     watchdog_enable(void);
static int      watchdog_open(struct inode *, struct file *);
static int      watchdog_release(struct inode *, struct file *);
static ssize_t  watchdog_read(struct file *, char *, size_t, loff_t *);
static ssize_t  watchdog_write(struct file *, const char *, size_t, loff_t *);
static int      watchdog_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
#ifdef WATCHDOG_TEST
static void watchdog_fire(int, void *, struct pt_regs *);
#endif





#endif
