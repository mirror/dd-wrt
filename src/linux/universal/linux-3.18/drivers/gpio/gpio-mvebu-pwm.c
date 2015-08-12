#include "asm/io.h"
#include <linux/err.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/pwm.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include "gpio-mvebu.h"
#include "gpiolib.h"
static void __iomem *mvebu_gpioreg_blink_select(struct mvebu_gpio_chip *mvchip)
{
	return mvchip->membase + GPIO_BLINK_CNT_SELECT;
}

static inline struct mvebu_pwm *to_mvebu_pwm(struct pwm_chip *chip)
{
	return container_of(chip, struct mvebu_pwm, chip);
}

static inline struct mvebu_gpio_chip *to_mvchip(struct mvebu_pwm *pwm)
{
	return container_of(pwm, struct mvebu_gpio_chip, pwm);
}

static int mvebu_pwm_request(struct pwm_chip *chip, struct pwm_device *pwmd)
{
	struct mvebu_pwm *pwm = to_mvebu_pwm(chip);
	struct mvebu_gpio_chip *mvchip = to_mvchip(pwm);
	struct gpio_desc *desc = gpio_to_desc(pwmd->pwm);
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&pwm->lock, flags);
	if (pwm->used) {
		ret = -EBUSY;
	} else {
		if (!desc) {
			ret = -ENODEV;
			goto out;
		}
		ret = gpiod_request(desc, "mvebu-pwm");
		if (ret)
			goto out;

		ret = gpiod_direction_output(desc, 0);
		if (ret) {
			gpiod_free(desc);
			goto out;
		}

		pwm->pin = pwmd->pwm - mvchip->chip.base;
		pwm->used = true;
	}

out:
	spin_unlock_irqrestore(&pwm->lock, flags);
	return ret;
}

static void mvebu_pwm_free(struct pwm_chip *chip, struct pwm_device *pwmd)
{
	struct mvebu_pwm *pwm = to_mvebu_pwm(chip);
	struct gpio_desc *desc = gpio_to_desc(pwmd->pwm);
	unsigned long flags;

	spin_lock_irqsave(&pwm->lock, flags);
	gpiod_free(desc);
	pwm->used = false;
	spin_unlock_irqrestore(&pwm->lock, flags);
}

static int mvebu_pwm_config(struct pwm_chip *chip, struct pwm_device *pwmd,
			    int duty_ns, int period_ns)
{
	struct mvebu_pwm *pwm = to_mvebu_pwm(chip);
	struct mvebu_gpio_chip *mvchip = to_mvchip(pwm);
	unsigned int on, off;
	unsigned long long val;
	u32 u;

	val = (unsigned long long) pwm->clk_rate * duty_ns;
	do_div(val, NSEC_PER_SEC);
	if (val > UINT_MAX)
		return -EINVAL;
	if (val)
		on = val;
	else
		on = 1;

	val = (unsigned long long) pwm->clk_rate * (period_ns - duty_ns);
	do_div(val, NSEC_PER_SEC);
	if (val > UINT_MAX)
		return -EINVAL;
	if (val)
		off = val;
	else
		off = 1;

	u = readl_relaxed(mvebu_gpioreg_blink_select(mvchip));
	u &= ~(1 << pwm->pin);
	u |= (pwm->id << pwm->pin);
	writel_relaxed(u, mvebu_gpioreg_blink_select(mvchip));

	writel_relaxed(on, pwm->membase + BLINK_ON_DURATION);
	writel_relaxed(off, pwm->membase + BLINK_OFF_DURATION);

	return 0;
}

static int mvebu_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwmd)
{
	struct mvebu_pwm *pwm = to_mvebu_pwm(chip);
	struct mvebu_gpio_chip *mvchip = to_mvchip(pwm);

	mvebu_gpio_blink(&mvchip->chip, pwm->pin, 1);

	return 0;
}

static void mvebu_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwmd)
{
	struct mvebu_pwm *pwm = to_mvebu_pwm(chip);
	struct mvebu_gpio_chip *mvchip = to_mvchip(pwm);

	mvebu_gpio_blink(&mvchip->chip, pwm->pin, 0);
}

static const struct pwm_ops mvebu_pwm_ops = {
	.request = mvebu_pwm_request,
	.free = mvebu_pwm_free,
	.config = mvebu_pwm_config,
	.enable = mvebu_pwm_enable,
	.disable = mvebu_pwm_disable,
	.owner = THIS_MODULE,
};

void mvebu_pwm_suspend(struct mvebu_gpio_chip *mvchip)
{
	struct mvebu_pwm *pwm = &mvchip->pwm;

	pwm->blink_select = readl_relaxed(mvebu_gpioreg_blink_select(mvchip));
	pwm->blink_on_duration =
		readl_relaxed(pwm->membase + BLINK_ON_DURATION);
	pwm->blink_off_duration =
		readl_relaxed(pwm->membase + BLINK_OFF_DURATION);
}

void mvebu_pwm_resume(struct mvebu_gpio_chip *mvchip)
{
	struct mvebu_pwm *pwm = &mvchip->pwm;

	writel_relaxed(pwm->blink_select, mvebu_gpioreg_blink_select(mvchip));
	writel_relaxed(pwm->blink_on_duration,
		       pwm->membase + BLINK_ON_DURATION);
	writel_relaxed(pwm->blink_off_duration,
		       pwm->membase + BLINK_OFF_DURATION);
}

/*
 * Armada 370/XP has simple PWM support for gpio lines. Other SoCs
 * don't have this hardware. So if we don't have the necessary
 * resource, it is not an error.
 */
int mvebu_pwm_probe(struct platform_device *pdev,
		    struct mvebu_gpio_chip *mvchip,
		    int id)
{
	struct device *dev = &pdev->dev;
	struct mvebu_pwm *pwm = &mvchip->pwm;
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pwm");
	if (!res)
		return 0;

	mvchip->pwm.membase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(mvchip->pwm.membase))
		return PTR_ERR(mvchip->percpu_membase);

	if (id < 0 || id > 1)
		return -EINVAL;
	pwm->id = id;

	if (IS_ERR(mvchip->clk))
		return PTR_ERR(mvchip->clk);

	pwm->clk_rate = clk_get_rate(mvchip->clk);
	if (!pwm->clk_rate) {
		dev_err(dev, "failed to get clock rate\n");
		return -EINVAL;
	}

	pwm->chip.dev = dev;
	pwm->chip.ops = &mvebu_pwm_ops;
	pwm->chip.base = mvchip->chip.base;
	pwm->chip.npwm = mvchip->chip.ngpio;
	pwm->chip.can_sleep = false;

	spin_lock_init(&pwm->lock);

	return pwmchip_add(&pwm->chip);
}
