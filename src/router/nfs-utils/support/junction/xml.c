/**
 * @file support/junction/xml.c
 * @brief Common utilities for managing junction XML
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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "junction.h"
#include "junction-internal.h"
#include "xlog.h"

/**
 * Predicate: is element content empty?
 *
 * @param content element content to test
 * @return true if content is empty
 */
_Bool
junction_xml_is_empty(const xmlChar *content)
{
	return content == NULL || *content == '\0';
}

/**
 * Match an XML parse tree node by its name
 *
 * @param node pointer to a node in an XML parse tree
 * @param name NUL-terminated C string containing name to match
 * @return true if "node" is named "name"
 */
_Bool
junction_xml_match_node_name(xmlNodePtr node, const xmlChar *name)
{
	return (node->type == XML_ELEMENT_NODE) &&
		(xmlStrcmp(node->name, name) == 0);
}

/**
 * Find a first-level child of "parent" named "name"
 *
 * @param parent pointer to node whose children are to be searched
 * @param name NUL-terminated C string containing name to match
 * @return pointer to child of "parent" whose name is "name"
 */
xmlNodePtr
junction_xml_find_child_by_name(xmlNodePtr parent, const xmlChar *name)
{
	xmlNodePtr node;

	for (node = parent->children; node != NULL; node = node->next)
		if (junction_xml_match_node_name(node, name))
			return node;
	return NULL;
}

/**
 * Read attribute into a boolean
 *
 * @param node pointer to a node in an XML parse tree
 * @param attrname NUL-terminated C string containing attribute name
 * @param value OUT: attribute's value converted to an integer
 * @return true if attribute "attrname" has a valid boolean value
 */
_Bool
junction_xml_get_bool_attribute(xmlNodePtr node, const xmlChar *attrname,
		_Bool *value)
{
	xmlChar *prop;
	_Bool retval;

	retval = false;
	prop = xmlGetProp(node, attrname);
	if (prop == NULL)
		goto out;

	if (xmlStrcmp(prop, (const xmlChar *)"true") == 0) {
		*value = true;
		retval = true;
		goto out;
	}

	if (xmlStrcmp(prop, (const xmlChar *)"false") == 0) {
		*value = false;
		retval = true;
		goto out;
	}

out:
	xmlFree(prop);
	return retval;
}

/**
 * Set attribute to a boolean
 *
 * @param node pointer to a node in an XML parse tree
 * @param attrname NUL-terminated C string containing attribute name
 * @param value boolean value to set
 */
void
junction_xml_set_bool_attribute(xmlNodePtr node, const xmlChar *attrname,
					_Bool value)
{
	xmlSetProp(node, attrname, (const xmlChar *)(value ? "true" : "false"));
}

/**
 * Read attribute into an uint8_t
 *
 * @param node pointer to a node in an XML parse tree
 * @param attrname NUL-terminated C string containing attribute name
 * @param value OUT: attribute's value converted to an uint8_t
 * @return true if attribute "attrname" has a valid uint8_t value
 */
_Bool
junction_xml_get_u8_attribute(xmlNodePtr node, const xmlChar *attrname,
		uint8_t *value)
{
	char *endptr;
	_Bool retval;
	char *prop;
	long tmp;

	retval = false;
	prop = (char *)xmlGetProp(node, attrname);
	if (prop == NULL)
		goto out;

	errno = 0;
	tmp = strtol(prop, &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tmp > 255 || tmp < 0)
		goto out;

	*value = (uint8_t)tmp;
	retval = true;

out:
	xmlFree(prop);
	return retval;
}

/**
 * Read attribute into an integer
 *
 * @param node pointer to a node in an XML parse tree
 * @param attrname NUL-terminated C string containing attribute name
 * @param value OUT: attribute's value converted to an integer
 * @return true if attribute "attrname" has a valid integer value
 */
_Bool
junction_xml_get_int_attribute(xmlNodePtr node, const xmlChar *attrname,
		int *value)
{
	char *endptr;
	_Bool retval;
	char *prop;
	long tmp;

	retval = false;
	prop = (char *)xmlGetProp(node, attrname);
	if (prop == NULL)
		goto out;

	errno = 0;
	tmp = strtol(prop, &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tmp > INT32_MAX || tmp < INT32_MIN)
		goto out;

	*value = (int)tmp;
	retval = true;

out:
	xmlFree(prop);
	return retval;
}

/**
 * Set attribute to an integer
 *
 * @param node pointer to a node in an XML parse tree
 * @param attrname NUL-terminated C string containing attribute name
 * @param value integer value to set
 */
