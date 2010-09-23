

find_package(OSG)

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
    ${OSG_LIBRARY}
    ${OSGWIDGET_LIBRARY}
    ${OSGANIMATION_LIBRARY}
)
#SET(TARGET_ADDED_LIBRARIES osgAnimation osgWidget)

# install

install(
    TARGETS daenim
    RUNTIME
    DESTINATION bin)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
    @ONLY
)
install(
    SCRIPT
    ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
)
