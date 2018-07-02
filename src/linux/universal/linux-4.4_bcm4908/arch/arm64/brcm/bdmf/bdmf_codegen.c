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
 * bdmf_codegen.c
 *
 * BDMF framework - code generator
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
#include <bdmf_session.h>
#include <bdmf_shell.h>

#define BDMF_ACTION_MASK_ATTR_ENUM              1
#define BDMF_ACTION_MASK_KEY                    2
#define BDMF_ACTION_MASK_ATTR_AGGREGATE         4
#define BDMF_ACTION_MASK_ATTR_ALL               8
#define BDMF_ACTION_MASK_ATTR_ACCESS            0x10
#define BDMF_ACTION_MASK_ATTR_MACCESS           0x20
#define BDMF_ACTION_MASK_ATTR_GEN_ACCESS        0x40

#define BDMF_ACTION_MASK_ALL (  BDMF_ACTION_MASK_ATTR_ENUM    | \
                                BDMF_ACTION_MASK_KEY          | \
                                BDMF_ACTION_MASK_ATTR_ALL     | \
                                BDMF_ACTION_MASK_ATTR_ACCESS  | \
                                BDMF_ACTION_MASK_ATTR_MACCESS | \
                                BDMF_ACTION_MASK_ATTR_GEN_ACCESS )
#define BDMF_ACTION_MASK_ALL_NO_GEN_ACCESS (BDMF_ACTION_MASK_ALL & ~BDMF_ACTION_MASK_ATTR_GEN_ACCESS)

#define BDMF_MAX_TYPE_STACK_DEPTH 32

#define BDMF_FILE_HEADER      1
#define BDMF_FILE_GPL_SHIM    2



/* Function / description.
 * Used for extending auto-generated text
 */
struct func_desc {
    char *name;
    char *desc;
    int is_tagged;      /* 1 if fdesc contains \param or \return tag */
    STAILQ_ENTRY(func_desc) list;
};

static STAILQ_HEAD(_func_desc_list, func_desc) func_desc_list;

#define CODEGEN_FSTART_TAG      "_FUNC"
#define CODEGEN_FEND_TAG        "_EOF"

/* codegen parameter block */
typedef struct {
    bdmf_session_handle session;
    const char *name_prefix;
    uint32_t action_mask;
    int attr_level;
    int gpl_shim;
    struct bdmf_type *drv;
    FILE *hf;
    char attr_id[64];
    char keytype[64];
    char keyname[64];
} codegen_parm_t;

typedef enum {
    func_read,
    func_write,
    func_add,
    func_delete,
    func_get_next,
    func_find
} bdmf_func_type;


static const char *_bdmf_mon_attr_val_type(bdmf_session_handle session, const struct bdmf_attr *attr, int is_read)
{
    static char type_name[64];
    switch(attr->type)
    {
    case bdmf_attr_number:       /**< Numeric attribute */
    case bdmf_attr_enum_mask:    /**< Enum mask attribute */
        if (attr->data_type_name)
            snprintf(type_name, sizeof(type_name), "%s", attr->data_type_name);
        else
            strcpy(type_name, "bdmf_number");
        break;
    case bdmf_attr_string:       /**< 0-terminated string */
        strcpy(type_name, is_read ? "char *" : "const char *");
        break;
    case bdmf_attr_buffer:       /**< Buffer with binary data */
        strcpy(type_name, is_read ? "void *" : "const void *");
        break;
    case bdmf_attr_pointer:      /**< A pointer */
        strcpy(type_name, "void *");
        break;
    case bdmf_attr_object:      /**< Object reference */
        strcpy(type_name, "bdmf_object_handle");
        break;
    case bdmf_attr_ether_addr:   /**< 6-byte Ethernet h/w address */
        strcpy(type_name, is_read ? "bdmf_mac_t *" : "const bdmf_mac_t *");
        break;
    case bdmf_attr_ip_addr:      /**< 4-byte IPv4 address or 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ip_t *" : "const bdmf_ip_t *");
        break;
    case bdmf_attr_ipv4_addr:    /**< 4-byte IPv4 address */
        strcpy(type_name, "bdmf_ipv4");
        break;
    case bdmf_attr_ipv6_addr:    /**< 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ipv6_t *" : "const bdmf_ipv6_t *");
        break;
    case bdmf_attr_boolean:      /**< boolean. default(first) value = true (1) */
        strcpy(type_name, "bdmf_boolean");
        break;
    case bdmf_attr_enum:         /**< enumeration with list of values in static table */
        if (attr->ts.enum_table->type_name)
            snprintf(type_name, sizeof(type_name), "%s", attr->ts.enum_table->type_name);
        else
            strcpy(type_name, "int");
        break;
    case bdmf_attr_dyn_enum:     /**< dynamic enumeration with list of values generated by callback */
        strcpy(type_name, "int");
        break;
    case bdmf_attr_aggregate:    /**< aggregate type: "structure" consisting of multiple attributes */
        if (!attr->aggr_type->struct_name)
        {
            bdmf_session_print(session, "CODEGEN: can't generate value type for aggregate type %s, attribute %s\n",
                    attr->ts.aggr_type_name, attr->name);
            return NULL;
        }
        if (is_read)
            snprintf(type_name, sizeof(type_name), "%s *", attr->aggr_type->struct_name);
        else
            snprintf(type_name, sizeof(type_name), "const %s *", attr->aggr_type->struct_name);
        break;
    default:
        bdmf_session_print(session, "CODEGEN: can't generate value type for attribute %s\n", attr->name);
        return NULL;
    }
    return type_name;
}

