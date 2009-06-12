/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  */

/* Overrive the default _dl_boot function, and replace it with a bit of asm.
 * Then call the real _dl_boot function, which is now named _dl_boot2. */

asm("" \
"	.text\n"			\
"	.globl	_dl_boot\n"		\
"_dl_boot:\n"				\
"	mov	r7, sp\n"		\
"	@ldr	r0, [sp], #4\n"		\
"	mov	r0, sp\n"		\
"	bl	_dl_boot2\n"		\
"	mov	r6, r0\n"		\
"	mov	r0, r7\n"		\
"	mov	pc, r6\n"		\
);

#define _dl_boot _dl_boot2
#define LD_BOOT(X)   static void *  __attribute__ ((unused)) _dl_boot (X)


 /* It seems ARM needs an offset here */
#undef ELFMAGIC
#define	    ELFMAGIC	ELFMAG+load_addr 



