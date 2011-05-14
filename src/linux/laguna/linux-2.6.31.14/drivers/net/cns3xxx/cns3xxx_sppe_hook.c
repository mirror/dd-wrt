/******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, Version 2, as
 *  published by the Free Software Foundation.
 *
 *  This file is distributed in the hope that it will be useful,
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 *  NONINFRINGEMENT.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this file; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or
 *  visit http://www.gnu.org/licenses/.
 *
 *  This file may also be available under a different license from Cavium.
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#if defined(CONFIG_CNS3XXX_SPPE)
#include <linux/module.h>
#include <linux/cns3xxx/sppe.h>

int sppe_hook_ready = 0;
int (*sppe_func_hook)(SPPE_PARAM *param) = NULL;
int sppe_pci_fp_ready = 0;
int (*sppe_pci_fp_hook)(SPPE_PARAM *param) = NULL;
int sppe_hook_mode = 1;

EXPORT_SYMBOL(sppe_hook_ready);
EXPORT_SYMBOL(sppe_func_hook);
EXPORT_SYMBOL(sppe_pci_fp_ready);
EXPORT_SYMBOL(sppe_pci_fp_hook);
EXPORT_SYMBOL(sppe_hook_mode);

#endif //#if defined(CONFIG_CNS3XXX_SPPE)

