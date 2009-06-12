/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  */

asm("" \
"	.text\n"			\
"	.globl	_dl_boot\n"		\
"_dl_boot:\n"				\
"	mov	r15, r4\n"		\
"	mov.l   .L_dl_boot2, r0\n"	\
"	bsrf    r0\n"			\
"	add	#4, r4\n"		\
".jmp_loc:\n"				\
"	jmp	@r0\n"			\
"	 mov    #0, r4 	!call _start with arg == 0\n" \
".L_dl_boot2:\n"			\
"	.long   _dl_boot2-.jmp_loc\n"	\
"	.previous\n"			\
);

#define _dl_boot _dl_boot2
#define LD_BOOT(X)   static void *  __attribute__ ((unused)) _dl_boot (X)
