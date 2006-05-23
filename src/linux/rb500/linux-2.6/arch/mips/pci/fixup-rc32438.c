/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI fixups for IDT EB438 board
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
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
 *
 **************************************************************************
 * May 2004 P. Sadik
 *
 * Initial Release
 *
 * 
 *
 **************************************************************************
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/idt-boards/rc32438/rc32438.h>
#include <asm/idt-boards/rc32438/rc32438_pci.h>
#include <asm/idt-boards/rc32438/rc32438_pci_v.h>

int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{

  if (dev->bus->number != 0) {
    return 0;
  }

  slot = PCI_SLOT(dev->devfn);
  dev->irq = 0;

  if (slot > 0 && slot <= 5) {
    unsigned char pin;
    pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);

    switch (pin) {
    case 1: /* INTA*/
      dev->irq = GROUP4_IRQ_BASE + 27;
      break;
    case 2: /* INTB */
      dev->irq = GROUP4_IRQ_BASE + 27;
      break;
    case 3: /* INTC */
      dev->irq = GROUP4_IRQ_BASE + 27;
      break;
    case 4: /* INTD */
      dev->irq = GROUP4_IRQ_BASE + 27;
      break;
    default:
      dev->irq = 0xff; 
      break;
    }
#ifdef DEBUG
    printk("irq fixup: slot %d, pin %d, irq %d\n",
	   slot, pin, dev->irq);
#endif
    pci_write_config_byte(dev, PCI_INTERRUPT_LINE,dev->irq);
  }
  return (dev->irq);
}

struct pci_fixup pcibios_fixups[] = {
  {0}
};
