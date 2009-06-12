#ifndef _HYPERSTONE_UACCESS_H_
#define _HYPERSTONE_UACCESS_H_

#include <linux/sched.h>
#include <asm/segment.h>

#define VERIFY_READ     0
#define VERIFY_WRITE    1


extern inline int access_ok(int type, const void * addr, unsigned long size)
{
extern unsigned long __ramend;
extern unsigned long _ramstart;
#define RANGE_CHECK_OK(addr, size, lower, upper) \
        (((addr) >= (lower)) && (((addr) + (size)) < (upper)))

/* Since we do not have an MMU we just check to 
 * ensure that the address is inside the valid address space. 
 * I suppose that we should also exclude the kernel space the data 
 * and bss segment as well as the exception entry table. Since by default
 * the exception table begins at 0xFC we do not make any other check.
 * or in the entry table. 0x01000000 since we have 16MB ram.
 */ 
	return(RANGE_CHECK_OK((unsigned long)addr, size, (unsigned long)PAGE_ALIGN(_ramstart), (unsigned long)&__ramend));
}

extern inline int verify_area(int type, const void * addr, unsigned long size)
{
        return access_ok(type,addr,size)?0:-EFAULT;
}

/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry
{
	unsigned long insn, fixup;
};

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

#define __get_user(var, ptr)						      \
  ({									      \
	  int __gu_err = 0;						      \
	  typeof(*(ptr)) __gu_val = 0;					      \
	  switch (sizeof (*(ptr))) {					      \
	  case 1:							      \
	  case 2:							      \
	  case 4:							      \
		  __gu_val = *(ptr);					      \
		  break;						      \
	  case 8:							      \
		  memcpy(&__gu_val, ptr, sizeof(__gu_val));		      \
		  break;						      \
	  default:							      \
		  __gu_val = 0;						      \
		  __gu_err = __get_user_bad ();				      \
		  break;						      \
	  }								      \
	  (var) = __gu_val;						      \
	  __gu_err;							      \
  })
#define __get_user_bad()	(bad_user_access_length (), (-EFAULT))

#define __put_user(var, ptr)						      \
  ({									      \
	  int __pu_err = 0;						      \
	  switch (sizeof (*(ptr))) {					      \
	  case 1:							      \
	  case 2:							      \
	  case 4:							      \
		  *(ptr) = (var);					      \
		  break;						      \
	  case 8: {							      \
	  	  typeof(*(ptr)) __pu_val = 0;				      \
		  __pu_val = var;						\
		  memcpy(ptr, &__pu_val, sizeof(__pu_val));		      \
		  }							      \
		  break;						      \
	  default:							      \
		  __pu_err = __put_user_bad ();				      \
		  break;						      \
	  }								      \
	  __pu_err;							      \
  })

extern int bad_user_access_length (void);

#define __put_user_bad()	(bad_user_access_length (), (-EFAULT))

#define put_user(x, ptr)	__put_user(x, ptr)
#define get_user(x, ptr)	__get_user(x, ptr)

#define __copy_from_user(to, from, n)	(memcpy (to, from, n), 0)
#define __copy_to_user(to, from, n)	(memcpy(to, from, n), 0)

#define copy_from_user(to, from, n)	__copy_from_user (to, from, n)
#define copy_to_user(to, from, n) 	__copy_to_user(to, from, n)

#define copy_to_user_ret(to,from,n,retval) \
  ({ if (copy_to_user (to,from,n)) return retval; })

#define copy_from_user_ret(to,from,n,retval) \
  ({ if (copy_from_user (to,from,n)) return retval; })

/*
 * Copy a null terminated string from userspace.
 */
static inline long
strncpy_from_user(char *dst, const char *src, long count)
{
        char *tmp;
        strncpy(dst, src, count);
        for (tmp = dst; *tmp && count > 0; tmp++, count--)
                ;
        return(tmp - dst);
}

/*
 * Return the size of a string (including the ending 0)
 *
 * Return 0 for error
 */
static inline long strnlen_user(const char * src, long n)
{
  return(strlen(src) + 1);
}
#define strlen_user(str) strnlen_user(str, ~0UL >> 1)


/*
 * Zero Userspace
 */

static inline unsigned long
clear_user (void *to, unsigned long n)
{
	memset (to, 0, n);
	return 0;
}
#endif /* !_HYPERSTONE_UACCESS_H_ */
