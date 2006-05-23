/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI fixups for Routerboard 532 board
 *
 *  Original (C) 2004 IDT Inc. (rischelp@idt.com)
 *  Adaptation: (C) 2006 P.Christeas (p_christeas@yahoo.com)
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
 *
 **************************************************************************
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/rb/rb500.h>


static int irq_map[2][12] = {
	{ 0, 0, 2, 3, 2, 3, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 1, 3, 0, 2, 1, 3, 0, 2, 1, 3 }
};
void __init pcibios_fixup_resources(struct pci_dev *dev)
{
	if (PCI_SLOT(dev->devfn) == 6 && dev->bus->number == 0) {
		/* disable prefetched memory range */
		pci_write_config_word(dev, PCI_PREF_MEMORY_LIMIT, 0);
		pci_write_config_word(dev, PCI_PREF_MEMORY_BASE, 0x10);

		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 4);
	}
}


int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	if (dev->bus->number < 2 && PCI_SLOT(dev->devfn) < 12) {
		dev->irq = irq_map[dev->bus->number][PCI_SLOT(dev->devfn)];
		dev->irq += GROUP4_IRQ_BASE + 4;
		printk("irq fixup: slot %d, pin %d, irq %d\n",
		       slot, pin, dev->irq);
		pci_write_config_byte(dev, PCI_INTERRUPT_LINE,dev->irq);
	}
	return (dev->irq);
}

struct pci_fixup pcibios_fixups[] = {
	{0}
};











