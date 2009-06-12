/*
 *
 * BRIEF MODULE DESCRIPTION
 *	Include file for Alchemy Semiconductor's Au1k CPU.
 *
 * Copyright 2004 Embedded Edge, LLC
 *	dan@embeddededge.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Specifics for the Au1xxx Programmable Serial Controllers, first
 * seen in the AU1550 part.
 */
#ifndef _AU1000_PSC_H_
#define _AU1000_PSC_H_

/* The PSC base addresses.
*/
#define PSC_BASE0		0xb1a00000
#define PSC_BASE1		0xb1b00000
#define PSC_BASE2		0xb0a00000
#define PSC_BASE3		0xb0d00000

/* These should be defined in a board specific file!
*/
#ifdef CONFIG_MIPS_PB1550
#define SPI_PSC_BASE		PSC_BASE0
#define AC97_PSC_BASE		PSC_BASE1
#define SMBUS_PSC_BASE		PSC_BASE2
#endif
#ifdef CONFIG_MIPS_DB1550
#define SPI_PSC_BASE		PSC_BASE0
#define AC97_PSC_BASE		PSC_BASE1
#define SMBUS_PSC_BASE		PSC_BASE2
#endif


/* The PSC select and control registers are common to
 * all protocols.
 */
#define PSC_SEL_OFFSET		0x00000000
#define PSC_CTRL_OFFSET		0x00000004

#define PSC_SEL_CLK_MASK	(3 << 4)
#define PSC_SEL_CLK_INTCLK	(0 << 4)
#define PSC_SEL_CLK_EXTCLK	(1 << 4)
#define PSC_SEL_CLK_SERCLK	(2 << 4)

#define PSC_SEL_PS_MASK		0x00000007
#define PSC_SEL_PS_DISABLED	(0)
#define PSC_SEL_PS_SPIMODE	(2)
#define PSC_SEL_PS_I2SMODE	(3)
#define PSC_SEL_PS_AC97MODE	(4)
#define PSC_SEL_PS_SMBUSMODE	(5)

#define PSC_CTRL_DISABLE	(0)
#define PSC_CTRL_SUSPEND	(2)
#define PSC_CTRL_ENABLE		(3)

/* AC97 Registers.
*/
#define PSC_AC97CFG_OFFSET	0x00000008
#define PSC_AC97MSK_OFFSET	0x0000000c
#define PSC_AC97PCR_OFFSET	0x00000010
#define PSC_AC97STAT_OFFSET	0x00000014
#define PSC_AC97EVNT_OFFSET	0x00000018
#define PSC_AC97TXRX_OFFSET	0x0000001c
#define PSC_AC97CDC_OFFSET	0x00000020
#define PSC_AC97RST_OFFSET	0x00000024
#define PSC_AC97GPO_OFFSET	0x00000028
#define PSC_AC97GPI_OFFSET	0x0000002c

#define AC97_PSC_SEL		(AC97_PSC_BASE + PSC_SEL_OFFSET)
#define AC97_PSC_CTRL		(AC97_PSC_BASE + PSC_CTRL_OFFSET)
#define PSC_AC97CFG		(AC97_PSC_BASE + PSC_AC97CFG_OFFSET)
#define PSC_AC97MSK		(AC97_PSC_BASE + PSC_AC97MSK_OFFSET)
#define PSC_AC97PCR		(AC97_PSC_BASE + PSC_AC97PCR_OFFSET)
#define PSC_AC97STAT		(AC97_PSC_BASE + PSC_AC97STAT_OFFSET)
#define PSC_AC97EVNT		(AC97_PSC_BASE + PSC_AC97EVNT_OFFSET)
#define PSC_AC97TXRX		(AC97_PSC_BASE + PSC_AC97TXRX_OFFSET)
#define PSC_AC97CDC		(AC97_PSC_BASE + PSC_AC97CDC_OFFSET)
#define PSC_AC97RST		(AC97_PSC_BASE + PSC_AC97RST_OFFSET)
#define PSC_AC97GPO		(AC97_PSC_BASE + PSC_AC97GPO_OFFSET)
#define PSC_AC97GPI		(AC97_PSC_BASE + PSC_AC97GPI_OFFSET)

/* AC97 Config Register.
*/
#define PSC_AC97CFG_RT_MASK	(3 << 30)
#define PSC_AC97CFG_RT_FIFO1	(0 << 30)
#define PSC_AC97CFG_RT_FIFO2	(1 << 30)
#define PSC_AC97CFG_RT_FIFO4	(2 << 30)
#define PSC_AC97CFG_RT_FIFO8	(3 << 30)

#define PSC_AC97CFG_TT_MASK	(3 << 28)
#define PSC_AC97CFG_TT_FIFO1	(0 << 28)
#define PSC_AC97CFG_TT_FIFO2	(1 << 28)
#define PSC_AC97CFG_TT_FIFO4	(2 << 28)
#define PSC_AC97CFG_TT_FIFO8	(3 << 28)

