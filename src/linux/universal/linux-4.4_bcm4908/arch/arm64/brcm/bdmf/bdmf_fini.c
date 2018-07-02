/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/

/*******************************************************************
 * bdmf_fini.c
 *
 * BDMF framework - terminate type and aggregate type sections
 * This file must be given to the linker last!
 *
 *******************************************************************/

unsigned long bdmf_type_section_end __attribute__((section("BDMF_init"))) = 0;
unsigned long bdmf_aggr_section_end __attribute__((section("BDMF_aggr_init"))) = 0;
