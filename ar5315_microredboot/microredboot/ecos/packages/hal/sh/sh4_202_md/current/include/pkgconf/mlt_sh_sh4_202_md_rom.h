// eCos memory layout - Mon Jul 16 12:45:16 2001

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_rom (0x80000000)
#define CYGMEM_REGION_rom_SIZE (0x02000000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#define CYGMEM_REGION_ram (0x88000000)
#define CYGMEM_REGION_ram_SIZE (0x04000000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__reserved) [];
#endif
#define CYGMEM_SECTION_reserved (CYG_LABEL_NAME (__reserved))
#define CYGMEM_SECTION_reserved_SIZE (0x200)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0x8c000000 - (size_t) CYG_LABEL_NAME (__heap1))
