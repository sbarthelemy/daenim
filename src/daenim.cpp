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

#ifdef WIN32
#include <windows.h>
#endif

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


void parse(osg::Node* curNode, std::string prefix = "")
{
    if (curNode->getName() == "frame_arrows") curNode->setNodeMask(0x1);
    if (curNode->getName() == "shape") curNode->setNodeMask(0x2);
    if (curNode->getName() == "link") curNode->setNodeMask(0x4);
    if (curNode->getName() == "inertia") curNode->setNodeMask(0x8);

    //std::cout<< prefix << " \"" << curNode->getName() << "\" (" << curNode->className() << ") " << std::endl;

    if (std::string(curNode->className()) == "MatrixTransform"){
        if ( !(curNode->getName() == "") && !(curNode->getName() == "frame_arrows")
          && !(curNode->getName() == "shape") && !(curNode->getName() == "link") && !(curNode->getName() == "inertia")){
            //std::cout<<"add a text Geode: "<<curNode->getName()<<std::endl;
            add_text_to_node(curNode, curNode->getName());
        }
    }

    osg::Group* curGroup = curNode->asGroup();
    if (curGroup) {
        for (unsigned int i = 0 ; i < curGroup->getNumChildren(); i ++) {
            parse(curGroup->getChild(i), prefix + "--->");
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
    arguments.getApplicationUsage()->addCommandLineOption("-socket h p","Set a port connection to update the scene. example \"-socket 127.0.0.1 5000\"");
    
    
    if (arguments.read("--help") || arguments.argc()<=1 )
    {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 0;
    }

    std::string daeFile = arguments[1]; //The daefile MUST be the first argument!!

    unsigned int x=50, y=50, width=800, height=600, framerate=-1;
    arguments.read("-pos", x, y);
    arguments.read("-window", width, height);
    arguments.read("-fr", framerate);
    
    std::string host;
    unsigned int port=0;
    bool communicationWithSocket = arguments.read("-socket", host, port);
    bool hasAnAnimation = false;




    //----------------- Load data -------------------//
    osg::Group* rootGroup = new osg::Group();
    osg::Node* fileNode  = osgDB::readNodeFile(daeFile);
    if(!fileNode)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    rootGroup->addChild(fileNode);
    parse(fileNode);

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

    if (hasAnAnimation || framerate<=0)
    {
        return viewer.run();
    }
    else
    {
        while (!viewer.done())
        {
            viewer.frame();
#ifdef UNIX
            usleep(1000000/framerate);
#elif WIN32
            Sleep(1000/framerate);
#endif
        }
        return 0;
    }
}




