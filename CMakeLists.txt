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
    # set the project version variables according to the "VERSION.txt" file
    # or (if absent) the git tags (using "git descibe")
    set(SOURCE_DIR ${${PROJECT_NAME}_SOURCE_DIR})
    if(EXISTS ${SOURCE_DIR}/VERSION.txt)
        # get version from VERSION.txt file
        file(READ ${SOURCE_DIR}/VERSION.txt version_string)
        string(STRIP ${version_string} version_string)
    else()
        # get version from git tag
        find_package(Git)
        if(GIT_FOUND)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --match v*.*.*
                WORKING_DIRECTORY ${SOURCE_DIR}
                RESULT_VARIABLE ERROR_CODE
                OUTPUT_VARIABLE OUTPUT
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(ERROR_CODE EQUAL 0)
                # all went well (we are in a git repository, etc.)
                if(${OUTPUT} MATCHES "^v(.*)")
                    set(version_string ${CMAKE_MATCH_1})
                endif()
            endif()
        endif()
    endif()
    # parse the version string
    if(${version_string} MATCHES "^([0-9]+)[.]([0-9]+)[.](.*)")
        set(${PROJECT_NAME}_VERSION ${CMAKE_MATCH_0} PARENT_SCOPE)
        set(${PROJECT_NAME}_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
        set(${PROJECT_NAME}_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
        set(${PROJECT_NAME}_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)
        message(STATUS "project: " ${PROJECT_NAME})
        message(STATUS "version: " ${CMAKE_MATCH_0})
    endif()
endfunction(get_version)

get_version()

# Packaging

# TODO: set the following options:
#CPACK_RESSOURCE_FILE_WELCOME
#CPACK_RESSOURCE_FILE_README
#CPACK_RESSOURCE_FILE_LICENSE
include(CPack)
