#ifndef _AR9100_H
#define _AR9100_H

#ifndef AR9100
#define AR9100
#endif


#include <ar7100.h>

#define AR9100_OBS_BASE     (AR7100_APB_BASE+0x00080000)


#define AR9100_OBS_GPIO_1   (AR9100_OBS_BASE+0x14)
#define AR9100_OBS_OE       (AR9100_OBS_BASE+0x0c)

#endif
