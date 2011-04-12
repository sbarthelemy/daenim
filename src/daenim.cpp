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
    textDrawable->setFont("Arial.ttf");
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
    viewer.getCamera()->setCullMask(0xffffffe7);

    if (setNewHome)
    {
        viewer.getCameraManipulator()->setHomePosition(eye,coi,up);
        viewer.home();
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




