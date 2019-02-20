/**
 * @file support/junction/nfs.c
 * @brief Create, delete, and read NFS junctions on the local file system
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

/*
 * An NFS junction is a list of NFS FSLs, represented in a well-formed XML
 * document:
 *
 * <?xml version="1.0" encoding="UTF-8"?>
 * <junction>
 *   <savedmode bits="1777" />
 *   <fileset>
 *     <location>
 *       <host name="fileserver.example.net" port="2049" />
 *       <path>
 *         <component>foo</component>
 *         <component>bar</component>
 *         <component>baz</component>
 *       </path>
 *       <currency>-1</currency>
 *       <genflags writable="false" going="false" split="true" />
 *       <transflags rdma="true" />
 *       <class simul="0" handle="0" fileid="0"
 *              writever="0" change="0" readdir="0" />
 *       <read rank="0" order="0" />
 *       <write rank="0" order="0" />
 *       <flags varsub="false" />
 *       <validfor>0</validfor>
 *     </location>
 *
 *     ....
 *
 *   </fileset>
 * </junction>
 *
 * NFS junction XML is stored in an extended attribute called
 * "trusted.junction.nfs".   The parent object is a directory.
 *
 * To help file servers discover junctions efficiently, the directory
 * has no execute bits, and the sticky bit is set.  In addition, an
 * extended attribute called "trusted.junction.type" is added.  The
 * contents are ignored in user space.
 *
 * Finally, for pre-existing directories that are converted to
 * junctions, their mode bits are saved in an extended attribute called
 * "trusted.junction.mode".  When the junction data is removed, the
 * directory's mode bits are restored from this information.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rpcsvc/nfs_prot.h>

#include "junction.h"
#include "junction-internal.h"
#include "xlog.h"

/**
 * Tag name of NFS location element of a junction XML document
 */
#define NFS_XML_LOCATION_TAG		(const xmlChar *)"location"

/**
 * Tag name of host child element of an NFS location element
 */
#define NFS_XML_HOST_TAG		(const xmlChar *)"host"

/**
 * Name of hostname attribute of a host element
 */
#define NFS_XML_HOST_NAME_ATTR		(const xmlChar *)"name"

/**
 * Name of IP port attribute of a host element
 */
#define NFS_XML_HOST_PORT_ATTR		(const xmlChar *)"port"

/**
 * Tag name of path child element of an NFS location element
 */
#define NFS_XML_PATH_TAG		(const xmlChar *)"path"

/**
 * Tag name of component child element of a path element
 */
#define NFS_XML_COMPONENT_TAG		(const xmlChar *)"component"

/**
 * Tag name of currency child element of an NFS location element
 */
#define NFS_XML_CURRENCY_TAG		(const xmlChar *)"currency"

/**
 * Tag name of genflags child element of an NFS location element
 */
#define NFS_XML_GENFLAGS_TAG		(const xmlChar *)"genflags"

/**
 * Name of writable attribute of a genflags element
 */
#define NFS_XML_GENFLAGS_WRITABLE_ATTR	(const xmlChar *)"writable"

/**
 * Name of going attribute of a genflags element
 */
#define NFS_XML_GENFLAGS_GOING_ATTR	(const xmlChar *)"going"

/**
 * Name of split attribute of a genflags element
 */
#define	NFS_XML_GENFLAGS_SPLIT_ATTR	(const xmlChar *)"split"

/**
 * Tag name of transflags child element of an NFS location element
 */
#define NFS_XML_TRANSFLAGS_TAG		(const xmlChar *)"transflags"

/**
 * Name of rdma attribute of a transflags element
 */
#define NFS_XML_TRANSFLAGS_RDMA_ATTR	(const xmlChar *)"rdma"

/**
 * Tag name of class child element of an NFS location element
 */
#define NFS_XML_CLASS_TAG		(const xmlChar *)"class"

/**
 * Name of simul attribute of a class element
 */
#define NFS_XML_CLASS_SIMUL_ATTR	(const xmlChar *)"simul"

/**
 * Name of handle attribute of a class element
 */
#define NFS_XML_CLASS_HANDLE_ATTR	(const xmlChar *)"handle"