static const char *_bdmf_mon_attr_index_type(bdmf_session_handle session, const struct bdmf_attr *attr, bdmf_func_type func_type)
{
    int is_read = (func_type==func_add || func_type==func_find || func_type==func_get_next);
    int is_can_modify = is_read || func_type==func_read || func_type==func_write;
    static char type_name[64];

    switch(attr->index_type)
    {
    case bdmf_attr_number:       /**< Numeric attribute */
        strcpy(type_name, is_read ? "bdmf_index *" : "bdmf_index");
        break;
    case bdmf_attr_string:       /**< 0-terminated string */
        strcpy(type_name, is_read ? "char *" : "const char *");
        break;
    case bdmf_attr_buffer:       /**< Buffer with binary data */
        strcpy(type_name, is_read ? "void *" : "const void *");
        break;
    case bdmf_attr_pointer:      /**< A pointer */
        strcpy(type_name, is_read ? "void **" : "void *");
        break;
    case bdmf_attr_object:      /**< Object reference */
        strcpy(type_name, is_read ? "bdmf_object_handle *" : "bdmf_object_handle");
        break;
    case bdmf_attr_ether_addr:   /**< 6-byte Ethernet h/w address */
        strcpy(type_name, is_read ? "bdmf_mac_t *" : "const bdmf_mac_t *");
        break;
    case bdmf_attr_ip_addr:      /**< 4-byte IPv4 address or 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ip_t *" : "const bdmf_ip_t *");
        break;
    case bdmf_attr_ipv4_addr:    /**< 4-byte IPv4 address */
        strcpy(type_name, is_read ? "bdmf_ipv4 *" : "bdmf_ipv4");
        break;
    case bdmf_attr_ipv6_addr:    /**< 16-byte IPv6 address */
        strcpy(type_name, is_read ? "bdmf_ipv6_t *" : "const bdmf_ipv6_t *");
        break;
    case bdmf_attr_enum:         /**< enumeration with list of values in static table */
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            snprintf(type_name, sizeof(type_name), "%s%s",
                attr->index_ts.enum_table->type_name, is_read ? " *" : "");
        }
        else
            strcpy(type_name, is_read ? "bdmf_index *" : "bdmf_index");
        break;
    case bdmf_attr_aggregate:    /**< aggregate type: "structure" consisting of multiple attributes */
        if (!attr->index_aggr_type || !attr->index_aggr_type->struct_name)
        {
            bdmf_session_print(session, "CODEGEN: can't generate index type for attribute %s\n", attr->name);
            return NULL;
        }
        if (is_can_modify)
            snprintf(type_name, sizeof(type_name), "%s *", attr->index_aggr_type->struct_name);
        else
            snprintf(type_name, sizeof(type_name), "const %s *", attr->index_aggr_type->struct_name);
        break;
    default:
        bdmf_session_print(session, "CODEGEN: can't generate index type for attribute %s\n", attr->name);
        return NULL;
    }
    return type_name;
}

/* generate get/set attr functions */
static int _bdmf_mon_codegen_gen_attr_gen_access(codegen_parm_t *parm)
{
    const struct bdmf_type *drv=parm->drv;
    const char *name_prefix=parm->name_prefix;
    const char *keytype=parm->keytype;
    const char *keyname=parm->keyname;
    FILE *hf=parm->hf;

    if (!parm->drv->aattr)
        return 0;

    if (strlen(keyname))
    {
        /* has key */
        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr by key as number\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number *val)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);


        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr by key as string\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    char *buffer,\n"
                    "    uint32_t size)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_string(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr by key as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return >=0-number of bytes copied, or error code < 0\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    void *buffer,\n"
                    "    uint32_t size)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr by key as number\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number val)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr by key as string\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const char *buffer)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_string(mo, id, index, buffer);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr by key as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     %s Object key\n", keyname);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return >=0-number of bytes copied, or error code < 0\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s %s,\n"
                    "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const void *buffer,\n"
                    "    uint32_t size)\n",
                    keytype, keyname, name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get(%s);\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name, keyname);
    }
    else
    {
        /* no key */
        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr as number\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number *val)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr as string\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    char *buffer,\n"
                    "    uint32_t size)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_string(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Get %s object's attr as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[out]    buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_get_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    void *buffer,\n"
                    "    uint32_t size)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_get_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr as number\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     val Attribute value in numeric format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_num(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    bdmf_number val)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_num(mo, id, index, val);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr as string\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_string(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const char *buffer)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_string(mo, id, index, buffer);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);

        fprintf(hf, "\n");
        fprintf(hf, "/** Set %s object's attr as buffer\n", drv->name);
        fprintf(hf, " * \\param[in]     id Attribute id\n");
        fprintf(hf, " * \\param[in]     index Attribute array index. Ignored for scalar attributes\n");
        fprintf(hf, " * \\param[in]     buffer Attribute value in string format\n");
        fprintf(hf, " * \\param[in]     size buffer size\n");
        fprintf(hf, " * \\return 0 or error code\n");
        fprintf(hf, " */\n");
        fprintf(hf, "static inline int %s_%s_attr_set_as_buf(\n", name_prefix, drv->name);
        fprintf(hf, "    %s_%s_attr_types id,\n"
                    "    uint32_t index,\n"
                    "    const void *buffer,\n"
                    "    uint32_t size)\n",
                    name_prefix, drv->name);
        fprintf(hf, "{\n"
                    "    bdmf_object_handle mo = %s_%s_get();\n"
                    "    int rc;\n"
                    "    if (!mo) return BDMF_ERR_NOENT;\n"
                    "    rc = bdmf_attrelem_set_as_buf(mo, id, index, buffer, size);\n"
                    "    bdmf_put(mo);\n"
                    "    return rc;\n"
                    "}\n\n",
                    name_prefix, drv->name);
    }

    return 0;
}

struct func_desc *_bdmf_mon_codegen_get_fdesc(const char *fname)
{
    struct func_desc *fdesc;
    STAILQ_FOREACH(fdesc, &func_desc_list, list)
    {
        if (!strcmp(fdesc->name, fname))
            break;
    }
    return fdesc;
}

static int _bdmf_mon_codegen_is_numeric(const struct bdmf_attr *attr)
{
    int numeric;
    switch(attr->type)
    {
    case bdmf_attr_number:       /**< Numeric attribute */
    case bdmf_attr_object:       /**< Object reference */
    case bdmf_attr_pointer:      /**< A pointer */
    case bdmf_attr_ipv4_addr:    /**< 4-byte IPv4 address */
    case bdmf_attr_boolean:      /**< boolean. default(first) value = true (1) */
    case bdmf_attr_enum:         /**< enumeration with list of values in static table */
    case bdmf_attr_enum_mask:    /**< enumeration mask */
    case bdmf_attr_dyn_enum:     /**< dynamic enumeration with list of values generated by callback */
        numeric=1;
        break;
    default:
        numeric=0;
        break;
    }
    return numeric;
}

#define CODEGEN_FUNC_SUFFIX_SIZE 32

