/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 */

asm("" \
"	.section .text..SHmedia32,\"ax\"\n"				\
"	.globl _dl_boot\n"						\
"	.type _dl_boot, @function\n"					\
"	.align 5\n"							\
"_dl_boot:\n"								\
"	! Set r12 to point to GOT\n"					\
"	movi	(((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ3-.)) >> 16) & 65535), r12\n"	\
"	shori	((datalabel _GLOBAL_OFFSET_TABLE_-(.LZZZ3-.)) & 65535), r12\n"		\
".LZZZ3:\n"								\
"	ptrel/u	r12, tr0\n"						\
"	gettr	tr0, r12	! GOT address\n"			\
"	add	r18, r63, r11	! save return address - needed?\n"	\
"	add	r15, r63, r2	! arg = stack pointer\n"		\
"	pt	_dl_boot2, tr0	! should work even if PIC\n"		\
"	blink	tr0, r18	! call _dl_boot2 - user EP is in r2\n"	\
);

#define _dl_boot _dl_boot2
#define LD_BOOT(X)   static void * __attribute__ ((unused)) _dl_boot (X)
