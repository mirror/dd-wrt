// eCos memory layout - Sun Jan 14 22:42:04 2001

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_vecs (0)
#define CYGMEM_REGION_vecs_SIZE (0x1000)
#define CYGMEM_REGION_vecs_ATTR (CYGMEM_REGION_ATTR_R)
#define CYGMEM_REGION_rom (0x40000)
#define CYGMEM_REGION_rom_SIZE (0x7c0000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#define CYGMEM_REGION_ram (0xa0000000)
#define CYGMEM_REGION_ram_SIZE (0x20000000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0xc0000000 - (size_t) CYG_LABEL_NAME (__heap1))
