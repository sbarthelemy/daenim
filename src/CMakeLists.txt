

find_package(OSG)

# build

include_directories(
    ${OSG_INCLUDE_DIR}
)

IF (UNIX)
    ADD_DEFINITIONS(-DUNIX)
ENDIF (UNIX)
IF (APPLE)
    ADD_DEFINITIONS(-DAPPLE)
ENDIF (APPLE)


add_executable(
    daenim
    daenim.cpp
    ViewerExt.cpp
    AnimtkViewerGUI.cpp
    SocketCallback.cpp
    KeyHandler.cpp
)
target_link_libraries(
    daenim
    ${OSG_LIBRARY}
    ${OSGWIDGET_LIBRARY}
    ${OSGANIMATION_LIBRARY}
    ${OSGVIEWER_LIBRARY}
    ${OSGGA_LIBRARY}
    ${OSGDB_LIBRARY}
    ${OSGTEXT_LIBRARY}
    ${OPENTHREADS_LIBRARY}
)
set_target_properties(
    daenim
    PROPERTIES COMPILE_DEFINITIONS
    INSTALL_PREFIX=${INSTALL_PREFIX} # necessary to locate button icons
)


install(
    TARGETS daenim
    RUNTIME
    DESTINATION bin)
