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

#include "SocketCallback"

#include <iostream>

#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgAnimation/AnimationManagerBase>
#include <osgAnimation/BasicAnimationManager>



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




int main(int argc, char** argv) 
{
    //----------------- Get and parse arguments -------------------//
    osg::ArgumentParser arguments(&argc, argv);
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" TODO."); //TODO
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","TODO List command line options."); //TODO
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 0;
    }
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    std::string daeFile = arguments[1]; //The daefile MUST be the first argument!!

    int x=50, y=50, width=800, height=600;
    arguments.read("-x", x);
    arguments.read("-y", y);
    arguments.read("-w", width);
    arguments.read("-h", height);
    arguments.read("-window", x, y, width, height);
    
    std::string host;
    unsigned int port=0;
    bool communicationWithSocket = arguments.read("-socket", host, port);




    //----------------- Load data -------------------//
    osg::Group* rootGroup = new osg::Group();
    osg::Group* fileNode  = osgDB::readNodeFile(daeFile)->asGroup();
    if(!fileNode)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }
    rootGroup->addChild(fileNode);


    //----------------- Create Viewer and interface -------------------//
    osgViewer::ViewerExt viewer;
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

    return viewer.run();

}




