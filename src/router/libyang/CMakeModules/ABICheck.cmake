# generate API/ABI report
macro(LIB_ABI_CHECK LIB_TARGET LIB_HEADERS LIB_SOVERSION_FULL ABI_BASE_HASH)
    # get short hash
    string(SUBSTRING "${ABI_BASE_HASH}" 0 8 ABI_BASE_HASH_SHORT)

    # find abi-dumper
    find_program(ABI_DUMPER abi-dumper)
    find_package_handle_standard_args(abi-dumper DEFAULT_MSG ABI_DUMPER)
    if(NOT ABI_DUMPER)
        message(FATAL_ERROR "Program abi-dumper not found!")
    endif()

    # find abi-checker
    find_program(ABI_CHECKER abi-compliance-checker)
    find_package_handle_standard_args(abi-compliance-checker DEFAULT_MSG ABI_CHECKER)
    if(NOT ABI_CHECKER)
        message(FATAL_ERROR "Program abi-compliance-checker not found!")
    endif()

    # abi-dump target - generating an ABI dump
    set(PUBLIC_HEADERS ${LIB_HEADERS})
    string(PREPEND PUBLIC_HEADERS "${CMAKE_SOURCE_DIR}/")
    string(REPLACE ";" "\n${CMAKE_SOURCE_DIR}/" PUBLIC_HEADERS "${PUBLIC_HEADERS}")
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/public_headers CONTENT "${PUBLIC_HEADERS}")
    add_custom_target(abi-dump
            COMMAND ${ABI_DUMPER} ./lib${LIB_TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX}
            -o lib${LIB_TARGET}.${LIB_SOVERSION_FULL}.dump
            -lver ${LIB_SOVERSION_FULL} -public-headers ${CMAKE_BINARY_DIR}/public_headers
            DEPENDS ${LIB_TARGET}
            BYPRODUCTS ${CMAKE_BINARY_DIR}/lib${LIB_TARGET}.${LIB_SOVERSION_FULL}.dump
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Dumping ABI information of version ${LIB_SOVERSION_FULL} for abi-check")

    # get URL for fetching origin
    execute_process(COMMAND git remote get-url origin OUTPUT_VARIABLE ORIGIN_URL OUTPUT_STRIP_TRAILING_WHITESPACE)

    # generate script for generating the base ABI dump
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/abibase.sh CONTENT "#!/bin/sh
if [ ! -d abibase ]; then mkdir abibase; fi
cd abibase
if [ ! -f build/lib${LIB_TARGET}.*.dump ]; then
    if [ -d .git ] && [ \"${ABI_BASE_HASH}\" != \"`git log --pretty=oneline | cut -d' ' -f1`\" ]; then rm -rf .* 2> /dev/null; fi
    if [ ! -d .git ]; then
        git init --initial-branch=master
        git remote add origin ${ORIGIN_URL}
        git fetch origin --depth 1 ${ABI_BASE_HASH}
        git reset --hard FETCH_HEAD
    fi
    if [ ! -d build ]; then mkdir build; fi
    cd build
    cmake -DCMAKE_BUILD_TYPE=ABICheck ..
    make abi-dump
fi
")

    # abi-check target - check ABI compatibility of current version and the base hash version
    add_custom_target(abi-check
            COMMAND bash ./abibase.sh
            COMMAND ${ABI_CHECKER} -l lib${LIB_TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX}
            -old abibase/build/lib${LIB_TARGET}.*.dump
            -new ./lib${LIB_TARGET}.${LIB_SOVERSION_FULL}.dump -s
            DEPENDS ${LIB_TARGET} abi-dump
            BYPRODUCTS ${CMAKE_BINARY_DIR}/compat_reports/lib${LIB_TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX}/*_to_${LIB_SOVERSION_FULL}/compat_report.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Checking ABI compatibility of version ${LIB_SOVERSION_FULL} and revision ${ABI_BASE_HASH_SHORT}")
endmacro()