static void _bdmf_mon_codegen_attr_suffix(const struct bdmf_attr *attr, bdmf_func_type func_type, char *comment, char *suffix)
{
    switch(func_type)
    {
    case func_read:
        strncpy(comment, "Get", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_get", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_write:
    {
        int no_value = (attr->flags & BDMF_ATTR_NO_VALUE) != 0;
        if (no_value)
        {
            strncpy(comment, "Invoke", CODEGEN_FUNC_SUFFIX_SIZE);
            *suffix = 0;
        }
        else
        {
            strncpy(comment, "Set", CODEGEN_FUNC_SUFFIX_SIZE);
            strncpy(suffix, "_set", CODEGEN_FUNC_SUFFIX_SIZE);
        }
        break;
    }
    case func_add:
        strncpy(comment, "Add", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_add", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_get_next:
        strncpy(comment, "Get next", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_get_next", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_find:
        strncpy(comment, "Find", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_find", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    case func_delete:
        strncpy(comment, "Delete", CODEGEN_FUNC_SUFFIX_SIZE);
        strncpy(suffix, "_delete", CODEGEN_FUNC_SUFFIX_SIZE);
        break;
    }
}

/* generate attr read access function */
static int _bdmf_mon_codegen_gen_attr1(codegen_parm_t *parm, const struct bdmf_attr *attr, bdmf_func_type func_type)
{
    const char *attr_val_type;
    const char *attr_index_type;
    char func_name[64];
    int is_read = (func_type==func_read || func_type==func_find);
    struct func_desc *fdesc = NULL;
    char func_suffix_comment[CODEGEN_FUNC_SUFFIX_SIZE], func_suffix[CODEGEN_FUNC_SUFFIX_SIZE];
    int no_value = (func_type == func_write) && (attr->flags & BDMF_ATTR_NO_VALUE) != 0;

    attr_val_type = _bdmf_mon_attr_val_type(parm->session, attr, is_read);
    if (!attr_val_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    attr_index_type = _bdmf_mon_attr_index_type(parm->session, attr, func_type);
    if (!attr_index_type)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't generate attribute access function for %s/%s\n", parm->drv->name, attr->name);
        return 0;
    }
    _bdmf_mon_codegen_attr_suffix(attr, func_type, func_suffix_comment, func_suffix);

    /* "delete" function */
    if (func_type == func_delete)
    {
        fprintf(parm->hf, "\n");

        snprintf(func_name, sizeof(func_name), "%s_%s_%s_delete",
            parm->name_prefix, parm->drv->name, attr->name);
        fdesc = _bdmf_mon_codegen_get_fdesc(func_name);
        if (fdesc && fdesc->is_tagged)
        {
            /* Replace description */
            fprintf(parm->hf, "%s", fdesc->desc);
        }
        else
        {
            fprintf(parm->hf, "/** Delete %s/%s attribute entry.\n",
                parm->drv->name, attr->name);
            fprintf(parm->hf, " *\n");
            fprintf(parm->hf, " * Delete %s.\n", attr->help);
            if (fdesc)
                fprintf(parm->hf, "%s", fdesc->desc);
            if ((attr->flags & BDMF_ATTR_DEPRECATED))
            {
                if (attr->deprecated_text)
                    fprintf(parm->hf, "\n * This function has been deprecated, %s.\n *\n",attr->deprecated_text);
                else
                    fprintf(parm->hf, "\n * This function has been deprecated.\n *\n");
            }
            fprintf(parm->hf, " * \\param[in]   mo_ %s object handle\n", parm->drv->name);
            fprintf(parm->hf, " * \\param[in]   ai_ Attribute array index\n");
            fprintf(parm->hf, " * \\return 0 or error code < 0\n");
            if ((attr->flags & BDMF_ATTR_NOLOCK))
                fprintf(parm->hf, " * The function can be called in task and softirq contexts.\n");
            else
                fprintf(parm->hf, " * The function can be called in task context only.\n");
            fprintf(parm->hf, " */\n");
        }

        fprintf(parm->hf, "static inline int %s(", func_name);
        fprintf(parm->hf, "bdmf_object_handle mo_, %s ai_)\n", attr_index_type);
        fprintf(parm->hf, "{\n");
        fprintf(parm->hf, "    return bdmf_attrelem_delete(mo_, %s, (bdmf_index)ai_);\n", parm->attr_id);
        fprintf(parm->hf, "}\n\n");
        return 0;
    }

    fprintf(parm->hf, "\n");
    snprintf(func_name, sizeof(func_name), "%s_%s_%s%s",
        parm->name_prefix, parm->drv->name, attr->name, func_suffix);
    fdesc = _bdmf_mon_codegen_get_fdesc(func_name);

    if (fdesc && fdesc->is_tagged)
    {
        /* Replace description */
        fprintf(parm->hf, "%s", fdesc->desc);
    }
    else
    {
        /* Auto-generate description */
        fprintf(parm->hf, "/** %s %s/%s attribute%s\n",
            func_suffix_comment,
            parm->drv->name, attr->name, (attr->array_size > 1) ? " entry." : ".");
        fprintf(parm->hf, " *\n");
        fprintf(parm->hf, " * %s %s.\n", func_suffix_comment, attr->help);
        if (fdesc)
            fprintf(parm->hf, "%s", fdesc->desc);
        if ((attr->flags & BDMF_ATTR_DEPRECATED))
        {
            if (attr->deprecated_text)
                fprintf(parm->hf, "\n * This function has been deprecated, %s.\n *\n",attr->deprecated_text);
            else
                fprintf(parm->hf, "\n * This function has been deprecated.\n *\n");
        }
        fprintf(parm->hf, " * \\param[in]   mo_ %s object handle or mattr transaction handle\n",
            parm->drv->name);
        if ((attr->array_size > 1) || (func_type==func_add))
            fprintf(parm->hf, " * \\param[%s]   ai_ Attribute array index\n",
                (func_type==func_add || func_type==func_get_next || func_type==func_find)?"in,out":"in");
        if (func_type==func_read)
            fprintf(parm->hf, " * \\param[out]  %s_ Attribute value\n", attr->name);
        else if (func_type == func_find)
            fprintf(parm->hf, " * \\param[in,out]   %s_ Attribute value\n", attr->name);
        else if ((func_type != func_get_next) && !no_value)
            fprintf(parm->hf, " * \\param[in]   %s_ Attribute value\n", attr->name);
        if ((func_type != func_get_next && func_type != func_find) &&
            (attr->type==bdmf_attr_buffer || (func_type==func_read && attr->type==bdmf_attr_string)))
        {
            fprintf(parm->hf, " * \\param[in]   size_ buffer size\n");
            if (func_type==func_read)
                fprintf(parm->hf, " * \\return number of bytes read >=0 or error code < 0\n");
            else
                fprintf(parm->hf, " * \\return number of bytes written >=0 or error code < 0\n");
        }
        else
            fprintf(parm->hf, " * \\return 0 or error code < 0\n");
        if ((attr->flags & BDMF_ATTR_NOLOCK))
            fprintf(parm->hf, " * The function can be called in task and softirq contexts.\n");
        else
            fprintf(parm->hf, " * The function can be called in task context only.\n");

        fprintf(parm->hf, " */\n");
    }
    fprintf(parm->hf, "static inline int %s(", func_name);
    fprintf(parm->hf, "bdmf_object_handle mo_");
    if (attr->array_size || (func_type==func_add))
        fprintf(parm->hf, ", %s ai_", attr_index_type);
    if ((func_type != func_get_next) && !(func_type == func_write && no_value))
    {
        fprintf(parm->hf, ", %s %s%s_", attr_val_type,
            (is_read && _bdmf_mon_codegen_is_numeric(attr)) ? "*" :"", attr->name);
    }
    if ((func_type != func_get_next && func_type != func_find) &&
         (attr->type==bdmf_attr_buffer || (func_type==func_read && attr->type==bdmf_attr_string)))
        fprintf(parm->hf, ", uint32_t size_)\n");
    else
        fprintf(parm->hf, ")\n");
    fprintf(parm->hf, "{\n");
    if (_bdmf_mon_codegen_is_numeric(attr) && func_type==func_read)
    {
        fprintf(parm->hf, "    bdmf_number _nn_;\n");
        fprintf(parm->hf, "    int _rc_;\n");
    }
    if (func_type==func_read)
    {
        if (attr->type == bdmf_attr_string)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_get_as_string(mo_, %s, (bdmf_index)ai_, %s_, size_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_get_as_string(mo_, %s, %s_, size_);\n",
                        parm->attr_id, attr->name);
        }
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    _rc_ = bdmf_attrelem_get_as_num(mo_, %s, (bdmf_index)ai_, &_nn_);\n",
                        parm->attr_id);
            else
                fprintf(parm->hf, "    _rc_ = bdmf_attr_get_as_num(mo_, %s, &_nn_);\n",
                        parm->attr_id);
            if (attr->type == bdmf_attr_object)
                fprintf(parm->hf, "    *%s_ = (bdmf_object_handle)(long)_nn_;\n", attr->name);
            else if (attr->type == bdmf_attr_pointer)
                fprintf(parm->hf, "    *%s_ = (void *)(long)_nn_;\n", attr->name);
            else
                fprintf(parm->hf, "    *%s_ = (%s)_nn_;\n", attr->name, attr_val_type);
            fprintf(parm->hf, "    return _rc_;\n");
        }
        else if (attr->type == bdmf_attr_buffer)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_get_as_buf(mo_, %s, (bdmf_index)ai_, %s_, size_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_get_as_buf(mo_, %s, %s_, size_);\n",
                        parm->attr_id, attr->name);
        }
        else
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_get_as_buf(mo_, %s, (bdmf_index)ai_, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_get_as_buf(mo_, %s, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
        }
    }
    else if (func_type==func_write)
    {
        if (attr->type == bdmf_attr_string)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_set_as_string(mo_, %s, (bdmf_index)ai_, %s_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_set_as_string(mo_, %s, %s_);\n",
                        parm->attr_id, attr->name);
        }
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if ((attr->type == bdmf_attr_object) || (attr->type == bdmf_attr_pointer))
            {
                if (attr->array_size > 1)
                    fprintf(parm->hf, "    return bdmf_attrelem_set_as_num(mo_, %s, (bdmf_index)ai_, (long)%s_);\n",
                            parm->attr_id, attr->name);
                else
                    fprintf(parm->hf, "    return bdmf_attr_set_as_num(mo_, %s, (long)%s_);\n",
                            parm->attr_id, attr->name);
            }
            else
            {
                char value_name[64];
                if (no_value)
                    strcpy(value_name, "1");
                else
                    snprintf(value_name, sizeof(value_name), "%s_", attr->name);
                if (attr->array_size > 1)
                    fprintf(parm->hf, "    return bdmf_attrelem_set_as_num(mo_, %s, (bdmf_index)ai_, %s);\n",
                            parm->attr_id, value_name);
                else
                    fprintf(parm->hf, "    return bdmf_attr_set_as_num(mo_, %s, %s);\n",
                            parm->attr_id, value_name);
            }
        }
        else if (attr->type == bdmf_attr_buffer)
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_set_as_buf(mo_, %s, (bdmf_index)ai_, %s_, size_);\n",
                        parm->attr_id, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_set_as_buf(mo_, %s, %s_, size_);\n",
                        parm->attr_id, attr->name);
        }
        else
        {
            if (attr->array_size > 1)
                fprintf(parm->hf, "    return bdmf_attrelem_set_as_buf(mo_, %s, (bdmf_index)ai_, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
            else
                fprintf(parm->hf, "    return bdmf_attr_set_as_buf(mo_, %s, %s_, sizeof(*%s_));\n",
                        parm->attr_id, attr->name, attr->name);
        }
    }
    else if (func_type==func_add)
    {
        char index_name[16]= "ai_";
        fprintf(parm->hf, "    int rc;\n");
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            fprintf(parm->hf, "    bdmf_index _ai_tmp_ = ai_ ? (bdmf_index)(*ai_) : BDMF_INDEX_UNASSIGNED;\n");
            strcpy(index_name, "&_ai_tmp_");
        }
        if (attr->type == bdmf_attr_string)
        {
            fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_string(mo_, %s, (bdmf_index *)%s, %s_);\n",
                    parm->attr_id, index_name, attr->name);
        }
        else if (_bdmf_mon_codegen_is_numeric(attr))
        {
            if ((attr->type == bdmf_attr_object) || (attr->type == bdmf_attr_pointer))
            {
                fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_num(mo_, %s, (bdmf_index *)ai_, (long)%s_);\n",
                        parm->attr_id, attr->name);
            }
            else
            {
                fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_num(mo_, %s, (bdmf_index *)%s, %s_);\n",
                        parm->attr_id, index_name, attr->name);
            }
        }
        else if (attr->type == bdmf_attr_buffer)
        {
            fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_buf(mo_, %s, (bdmf_index *)%s, %s_, size_);\n",
                    parm->attr_id, index_name, attr->name);
        }
        else
        {
            fprintf(parm->hf, "    rc = bdmf_attrelem_add_as_buf(mo_, %s, (bdmf_index *)%s, %s_, sizeof(*%s_));\n",
                    parm->attr_id, index_name, attr->name, attr->name);
        }
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
            fprintf(parm->hf, "    *ai_ = (%s)_ai_tmp_;\n", attr->index_ts.enum_table->type_name);
        fprintf(parm->hf, "    return rc;\n");
    }
    else if (func_type==func_get_next)
    {
        char index_name[16]= "ai_";
        int need_rc = 0;
        if (attr->index_type==bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            fprintf(parm->hf, "    int rc;\n");
            fprintf(parm->hf, "    bdmf_index _ai_tmp_ = *ai_;\n");
            strcpy(index_name, "&_ai_tmp_");
            need_rc = 1;
        }
        fprintf(parm->hf, "    %s bdmf_attrelem_get_next(mo_, %s, (bdmf_index *)%s);\n",
                need_rc ? "rc =" : "return", parm->attr_id, index_name);
        if (need_rc)
        {
            fprintf(parm->hf, "    *ai_ = (%s)_ai_tmp_;\n", attr->index_ts.enum_table->type_name);
            fprintf(parm->hf, "    return rc;\n");
        }
    }
    else if (func_type==func_find)
    {
        char index_name[16]= "ai_";
        fprintf(parm->hf, "    int rc;\n");
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
        {
            fprintf(parm->hf, "    bdmf_index _ai_tmp_ = *ai_;\n");
            strcpy(index_name, "&_ai_tmp_");
        }
        fprintf(parm->hf, "    rc = bdmf_attrelem_find(mo_, %s, (bdmf_index *)%s, %s_, sizeof(*%s_));\n",
                parm->attr_id, index_name, attr->name, attr->name);
        if (attr->index_type == bdmf_attr_enum && attr->index_ts.enum_table && attr->index_ts.enum_table->type_name)
            fprintf(parm->hf, "    *ai_ = (%s)_ai_tmp_;\n", attr->index_ts.enum_table->type_name);
        fprintf(parm->hf, "    return rc;\n");
    }
    fprintf(parm->hf, "}\n\n");

    return 0;
}

