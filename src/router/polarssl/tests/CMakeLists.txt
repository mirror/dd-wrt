set(libs
     polarssl
)

if(USE_PKCS11_HELPER_LIBRARY)
    set(libs ${libs} pkcs11-helper)
endif(USE_PKCS11_HELPER_LIBRARY)

function(add_test_suite suite_name)
    if(ARGV1)
    set(data_name ${ARGV1})
    else()
    set(data_name ${suite_name})
    endif()

    add_custom_command(
        OUTPUT test_suite_${data_name}.c
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_code.pl ${CMAKE_CURRENT_SOURCE_DIR}/suites test_suite_${suite_name} test_suite_${data_name}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_code.pl polarssl fct.h suites/helpers.function suites/test_suite_${suite_name}.function suites/test_suite_${data_name}.data
    )
  
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    add_executable(test_suite_${data_name} test_suite_${data_name}.c)
    target_link_libraries(test_suite_${data_name} ${libs})
    add_test(${data_name}-suite test_suite_${data_name})
endfunction(add_test_suite)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function -Wno-unused-value")
