#ifndef	_RTL8366_SMI_H_
#define	_RTL8366_SMI_H_

//#include "RTL8366S_DRIVER/rtl8366s_types.h"

uint32_t smi_init(void);
uint32_t smi_read(uint32_t mAddrs, uint32_t *rData);
uint32_t smi_write(uint32_t mAddrs, uint32_t rData);
uint32_t switch_reg_read(uint32_t reg,uint32_t *value);
void switch_reg_write(uint32_t reg, uint32_t data);

int rtl_chip_type_select(void);

#endif // _RTL8366_SMI_H_
