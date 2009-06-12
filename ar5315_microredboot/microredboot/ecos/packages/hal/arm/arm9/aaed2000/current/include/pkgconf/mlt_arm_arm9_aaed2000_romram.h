// eCos memory layout - Tue Oct 30 16:49:43 2001

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_ram (0)
#define CYGMEM_REGION_ram_SIZE (0x1F60000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_lcd (0x1f60000)
#define CYGMEM_REGION_lcd_SIZE (0xA0000)
#define CYGMEM_REGION_lcd_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0x1Fa0000 - (size_t) CYG_LABEL_NAME (__heap1))
