/*
 * This code fix the stack pointer so that the dynamic linker
 * can find argc, argv and auxvt (Auxillary Vector Table).
 */
asm(""					\
"	.text\n"			\
"	.globl _dl_boot\n"		\
"	.type _dl_boot,@function\n"	\
"_dl_boot:\n"				\
"	move.d $sp,$r10\n"		\
"	move.d $pc,$r9\n"		\
"	add.d _dl_boot2 - ., $r9\n"	\
"	jsr $r9\n"			\
);

#define _dl_boot _dl_boot2
#define LD_BOOT(X) static void * __attribute__ ((unused)) _dl_boot(X)
