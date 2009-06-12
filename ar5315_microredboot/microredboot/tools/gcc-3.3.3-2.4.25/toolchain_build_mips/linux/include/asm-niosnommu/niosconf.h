#include <asm/nios.h>

#if defined(na_flash_kernel) && defined(na_flash_kernel_end)
#define NIOS_FLASH_START  	((int)na_flash_kernel)				/* the start address of the flash */
#define NIOS_FLASH_END		((int)na_flash_kernel_end)			/* the end address of the flash */

#define KERNEL_FLASH_START 	(NIOS_FLASH_START+0) 				/* where the kernel is burned in the flash */
#define KERNEL_FLASH_LEN   	0x200000 					/* how many flash bytes reserved for the kernel */

#define LINUX_ROMFS_START	(KERNEL_FLASH_START+KERNEL_FLASH_LEN) 	/* where the romdisk start in the flash, dont overwrite the kernel!!! */
#define LINUX_ROMFS_END		NIOS_FLASH_END		 	 	/* how large the romdisk */
#else
#error Sorry,you dont have na_flash_kernel or na_flash_kernel_end defined in the core.
#endif

#if defined(nasys_program_mem) && defined(nasys_program_mem_end)
#define LINUX_SDRAM_START 	((int)nasys_program_mem)			/* the sdram where the linux plays */
#define LINUX_SDRAM_END		((int)nasys_program_mem_end)			/* the lenghth of the sdram linux plays in */
#else
#error Sorry,you dont have nasys_program_mem or nasys_program_mem_end defined in the core.
#endif	