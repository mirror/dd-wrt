/*
 * Linux OS Independent Layer
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#define LINUX_OSL

#include <typedefs.h>
#include <bcmendian.h>
#include <linux/module.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>
#include <linux/delay.h>
#ifdef mips
#include <asm/paccess.h>
#endif
#include <pcicfg.h>

#define PCI_CFG_RETRY 		10	

#define OS_HANDLE_MAGIC		0x1234abcd
#define BCM_MEM_FILENAME_LEN 	24

typedef struct bcm_mem_link {
	struct bcm_mem_link *prev;
	struct bcm_mem_link *next;
	uint	size;
	int	line;
	char	file[BCM_MEM_FILENAME_LEN];
} bcm_mem_link_t;

struct os_handle {
	uint magic;
	void *pdev;
	uint malloced;
	uint failed;
	bcm_mem_link_t *dbgmem_list;
};

static int16 linuxbcmerrormap[] =  \
{	0, 			/* 0 */
	-EINVAL,		/* BCME_ERROR */
	-EINVAL,		/* BCME_BADARG*/
	-EINVAL,		/* BCME_BADOPTION*/
	-EINVAL,		/* BCME_NOTUP */
	-EINVAL,		/* BCME_NOTDOWN */
	-EINVAL,		/* BCME_NOTAP */
	-EINVAL,		/* BCME_NOTSTA */
	-EINVAL,		/* BCME_BADKEYIDX */
	-EINVAL,		/* BCME_RADIOOFF */
	-EINVAL,		/* BCME_NOTBANDLOCKED */
	-EINVAL, 		/* BCME_NOCLK */
	-EINVAL, 		/* BCME_BADRATESET */
	-EINVAL, 		/* BCME_BADBAND */
	-E2BIG,			/* BCME_BUFTOOSHORT */
	-E2BIG,			/* BCME_BUFTOOLONG */
	-EBUSY, 		/* BCME_BUSY */
	-EINVAL, 		/* BCME_NOTASSOCIATED */
	-EINVAL, 		/* BCME_BADSSIDLEN */
	-EINVAL, 		/* BCME_OUTOFRANGECHAN */
	-EINVAL, 		/* BCME_BADCHAN */
	-EFAULT, 		/* BCME_BADADDR */
	-ENOMEM, 		/* BCME_NORESOURCE */
	-EOPNOTSUPP,		/* BCME_UNSUPPORTED */
	-EMSGSIZE,		/* BCME_BADLENGTH */
	-EINVAL,		/* BCME_NOTREADY */
	-EPERM,			/* BCME_NOTPERMITTED */
	-ENOMEM, 		/* BCME_NOMEM */
	-EINVAL, 		/* BCME_ASSOCIATED */
	-ERANGE, 		/* BCME_RANGE */
	-EINVAL 		/* BCME_NOTFOUND */
}; 

/* translate bcmerrors into linux errors*/
int 
osl_error(int bcmerror)
{
	int abs_bcmerror;
	int array_size = ARRAYSIZE(linuxbcmerrormap); 
	
	abs_bcmerror = ABS(bcmerror);	

	if (bcmerror > 0)
		abs_bcmerror = 0;

 	else if (abs_bcmerror >= array_size)
		abs_bcmerror = BCME_ERROR;

	return linuxbcmerrormap[abs_bcmerror];
}

osl_t *
osl_attach(void *pdev)
{
	osl_t *osh;

	osh = kmalloc(sizeof(osl_t), GFP_ATOMIC);
	ASSERT(osh);

	/* 
	 * check the cases where 
	 * 1.Error code Added to bcmerror table, but forgot to add it to the OS 
	 * dependent error code
	 * 2. Error code is added to the bcmerror table, but forgot to add the 
	 * corresponding errorstring(dummy call to bcmerrorstr)
	 */
	bcmerrorstr(0);
	ASSERT(ABS(BCME_LAST) == (ARRAYSIZE(linuxbcmerrormap) - 1));

	osh->magic = OS_HANDLE_MAGIC;
	osh->malloced = 0;
	osh->failed = 0;
	osh->dbgmem_list = NULL;
	osh->pdev = pdev;

	return osh;
}

