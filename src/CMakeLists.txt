find_package(OSG REQUIRED)

# build

include_directories(
    ${OSG_INCLUDE_DIR}
)
add_executable(
    daenim
    AnimtkViewer.cpp
    ViewerExt.cpp
    AnimtkViewerKeyHandler.cpp
    AnimtkViewerGUI.cpp
)
target_link_libraries(
    daenim
    ${OSG_LIBRARY_DEBUG}
    ${OSGWIDGET_LIBRARY_DEBUG}
    ${OSGANIMATION_LIBRARY_DEBUG}
    ${OSGVIEWER_LIBRARY_DEBUG}
    ${OSGGA_LIBRARY_DEBUG}
    ${OSGUTIL_LIBRARY_DEBUG}
    ${OSGDB_LIBRARY_DEBUG}
    ${OSGTEXT_LIBRARY_DEBUG}
    ${OPENTHREADS_LIBRARY_DEBUG}
)
set_target_properties(
    daenim
    PROPERTIES COMPILE_DEFINITIONS
    INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} # necessary to locate button icons
)

# install

install(
    TARGETS daenim
    RUNTIME
    DESTINATION bin)
if (APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
        @ONLY
    )
    install(
        SCRIPT
        ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
    )
endif (APPLE)
