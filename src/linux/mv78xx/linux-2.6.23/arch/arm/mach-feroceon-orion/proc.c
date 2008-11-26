/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>
#include <linux/proc_fs.h>
 
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
 
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
 
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "gpp/mvGpp.h"
#include "mvOs.h"

#if defined (CONFIG_MV_GATEWAY)
#include "msApi.h"
extern GT_BOOL ReadMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int* value);
extern GT_BOOL WriteMiiWrap(GT_QD_DEV* dev, unsigned int portNumber, unsigned int MIIReg, unsigned int data);
#elif defined (CONFIG_MV_ETHERNET)
extern int ReadMiiWrap(unsigned int portNumber, unsigned int MIIReg, unsigned int* value);
extern int WriteMiiWrap(unsigned int portNumber, unsigned int MIIReg, unsigned int data);
#endif

/* global variables from 'regdump' */
static struct proc_dir_entry *evb_resource_dump;
static u32 evb_resource_dump_request , evb_resource_dump_result;
                                                                                                                             
/* Some service routines */
                                                                                                                             
static int ishex (char ch) {
  if (((ch>='0') && (ch <='9')) || ((ch>='a') && (ch <='f')) || ((ch >='A') && (ch <='F'))) return 1;
  return 0;
}
                                                                                                                             
static int hex_value (char ch) {
  if ((ch >= '0') && (ch <= '9')) return ch-'0';
  if ((ch >= 'a') && (ch <= 'f')) return ch-'a'+10;
  if ((ch >= 'A') && (ch <= 'F')) return ch-'A'+10;
  return 0;
}
                                                                                                                             
static int atoh(char *s , int len) {
  int i=0;
  while (ishex(*s) && len--) {
    i = i*0x10 + hex_value(*s);
    s++;
  }
  return i;
}
                                                                                                                             
/* The format of writing to this module is as follows -
   char 0 - r/w (Reading from register or Writing to register/memory)
   char 1 - space
   char 2 - register/mem_addr offset 7
   char 3 - register/mem_addr offset 6
   char 4 - register/mem_addr offset 5
   char 5 - register/mem_addr offset 4
   char 6 - register/mem_addr offset 3
   char 7 - register/mem_addr offset 2
   char 8 - register/mem_addr offset 1
   char 9 - register/mem_addr offset 0
   // The following is valid only if write request
   char 10 - space
   char 11 - register/mem_addr value 7
   char 12 - register/mem_addr value 6
   char 13 - register/mem_addr value 5
   char 14 - register/mem_addr value 4
   char 15 - register/mem_addr value 3
   char 16 - register/mem_addr value 2
   char 17 - register/mem_addr value 1
   char 18 - register/mem_addr value 0
 
*/
 
