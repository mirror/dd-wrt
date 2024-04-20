
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/basic/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/data/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extensions/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/schema/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/restriction/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/node/*.c
    PARENT_SCOPE)

ly_add_utest(NAME int8   SOURCES types/int8.c)
ly_add_utest(NAME string SOURCES types/string.c)
ly_add_utest(NAME bits   SOURCES types/bits.c)
ly_add_utest(NAME binary SOURCES types/binary.c)
ly_add_utest(NAME inet_types   SOURCES types/inet_types.c)
ly_add_utest(NAME yang_types   SOURCES types/yang_types.c)

ly_add_utest(NAME range  SOURCES restriction/test_range.c)
ly_add_utest(NAME pattern  SOURCES restriction/test_pattern.c)

ly_add_utest(NAME list  SOURCES node/list.c)

ly_add_utest(NAME common SOURCES basic/test_common.c)
ly_add_utest(NAME set SOURCES basic/test_set.c)
ly_add_utest(NAME hash_table SOURCES basic/test_hash_table.c)
ly_add_utest(NAME inout SOURCES basic/test_inout.c)
ly_add_utest(NAME context SOURCES basic/test_context.c)
ly_add_utest(NAME plugins SOURCES basic/test_plugins.c)
ly_add_utest(NAME xml SOURCES basic/test_xml.c)
ly_add_utest(NAME json SOURCES basic/test_json.c)
ly_add_utest(NAME xpath SOURCES basic/test_xpath.c)
ly_add_utest(NAME yanglib SOURCES basic/test_yanglib.c)

ly_add_utest(NAME schema SOURCES schema/test_schema.c schema/test_schema_common.c schema/test_schema_stmts.c schema/test_schema_extensions.c)
ly_add_utest(NAME parser_yang SOURCES schema/test_parser_yang.c)
ly_add_utest(NAME parser_yin SOURCES schema/test_parser_yin.c)
ly_add_utest(NAME tree_schema_compile SOURCES schema/test_tree_schema_compile.c)
ly_add_utest(NAME printer_tree SOURCES schema/test_printer_tree.c)
ly_add_utest(NAME printer_yang SOURCES schema/test_printer_yang.c)
ly_add_utest(NAME printer_yin SOURCES schema/test_printer_yin.c)

ly_add_utest(NAME tree_data SOURCES data/test_tree_data.c)
ly_add_utest(NAME new SOURCES data/test_new.c)
ly_add_utest(NAME parser_xml SOURCES data/test_parser_xml.c)
ly_add_utest(NAME printer_xml SOURCES data/test_printer_xml.c)
ly_add_utest(NAME parser_json SOURCES data/test_parser_json.c)
ly_add_utest(NAME lyb SOURCES data/test_lyb.c)
ly_add_utest(NAME validation SOURCES data/test_validation.c)
ly_add_utest(NAME types SOURCES data/test_types.c)
ly_add_utest(NAME merge SOURCES data/test_merge.c)
ly_add_utest(NAME diff SOURCES data/test_diff.c)

ly_add_utest(NAME metadata SOURCES extensions/test_metadata.c)
ly_add_utest(NAME nacm SOURCES extensions/test_nacm.c)
ly_add_utest(NAME yangdata SOURCES extensions/test_yangdata.c)
