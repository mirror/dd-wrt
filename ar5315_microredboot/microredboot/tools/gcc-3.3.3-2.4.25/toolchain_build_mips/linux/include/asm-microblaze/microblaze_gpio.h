
#ifndef _microblaze_gpio_h
#define _microblaze_gpio_h

/*
 * - MICROBLAZE_GPIO_DATA_OFFSET    Data register
 * - MICROBLAZE_GPIO_TRI_OFFSET     Three state register (sets input/output direction)
 *                        0 configures pin for output and 1 for input.
 */
#define MICROBLAZE_GPIO_DATA_OFFSET  0x00000000
#define MICROBLAZE_GPIO_TRI_OFFSET   0x00000004

#define microblaze_gpio_setdir(BaseAddress, Data) \
	*((unsigned int *)(BaseAddress+MICROBLAZE_GPIO_TRI_OFFSET)) = (unsigned int)Data

#define microblaze_gpio_write(BaseAddress, Data) \
	*((unsigned int *)(BaseAddress+MICROBLAZE_GPIO_DATA_OFFSET)) = (unsigned int)Data

#define microblaze_gpio_read(BaseAddress) \
	*((unsigned int *)(BaseAddress+MICROBLAZE_GPIO_DATA_OFFSET))

#endif

