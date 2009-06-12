// eCos memory layout - Mon Apr 09 14:17:14 2001

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_ram (0x80000400)
#define CYGMEM_REGION_ram_SIZE (0x00fffc00)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

#define CYGMEM_REGION_rom (0x9fc00000)
#define CYGMEM_REGION_rom_SIZE (0x400000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)

#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - (size_t) CYG_LABEL_NAME (__heap1))
