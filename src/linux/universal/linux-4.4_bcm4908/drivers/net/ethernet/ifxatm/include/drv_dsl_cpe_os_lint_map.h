/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_OS_LINT_MAP_H
#define _DRV_DSL_CPE_OS_LINT_MAP_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_dsl_cpe_api_types.h"

#define DSL_DRV_CRLF "\n"
#define NULL         ((void *)0)

#define DRV_DSL_CPE_API_DEV_NAME "dsl_cpe_api"

/** LINUX Kernel Thread Name Lenght*/
#define DSL_DRV_THREAD_NAME_LEN 16

#define DSL_DRV_THREAD_DELETE_WAIT_FOREVER     0xFFFFFFF

#define DSL_DRV_THREAD_OPTION_NOT_USED_FOR_LINUX     0

/** LINUX Kernel Thread - priority - IDLE */
#define DSL_DRV_THREAD_PRIO_IDLE                     1
/** LINUX Kernel Thread - priority - LOWEST */
#define DSL_DRV_THREAD_PRIO_LOWEST                   5
/** LINUX Kernel Thread - priority - LOW */
#define DSL_DRV_THREAD_PRIO_LOW                      20
/** LINUX Kernel Thread - priority - NORMAL */
#define DSL_DRV_THREAD_PRIO_NORMAL                   40
/** LINUX Kernel Thread - priority - HIGH */
#define DSL_DRV_THREAD_PRIO_HIGH                     60
/** LINUX Kernel Thread - priority - HIGHEST */
#define DSL_DRV_THREAD_PRIO_HIGHEST                  80
/** LINUX Kernel Thread - priority - TIME_CRITICAL 
\attention
   You should use this priority only for driver threads.
*/
#define DSL_DRV_THREAD_PRIO_TIME_CRITICAL            90

/** LINUX Kernel Thread - default prio (use OS default)  */
#define DSL_DRV_DEFAULT_PRIO                         DSL_DRV_THREAD_OPTION_NOT_USED_FOR_LINUX

/** LINUX Kernel Thread - internal poll time for check thread end */
#define DSL_DRV_THREAD_DOWN_WAIT_POLL_MS             10

#define CLONE_FS                                     1
#define CLONE_FILES                                  1

/** LINUX Kernel Thread - thread options */
#define DSL_DRV_DRV_THREAD_OPTIONS                   (CLONE_FS | CLONE_FILES)
/** LINUX Kernel Thread - default stack size (use OS default)  */
#define DSL_DRV_DEFAULT_STACK_SIZE                   DSL_DRV_THREAD_OPTION_NOT_USED_FOR_LINUX

#ifndef DSL_DRV_STACKSIZE
#define DSL_DRV_STACKSIZE 2048
#endif

#ifndef DSL_DRV_PRIORITY
#define DSL_DRV_PRIORITY  64
#endif

/* The major number of a CPE API device
      Typically it is a Voodoo 3dfx device (107) for Danube, Amazon-SE, AR9 and
      logical volume manager (109) for VINAX
*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #define DRV_DSL_CPE_API_DEV_MAJOR 107
#elif defined (INCLUDE_DSL_CPE_API_VINAX)
   #define DRV_DSL_CPE_API_DEV_MAJOR 109
#else
   #error "Device is not defined!"
#endif

#define __func__ ""
#define __LINE__ (0)

#define __init
#define __exit
#define _IOR(a,b,c)      (b)
#define _IOWR(a, b, c)   (b)
#define _IO(a,b)         (b)
#define _IOC_TYPE(x)                      (((x)>>8) & 0xFF)

#define DSL_Le2Cpu(x)           (x)
#define DSL_WAIT(ms)            msleep(ms)
#define DSL_DRV_TimeSecGet(t)   (t)
#define MINOR(a)                (a)
#define EIO                     (0x1)
#define EFAULT                  (0x2)
#define ENOIOCTLCMD             (0x3)
#define ENODEV                  (0x4)
#define EINVAL                  (0x5)
#define ENOTSUPP                (0x6)
#define ENOMEM                  (0x7)
#define EPERM                   (0x8)

#define POLLIN                  (0x1)
#define POLLRDNORM              (0x2)

#define GFP_KERNEL              (0x1)
#define PAGE_KERNEL             (0x1)

#define S_IFREG                 (0x1)
#define S_IRUGO                 (0x2)

typedef char         *va_list;
typedef int          size_t;
typedef int          ssize_t;
typedef int          loff_t;
typedef int          off_t;
typedef int          mode_t;
typedef int          DSL_DRV_size_t;
typedef int          DSL_DRV_Mutex_t;
typedef int          DSL_DRV_WaitQueue_t;
typedef int          DSL_DRV_Event_t;
typedef unsigned int DSL_DRV_TimeVal_t;
typedef int          DSL_DRV_WaitQueue_t;
typedef int          DSL_DRV_Event_t;
typedef int          DSL_ssize_t;
typedef int          DSL_DRV_offset_t;

typedef enum
{
   SPIN_LOCK_UNLOCKED = 1,
   SPIN_LOCK_LOCKED = 2
} spinlock_t;

struct timeval
{
	unsigned int tv_sec;		/* seconds */
	unsigned int tv_usec;	/* microseconds */
};

