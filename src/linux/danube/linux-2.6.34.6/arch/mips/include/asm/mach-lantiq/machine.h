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
	LANTIQ_MACH_ARV4518,		/* Airties WAV-221, SMC-7908A-ISP */
	LANTIQ_MACH_ARV452,			/* Airties WAV-281, Arcor EasyboxA800 */
	LANTIQ_MACH_ARV4525,		/* Speedport W502V */
};
