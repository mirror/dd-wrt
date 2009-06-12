#ifndef _HAL_STARTUP_H
#define _HAL_STARTUP_H

#include <cyg/infra/cyg_type.h>

//      Include the variant-specific startup header.

#include <cyg/hal/var_startup.h>

//      Declare the routine to call to simulate a hardware reset.

externC void hal_hw_reset(void);

#endif // _HAL_STARTUP_H

