cmake_minimum_required(VERSION 2.8)
project(daenim CXX)
set(CMAKE_MODULE_PATH "${daenim_SOURCE_DIR}/cmake")

set(CMAKE_BUILD_TYPE Debug)
set(EXECUTABLE_OUTPUT_PATH ${daenim_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${daenim_BINARY_DIR}/lib)

include(InstallRequiredSystemLibraries)

add_subdirectory(src)
install(
    DIRECTORY ${daenim_SOURCE_DIR}/data/Icons
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

function(get_version)
    # Returns the version as defined by ``git describe``, or None."""
    find_package(Git)
    if (GIT_FOUND) #TODO : check we are in a git repository
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --match v*.*.*
            WORKING_DIRECTORY ${daenim_SOURCE_DIR}
            RESULT_VARIABLE ERROR_CODE
            OUTPUT_VARIABLE VERSION_STRING
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    if(ERROR_CODE EQUAL 0)
            set(VERSION_FOUND TRUE)
            string(
                REGEX MATCH
                "^v([0-9]+)[.]([0-9]+)[.](.*)"
                FOO
                ${VERSION_STRING}
            )
            set(DAENIM_VERSION_MAJOR ${CMAKE_MATCH_1})
            set(DAENIM_VERSION_MINOR ${CMAKE_MATCH_2})
            set(DAENIM_VERSION_PATCH ${CMAKE_MATCH_3})
        endif ()
    endif ()
    if (VERSION_FOUND)
        configure_file(
            ${daenim_SOURCE_DIR}/version.cmake.in
            ${daenim_SOURCE_DIR}/version.cmake)
    else ()
        include(version.cmake)
    endif ()
endfunction(get_version)

get_version()

# Packaging

# TODO: set the following options:
#CPACK_RESSOURCE_FILE_WELCOME
#CPACK_RESSOURCE_FILE_README
#CPACK_RESSOURCE_FILE_LICENSE
include(CPack)
