// eCos memory layout - Wed Nov 24 13:10:23 1999

// This is a generated file - changes will be lost if ConfigTool(MLT) is run

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif

#define CYGMEM_REGION_ram (0x200000)
#define CYGMEM_REGION_ram_SIZE (0x200000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (0x300000 - (size_t) CYG_LABEL_NAME (__heap1))