void
osl_detach(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC));
	kfree(osh);
}

void*
osl_pktget(osl_t *osh, uint len, bool send)
{
	struct sk_buff *skb;

	if ((skb = dev_alloc_skb(len)) == NULL)
		return (NULL);

	skb_put(skb, len);

	/* ensure the cookie field is cleared */ 
	PKTSETCOOKIE(skb, NULL);

	return ((void*) skb);
}

void
osl_pktfree(void *p)
{
	struct sk_buff *skb, *nskb;

	skb = (struct sk_buff*) p;

	/* perversion: we use skb->next to chain multi-skb packets */
	while (skb) {
		nskb = skb->next;
		skb->next = NULL;
		if (skb->destructor) {
			/* cannot kfree_skb() on hard IRQ (net/core/skbuff.c) if destructor exists */
			dev_kfree_skb_any(skb);
		} else {
			/* can free immediately (even in_irq()) if destructor does not exist */
			dev_kfree_skb(skb);
		}
		skb = nskb;
	}
}

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	uint val;
	uint retry=PCI_CFG_RETRY;	 

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		pci_read_config_dword(osh->pdev, offset, &val);
		if (val != 0xffffffff)
			break;
	} while (retry--);


	return (val);
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	uint retry=PCI_CFG_RETRY;	 

	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	/* only 4byte access supported */
	ASSERT(size == 4);

	do {
		pci_write_config_dword(osh->pdev, offset, val);
		if (offset!=PCI_BAR0_WIN)
			break;
		if (osl_pci_read_config(osh,offset,size) == val) 
			break;
	} while (retry--);

}

/* return bus # for the pci device pointed by osh->pdev */
uint
osl_pci_bus(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return ((struct pci_dev *)osh->pdev)->bus->number;
}

/* return slot # for the pci device pointed by osh->pdev */
uint
osl_pci_slot(osl_t *osh)
{
	ASSERT(osh && (osh->magic == OS_HANDLE_MAGIC) && osh->pdev);

	return PCI_SLOT(((struct pci_dev *)osh->pdev)->devfn);
}

static void
osl_pcmcia_attr(osl_t *osh, uint offset, char *buf, int size, bool write)
{
}

void
osl_pcmcia_read_attr(osl_t *osh, uint offset, void *buf, int size)
{
	osl_pcmcia_attr(osh, offset, (char *) buf, size, FALSE);
}

void
osl_pcmcia_write_attr(osl_t *osh, uint offset, void *buf, int size)
{
	osl_pcmcia_attr(osh, offset, (char *) buf, size, TRUE);
}


#ifdef BCMDBG_MEM

void*
osl_debug_malloc(osl_t *osh, uint size, int line, char* file)
{
	bcm_mem_link_t *p;
	char* basename;

	ASSERT(size);
	
	if ((p = (bcm_mem_link_t*)osl_malloc(osh, sizeof(bcm_mem_link_t) + size)) == NULL)
		return (NULL);
	
	p->size = size;
	p->line = line;
	
	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;

	if (!basename)
		basename = file;
	
	strncpy(p->file, basename, BCM_MEM_FILENAME_LEN);
	p->file[BCM_MEM_FILENAME_LEN - 1] = '\0';

	/* link this block */
	p->prev = NULL;
	p->next = osh->dbgmem_list;
	if (p->next)
		p->next->prev = p;
	osh->dbgmem_list = p;

	return p + 1;
}

void
osl_debug_mfree(osl_t *osh, void *addr, uint size, int line, char* file)
{
	bcm_mem_link_t *p = (bcm_mem_link_t *)((int8*)addr - sizeof(bcm_mem_link_t));
	
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	if (p->size == 0) {
		printk("osl_debug_mfree: double free on addr 0x%x size %d at line %d file %s\n", 
			(uint)addr, size, line, file);
		ASSERT(p->size);
		return;
	}

	if (p->size != size) {
		printk("osl_debug_mfree: dealloc size %d does not match alloc size %d on addr 0x%x at line %d file %s\n",
		       size, p->size, (uint)addr, line, file);
		ASSERT(p->size == size);
		return;
	}

	/* unlink this block */
	if (p->prev)
		p->prev->next = p->next;
	if (p->next)
		p->next->prev = p->prev;
	if (osh->dbgmem_list == p)
		osh->dbgmem_list = p->next;
	p->next = p->prev = NULL;

	osl_mfree(osh, p, size + sizeof(bcm_mem_link_t));
}