/********************************************************************
* evb_resource_dump_write -
*
* When written to the /proc/resource_dump file this function is called
*
* Inputs: file / data are not used. Buffer and count are the pointer
*         and length of the input string
* Returns: Read from GT register
* Outputs: count
*********************************************************************/
int evb_resource_dump_write (struct file *file, const char *buffer,
                      unsigned long count, void *data) {
  
  /* Reading / Writing from system controller internal registers */
  if (!strncmp (buffer , "register" , 8)) {
    if (buffer[10] == 'r') {
      evb_resource_dump_request = atoh((char *)((unsigned int)buffer + 12),8);
      evb_resource_dump_result = MV_REG_READ(evb_resource_dump_request);
    }
    if (buffer[10] == 'w') {
      evb_resource_dump_request = atoh((char *)((unsigned int)buffer + 12), 8);
      evb_resource_dump_result = atoh ((char *)((unsigned int)buffer + 12 + 8 + 1) , 8);
      MV_REG_WRITE (evb_resource_dump_request , evb_resource_dump_result);
    }
  }
 
  /* Reading / Writing from 32bit address - mostly usable for memory */
  if (!strncmp (buffer , "memory  " , 8)) {
    if (buffer[10] == 'r') {
      evb_resource_dump_request = atoh((char *)((unsigned int)buffer + 12),8);
      evb_resource_dump_result =  *(unsigned int *)evb_resource_dump_request;
    }
    if (buffer[10] == 'w') {
      evb_resource_dump_request = atoh((char *)((unsigned int)buffer + 12), 8);
      evb_resource_dump_result = atoh ((char *)((unsigned int)buffer + 12 + 8 + 1) , 8);
      * (unsigned int *) evb_resource_dump_request = evb_resource_dump_result;
    }
  }
#if (defined (CONFIG_MV_ETHERNET) || defined (CONFIG_MV_GATEWAY)) && !defined(CONFIG_GTW_LOADABLE_DRV)
  /* Reading / Writing from a rgister via SMI */
  if (!strncmp (buffer , "smi" , 3)) {

    unsigned int dev_addr = atoh((char *)((unsigned int)buffer + 7),8);

    if (buffer[5] == 'r') {
      evb_resource_dump_request = atoh((char *)((unsigned int)buffer + 7 + 8 + 1),8);
      evb_resource_dump_result = 0;
#if defined (CONFIG_MV_ETHERNET) 
      ReadMiiWrap(dev_addr, evb_resource_dump_request, &evb_resource_dump_result); 
#elif defined (CONFIG_MV_GATEWAY)
      ReadMiiWrap(NULL, dev_addr, evb_resource_dump_request, &evb_resource_dump_result); 
#endif
    }
    if (buffer[5] == 'w') {
      evb_resource_dump_request = atoh((char *)((unsigned int)buffer + 7 + 8 + 1), 8);
      evb_resource_dump_result = atoh ((char *)((unsigned int)buffer + 7 + 8  + 8 + 1) , 8);
#if defined (CONFIG_MV_ETHERNET)
      WriteMiiWrap(dev_addr, evb_resource_dump_request , evb_resource_dump_result);
#elif defined (CONFIG_MV_GATEWAY)
      WriteMiiWrap(NULL, dev_addr, evb_resource_dump_request , evb_resource_dump_result);
#endif
    }
  }
#endif
    
  return count;
}

/********************************************************************
* evb_resource_dump_read -
*
* When read from the /proc/resource_dump file this function is called
*
* Inputs: buffer_location and buffer_length and zero are not used.
*         buffer is the pointer where to post the result
* Returns: N/A
* Outputs: length of string posted
*********************************************************************/
int evb_resource_dump_read (char *buffer, char **buffer_location, off_t offset,
                            int buffer_length, int *zero, void *ptr) {
                                                                                                                             
  if (offset > 0)
    return 0;
  return sprintf(buffer, "%08x\n", evb_resource_dump_result);
}

/********************************************************************
* start_regdump_memdump -
*
* Register the /proc/regdump file at the /proc filesystem
* Register the /proc/memdump file at the /proc filesystem
*
* Inputs: N/A
* Returns: N/A
* Outputs: N/A
*********************************************************************/
int __init start_resource_dump(void)
{
  evb_resource_dump = create_proc_entry ("resource_dump" , 0666 , &proc_root);
  evb_resource_dump->read_proc = evb_resource_dump_read;
  evb_resource_dump->write_proc = evb_resource_dump_write;
  evb_resource_dump->nlink = 1;
  return 0;
}
                                                                                                                             
module_init(start_resource_dump);
 


/* global variables from 'regdump' */
static struct proc_dir_entry *soc_type;
static struct proc_dir_entry *board_type;

/********************************************************************
* soc_type_read -
*********************************************************************/
int soc_type_read (char *buffer, char **buffer_location, off_t offset,
                            int buffer_length, int *zero, void *ptr) {
                                                                                                                             
  if (offset > 0)
    return 0;
  return sprintf(buffer, "%s%x Rev %d\n", SOC_NAME_PREFIX, mvCtrlModelGet(), mvCtrlRevGet());
}
/********************************************************************
* board_type_read -
*********************************************************************/
int board_type_read (char *buffer, char **buffer_location, off_t offset,
                            int buffer_length, int *zero, void *ptr) {
  char name_buff[50];
  if (offset > 0)
    return 0;
  mvBoardNameGet(name_buff);
  return sprintf(buffer, "%s\n", name_buff);
}

/********************************************************************
* start_soc_type -
*********************************************************************/
int __init start_soc_type(void)
{
  soc_type = create_proc_entry ("soc_type" , 0666 , &proc_root);
  soc_type->read_proc = soc_type_read;
  soc_type->write_proc = NULL;
  soc_type->nlink = 1;

  board_type = create_proc_entry ("board_type" , 0666 , &proc_root);
  board_type->read_proc = board_type_read;
  board_type->write_proc = NULL;
  board_type->nlink = 1;

  return 0;
}
                                                                                                                             
module_init(start_soc_type);


