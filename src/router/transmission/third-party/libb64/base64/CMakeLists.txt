enable_language(CXX)

add_executable(base64
    base64.cc)

if(NOT LIBB64_STANDALONE_BUILD)
    add_executable(${PROJECT_NAME}::base64 ALIAS base64)
endif()

target_compile_definitions(base64
    PRIVATE
        BUFFERSIZE=16777216)

target_link_libraries(base64
    PRIVATE
        libb64)

if(LIBB64_ENABLE_INSTALL)
    install(
        TARGETS base64
        EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