/* generate get/set attr functions */
static int _bdmf_mon_codegen_gen_attr_access(codegen_parm_t *parm)
{
    const struct bdmf_attr *attr=parm->drv->aattr;
    int rc=0;
    while(attr && attr->name)
    {
        if ((attr->flags & parm->attr_level)!=0 &&
            !(attr->flags & BDMF_ATTR_NO_AUTO_GEN) &&
            ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ALL)!=0 ||
             (attr->type==bdmf_attr_aggregate)) )
        {
            snprintf(parm->attr_id, sizeof(parm->attr_id), "%s_%s_attr_%s",
                    parm->name_prefix, parm->drv->name, attr->name);
            if ((attr->flags & BDMF_ATTR_READ))
                rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_read);
            if ((attr->flags & BDMF_ATTR_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
            {
                if ((attr->flags & BDMF_ATTR_UDEF_WRITE) || (attr->flags & BDMF_ATTR_WRITE_INIT))
                    rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_write);
                if (attr->add)
                    rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_add);
                if (attr->del)
                    rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_delete);
            }
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_GET_NEXT))
                rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_get_next);
            if (attr->array_size > 1 && (attr->flags & BDMF_ATTR_UDEF_FIND))
                rc = rc ? rc : _bdmf_mon_codegen_gen_attr1(parm, attr, func_find);
        }
        ++attr;
    }

    return rc;
}

