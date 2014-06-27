/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   +----------------------------------------------------------------------+
*/

/* $Id$ */
#ifndef DOM_PROPERTIES_H
#define DOM_PROPERTIES_H

/* attr properties */
int dom_attr_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_attr_specified_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_attr_value_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_attr_value_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_attr_owner_element_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_attr_schema_type_info_read(dom_object *obj, zval **retval TSRMLS_DC);

/* characterdata properties */
int dom_characterdata_data_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_characterdata_data_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_characterdata_length_read(dom_object *obj, zval **retval TSRMLS_DC);

/* document properties */
int dom_document_doctype_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_implementation_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_document_element_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_actual_encoding_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_actual_encoding_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_encoding_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_encoding_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_standalone_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_standalone_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_version_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_version_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_strict_error_checking_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_strict_error_checking_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_document_uri_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_document_uri_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_config_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_format_output_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_format_output_write(dom_object *obj, zval *newval TSRMLS_DC);
int	dom_document_validate_on_parse_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_validate_on_parse_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_resolve_externals_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_resolve_externals_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_preserve_whitespace_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_preserve_whitespace_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_recover_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_recover_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_document_substitue_entities_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_document_substitue_entities_write(dom_object *obj, zval *newval TSRMLS_DC);

/* documenttype properties */
int dom_documenttype_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_documenttype_entities_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_documenttype_notations_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_documenttype_public_id_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_documenttype_system_id_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_documenttype_internal_subset_read(dom_object *obj, zval **retval TSRMLS_DC);

/* domerror properties */
int dom_domerror_severity_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domerror_message_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domerror_type_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domerror_related_exception_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domerror_related_data_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domerror_location_read(dom_object *obj, zval **retval TSRMLS_DC);

/* domimplementationlist properties */
int dom_domimplementationlist_length_read(dom_object *obj, zval **retval TSRMLS_DC);

/* domlocator properties */
int dom_domlocator_line_number_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domlocator_column_number_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domlocator_offset_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domlocator_related_node_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_domlocator_uri_read(dom_object *obj, zval **retval TSRMLS_DC);

/* domstringlist properties */
int dom_domstringlist_length_read(dom_object *obj, zval **retval TSRMLS_DC);

/* element properties */
int dom_element_tag_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_element_schema_type_info_read(dom_object *obj, zval **retval TSRMLS_DC);

/* entity properties */
int dom_entity_public_id_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_entity_system_id_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_entity_notation_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_entity_actual_encoding_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_entity_actual_encoding_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_entity_encoding_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_entity_encoding_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_entity_version_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_entity_version_write(dom_object *obj, zval *newval TSRMLS_DC);

/* namednodemap properties */
int dom_namednodemap_length_read(dom_object *obj, zval **retval TSRMLS_DC);

/* namelist properties */
int dom_namelist_length_read(dom_object *obj, zval **retval TSRMLS_DC);

/* node properties */
int dom_node_node_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_node_value_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_node_value_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_node_node_type_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_parent_node_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_child_nodes_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_first_child_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_last_child_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_previous_sibling_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_next_sibling_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_attributes_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_owner_document_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_namespace_uri_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_prefix_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_prefix_write(dom_object *obj, zval *newval TSRMLS_DC);
int dom_node_local_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_base_uri_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_text_content_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_node_text_content_write(dom_object *obj, zval *newval TSRMLS_DC);

/* nodelist properties */
int dom_nodelist_length_read(dom_object *obj, zval **retval TSRMLS_DC);

/* notation properties */
int dom_notation_public_id_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_notation_system_id_read(dom_object *obj, zval **retval TSRMLS_DC);

/* processinginstruction properties */
int dom_processinginstruction_target_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_processinginstruction_data_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_processinginstruction_data_write(dom_object *obj, zval *newval TSRMLS_DC);

/* text properties */
int dom_text_whole_text_read(dom_object *obj, zval **retval TSRMLS_DC);

/* typeinfo properties */
int dom_typeinfo_type_name_read(dom_object *obj, zval **retval TSRMLS_DC);
int dom_typeinfo_type_namespace_read(dom_object *obj, zval **retval TSRMLS_DC);

#if defined(LIBXML_XPATH_ENABLED)
/* xpath properties */
int dom_xpath_document_read(dom_object *obj, zval **retval TSRMLS_DC);
#endif

#endif /* DOM_PROPERTIERS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
