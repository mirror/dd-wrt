#ifndef __NIOS_UNALIGNED_H
#define __NIOS_UNALIGNED_H

/*
 * The nios cannot do unaligned accesses itself. 
 */ 

#define get_unaligned(ptr) ({			\
	typeof((*(ptr))) x;			\
	memcpy(&x, (void*)ptr, sizeof(*(ptr)));	\
	x;					\
})

#define put_unaligned(val, ptr) ({		\
	typeof((*(ptr))) x = val;		\
	memcpy((void*)ptr, &x, sizeof(*(ptr)));	\
})

#endif /* __NIOS_UNALIGNED_H */