/* Generate "object_key structure */
static int _bdmf_mon_codegen_gen_key(codegen_parm_t *parm)
{
    const struct bdmf_type *d = parm->drv;
    const struct bdmf_type *drv_stack[BDMF_MAX_TYPE_STACK_DEPTH];
    FILE *hf = parm->hf;
    const struct bdmf_attr *a;
    int stack_depth=0;
    int nkeys=0;

    /* Calculate driver stack */
    while(d)
    {
        if (stack_depth >= BDMF_MAX_TYPE_STACK_DEPTH)
        {
            bdmf_session_print(parm->session, "CODEGEN: parent type stack overflow for %s\n", parm->drv->name);
            return 0;
        }
        drv_stack[stack_depth++] = d;

        /* calculate total number of keys */
        a = d->aattr;
        while(a && a->name)
        {
            if ((a->flags & BDMF_ATTR_KEY))
                ++nkeys;
            ++a;
        }

        d = d->po;
    }

    /* Generate key structure */
    if (nkeys)
    {
        int i=stack_depth;
        if (nkeys > 1)
        {
            snprintf(parm->keytype, sizeof(parm->keytype), "const %s_%s_key_t *", parm->name_prefix, parm->drv->name);
            snprintf(parm->keyname, sizeof(parm->keyname), "key_");
            if (!parm->gpl_shim)
            {
                fprintf(hf, "\n/** %s object key. */\n", parm->drv->name);
                fprintf(hf, "typedef struct {\n");
                while(--i >= 0)
                {
                    d = drv_stack[i];
                    /* generate structure field for each key field */
                    a = d->aattr;
                    while(a && a->name)
                    {
                        if ((a->flags & BDMF_ATTR_KEY))
                        {
                            fprintf(hf, "    %s %s; /**< %s: %s */\n",
                                    _bdmf_mon_attr_val_type(parm->session, a, 0), a->name,
                                    d->name, a->help?a->help:"");
                        }
                        ++a;
                    }
                };
                fprintf(hf, "} %s_%s_key_t;\n\n", parm->name_prefix, parm->drv->name);
            }
        }
        else
        {
            while(--i >= 0)
            {
                d = drv_stack[i];
                /* generate structure field for each key field */
                a = d->aattr;
                while(a && a->name)
                {
                    if ((a->flags & BDMF_ATTR_KEY))
                    {
                        snprintf(parm->keytype, sizeof(parm->keytype), "%s",
                                _bdmf_mon_attr_val_type(parm->session, a, 0));
                        snprintf(parm->keyname, sizeof(parm->keyname), "%s_", a->name);
                        break;
                    }
                    ++a;
                }
            };
        }
    }

    /* Generate "get object" function */
    fprintf(hf, "\n");
    if (nkeys)
    {
        fprintf(hf, "%sint (*f_%s_%s_get)(%s %s, bdmf_object_handle *pmo);\n",
            parm->gpl_shim ? "" : "extern ",
            parm->name_prefix, parm->drv->name, parm->keytype, parm->keyname);
    }
    else
    {
        fprintf(hf, "%sint (*f_%s_%s_get)(bdmf_object_handle *pmo);\n",
            parm->gpl_shim ? "" : "extern ",
            parm->name_prefix, parm->drv->name);
    }
    if (parm->gpl_shim)
        fprintf(hf, "EXPORT_SYMBOL(f_%s_%s_get);\n", parm->name_prefix, parm->drv->name);

    fprintf(hf, "\n");
    fprintf(hf, "/** Get %s object%s", parm->drv->name, nkeys ? " by key." : ".");
    fprintf(hf, "\n\n");
    fprintf(hf, " * This function returns %s object instance%s",
        parm->drv->name, nkeys ? " by key." : ".");
    fprintf(hf, "\n");
    if (nkeys)
        fprintf(hf, " * \\param[in] %s    Object key\n", parm->keyname);
    fprintf(hf, " * \\param[out] %s_obj    Object handle\n", parm->drv->name);
    fprintf(hf, " * \\return    0=OK or error <0\n");
    fprintf(hf, " */\n");
    fprintf(hf, "int %s_%s_get(", parm->name_prefix, parm->drv->name);
    if (nkeys)
        fprintf(hf, "%s %s, ", parm->keytype, parm->keyname);
    fprintf(hf, "bdmf_object_handle *%s_obj)", parm->drv->name);
    if (parm->gpl_shim)
    {
        fprintf(hf, "\n{\n"
                    "   if (!f_%s_%s_get)\n"
                    "       return BDMF_ERR_STATE;\n"
                    "   return f_%s_%s_get(",
                    parm->name_prefix, parm->drv->name, parm->name_prefix, parm->drv->name);
        if (nkeys)
            fprintf(hf, "%s, ", parm->keyname);
        fprintf(hf, "%s_obj);\n", parm->drv->name);
        fprintf(hf, "}\n");
        fprintf(hf, "EXPORT_SYMBOL(%s_%s_get);\n", parm->name_prefix, parm->drv->name);
    }
    else
        fprintf(hf, ";\n");

    return 0;
}

