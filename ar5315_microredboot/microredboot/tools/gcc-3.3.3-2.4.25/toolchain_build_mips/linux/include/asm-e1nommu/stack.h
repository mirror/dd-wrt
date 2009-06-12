#ifndef __ASM_E1_STACK_H__
#define __ASM_E1_STACK_H__

/* We assign a stack offset of 512 bytes
 * We need at least <register_size>*64 = 256 bytes
 */
#define STACK_OFFSET  0x200

/* A register stack size of 4K is considered enough for most applications
 * If not increase from here...
 */
#define USER_REGISTER_STACK_SIZE 0x1000

#endif
