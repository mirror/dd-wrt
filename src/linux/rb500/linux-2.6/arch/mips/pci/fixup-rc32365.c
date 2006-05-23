/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     PCI fixups for IDT EB365 board
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

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/idt-boards/rc32300/rc32300.h>
#include <asm/idt-boards/rc32300/rc32365.h>

int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{

    if (dev->bus->number != 0) {
      return 0;
    }
    
    slot = PCI_SLOT(dev->devfn);
    dev->irq = 0;

    if (slot >= 2 && slot <= 4) {
      pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &pin);
      switch (slot) {
      case 2: /* first SLOT*/
	switch (pin) {
	case 1: /* INTA*/
	  dev->irq = RC32365_PCI_INTB_IRQ;
	  break;
	case 2: /* INTB */
	  dev->irq = RC32365_PCI_INTC_IRQ;
	  break;
	case 3: /* INTC */
	  dev->irq = RC32365_PCI_INTD_IRQ;
	  break;
	case 4: /* INTD */
	  dev->irq = RC32365_PCI_INTA_IRQ;
	  break;
	default:
	  dev->irq = 0xff; 
	  break;
	}
	break;
	
      case 3: /* second SLOT */
	switch (pin) {
	case 1: /* INTA*/
	  dev->irq = RC32365_PCI_INTC_IRQ;
	  break;
	case 2: /* INTB */
	  dev->irq = RC32365_PCI_INTD_IRQ;
	  break;
	case 3: /* INTC */
	  dev->irq = RC32365_PCI_INTA_IRQ;
	  break;
	case 4: /* INTD */
	  dev->irq = RC32365_PCI_INTB_IRQ;
	  break;
	default:
	  dev->irq = 0xff; 
	  break;
	}
	break;
	
      case 4: /* miniPCI SLOT */
	switch (pin) {
	case 1: /* INTA*/
	  dev->irq = RC32365_PCI_INTA_IRQ;
	  break;
	case 2: /* INTB */
	  dev->irq = RC32365_PCI_INTB_IRQ;
	  break;
	case 3: /* INTC */
	  dev->irq = RC32365_PCI_INTC_IRQ;
	  break;
	case 4: /* INTD */
	  dev->irq = RC32365_PCI_INTD_IRQ;
	  break;
	default:
	  dev->irq = 0xff; 
	  break;
	}
	break;
      
#ifdef DEBUG
      printk("irq fixup: slot %d, pin %d, irq %d\n",
	     slot, pin, dev->irq);
#endif
      pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
      }
    }
    return(dev->irq);
}
struct pci_fixup pcibios_fixups[] __initdata  = {
  {0}
};