/* Get "a" / "an" article */
static const char *_bdmf_codegen_get_article(const char *name)
{
    static char *a_article = "a";
    static char *an_article = "an";

    if (strchr("aeuio", name[0]))
        return an_article;
    return a_article;
}

/* head comments for xx_drv() function */
static void _bdmf_mon_xx_drv_header(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    const struct bdmf_type *drv = parm->drv;
    const char *article = _bdmf_codegen_get_article(drv->name);

    fprintf(hf, "\n"
                "/** Get %s type handle.\n"
                " *\n"
                " * This handle should be passed to bdmf_new_and_set() function in\n"
                " * order to create %s %s object.\n"
                " * \\return %s type handle\n"
                " */\n", drv->name, article, drv->name, drv->name);
}


/* Generate object-access header file for a single object type
 */
static int _bdmf_mon_codegen_gen1_header(codegen_parm_t *parm)
{
    const struct bdmf_attr *attr=parm->drv->aattr;
    FILE *hf = parm->hf;
    const struct bdmf_type *drv = parm->drv;
    char attrhlp[128];
    int rc = 0;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* Doxygen description */
    fprintf(hf, "/** \\addtogroup %s\n", drv->name);
    fprintf(hf, " * @{\n */\n\n");

    /* Generate "xx_drv_get" function */
    _bdmf_mon_xx_drv_header(parm);
    fprintf(hf, "bdmf_type_handle %s_%s_drv(void);\n", parm->name_prefix, drv->name);

    /* Generate attribute enums */
    if ((parm->action_mask & (BDMF_ACTION_MASK_ATTR_ENUM | BDMF_ACTION_MASK_ATTR_ACCESS)) && attr)
    {
        char *doxygen_hdr_comment = (parm->action_mask & BDMF_ACTION_MASK_ATTR_ENUM) ? "*" : "";
        char *doxygen_body_comment = (parm->action_mask & BDMF_ACTION_MASK_ATTR_ENUM) ? "*<" : "";

        fprintf(hf, "\n/*%s %s: Attribute types */\n", doxygen_hdr_comment, drv->name);
        fprintf(hf, "typedef enum {\n");
        while(attr && attr->name)
        {
            if ((attr->flags & parm->attr_level) && !(attr->flags & BDMF_ATTR_NO_AUTO_GEN))
            {
                bdmf_attr_help_compact(attr, attrhlp, sizeof(attrhlp));
                fprintf(hf, "    %s_%s_attr_%s = %d, /*%s %s */\n",
                    parm->name_prefix, drv->name,
                    attr->name, (int)(attr-drv->aattr),
                    doxygen_body_comment, attrhlp);
            }
            ++attr;
        }
        fprintf(hf, "} %s_%s_attr_types;\n", parm->name_prefix, drv->name);
    }

    /* Generate key and attribute access by key */
    if ((parm->action_mask & BDMF_ACTION_MASK_KEY))
        rc = _bdmf_mon_codegen_gen_key(parm);

    /* Generate specific attribute access functions */
    if ((parm->action_mask & BDMF_ACTION_MASK_ATTR_ACCESS))
        rc = rc ? rc : _bdmf_mon_codegen_gen_attr_access(parm);

    /* Generate generic attribute access functions */
    if ((parm->action_mask & BDMF_ACTION_MASK_ATTR_GEN_ACCESS))
        rc = rc ? rc : _bdmf_mon_codegen_gen_attr_gen_access(parm);

    fprintf(hf, "/** @} end of %s Doxygen group */\n\n\n", drv->name);

    return rc;
}

/* Generate object-access gpl shim file for a single object type
 */
static int _bdmf_mon_codegen_gen1_gpl_shim(codegen_parm_t *parm)
{
    FILE *hf = parm->hf;
    const struct bdmf_type *drv = parm->drv;
    int rc = 0;

    /* do nothing for root */
    if (!parm->drv->po)
        return 0;

    /* Generate "xx_type_get" function */
    fprintf(hf, "\nbdmf_type_handle (*f_%s_%s_drv)(void);\n", parm->name_prefix, drv->name);
    fprintf(hf, "\nEXPORT_SYMBOL(f_%s_%s_drv);\n", parm->name_prefix, drv->name);
    _bdmf_mon_xx_drv_header(parm);
    fprintf(hf, "bdmf_type_handle %s_%s_drv(void)\n"
                "{\n"
                "   if (!f_%s_%s_drv)\n"
                "       return NULL;\n"
                "   return f_%s_%s_drv();\n"
                "}\n",
                parm->name_prefix, drv->name, parm->name_prefix, drv->name,
                parm->name_prefix, drv->name);
    fprintf(hf, "\nEXPORT_SYMBOL(%s_%s_drv);\n", parm->name_prefix, drv->name);

    /* Generate key and attribute access by key */
    if ((parm->action_mask & BDMF_ACTION_MASK_KEY))
        rc = _bdmf_mon_codegen_gen_key(parm);

    return rc;
}

