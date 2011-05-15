#ifndef __SWITCH_GPIO_H
#define __SWITCH_GPIO_H


#if defined(BCMGPIO2)

#ifdef LINUX_2_4
#define sbh bcm947xx_sbh
extern void *bcm947xx_sbh;
#else
extern void *sbh;
#endif

extern __u32 sb_gpioin(void *sbh);
extern __u32 sb_gpiointpolarity(void *sbh, __u32 mask, __u32 val, __u8 prio);
extern __u32 sb_gpiointmask(void *sbh, __u32 mask, __u32 val, __u8 prio);
extern __u32 sb_gpioouten(void *sbh, __u32 mask, __u32 val, __u8 prio);
extern __u32 sb_gpioout(void *sbh, __u32 mask, __u32 val, __u8 prio);

#define gpio_in() sb_gpioin(sbh)
#define gpio_intpolarity(mask,val) sb_gpiointpolarity(sbh, mask, val, 0)
#define gpio_intmask(mask,val) sb_gpiointmask(sbh, mask, val, 0)
#define gpio_outen(mask,val) sb_gpioouten(sbh, mask, val, 0)
#define gpio_out(mask,val) sb_gpioout(sbh, mask, val, 0)

#elif defined(BCMGPIO)

#define sbh bcm947xx_sbh
extern void *bcm947xx_sbh;
extern __u32 sb_gpioin(void *sbh);
extern __u32 sb_gpiointpolarity(void *sbh, __u32 mask, __u32 val);
extern __u32 sb_gpiointmask(void *sbh, __u32 mask, __u32 val);
extern __u32 sb_gpioouten(void *sbh, __u32 mask, __u32 val);
extern __u32 sb_gpioout(void *sbh, __u32 mask, __u32 val);

#define gpio_in() sb_gpioin(sbh)
#define gpio_intpolarity(mask,val) sb_gpiointpolarity(sbh, mask, val)
#define gpio_intmask(mask,val) sb_gpiointmask(sbh, mask, val)
#define gpio_outen(mask,val) sb_gpioouten(sbh, mask, val)
#define gpio_out(mask,val) sb_gpioout(sbh, mask, val)

#else
#error Unsupported/unknown GPIO configuration
#endif


#endif /* __SWITCH_GPIO_H */
