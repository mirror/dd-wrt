set(PACKAGE "libyang")
set(CPP_PACKAGE "libyang-cpp")
set(PYTHON_PACKAGE "python3-yang")
set(TOOLS_PACKAGE "libyang-tools")

find_program(DEB_BUILDER NAMES debuild)
find_program(RPM_BUILDER NAMES rpmbuild)

# setup package build
configure_file(${PROJECT_SOURCE_DIR}/packages/${PACKAGE}.spec.in ${PROJECT_BINARY_DIR}/build-packages/${PACKAGE}.spec)
configure_file(${PROJECT_SOURCE_DIR}/packages/${PACKAGE}.dsc.in ${PROJECT_BINARY_DIR}/build-packages/${PACKAGE}.dsc)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.control.in ${PROJECT_BINARY_DIR}/build-packages/debian.control @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.rules.in ${PROJECT_BINARY_DIR}/build-packages/debian.rules)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.compat
               ${PROJECT_BINARY_DIR}/build-packages/debian.compat COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/packages/${PACKAGE}.rpmlintrc
               ${PROJECT_BINARY_DIR}/build-packages/${PACKAGE}.rpmlintrc COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.${PACKAGE}.install
               ${PROJECT_BINARY_DIR}/build-packages/debian.${PACKAGE}.install COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.${PACKAGE}-dev.install
               ${PROJECT_BINARY_DIR}/build-packages/debian.${PACKAGE}-dev.install COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.${CPP_PACKAGE}.install
               ${PROJECT_BINARY_DIR}/build-packages/debian.${CPP_PACKAGE}.install COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/packages/debian.${CPP_PACKAGE}-dev.install
               ${PROJECT_BINARY_DIR}/build-packages/debian.${CPP_PACKAGE}-dev.install COPYONLY)
# no python package for Debian because there is only SWIG 3.10 on Debian 9 :-/
#configure_file(${PROJECT_SOURCE_DIR}/packages/debian.${PYTHON_PACKAGE}.install
#               ${PROJECT_BINARY_DIR}/build-packages/debian.${PYTHON_PACKAGE}.install COPYONLY)

if(NOT DEB_BUILDER)
    message(STATUS "Missing tools (devscripts, debhelper package) for building DEB package.")
else()
    # target for local build deb package
    message(STATUS "To build local DEB package, use \"build-deb\" target.")
    add_custom_target(build-deb
                      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                      COMMAND build-packages/local-deb.sh
    )
    configure_file(${PROJECT_SOURCE_DIR}/packages/local-deb.sh.in ${PROJECT_BINARY_DIR}/build-packages/local-deb.sh @ONLY)
endif()

if(NOT RPM_BUILDER)
    message(STATUS "Missing tools (rpm package) for building RPM package.")
else()
    # target for local build rpm package
    message(STATUS "To build local RPM package, use \"build-rpm\" target.")
    string(REPLACE ${PROJECT_SOURCE_DIR} "." EXCLUDE_BUILD_DIR ${PROJECT_BINARY_DIR})
    add_custom_target(build-rpm
                      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                      COMMAND build-packages/local-rpm.sh
    )
    configure_file(${PROJECT_SOURCE_DIR}/packages/local-rpm.sh.in ${PROJECT_BINARY_DIR}/build-packages/local-rpm.sh @ONLY)
endif()