/* Generate object-access header/gpl shim file for object type tree
 * starting from drv's children
 */
static int _bdmf_mon_codegen_gen_tree(codegen_parm_t *parm, struct bdmf_type *drv,
    int (*gen1)(codegen_parm_t *parm))
{
    int rc = 0;
    struct bdmf_type *d=(struct bdmf_type *)drv;
    parm->drv = drv;
    rc = gen1(parm);
    bdmf_type_get(parm->drv); /* will be reduced by "get_next" */
    while((d=bdmf_type_get_next(d)))
    {
        if (d->po == drv)
        {
            parm->drv = d;
            rc = rc ? rc : gen1(parm);
        }
    }
    parm->drv = drv;
    return rc;
}

static int bdmf_mon_codegen_standard_gpl_comments(codegen_parm_t *parm)
{
    static char *gpl_string =
        "// %scopyright-BRCM:2013:DUAL/GPL:standard\n"
        "// :>\n";
    int rc;

    rc = fprintf(parm->hf, gpl_string, "<:");
    return (rc > 0) ? 0 : BDMF_ERR_IO;
}

static int bdmf_mon_codegen_gen_file(codegen_parm_t *parm, const char *fname, int hierarchical)
{
    char ch_filename[256];
    char *pdot;
    char hdr_def[64]="";
    char *phdr_def=hdr_def;
    int rc;

    /* Generate .h or .c file name */
    strncpy(ch_filename, fname, sizeof(ch_filename));
    pdot = strchr(ch_filename, '.');
    while(pdot && *(pdot+1)!= 'h' && *(pdot+1)!= 'c')
        pdot = strchr(pdot+1, '.');
    if (pdot)
        *pdot = 0;
    strncat(ch_filename, parm->gpl_shim ? ".c" : ".h", sizeof(ch_filename));

    parm->hf = fopen(ch_filename, "w");
    if (!parm->hf)
    {
        bdmf_session_print(parm->session, "CODEGEN: can't open file %s for writing\n", ch_filename);
        return BDMF_ERR_PARM;
    }
    bdmf_session_print(parm->session, "CODEGEN: generating %s\n", ch_filename);

    /* Generate head comments */
    rc = bdmf_mon_codegen_standard_gpl_comments(parm);
    if (rc)
    {
        fclose(parm->hf);
        return rc;
    }

    /* Generate file-specific comments */

    if (parm->gpl_shim)
    {
        fprintf(parm->hf, "/*\n"
                    " * %s object GPL shim file.\n"
                    " * This file is generated automatically. Do not edit!\n"
                    " */\n", parm->drv->name);
        if (hierarchical)
            rc = _bdmf_mon_codegen_gen_tree(parm, parm->drv, _bdmf_mon_codegen_gen1_gpl_shim);
        else
            rc = _bdmf_mon_codegen_gen1_gpl_shim(parm);
        fprintf(parm->hf, "\nMODULE_LICENSE(\"GPL\");\n");
    }
    else
    {
        /* generate file header */
        snprintf(hdr_def, sizeof(hdr_def), "_%s_AG_%s_H_", parm->name_prefix,
            parm->drv->name);
        while(*phdr_def)
        {
            *phdr_def = toupper(*phdr_def);
            ++phdr_def;
        }
        fprintf(parm->hf, "/*\n"
                    " * %s object header file.\n"
                    " * This header file is generated automatically. Do not edit!\n"
                    " */\n", parm->drv->name);
        fprintf(parm->hf, "#ifndef %s\n"
                    "#define %s\n\n", hdr_def, hdr_def);
        if (hierarchical)
            rc = _bdmf_mon_codegen_gen_tree(parm, parm->drv, _bdmf_mon_codegen_gen1_header);
        else
            rc = _bdmf_mon_codegen_gen1_header(parm);
        fprintf(parm->hf, "\n\n#endif /* %s */\n", hdr_def);
    }


    fclose(parm->hf);

    return rc;
}

static int fdesc_line;

/* Find and read function name.
 * Returns function name in dynamically-allocated memory
 */
static char *_bdmf_codegen_get_fname(bdmf_session_handle session, FILE *hf)
{
    char buf[256];
    char *fname;
    char *pbuf;
    char *pend;

    while((pbuf=fgets(buf, sizeof(buf), hf)))
    {
        ++fdesc_line;
        if (!memcmp(buf, CODEGEN_FSTART_TAG, sizeof(CODEGEN_FSTART_TAG)-1))
            break;
    }
    if (!pbuf)
        return NULL;
    pbuf += sizeof(CODEGEN_FSTART_TAG);
    while(isspace(*pbuf) && *pbuf)
        ++pbuf;
    pend = pbuf;
    while(!isspace(*pend) && *pend)
        ++pend;
    *pend = 0;
    if (pend == pbuf)
    {
        bdmf_session_print(session, "CODEGEN: name is missing after %s in line %d\n", CODEGEN_FSTART_TAG, fdesc_line);
        return NULL;
    }
    fname = bdmf_alloc(strlen(pbuf)+1);
    if (!fname)
        return NULL;
    strcpy(fname, pbuf);
    return fname;
}

/* function description
 * Returns function description in dynamically-allocated memory
 */
static char *_bdmf_codegen_get_fdesc(bdmf_session_handle session, FILE *hf)
{
    char buf[256];
    char *pbuf;
    char *fdesc = NULL;

    while((pbuf=fgets(buf, sizeof(buf), hf)))
    {
        char *fdesc_tmp;
        ++fdesc_line;
        if (!memcmp(buf, CODEGEN_FEND_TAG, sizeof(CODEGEN_FEND_TAG)-1))
            break;
        if (fdesc)
            fdesc_tmp = realloc(fdesc, strlen(fdesc)+strlen(buf)+2);
        else
            fdesc_tmp = bdmf_alloc(strlen(buf)+1);
        if (!fdesc_tmp)
            return NULL;
        if (!fdesc)
            *fdesc_tmp = 0;
        strcat(fdesc_tmp, buf);
        fdesc = fdesc_tmp;
    }
    if (!pbuf)
    {
        bdmf_session_print(session, "CODEGEN: unexpected end of file. %s is missing\n", CODEGEN_FEND_TAG);
        return NULL;
    }
    return fdesc;
}

