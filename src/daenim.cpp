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
#include <osg/PositionAttitudeTransform>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>
#include <osgGA/TrackballManipulator>
#include <osg/Material>
#include <osg/BlendFunc>


#if defined WIN32
    #include <windows.h>
    #define FONT "FreeMono.ttf"
#elif defined UNIX
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h> //TODO
    #define FONT "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
#endif





/**
 * This is a structure used to know if the collada file has animations or not.
 */
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



/**
 * Add text to nodes with their id.
 */
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


/**
 * This is a recursive function to parse the collada file and set a node
 * mask depending on the category of each node.
 */
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



/**
 * Define this program arguments
 */
osg::ArgumentParser* defineProgramArguments(int *argc, char** argv)
{
    //----------------- Define arguments -------------------//
    osg::ArgumentParser* arg = new osg::ArgumentParser(argc, argv);

    arg->getApplicationUsage()->setApplicationName(arg->getApplicationName());
    arg->getApplicationUsage()->setDescription(arg->getApplicationName()+" TODO."); //TODO
    arg->getApplicationUsage()->addCommandLineOption("-h | -help | --help","Display the help command");
    arg->getApplicationUsage()->addCommandLineOption("-pos x y","Set the window position along x and y");
    arg->getApplicationUsage()->addCommandLineOption("-window w h","Set the window width and height");
    arg->getApplicationUsage()->addCommandLineOption("-fps <int>","Set the framerate of the view. Useful to save cpu consumption");
    arg->getApplicationUsage()->addCommandLineOption("-bgcolor r g b a","Set background color: 4 floats in [0,1]");
    
    arg->getApplicationUsage()->addCommandLineOption("-showframes <bool>","Display frames from the beginning");
    arg->getApplicationUsage()->addCommandLineOption("-showshapes <bool>","Display shapes from the beginning");
    arg->getApplicationUsage()->addCommandLineOption("-showlinks <bool>","Display links from the beginning");
    arg->getApplicationUsage()->addCommandLineOption("-showinertias <bool>","Display inertias from the beginning");
    arg->getApplicationUsage()->addCommandLineOption("-shownames <bool>","Display names from the beginning");
    
    arg->getApplicationUsage()->addCommandLineOption("-snapshot <filename>","Take a snapshot of the scene, save in <filename> and close window");
    arg->getApplicationUsage()->addCommandLineOption("-time <float>","Give the current time of the scene, to take snapshot.");
    arg->getApplicationUsage()->addCommandLineOption("-rec <directoryname>","Record frames of animation, if any, in <directoryname>");
    arg->getApplicationUsage()->addCommandLineOption("-extension <type>","type of extension for record and snapshot (default 'png')");
    
    arg->getApplicationUsage()->addCommandLineOption("-eye x y z","Set the eye/camera position(default 2 2 2)");
    arg->getApplicationUsage()->addCommandLineOption("-coi x y z","Set the Center Of Interest position (default 0 0 0)");
    arg->getApplicationUsage()->addCommandLineOption("-up x y z","Set the Up vector (default set in dae file or 0 0 1)");
    
    arg->getApplicationUsage()->addCommandLineOption("-socket h p","Set a port connection to update the scene. example \"-socket 127.0.0.1 5000\"");
    arg->getApplicationUsage()->addCommandLineOption("-verbose","Set the application to be verbose during graph building");

    return arg;
}



struct ArgContainer
{
    std::string         daeFile;
    unsigned int        x, y, width, height, port;
    float               time, fps;
    osg::Vec4f          backgroundColor;
    osg::Node::NodeMask displayMask;
    bool                takeSnapShot, recordAnimation, hasAnAnimation, userChangeUp, verbose, communicationWithSocket;
    osg::Vec3d          eye, coi, up;
    std::string         snapShotName, recordDirectoryName, extension_type, host;

    ArgContainer() {
        x=50;
        y=50;
        width=800;
        height=600;
        fps=-1;
        time=0;
        
        backgroundColor = osg::Vec4f(-1,-1,-1,-1);
        
        displayMask = 0xffffffe7;
        
        takeSnapShot=false;
        recordAnimation=false;
        userChangeUp=false;
        hasAnAnimation = false;
        verbose = false;
        communicationWithSocket = false;
        
        eye = osg::Vec3d(2,2,2);
        coi = osg::Vec3d(0,0,0);
        up  =osg::Vec3d(0,0,1);
        
        snapShotName = "snapshot_daenim.png";
        recordDirectoryName = "daenim_recordAnimation";
        extension_type = "png";
        
        port=0;
    };
};




