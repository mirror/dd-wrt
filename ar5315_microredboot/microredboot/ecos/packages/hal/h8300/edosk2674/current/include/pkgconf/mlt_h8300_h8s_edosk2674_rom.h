// eCos memory layout - Wed Nov 24 13:10:23 1999

// This is a generated file - changes will be lost if ConfigTool(MLT) is run

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif

#define CYGMEM_REGION_ram (0x400000)
#if !defined(CYGPKG_IO_ETH_DRIVERS)
#define CYGMEM_REGION_ram_SIZE (0x800000)
#else
#define CYGMEM_REGION_ram_SIZE (0x800000-0xC000)
#endif
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

