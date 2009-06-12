/* When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL
*/

#include <features.h>
#include <sys/regdef.h>

.text
	.global	__start
	.type	__start,%function
	.type	__uClibc_start_main,%function
	.weak	_init
	.weak	_fini
	.type	__uClibc_start_main,%function
/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
	.type	main,%function

__start:              
#ifdef __PIC__
        .set noreorder
        bltzal zero,0f
        nop
0:      .cpload $31
        .set reorder
#endif

	move $31, zero	    /* no return, but since the jal kills $31, skip this */
	lw   a0, 0($29)	    /* argc */
	addu a1, $29, 4	    /* get to start of argv */
	addu a2, a0, 1	    /* argv[0] program name (ordinal->cardinal) */
        sll  a2, a2, 2	    /* multiple by 4 */
	add  a2, a2, a1	    /* a2 now points to start of envp */
	la   a3, _init	    /* a3 is address of _init */
	addiu sp, sp, -24   /* 16 + 4 rounded up to multiple of 8 */
			    /* multiple of 8 for longlong/double support */
	la   v0, _fini
	sw   v0, 16(sp)	    /* stack has 5th argument, address of _fini */
	
	/* Ok, now run uClibc's main() -- shouldn't return */
	jal  __uClibc_start_main

	addiu sp, sp, 24    /* undo stack argument */

	/* Crash if somehow `exit' returns anyways.  */
hlt:
	b   hlt

	.section ".data"
	.globl __data_start
__data_start:
