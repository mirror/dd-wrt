/* generic_spi.h */

#ifndef _GENERIC_SPI_H
#define _GENERIC_SPI_H

#ifndef CEXTERN
#define  CEXTERN static inline
#endif

#include "generic_i2c.h"

#define GENERIC_SPI_VSC73XX_CS    GENERIC_I2C_VSC73XX_CS
#define GENERIC_SPI_STEREO_CS     GENERIC_I2C_STEREO_CS
#define GENERIC_SPI_SLIC_CS0      GENERIC_I2C_SLIC_CS0
#define GENERIC_SPI_SLIC_CS1      GENERIC_I2C_SLIC_CS1

int
generic_spi_init(int cs);

int
generic_spi_access_enable(int cs);

int
generic_spi_access_done(void);

void
generic_spi_raw_output_u8(unsigned char val);

unsigned int 
generic_spi_raw_input_u32(void);

unsigned int 
generic_spi_raw_input_u8(void);

#endif
