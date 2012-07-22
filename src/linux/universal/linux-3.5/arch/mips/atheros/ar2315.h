#ifndef __AR2315_H
#define __AR2315_H

#ifdef CONFIG_ATHEROS_AR5315

extern void ar2315_irq_init(void);
extern unsigned int ar2315_cpu_frequency(void);
extern int ar2315_init_devices(void);
extern void ar2315_prom_init(void);
extern void ar2315_plat_setup(void);
extern void ar2315_time_init(void);

#else

static inline void ar2315_irq_init(void)
{
}

static inline unsigned int ar2315_cpu_frequency(void)
{
	return 0;
}

static inline int ar2315_init_devices(void)
{
	return 0;
}

static inline void ar2315_prom_init(void)
{
}

static inline void ar2315_plat_setup(void)
{
}

static inline void ar2315_time_init(void)
{
}

#endif

#endif