/* Load extended function descriptions
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
*/
static int bdmf_mon_codegen_load(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    const char *filename = parm[0].value.string;
    char *name, *desc;
    struct func_desc *fdesc;
    FILE *hf;

    if (!STAILQ_EMPTY(&func_desc_list))
    {
        bdmf_session_print(session, "CODEGEN: already loaded\n");
        return BDMF_ERR_ALREADY;
    }

    hf = fopen(filename, "r");
    if (!hf)
    {
        bdmf_session_print(session, "CODEGEN: can't open file %s for reading\n", filename);
        return BDMF_ERR_PARM;
    }
    while((name=_bdmf_codegen_get_fname(session, hf)) &&
          (desc=_bdmf_codegen_get_fdesc(session, hf)))
    {
        fdesc = bdmf_calloc(sizeof(*fdesc));
        if (!fdesc)
            return BDMF_ERR_NOMEM;
        fdesc->name = name;
        fdesc->desc = desc;
        fdesc->is_tagged = (strstr(desc, "\\param") != NULL || strstr(desc, "\\return") != NULL);
        STAILQ_INSERT_TAIL(&func_desc_list, fdesc, list);
    }
    fclose(hf);
    bdmf_session_print(session, "CODEGEN: %d lines read from %s\n", fdesc_line, filename);

    return 0;
}

/* Generate object-access header file
    BDMFMON_MAKE_PARM("type", "Object type", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM("name_prefix", "Type/function name prefix", BDMFMON_PARM_STRING, 0),
    BDMFMON_MAKE_PARM_RANGE_DEFVAL("what_generate", "1=header,2=GPL shim,3=both", BDMFMON_PARM_NUMBER, 0, 1, 3, 3),
    BDMFMON_MAKE_PARM_DEFVAL("mask", "What to gen bitmask: 1-attr enum,2-access funcs,4-maccess helpers",
                    BDMFMON_PARM_NUMBER, 0, 7),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("level", "Attribute level", attr_level_table, 0, "minor"),
    BDMFMON_MAKE_PARM_ENUM_DEFVAL("children", "Hierarchical", bdmfmon_enum_bool_table, 0, "no"),
*/
static int bdmf_mon_codegen_generate(bdmf_session_handle session,
                               const bdmfmon_cmd_parm_t parm[],  uint16_t n_parms)
{
    const char *type = parm[0].value.string;
    const char *filename = parm[1].value.string;
    const char *name_prefix = parm[2].value.string;
    int what_generate = parm[3].value.unumber;
    uint32_t action_mask = parm[4].value.unumber;
    int level = parm[5].value.unumber;
    int children = parm[6].value.unumber;

    codegen_parm_t p;
    int rc;

    memset(&p, 0, sizeof(p));
    p.action_mask = action_mask;
    p.attr_level = level;
    p.name_prefix = name_prefix;
    p.session = session;

    if (!(action_mask & BDMF_ACTION_MASK_ALL))
    {
        bdmf_session_print(session, "CODEGEN: action mask is not set. Nothing to do\n");
        return -EINVAL;
    }
    bdmf_session_print(session, "CODEGEN: processing type %s. action=0x%x\n", type, action_mask);
    rc = bdmf_type_find_get(type, &p.drv);
    if (rc)
        return rc;

    if ((what_generate & BDMF_FILE_HEADER))
        rc = bdmf_mon_codegen_gen_file(&p, filename, children);
    if ((what_generate & BDMF_FILE_GPL_SHIM))
    {
        p.gpl_shim = 1;
        rc = rc ? rc : bdmf_mon_codegen_gen_file(&p, filename, children);
    }
    bdmf_type_put(p.drv);
    return rc;
}

bdmfmon_handle_t bdmf_codegen_mon_init(void)
{
    bdmfmon_handle_t bdmf_dir;
    bdmfmon_handle_t codegen_dir;

    STAILQ_INIT(&func_desc_list);

    bdmf_dir=bdmfmon_dir_find(NULL, "bdmf");
    codegen_dir = bdmfmon_dir_add(bdmf_dir, "codegen",
                             "Code Generator",
                             BDMF_ACCESS_ADMIN, NULL);

    /* Load extended function descriptions
     * Available only in simulation environment
     */
    {
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(codegen_dir, "load", bdmf_mon_codegen_load,
                      "Load extended function descriptions",
                      BDMF_ACCESS_GUEST, NULL, parms);
    }

    /* Object access header  file generation.
     * Available only in simulation environment
     */
    {
        static bdmfmon_enum_val_t attr_level_table[] = {
            { .name="major",  .val=BDMF_ATTR_MAJOR},          /* Major only */
            { .name="minor",  .val=BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR }, /* Major+minor */
            { .name="debug",  .val=BDMF_ATTR_MAJOR | BDMF_ATTR_MINOR | BDMF_ATTR_HIDDEN }, /* all */
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_cmd_parm_t parms[]={
            BDMFMON_MAKE_PARM("type", "Object type", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM("file", "File name", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM("name_prefix", "Type/function name prefix", BDMFMON_PARM_STRING, 0),
            BDMFMON_MAKE_PARM_RANGE_DEFVAL("what_generate", "1=header,2=GPL shim,3=both", BDMFMON_PARM_NUMBER, 0,
                BDMF_FILE_HEADER, BDMF_FILE_HEADER | BDMF_FILE_GPL_SHIM, BDMF_FILE_HEADER | BDMF_FILE_GPL_SHIM),
            BDMFMON_MAKE_PARM_DEFVAL("action_mask", "What bitmask: 1-attr enum,2-key,4-ag,8-all; 10-access,20-maccess,40-gen access",
                            BDMFMON_PARM_NUMBER, 0, BDMF_ACTION_MASK_ALL_NO_GEN_ACCESS),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("level", "Attribute level", attr_level_table, 0, "minor"),
            BDMFMON_MAKE_PARM_ENUM_DEFVAL("children", "Hierarchical", bdmfmon_enum_bool_table, 0, "no"),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        bdmfmon_cmd_add(codegen_dir, "generate", bdmf_mon_codegen_generate,
                      "Generate header file(s)",
                      BDMF_ACCESS_GUEST, NULL, parms);
    }
    return codegen_dir;
}
