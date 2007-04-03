#include <osl.h>

#include <bcmutils.h>
#include <bcmdevs.h>
#include <sbutils.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <sbpci.h>
#include <sbextif.h>

/* Use the existing Silicon Backplane handle (sbh)
 */
extern void *bcm947xx_sbh;
#define sbh bcm947xx_sbh

#define SB_INFO(sbh)    (sb_info_t*)sbh
/*
 * Macros to disable/restore function core(D11, ENET, ILINE20, etc) interrupts before/
 * after core switching to avoid invalid register accesss inside ISR.
 */
#define INTR_OFF(si, intr_val) \
        if ((si)->intrsoff_fn && (si)->coreid[(si)->curidx] == (si)->dev_coreid) {      \
                intr_val = (*(si)->intrsoff_fn)((si)->intr_arg); }
#define INTR_RESTORE(si, intr_val) \
        if ((si)->intrsrestore_fn && (si)->coreid[(si)->curidx] == (si)->dev_coreid) {  \
                (*(si)->intrsrestore_fn)((si)->intr_arg, intr_val); }
		
typedef uint32 (*sb_intrsoff_t)(void *intr_arg);
typedef void (*sb_intrsrestore_t)(void *intr_arg, uint32 arg);
typedef bool (*sb_intrsenabled_t)(void *intr_arg);

/* misc sb info needed by some of the routines */
typedef struct sb_info {
        uint    chip;                   /* chip number */
        uint    chiprev;                /* chip revision */
        uint    chippkg;                /* chip package option */
        uint    boardtype;              /* board type */
        uint    boardvendor;            /* board vendor id */
        uint    bustype;                /* what bus type we are going through */

        void    *osh;                   /* osl os handle */
        void    *sdh;                   /* bcmsdh handle */

        void    *curmap;                /* current regs va */
        void    *regs[SB_MAXCORES];     /* other regs va */

        uint    curidx;                 /* current core index */
        uint    dev_coreid;             /* the core provides driver functions */
        uint    pciidx;                 /* pci core index */
        uint    pcirev;                 /* pci core rev */

        uint    pcmciaidx;              /* pcmcia core index */
        uint    pcmciarev;              /* pcmcia core rev */
        bool    memseg;                 /* flag to toggle MEM_SEG register */

        uint    ccrev;                  /* chipc core rev */

        uint    gpioidx;                /* gpio control core index */
        uint    gpioid;                 /* gpio control coretype */

        uint    numcores;               /* # discovered cores */
        uint    coreid[SB_MAXCORES];    /* id of each core */

        void    *intr_arg;              /* interrupt callback function arg */
        sb_intrsoff_t           intrsoff_fn;            /* function turns chip interrupts off */
        sb_intrsrestore_t       intrsrestore_fn;        /* function restore chip interrupts */
        sb_intrsenabled_t       intrsenabled_fn;        /* function to check if chip interrupts are enabled */
} sb_info_t;

/* get gpio registers base addr */
static uint32* get_gpio_base_addr(void) {
        sb_info_t *si;
	uint origidx;
	uint32 *addr = NULL;
	uint intr_val = 0;
	
        si = SB_INFO(sbh);

	INTR_OFF(si, intr_val);

	/* save current core index */
        origidx = sb_coreidx(sbh);

	addr = (uint32*) sb_setcoreidx(sbh, si->gpioidx);

	/* restore core index */
        if (origidx != si->gpioidx) sb_setcoreidx(sbh, origidx);

        INTR_RESTORE(si, intr_val);

	return addr;
}

/* gpio output enable addr */
static uint32* get_addr_gpioouten(void) {
        sb_info_t *si = SB_INFO(sbh);
        uint offset = 0;

        switch (si->gpioid) {
        case SB_CC:
                offset = OFFSETOF(chipcregs_t, gpioouten);
                break;

        case SB_PCI:
                offset = OFFSETOF(sbpciregs_t, gpioouten);
                break;

        case SB_EXTIF:
                offset = OFFSETOF(extifregs_t, gpio[0].outen);
                break;
	default:
		return NULL;
        }

	return (uint32 *) ((uchar *)get_gpio_base_addr() + offset);
}

/* gpio output addr */
static uint32* get_addr_gpioout(void) {
        sb_info_t *si = SB_INFO(sbh);
        uint offset = 0;

        switch (si->gpioid) {
        case SB_CC:
                offset = OFFSETOF(chipcregs_t, gpioout);
                break;

        case SB_PCI:
                offset = OFFSETOF(sbpciregs_t, gpioout);
                break;

        case SB_EXTIF:
                offset = OFFSETOF(extifregs_t, gpio[0].out);
                break;
	default:
		return NULL;
        }

	return (uint32 *) ((uchar *)get_gpio_base_addr() + offset);
}

/* gpio input addr */
static uint32* get_addr_gpioin(void) {
        sb_info_t *si = SB_INFO(sbh);
        uint offset = 0;

        switch (si->gpioid) {
        case SB_CC:
                offset = OFFSETOF(chipcregs_t, gpioin);
                break;

        case SB_PCI:
                offset = OFFSETOF(sbpciregs_t, gpioin);
                break;

        case SB_EXTIF:
                offset = OFFSETOF(extifregs_t, gpioin);
                break;
	default:
		return NULL;
        }

	return (uint32 *) ((uchar *)get_gpio_base_addr() + offset);
}