#define PSC_AC97CFG_DD_DISABLE	(1 << 27)
#define PSC_AC97CFG_DE_ENABLE	(1 << 26)
#define PSC_AC97CFG_SE_ENABLE	(1 << 25)

#define PSC_AC97CFG_LEN_MASK	(0xf << 21)
#define PSC_AC97CFG_TXSLOT_MASK	(0x3ff << 11)
#define PSC_AC97CFG_RXSLOT_MASK	(0x3ff << 1)
#define PSC_AC97CFG_GE_ENABLE	(1)

/* Enable slots 3-12.
*/
#define PSC_AC97CFG_TXSLOT_ENA(x)	(1 << (((x) - 3) + 11))
#define PSC_AC97CFG_RXSLOT_ENA(x)	(1 << (((x) - 3) + 1))

/* The word length equation is ((x) * 2) + 2, so choose 'x' appropriately.
 * The only sensible numbers are 7, 9, or possibly 11.  Nah, just do the
 * arithmetic in the macro.
 */
#define PSC_AC97CFG_SET_LEN(x)	(((((x)-2)/2) & 0xf) << 21)
#define PSC_AC97CFG_GET_LEN(x)	(((((x) >> 21) & 0xf) * 2) + 2)

/* AC97 Mask Register.
*/
#define PSC_AC97MSK_GR		(1 << 25)
#define PSC_AC97MSK_CD		(1 << 24)
#define PSC_AC97MSK_RR		(1 << 13)
#define PSC_AC97MSK_RO		(1 << 12)
#define PSC_AC97MSK_RU		(1 << 11)
#define PSC_AC97MSK_TR		(1 << 10)
#define PSC_AC97MSK_TO		(1 << 9)
#define PSC_AC97MSK_TU		(1 << 8)
#define PSC_AC97MSK_RD		(1 << 5)
#define PSC_AC97MSK_TD		(1 << 4)
#define PSC_AC97MSK_ALLMASK	(PSC_AC97MSK_GR | PSC_AC97MSK_CD | \
				 PSC_AC97MSK_RR | PSC_AC97MSK_RO | \
				 PSC_AC97MSK_RU | PSC_AC97MSK_TR | \
				 PSC_AC97MSK_TO | PSC_AC97MSK_TU | \
				 PSC_AC97MSK_RD | PSC_AC97MSK_TD)

/* AC97 Protocol Control Register.
*/
#define PSC_AC97PCR_RC		(1 << 6)
#define PSC_AC97PCR_RP		(1 << 5)
#define PSC_AC97PCR_RS		(1 << 4)
#define PSC_AC97PCR_TC		(1 << 2)
#define PSC_AC97PCR_TP		(1 << 1)
#define PSC_AC97PCR_TS		(1 << 0)

/* AC97 Status register (read only).
*/
#define PSC_AC97STAT_CB		(1 << 26)
#define PSC_AC97STAT_CP		(1 << 25)
#define PSC_AC97STAT_CR		(1 << 24)
#define PSC_AC97STAT_RF		(1 << 13)
#define PSC_AC97STAT_RE		(1 << 12)
#define PSC_AC97STAT_RR		(1 << 11)
#define PSC_AC97STAT_TF		(1 << 10)
#define PSC_AC97STAT_TE		(1 << 9)
#define PSC_AC97STAT_TR		(1 << 8)
#define PSC_AC97STAT_RB		(1 << 5)
#define PSC_AC97STAT_TB		(1 << 4)
#define PSC_AC97STAT_DI		(1 << 2)
#define PSC_AC97STAT_DR		(1 << 1)
#define PSC_AC97STAT_SR		(1 << 0)

/* AC97 Event Register.
*/
#define PSC_AC97EVNT_GR		(1 << 25)
#define PSC_AC97EVNT_CD		(1 << 24)
#define PSC_AC97EVNT_RR		(1 << 13)
#define PSC_AC97EVNT_RO		(1 << 12)
#define PSC_AC97EVNT_RU		(1 << 11)
#define PSC_AC97EVNT_TR		(1 << 10)
#define PSC_AC97EVNT_TO		(1 << 9)
#define PSC_AC97EVNT_TU		(1 << 8)
#define PSC_AC97EVNT_RD		(1 << 5)
#define PSC_AC97EVNT_TD		(1 << 4)

/* CODEC Command Register.
*/
#define PSC_AC97CDC_RD		(1 << 25)
#define PSC_AC97CDC_ID_MASK	(3 << 23)
#define PSC_AC97CDC_INDX_MASK	(0x7f << 16)
#define PSC_AC97CDC_ID(x)	(((x) & 0x3) << 23)
#define PSC_AC97CDC_INDX(x)	(((x) & 0x7f) << 16)

/* AC97 Reset Control Register.
*/
#define PSC_AC97RST_RST		(1 << 1)
#define PSC_AC97RST_SNC		(1 << 0)


#endif /* _AU1000_PSC_H_ */