struct file
{
   void *private_data;
};

typedef struct
{
   int dummy;
} DSL_DRV_Poll_Table_t;

struct inode
{
   int i_rdev;
};

typedef struct inode DSL_DRV_inode_t;

typedef struct file DSL_DRV_file_t;

struct seq_operations;
struct file;

struct mutex
{
   int dummy;
};

typedef	int (read_proc_t)(char *page, char **start, off_t off,
			  int count, int *eof, void *data);
typedef	int (write_proc_t)(struct file *file, const char *buffer,
			   unsigned long count, void *data);
typedef int (get_info_t)(char *, char **, off_t, int);

struct proc_dir_entry {
	unsigned int low_ino;
	unsigned short namelen;
	const char *name;
	loff_t size;
	const struct file_operations *proc_fops;
	get_info_t *get_info;
	struct proc_dir_entry *next, *parent, *subdir;
	void *data;
	read_proc_t *read_proc;
	write_proc_t *write_proc;
	int deleted;		/* delete flag */
	void *set;
};

struct seq_file {
	char *buf;
	size_t size;
	size_t from;
	size_t count;
	loff_t index;
	loff_t version;
	struct mutex lock;
	const struct seq_operations *op;
	void *private;
};

struct seq_operations {
	void * (*start) (struct seq_file *m, loff_t *pos);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	int (*show) (struct seq_file *m, void *v);
};

struct file_operations {
	int (*open) (struct inode *, struct file *);
	int (*release) (struct inode *, DSL_DRV_file_t *);
    DSL_ssize_t (*read) (struct file *, char *, DSL_DRV_size_t, DSL_DRV_offset_t *);
	DSL_ssize_t (*write) (struct file *, const char *, DSL_DRV_size_t, DSL_DRV_offset_t *);
	DSL_DRV_offset_t (*llseek) (struct file *, DSL_DRV_offset_t, int);
	int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
	unsigned int (*poll) (struct file *, DSL_DRV_Poll_Table_t *);
}; 

typedef int (*DSL_DRV_KERNEL_THREAD_StartRoutine)(void *);

typedef struct DSL_DRV_ThreadParams_s
{
   /** user argument 1 */
   DSL_uint32_t   nArg1;
   /** user argument 2 */
   DSL_uint32_t   nArg2;
   /** name of the thread/task */
   DSL_char_t     pName[DSL_DRV_THREAD_NAME_LEN];

   /** control - signal the run state */
   volatile DSL_boolean_t  bRunning;
   /** control - set to shutdown the thread */
   volatile DSL_boolean_t  bShutDown;
} DSL_DRV_ThreadParams_t;

typedef DSL_int_t (*DSL_DRV_ThreadFunction_t)(DSL_DRV_ThreadParams_t *);

typedef struct
{
   /** Contains the user and thread control parameters */
   DSL_DRV_ThreadParams_t    thrParams;

   /** Points to the thread start routine */
   DSL_DRV_ThreadFunction_t  pThrFct;

   /** Kernel thread process ID */
   DSL_int32_t             pid;

   /** requested kernel thread priority */
   DSL_int32_t             nPriority;
   
   /** LINUX specific internal data - completion handling */
   int                     thrCompletion;

   /** flag indicates that the structure is initialized */
   DSL_boolean_t           bValid;
   
} DSL_DRV_ThreadCtrl_t;


