#include <asm/mips_machine.h>

enum lantiq_mach_type {
	LANTIQ_MACH_GENERIC,

	/* FALCON */
	LANTIQ_MACH_EASY98000,		/* Falcon Eval Board, NOR Flash */
	LANTIQ_MACH_EASY98020,		/* Falcon Reference Board */

	/* XWAY */
	LANTIQ_MACH_EASY4010,		/* Twinpass evalkit */
	LANTIQ_MACH_EASY50712,		/* Danube evalkit */
	LANTIQ_MACH_EASY50812,		/* AR9 eval board */
	/* Arcadyan */
	LANTIQ_MACH_ARV4510PW,		/* Wippies Homebox */
	LANTIQ_MACH_ARV4518PW,		/* Airties WAV-221, SMC-7908A-ISP */
	LANTIQ_MACH_ARV4520PW,		/* Airties WAV-281, Arcor EasyboxA800 */
	LANTIQ_MACH_ARV452CPW,		/* Arcor EasyboxA801 */
	LANTIQ_MACH_ARV4525PW,		/* Speedport W502V */
	LANTIQ_MACH_ARV752DPW,		/* Arcor easybox a802 */
	LANTIQ_MACH_ARV752DPW22,	/* Arcor easybox a803 */
	LANTIQ_MACH_ARV7518PW,		/* ASTORIA */
	LANTIQ_MACH_WMBR,		/* ASTORIA */
};
