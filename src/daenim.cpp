/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
 *
 * Authors:
 * Cedric Pinson <mornifle@plopbyte.net>
 * jeremy Moles <jeremy@emperorlinux.com>
*/

#include "AnimtkViewerGUI"
#include "ViewerExt"
#include "KeyHandler"

#include "SocketCallback"

#include <iostream>

#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>

#include <osgGA/TrackballManipulator>


//#if defined WIN32
//    
//    #define APP_PATH_FINDER GetModuleFileName
//    #define FIRST_ARG NULL
//    #define ICON_PATH "Icons\\"
//    #define FONT "FreeMono.ttf"
//#elif defined UNIX
//    
//    #define APP_PATH_FINDER readlink
//    #define FIRST_ARG "/proc/self/exe"
//    #define ICON_PATH "../share/daenim/Icons/"
//    #define FONT "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
//    #include <sys/stat.h>
//    #include <sys/types.h> //TODO
//#endif


#if defined WIN32
    #include <windows.h>
    #define FONT "FreeMono.ttf"
#elif defined UNIX
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h> //TODO
    #define FONT "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
#endif

struct AnimationManagerFinder : public osg::NodeVisitor
{
    osg::ref_ptr<osgAnimation::BasicAnimationManager> _animManager;
    AnimationManagerFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {};

    void apply(osg::Node& node)
    {
        if (_animManager.valid())
        {
            return;
        }
        if (node.getUpdateCallback())
        {
            osgAnimation::AnimationManagerBase* b = dynamic_cast<osgAnimation::AnimationManagerBase*>(node.getUpdateCallback());
            if (b)
            {
                _animManager = new osgAnimation::BasicAnimationManager(*b);
                return;
            }
        }
        traverse(node);
    }
};



#include <osg/Material>
#include <osg/BlendFunc>

void add_text_to_node(osg::Node* curNode, std::string txt) {
    osg::Group* curGroup = curNode->asGroup();
    osg::Geode* textGeode = new osg::Geode();
    osgText::Text* textDrawable = new osgText::Text();
    textDrawable->setText(txt);
    //textDrawable->setAutoRotateToScreen(true);
    textDrawable->setCharacterSize(.04);
    textDrawable->setFont(FONT);
    curGroup->addChild(textGeode);
    textGeode->addDrawable(textDrawable);
    textGeode->setName("text");
    textGeode->setNodeMask(0x10);
}


void parse(osg::Node* curNode, std::string prefix = "", bool verbose=false)
{
    if (verbose) {
        std::cout<< prefix << "(" << curNode->className() << ") \"" << curNode->getName() << "\""<< std::endl;
    }
    if ( (std::string(curNode->className()) == "MatrixTransform") &&
    !(curNode->getName() == "")){
        if (verbose){
            std::cout<<"add a text Geode: "<<curNode->getName()<<std::endl;
        }
        add_text_to_node(curNode, curNode->getName());
    }

    osg::Node::DescriptionList Des = curNode->getDescriptions();
    for (int i=0; i<Des.size(); i++){
        if (verbose){
            std::cout<<"description: "<<Des[i]<<std::endl;
        }
        if      (Des[i] == "frame") curNode->setNodeMask(0x1);
        else if (Des[i] == "shape") curNode->setNodeMask(0x2);
        else if (Des[i] == "link") curNode->setNodeMask(0x4);
        else if (Des[i] == "inertia") curNode->setNodeMask(0x8);
    }

    osg::Group* curGroup = curNode->asGroup();
    if (curGroup) {
        for (unsigned int i = 0 ; i < curGroup->getNumChildren(); i ++) {
            parse(curGroup->getChild(i), prefix + "--->", verbose);
        }
    }
}



