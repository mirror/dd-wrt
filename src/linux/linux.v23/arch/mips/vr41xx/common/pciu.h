/*
 * arch/mips/vr41xx/common/pciu.h
 *
 * Include file for PCI Control Unit of the NEC VR4100 series.
 *
 * Author: Yoichi Yuasa <yyuasa@mvista.com, or source@mvista.com>
 *
 * 2002-2003 (c) MontaVista, Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */
/*
 * Changes:
 *  MontaVista Software Inc. <yyuasa@mvista.com> or <source@mvista.com>
 *  - New creation, NEC VR4122 and VR4131 are supported.
 */
#ifndef __VR41XX_PCIU_H
#define __VR41XX_PCIU_H

#include <linux/config.h>
#include <asm/addrspace.h>

#define BIT(x)	(1U << (x))

#define PCIMMAW1REG			KSEG1ADDR(0x0f000c00)
#define PCIMMAW2REG			KSEG1ADDR(0x0f000c04)
#define PCITAW1REG			KSEG1ADDR(0x0f000c08)
#define PCITAW2REG			KSEG1ADDR(0x0f000c0c)
#define PCIMIOAWREG			KSEG1ADDR(0x0f000c10)
#define INTERNAL_BUS_BASE_ADDRESS	0xff000000U
#define ADDRESS_MASK			0x000fe000U
#define PCI_ACCESS_ENABLE		BIT(12)
#define PCI_ADDRESS_SETTING		0x000000ffU

#define PCICONFDREG			KSEG1ADDR(0x0f000c14)
#define PCICONFAREG			KSEG1ADDR(0x0f000c18)
#define PCIMAILREG			KSEG1ADDR(0x0f000c1c)

#define BUSERRADREG			KSEG1ADDR(0x0f000c24)
#define ERROR_ADDRESS			0xfffffffcU

#define INTCNTSTAREG			KSEG1ADDR(0x0f000c28)
#define MABTCLR				BIT(31)
#define TRDYCLR				BIT(30)
#define PARCLR				BIT(29)
#define MBCLR				BIT(28)
#define SERRCLR				BIT(27)

#define PCIEXACCREG			KSEG1ADDR(0x0f000c2c)
#define UNLOCK				BIT(1)
#define EAREQ				BIT(0)

#define PCIRECONTREG			KSEG1ADDR(0x0f000c30)
#define RTRYCNT				0xffU

#define PCIENREG			KSEG1ADDR(0x0f000c34)
#define CONFIG_DONE			BIT(2)

#define PCICLKSELREG			KSEG1ADDR(0x0f000c38)
#define EQUAL_VTCLOCK			0x2U
#define HALF_VTCLOCK			0x0U
#define QUARTER_VTCLOCK			0x1U

#define PCITRDYVREG			KSEG1ADDR(0x0f000c3c)

#define PCICLKRUNREG			KSEG1ADDR(0x0f000c60)

#define VENDORIDREG			KSEG1ADDR(0x0f000d00)

#define MPCIINTREG			KSEG1ADDR(0x0f0000b2)

#define MAX_PCI_CLOCK			33333333

static inline int pciu_read_config_byte(int where, uint8_t *val)
{
	uint32_t data;

	if (where > 0xff)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	data = readl(VENDORIDREG + (where & 0xfc));
	*val = (uint8_t)(data >> ((where & 3) << 3));

	return PCIBIOS_SUCCESSFUL;
}

static inline int pciu_read_config_word(int where, uint16_t *val)
{
	uint32_t data;

	if (where > 0xff || (where & 1))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	data = readl(VENDORIDREG + (where & 0xfc));
	*val = (uint16_t)(data >> ((where & 2) << 3));

	return PCIBIOS_SUCCESSFUL;
}

static inline int pciu_read_config_dword(int where, uint32_t *val)
{
	if (where > 0xff || (where & 3))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	*val = readl(VENDORIDREG + where);

	return PCIBIOS_SUCCESSFUL;
}

static inline int pciu_write_config_byte(int where, uint8_t val)
{
	uint32_t data;
	int shift;

	if (where > 0xff)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	data = readl(VENDORIDREG + (where & 0xfc));
	shift = (where & 3) << 3;
	data &= ~(0xffU << shift);
	data |= (uint32_t)val << shift;
	writel(data, VENDORIDREG + (where & 0xfc));

	return 0;
}

static inline int pciu_write_config_word(int where, uint16_t val)
{
	uint32_t data;
	int shift;

	if (where > 0xff || (where & 1))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	data = readl(VENDORIDREG + (where & 0xfc));
	shift = (where & 2) << 3;
	data &= ~(0xffffU << shift);
	data |= (uint32_t)val << shift;
	writel(data, VENDORIDREG + (where & 0xfc));

	return 0;
}

static inline int pciu_write_config_dword(int where, uint32_t val)
{
	if (where > 0xff || (where & 3))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	writel(val, VENDORIDREG + where);

	return 0;
}

#endif /* __VR41XX_PCIU_H */
