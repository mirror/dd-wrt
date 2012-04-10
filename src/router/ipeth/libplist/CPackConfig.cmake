# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. Example variables are:
#   CPACK_GENERATOR                     - Generator used to create package
#   CPACK_INSTALL_CMAKE_PROJECTS        - For each project (path, name, component)
#   CPACK_CMAKE_GENERATOR               - CMake Generator used for the projects
#   CPACK_INSTALL_COMMANDS              - Extra commands to install components
#   CPACK_INSTALLED_DIRECTORIES           - Extra directories to install
#   CPACK_PACKAGE_DESCRIPTION_FILE      - Description file for the package
#   CPACK_PACKAGE_DESCRIPTION_SUMMARY   - Summary of the package
#   CPACK_PACKAGE_EXECUTABLES           - List of pairs of executables and labels
#   CPACK_PACKAGE_FILE_NAME             - Name of the package generated
#   CPACK_PACKAGE_ICON                  - Icon used for the package
#   CPACK_PACKAGE_INSTALL_DIRECTORY     - Name of directory for the installer
#   CPACK_PACKAGE_NAME                  - Package project name
#   CPACK_PACKAGE_VENDOR                - Package project vendor
#   CPACK_PACKAGE_VERSION               - Package project version
#   CPACK_PACKAGE_VERSION_MAJOR         - Package project version (major)
#   CPACK_PACKAGE_VERSION_MINOR         - Package project version (minor)
#   CPACK_PACKAGE_VERSION_PATCH         - Package project version (patch)

# There are certain generator specific ones

# NSIS Generator:
#   CPACK_PACKAGE_INSTALL_REGISTRY_KEY  - Name of the registry key for the installer
#   CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS - Extra commands used during uninstall
#   CPACK_NSIS_EXTRA_INSTALL_COMMANDS   - Extra commands used during install


SET(CPACK_BINARY_BUNDLE "")
SET(CPACK_BINARY_CYGWIN "")
SET(CPACK_BINARY_DEB "OFF")
SET(CPACK_BINARY_DRAGNDROP "")
SET(CPACK_BINARY_NSIS "OFF")
SET(CPACK_BINARY_OSXX11 "")
SET(CPACK_BINARY_PACKAGEMAKER "")
SET(CPACK_BINARY_RPM "OFF")
SET(CPACK_BINARY_STGZ "ON")
SET(CPACK_BINARY_TBZ2 "OFF")
SET(CPACK_BINARY_TGZ "ON")
SET(CPACK_BINARY_TZ "ON")
SET(CPACK_BINARY_ZIP "")
SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET(CPACK_COMPONENTS_ALL "Unspecified;dev;plutil")
SET(CPACK_COMPONENT_DEV_DEPENDS "lib")
SET(CPACK_COMPONENT_DEV_DISPLAY_NAME "PList development files")
SET(CPACK_COMPONENT_LIB_DISPLAY_NAME "PList library")
SET(CPACK_COMPONENT_PLUTIL_DEPENDS "lib")
SET(CPACK_COMPONENT_PLUTIL_DISPLAY_NAME "PList conversion tool")
SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
SET(CPACK_GENERATOR "STGZ;TGZ;TZ")
SET(CPACK_INSTALL_CMAKE_PROJECTS "/home/seg/DEV/pb42/src/router/ipeth/libplist;libplist;ALL;/")
SET(CPACK_INSTALL_PREFIX "/usr")
SET(CPACK_MODULE_PATH "/home/seg/DEV/pb42/src/router/ipeth/libplist/cmake;/home/seg/DEV/pb42/src/router/ipeth/libplist/cmake/modules")
SET(CPACK_NSIS_DISPLAY_NAME "libplist LIBPLIST_VERSION_MAJOR.LIBPLIST_VERSION_MINOR.1")
SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
SET(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
SET(CPACK_NSIS_PACKAGE_NAME "libplist LIBPLIST_VERSION_MAJOR.LIBPLIST_VERSION_MINOR.1")
SET(CPACK_OUTPUT_CONFIG_FILE "/home/seg/DEV/pb42/src/router/ipeth/libplist/CPackConfig.cmake")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "/home/seg/DEV/pb42/src/router/ipeth/libplist/README")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library to parse and generate Apple's binary and XML PList format")
SET(CPACK_PACKAGE_FILE_NAME "libplist-LIBPLIST_VERSION_MAJOR.LIBPLIST_VERSION_MINOR.1-Linux")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "libplist LIBPLIST_VERSION_MAJOR.LIBPLIST_VERSION_MINOR.1")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "libplist LIBPLIST_VERSION_MAJOR.LIBPLIST_VERSION_MINOR.1")
SET(CPACK_PACKAGE_NAME "libplist")
SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_PACKAGE_VENDOR "Humanity")
SET(CPACK_PACKAGE_VERSION "LIBPLIST_VERSION_MAJOR.LIBPLIST_VERSION_MINOR.1")
SET(CPACK_PACKAGE_VERSION_MAJOR "LIBPLIST_VERSION_MAJOR")
SET(CPACK_PACKAGE_VERSION_MINOR "LIBPLIST_VERSION_MINOR")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_RESOURCE_FILE_LICENSE "/home/seg/DEV/pb42/src/router/ipeth/libplist/COPYING.LESSER")
SET(CPACK_RESOURCE_FILE_README "/usr/share/cmake/Templates/CPack.GenericDescription.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake/Templates/CPack.GenericWelcome.txt")
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_SOURCE_CYGWIN "")
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;TZ")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/seg/DEV/pb42/src/router/ipeth/libplist/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_TBZ2 "ON")
SET(CPACK_SOURCE_TGZ "ON")
SET(CPACK_SOURCE_TZ "ON")
SET(CPACK_SOURCE_ZIP "OFF")
SET(CPACK_SYSTEM_NAME "Linux")
SET(CPACK_TOPLEVEL_TAG "Linux")
