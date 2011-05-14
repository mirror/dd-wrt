#ifndef __ARM11_HW_COUNTERS
#define __ARM11_HW_COUNTERS

static inline u32 arm11_read_ccnt(void)
{
        u32 val;
        asm volatile("mrc p15, 0, %0, c15, c12, 1" : "=r" (val));
        return val;
}


static inline void arm11_enable_ccnt(void)
{
        u32 val;

        asm volatile("mrc p15, 0, %0, c15, c12, 0" : "=r" (val));
                val &= ~0x8;
                val |= 0x1;
        asm volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (val));
}

/* CCNT incremented for every 64th cycle */
static inline void arm11_enable_ccnt64(void)
{
        u32 val;

        asm volatile("mrc p15, 0, %0, c15, c12, 0" : "=r" (val));
                val &= ~0x8;
                val |= 0x9;
        asm volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (val));
}

static inline void arm11_reset_ccnt(void)
{
        u32 val;

        asm volatile("mrc p15, 0, %0, c15, c12, 0" : "=r" (val));
                val |= 0x4;
        asm volatile("mcr p15, 0, %0, c15, c12, 0" : : "r" (val));
}


static inline u32 arm11_reset_and_read_ccnt(void)
{
        arm11_reset_ccnt();
        return arm11_read_ccnt();
}

#endif