/**
 * Name of fileid attribute of a class element
 */
#define NFS_XML_CLASS_FILEID_ATTR	(const xmlChar *)"fileid"

/**
 * Name of writever attribute of a class element
 */
#define NFS_XML_CLASS_WRITEVER_ATTR	(const xmlChar *)"writever"

/**
 * Name of change attribute of a class element
 */
#define NFS_XML_CLASS_CHANGE_ATTR	(const xmlChar *)"change"

/**
 * Name of readdir attribute of a class element
 */
#define NFS_XML_CLASS_READDIR_ATTR	(const xmlChar *)"readdir"

/**
 * Tag name of read child element of an NFS location element
 */
#define NFS_XML_READ_TAG		(const xmlChar *)"read"

/**
 * Name of rank attribute of a read element
 */
#define NFS_XML_READ_RANK_ATTR		(const xmlChar *)"rank"

/**
 * Name of order attribute of a read element
 */
#define NFS_XML_READ_ORDER_ATTR		(const xmlChar *)"order"

/**
 * Tag name of write attribute of an NFS location element
 */
#define NFS_XML_WRITE_TAG		(const xmlChar *)"write"

/**
 * Name of rank attribute of a write element
 */
#define NFS_XML_WRITE_RANK_ATTR		(const xmlChar *)"rank"

/**
 * Name of order attribute of a write element
 */
#define NFS_XML_WRITE_ORDER_ATTR	(const xmlChar *)"order"

/**
 * Tag name of flags child element of an NFS location element
 */
#define NFS_XML_FLAGS_TAG		(const xmlChar *)"flags"

/**
 * Name of varsub attribute of a flags element
 */
#define NFS_XML_FLAGS_VARSUB_ATTR	(const xmlChar *)"varsub"

/**
 * Tag name of a validfor child element of an NFS location element
 */
#define NFS_XML_VALIDFOR_TAG		(const xmlChar *)"validfor"

/**
 * XPath path to NFS location elements in a junction document
 */
#define NFS_XML_LOCATION_XPATH		(const xmlChar *)	\
						"/junction/fileset/location"


