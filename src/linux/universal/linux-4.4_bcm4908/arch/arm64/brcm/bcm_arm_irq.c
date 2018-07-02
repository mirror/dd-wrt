#include <linux/bcm_assert.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>

#define INTR_NAME_MAX_LENGTH 16

static DEFINE_SPINLOCK(brcm_irqlock);

/** Broadcom wrapper to linux request_irq.  This version does more stuff.
 * 
 *  @param pfunc (IN) interrupt handler function
 *  @param param (IN) context/cookie that is passed to interrupt handler
 *  @param irq   (IN) interrupt number
 *  @param interruptName (IN) descriptive name for the interrupt.  15 chars
 *                            or less.  This function will make a copy of
 *                            the name.
 *  @param INTR_REARM_MODE    (IN) See bcm_intr.h, not used in ARM
 *  @param INTR_AFFINITY_MODE (IN) See bcm_intr.h
 *  
 *  @return 0 on success.
 *   */
unsigned int BcmHalMapInterruptEx(FN_HANDLER pfunc, void* param,
        unsigned int irq, const char *interruptName,
        INTR_REARM_MODE_ENUM rearmMode,
        INTR_AFFINITY_MODE_ENUM affinMode)
{
    char *devname;
    unsigned long irqflags = 0x00;
    unsigned int retval;
    struct cpumask mask;
#if !defined(CONFIG_BRCM_IKOS)
    unsigned long flags;
#endif

#if defined(CONFIG_BCM_KF_ASSERT)
    BCM_ASSERT_R(interruptName != NULL, -1);
    BCM_ASSERT_R(strlen(interruptName) < INTR_NAME_MAX_LENGTH, -1);
#endif

    if ((devname = kmalloc(INTR_NAME_MAX_LENGTH, GFP_ATOMIC)) == NULL) {
        printk(KERN_ERR "kmalloc(%d, GFP_ATOMIC) failed for intr name\n",
                INTR_NAME_MAX_LENGTH);
        return -1;
    }
    sprintf( devname, "%s", interruptName );

    if ((irq >= INTERRUPT_ID_TIMER0) && (irq <= INTERRUPT_ID_TIMER_MAX))
        irqflags |= IRQF_TIMER;

#if !defined(CONFIG_BRCM_IKOS)
    /* For external interrupt, check if it is shared */
    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_MAX) {
        if (IsExtIntrShared(kerSysGetExtIntInfo(irq)))
            irqflags |= IRQF_SHARED;
    }
#endif

    retval = request_irq(irq, (void*)pfunc, irqflags, devname,
            (void *)param);
    if (retval != 0) {
        printk(KERN_WARNING "request_irq failed for irq=%d (%s) "
                "retval=%d\n", irq, devname, retval);
        kfree(devname);
        return retval;
    }

#ifdef CONFIG_SMP
    /* for Timer interrupt, we always use CPU#0 to handle it */
    if ((irq >= INTERRUPT_ID_TIMER) && (irq <= INTERRUPT_ID_TIMER_MAX)) {
        cpumask_clear(&mask);
        cpumask_set_cpu(0, &mask);
        irq_set_affinity(irq, &mask);
    }
#endif
    /* now deal with interrupt affinity requests */
    if (affinMode != INTR_AFFINITY_DEFAULT) {
        cpumask_clear(&mask);

        if (affinMode == INTR_AFFINITY_TP1_ONLY ||
                affinMode == INTR_AFFINITY_TP1_IF_POSSIBLE) {
            if (cpu_online(1)) {
#if defined CONFIG_HOTPLUG_CPU
                // static affinity_hint mask with only cpu1
                const cpumask_t *onemask = get_cpu_mask(1);
                // sets both affinity_hint and affinity
                irq_set_affinity_hint(irq, onemask);
#else
                cpumask_set_cpu(1, &mask);
                irq_set_affinity(irq, &mask);
#endif
            } else {
                /* TP1 is not on-line but caller insisted on it */
                if (affinMode == INTR_AFFINITY_TP1_ONLY) {
                    printk(KERN_WARNING "cannot assign "
                            "intr %d to TP1, not "
                            "online\n", irq);
                    retval = request_irq(irq, NULL, 0,
                            NULL, NULL);
                    kfree(devname);
                    retval = -1;
                }
            }
        } else {
            /* INTR_AFFINITY_BOTH_IF_POSSIBLE */
            cpumask_set_cpu(0, &mask);
            if (cpu_online(1)) {
                cpumask_set_cpu(1, &mask);
                irq_set_affinity(irq, &mask);
            }
        }
    }

#if !defined(CONFIG_BRCM_IKOS)
    if (irq >= INTERRUPT_ID_EXTERNAL_0 && irq <= INTERRUPT_ID_EXTERNAL_MAX)
    {
        int levelOrEdge, detectSense;
        int ein = irq - INTERRUPT_ID_EXTERNAL_0;

        if( IsExtIntrTypeActHigh(kerSysGetExtIntInfo(irq)) )
            detectSense = 1;
        else
            detectSense = 0;

        if( IsExtIntrTypeSenseLevel(kerSysGetExtIntInfo(irq)) )
            levelOrEdge = 1;
        else
            levelOrEdge = 0;
        spin_lock_irqsave(&brcm_irqlock, flags);
        PERF->ExtIrqCtrl |= (levelOrEdge << (EI_LEVEL_SHFT + ein))
            | (detectSense << (EI_SENSE_SHFT + ein))
            | (1 << (EI_CLEAR_SHFT + ein));
        PERF->ExtIrqStatus |= (1 << (EI_MASK_SHFT + ein));
        spin_unlock_irqrestore(&brcm_irqlock, flags);
    }
#endif

    return retval;
}
EXPORT_SYMBOL(BcmHalMapInterruptEx);

unsigned int BcmHalMapInterrupt(FN_HANDLER pfunc, void* param, unsigned int irq)
{
    char devname[INTR_NAME_MAX_LENGTH];

    sprintf(devname, "brcm_%d", irq);
    return BcmHalMapInterruptEx(pfunc, param, irq, devname, INTR_REARM_YES,
        INTR_AFFINITY_DEFAULT);
}
EXPORT_SYMBOL(BcmHalMapInterrupt);

void enable_brcm_irq_irq(unsigned int irq)
{
    enable_irq(irq);
}

void disable_brcm_irq_irq(unsigned int irq)
{
    disable_irq(irq);
}
EXPORT_SYMBOL(enable_brcm_irq_irq);
EXPORT_SYMBOL(disable_brcm_irq_irq);

void BcmHalExternalIrqClear(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
    PERF->ExtIrqCtrl |=  (1 << (EI_CLEAR_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
    PERF->ExtIrqCtrl &= ~(1 << (EI_CLEAR_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void BcmHalExternalIrqMask(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
    PERF->ExtIrqStatus &= ~(1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void BcmHalExternalIrqUnmask(unsigned int irq)
{
    unsigned long flags;
    spin_lock_irqsave(&brcm_irqlock, flags);
    PERF->ExtIrqStatus |= (1 << (EI_MASK_SHFT + irq - INTERRUPT_ID_EXTERNAL_0));
    spin_unlock_irqrestore(&brcm_irqlock, flags);
}

void BcmHalSetIrqAffinity(unsigned int irq, struct cpumask *mask)
{
    irq_set_affinity(irq, mask);
}
EXPORT_SYMBOL(BcmHalSetIrqAffinity);
EXPORT_SYMBOL(BcmHalExternalIrqClear);
EXPORT_SYMBOL(BcmHalExternalIrqMask);
EXPORT_SYMBOL(BcmHalExternalIrqUnmask);

