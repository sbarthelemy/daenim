cmake_minimum_required(VERSION 2.8)
project(daenim CXX)
set(CMAKE_MODULE_PATH "${daenim_SOURCE_DIR}/cmake")

set(CMAKE_BUILD_TYPE Debug)
set(EXECUTABLE_OUTPUT_PATH ${daenim_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${daenim_BINARY_DIR}/lib)

include(InstallRequiredSystemLibraries)

add_subdirectory(src)
install(
    DIRECTORY "${daenim_SOURCE_DIR}/data/Icons"
    DESTINATION share/daenim
    FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ
    DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_EXECUTE OWNER_WRITE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

# Packaging

# TODO: set the following options:
#CPACK_RESSOURCE_FILE_WELCOME
#CPACK_RESSOURCE_FILE_README
#CPACK_RESSOURCE_FILE_LICENSE
include(CPack)
