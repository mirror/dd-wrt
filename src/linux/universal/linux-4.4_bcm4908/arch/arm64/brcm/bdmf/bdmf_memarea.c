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
 * bdmf_area.c
 *
 * Data path builder - memory area access
 *
 * This file is Copyright (c) 2011, Broadlight Communications.
 * This file is licensed under GNU Public License, except that if
 * you have entered in to a signed, written license agreement with
 * Broadlight covering this file, that agreement applies to this
 * file instead of the GNU Public License.
 *
 * This file is free software: you can redistribute and/or modify it
 * under the terms of the GNU Public License, Version 2, as published
 * by the Free Software Foundation, unless a different license
 * applies as provided above.
 *
 * This program is distributed in the hope that it will be useful,
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied
 * warranties of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * TITLE or NONINFRINGEMENT. Redistribution, except as permitted by
 * the GNU Public License or another license agreement between you
 * and Broadlight, is prohibited.
 *
 * You should have received a copy of the GNU Public License,
 * Version 2 along with this file; if not, see
 * <http://www.gnu.org/licenses>.
 *
 * Author: Igor Ternovsky
 *******************************************************************/

#include <bdmf_dev.h>

static struct bdmf_mem_area *bdmf_areas[BDMF_MEM__NUMBER_OF];

static int bdmf_dft_mem_access(struct bdmf_mem_area *area, void *dst, const void *src, uint32_t size)
{
    memcpy(dst, src, size);
    return size;
}

static void *bdmf_dft_mem_alloc(struct bdmf_mem_area *area, unsigned long start,
                                uint32_t size, uint32_t align)
{
    return bdmf_alloc(size);
}

static void bdmf_dft_mem_free(struct bdmf_mem_area *area, void *ptr)
{
    bdmf_free(ptr);
}

static struct bdmf_mem_area bdmf_dft_mem_area = {
    .name = "default",
    .read = bdmf_dft_mem_access,
    .write = bdmf_dft_mem_access,
    .alloc = bdmf_dft_mem_alloc,
    .free = bdmf_dft_mem_free
};

int bdmf_mem_area_register(struct bdmf_mem_area *area)
{
    bdmf_areas[area->mem_type] = area;
    return 0;
}

void bdmf_mem_area_unregister(struct bdmf_mem_area *area)
{
    bdmf_areas[area->mem_type] = &bdmf_dft_mem_area;
}

int bdmf_mem_read(bdmf_mem_type_t mem_type, void *dst, const void *src, uint32_t size)
{
    struct bdmf_mem_area *area=bdmf_areas[mem_type];
    return area->read(area, dst, src, size);
}

int bdmf_mem_write(bdmf_mem_type_t mem_type, void *dst, const void *src, uint32_t size)
{
    struct bdmf_mem_area *area=bdmf_areas[mem_type];
    return area->write(area, dst, src, size);
}

void *bdmf_mem_alloc_ext(struct bdmf_object *mo, bdmf_mem_type_t mem_type,
                unsigned long start, uint32_t size, uint32_t align)
{
    struct bdmf_mem_area *area=bdmf_areas[mem_type];
    return area->alloc(area, start, size, align);
}

void bdmf_mem_free(bdmf_mem_type_t mem_type, void *ptr)
{
    struct bdmf_mem_area *area=bdmf_areas[mem_type];
    return area->free(area, ptr);
}

int bdmf_area_module_init(void)
{
    int i;
    for (i=0; i<BDMF_MEM__NUMBER_OF; i++)
        bdmf_areas[i] = &bdmf_dft_mem_area;
    return 0;
}

void bdmf_area_module_exit(void)
{
    int i;
    for (i=0; i<BDMF_MEM__NUMBER_OF; i++)
        bdmf_areas[i] = &bdmf_dft_mem_area;
}