/**
   Define the function proto type for "DSL_DRV_ThreadInit"
*/
DSL_int32_t DSL_DRV_ThreadInit(
               DSL_DRV_ThreadCtrl_t *pThrCntrl,
               DSL_char_t     *pName,
               DSL_DRV_ThreadFunction_t pThreadFunction,
               DSL_uint32_t   nStackSize,
               DSL_uint32_t   nPriority,
               DSL_uint32_t   nArg1,
               DSL_uint32_t   nArg2);

/**
   Define the function proto type for "DSL_DRV_ThreadShutdown"
*/
DSL_int32_t DSL_DRV_ThreadShutdown(
               DSL_DRV_ThreadCtrl_t *pThrCntrl,
               DSL_uint32_t       waitTime_ms);

#define DSL_DRV_THREAD(a, b, c, d)   DSL_DRV_ThreadInit((a), (b), (c), DSL_DRV_STACKSIZE, DSL_DRV_PRIORITY, (d), 0)
#define DSL_DRV_THREAD_INIT_VALID(P_THREAD_ID)  (((P_THREAD_ID)) ? (((P_THREAD_ID)->bValid == DSL_TRUE) ? DSL_TRUE : DSL_FALSE) : DSL_FALSE)

#define copy_from_user(to,from,n)   (from)
#define copy_to_user(to,from,n)     (from)
#define isprint(x)                  ((int)x ? 0 : 1)

#define jiffies   (1)
#define HZ        (1)

/**
   Define the function proto type for "DSL_DRV_SysTimeGet"
*/
DSL_uint32_t DSL_DRV_SysTimeGet(DSL_uint32_t nOffset);

void spin_lock_irqsave(spinlock_t* , DSL_uint32_t);

void spin_unlock_irqrestore(spinlock_t* , DSL_uint32_t);

void wake_up_interruptible(DSL_DRV_Event_t* );

void interruptible_sleep_on_timeout(DSL_DRV_Event_t* , DSL_uint32_t);
long wait_event_interruptible_timeout(DSL_DRV_Event_t*, DSL_DRV_Event_t, DSL_uint32_t);

/**
   Define the function proto type for "msleep"
*/
void msleep(unsigned int ms);

/**
   Define the function proto type for "sprintf"
*/
int sprintf(char * buf, const char * fmt, ...);

/**
   Define the function proto type for "remove_proc_entry"
*/
void remove_proc_entry(const char *name, struct proc_dir_entry *parent); 

/**
   Define the function proto type for "create_proc_entry"
*/
struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
						struct proc_dir_entry *parent); 

/**
   Define the function proto type for "proc_mkdir"
*/
struct proc_dir_entry *proc_mkdir(const char *,struct proc_dir_entry *);

/**
   Define the function proto type for "seq_open"
*/
int seq_printf(struct seq_file *, const char *, ...);

/**
   Define the function proto type for "seq_open"
*/
int seq_open(struct file *, const struct seq_operations *);

/**
   Define the function proto type for "seq_release"
*/
ssize_t seq_read(struct file *, char *, size_t, loff_t *);

/**
   Define the function proto type for "seq_lseek"
*/
loff_t seq_lseek(struct file *, loff_t, int);

/**
   Define the function proto type for "seq_release"
*/
int seq_release(struct inode *, struct file *);

/**
   Define the function proto type for "do_gettimeofday"
*/
void do_gettimeofday(struct timeval* );

/**
   Define the function proto type for "kernel_thread"
*/
long kernel_thread(int (*fn)(void *), void * arg, unsigned long flags);

/**
   Define the function proto type for "init_completion"
*/
void init_completion(int* );

/**
   Define the function proto type for "wait_for_completion"
*/
void wait_for_completion(int* );

/**
   Define the function proto type for "printk"
*/
int printk(const char * fmt, ...);

/**
   Define the function proto type for "vsnprintf"
*/
int vsnprintf(char *buf, DSL_DRV_size_t size, const char *fmt, va_list args);

/**
   Define the function proto type for "va_start"
*/
void va_start(va_list, DSL_char_t const *);

/**
   Define the function proto type for "va_end"
*/
void va_end(va_list);

/**
   Define the function proto type for "register_chrdev"
*/
int register_chrdev(int, char*, struct file_operations*);

/**
   Define the function proto type for "unregister_chrdev"
*/
int unregister_chrdev(int, char*);

/**
   Define the function proto type for "memset"
*/
void poll_wait(DSL_DRV_file_t*, DSL_DRV_WaitQueue_t*, DSL_DRV_Poll_Table_t*);