void
junction_xml_set_int_attribute(xmlNodePtr node, const xmlChar *attrname,
		int value)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%d", value);
	xmlSetProp(node, attrname, (const xmlChar *)buf);
}

/**
 * Read node content into an integer
 *
 * @param node pointer to a node in an XML parse tree
 * @param value OUT: node's content converted to an integer
 * @return true if "node" has valid integer content
 */
_Bool
junction_xml_get_int_content(xmlNodePtr node, int *value)
{
	xmlChar *content;
	char *endptr;
	_Bool retval;
	long tmp;

	retval = false;
	content = xmlNodeGetContent(node);
	if (content == NULL)
		goto out;

	errno = 0;
	tmp = strtol((const char *)content, &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tmp > INT32_MAX || tmp < INT32_MIN)
		goto out;

	*value = (int)tmp;
	retval = true;

out:
	xmlFree(content);
	return retval;
}

/**
 * Add a child node with integer content
 *
 * @param parent  pointer to a node in an XML parse tree
 * @param name NUL-terminated C string containing name of child to add
 * @param value set node content to this value
 * @return pointer to new child node
 */
xmlNodePtr
junction_xml_set_int_content(xmlNodePtr parent, const xmlChar *name, int value)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%d", value);
	return xmlNewTextChild(parent, NULL, name, (const xmlChar *)buf);
}

/**
 * Parse XML document in a buffer into an XML document tree
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to replace
 * @param buf opaque byte array containing XML to parse
 * @param len size of "buf" in bytes
 * @param doc OUT: an XML parse tree containing junction XML
 * @return a FedFsStatus code
 *
 * If junction_parse_xml_buf() returns success, caller must free "*doc"
 * using xmlFreeDoc(3).
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
static FedFsStatus
junction_parse_xml_buf(const char *pathname, const char *name,
		void *buf, size_t len, xmlDocPtr *doc)
{
	xmlDocPtr tmp;

	tmp = xmlParseMemory(buf, (int)len);
	if (tmp == NULL) {
		xlog(D_GENERAL, "Failed to parse XML in %s(%s)\n",
			pathname, name);
		return FEDFS_ERR_SVRFAULT;
	}

	*doc = tmp;
	return FEDFS_OK;
}

/**
 * Read an XML document from an extended attribute into an XML document tree
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @param fd an open file descriptor
 * @param name NUL-terminated C string containing name of xattr to replace
 * @param doc OUT: an XML parse tree containing junction XML
 * @return a FedFsStatus code
 *
 * If junction_parse_xml_read() returns success, caller must free "*doc"
 * using xmlFreeDoc(3).
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
static FedFsStatus
junction_parse_xml_read(const char *pathname, int fd, const char *name,
		xmlDocPtr *doc)
{
	FedFsStatus retval;
	void *buf = NULL;
	size_t len;

	retval = junction_get_xattr(fd, pathname, name, &buf, &len);
	if (retval != FEDFS_OK)
		return retval;

	xlog(D_CALL, "%s: XML document contained in junction:\n%zu.%s",
		__func__, len, (char *)buf);

	retval = junction_parse_xml_buf(pathname, name, buf, len, doc);

	free(buf);
	return retval;
}

/**
 * Read an XML document from an extended attribute into an XML document tree
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to replace
 * @param doc OUT: an XML parse tree containing junction XML
 * @return a FedFsStatus code
 *
 * If junction_parse_xml() returns success, caller must free "*doc"
 * using xmlFreeDoc(3).
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_xml_parse(const char *pathname, const char *name, xmlDocPtr *doc)
{
	FedFsStatus retval;
	int fd;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_parse_xml_read(pathname, fd, name, doc);

	(void)close(fd);
	return retval;
}

/**
 * Write an XML document into an extended attribute
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @param name NUL-terminated C string containing name of xattr to replace
 * @param doc an XML parse tree containing junction XML
 * @return a FedFsStatus code
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
FedFsStatus
junction_xml_write(const char *pathname, const char *name, xmlDocPtr doc)
{
	xmlChar *buf = NULL;
	FedFsStatus retval;
	int fd, len;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = FEDFS_ERR_SVRFAULT;
	xmlIndentTreeOutput = 1;
	xmlDocDumpFormatMemoryEnc(doc, &buf, &len, "UTF-8", 1);
	if (len < 0)
		goto out;

	retval = junction_set_xattr(fd, pathname, name, buf, (size_t)len);

out:
	xmlFree(buf);
	(void)close(fd);
	return retval;
}
