
#ifndef __SMI_H__
#define __SMI_H__

#include "rtl8368s_types.h"


int32 smi_reset(uint32 port, uint32 pinRST);
void smi_init(void);

int32 smi_read(uint32 mAddrs, uint32 *rData);
int32 smi_write(uint32 mAddrs, uint32 rData);

#endif /* __SMI_H__ */


