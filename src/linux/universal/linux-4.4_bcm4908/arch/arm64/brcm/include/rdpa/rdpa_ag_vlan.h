// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2, as published by
// the Free Software Foundation (the "GPL").
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// 
// A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
// writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
// 
// :>
/*
 * vlan object header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_AG_VLAN_H_
#define _RDPA_AG_VLAN_H_

/** \addtogroup vlan
 * @{
 */


/** Get vlan type handle.
 *
 * This handle should be passed to bdmf_new_and_set() function in
 * order to create a vlan object.
 * \return vlan type handle
 */
bdmf_type_handle rdpa_vlan_drv(void);

/* vlan: Attribute types */
typedef enum {
    rdpa_vlan_attr_name = 0, /* name : KRI : string/32 : unique container name */
    rdpa_vlan_attr_vid_enable = 1, /* vid_enable : RWF : bool/1[4096] : VID enabled */
} rdpa_vlan_attr_types;

extern int (*f_rdpa_vlan_get)(const char * name_, bdmf_object_handle *pmo);

/** Get vlan object by key.

 * This function returns vlan object instance by key.
 * \param[in] name_    Object key
 * \param[out] vlan_obj    Object handle
 * \return    0=OK or error <0
 */
int rdpa_vlan_get(const char * name_, bdmf_object_handle *vlan_obj);

/** Get vlan/name attribute.
 *
 * Get unique container name.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[out]  name_ Attribute value
 * \param[in]   size_ buffer size
 * \return number of bytes read >=0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_name_get(bdmf_object_handle mo_, char * name_, uint32_t size_)
{
    return bdmf_attr_get_as_string(mo_, rdpa_vlan_attr_name, name_, size_);
}


/** Set vlan/name attribute.
 *
 * Set unique container name.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   name_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task and softirq contexts.
 */
static inline int rdpa_vlan_name_set(bdmf_object_handle mo_, const char * name_)
{
    return bdmf_attr_set_as_string(mo_, rdpa_vlan_attr_name, name_);
}


/** Get vlan/vid_enable attribute entry.
 *
 * Get VID enabled.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[out]  vid_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_vid_enable_get(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean *vid_enable_)
{
    bdmf_number _nn_;
    int _rc_;
    _rc_ = bdmf_attrelem_get_as_num(mo_, rdpa_vlan_attr_vid_enable, (bdmf_index)ai_, &_nn_);
    *vid_enable_ = (bdmf_boolean)_nn_;
    return _rc_;
}


/** Set vlan/vid_enable attribute entry.
 *
 * Set VID enabled.
 * \param[in]   mo_ vlan object handle or mattr transaction handle
 * \param[in]   ai_ Attribute array index
 * \param[in]   vid_enable_ Attribute value
 * \return 0 or error code < 0
 * The function can be called in task context only.
 */
static inline int rdpa_vlan_vid_enable_set(bdmf_object_handle mo_, bdmf_index ai_, bdmf_boolean vid_enable_)
{
    return bdmf_attrelem_set_as_num(mo_, rdpa_vlan_attr_vid_enable, (bdmf_index)ai_, vid_enable_);
}

/** @} end of vlan Doxygen group */




#endif /* _RDPA_AG_VLAN_H_ */
