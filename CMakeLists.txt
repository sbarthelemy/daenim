cmake_minimum_required(VERSION 2.8)
project(daenim CXX)
set(CMAKE_MODULE_PATH "${daenim_SOURCE_DIR}/cmake")

set(CMAKE_BUILD_TYPE Release)
set(EXECUTABLE_OUTPUT_PATH ${daenim_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${daenim_BINARY_DIR}/lib)


include(InstallRequiredSystemLibraries)
include(getversion)

add_subdirectory(src)
install(
    DIRECTORY ${daenim_SOURCE_DIR}/data/Icons
    DESTINATION share/daenim
)
get_version()

# Packaging

set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/" "/\\\\.DS_Store" ".*~" "\\\\.swp$")
set(CPACK_SOURCE_GENERATOR "TGZ")
# generate source tarball compatible with debian naming scheme
set(CPACK_SOURCE_PACKAGE_FILE_NAME "daenim_${daenim_VERSION}")
#CPACK_RESSOURCE_FILE_LICENSE
#CPACK_RESSOURCE_FILE_WELCOME
set(CPACK_RESSOURCE_FILE_README ${daenim_SOURCE_DIR}/README.txt)
if(APPLE)
    set(CPACK_GENERATOR "PackageMaker")
endif()
include(CPack)
