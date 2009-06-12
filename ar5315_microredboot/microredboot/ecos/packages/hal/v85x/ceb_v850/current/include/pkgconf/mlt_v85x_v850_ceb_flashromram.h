// eCos memory layout - Fri Jan 26 09:09:49 2001

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_rom (0)
#define CYGMEM_REGION_rom_SIZE (0x80000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#define CYGMEM_REGION_ram (0xfc0000)
#define CYGMEM_REGION_ram_SIZE (0x3c000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0xffc000 - (size_t) CYG_LABEL_NAME (__heap1))
