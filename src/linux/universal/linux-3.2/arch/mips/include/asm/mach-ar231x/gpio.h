#ifndef _ATHEROS_GPIO_H_
#define _ATHEROS_GPIO_H_

#include <ar231x.h>

struct ar231x_gpiodev {
	u32 valid_mask;
	u32 (*get_output)(void);
	u32 (*set_output)(u32 mask, u32 val);
	u32 (*get)(void);
	u32 (*set)(u32 mask, u32 val);
};

extern const struct ar231x_gpiodev *ar231x_gpiodev;

/*
 * Wrappers for the generic GPIO layer
 */

static inline int gpio_direction_input(unsigned gpio) {
	u32 mask = 1 << gpio;

	if (!(ar231x_gpiodev->valid_mask & mask))
		return -ENXIO;

	ar231x_gpiodev->set_output(mask, 0);
	return 0;
}

static inline void gpio_set_value(unsigned gpio, int value) {
	u32 mask = 1 << gpio;

	if (!(ar231x_gpiodev->valid_mask & mask))
		return;

	ar231x_gpiodev->set(mask, (!!value) * mask);
}
#define gpio_set_value_cansleep gpio_set_value

static inline int gpio_cansleep(unsigned gpio)
{
	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value) {
	u32 mask = 1 << gpio;

	if (!(ar231x_gpiodev->valid_mask & mask))
		return -ENXIO;

	ar231x_gpiodev->set_output(mask, mask);
	ar231x_gpiodev->set(mask, (!!value) * mask);
	return 0;
}

/* Reads the gpio pin.  Unchecked function */
static inline int gpio_get_value(unsigned gpio) {
	u32 mask = 1 << gpio;

	if (!(ar231x_gpiodev->valid_mask & mask))
		return 0;

	return !!(ar231x_gpiodev->get() & mask);
}

#define gpio_get_value_cansleep	gpio_get_value

static inline int gpio_request(unsigned gpio, const char *label) {
	return 0;
}

static inline void gpio_free(unsigned gpio) {
}

/* Returns IRQ to attach for gpio.  Unchecked function */
static inline int gpio_to_irq(unsigned gpio) {
	return AR531X_GPIO_IRQ(gpio);
}

/* Returns gpio for IRQ attached.  Unchecked function */
static inline int irq_to_gpio(unsigned irq) {
	return (irq - (AR531X_GPIO_IRQ(0)));
}

static inline int gpio_set_debounce(unsigned gpio, unsigned debounce)
{
	return -ENOSYS;
}


#endif