ArgContainer* getUserArguments(osg::ArgumentParser* arg)
{
    //----------------- Get and parse arguments -------------------//
    ArgContainer* userArgs = new ArgContainer();

    userArgs->daeFile = (*arg)[1]; //The daefile MUST be the first argument!!
    
    arg->read("-pos", userArgs->x, userArgs->y);
    arg->read("-window", userArgs->width, userArgs->height);
    arg->read("-fps", userArgs->fps);

    arg->read("-bgcolor", userArgs->backgroundColor[0], userArgs->backgroundColor[1],
                          userArgs->backgroundColor[2], userArgs->backgroundColor[3]);

    int res;
    if (arg->read("-showframes", res))   {if (res) userArgs->displayMask |= (1 << 0); else userArgs->displayMask &= ~(1 << 0);};
    if (arg->read("-showshapes", res))   {if (res) userArgs->displayMask |= (1 << 1); else userArgs->displayMask &= ~(1 << 1);};
    if (arg->read("-showlinks", res))    {if (res) userArgs->displayMask |= (1 << 2); else userArgs->displayMask &= ~(1 << 2);};
    if (arg->read("-showinertias", res)) {if (res) userArgs->displayMask |= (1 << 3); else userArgs->displayMask &= ~(1 << 3);};
    if (arg->read("-shownames", res))    {if (res) userArgs->displayMask |= (1 << 4); else userArgs->displayMask &= ~(1 << 4);};

    userArgs->takeSnapShot = arg->read("-snapshot", userArgs->snapShotName);
    arg->read("-time", userArgs->time);
    userArgs->recordAnimation = arg->read("-rec", userArgs->recordDirectoryName);
    arg->read("-extension", userArgs->extension_type);

    arg->read("-eye", userArgs->eye[0], userArgs->eye[1], userArgs->eye[2]);
    arg->read("-coi", userArgs->coi[0], userArgs->coi[1], userArgs->coi[2]);
    userArgs->userChangeUp = arg->read("-up" , userArgs->up[0] , userArgs->up[1] , userArgs->up[2]);

    userArgs->communicationWithSocket = arg->read("-socket", userArgs->host, userArgs->port);

    userArgs->verbose = arg->read("-verbose");
    
    
    return userArgs;
}



void defineUpAxisInScene(osg::Node* fileNode, ArgContainer* userArgs)
{
    if ((std::string(fileNode->className()) == "PositionAttitudeTransform")) {
        osg::Quat quat = fileNode->asTransform()->asPositionAttitudeTransform()->getAttitude();

        if (userArgs->userChangeUp == false)
        {
            if ( quat == osg::Quat(osg::inDegrees(90.0f), osg::Vec3(1.0f,0.0f,0.0f)) )
            {
                userArgs->up  = osg::Vec3d(0,1,0);
            }
            else if ( quat == osg::Quat(osg::inDegrees(90.0f), osg::Vec3(0.0f,1.0f,0.0f)) )
            {
                userArgs->up  = osg::Vec3d(1,0,0);
            }
        }
        fileNode->asTransform()->asPositionAttitudeTransform()->setAttitude(osg::Quat(0.0f, osg::Vec3(1.0f,0.0f,0.0f)));
    }
}



void initViewer(osgViewer::ViewerExt* viewer, ArgContainer* userArgs)
{
    //------------------- Init and Start Viewer -----------------------//
    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator();
    viewer->setCameraManipulator(manipulator);
    
    osg::Camera* cam = viewer->getCamera();
    cam->setCullMask(userArgs->displayMask);
    if (userArgs->backgroundColor[3]>=0.)
    {
        cam->setClearColor(userArgs->backgroundColor);
    }
    viewer->setUpViewInWindow(userArgs->x, userArgs->y, userArgs->width, userArgs->height);
    
    
    //----------------- Init Camera and alpha channel ---------------------//
    osg::DisplaySettings* ds = cam->getDisplaySettings();
    osg::GraphicsContext::Traits* traits = new osg::GraphicsContext::Traits(ds);
    const osg::GraphicsContext::Traits* src_traits = cam->getGraphicsContext()->getTraits();
    traits->x = userArgs->x;
    traits->y = userArgs->y;
    traits->width = userArgs->width;
    traits->height = userArgs->height;
    traits->alpha = 8;
    traits->windowDecoration = src_traits->windowDecoration;
    traits->doubleBuffer = src_traits->doubleBuffer;
    
    osg::GraphicsContext* pbuffer = osg::GraphicsContext::createGraphicsContext(traits);
    cam->setGraphicsContext(pbuffer);
    
    
    //----------------- Init Camera position ---------------------//
    viewer->getCameraManipulator()->setHomePosition(userArgs->eye, userArgs->coi, userArgs->up);
    viewer->home();
}




