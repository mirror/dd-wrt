#ifndef __AR5312_H
#define __AR5312_H

#ifdef CONFIG_ATHEROS_AR5312

extern void ar5312_irq_init(void);
extern int ar5312_init_devices(void);
extern void ar5312_prom_init(void);
extern void ar5312_plat_setup(void);
extern void ar5312_time_init(void);
extern unsigned int ar5312_cpu_frequency(void);
extern void ar5312_time_init(void);

#else

static inline void ar5312_irq_init(void)
{
}

static inline int ar5312_init_devices(void)
{
	return 0;
}
static inline unsigned int ar5312_cpu_frequency(void)
{
	return 0;
}

static inline void ar5312_prom_init(void)
{
}

static inline void ar5312_plat_setup(void)
{
}

static inline void ar5312_time_init(void)
{
}

#endif

#endif
