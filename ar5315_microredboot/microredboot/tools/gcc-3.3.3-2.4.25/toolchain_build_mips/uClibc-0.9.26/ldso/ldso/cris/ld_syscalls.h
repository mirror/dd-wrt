/* 
 * Define the __set_errno macro as nothing so that INLINE_SYSCALL
 * won't set errno, which is important since we make system calls
 * before the errno symbol is dynamicly linked. 
 */
#define __set_errno(X) {(void)(X);}
#include "sys/syscall.h"