int main(int argc, char** argv) 
{
    //----------------- Get and parse arguments -------------------//
    osg::ArgumentParser arguments(&argc, argv);
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" TODO."); //TODO
    arguments.getApplicationUsage()->addCommandLineOption("--help","Display the help command");
    arguments.getApplicationUsage()->addCommandLineOption("-pos x y","Set the window position along x and y");
    arguments.getApplicationUsage()->addCommandLineOption("-window w h","Set the window width and height");
    arguments.getApplicationUsage()->addCommandLineOption("-fps <int>","Set the framerate of the view. Useful to save cpu consumption");
    arguments.getApplicationUsage()->addCommandLineOption("-bgcolor r g b a","Set background color: 4 floats in [0,1]");
    
    arguments.getApplicationUsage()->addCommandLineOption("-showframes <bool>","Display frames from the beginning");
    arguments.getApplicationUsage()->addCommandLineOption("-showshapes <bool>","Display shapes from the beginning");
    arguments.getApplicationUsage()->addCommandLineOption("-showlinks <bool>","Display links from the beginning");
    arguments.getApplicationUsage()->addCommandLineOption("-showinertias <bool>","Display inertias from the beginning");
    arguments.getApplicationUsage()->addCommandLineOption("-shownames <bool>","Display names from the beginning");
    
    arguments.getApplicationUsage()->addCommandLineOption("-snapshot <filename>","Take a snapshot of the scene, save in <filename> and close window");
    arguments.getApplicationUsage()->addCommandLineOption("-rec <directoryname>","Record frames of animation, if any, in <directoryname>");
    
    arguments.getApplicationUsage()->addCommandLineOption("-eye x y z","Set the eye/camera position");
    arguments.getApplicationUsage()->addCommandLineOption("-coi x y z","Set the Center Of Interest position");
    arguments.getApplicationUsage()->addCommandLineOption("-up x y z","Set the Up vector");
    
    arguments.getApplicationUsage()->addCommandLineOption("-socket h p","Set a port connection to update the scene. example \"-socket 127.0.0.1 5000\"");
    arguments.getApplicationUsage()->addCommandLineOption("-verbose","Set the application to be verbose during graph building");
    
    
    if (arguments.read("--help") || arguments.argc()<=1 )
    {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 0;
    }

    std::string daeFile = arguments[1]; //The daefile MUST be the first argument!!

    unsigned int x=50, y=50, width=800, height=600, fps=-1;
    arguments.read("-pos", x, y);
    arguments.read("-window", width, height);
    arguments.read("-fps", fps);
    
    osg::Vec4f backGroundColor(-1,-1,-1,-1);
    arguments.read("-bgcolor", backGroundColor[0], backGroundColor[1],
                               backGroundColor[2], backGroundColor[3]);
    
    osg::Node::NodeMask displayMask = 0xffffffe7;
    int res;
    if (arguments.read("-showframes", res))   {if (res) displayMask |= (1 << 0); else displayMask &= ~(1 << 0);};
    if (arguments.read("-showshapes", res))   {if (res) displayMask |= (1 << 1); else displayMask &= ~(1 << 1);};
    if (arguments.read("-showlinks", res))    {if (res) displayMask |= (1 << 2); else displayMask &= ~(1 << 2);};
    if (arguments.read("-showinertias", res)) {if (res) displayMask |= (1 << 3); else displayMask &= ~(1 << 3);};
    if (arguments.read("-shownames", res))    {if (res) displayMask |= (1 << 4); else displayMask &= ~(1 << 4);};

    bool takeSnapShot = false;
    std::string snapShotName = "snapshot_daenim.png";
    takeSnapShot = arguments.read("-snapshot", snapShotName);
    bool recordAnimation = false;
    std::string recordDirectoryName = "daenim_recordAnimation";
    recordAnimation = arguments.read("-rec", recordDirectoryName);

    osg::Vec3d eye(0,0,1);
    osg::Vec3d coi(0,0,0);
    osg::Vec3d up(0,1,0);
    
    bool setNewHome = false;
    setNewHome |= arguments.read("-eye", eye[0], eye[1], eye[2]);
    setNewHome |= arguments.read("-coi", coi[0], coi[1], coi[2]);
    setNewHome |= arguments.read("-up", up[0], up[1], up[2]);

    std::string host;
    unsigned int port=0;
    bool communicationWithSocket = arguments.read("-socket", host, port);
    bool hasAnAnimation = false;
    
    bool verbose = false;
    verbose = arguments.read("-verbose");




    //----------------- Load data -------------------//
    osg::Group* rootGroup = new osg::Group();
    osg::Node* fileNode  = osgDB::readNodeFile(daeFile);
    if(!fileNode)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    rootGroup->addChild(fileNode);
    parse(fileNode, "", verbose);

    //----------------- Create Viewer and interface -------------------//
    osgViewer::ViewerExt viewer;
    viewer.addEventHandler(new KeyEventHandler());
    
    if (communicationWithSocket)
    {
        SOCKET sock = OpenPort(host.c_str(), port);
        fileNode->setUpdateCallback(new SocketCallback(fileNode, sock));
    }
    else
    {
        //----------------- Check animation data -------------------//
        AnimationManagerFinder finder;
        fileNode->accept(finder);
        if (finder._animManager.valid())
        {
            hasAnAnimation = true;
            fileNode->setUpdateCallback(finder._animManager.get());
            AnimtkViewerGUI*        gui = new AnimtkViewerGUI(&viewer, width, height, finder._animManager.get()); //interface
            osg::Camera*     camera = gui->createParentOrthoCamera();
            rootGroup->addChild(camera);

            viewer.addEventHandler(new osgWidget::MouseHandler(gui));
            viewer.addEventHandler(new osgWidget::ResizeHandler(gui, camera));
        }
        else
        {
            osg::notify(osg::WARN) << "no animation found in the subgraph"<<std::endl;
        }
    }

    //------------------- Init and Start Viewer -----------------------//
    viewer.setUpViewInWindow(x, y, width, height);
    viewer.setSceneData(rootGroup);

    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator();
    viewer.setCameraManipulator(manipulator);
    
    osg::Camera* cam = viewer.getCamera();
    cam->setCullMask(displayMask);
    if (backGroundColor[3]>=0.)
    {
        cam->setClearColor(backGroundColor);
    }
    
    
    //----------------- Init Camera and alpha channel ---------------------//
    osg::DisplaySettings* ds = cam->getDisplaySettings();
    osg::GraphicsContext::Traits* traits = new osg::GraphicsContext::Traits(ds);
    const osg::GraphicsContext::Traits* src_traits = cam->getGraphicsContext()->getTraits();
    traits->x = src_traits->x;
    traits->y = src_traits->y;
    traits->width = src_traits->width;
    traits->height = src_traits->height;
    traits->alpha = 8;
    traits->windowDecoration = src_traits->windowDecoration;
    traits->doubleBuffer = src_traits->doubleBuffer;
    
    osg::GraphicsContext* pbuffer = osg::GraphicsContext::createGraphicsContext(traits);
    cam->setGraphicsContext(pbuffer);
    
    
    if (setNewHome)
    {
        viewer.getCameraManipulator()->setHomePosition(eye,coi,up);
        viewer.home();
    }

    if (takeSnapShot)
    {
        viewer.realize();
        viewer.frame(0);
        viewer.frame(0);
        viewer.takeSnapshot(snapShotName);
        return 0;
    }
    if (recordAnimation && hasAnAnimation)
    {
        viewer.realize();
        viewer.frame(0);
        viewer.frame(0);
#if defined WIN32
            CreateDirectory(recordDirectoryName.c_str(), NULL);
#elif defined UNIX
            mkdir(recordDirectoryName.c_str(), 0755);
#endif
        char buffer[64];
        for (int i=0; i<=viewer.getTotalFrame(); i++)
        {
#if defined WIN32
            sprintf(buffer, ".\\%s\\%06i.png", recordDirectoryName.c_str(), i);
#elif defined UNIX
            sprintf(buffer, "./%s/%06i.png", recordDirectoryName.c_str(), i);
#endif
            viewer.setFrame(i);
            viewer.frame(viewer.getCurrentTime());
            viewer.takeSnapshot(buffer);
        }
        return 0;
    }

    if (hasAnAnimation || fps<=0)
    {
        return viewer.run();
    }
    else
    {
        while (!viewer.done())
        {
            viewer.frame();
#ifdef UNIX
            usleep(1000000/fps);
#elif WIN32
            Sleep(1000/fps);
#endif
        }
        return 0;
    }
}




