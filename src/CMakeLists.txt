SET(TARGET_SRC 
  ViewerExt.cpp
  AnimtkViewer.cpp
  AnimtkViewerKeyHandler.cpp
  AnimtkViewerGUI.cpp
  )
SET(TARGET_ADDED_LIBRARIES osgAnimation osgWidget)
SETUP_EXAMPLE(osganimationviewertest)