char*
osl_debug_memdump(osl_t *osh, char *buf, uint sz)
{
	bcm_mem_link_t *p;
	char *obuf;
	
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	obuf = buf;

	buf += sprintf(buf, "   Address\tSize\tFile:line\n");
	for (p = osh->dbgmem_list; p && ((buf - obuf) < (sz - 128)); p = p->next)
		buf += sprintf(buf, "0x%08x\t%5d\t%s:%d\n",
			(int)p + sizeof(bcm_mem_link_t), p->size, p->file, p->line);

	return (obuf);
}

#endif	/* BCMDBG_MEM */

void*
osl_malloc(osl_t *osh, uint size)
{
	void *addr;
	
	/* only ASSERT if osh is defined */
	if (osh)
		ASSERT(osh->magic == OS_HANDLE_MAGIC);

	if ((addr = kmalloc(size, GFP_ATOMIC)) == NULL) {
		if(osh)
			osh->failed++;
		return (NULL);
	}
	if (osh)
		osh->malloced += size;
	
	return (addr);
}

void
osl_mfree(osl_t *osh, void *addr, uint size)
{
	if (osh) {
		ASSERT(osh->magic == OS_HANDLE_MAGIC);
		osh->malloced -= size;
	}
	kfree(addr);
}

uint
osl_malloced(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->malloced);
}

uint osl_malloc_failed(osl_t *osh)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	return (osh->failed);
}

void*
osl_dma_alloc_consistent(osl_t *osh, uint size, ulong *pap)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	return (pci_alloc_consistent(osh->pdev, size, (dma_addr_t*)pap));
}

void
osl_dma_free_consistent(osl_t *osh, void *va, uint size, ulong pa)
{
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));

	pci_free_consistent(osh->pdev, size, va, (dma_addr_t)pa);
}

uint
osl_dma_map(osl_t *osh, void *va, uint size, int direction)
{
	int dir;
	
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	dir = (direction == DMA_TX)? PCI_DMA_TODEVICE: PCI_DMA_FROMDEVICE;
	return (pci_map_single(osh->pdev, va, size, dir));
}

void
osl_dma_unmap(osl_t *osh, uint pa, uint size, int direction)
{
	int dir;
	
	ASSERT((osh && (osh->magic == OS_HANDLE_MAGIC)));
	dir = (direction == DMA_TX)? PCI_DMA_TODEVICE: PCI_DMA_FROMDEVICE;
	pci_unmap_single(osh->pdev, (uint32)pa, size, dir);
}

