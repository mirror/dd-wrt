/* ipci165_fw.h
 * Linux CAN-bus device driver firmware for IXXAT iPC-I 165 (PCI) compatible HW.
 *
 * email:kalas@unicontrols.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

struct ipci165_fw_t {
  unsigned char len;
  unsigned long addr;
  unsigned char a_data[16];
};

extern struct ipci165_fw_t ipci165_fw[];