int takeOneSnapShotAndQuit(osgViewer::ViewerExt* viewer, ArgContainer* userArgs)
{
    viewer->realize();
    viewer->frame(userArgs->time);
    viewer->frame(userArgs->time);
    viewer->takeSnapshot(userArgs->snapShotName);
    return 0;
}


int recordAnimationAndQuit(osgViewer::ViewerExt* viewer, ArgContainer* userArgs)
{
    viewer->realize();
    viewer->frame(0);
    viewer->frame(0);
#if defined WIN32
        CreateDirectory(userArgs->recordDirectoryName.c_str(), NULL);
#elif defined UNIX
        mkdir(userArgs->recordDirectoryName.c_str(), 0755);
#endif
    char buffer[64];
    for (int i=0; i<=viewer->getTotalFrame(); i++)
    {
#if defined WIN32
        sprintf(buffer, ".\\%s\\%06i.png", userArgs->recordDirectoryName.c_str(), i);
#elif defined UNIX
        sprintf(buffer, "./%s/%06i.png", userArgs->recordDirectoryName.c_str(), i);
#endif
        viewer->setFrame(i);
        viewer->frame(viewer->getCurrentTime());
        viewer->takeSnapshot(buffer);
    }
    return 0;
}





/**
 * Main function: load and display collada file.
 */
int main(int argc, char** argv) 
{
    //----------------- Define program arguments -------------------//
    osg::ArgumentParser* OSGArguments = defineProgramArguments(&argc, argv);


    //----------------- Display help and quit -------------------//
    if (OSGArguments->read("-h")     ||
        OSGArguments->read("-help")  ||
        OSGArguments->read("--help") ||
        OSGArguments->argc()<=1 )
    {
        OSGArguments->getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 0;
    }

    //----------------- Get arguments -------------------//
    ArgContainer* userArgs = getUserArguments(OSGArguments);


    //----------------- Load data in dae file -------------------//
    osg::Group* rootGroup = new osg::Group();
    osg::Node* fileNode   = osgDB::readNodeFile(userArgs->daeFile);
    if(!fileNode)
    {
        std::cout << OSGArguments->getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    rootGroup->addChild(fileNode);
    parse(fileNode, "", userArgs->verbose);


    //----------------- Modifiy fileNode to set the UP vector along Z -------------------//
    defineUpAxisInScene(fileNode, userArgs);


    //----------------- Create Viewer and interface -------------------//
    osgViewer::ViewerExt viewer;
    viewer.addEventHandler(new KeyEventHandler());
    
    if (userArgs->communicationWithSocket)
    {
        SOCKET sock = OpenPort(userArgs->host.c_str(), userArgs->port);
        fileNode->setUpdateCallback(new SocketCallback(fileNode, sock));
    }
    else
    {
        //----------------- Check animation data -------------------//
        AnimationManagerFinder finder;
        fileNode->accept(finder);
        if (finder._animManager.valid())
        {
            userArgs->hasAnAnimation = true;
            fileNode->setUpdateCallback(finder._animManager.get());
            AnimtkViewerGUI* gui = new AnimtkViewerGUI(&viewer, userArgs->width, userArgs->height, finder._animManager.get(), userArgs->extension_type); //interface
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
    viewer.setSceneData(rootGroup);


    //------------------- Init and Start Viewer -----------------------//
    initViewer(&viewer, userArgs);




    //------------------- What to do... -----------------------//
    if (userArgs->takeSnapShot)
    {
        return takeOneSnapShotAndQuit(&viewer, userArgs);
    }
    if (userArgs->recordAnimation && userArgs->hasAnAnimation)
    {
        return recordAnimationAndQuit(&viewer, userArgs);
    }

    if (userArgs->hasAnAnimation || userArgs->fps<=0)
    {
        return viewer.run();
    }
    else
    {
        while (!viewer.done())
        {
            viewer.frame();
#ifdef UNIX
            usleep(1000000/userArgs->fps);
#elif WIN32
            Sleep(1000/userArgs->fps);
#endif
        }
        return 0;
    }
}




