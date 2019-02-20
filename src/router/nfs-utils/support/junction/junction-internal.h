/*
 * @file support/junction/junction-internal.h
 * @brief Internal declarations for libjunction.a
 */

/*
 * Copyright 2011, 2018 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * nfs-utils is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2.0 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2.0 along with nfs-utils.  If not, see:
 *
 *	http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _FEDFS_JUNCTION_INTERNAL_H_
#define _FEDFS_JUNCTION_INTERNAL_H_

#include <libxml/tree.h>
#include <libxml/xpath.h>

/**
 ** Names of extended attributes that store junction data
 **/

/**
 * Name of extended attribute containing saved mode bits
 */
#define JUNCTION_XATTR_NAME_MODE	"trusted.junction.mode"

/**
 * Name of extended attribute containing NFS-related junction data
 */
#define JUNCTION_XATTR_NAME_NFS		"trusted.junction.nfs"


/**
 ** Names of XML elements and attributes that represent junction data
 **/

/**
 * Tag name of root element of a junction XML document
 */
#define JUNCTION_XML_ROOT_TAG		(const xmlChar *)"junction"

/**
 * Tag name of fileset element of a junction XML document
 */
#define JUNCTION_XML_FILESET_TAG	(const xmlChar *)"fileset"

/**
 * Tag name of savedmode element of a junction XML document
 */
#define JUNCTION_XML_SAVEDMODE_TAG	(const xmlChar *)"savedmode"

/**
 * Name of mode bits attribute on a savedmode element
 */
#define JUNCTION_XML_MODEBITS_ATTR	(const xmlChar *)"bits"

/**
 ** Junction helper functions
 **/

FedFsStatus	 junction_open_path(const char *pathname, int *fd);
FedFsStatus	 junction_is_directory(int fd, const char *path);
FedFsStatus	 junction_is_sticky_bit_set(int fd, const char *path);
FedFsStatus	 junction_set_sticky_bit(int fd, const char *path);
FedFsStatus	 junction_is_xattr_present(int fd, const char *path,
				const char *name);
FedFsStatus	 junction_read_xattr(int fd, const char *path, const char *name,
				char **contents);
FedFsStatus	 junction_get_xattr(int fd, const char *path, const char *name,
				void **contents, size_t *contentlen);
FedFsStatus	 junction_set_xattr(int fd, const char *path, const char *name,
			const void *contents, const size_t contentlen);
FedFsStatus	 junction_remove_xattr(int fd, const char *pathname,
			const char *name);
FedFsStatus	 junction_get_mode(const char *pathname, mode_t *mode);
FedFsStatus	 junction_save_mode(const char *pathname);
FedFsStatus	 junction_restore_mode(const char *pathname);


/**
 ** XML helper functions
 **/

_Bool		 junction_xml_is_empty(const xmlChar *content);
_Bool		 junction_xml_match_node_name(xmlNodePtr node,
			const xmlChar *name);
xmlNodePtr	 junction_xml_find_child_by_name(xmlNodePtr parent,
			const xmlChar *name);
_Bool		 junction_xml_get_bool_attribute(xmlNodePtr node,
			const xmlChar *attrname, _Bool *value);
void		 junction_xml_set_bool_attribute(xmlNodePtr node,
			const xmlChar *attrname, _Bool value);
_Bool		 junction_xml_get_u8_attribute(xmlNodePtr node,
			const xmlChar *attrname, uint8_t *value);
_Bool		 junction_xml_get_int_attribute(xmlNodePtr node,
			const xmlChar *attrname, int *value);
void		 junction_xml_set_int_attribute(xmlNodePtr node,
			const xmlChar *attrname, int value);
_Bool		 junction_xml_get_int_content(xmlNodePtr node, int *value);
xmlNodePtr	 junction_xml_set_int_content(xmlNodePtr parent,
			const xmlChar *name, int value);
FedFsStatus	 junction_xml_parse(const char *pathname, const char *name,
			xmlDocPtr *doc);
FedFsStatus	 junction_xml_write(const char *pathname, const char *name,
			xmlDocPtr doc);

#endif	/* !_FEDFS_JUNCTION_INTERNAL_H_ */
