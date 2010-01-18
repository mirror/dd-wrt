/*
 * relocate_kernel.S - put the kernel image in place to boot
 */

#include <asm/kexec.h>

	.globl relocate_new_kernel
relocate_new_kernel:

	ldr	r0,kexec_indirection_page
	ldr	r1,kexec_start_address
    ldr r8,kexec_initrd_dest
    mov r9,#0x500000

0:	/* top, read another word for the indirection page */
	ldr	r3, [r0],#4

	/* Is it a destination page. Put destination address to r4 */
	tst	r3,#1,0
	beq	1f
	bic	r4,r3,#1
	b	0b
1:
	/* Is it an indirection page */
	tst	r3,#2,0
	beq	1f
	bic	r0,r3,#2
	b	0b
1:

	/* are we done ? */
	tst	r3,#4,0
	beq	1f
    cmp r8,#0
    bne 2f
	b	11f

1:
	/* is it source ? */
	tst	r3,#8,0
	beq	0b
	bic r3,r3,#8
	mov r6,#1024
9:
	ldr r5,[r3],#4
	str r5,[r4],#4
	subs r6,r6,#1
	bne 9b
	b 0b

2:
     /* Get the kernel size and calculate the start address of the initrd*/
     ldr r5,[r1,#44]
     add r7,r1,r5
     subs r7,r7,#4
     subs r8,r8,#4
 10:
     /* Copy the ramdisk from the end of the kernel to the final destination */
     ldr r10,[r7,#4]!
     str r10,[r8,#4]!
     subs r9,r9,#1
     bne 10b
 11:
	/* Jump to relocated kernel */
	mov lr,r1
	mov r0,#0
	ldr r1,kexec_mach_type
 	mov r2,#0
 
    	mov pc,lr

	.globl kexec_start_address
kexec_start_address:
	.long	0x0

	.globl kexec_indirection_page
kexec_indirection_page:
	.long	0x0

	.globl kexec_mach_type
kexec_mach_type:
	.long	0x0

 	.globl kexec_initrd_dest
 kexec_initrd_dest:
  	.long	0x0

relocate_new_kernel_end:

	.globl relocate_new_kernel_size
relocate_new_kernel_size:
	.long relocate_new_kernel_end - relocate_new_kernel



