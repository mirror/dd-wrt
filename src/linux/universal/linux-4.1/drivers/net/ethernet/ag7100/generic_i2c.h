/* generic_i2c.h
 *
 * History:
 * Jan  4, 2007 wclewis initial
 * Jan 11, 2007 wclewis ready for checkin
 * Jan 14, 2007 wclewis common version for BDI/ECOS/Linux
 * May 15, 2007 wclewis changed CS1 to TEST
 */

#ifndef _GENERIC_I2C_H
#define _GENERIC_I2C_H

/* OUT, ACTIVE LOW, INITIAL STATE HIGH */
#define GENERIC_I2C_VSC73XX_CS    (1<<0)
#define GENERIC_I2C_STEREO_CS     (1<<1)
#define GENERIC_I2C_SLIC_CS0      (1<<2)
#define GENERIC_I2C_SLIC_TEST     (1<<3)

#define GENERIC_I2C_INIT_OUT_HIGH (GENERIC_I2C_VSC73XX_CS | GENERIC_I2C_STEREO_CS | GENERIC_I2C_SLIC_CS0 | GENERIC_I2C_TP28)

/* OUT, ACTIVE HIGH, INITIAL STATE LOW */
#define GENERIC_I2C_JUMP_LED1     (1<<9)
#define GENERIC_I2C_JUMP_LED2     (1<<10)
#define GENERIC_I2C_TP25          (1<<12)
#define GENERIC_I2C_TP26          (1<<13)

#define GENERIC_I2C_INIT_OUT_LOW  (GENERIC_I2C_JUMP_LED1 | GENERIC_I2C_JUMP_LED2 | GENERIC_I2C_SLIC_TEST | GENERIC_I2C_TP25 | GENERIC_I2C_TP26)

/* IN */
#define GENERIC_I2C_SLIC_INT0     (1<<4)
#define GENERIC_I2C_SLIC_INT1     (1<<5)
#define GENERIC_i2C_SW_RESET      (1<<6)
#define GENERIC_I2C_JUMP_SW       (1<<8)
#define GENERIC_I2C_TP24          (1<<11)
#define GENERIC_I2C_TP27          (1<<14)
#define GENERIC_I2C_TP28          (1<<15)

#define GENERIC_I2C_INIT_INPUT (~(GENERIC_I2C_INIT_OUT_HIGH|GENERIC_I2C_INIT_OUT_LOW))

#define GENERIC_I2C_INPUT_LOW (GENERIC_I2C_SLIC_INT0 | GENERIC_I2C_SLIC_INT1 | GENERIC_i2C_SW_RESET | GENERIC_I2C_JUMP_SW)

int
generic_i2c_init_gpio(void);

int
generic_i2c_write_gpio(unsigned short d);

int
generic_i2c_read_gpio(unsigned short *d);

int 
generic_i2c_assert_cs(int cs);

void 
generic_i2c_deassert_cs(void);

#ifdef USE_TEST_CODE

void
generic_i2c_test_scl(void);

void
generic_i2c_test_sda(void);

void 
generic_i2c_test_write_bits(void);

void
generic_i2c_test_toggle(unsigned short t);

void
generic_i2c_test_addr_strapping(void);

void
generic_i2c_check_input(void);

void
generic_i2c_test_read_write_gpio(void);

void
generic_i2c_test_vs7385_toggle(void);

#endif
#endif
