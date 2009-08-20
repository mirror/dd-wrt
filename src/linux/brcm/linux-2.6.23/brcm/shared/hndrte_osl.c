/*
 * Initialization and support routines for self-booting
 * compressed image.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_osl.c,v 1.28.2.1 2008/05/24 00:21:48 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#ifdef SBPCI
#include <hndpci.h>
#endif /* SBPCI */

osl_t *
osl_attach(void *dev)
{
	osl_t *osh;

	osh = (osl_t *)hndrte_malloc(sizeof(osl_t));
	ASSERT(osh);

	bzero(osh, sizeof(osl_t));
	osh->dev = dev;
	return osh;
}

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;
	hndrte_free(osh);
}

#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
void *
osl_malloc(osl_t *osh, uint size, char *file, int line)
{
	return hndrte_malloc_align(size, 0, file, line);
}
#else
void *
osl_malloc(osl_t *osh, uint size)
{
	return hndrte_malloc_align(size, 0);
}
#endif /* BCMDBG_MEM */

int
osl_mfree(osl_t *osh, void *addr, uint size)
{
	return hndrte_free(addr);
}

uint
osl_malloced(osl_t *osh)
{
	return 0;
}

uint
osl_malloc_failed(osl_t *osh)
{
	return 0;
}

int
osl_busprobe(uint32 *val, uint32 addr)
{
	*val = *(uint32 *)addr;
	return 0;
}

/* translate these erros into hndrte specific errors */
int
osl_error(int bcmerror)
{
	return bcmerror;
}

#ifdef SBPCI
uint
osl_pci_bus(osl_t *osh)
{
	hndrte_dev_t *dev = (hndrte_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->bus;
}

uint
osl_pci_slot(osl_t *osh)
{
	hndrte_dev_t *dev = (hndrte_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;

	return pdev->slot;
}

uint32
osl_pci_read_config(osl_t *osh, uint offset, uint size)
{
	hndrte_dev_t *dev = (hndrte_dev_t *)osh->dev;
	pdev_t *pdev = (pdev_t *)dev->pdev;
	uint32 data;

	if (extpci_read_config(hndrte_sih, pdev->bus, pdev->slot, pdev->func, offset,
	                       &data, size) != 0)
		data = 0xffffffff;

	printf("%s: cfgrd for %d-%d-%d 0x%x/%d => 0x%x\n", __FUNCTION__,
	       pdev->bus, pdev->slot, pdev->func, offset, size, data);
	return data;
}

void
osl_pci_write_config(osl_t *osh, uint offset, uint size, uint val)
{
	hndrte_dev_t *dev = (hndrte_dev_t *)osh->dev;
	pdev_t *pdev = dev->pdev;

	printf("%s: cfgwr for %d-%d-%d 0x%x =>0x%x/%d\n", __FUNCTION__,
	       pdev->bus, pdev->slot, pdev->func, val, offset, size);
	extpci_write_config(hndrte_sih, pdev->bus, pdev->slot, pdev->func, offset, &val, size);
}
#endif /* SBPCI */

void *
osl_pktget(osl_t *osh, uint len)
{
	void *pkt;
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
	if ((pkt = (void *)lb_alloc(len, __FILE__, __LINE__)))
#else
	if ((pkt = (void *)lb_alloc(len)))
#endif
		osh->pktalloced++;

	return pkt;
}

void
osl_pktfree(osl_t *osh, void* p, bool send)
{
	struct lbuf *nlb;

	if (send && osh->tx_fn)
		osh->tx_fn(osh->tx_ctx, p, 0);

	for (nlb = (struct lbuf *)p; nlb; nlb = nlb->next) {
		ASSERT(osh->pktalloced > 0);
		osh->pktalloced--;
	}

	lb_free((struct lbuf *)p);
}

void *
osl_pktdup(osl_t *osh, void *p)
{
	void *pkt;
	if ((pkt = (void *)lb_dup((struct lbuf *)p)))
		osh->pktalloced++;

	return pkt;
}

void *
osl_pktfrmnative(osl_t *osh, struct lbuf *lb)
{
	struct lbuf *nlb;

	for (nlb = lb; nlb; nlb = nlb->next)
		osh->pktalloced++;

	return ((void *)lb);
}

struct lbuf *
osl_pkttonative(osl_t *osh, void *p)
{
	struct lbuf *nlb;

	for (nlb = (struct lbuf *)p; nlb; nlb = nlb->next) {
		ASSERT(osh->pktalloced > 0);
		osh->pktalloced--;
	}

	return ((struct lbuf *)p);
}