/**
 * Remove all NFS-related xattrs from a directory
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
static FedFsStatus
nfs_remove_locations(const char *pathname)
{
	FedFsStatus retval;
	int fd;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_remove_xattr(fd, pathname, JUNCTION_XATTR_NAME_NFS);

	(void)close(fd);
	return retval;
}

/**
 * Add a "host" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_host_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	uint16_t port = fsloc->nfl_hostport;
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_HOST_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add host element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	xmlSetProp(new, NFS_XML_HOST_NAME_ATTR,
			(const xmlChar *)fsloc->nfl_hostname);
	if (port != NFS_PORT && port != 0)
		junction_xml_set_int_attribute(new, NFS_XML_HOST_PORT_ATTR,
									port);

	return FEDFS_OK;
}

/**
 * Add a "path" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_path_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;
	int i;

	new = xmlNewTextChild(parent, NULL, NFS_XML_PATH_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add path element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	for (i = 0; fsloc->nfl_rootpath[i] != NULL; i++) {
		xmlNodePtr component;

		component = xmlNewTextChild(new , NULL,
						NFS_XML_COMPONENT_TAG,
						(const xmlChar *)
						fsloc->nfl_rootpath[i]);
		if (component == NULL) {
			xlog(D_GENERAL, "%s: Failed to add component "
					"element for %s",
				__func__, pathname);
			return FEDFS_ERR_SVRFAULT;
		}
	}

	return FEDFS_OK;
}

/**
 * Add a "currency" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_currency_xml(__attribute__((unused)) const char *pathname,
		xmlNodePtr parent, struct nfs_fsloc *fsloc)
{
	if (junction_xml_set_int_content(parent, NFS_XML_CURRENCY_TAG,
						fsloc->nfl_currency) == NULL)
		return FEDFS_ERR_SVRFAULT;
	return FEDFS_OK;
}

/**
 * Add a "genflags" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_genflags_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_GENFLAGS_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add genflags element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	junction_xml_set_bool_attribute(new, NFS_XML_GENFLAGS_WRITABLE_ATTR,
					fsloc->nfl_genflags.nfl_writable);
	junction_xml_set_bool_attribute(new, NFS_XML_GENFLAGS_GOING_ATTR,
					fsloc->nfl_genflags.nfl_going);
	junction_xml_set_bool_attribute(new, NFS_XML_GENFLAGS_SPLIT_ATTR,
					fsloc->nfl_genflags.nfl_split);

	return FEDFS_OK;
}

/**
 * Add a "transflags" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_transflags_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_TRANSFLAGS_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add transflags element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	junction_xml_set_bool_attribute(new, NFS_XML_TRANSFLAGS_RDMA_ATTR,
					fsloc->nfl_transflags.nfl_rdma);

	return FEDFS_OK;
}

/**
 * Add a "class" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_class_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_CLASS_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add class element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	junction_xml_set_int_attribute(new, NFS_XML_CLASS_SIMUL_ATTR,
						fsloc->nfl_info.nfl_simul);
	junction_xml_set_int_attribute(new, NFS_XML_CLASS_HANDLE_ATTR,
						fsloc->nfl_info.nfl_handle);
	junction_xml_set_int_attribute(new, NFS_XML_CLASS_FILEID_ATTR,
						fsloc->nfl_info.nfl_fileid);
	junction_xml_set_int_attribute(new, NFS_XML_CLASS_WRITEVER_ATTR,
						fsloc->nfl_info.nfl_writever);
	junction_xml_set_int_attribute(new, NFS_XML_CLASS_CHANGE_ATTR,
						fsloc->nfl_info.nfl_change);
	junction_xml_set_int_attribute(new, NFS_XML_CLASS_READDIR_ATTR,
						fsloc->nfl_info.nfl_readdir);

	return FEDFS_OK;
}

/**
 * Add a "read" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_read_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_READ_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add read element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	junction_xml_set_int_attribute(new, NFS_XML_READ_RANK_ATTR,
					fsloc->nfl_info.nfl_readrank);
	junction_xml_set_int_attribute(new, NFS_XML_READ_ORDER_ATTR,
					fsloc->nfl_info.nfl_readorder);

	return FEDFS_OK;
}

/**
 * Add a "write" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_write_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_WRITE_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add write element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	junction_xml_set_int_attribute(new, NFS_XML_WRITE_RANK_ATTR,
					fsloc->nfl_info.nfl_writerank);
	junction_xml_set_int_attribute(new, NFS_XML_WRITE_ORDER_ATTR,
					fsloc->nfl_info.nfl_writeorder);

	return FEDFS_OK;
}

/**
 * Add a "flags" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_flags_xml(const char *pathname, xmlNodePtr parent,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr new;

	new = xmlNewTextChild(parent, NULL, NFS_XML_FLAGS_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add flags element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	junction_xml_set_bool_attribute(new, NFS_XML_FLAGS_VARSUB_ATTR,
					fsloc->nfl_flags.nfl_varsub);

	return FEDFS_OK;
}

/**
 * Add a "validfor" child to a "location" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param parent parent element to which to add "host" child
 * @param fsloc NFS location containing host information to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_validfor_xml(__attribute__((unused)) const char *pathname,
		xmlNodePtr parent, struct nfs_fsloc *fsloc)
{
	if (junction_xml_set_int_content(parent, NFS_XML_VALIDFOR_TAG,
						fsloc->nfl_validfor) == NULL)
		return FEDFS_ERR_SVRFAULT;
	return FEDFS_OK;
}

/**
 * Construct and add one "location" element to a "fileset"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param fileset fileset element of junction XML parse tree
 * @param fsloc one NFS location to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_location_xml(const char *pathname, xmlNodePtr fileset,
		struct nfs_fsloc *fsloc)
{
	FedFsStatus retval;
	xmlNodePtr new;

	new = xmlNewTextChild(fileset, NULL, NFS_XML_LOCATION_TAG, NULL);
	if (new == NULL) {
		xlog(D_GENERAL, "%s: Failed to add location element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	retval = nfs_location_host_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_path_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_currency_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_genflags_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_transflags_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_class_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_read_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_write_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_location_flags_xml(pathname, new, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	return nfs_location_validfor_xml(pathname, new, fsloc);
}

/**
 * Construct and add a "fileset" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param root root element of junction XML parse tree
 * @param fslocs list of NFS locations to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_fileset_xml(const char *pathname, xmlNodePtr root,
		struct nfs_fsloc *fslocs)
{
	struct nfs_fsloc *next;
	xmlNodePtr fileset;
	FedFsStatus retval;

	fileset = xmlNewTextChild(root, NULL, JUNCTION_XML_FILESET_TAG, NULL);
	if (fileset == NULL) {
		xlog(D_GENERAL, "%s: Failed to add fileset element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	for (next = fslocs; next != NULL; next = next->nfl_next) {
		retval = nfs_location_xml(pathname, fileset, next);
		if (retval != FEDFS_OK)
			return retval;
	}

	return FEDFS_OK;
}

/**
 * Construct a "savedmode" element
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param root root element of XML document tree
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_savedmode_xml(const char *pathname, xmlNodePtr root)
{
	xmlNodePtr savedmode;
	FedFsStatus retval;
	mode_t mode;
	char buf[8];

	retval = junction_get_mode(pathname, &mode);
	if (retval != FEDFS_OK)
		return retval;

	savedmode = xmlNewTextChild(root, NULL, JUNCTION_XML_SAVEDMODE_TAG, NULL);
	if (savedmode == NULL) {
		xlog(D_GENERAL, "%s: Failed to add savedmode element for %s\n",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	(void)snprintf(buf, sizeof(buf), "%o", ALLPERMS & mode);
	xmlSetProp(savedmode, JUNCTION_XML_MODEBITS_ATTR, (const xmlChar *)buf);

	return FEDFS_OK;
}

/**
 * Construct NFS junction XML document from list of NFS locations
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param doc an XML parse tree in which to construct the junction XML document
 * @param fslocs list of NFS locations to add
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_junction_xml(const char *pathname, xmlDocPtr doc,
		struct nfs_fsloc *fslocs)
{
	FedFsStatus retval;
	xmlNodePtr root;

	root = xmlNewNode(NULL, JUNCTION_XML_ROOT_TAG);
	if (root == NULL) {
		xlog(D_GENERAL, "%s: Failed to create root element for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}
	(void)xmlDocSetRootElement(doc, root);

	retval = nfs_savedmode_xml(pathname, root);
	if (retval != FEDFS_OK)
		return retval;

	return nfs_fileset_xml(pathname, root, fslocs);
}

/**
 * Write NFS locations information into an NFS junction extended attribute
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param doc an empty XML parse tree in which to construct the junction XML document
 * @param fslocs list of NFS locations to add
 * @return a FedFsStatus code
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
static FedFsStatus
nfs_write_junction(const char *pathname, xmlDocPtr doc,
		struct nfs_fsloc *fslocs)
{
	FedFsStatus retval;

	retval = nfs_junction_xml(pathname, doc, fslocs);
	if (retval != FEDFS_OK)
		return retval;

	return junction_xml_write(pathname, JUNCTION_XATTR_NAME_NFS, doc);
}

/**
 * Store NFS locations information into a junction object
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param fslocs list of NFS locations to add
 * @return a FedFsStatus code
 *
 * @note Access to trusted attributes requires CAP_SYS_ADMIN.
 */