/**
   Define the function proto type for "memset"
*/
void *memset(void *s, int c, DSL_DRV_size_t n);

/**
   Define the function proto type for "memcpy"
*/
void *memcpy(void* dest, const void* src, DSL_DRV_size_t count);

/**
   Define the function proto type for "__vmalloc"
*/
void *__vmalloc(unsigned long size, unsigned int gfp_mask, unsigned int prot); 

/**
   Define the function proto type for "memmove"
*/
void memmove(void* , void* , unsigned long size);

/**
   Define the function proto type for "vfree"
*/
void vfree(void* pPtr);

/**
   Define the function proto type for "kmalloc"
*/
void *kmalloc(unsigned long size, int flags);

/**
   Define the function proto type for "kfree"
*/
void kfree(void* pPtr);

/**
   Define the function proto type for "strncpy"
*/
char *strncpy( char *strDest, const char *strSource, DSL_DRV_size_t count );

/**
   Define the function proto type for "strncpy"
*/
char *strcpy( char *strDestination, const char *strSource );

/**
   Define the function proto type for "DSL_DRV_Malloc"
*/
void *DSL_DRV_Malloc(DSL_DRV_size_t size);

/**
   Define the function proto type for "DSL_DRV_MemFree"
*/
void DSL_DRV_MemFree(void*);

/**
   Define the function proto type for "DSL_DRV_PMalloc"
*/
void *DSL_DRV_PMalloc(DSL_DRV_size_t size);

/**
   Define the function proto type for "DSL_DRV_PFree"
*/
void DSL_DRV_PFree(void*);

/**
   Define the function proto type for "DSL_DRV_MSecSleep"
*/
void DSL_DRV_MSecSleep(DSL_uint32_t msec);

/**
   Define the function proto type for "DSL_DRV_ElapsedTimeMSecGet"
*/
DSL_uint32_t DSL_DRV_ElapsedTimeMSecGet(DSL_uint32_t refTime_ms);

/**
   Define the function proto type for "DSL_DRV_TimeMSecGet"
*/
DSL_uint32_t DSL_DRV_TimeMSecGet(void);

/**
   Define the function proto type for "DSL_DRV_MUTEX_LOCK"
*/
int DSL_DRV_MUTEX_LOCK(DSL_DRV_Mutex_t mutex);

/**
   Define the function proto type for "DSL_DRV_MUTEX_UNLOCK"
*/
void DSL_DRV_MUTEX_UNLOCK(DSL_DRV_Mutex_t mutex);

/**
   Define the function proto type for "DSL_DRV_MUTEX_INIT"
*/
void DSL_DRV_MUTEX_INIT(DSL_DRV_Mutex_t mutex);

/**
   Define the function proto type for "DSL_DRV_INIT_WAKELIST"
*/
void DSL_DRV_INIT_WAKELIST(char* name, DSL_DRV_WaitQueue_t queue);

/**
   Define the function proto type for "DSL_DRV_WAKEUP_WAKELIST"
*/
void DSL_DRV_WAKEUP_WAKELIST(DSL_DRV_WaitQueue_t queue);

/**
   Define the function proto type for "DSL_DRV_WAKEUP_EVENT"
*/
void DSL_DRV_WAKEUP_EVENT(DSL_DRV_Event_t ev);

/**
   Define the function proto type for "DSL_DRV_WAKEUP_EVENT"
*/
int DSL_DRV_WAIT_EVENT_TIMEOUT(DSL_DRV_Event_t ev, DSL_uint32_t t);

/**
   Define the function proto type for "DSL_DRV_WAKEUP_EVENT"
*/
void DSL_DRV_WAIT_COMPLETION(DSL_DRV_ThreadCtrl_t *pThrdCtrl);

/**
   Define the function proto type for "DSL_DRV_INIT_EVENT"
*/
void DSL_DRV_INIT_EVENT(char* name, DSL_DRV_Event_t ev);

/**
   Define the function proto type for "DSL_DRV_OS_ModUseCountIncrement"
*/
void DSL_DRV_OS_ModUseCountIncrement(void);

/**
   Define the function proto type for "DSL_DRV_OS_ModUseCountDecrement"
*/
void DSL_DRV_OS_ModUseCountDecrement(void);

#ifdef __cplusplus
}
#endif

#endif
