// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/iopoll.h>

#include "mgag200_drv.h"

void mgag200_bmc_disable_vidrst(struct mga_device *mdev)
{
	u8 tmp;
	int ret;

	/*
	 * 1 - The first step is to inform the BMC of an upcoming mode
	 * change. We are putting the misc<0> to output.
	 */

	WREG8(DAC_INDEX, MGA1064_GEN_IO_CTL);
	tmp = RREG8(DAC_DATA);
	tmp |= 0x10;
	WREG_DAC(MGA1064_GEN_IO_CTL, tmp);

	/* we are putting a 1 on the misc<0> line */
	WREG8(DAC_INDEX, MGA1064_GEN_IO_DATA);
	tmp = RREG8(DAC_DATA);
	tmp |= 0x10;
	WREG_DAC(MGA1064_GEN_IO_DATA, tmp);

	/*
	 * 2- Second step to mask any further scan request. This is
	 * done by asserting the remfreqmsk bit (XSPAREREG<7>)
	 */

	WREG8(DAC_INDEX, MGA1064_SPAREREG);
	tmp = RREG8(DAC_DATA);
	tmp |= 0x80;
	WREG_DAC(MGA1064_SPAREREG, tmp);

	/*
	 * 3a- The third step is to verify if there is an active scan.
	 * We are waiting for a 0 on remhsyncsts (<XSPAREREG<0>).
	 */
	ret = read_poll_timeout(RREG_DAC, tmp, !(tmp & 0x1),
				1000, 300000, false,
				MGA1064_SPAREREG);
	if (ret == -ETIMEDOUT)
		return;

	/*
	 * 3b- This step occurs only if the remote BMC is actually
	 * scanning. We are waiting for the end of the frame which is
	 * a 1 on remvsyncsts (XSPAREREG<1>)
	 */
	(void)read_poll_timeout(RREG_DAC, tmp, (tmp & 0x2),
				1000, 300000, false,
				MGA1064_SPAREREG);
}

void mgag200_bmc_enable_vidrst(struct mga_device *mdev)
{
	u8 tmp;

	/* Ensure that the vrsten and hrsten are set */
	WREG8(MGAREG_CRTCEXT_INDEX, 1);
	tmp = RREG8(MGAREG_CRTCEXT_DATA);
	WREG8(MGAREG_CRTCEXT_DATA, tmp | 0x88);

	/* Assert rstlvl2 */
	WREG8(DAC_INDEX, MGA1064_REMHEADCTL2);
	tmp = RREG8(DAC_DATA);
	tmp |= 0x8;
	WREG8(DAC_DATA, tmp);

	udelay(10);

	/* Deassert rstlvl2 */
	tmp &= ~0x08;
	WREG8(DAC_INDEX, MGA1064_REMHEADCTL2);
	WREG8(DAC_DATA, tmp);

	/* Remove mask of scan request */
	WREG8(DAC_INDEX, MGA1064_SPAREREG);
	tmp = RREG8(DAC_DATA);
	tmp &= ~0x80;
	WREG8(DAC_DATA, tmp);

	/* Put back a 0 on the misc<0> line */
	WREG8(DAC_INDEX, MGA1064_GEN_IO_DATA);
	tmp = RREG8(DAC_DATA);
	tmp &= ~0x10;
	WREG_DAC(MGA1064_GEN_IO_DATA, tmp);
}