static FedFsStatus
nfs_store_locations(const char *pathname, struct nfs_fsloc *fslocs)
{
	FedFsStatus retval;
	xmlDocPtr doc;

	doc = xmlNewDoc((xmlChar *)"1.0");
	if (doc == NULL) {
		xlog(D_GENERAL, "%s: Failed to create XML doc for %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	retval = nfs_write_junction(pathname, doc, fslocs);

	xmlFreeDoc(doc);
	return retval;
}

/**
 * Add NFS junction information to a pre-existing object
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param fslocs list of NFS locations to add
 * @return a FedFsStatus code
 *
 * An error occurs if the object referred to by "pathname" does not
 * exist or contains existing junction data.
 */
FedFsStatus
nfs_add_junction(const char *pathname, struct nfs_fsloc *fslocs)
{
	FedFsStatus retval;

	if (fslocs == NULL)
		return FEDFS_ERR_INVAL;

	retval = nfs_is_prejunction(pathname);
	if (retval != FEDFS_ERR_NOTJUNCT)
		return retval;

	retval = nfs_store_locations(pathname, fslocs);
	if (retval != FEDFS_OK)
		goto out_err;

	retval = junction_save_mode(pathname);
	if (retval != FEDFS_OK)
		goto out_err;

	return retval;

out_err:
	(void)nfs_remove_locations(pathname);
	return retval;
}

/**
 * Remove NFS junction information from an object
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 *
 * An error occurs if the object referred to by "pathname" does not
 * exist or does not contain NFS junction data.
 */
FedFsStatus
nfs_delete_junction(const char *pathname)
{
	FedFsStatus retval;

	retval = nfs_is_junction(pathname);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_restore_mode(pathname);
	if (retval != FEDFS_OK)
		return retval;

	return nfs_remove_locations(pathname);
}

/**
 * Parse the first "host" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_host(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	FedFsStatus retval;
	xmlChar *hostname;
	xmlNodePtr node;
	int hostport;

	retval = FEDFS_ERR_NOTJUNCT;
	node = junction_xml_find_child_by_name(location, NFS_XML_HOST_TAG);
	if (node == NULL)
		return retval;

	hostname = xmlGetProp(node, NFS_XML_HOST_NAME_ATTR);
	if (!junction_xml_get_int_attribute(node, NFS_XML_HOST_PORT_ATTR,
							&hostport))
		fsloc->nfl_hostport = NFS_PORT;
	else {
		if (hostport < 1 || hostport > UINT16_MAX) {
			xlog(D_GENERAL, "%s: Bad port attribute on %s",
				__func__, pathname);
			goto out;
		}
		fsloc->nfl_hostport = (uint16_t)hostport;
	}
	if (hostname == NULL) {
		xlog(D_GENERAL, "%s: No hostname attribute on %s",
			__func__, pathname);
		goto out;
	}
	fsloc->nfl_hostname = strdup((const char *)hostname);
	if (fsloc->nfl_hostname == NULL) {
		retval = FEDFS_ERR_SVRFAULT;
		goto out;
	}

	retval = FEDFS_OK;

out:
	xmlFree(hostname);
	return retval;
}

/**
 * Parse the first "path" child of "location" into a path array
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_path(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node, component;
	unsigned int count;
	xmlChar *value;
	char **result;

	node = junction_xml_find_child_by_name(location, NFS_XML_PATH_TAG);
	if (node == NULL)
		return FEDFS_ERR_NOTJUNCT;

	count = 0;
	for (component = node->children;
	     component != NULL;
	     component = component->next) {
		if (!junction_xml_match_node_name(component,
						NFS_XML_COMPONENT_TAG))
			continue;
		value = xmlNodeGetContent(component);
		if (junction_xml_is_empty(value)) {
			xlog(D_GENERAL, "%s: Bad pathname component in %s",
				__func__, pathname);
			return FEDFS_ERR_NOTJUNCT;
		}
		xmlFree(value);
		count++;
	}
	xlog(D_GENERAL, "%s: Found %u component(s)", __func__, count);

	if (count == 0) {
		xlog(D_GENERAL, "%s: Zero-component pathname", __func__);
		fsloc->nfl_rootpath = (char **)calloc(1, sizeof(char *));
		if (fsloc->nfl_rootpath == NULL)
			return FEDFS_ERR_SVRFAULT;
		fsloc->nfl_rootpath[0] = NULL;
		return FEDFS_OK;
	}

	result = calloc(count + 1, sizeof(char *));
	if (result == NULL)
		return FEDFS_ERR_SVRFAULT;

	count = 0;
	for (component = node->children;
	     component != NULL;
	     component = component->next) {
		if (!junction_xml_match_node_name(component,
						NFS_XML_COMPONENT_TAG))
			continue;
		value = xmlNodeGetContent(component);
		result[count] = strdup((const char *)value);
		xmlFree(value);
		if (result[count] == NULL) {
			nfs_free_string_array(result);
			return FEDFS_ERR_SVRFAULT;
		}
		count++;
	}

	fsloc->nfl_rootpath = result;
	return FEDFS_OK;
}

/**
 * Parse the first "currency" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_currency(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_CURRENCY_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_int_content(node, &fsloc->nfl_currency))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid currency element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "genflags" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_genflags(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_GENFLAGS_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_bool_attribute(node,
					NFS_XML_GENFLAGS_WRITABLE_ATTR,
					&fsloc->nfl_genflags.nfl_writable))
		goto out_err;
	if (!junction_xml_get_bool_attribute(node,
					NFS_XML_GENFLAGS_GOING_ATTR,
					&fsloc->nfl_genflags.nfl_going))
		goto out_err;
	if (!junction_xml_get_bool_attribute(node,
					NFS_XML_GENFLAGS_SPLIT_ATTR,
					&fsloc->nfl_genflags.nfl_split))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid genflags element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "transflags" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_transflags(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_TRANSFLAGS_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_bool_attribute(node,
					NFS_XML_TRANSFLAGS_RDMA_ATTR,
					&fsloc->nfl_transflags.nfl_rdma))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid transflags element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "class" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_class(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_CLASS_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_SIMUL_ATTR,
					&fsloc->nfl_info.nfl_simul))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_HANDLE_ATTR,
					&fsloc->nfl_info.nfl_handle))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_FILEID_ATTR,
					&fsloc->nfl_info.nfl_fileid))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_WRITEVER_ATTR,
					&fsloc->nfl_info.nfl_writever))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_WRITEVER_ATTR,
					&fsloc->nfl_info.nfl_writever))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_CHANGE_ATTR,
					&fsloc->nfl_info.nfl_change))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_CLASS_READDIR_ATTR,
					&fsloc->nfl_info.nfl_readdir))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid class element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "read" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_read(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_READ_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_READ_RANK_ATTR,
					&fsloc->nfl_info.nfl_readrank))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_READ_ORDER_ATTR,
					&fsloc->nfl_info.nfl_readorder))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid read element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "write" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_write(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_WRITE_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_WRITE_RANK_ATTR,
					&fsloc->nfl_info.nfl_writerank))
		goto out_err;
	if (!junction_xml_get_u8_attribute(node,
					NFS_XML_WRITE_ORDER_ATTR,
					&fsloc->nfl_info.nfl_writeorder))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid write element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "flags" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_flags(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_FLAGS_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_bool_attribute(node,
					NFS_XML_FLAGS_VARSUB_ATTR,
					&fsloc->nfl_flags.nfl_varsub))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid flags element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse the first "validfor" child of "location"
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 */
static FedFsStatus
nfs_parse_location_validfor(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	xmlNodePtr node;

	node = junction_xml_find_child_by_name(location, NFS_XML_VALIDFOR_TAG);
	if (node == NULL)
		goto out_err;

	if (!junction_xml_get_int_content(node, &fsloc->nfl_validfor))
		goto out_err;

	return FEDFS_OK;

out_err:
	xlog(D_GENERAL, "%s: Missing or invalid validfor element in %s",
		__func__, pathname);
	return FEDFS_ERR_NOTJUNCT;
}

/**
 * Parse children of NFS location element in an NFS junction
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc a blank nfs_fsloc to fill in
 * @return a FedFsStatus code
 *
 * All children are required only-once elements, and may appear in any order.
 * Extraneous or repeated elements are ignored for now.
 */
static FedFsStatus
nfs_parse_location_children(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc *fsloc)
{
	FedFsStatus retval;

	retval = nfs_parse_location_host(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_path(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_currency(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_genflags(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_transflags(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_class(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_read(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_write(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	retval = nfs_parse_location_flags(pathname, location, fsloc);
	if (retval != FEDFS_OK)
		return retval;
	return nfs_parse_location_validfor(pathname, location, fsloc);
}

/**
 * Parse NFS location element in an NFS junction
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param location XML parse tree containing fileset location element
 * @param fsloc OUT: a single NFS location item
 * @return a FedFsStatus code
 *
 * If nfs_parse_location() returns FEDFS_OK, caller must free the returned
 * location with nfs_free_location().
 */
static FedFsStatus
nfs_parse_node(const char *pathname, xmlNodePtr location,
		struct nfs_fsloc **fsloc)
{
	struct nfs_fsloc *tmp;
	FedFsStatus retval;

	tmp = nfs_new_location();
	if (tmp == NULL)
		return FEDFS_ERR_SVRFAULT;

	retval = nfs_parse_location_children(pathname, location, tmp);
	if (retval != FEDFS_OK)
		nfs_free_location(tmp);
	else
		*fsloc = tmp;
	return retval;
}

/**
 * Build list of NFS locations from a nodeset
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param nodeset XML nodeset containing "location" elements
 * @param fslocs OUT: pointer to a list of NFS locations
 * @return a FedFsStatus code
 *
 * If nfs_parse_nodeset() returns FEDFS_OK, caller must free the returned
 * list of locations with nfs_free_locations().
 */
static FedFsStatus
nfs_parse_nodeset(const char *pathname, xmlNodeSetPtr nodeset,
		struct nfs_fsloc **fslocs)
{
	struct nfs_fsloc *location, *result = NULL;
	FedFsStatus retval;
	int i;

	if (xmlXPathNodeSetIsEmpty(nodeset)) {
		xlog(D_GENERAL, "%s: No fileset locations found in %s",
			__func__, pathname);
		return FEDFS_ERR_NOTJUNCT;
	}

	for (i = 0; i < nodeset->nodeNr; i++) {
		xmlNodePtr node = nodeset->nodeTab[i];

		retval = nfs_parse_node(pathname, node, &location);
		if (retval != FEDFS_OK) {
			nfs_free_locations(result);
			return retval;
		}

		if (result == NULL)
			result = location;
		else
			result->nfl_next = location;
	}

	*fslocs = result;
	return FEDFS_OK;
}

/**
 * Parse fileset location information from junction XML
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param context XML path context containing junction XML
 * @param fslocs OUT: pointer to a list of NFS locations
 * @return a FedFsStatus code
 *
 * If nfs_parse_context() returns FEDFS_OK, caller must free the returned
 * list of locations with nfs_free_locations().
 */
static FedFsStatus
nfs_parse_context(const char *pathname, xmlXPathContextPtr context,
		struct nfs_fsloc **fslocs)
{
	xmlXPathObjectPtr object;
	FedFsStatus retval;

	object = xmlXPathEvalExpression(NFS_XML_LOCATION_XPATH, context);
	if (object == NULL) {
		xlog(D_GENERAL, "%s: Failed to evaluate XML in %s",
			__func__, pathname);
		return FEDFS_ERR_NOTJUNCT;
	}

	retval = nfs_parse_nodeset(pathname, object->nodesetval, fslocs);

	xmlXPathFreeObject(object);
	return retval;
}

/**
 * Parse NFS locations information from junction XML
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param doc XML parse tree containing junction XML document
 * @param fslocs OUT: pointer to a list of NFS locations
 * @return a FedFsStatus code
 *
 * If nfs_parse_xml() returns FEDFS_OK, caller must free the returned
 * list of locations with nfs_free_locations().
 */
static FedFsStatus
nfs_parse_xml(const char *pathname, xmlDocPtr doc, struct nfs_fsloc **fslocs)
{
	xmlXPathContextPtr context;
	FedFsStatus retval;

	context = xmlXPathNewContext(doc);
	if (context == NULL) {
		xlog(D_GENERAL, "%s: Failed to create XPath context from %s",
			__func__, pathname);
		return FEDFS_ERR_SVRFAULT;
	}

	retval = nfs_parse_context(pathname, context, fslocs);

	xmlXPathFreeContext(context);
	return retval;
}

/**
 * Retrieve list of NFS locations from an NFS junction
 *
 * @param pathname NUL-terminated C string containing pathname of a junction
 * @param fslocs OUT: pointer to a list of NFS locations
 * @return a FedFsStatus code
 *
 * If nfs_get_locations() returns FEDFS_OK, caller must free the returned
 * list of locations with nfs_free_locations().
 */
FedFsStatus
nfs_get_locations(const char *pathname, struct nfs_fsloc **fslocs)
{
	FedFsStatus retval;
	xmlDocPtr doc;

	if (fslocs == NULL)
		return FEDFS_ERR_INVAL;

	retval = junction_xml_parse(pathname, JUNCTION_XATTR_NAME_NFS, &doc);
	if (retval != FEDFS_OK)
		return retval;

	retval = nfs_parse_xml(pathname, doc, fslocs);

	xmlFreeDoc(doc);
	return retval;
}

/**
 * Predicate: does "pathname" refer to an object that can become an NFS junction?
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 *
 * Return values:
 *	FEDFS_ERR_NOTJUNCT:	"pathname" refers to an object that can be
 *				made into a NFS junction
 *	FEDFS_ERR_EXIST:	"pathname" refers to something that is
 *				already a junction
 *	FEDFS_ERR_INVAL:	"pathname" does not exist
 *	Other:			Some error occurred, "pathname" not
 *				investigated
 */
FedFsStatus
nfs_is_prejunction(const char *pathname)
{
	FedFsStatus retval;
	int fd;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_is_directory(fd, pathname);
	if (retval != FEDFS_OK)
		goto out_close;

	retval = junction_is_sticky_bit_set(fd, pathname);
	switch (retval) {
	case FEDFS_ERR_NOTJUNCT:
		break;
	case FEDFS_OK:
		goto out_exist;
	default:
		goto out_close;
	}

	retval = junction_is_xattr_present(fd, pathname, JUNCTION_XATTR_NAME_NFS);
	switch (retval) {
	case FEDFS_ERR_NOTJUNCT:
		break;
	case FEDFS_OK:
		goto out_exist;
	default:
		goto out_close;
	}

out_close:
	(void)close(fd);
	return retval;
out_exist:
	retval = FEDFS_ERR_EXIST;
	goto out_close;
}

/**
 * Verify that junction contains NFS junction XML
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 *
 * Return values:
 *	FEDFS_OK:		"pathname" refers to an NFS junction
 *	FEDFS_ERR_NOTJUNCT:	"pathname" refers to something that is
 *				not an NFS junction
 *	FEDFS_ERR_INVAL:	"pathname" does not exist
 *	Other:			Some error occurred, "pathname" not
 *				investigated
 *
 * NB: This is an expensive test.  However, it is only done if the object
 * actually has a junction extended attribute, meaning it should be done
 * rarely.  If this is really a problem, we can make the XML test cheaper.
 */
static FedFsStatus
nfs_is_junction_xml(const char *pathname)
{
	struct nfs_fsloc *fslocs = NULL;
	FedFsStatus retval;
	xmlDocPtr doc;

	retval = junction_xml_parse(pathname, JUNCTION_XATTR_NAME_NFS, &doc);
	if (retval != FEDFS_OK)
		return retval;

	retval = nfs_parse_xml(pathname, doc, &fslocs);
	nfs_free_locations(fslocs);

	xmlFreeDoc(doc);
	return retval;
}

/**
 * Predicate: does "pathname" refer to an NFS junction?
 *
 * @param pathname NUL-terminated C string containing pathname of a directory
 * @return a FedFsStatus code
 *
 * Return values:
 *	FEDFS_OK:		"pathname" refers to an NFS junction
 *	FEDFS_ERR_NOTJUNCT:	"pathname" refers to an object that is
 *				not a junction
 *	FEDFS_ERR_INVAL:	"pathname" does not exist
 *	Other:			Some error occurred, "pathname" not
 *				investigated
 */
FedFsStatus
nfs_is_junction(const char *pathname)
{
	FedFsStatus retval;
	int fd;

	retval = junction_open_path(pathname, &fd);
	if (retval != FEDFS_OK)
		return retval;

	retval = junction_is_directory(fd, pathname);
	if (retval != FEDFS_OK)
		goto out_close;

	retval = junction_is_sticky_bit_set(fd, pathname);
	if (retval != FEDFS_OK)
		goto out_close;

	retval = junction_is_xattr_present(fd, pathname, JUNCTION_XATTR_NAME_NFS);
	if (retval != FEDFS_OK)
		goto out_close;

	(void)close(fd);

	return nfs_is_junction_xml(pathname);

out_close:
	(void)close(fd);
	return retval;
}