#if defined(BINOSL)
void
osl_assert(char *exp, char *file, int line)
{
	char tempbuf[255];

	sprintf(tempbuf, "assertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	panic(tempbuf);
}
#endif	/* BCMDBG || BINOSL */

void
osl_delay(uint usec)
{
	uint d;

	while (usec > 0) {
		d = MIN(usec, 1000);
		udelay(d);
		usec -= d;
	}
}

/*
 * BINOSL selects the slightly slower function-call-based binary compatible osl.
 */
#ifdef BINOSL

int
osl_printf(const char *format, ...)
{
	va_list args;
	char buf[1024];
	int len;

	/* sprintf into a local buffer because there *is* no "vprintk()".. */
	va_start(args, format);
	len = vsprintf(buf, format, args);
	va_end(args);

	if (len > sizeof (buf)) {
		printk("osl_printf: buffer overrun\n");
		return (0);
	}

	return (printk(buf));
}

int
osl_sprintf(char *buf, const char *format, ...)
{
	va_list args;
	int rc;

	va_start(args, format);
	rc = vsprintf(buf, format, args);
	va_end(args);
	return (rc);
}

int
osl_strcmp(const char *s1, const char *s2)
{
	return (strcmp(s1, s2));
}

int
osl_strncmp(const char *s1, const char *s2, uint n)
{
	return (strncmp(s1, s2, n));
}

int
osl_strlen(const char *s)
{
	return (strlen(s));
}

char*
osl_strcpy(char *d, const char *s)
{
	return (strcpy(d, s));
}

char*
osl_strncpy(char *d, const char *s, uint n)
{
	return (strncpy(d, s, n));
}

void
bcopy(const void *src, void *dst, int len)
{
	memcpy(dst, src, len);
}

int
bcmp(const void *b1, const void *b2, int len)
{
	return (memcmp(b1, b2, len));
}

void
bzero(void *b, int len)
{
	memset(b, '\0', len);
}

uint32
osl_readl(volatile uint32 *r)
{
	return (readl(r));
}

uint16
osl_readw(volatile uint16 *r)
{
	return (readw(r));
}

uint8
osl_readb(volatile uint8 *r)
{
	return (readb(r));
}

void
osl_writel(uint32 v, volatile uint32 *r)
{
	writel(v, r);
}

void
osl_writew(uint16 v, volatile uint16 *r)
{
	writew(v, r);
}

void
osl_writeb(uint8 v, volatile uint8 *r)
{
	writeb(v, r);
}

void *
osl_uncached(void *va)
{
#ifdef mips
	return ((void*)KSEG1ADDR(va));
#else
	return ((void*)va);
#endif
}

uint
osl_getcycles(void)
{
	uint cycles;

#if defined(mips)
	cycles = read_c0_count() * 2;
#elif defined(__i386__)
	rdtscl(cycles);
#else
	cycles = 0;
#endif
	return cycles;
}

void *
osl_reg_map(uint32 pa, uint size)
{
	return (ioremap_nocache((unsigned long)pa, (unsigned long)size));
}

void
osl_reg_unmap(void *va)
{
	iounmap(va);
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
#ifdef mips
	return get_dbe(*val, (uint32*)addr);
#else
	*val = readl(addr);
	return 0;
#endif
}

uchar*
osl_pktdata(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->data);
}

uint
osl_pktlen(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->len);
}

uint
osl_pktheadroom(osl_t *osh, void *skb)
{
	return (uint) skb_headroom((struct sk_buff *) skb);
}

uint
osl_pkttailroom(osl_t *osh, void *skb)
{
	return (uint) skb_tailroom((struct sk_buff *) skb);
}

void*
osl_pktnext(osl_t *osh, void *skb)
{
	return (((struct sk_buff*)skb)->next);
}

void
osl_pktsetnext(void *skb, void *x)
{
	((struct sk_buff*)skb)->next = (struct sk_buff*)x;
}

void
osl_pktsetlen(osl_t *osh, void *skb, uint len)
{
	__skb_trim((struct sk_buff*)skb, len);
}

uchar*
osl_pktpush(osl_t *osh, void *skb, int bytes)
{
	return (skb_push((struct sk_buff*)skb, bytes));
}

uchar*
osl_pktpull(osl_t *osh, void *skb, int bytes)
{
	return (skb_pull((struct sk_buff*)skb, bytes));
}

void*
osl_pktdup(osl_t *osh, void *skb)
{
	return (skb_clone((struct sk_buff*)skb, GFP_ATOMIC));
}

void*
osl_pktcookie(void *skb)
{
	return ((void*)((struct sk_buff*)skb)->csum);
}

void
osl_pktsetcookie(void *skb, void *x)
{
	((struct sk_buff*)skb)->csum = (uint)x;
}

void*
osl_pktlink(void *skb)
{
	return (((struct sk_buff*)skb)->prev);
}

void
osl_pktsetlink(void *skb, void *x)
{
	((struct sk_buff*)skb)->prev = (struct sk_buff*)x;
}

uint
osl_pktprio(void *skb)
{
	return (((struct sk_buff*)skb)->priority);
}

void
osl_pktsetprio(void *skb, uint x)
{
	((struct sk_buff*)skb)->priority = x;
}


#endif	/* BINOSL */
